#!/usr/bin/ucode
// SPDX-License-Identifier: GPL-2.0-only
'use strict';

import * as fs from 'fs';
import { cursor } from 'uci';

let STATE_DIR = getenv('EQOS_STATE_DIR') || '/var/run/eqos';
let FW4_STATE = getenv('EQOS_FW4_STATE') || `${STATE_DIR}/../fw4.state`;
let UCI_DIR = getenv('EQOS_UCI_DIR') || '/etc/config';
let UCI_SAVEDIR = getenv('EQOS_UCI_SAVEDIR') || '';
let NFT_BIN = getenv('EQOS_NFT_BIN') || 'nft';
let UBUS_BIN = getenv('EQOS_UBUS_BIN') || 'ubus';
let IP_BIN = getenv('EQOS_IP_BIN') || 'ip';
let TC_BIN = getenv('EQOS_TC_BIN') || 'tc';
let UCI_BIN = getenv('EQOS_UCI_BIN') || 'uci';
let UCODE_BIN = getenv('EQOS_UCODE_BIN') || '/usr/bin/ucode';
let UCODE_ARGS = getenv('EQOS_UCODE_ARGS') || '';
let LIBEXEC_DIR = getenv('EQOS_LIBEXEC_DIR') || '/usr/libexec/eqos';
let SBIN_EQOS = getenv('EQOS_SBIN_EQOS') || '/usr/sbin/eqos';
let LOADBALANCE_BIN = getenv('EQOS_LOADBALANCE_BIN') || '/usr/sbin/loadbalance';
let PROC_DIR = getenv('EQOS_PROC_DIR') || '/proc';
let HNAT_DIR = getenv('EQOS_HNAT_DIR') || '/sys/kernel/debug/hnat';
let FIREWALL_USER = getenv('EQOS_FIREWALL_USER') || '/etc/firewall.user';
let NOW_EPOCH = getenv('EQOS_NOW_EPOCH') ? int(getenv('EQOS_NOW_EPOCH')) : time();
let TOMBSTONE_GRACE = getenv('EQOS_TOMBSTONE_GRACE') ? int(getenv('EQOS_TOMBSTONE_GRACE')) : 300;

function has_key(obj, key) {
	if (type(obj) != 'object')
		return false;
	for (let k in obj)
		if (k == key)
			return true;
	return false;
}

function mkdir_p(dir) {
	if (!dir || dir == '' || dir == '/')
		return true;
	if (fs.stat(dir))
		return true;
	let parent = fs.dirname(dir);
	if (parent && parent != dir)
		mkdir_p(parent);
	fs.mkdir(dir);
	return fs.stat(dir) != null;
}

let TMP_SEQ = 0;

function write_file_atomic(path, content) {
	let st_dst = fs.lstat(path);
	if (st_dst && (st_dst.type == 'link' || st_dst.type == 'symlink'))
		return false;
	if (!mkdir_p(fs.dirname(path)))
		return false;
	let tmp = sprintf('%s.tmp.%d.%d', path, time(), ++TMP_SEQ);
	let fp = fs.open(tmp, 'w');
	if (!fp)
		return false;

	let s = `${content == null ? '' : content}`;
	let expected = length(s);
	let written = fp.write(s);
	if (written === null || type(written) != 'int' || written != expected) {
		fp.close();
		fs.unlink(tmp);
		return false;
	}

	if (!fp.flush()) {
		fp.close();
		fs.unlink(tmp);
		return false;
	}

	fp.close();

	let readback = fs.readfile(tmp);
	if (readback === null || readback != s) {
		fs.unlink(tmp);
		return false;
	}

	if (fs.rename(tmp, path) != true) {
		fs.unlink(tmp);
		return false;
	}

	return true;
}

function write_json_atomic(path, obj) {
	return write_file_atomic(path, sprintf('%J\n', obj));
}

function write_debugfs_node(path, content) {
	let s = `${content == null ? '' : content}`;
	let fp = fs.open(path, 'w');
	if (!fp)
		return false;
	let written = fp.write(s);
	if (written === null || type(written) != 'int' || written != length(s)) {
		fp.close();
		return false;
	}
	if (!fp.flush()) {
		fp.close();
		return false;
	}
	fp.close();
	return true;
}

function sh_quote(val) {
	let s = `${val == null ? '' : val}`;
	return "'" + replace(s, "'", "'\\''") + "'";
}

function run_cmd(cmd) {
	let p = fs.popen(cmd, 'r');
	if (!p)
		return { rc: 127, stdout: '' };
	let out = p.read('all') || '';
	let rc = p.close();
	return { rc, stdout: out };
}

function run_nft_stdin(script) {
	let tmp = `${STATE_DIR}/cleanup.nft`;
	if (!write_file_atomic(tmp, script))
		return { rc: 1, stdout: 'failed to write cleanup.nft' };
	let res = run_cmd(`${sh_quote(NFT_BIN)} -f - < ${sh_quote(tmp)}`);
	fs.unlink(tmp);
	return res;
}

let trigger = 'reload';
for (let i = 0; i < length(ARGV); i++) {
	let arg = ARGV[i];
	if (arg == '--trigger' && i + 1 < length(ARGV)) {
		trigger = ARGV[++i];
	}
	else if (substr(arg, 0, 10) == '--trigger=') {
		trigger = substr(arg, 10);
	}
	else if (substr(arg, 0, 1) != '-') {
		trigger = arg;
	}
}

if (!mkdir_p(STATE_DIR)) {
	warn('eqos-reconcile: cannot create state directory\n');
	exit(1);
}
let lock_fp = fs.open(`${STATE_DIR}/reconcile.lock`, 'w');
if (!lock_fp || !lock_fp.lock('x')) {
	warn('eqos-reconcile: cannot acquire reconcile.lock\n');
	exit(1);
}

function write_status(status, extra) {
	let doc = {
		schema: 'eqos-reconcile-status/v1',
		status,
		trigger
	};
	if (type(extra) == 'object')
		for (let k in extra)
			doc[k] = extra[k];
	return write_json_atomic(`${STATE_DIR}/status.json`, doc);
}

if (fs.stat(`${STATE_DIR}/legacy-reboot-required`)) {
	if (!write_status('pending_reboot_after_legacy_upgrade', {}))
		exit(1);
	exit(0);
}

function inspect_table(family, table_name) {
	let list_res = run_cmd(`${sh_quote(NFT_BIN)} -j list tables 2>&1`);
	if (list_res.rc != 0) {
		return {
			ok: false,
			exists: false,
			owned: false,
			raw: '',
			error: trim(list_res.stdout) || 'nft -j list tables failed',
			rc: list_res.rc
		};
	}
	let doc = null;
	try {
		doc = json(list_res.stdout);
	}
	catch (e) {
		doc = null;
	}
	if (type(doc) != 'object' || type(doc.nftables) != 'array') {
		return {
			ok: false,
			exists: false,
			owned: false,
			raw: '',
			error: 'invalid JSON from nft -j list tables',
			rc: 1
		};
	}
	let found = false;
	for (let item in doc.nftables) {
		if (type(item) == 'object' && type(item.table) == 'object' &&
		    item.table.family == family && item.table.name == table_name) {
			found = true;
			break;
		}
	}
	if (!found) {
		return {
			ok: true,
			exists: false,
			owned: false,
			raw: '',
			error: '',
			rc: 0
		};
	}
	let detail_res = run_cmd(`${sh_quote(NFT_BIN)} list table ${sh_quote(family)} ${sh_quote(table_name)} 2>&1`);
	if (detail_res.rc != 0) {
		return {
			ok: false,
			exists: true,
			owned: false,
			raw: '',
			error: trim(detail_res.stdout) || sprintf('nft list table %s %s failed', family, table_name),
			rc: detail_res.rc
		};
	}
	let owned = index(detail_res.stdout, 'eqos-owner:v1') >= 0;
	return {
		ok: true,
		exists: true,
		owned,
		raw: detail_res.stdout,
		error: '',
		rc: 0
	};
}

