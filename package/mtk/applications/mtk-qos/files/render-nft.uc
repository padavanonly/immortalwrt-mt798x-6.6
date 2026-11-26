#!/usr/bin/env ucode
// SPDX-License-Identifier: GPL-2.0-only
// Render nftables inet/mtk_qos rules and QDMA EQoS slot mappings for mtk-qos.

'use strict';

import * as fs from 'fs';
import * as uci from 'uci';

function usage() {
	warn('Usage: render-nft.uc [--uci-dir DIR] [--leases PATH] [--arp PATH] ' +
	     '[--out-nft PATH] [--out-slots PATH] [--wan-devs CSV] [--lan-devs CSV] ' +
	     '[--with-connlimit 0|1] [--mtu-fix 0|1]\n');
	exit(1);
}

let opts = {
	uci_dir: '/etc/config',
	leases: '/tmp/dhcp.leases',
	arp: '/proc/net/arp',
	out_nft: '/var/run/mtk_qos/mtk_qos.nft',
	out_slots: '/var/run/mtk_qos/eqos_slots.tsv',
	wan_devs: 'eth1,wwan0_1',
	lan_devs: 'br-lan,eth0',
	with_connlimit: fs.access('/sys/module/nft_connlimit') ? 1 : 0,
	mtu_fix: 1,
};

for (let i = 0; i < length(ARGV); i++) {
	let a = ARGV[i];
	let v = ARGV[i + 1];
	if (a == '--uci-dir' && v != null) { opts.uci_dir = v; i++; }
	else if (a == '--leases' && v != null) { opts.leases = v; i++; }
	else if (a == '--arp' && v != null) { opts.arp = v; i++; }
	else if (a == '--out-nft' && v != null) { opts.out_nft = v; i++; }
	else if (a == '--out-slots' && v != null) { opts.out_slots = v; i++; }
	else if (a == '--wan-devs' && v != null) { opts.wan_devs = v; i++; }
	else if (a == '--lan-devs' && v != null) { opts.lan_devs = v; i++; }
	else if (a == '--with-connlimit' && v != null) { opts.with_connlimit = int(v); i++; }
	else if (a == '--mtu-fix' && v != null) { opts.mtu_fix = int(v); i++; }
	else usage();
}

function split_csv(s) {
	let seen = {};
	let res = [];
	for (let raw in split(s || '', /[,\s]+/)) {
		let d = trim(raw);
		if (d != '' && !seen[d]) {
			seen[d] = true;
			push(res, d);
		}
	}
	return res;
}

function fmt_ifset(devs) {
	let parts = map(devs, d => `"${replace(d, /"/g, '')}"`);
	if (length(parts) == 1)
		return parts[0];
	return '{ ' + join(', ', parts) + ' }';
}

function normalize_mac(mac) {
	if (!mac)
		return '';
	let m = lc(trim(mac));
	if (match(m, /^([0-9a-f]{2}:){5}[0-9a-f]{2}$/))
		return m;
	return '';
}

function is_ipv4(ip) {
	return !!match(trim(ip || ''), /^(\d{1,3}\.){3}\d{1,3}(\/\d{1,2})?$/);
}

function is_ipv6(ip) {
	return index(ip || '', ':') >= 0 && !!match(trim(ip || ''), /^[0-9a-fA-F:]+(\/\d{1,3})?$/);
}

function to_uint(val) {
	let n = int(trim('' + (val ?? '0')));
	return n > 0 ? n : 0;
}

function load_mac_ip_maps() {
	let mac_to_ip = {};
	let ip_to_mac = {};

	let lf = fs.open(opts.leases, 'r');
	if (lf) {
		for (let line = lf.read('line'); line != null; line = lf.read('line')) {
			let f = split(trim(line), /\s+/);
			if (length(f) >= 3) {
				let mac = normalize_mac(f[1]);
				let ip = trim(f[2]);
				if (mac != '' && is_ipv4(ip)) {
					mac_to_ip[mac] = ip;
					ip_to_mac[ip] = mac;
				}
			}
		}
		lf.close();
	}

	let af = fs.open(opts.arp, 'r');
	if (af) {
		let hdr = af.read('line');
		for (let line = af.read('line'); line != null; line = af.read('line')) {
			let f = split(trim(line), /\s+/);
			if (length(f) >= 4) {
				let ip = trim(f[0]);
				let flags = trim(f[2]);
				let mac = normalize_mac(f[3]);
				if (flags != '0x0' && mac != '' && mac != '00:00:00:00:00:00' && is_ipv4(ip)) {
					if (!mac_to_ip[mac])
						mac_to_ip[mac] = ip;
					if (!ip_to_mac[ip])
						ip_to_mac[ip] = mac;
				}
			}
		}
		af.close();
	}

	return { mac_to_ip, ip_to_mac };
}

function ensure_parent_dir(path) {
	let idx = rindex(path, '/');
	if (idx > 0) {
		let dir = substr(path, 0, idx);
		if (!fs.access(dir))
			fs.mkdir(dir, 0o755);
	}
}

let ctx = uci.cursor(opts.uci_dir);
let global_enabled = ctx.get('mtk_qos', 'global', 'enabled') == '1';
let eqos_enabled = ctx.get('mtk_qos', 'global', 'eqos_enabled') != '0';
let wan_devs = split_csv(opts.wan_devs);
let lan_devs = split_csv(opts.lan_devs);
if (length(wan_devs) == 0)
	wan_devs = ['eth1'];