function verify_no_unmarked_tables() {
	let inet_info = inspect_table('inet', 'eqos');
	if (!inet_info.ok) {
		write_status('error', {
			code: 'nft_inspect_failed',
			family: 'inet',
			table: 'eqos',
			detail: inet_info.error
		});
		exit(1);
	}
	if (inet_info.exists && !inet_info.owned) {
		write_status('error', {
			code: 'unmarked_table_conflict',
			family: 'inet',
			table: 'eqos',
			detail: 'refusing to clobber existing inet eqos table without eqos-owner:v1 marker'
		});
		exit(1);
	}
	let bridge_info = inspect_table('bridge', 'eqos');
	if (!bridge_info.ok) {
		write_status('error', {
			code: 'nft_inspect_failed',
			family: 'bridge',
			table: 'eqos',
			detail: bridge_info.error
		});
		exit(1);
	}
	if (bridge_info.exists && !bridge_info.owned) {
		write_status('error', {
			code: 'unmarked_table_conflict',
			family: 'bridge',
			table: 'eqos',
			detail: 'refusing to clobber existing bridge eqos table without eqos-owner:v1 marker'
		});
		exit(1);
	}
	return { inet: inet_info, bridge: bridge_info };
}

function delete_owned_tables(families) {
	let info = verify_no_unmarked_tables();
	let lines = [];
	for (let fam in families) {
		if (info[fam] && info[fam].exists && info[fam].owned)
			push(lines, `delete table ${fam} eqos`);
	}
	if (length(lines)) {
		let res = run_nft_stdin(join('\n', lines) + '\n');
		if (res.rc != 0) {
			write_status('error', {
				code: 'nft_delete_failed',
				detail: res.stdout
			});
			exit(1);
		}
	}
}

function recover_prior_transaction() {
	let journal_file = `${STATE_DIR}/journal.json`;
	if (fs.stat(journal_file)) {
		let rec_res = run_cmd(`POSIXLY_CORRECT=1 bash ${sh_quote(LOADBALANCE_BIN)} --stage=recover`);
		if (rec_res.rc != 0) {
			let err_doc = null;
			try { err_doc = json(rec_res.stdout); } catch (e) {}
			write_status('error', {
				code: (err_doc && err_doc.code) ? err_doc.code : 'runtime_helper_failed',
				failed_stage: 'journal_recover',
				partial_completion: true,
				detail: (err_doc && err_doc.detail) ? err_doc.detail : rec_res.stdout
			});
			exit(1);
		}
	}
}

if (trigger == 'stop') {
	recover_prior_transaction();
	let eqos_res = run_cmd(`${sh_quote(SBIN_EQOS)} stop`);
	if (eqos_res.rc != 0) {
		write_status('error', {
			code: 'runtime_helper_failed',
			failed_stage: 'eqos_stop',
			partial_completion: true,
			detail: eqos_res.stdout
		});
		exit(1);
	}
	let lb_res = run_cmd(`POSIXLY_CORRECT=1 bash ${sh_quote(LOADBALANCE_BIN)} "[]"`);
	if (lb_res.rc != 0) {
		let err_doc = null;
		try { err_doc = json(lb_res.stdout); } catch (e) {}
		write_status('error', {
			code: (err_doc && err_doc.code) ? err_doc.code : 'runtime_helper_failed',
			failed_stage: 'loadbalance_stop',
			partial_completion: true,
			detail: (err_doc && err_doc.detail) ? err_doc.detail : lb_res.stdout
		});
		exit(1);
	}
	delete_owned_tables([ 'inet', 'bridge' ]);
	if (!write_status('stopped', {}))
		exit(1);
	if (fs.stat(`${STATE_DIR}/journal.json`))
		fs.unlink(`${STATE_DIR}/journal.json`);
	exit(0);
}

function parse_bool(val, def_val) {
	if (val == null || val == '')
		return def_val;
	let s = lc(`${val}`);
	if (s == '1' || s == 'true' || s == 'yes' || s == 'on')
		return true;
	if (s == '0' || s == 'false' || s == 'no' || s == 'off')
		return false;
	return def_val;
}

function parse_int_or(val, def_val) {
	if (val == null || val == '')
		return def_val;
	let n = int(val);
	return (type(n) == 'int') ? n : def_val;
}

function load_eqos_config(ctx) {
	let all = ctx ? ctx.get_all('eqos') : null;
	let cfg_sec = null;
	if (type(all) == 'object') {
		if (has_key(all, 'config') && type(all.config) == 'object') {
			cfg_sec = all.config;
		}
		else {
			for (let k in all) {
				if (type(all[k]) == 'object' && all[k]['.type'] == 'eqos') {
					cfg_sec = all[k];
					break;
				}
			}
		}
	}

	let enabled = cfg_sec ? parse_bool(cfg_sec.enabled, false) : false;
	let ipv6_enabled = cfg_sec ? parse_bool(cfg_sec.ipv6enabled, false) : false;
	let download_mbit = cfg_sec ? parse_int_or(cfg_sec.download, 100) : 100;
	let upload_mbit = cfg_sec ? parse_int_or(cfg_sec.upload, 20) : 20;
	let comment = (cfg_sec && cfg_sec.comment != null) ? `${cfg_sec.comment}` : '';
	let smartqos = cfg_sec ? parse_bool(cfg_sec.smarthqos, false) : false;
	let iface_raw = '';
	if (cfg_sec && cfg_sec.interface != null) {
		if (type(cfg_sec.interface) == 'array')
			iface_raw = join(',', cfg_sec.interface);
		else
			iface_raw = trim(`${cfg_sec.interface}`);
	}

	let bypass_v4 = [];
	if (cfg_sec && cfg_sec.bypass_v4 != null) {
		let raw_list = (type(cfg_sec.bypass_v4) == 'array') ? cfg_sec.bypass_v4 : split(`${cfg_sec.bypass_v4}`, /[,\s]+/);
		for (let item in raw_list) {
			let s = trim(`${item}`);
			if (s != '')
				push(bypass_v4, s);
		}
	}

	let static_clients = [];
	let comments_by_key = {};
	if (ctx) {
		ctx.foreach('eqos', 'device', function(sec) {
			if (!parse_bool(sec.enabled, true))
				return;
			let ip = (sec.ip != null && trim(`${sec.ip}`) != '') ? trim(`${sec.ip}`) : null;
			let mac = (sec.mac != null && trim(`${sec.mac}`) != '') ? lc(trim(`${sec.mac}`)) : null;
			if (!ip && !mac)
				return;
			let dl_kbit = parse_int_or(sec.download, 0);
			let up_kbit = parse_int_or(sec.upload, 0);
			let rec = {
				upload: up_kbit > 0,
				download: dl_kbit > 0,
				upload_kbit: up_kbit,
				download_kbit: dl_kbit
			};
			if (ip)
				rec.ipv4 = ip;
			if (mac)
				rec.mac = mac;
			if (sec.interfacename != null && trim(`${sec.interfacename}`) != '')
				rec.interfacename = trim(`${sec.interfacename}`);
			let dev_comment = (sec.comment != null) ? `${sec.comment}` : '';
			if (ip)
				comments_by_key[`ipv4:${ip}`] = dev_comment;
			if (mac)
				comments_by_key[`mac:${mac}`] = dev_comment;
			push(static_clients, rec);
		});
	}

	return {
		enabled,
		ipv6_enabled,
		download_mbit,
		upload_mbit,
		comment,
		smartqos,
		interface: iface_raw,
		bypass_v4,
		static_clients,
		comments_by_key
	};
}

let uci_ctx = (UCI_SAVEDIR != '') ? cursor(UCI_DIR, UCI_SAVEDIR) : cursor(UCI_DIR);
let cfg = load_eqos_config(uci_ctx);

if (!cfg.enabled) {
	recover_prior_transaction();
	if (trigger == 'start' || trigger == 'reload') {
		let eqos_res = run_cmd(`${sh_quote(SBIN_EQOS)} stop`);
		if (eqos_res.rc != 0) {
			write_status('error', {
				code: 'runtime_helper_failed',
				failed_stage: 'eqos_stop',
				partial_completion: true,
				detail: eqos_res.stdout
			});
			exit(1);
		}
	}
	let lb_res = run_cmd(`POSIXLY_CORRECT=1 bash ${sh_quote(LOADBALANCE_BIN)} "[]"`);
	if (lb_res.rc != 0) {
		let err_doc = null;
		try { err_doc = json(lb_res.stdout); } catch (e) {}
		write_status('error', {
			code: (err_doc && err_doc.code) ? err_doc.code : 'runtime_helper_failed',
			failed_stage: 'loadbalance_stop',
			partial_completion: true,
			detail: (err_doc && err_doc.detail) ? err_doc.detail : lb_res.stdout
		});
		exit(1);
	}
	if (trigger == 'start' || trigger == 'reload' || trigger == 'fw4' || trigger == 'install')
		delete_owned_tables([ 'inet', 'bridge' ]);
	if (!write_status('disabled', {}))
		exit(1);
	if (fs.stat(`${STATE_DIR}/journal.json`))
		fs.unlink(`${STATE_DIR}/journal.json`);
	exit(0);
}

function is_fw4_alive() {
	if (!fs.stat(FW4_STATE))
		return false;
	let fw4_info = inspect_table('inet', 'fw4');
	if (!fw4_info.ok) {
		write_status('error', {
			code: 'nft_inspect_failed',
			family: 'inet',
			table: 'fw4',
			detail: fw4_info.error
		});
		exit(1);
	}
	return fw4_info.exists;
}

if (!is_fw4_alive()) {
	delete_owned_tables([ 'bridge', 'inet' ]);
	if (!write_file_atomic(`${STATE_DIR}/pending_fw4`, '1\n'))
		exit(1);
	if (!write_status('pending_fw4', {}))
		exit(1);
	exit(0);
}

function read_proc_starttime(pid) {
	if (pid == null)
		return null;
	let raw = fs.readfile(`${PROC_DIR}/${pid}/stat`);
	if (raw == null)
		return null;
	let s = trim(raw);
	let close_paren = rindex(s, ')');
	if (close_paren >= 0 && close_paren + 2 < length(s)) {
		let rest = trim(substr(s, close_paren + 1));
		let parts = split(rest, /\s+/);
		if (length(parts) >= 20) {
			let starttime = parts[19];
			let matched = match(starttime, /^[0-9]+$/);
			if (type(matched) == 'array' && length(matched) && matched[0] == starttime)
				return starttime;
		}
	}
	return null;
}

function is_uint32(value) {
	return type(value) == 'int' && value >= 0 && value <= 4294967295;
}