if (length(lan_devs) == 0)
	lan_devs = ['br-lan'];

let maps = load_mac_ip_maps();
let slots = [];
let slot_id = 1;

if (global_enabled && eqos_enabled) {
	ctx.foreach('mtk_qos', 'device', function(s) {
		if (slot_id > 15)
			return;
		if (s.enabled == '0')
			return;

		let ip = trim(s.ip || '');
		let mac = normalize_mac(s.mac || '');
		let resolved_ip = ip;
		if (resolved_ip == '' && mac != '' && maps.mac_to_ip[mac])
			resolved_ip = maps.mac_to_ip[mac];
		let resolved_mac = mac;
		if (resolved_mac == '' && resolved_ip != '' && maps.ip_to_mac[resolved_ip])
			resolved_mac = maps.ip_to_mac[resolved_ip];

		let ul = to_uint(s.upload_kbps);
		let dl = to_uint(s.download_kbps);
		let max_conn = to_uint(s.max_conn);
		let comment = replace(trim(s.comment || ''), /[\t\r\n|]/g, ' ');

		if ((ip == '' && mac == '') || (ul == 0 && dl == 0 && max_conn == 0))
			return;

		push(slots, {
			slot: slot_id,
			ul_kbps: ul,
			dl_kbps: dl,
			max_conn: max_conn,
			ip: resolved_ip,
			mac: resolved_mac,
			explicit_ip: ip,
			comment: comment,
		});
		slot_id++;
	});
}

let wan_expr = fmt_ifset(wan_devs);
let lan_expr = fmt_ifset(lan_devs);

let lines = [
	'table inet mtk_qos',
	'delete table inet mtk_qos',
	'table inet mtk_qos {',
	'	chain forward {',
	'		type filter hook forward priority mangle; policy accept;',
];

if (opts.mtu_fix == 1) {
	push(lines, `\t\toifname ${wan_expr} tcp flags syn / syn,rst tcp option maxseg size set rt mtu comment "mtk-qos mss clamp ul"`);
	push(lines, `\t\tiifname ${wan_expr} tcp flags syn / syn,rst tcp option maxseg size set rt mtu comment "mtk-qos mss clamp dl"`);
}

for (let rule in slots) {
	let ul_mark = rule.slot;
	let dl_mark = 32 + rule.slot;

	if (rule.max_conn > 0 && opts.with_connlimit == 1) {
		if (is_ipv4(rule.ip)) {
			push(lines, `\t\toifname ${wan_expr} ip saddr ${rule.ip} ct state new ct count over ${rule.max_conn} counter drop comment "mtk-qos connlimit slot ${rule.slot}"`);
		} else if (is_ipv6(rule.ip)) {
			push(lines, `\t\toifname ${wan_expr} ip6 saddr ${rule.ip} ct state new ct count over ${rule.max_conn} counter drop comment "mtk-qos connlimit6 slot ${rule.slot}"`);
		}
	}

	if (rule.ul_kbps > 0) {
		if (is_ipv4(rule.ip))
			push(lines, `\t\tiifname ${lan_expr} oifname ${wan_expr} ip saddr ${rule.ip} meta mark set ${ul_mark} comment "mtk-qos eqos ul ip slot ${rule.slot}"`);
		else if (is_ipv6(rule.ip))
			push(lines, `\t\tiifname ${lan_expr} oifname ${wan_expr} ip6 saddr ${rule.ip} meta mark set ${ul_mark} comment "mtk-qos eqos ul ip6 slot ${rule.slot}"`);
		if (rule.mac != '')
			push(lines, `\t\tiifname ${lan_expr} oifname ${wan_expr} ether saddr ${rule.mac} meta mark set ${ul_mark} comment "mtk-qos eqos ul mac slot ${rule.slot}"`);
	}

	if (rule.dl_kbps > 0) {
		if (is_ipv4(rule.ip))
			push(lines, `\t\tiifname ${wan_expr} oifname ${lan_expr} ip daddr ${rule.ip} meta mark set ${dl_mark} comment "mtk-qos eqos dl ip slot ${rule.slot}"`);
		else if (is_ipv6(rule.ip))
			push(lines, `\t\tiifname ${wan_expr} oifname ${lan_expr} ip6 daddr ${rule.ip} meta mark set ${dl_mark} comment "mtk-qos eqos dl ip6 slot ${rule.slot}"`);
	}
}

push(lines, '\t}');
push(lines, '}');

ensure_parent_dir(opts.out_nft);
let nf = fs.open(opts.out_nft, 'w');
if (!nf) {
	warn(`failed to open ${opts.out_nft} for writing\n`);
	exit(2);
}
nf.write(join('\n', lines) + '\n');
nf.close();

ensure_parent_dir(opts.out_slots);
let sf = fs.open(opts.out_slots, 'w');
if (!sf) {
	warn(`failed to open ${opts.out_slots} for writing\n`);
	exit(2);
}
for (let rule in slots) {
	sf.write(sprintf('%d|%d|%d|%d|%s|%s|%s\n',
		rule.slot, rule.ul_kbps, rule.dl_kbps, rule.max_conn,
		rule.ip, rule.mac, rule.comment));
}
sf.close();
exit(0);