function valid_snapshot_record(record) {
	if (type(record) != 'object' ||
	    type(record.address) != 'string' ||
	    type(record.mac) != 'string' ||
	    type(record.expires_epoch) != 'int' || record.expires_epoch < 0)
		return false;

	let ipv4_match = match(record.address, /^(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$/);
	let mac_match = match(record.mac, /^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/);
	return type(ipv4_match) == 'array' && length(ipv4_match) && ipv4_match[0] == record.address &&
	       type(mac_match) == 'array' && length(mac_match) && mac_match[0] == record.mac;
}

function valid_snapshot_reply(doc, obj_name, metadata_only) {
	if (type(doc) != 'object' ||
	    type(doc.schema) != 'string' || doc.schema != 'eqos-dnsmasq-leases/v1' ||
	    type(doc.instance) != 'string' || doc.instance != obj_name ||
	    type(doc.pid) != 'int' || doc.pid <= 0 || !is_uint32(doc.pid) ||
	    type(doc.generation) != 'int' || doc.generation < 0 ||
	    type(doc.snapshot_now_epoch) != 'int' || doc.snapshot_now_epoch != NOW_EPOCH ||
	    type(doc.metadata_only) != 'bool' || doc.metadata_only != metadata_only ||
	    !is_uint32(doc.record_count) ||
	    !is_uint32(doc.unclassified_count) || doc.unclassified_count != 0 ||
	    type(doc.complete) != 'bool' || doc.complete !== true)
		return false;

	if (metadata_only)
		return !has_key(doc, 'records');

	if (type(doc.records) != 'array' || length(doc.records) != doc.record_count)
		return false;

	for (let record in doc.records)
		if (!valid_snapshot_record(record))
			return false;

	return true;
}

function call_ubus_snapshot(obj_name, metadata_only) {
	let payload = {
		now_epoch: NOW_EPOCH,
		metadata_only
	};
	let cmd = `${sh_quote(UBUS_BIN)} call ${sh_quote(obj_name)} lease_snapshot ${sh_quote(sprintf('%J', payload))} 2>/dev/null`;
	let res = run_cmd(cmd);
	if (res.rc != 0)
		return null;
	let doc;
	try {
		doc = json(res.stdout);
	}
	catch (e) {
		return null;
	}
	if (!valid_snapshot_reply(doc, obj_name, metadata_only))
		return null;
	return doc;
}

function filter_unexpired_leases(records) {
	let out = [];
	if (type(records) != 'array')
		return out;
	for (let r in records) {
		if (type(r) != 'object')
			continue;
		let ipv4 = r.ipv4 || r.address;
		let mac = r.mac ? lc(`${r.mac}`) : null;
		let exp = (r.expires_epoch != null) ? int(r.expires_epoch) : null;
		if (!ipv4 || !mac || type(exp) != 'int' || exp < 0)
			continue;
		if (exp != 0 && exp <= NOW_EPOCH)
			continue;
		push(out, {
			mac,
			ipv4: `${ipv4}`,
			expires_epoch: exp
		});
	}
	return out;
}

function collect_leases(ctx) {
	let instances = [];
	let dhcp_all = ctx ? ctx.get_all('dhcp') : null;
	if (type(dhcp_all) == 'object') {
		ctx.foreach('dhcp', 'dnsmasq', function(sec) {
			if (sec.disabled == '1' || sec.no_dhcpv4 == '1')
				return;
			let leasefile = (sec.leasefile != null && trim(`${sec.leasefile}`) != '')
				? trim(`${sec.leasefile}`)
				: '/tmp/dhcp.leases';
			let ubus_enabled = (sec.ubus != '0');
			let name = sec['.name'] || 'dnsmasq';
			let anon = (sec['.anonymous'] == true);
			let inst_opt = (sec.instance != null && trim(`${sec.instance}`) != '') ? trim(`${sec.instance}`) : null;
			push(instances, {
				name,
				anonymous: anon,
				instance_opt: inst_opt,
				leasefile,
				ubus_enabled
			});
		});
	}
	else {
		push(instances, {
			name: 'dnsmasq',
			anonymous: true,
			instance_opt: null,
			leasefile: '/tmp/dhcp.leases',
			ubus_enabled: true
		});
	}

	let seen_leasefiles = {};
	for (let inst in instances) {
		if (has_key(seen_leasefiles, inst.leasefile)) {
			write_status('error', {
				code: 'duplicate_leasefile',
				leasefile: inst.leasefile,
				detail: `Multiple active dnsmasq instances share leasefile ${inst.leasefile}`
			});
			exit(1);
		}
		seen_leasefiles[inst.leasefile] = inst.name;
	}

	for (let inst in instances) {
		if (!inst.ubus_enabled)
			return { status: 'PENDING', reason: 'ubus_disabled', records: [] };
	}

	let all_records = [];
	for (let inst in instances) {
		let primary_obj = 'dnsmasq';
		if (inst.instance_opt)
			primary_obj = `dnsmasq.${inst.instance_opt}`;
		else if (!inst.anonymous)
			primary_obj = `dnsmasq.${inst.name}`;

		let stable_records = null;
		let obj_to_use = primary_obj;
		for (let attempt = 0; attempt < 3; attempt++) {
			let p1 = call_ubus_snapshot(obj_to_use, false);
			if (!p1 && obj_to_use != 'dnsmasq' && length(instances) == 1 && attempt == 0) {
				obj_to_use = 'dnsmasq';
				p1 = call_ubus_snapshot(obj_to_use, false);
			}
			if (!p1)
				continue;
			let st1 = read_proc_starttime(p1.pid);
			if (st1 == null)
				continue;

			let p2 = call_ubus_snapshot(obj_to_use, true);
			if (!p2)
				continue;
			let st2 = read_proc_starttime(p2.pid);
			if (st2 == null)
				continue;

			if (p1.instance == p2.instance &&
			    p1.pid == p2.pid &&
			    p1.generation == p2.generation &&
			    p1.record_count == p2.record_count &&
			    st1 == st2) {
				stable_records = filter_unexpired_leases(p1.records);
				break;
			}
		}

		if (stable_records == null)
			return { status: 'PENDING', reason: 'ubus_snapshot_unstable_or_unavailable', records: [] };

		for (let r in stable_records)
			push(all_records, r);
	}

	return { status: 'COMPLETE', reason: 'ok', records: all_records };
}

let lease_status = 'DISABLED';
let lease_reason = 'smartqos_disabled';
let active_leases = [];
let used_last_good_leases = false;

if (cfg.smartqos) {
	let collected = collect_leases(uci_ctx);
	lease_status = collected.status;
	lease_reason = collected.reason;
	if (lease_status == 'COMPLETE') {
		active_leases = collected.records;
	}
	else {
		let last_good_raw = fs.readfile(`${STATE_DIR}/last-good-leases.json`);
		if (last_good_raw != null) {
			try {
				let parsed = json(last_good_raw);
				let arr = (type(parsed) == 'array') ? parsed : (type(parsed) == 'object' ? parsed.records : null);
				if (type(arr) == 'array') {
					active_leases = filter_unexpired_leases(arr);
					used_last_good_leases = true;
				}
			}
			catch (e) {
				active_leases = [];
			}
		}
	}
}

function ipv4_octets(address) {
	let matched = match(address, /^(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$/);
	if (type(matched) != 'array' || !length(matched) || matched[0] != address)
		return null;
	let parts = split(address, '.');
	let octets = [];
	for (let part in parts)
		push(octets, int(part));
	return octets;
}

function parse_link_route_prefix(destination) {
	let parts = split(destination, '/');
	if (length(parts) > 2)
		return null;
	let octets = ipv4_octets(parts[0]);
	if (!octets)
		return null;
	let prefix = 32;
	if (length(parts) == 2) {
		let prefix_match = match(parts[1], /^(0|[1-9]|[12][0-9]|3[0-2])$/);
		if (type(prefix_match) != 'array' || !length(prefix_match) || prefix_match[0] != parts[1])
			return null;
		prefix = int(parts[1]);
	}
	return { octets, prefix, network: destination };
}

function prefix_contains(prefix, address_octets) {
	let whole_octets = int(prefix.prefix / 8);
	let partial_bits = prefix.prefix % 8;
	for (let i = 0; i < whole_octets; i++)
		if (prefix.octets[i] != address_octets[i])
			return false;
	if (partial_bits > 0) {
		let mask = 256 - (1 << (8 - partial_bits));
		if ((prefix.octets[whole_octets] & mask) != (address_octets[whole_octets] & mask))
			return false;
	}
	return true;
}

function link_subnet_for_gateway(route_lines, ifname, gateway) {
	let gateway_octets = ipv4_octets(gateway);
	if (!gateway_octets)
		return '';
	let best_prefix = -1;
	let best_network = '';
	for (let line in route_lines) {
		let tokens = split(trim(line), /\s+/);
		if (!length(tokens) || tokens[0] == 'default')
			continue;
		let has_link_scope = false;
		let exact_dev = false;
		for (let i = 0; i < length(tokens); i++) {
			if (tokens[i] == 'scope' && i + 1 < length(tokens) && tokens[i + 1] == 'link')
				has_link_scope = true;
			if (tokens[i] == 'dev' && i + 1 < length(tokens) && tokens[i + 1] == ifname)
				exact_dev = true;
		}
		if (!has_link_scope || !exact_dev)
			continue;
		let route_prefix = parse_link_route_prefix(tokens[0]);
		if (route_prefix && route_prefix.prefix > best_prefix && prefix_contains(route_prefix, gateway_octets)) {
			best_prefix = route_prefix.prefix;
			best_network = route_prefix.network;
		}
	}
	return best_network;
}

function collect_wan_balance(iface_csv) {
	if (!iface_csv || iface_csv == '') {
		return {
			enabled: false,
			interfaces: [],
			special_marks: []
		};
	}

	let parts = split(iface_csv, ',');
	let ifnames = [];
	for (let p in parts) {
		let s = trim(p);
		if (s != '')
			push(ifnames, s);
	}
	if (!length(ifnames)) {
		return {
			enabled: false,
			interfaces: [],
			special_marks: []
		};
	}

	let interfaces = [];
	let route_res = run_cmd(`${sh_quote(IP_BIN)} -4 route show table main 2>/dev/null`);
	if (route_res.rc != 0) {
		write_status('error', {
			code: 'wan_route_query_failed',
			detail: route_res.stdout
		});
		exit(1);
	}
	let route_lines = split(route_res.stdout || '', '\n');
	for (let i = 0; i < length(ifnames); i++) {
		let ifname = ifnames[i];
		let gateway = '';
		let found = false;
		for (let line in route_lines) {
			let tokens = split(trim(line), /\s+/);
			if (!length(tokens) || tokens[0] != 'default')
				continue;
			let exact_dev = false;
			let line_gateway = '';
			for (let j = 0; j < length(tokens); j++) {
				if (tokens[j] == 'dev' && j + 1 < length(tokens) && tokens[j + 1] == ifname)
					exact_dev = true;
				if (tokens[j] == 'via' && j + 1 < length(tokens))
					line_gateway = tokens[j + 1];
			}
			if (!exact_dev)
				continue;
			found = true;
			if (line_gateway != '' && ipv4_octets(line_gateway))
				gateway = line_gateway;
			break;
		}
		if (!found)
			continue;
		let subnet = gateway ? link_subnet_for_gateway(route_lines, ifname, gateway) : '';
		push(interfaces, {
			index: i,
			ifname,
			gateway,
			subnet,
			mark: `2${i}`,
			table: `2${i}0`
		});
	}

	return {
		enabled: length(interfaces) > 0,
		interfaces,
		special_marks: length(interfaces) > 0 ? [
			{ protocol: 'tcp', destination_port: 443, mark: interfaces[0].mark },
			{ protocol: 'udp', destination_port: 443, mark: interfaces[0].mark }
		] : []
	};
}

let wan_balance = collect_wan_balance(cfg.interface);

let prior_registry = null;
let reg_path = `${STATE_DIR}/slot-registry.json`;
if (fs.stat(reg_path)) {
	let raw_reg = fs.readfile(reg_path);
	try {
		prior_registry = json(raw_reg);
	}
	catch (e) {
		write_status('error', {
			code: 'invalid_registry',
			detail: `failed to parse slot-registry.json: ${e}`
		});
		exit(1);
	}
}

let planner_input = {
	schema: 'eqos-planner-input/v1',
	now_epoch: NOW_EPOCH,
	tombstone_grace_seconds: TOMBSTONE_GRACE,
	snapshot: {
		policy: {
			enabled: cfg.enabled,
			ipv6_enabled: cfg.ipv6_enabled,
			shaper: {
				download_mbit: cfg.download_mbit,
				upload_mbit: cfg.upload_mbit,
				smartqos: cfg.smartqos
			},
			wan_balance
		},
		static_clients: cfg.static_clients,
		active_leases
	},
	prior_registry
};

let planner_input_path = `${STATE_DIR}/planner-input.json`;
if (!write_json_atomic(planner_input_path, planner_input)) {
	write_status('error', {
		code: 'state_write_failed',
		detail: 'failed to write planner-input.json'
	});
	exit(1);
}

let plan_cmd = `${sh_quote(UCODE_BIN)} ${UCODE_ARGS} ${sh_quote(LIBEXEC_DIR + '/policy-plan.uc')} ${sh_quote(planner_input_path)}`;
let plan_res = run_cmd(plan_cmd);
let plan_doc = null;
try {
	plan_doc = json(plan_res.stdout);
}
catch (e) {
	plan_doc = null;
}

if (plan_res.rc != 0 || type(plan_doc) != 'object' || plan_doc.status != 'ready') {
	write_status('error', {
		code: (type(plan_doc) == 'object' && plan_doc.code) ? plan_doc.code : 'planner_failed',
		detail: (type(plan_doc) == 'object' && plan_doc.detail) ? plan_doc.detail : plan_res.stdout
	});
	exit(1);
}

let table_state = verify_no_unmarked_tables();
let plan = plan_doc.plan;
if (type(plan.bypass_v4) != 'array')
	plan.bypass_v4 = [];
if (length(cfg.bypass_v4) > 0) {
	for (let b in cfg.bypass_v4) {
		let exists = false;
		for (let cur in plan.bypass_v4)
			if (cur == b)
				exists = true;
		if (!exists)
			push(plan.bypass_v4, b);
	}
}
if (plan.wan_balance && plan.wan_balance.enabled && type(plan.wan_balance.interfaces) == 'array') {
	for (let w in plan.wan_balance.interfaces) {
		if (w.subnet) {
			let exists = false;
			for (let cur in plan.bypass_v4)
				if (cur == w.subnet)
					exists = true;
			if (!exists)
				push(plan.bypass_v4, w.subnet);
		}
	}
}

function build_lease_maps(plan) {
	let v4_up_elems = [];
	let v4_dl_elems = [];
	let v6_up_elems = [];
	let v6_dl_elems = [];
	let seen_v4_up = {};
	let seen_v4_dl = {};
	let seen_v6_up = {};
	let seen_v6_dl = {};

	if (type(plan.lease_clients) == 'array') {
		for (let c in plan.lease_clients) {
			let pair = c.slot + 32;
			if (c.ipv4 && c.upload && !has_key(seen_v4_up, c.ipv4)) {
				seen_v4_up[c.ipv4] = true;
				push(v4_up_elems, `${c.ipv4} : jump dscp_slot_${c.slot}`);
			}
			if (c.ipv4 && c.download && !has_key(seen_v4_dl, c.ipv4)) {
				seen_v4_dl[c.ipv4] = true;
				push(v4_dl_elems, `${c.ipv4} : jump dscp_slot_${pair}`);
			}
			if (c.mac && c.upload && !has_key(seen_v6_up, c.mac)) {
				seen_v6_up[c.mac] = true;
				push(v6_up_elems, `${c.mac} : ${c.slot}`);
			}
			if (c.mac && c.download && !has_key(seen_v6_dl, c.mac)) {
				seen_v6_dl[c.mac] = true;
				push(v6_dl_elems, `${c.mac} : ${pair}`);
			}
		}
	}

	return {
		v4_up: v4_up_elems,
		v4_dl: v4_dl_elems,
		v6_up: v6_up_elems,
		v6_dl: v6_dl_elems
	};
}

function render_map_elements(map_data) {
	if (type(map_data) == 'array')
		return map_data;
	if (type(map_data) == 'object') {
		let elems = [];
		for (let k in map_data)
			push(elems, `${k} : ${map_data[k]}`);
		return elems;
	}
	return [];
}

function wan_mark_for_client(index, interfaces) {
	if (index == null || `${index}` == '')
		return null;
	let text = `${index}`;
	let matched = match(text, /^(0|[1-9][0-9]*)$/);
	if (type(matched) != 'array' || !length(matched) || matched[0] != text)
		return null;
	if (!length(interfaces))
		return null;
	for (let iface in interfaces)
		if (`${iface.index}` == text)
			return iface.mark;
	return null;
}

function unique_connmarks(plan) {
	let marks = [];
	let seen = {};
	function add(mark) {
		if (mark != null && !has_key(seen, `${mark}`)) {
			seen[`${mark}`] = true;
			push(marks, `${mark}`);
		}
	}
	for (let iface in plan.wan_balance.interfaces)
		add(iface.mark);
	for (let special in plan.wan_balance.special_marks)
		add(special.mark);
	for (let client in plan.static_clients)
		add(wan_mark_for_client(client.interfacename, plan.wan_balance.interfaces));
	return marks;
}

function connmark_match(values) {
	if (length(values) == 1)
		return `ct mark ${values[0]}`;
	return `ct mark { ${join(', ', values)} }`;
}

function has_prerouting_marks(plan) {
	if (plan.wan_balance.enabled &&
	    (length(plan.wan_balance.interfaces) > 0 || length(plan.wan_balance.special_marks) > 0))
		return true;

	for (let client in plan.static_clients)
		if (client.ipv4 != null && wan_mark_for_client(client.interfacename, plan.wan_balance.interfaces) != null)
			return true;

	return false;
}

let lease_maps = build_lease_maps(plan);
plan.lease_v4_upload = lease_maps.v4_up;
plan.lease_v4_download = lease_maps.v4_dl;
plan.lease_v6_upload = lease_maps.v6_up;
plan.lease_v6_download = lease_maps.v6_dl;

function render_dhcp_elements_only_text(plan) {
	if (!has_key(plan, 'lease_v4_upload') && type(plan.lease_clients) == 'array') {
		let m = build_lease_maps(plan);
		plan.lease_v4_upload = m.v4_up;
		plan.lease_v4_download = m.v4_dl;
		plan.lease_v6_upload = m.v6_up;
		plan.lease_v6_download = m.v6_dl;
	}
	let lines = [];
	let v4_up = render_map_elements(plan.lease_v4_upload);
	let v4_dl = render_map_elements(plan.lease_v4_download);
	push(lines, 'flush map inet eqos lease_v4_upload');
	if (length(v4_up))
		push(lines, `add element inet eqos lease_v4_upload { ${join(', ', v4_up)} }`);
	push(lines, 'flush map inet eqos lease_v4_download');
	if (length(v4_dl))
		push(lines, `add element inet eqos lease_v4_download { ${join(', ', v4_dl)} }`);
	if (plan.ipv6_enabled) {
		let v6_up = render_map_elements(plan.lease_v6_upload);
		let v6_dl = render_map_elements(plan.lease_v6_download);
		push(lines, 'flush map inet eqos lease_v6_upload');
		if (length(v6_up))
			push(lines, `add element inet eqos lease_v6_upload { ${join(', ', v6_up)} }`);
		push(lines, 'flush map bridge eqos lease_v6_download');
		if (length(v6_dl))
			push(lines, `add element bridge eqos lease_v6_download { ${join(', ', v6_dl)} }`);
	}
	return join('\n', lines) + '\n';
}

function render_candidate_nft(plan, existing_tables) {
	let lines = [ '# eqos-owner:v1' ];
	let owned_connmarks = unique_connmarks(plan);
	if (existing_tables.inet.exists && existing_tables.inet.owned)
		push(lines, 'delete table inet eqos');
	if (existing_tables.bridge.exists && existing_tables.bridge.owned)
		push(lines, 'delete table bridge eqos');

	let m = build_lease_maps(plan);
	let v4_up_elems = has_key(plan, 'lease_v4_upload') ? render_map_elements(plan.lease_v4_upload) : m.v4_up;
	let v4_dl_elems = has_key(plan, 'lease_v4_download') ? render_map_elements(plan.lease_v4_download) : m.v4_dl;
	let v6_up_elems = has_key(plan, 'lease_v6_upload') ? render_map_elements(plan.lease_v6_upload) : m.v6_up;
	let v6_dl_elems = has_key(plan, 'lease_v6_download') ? render_map_elements(plan.lease_v6_download) : m.v6_dl;

	push(lines, 'table inet eqos {');
	push(lines, '\tcomment "eqos-owner:v1";');

	for (let s = 1; s <= 31; s++)
		push(lines, `\tchain dscp_slot_${s} { ip dscp set ${s}; }`);
	for (let s = 33; s <= 63; s++)
		push(lines, `\tchain dscp_slot_${s} { ip dscp set ${s}; }`);

	push(lines, '\tmap lease_v4_upload {');
	push(lines, '\t\ttype ipv4_addr : verdict;');
	if (length(v4_up_elems))
		push(lines, `\t\telements = { ${join(', ', v4_up_elems)} }`);
	push(lines, '\t}');

	push(lines, '\tmap lease_v4_download {');
	push(lines, '\t\ttype ipv4_addr : verdict;');
	if (length(v4_dl_elems))
		push(lines, `\t\telements = { ${join(', ', v4_dl_elems)} }`);
	push(lines, '\t}');

	if (plan.ipv6_enabled) {
		push(lines, '\tmap lease_v6_upload {');
		push(lines, '\t\ttype ether_addr : mark;');
		if (length(v6_up_elems))
			push(lines, `\t\telements = { ${join(', ', v6_up_elems)} }`);
		push(lines, '\t}');
	}

	push(lines, '\tchain ipv4_upload {');
	for (let c in plan.static_clients) {
		if (c.ipv4) {
			let act = c.upload ? `ip dscp set ${c.slot} ` : '';
			push(lines, `\t\tip saddr ${c.ipv4} ${act}return`);
		}
	}
	push(lines, '\t\tip saddr vmap @lease_v4_upload');
	push(lines, '\t}');

	push(lines, '\tchain ipv4_download {');
	for (let c in plan.static_clients) {
		if (c.ipv4) {
			let act = c.download ? `ip dscp set ${c.slot + 32} ` : '';
			push(lines, `\t\tip daddr ${c.ipv4} ${act}return`);
		}
	}
	push(lines, '\t\tip daddr vmap @lease_v4_download');
	push(lines, '\t}');

	if (plan.ipv6_enabled) {
		push(lines, '\tchain ipv6_upload {');
		let seen_v6_static_up = {};
		for (let c in plan.static_clients) {
			if (c.mac && !has_key(seen_v6_static_up, c.mac)) {
				seen_v6_static_up[c.mac] = true;
				let act = c.upload ? `meta mark set ${c.slot} ` : '';
				push(lines, `\t\tmeta nfproto ipv6 ether saddr ${c.mac} ${act}return`);
			}
		}
		push(lines, '\t\tmeta nfproto ipv6 meta mark set ether saddr map @lease_v6_upload');
		push(lines, '\t}');
	}

	push(lines, '\tchain forward {');
	push(lines, '\t\ttype filter hook forward priority -149; policy accept;');
	if (type(plan.bypass_v4) == 'array') {
		for (let i = 0; i < length(plan.bypass_v4); i++) {
			let entry = plan.bypass_v4[i];
			if (type(entry) == 'string') {
				push(lines, `\t\tip saddr ${entry} meta mark set 0x99 return`);
				push(lines, `\t\tip daddr ${entry} meta mark set 0x99 return`);
			}
			else if (type(entry) == 'object' && entry.ipv4) {
				if (entry.upload) {
					push(lines, `\t\tip saddr ${entry.ipv4} meta mark set 0x99 return`);
				}
				if (entry.download) {
					push(lines, `\t\tip daddr ${entry.ipv4} meta mark set 0x99 return`);
				}
			}
		}
	}
	push(lines, '\t\tcomment "eqos-owner:v1";');
	push(lines, '\t\tiifname "br-lan" ip dscp set 0');
	push(lines, '\t\toifname "br-lan" ip dscp set 0');
	if (plan.ipv6_enabled) {
		push(lines, '\t\tiifname "br-lan" ip6 dscp set 0');
		push(lines, '\t\toifname "br-lan" ip6 dscp set 0');
	}
	push(lines, '\t\tjump ipv4_upload');
	push(lines, '\t\tjump ipv4_download');
	if (plan.ipv6_enabled)
		push(lines, '\t\tmeta nfproto ipv6 jump ipv6_upload');
	push(lines, '\t}');

	if (has_prerouting_marks(plan)) {
		let n_wan = length(plan.wan_balance.interfaces);
		push(lines, '\tchain prerouting {');
		push(lines, '\t\ttype filter hook prerouting priority mangle; policy accept;');
		for (let c in plan.static_clients) {
			let client_mark = wan_mark_for_client(c.interfacename, plan.wan_balance.interfaces);
			if (c.ipv4 && client_mark != null)
				push(lines, `\t\tiifname "br-lan" meta nfproto ipv4 ip saddr ${c.ipv4} ct state new ct mark 0 ct mark set ${client_mark} meta mark set ${client_mark}`);
		}
		if (plan.wan_balance.enabled) {
			for (let sm in plan.wan_balance.special_marks)
				push(lines, `\t\tiifname "br-lan" meta nfproto ipv4 ct state new ct mark 0 ${sm.protocol} dport ${sm.destination_port} ct mark set ${sm.mark} meta mark set ${sm.mark}`);
			for (let w in plan.wan_balance.interfaces) {
				if (w.subnet)
					push(lines, `\t\tiifname "br-lan" meta nfproto ipv4 ct state new ct mark 0 ip daddr ${w.subnet} ct mark set ${w.mark} meta mark set ${w.mark}`);
			}
			if (n_wan > 0) {
				let map_elems = [];
				for (let bucket = 0; bucket < n_wan; bucket++)
					push(map_elems, `${bucket} : ${plan.wan_balance.interfaces[bucket].mark}`);
				let generic_wan_rule = `iifname "br-lan" meta nfproto ipv4 ct state new ct mark 0 ct mark set numgen inc mod ${n_wan} map { ${join(', ', map_elems)} } meta mark set ct mark`;
				push(lines, `\t\t${generic_wan_rule}`);
			}
		}
		if (length(owned_connmarks))
			push(lines, `\t\tiifname "br-lan" meta nfproto ipv4 ${connmark_match(owned_connmarks)} meta mark set ct mark`);
		push(lines, '\t}');
	}

	push(lines, '}');

	if (plan.ipv6_enabled) {
		push(lines, 'table bridge eqos {');
		push(lines, '\tcomment "eqos-owner:v1";');

		push(lines, '\tmap lease_v6_download {');
		push(lines, '\t\ttype ether_addr : mark;');
		if (length(v6_dl_elems))
			push(lines, `\t\telements = { ${join(', ', v6_dl_elems)} }`);
		push(lines, '\t}');

		push(lines, '\tchain ipv6_download {');
		let seen_v6_static_dl = {};
		for (let c in plan.static_clients) {
			if (c.mac && !has_key(seen_v6_static_dl, c.mac)) {
				seen_v6_static_dl[c.mac] = true;
				let act = c.download ? `meta mark set ${c.slot + 32} ` : '';
				push(lines, `\t\tether type ip6 ether daddr ${c.mac} ${act}return`);
			}
		}
		push(lines, '\t\tether type ip6 meta mark set ether daddr map @lease_v6_download');
		push(lines, '\t}');

		push(lines, '\tchain postrouting {');
		push(lines, '\t\ttype filter hook postrouting priority 300; policy accept;');
		push(lines, '\t\tcomment "eqos-owner:v1";');
		push(lines, '\t\tether type ip6 jump ipv6_download');
		push(lines, '\t}');

		push(lines, '}');
	}

	return join('\n', lines) + '\n';
}

function render_ruleset_text(plan, existing_tables) {
	return render_candidate_nft(plan, existing_tables || table_state);
}

let desired_wans = (plan.wan_balance && plan.wan_balance.enabled && type(plan.wan_balance.interfaces) == 'array')
	? plan.wan_balance.interfaces
	: [];

recover_prior_transaction();

if (trigger == 'start' || trigger == 'reload' || trigger == 'fw4' || trigger == 'install' || trigger == 'iface') {
	let precheck_res = run_cmd(`POSIXLY_CORRECT=1 bash ${sh_quote(LOADBALANCE_BIN)} --stage=precheck ${sh_quote(sprintf('%J', desired_wans))}`);
	if (precheck_res.rc != 0) {
		let err_doc = null;
		try { err_doc = json(precheck_res.stdout); } catch (e) {}
		write_status('error', {
			code: (err_doc && err_doc.code) ? err_doc.code : 'route_conflict',
			detail: (err_doc && err_doc.detail) ? err_doc.detail : precheck_res.stdout
		});
		exit(1);
	}
}

let candidate_path = `${STATE_DIR}/candidate.nft`;
let nft_script = (trigger == 'dhcp' && table_state.inet.exists && (!plan.ipv6_enabled || table_state.bridge.exists))
	? render_dhcp_elements_only_text(plan)
	: render_ruleset_text(plan, table_state);
if (!write_file_atomic(candidate_path, nft_script)) {
	write_status('error', {
		code: 'state_write_failed',
		detail: 'failed to write candidate.nft'
	});
	exit(1);
}

let check_res = run_cmd(`${sh_quote(NFT_BIN)} -c -f ${sh_quote(candidate_path)} 2>&1`);
if (check_res.rc != 0) {
	write_status('error', {
		code: 'nft_check_failed',
		detail: check_res.stdout
	});
	exit(1);
}

let journal_path = `${STATE_DIR}/journal.json`;
let journal_doc = {
	schema: 'eqos-reconcile-journal/v1',
	trigger,
	generation: plan_doc.next_registry.generation,
	stage: 'applying',
	completed_stages: [],
	partial_completion: false,
	desired_wans,
	route_ops: []
};
if (!write_json_atomic(journal_path, journal_doc)) {
	write_status('error', {
		code: 'state_write_failed',
		detail: 'failed to write journal.json'
	});
	exit(1);
}

write_status('applying', {
	generation: plan_doc.next_registry.generation,
	journal: journal_doc
});

function fail_with_journal(code, detail, failed_stage, partial_completion) {
	journal_doc.status = 'failed';
	journal_doc.stage = 'failed';
	journal_doc.failed_stage = failed_stage;
	journal_doc.partial_completion = (partial_completion == true);
	journal_doc.error_code = code;
	journal_doc.detail = detail;
	write_json_atomic(journal_path, journal_doc);

	write_status('error', {
		code,
		detail,
		failed_stage,
		partial_completion: (partial_completion == true),
		generation: plan_doc.next_registry.generation
	});
	exit(1);
}

if (trigger == 'start' || trigger == 'reload' || trigger == 'fw4' || trigger == 'install' || trigger == 'iface') {
	let lb_add_res = run_cmd(`POSIXLY_CORRECT=1 bash ${sh_quote(LOADBALANCE_BIN)} --stage=add ${sh_quote(sprintf('%J', desired_wans))}`);
	let j_reloaded = null;
	try { j_reloaded = json(fs.readfile(journal_path)); } catch (e) {}
	if (type(j_reloaded) == 'object' && type(j_reloaded.route_ops) == 'array') {
		journal_doc.route_ops = j_reloaded.route_ops;
	}
	if (lb_add_res.rc != 0) {
		let err_doc = null;
		try { err_doc = json(lb_add_res.stdout); } catch (e) {}
		fail_with_journal(
			(err_doc && err_doc.code) ? err_doc.code : 'runtime_helper_failed',
			(err_doc && err_doc.detail) ? err_doc.detail : lb_add_res.stdout,
			'routes_add',
			true
		);
	}
	push(journal_doc.completed_stages, 'routes_add');
	if (!write_json_atomic(journal_path, journal_doc)) {
		fail_with_journal('state_write_failed', 'failed to update journal.json after routes_add', 'routes_add', true);
	}
}

if (trigger == 'start' || trigger == 'reload' || trigger == 'fw4' || trigger == 'install') {
	let eqos_res = run_cmd(`${sh_quote(SBIN_EQOS)} start ${sh_quote(cfg.download_mbit)} ${sh_quote(cfg.upload_mbit)} ${sh_quote(cfg.comment)}`);
	if (eqos_res.rc != 0) {
		fail_with_journal('runtime_helper_failed', eqos_res.stdout, 'tc_hnat_start', true);
	}
	if (cfg.smartqos) {
		if (!mkdir_p(HNAT_DIR)) {
			fail_with_journal('runtime_helper_failed', `failed to create ${HNAT_DIR}`, 'smartqos_debugfs', true);
		}
		if (cfg.upload_mbit > 0) {
			for (let i = 1; i <= 31; i++) {
				if (!write_debugfs_node(`${HNAT_DIR}/qdma_txq${i}`, `2 1 ${cfg.upload_mbit * 225} 1 ${cfg.upload_mbit * 1000} 4 1\n`)) {
					fail_with_journal('runtime_helper_failed', `failed to write ${HNAT_DIR}/qdma_txq${i}`, 'smartqos_debugfs', true);
				}
			}
		}
		if (cfg.download_mbit > 0) {
			for (let i = 33; i <= 63; i++) {
				if (!write_debugfs_node(`${HNAT_DIR}/qdma_txq${i}`, `3 1 ${cfg.download_mbit * 200} 1 ${cfg.download_mbit * 1000} 4 1\n`)) {
					fail_with_journal('runtime_helper_failed', `failed to write ${HNAT_DIR}/qdma_txq${i}`, 'smartqos_debugfs', true);
				}
			}
		}
	}
	let programmed_slots = {};
	for (let c in plan.static_clients) {
		let slot_key = `${c.slot}`;
		if (has_key(programmed_slots, slot_key))
			continue;
		programmed_slots[slot_key] = true;
		let ip_arg = c.ipv4 || '';
		let mac_arg = c.mac || '';
		let iface_arg = (c.interfacename != null) ? `${c.interfacename}` : '';
		let comment_arg = '';
		if (ip_arg != '' && has_key(cfg.comments_by_key, `ipv4:${ip_arg}`))
			comment_arg = cfg.comments_by_key[`ipv4:${ip_arg}`];
		else if (mac_arg != '' && has_key(cfg.comments_by_key, `mac:${mac_arg}`))
			comment_arg = cfg.comments_by_key[`mac:${mac_arg}`];
		let add_res = run_cmd(`${sh_quote(SBIN_EQOS)} add ${sh_quote(ip_arg)} ${sh_quote(c.download_kbit)} ${sh_quote(c.upload_kbit)} ${sh_quote(comment_arg)} ${sh_quote(mac_arg)} ${sh_quote(iface_arg)} ${sh_quote(c.slot)}`);
		if (add_res.rc != 0) {
			fail_with_journal('runtime_helper_failed', add_res.stdout, 'static_client_qos', true);
		}
	}
	push(journal_doc.completed_stages, 'tc_hnat');
	if (!write_json_atomic(journal_path, journal_doc)) {
		fail_with_journal('state_write_failed', 'failed to update journal.json after tc_hnat', 'tc_hnat', true);
	}
}

let apply_res = run_cmd(`${sh_quote(NFT_BIN)} -f ${sh_quote(candidate_path)} 2>&1`);
if (apply_res.rc != 0) {
	fail_with_journal('nft_apply_failed', apply_res.stdout, 'nft_apply', true);
}
push(journal_doc.completed_stages, 'nft_apply');
if (!write_json_atomic(journal_path, journal_doc)) {
	fail_with_journal('state_write_failed', 'failed to update journal.json after nft_apply', 'nft_apply', true);
}

if (trigger == 'start' || trigger == 'reload' || trigger == 'fw4' || trigger == 'install' || trigger == 'iface') {
	let lb_prune_res = run_cmd(`POSIXLY_CORRECT=1 bash ${sh_quote(LOADBALANCE_BIN)} --stage=prune ${sh_quote(sprintf('%J', desired_wans))}`);
	if (lb_prune_res.rc != 0) {
		let err_doc = null;
		try { err_doc = json(lb_prune_res.stdout); } catch (e) {}
		fail_with_journal(
			(err_doc && err_doc.code) ? err_doc.code : 'runtime_helper_failed',
			(err_doc && err_doc.detail) ? err_doc.detail : lb_prune_res.stdout,
			'routes_prune',
			true
		);
	}
	push(journal_doc.completed_stages, 'routes_prune');
	if (!write_json_atomic(journal_path, journal_doc)) {
		fail_with_journal('state_write_failed', 'failed to update journal.json after routes_prune', 'routes_prune', true);
	}
}

if (!write_json_atomic(reg_path, plan_doc.next_registry)) {
	fail_with_journal('state_write_failed', 'failed to commit slot-registry.json', 'commit_registry', true);
}

if (cfg.smartqos && lease_status == 'COMPLETE') {
	if (!write_json_atomic(`${STATE_DIR}/last-good-leases.json`, active_leases)) {
		fail_with_journal('state_write_failed', 'failed to write last-good-leases.json', 'commit_last_good_leases', true);
	}
}

journal_doc.stage = 'committed';
journal_doc.status = 'committed';
push(journal_doc.completed_stages, 'committed');
if (!write_json_atomic(journal_path, journal_doc)) {
	fail_with_journal('state_write_failed', 'failed to update journal.json to committed', 'commit_journal', true);
}

let final_status = (cfg.smartqos && lease_status == 'PENDING') ? 'pending_leases' : 'ready';
if (!write_status(final_status, {
	generation: plan_doc.next_registry.generation,
	registry_changed: plan_doc.registry_changed,
	lease_status,
	lease_reason,
	used_last_good_leases
})) {
	exit(1);
}

if (fs.stat(`${STATE_DIR}/pending_fw4`))
	fs.unlink(`${STATE_DIR}/pending_fw4`);

if (fs.stat(journal_path))
	fs.unlink(journal_path);

exit(0);
