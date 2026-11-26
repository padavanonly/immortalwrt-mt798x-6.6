#!/usr/bin/ucode
// SPDX-License-Identifier: GPL-2.0-only
'use strict';

/*
 * EQoS native nftables renderer.
 *
 * Supports:
 * 1. Legacy candidate schema "eqos-nft/v1" (100% byte-for-byte compatible
 *    ordinary chains/maps in eqos_candidate without base hooks).
 * 2. Production plan schema "eqos-policy-plan/v1" (direct or wrapped in
 *    "eqos-nft-plan/v1" with mode: "install" | "leases" | "stop"), emitting
 *    owned "inet eqos" and "bridge eqos" tables, explicit eqos_owner_v1
 *    ownership chains, br-lan DSCP reset before classification in inet
 *    forward (priority mangle + 1), IPv6 gate (bridge postrouting priority 300
 *    and inet ipv6_upload only when ipv6_enabled is true), Multi-WAN
 *    wan_balance and per-device interfacename marking in inet prerouting and
 *    postrouting (priority mangle), dynamic lease map updates, and safe
 *    idempotent owned-table stop cleanup.
 */

import { open, error } from 'fs';

let IPV4_RE = /^(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$/;
let MAC_RE = /^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/;
let TOKEN_RE = /^[0-9a-zA-Z_.-]+$/;
let IFNAME_RE = /^[0-9a-zA-Z_.:-]+$/;
let MARK_RE = /^(0x[0-9a-fA-F]+|[0-9]+)$/;

let LEGACY_TABLE_INET = 'eqos_candidate';
let LEGACY_TABLE_BRIDGE = 'eqos_candidate';
let PROD_TABLE_INET = 'eqos';
let PROD_TABLE_BRIDGE = 'eqos';

function fail(message) {
	warn(`eqos-nft-render: ${message}\n`);
	exit(2);
}

function array_has(array, needle) {
	for (let value in array)
		if (value == needle)
			return true;

	return false;
}

function object_has(object, needle) {
	for (let key in object)
		if (key == needle)
			return true;

	return false;
}

function check_keys(object, allowed, required, path) {
	if (type(object) != 'object')
		fail(`${path}: expected object`);

	for (let key in object)
		if (!array_has(allowed, key))
			fail(`${path}: unsupported key '${key}'`);

	for (let key in required)
		if (!object_has(object, key))
			fail(`${path}: missing required key '${key}'`);
}

function require_array(value, path) {
	if (type(value) != 'array')
		fail(`${path}: expected array`);
}

function require_bool(value, path) {
	if (type(value) != 'bool')
		fail(`${path}: expected boolean`);
}

function require_int(value, path, minimum) {
	if (type(value) != 'int' || value < minimum)
		fail(`${path}: expected integer >= ${minimum}`);
}

function canonical_ipv4(value, path) {
	if (type(value) != 'string')
		fail(`${path}: expected canonical dotted-decimal IPv4 without leading zeros`);

	let matched = match(value, IPV4_RE);
	if (type(matched) != 'array' || !length(matched) || matched[0] != value)
		fail(`${path}: expected canonical dotted-decimal IPv4 without leading zeros`);

	return value;
}

function canonical_ipv4_or_cidr(value, path) {
	if (type(value) != 'string' || value == '')
		fail(`${path}: expected canonical IPv4 or CIDR string`);

	let parts = split(value, '/');
	if (length(parts) == 1)
		return canonical_ipv4(parts[0], path);

	if (length(parts) != 2)
		fail(`${path}: expected canonical IPv4 or CIDR string`);

	let ip = canonical_ipv4(parts[0], path);
	let prefix_match = match(parts[1], /^(0|[1-9]|[12][0-9]|3[0-2])$/);
	if (type(prefix_match) != 'array' || !length(prefix_match) || prefix_match[0] != parts[1])
		fail(`${path}: expected CIDR prefix length 0..32`);

	return `${ip}/${parts[1]}`;
}

function canonical_mac(value, path) {
	if (type(value) != 'string')
		fail(`${path}: expected six colon-separated hexadecimal octets`);

	let matched = match(value, MAC_RE);
	if (type(matched) != 'array' || !length(matched) || matched[0] != value)
		fail(`${path}: expected six colon-separated hexadecimal octets`);

	return lc(value);
}

function canonical_identity(value, path) {
	if (type(value) != 'string')
		fail(`${path}: expected identity string starting with mac: or ipv4:`);

	if (substr(value, 0, 4) == 'mac:')
		return `mac:${canonical_mac(substr(value, 4), path)}`;

	if (substr(value, 0, 5) == 'ipv4:')
		return `ipv4:${canonical_ipv4(substr(value, 5), path)}`;

	fail(`${path}: identity must start with mac: or ipv4:`);
}

function validate_token(value, regex, path, description) {
	if (type(value) != 'string' || value == '')
		fail(`${path}: expected ${description}`);

	let matched = match(value, regex);
	if (type(matched) != 'array' || !length(matched) || matched[0] != value)
		fail(`${path}: invalid ${description}`);

	return value;
}

function normalize_legacy_client(record, path) {
	check_keys(record,
		[ 'slot', 'mac', 'ipv4', 'upload', 'download' ],
		[ 'slot', 'upload', 'download' ], path);

	if (type(record.slot) != 'int' || record.slot < 1 || record.slot > 31)
		fail(`${path}.slot: expected integer 1..31 from the external allocator`);

	if (type(record.upload) != 'bool' || type(record.download) != 'bool')
		fail(`${path}: upload and download must be JSON booleans`);

	let mac = null;
	let ipv4 = null;

	if (object_has(record, 'mac'))
		mac = canonical_mac(record.mac, `${path}.mac`);

	if (object_has(record, 'ipv4'))
		ipv4 = canonical_ipv4(record.ipv4, `${path}.ipv4`);

	if (!mac && !ipv4)
		fail(`${path}: at least one of mac or ipv4 is required`);

	return {
		slot: record.slot,
		mac,
		ipv4,
		upload: record.upload,
		download: record.download
	};
}

function client_signature(client) {
	return `${client.slot}|${client.mac || ''}|${client.ipv4 || ''}|${client.upload ? 1 : 0}|${client.download ? 1 : 0}|${client.interfacename || ''}`;
}

function normalize_legacy_clients(value, path) {
	require_array(value, path);

	let out = [];
	let seen = {};

	for (let i = 0; i < length(value); i++) {
		let client = normalize_legacy_client(value[i], `${path}[${i}]`);
		let signature = client_signature(client);

		if (object_has(seen, signature))
			continue;

		seen[signature] = true;
		push(out, client);
	}

	return out;
}

function normalize_plan_static_client(record, path) {
	check_keys(record,
		[ 'identity', 'slot', 'mac', 'ipv4', 'upload', 'download', 'upload_kbit', 'download_kbit', 'interfacename' ],
		[ 'identity', 'slot', 'upload', 'download', 'upload_kbit', 'download_kbit' ], path);

	let identity = canonical_identity(record.identity, `${path}.identity`);

	if (type(record.slot) != 'int' || record.slot < 1 || record.slot > 31)
		fail(`${path}.slot: expected integer 1..31 from the external allocator`);

	if (type(record.upload) != 'bool' || type(record.download) != 'bool')
		fail(`${path}: upload and download must be JSON booleans`);

	require_int(record.upload_kbit, `${path}.upload_kbit`, 0);
	require_int(record.download_kbit, `${path}.download_kbit`, 0);

	let mac = null;
	let ipv4 = null;

	if (object_has(record, 'mac') && record.mac != null)
		mac = canonical_mac(record.mac, `${path}.mac`);

	if (object_has(record, 'ipv4') && record.ipv4 != null)
		ipv4 = canonical_ipv4(record.ipv4, `${path}.ipv4`);

	if (!mac && !ipv4)
		fail(`${path}: at least one of mac or ipv4 is required`);

	if (substr(identity, 0, 4) == 'mac:' && mac && substr(identity, 4) != mac)
		fail(`${path}.identity: MAC identity '${identity}' conflicts with mac '${mac}'`);

	let interfacename = null;
	if (object_has(record, 'interfacename') && record.interfacename != null)
		interfacename = validate_token(record.interfacename, TOKEN_RE, `${path}.interfacename`, 'non-empty interface token');

	return {
		identity,
		slot: record.slot,
		mac,
		ipv4,
		upload: record.upload,
		download: record.download,
		upload_kbit: record.upload_kbit,
		download_kbit: record.download_kbit,
		interfacename
	};
}

function normalize_plan_lease_client(record, path) {
	check_keys(record,
		[ 'identity', 'slot', 'mac', 'ipv4', 'upload', 'download' ],
		[ 'identity', 'slot', 'upload', 'download' ], path);

	let identity = canonical_identity(record.identity, `${path}.identity`);

	if (type(record.slot) != 'int' || record.slot < 1 || record.slot > 31)
		fail(`${path}.slot: expected integer 1..31 from the external allocator`);

	if (type(record.upload) != 'bool' || type(record.download) != 'bool')
		fail(`${path}: upload and download must be JSON booleans`);

	let mac = null;
	let ipv4 = null;

	if (object_has(record, 'mac') && record.mac != null)
		mac = canonical_mac(record.mac, `${path}.mac`);

	if (object_has(record, 'ipv4') && record.ipv4 != null)
		ipv4 = canonical_ipv4(record.ipv4, `${path}.ipv4`);

	if (!mac && !ipv4)
		fail(`${path}: at least one of mac or ipv4 is required`);

	if (substr(identity, 0, 4) == 'mac:' && mac && substr(identity, 4) != mac)
		fail(`${path}.identity: MAC identity '${identity}' conflicts with mac '${mac}'`);

	return {
		identity,
		slot: record.slot,
		mac,
		ipv4,
		upload: record.upload,
		download: record.download
	};
}

function normalize_plan_clients(value, path, is_static) {
	require_array(value, path);

	let out = [];
	let seen = {};

	for (let i = 0; i < length(value); i++) {
		let client = is_static
			? normalize_plan_static_client(value[i], `${path}[${i}]`)
			: normalize_plan_lease_client(value[i], `${path}[${i}]`);
		let signature = client_signature(client);

		if (object_has(seen, signature))
			continue;

		seen[signature] = true;
		push(out, client);
	}

	return out;
}

function normalize_bypass(record, path) {
	check_keys(record,
		[ 'ipv4', 'upload', 'download' ],
		[ 'ipv4', 'upload', 'download' ], path);

	if (type(record.upload) != 'bool' || type(record.download) != 'bool')
		fail(`${path}: upload and download must be JSON booleans`);

	return {
		ipv4: canonical_ipv4(record.ipv4, `${path}.ipv4`),
		upload: record.upload,
		download: record.download
	};
}

function normalize_bypasses(value, path) {
	let field_path = path || 'bypass_v4';
	require_array(value, field_path);

	let out = [];
	let seen = {};

	for (let i = 0; i < length(value); i++) {
		let bypass = normalize_bypass(value[i], `${field_path}[${i}]`);
		let signature = `${bypass.ipv4}|${bypass.upload ? 1 : 0}|${bypass.download ? 1 : 0}`;

		if (object_has(seen, signature))
			continue;

		seen[signature] = true;
		push(out, bypass);
	}

	return out;
}

function bind_unique(index, key, value, label) {
	if (object_has(index, key) && index[key] != value)
		fail(`${label}: conflicting values for '${key}'`);

	index[key] = value;
}

function flags_signature(client) {
	return `${client.upload ? 1 : 0}:${client.download ? 1 : 0}`;
}

function validate_source_flags(clients, source) {
	let mac_flags = {};
	let ipv4_flags = {};
	let mac_iface = {};
	let ipv4_iface = {};

	for (let client in clients) {
		let flags = flags_signature(client);

		if (client.mac)
			bind_unique(mac_flags, client.mac, flags, `${source} MAC directions`);

		if (client.ipv4)
			bind_unique(ipv4_flags, client.ipv4, flags, `${source} IPv4 directions`);

		if (object_has(client, 'interfacename')) {
			let iface = client.interfacename || '';
			if (client.mac)
				bind_unique(mac_iface, client.mac, iface, `${source} MAC interfacename`);
			if (client.ipv4)
				bind_unique(ipv4_iface, client.ipv4, iface, `${source} IPv4 interfacename`);
		}
	}
}

function validate_identities(clients) {
	/* A macless IPv4 joins a slot only through an explicit same-slot MAC+IPv4 record. */
	let mac_slot = {};
	let ipv4_slot = {};
	let slot_mac = {};
	let slot_mac_ipv4 = {};
	let slot_macless_ipv4 = {};

	for (let client in clients) {
		let slot_key = `${client.slot}`;

		if (client.mac) {
			bind_unique(mac_slot, client.mac, slot_key, 'MAC to slot');
			bind_unique(slot_mac, slot_key, client.mac, 'slot identity MAC');

			if (client.ipv4) {
				if (!object_has(slot_mac_ipv4, slot_key))
					slot_mac_ipv4[slot_key] = {};

				slot_mac_ipv4[slot_key][client.ipv4] = true;
			}
		}
		else {
			if (!object_has(slot_macless_ipv4, slot_key))
				slot_macless_ipv4[slot_key] = {};

			slot_macless_ipv4[slot_key][client.ipv4] = true;
		}

		if (client.ipv4)
			bind_unique(ipv4_slot, client.ipv4, slot_key, 'IPv4 to slot');
	}

	for (let slot_key in slot_macless_ipv4) {
		let macless_ipv4 = slot_macless_ipv4[slot_key];

		if (object_has(slot_mac, slot_key)) {
			let mac_ipv4 = slot_mac_ipv4[slot_key] || {};

			for (let ipv4 in macless_ipv4)
				if (!object_has(mac_ipv4, ipv4))
					fail(`slot identity: disconnected IPv4 '${ipv4}' for MAC '${slot_mac[slot_key]}' in slot '${slot_key}'`);
		}
		else {
			let only_ipv4 = null;

			for (let ipv4 in macless_ipv4) {
				if (only_ipv4 != null && only_ipv4 != ipv4)
					fail(`slot identity: multiple IPv4 addresses without a MAC in slot '${slot_key}'`);

				only_ipv4 = ipv4;
			}
		}
	}

	return ipv4_slot;
}

function validate_bypasses_against_slots(bypasses, ipv4_slot) {
	let seen_bypass = {};

	for (let bypass in bypasses) {
		if (object_has(seen_bypass, bypass.ipv4))
			fail(`bypass_v4: conflicting entries for IPv4 '${bypass.ipv4}'`);

		if (object_has(ipv4_slot, bypass.ipv4))
			fail(`bypass_v4: IPv4 '${bypass.ipv4}' also has a client slot`);

		seen_bypass[bypass.ipv4] = true;
	}
}

function normalize_legacy_document(document) {
	check_keys(document,
		[ 'schema', 'mode', 'static_clients', 'lease_clients', 'bypass_v4' ],
		[ 'schema', 'mode', 'static_clients', 'lease_clients', 'bypass_v4' ], 'input');

	if (type(document.schema) != 'string' || document.schema != 'eqos-nft/v1')
		fail('input.schema: expected eqos-nft/v1');

	if (type(document.mode) != 'string' ||
	    (document.mode != 'install' && document.mode != 'leases'))
		fail('input.mode: expected install or leases');

	let normalized = {
		schema: 'eqos-nft/v1',
		mode: document.mode,
		static_clients: normalize_legacy_clients(document.static_clients, 'static_clients'),
		lease_clients: normalize_legacy_clients(document.lease_clients, 'lease_clients'),
		bypass_v4: normalize_bypasses(document.bypass_v4, 'bypass_v4')
	};

	validate_source_flags(normalized.static_clients, 'static');
	validate_source_flags(normalized.lease_clients, 'lease');

	let all_clients = [ ...normalized.static_clients, ...normalized.lease_clients ];
	let ipv4_slot = validate_identities(all_clients);
	validate_bypasses_against_slots(normalized.bypass_v4, ipv4_slot);

	return normalized;
}

function normalize_shaper(shaper, path) {
	check_keys(shaper,
		[ 'download_mbit', 'upload_mbit', 'smartqos' ],
		[ 'download_mbit', 'upload_mbit', 'smartqos' ], path);
	require_int(shaper.download_mbit, `${path}.download_mbit`, 0);
	require_int(shaper.upload_mbit, `${path}.upload_mbit`, 0);
	require_bool(shaper.smartqos, `${path}.smartqos`);

	return {
		download_mbit: shaper.download_mbit,
		upload_mbit: shaper.upload_mbit,
		smartqos: shaper.smartqos
	};
}

function normalize_wan_balance(wan, path) {
	check_keys(wan,
		[ 'enabled', 'interfaces', 'special_marks' ],
		[ 'enabled', 'interfaces', 'special_marks' ], path);
	require_bool(wan.enabled, `${path}.enabled`);
	require_array(wan.interfaces, `${path}.interfaces`);
	require_array(wan.special_marks, `${path}.special_marks`);

	let interfaces = [];
	let seen_indices = {};
	for (let i = 0; i < length(wan.interfaces); i++) {
		let source = wan.interfaces[i];
		let ipath = `${path}.interfaces[${i}]`;
		check_keys(source,
			[ 'index', 'ifname', 'gateway', 'subnet', 'mark', 'table' ],
			[ 'index', 'ifname', 'gateway', 'subnet', 'mark', 'table' ], ipath);
		require_int(source.index, `${ipath}.index`, 0);
		let ifname = validate_token(source.ifname, IFNAME_RE, `${ipath}.ifname`, 'interface name');
		let gateway = source.gateway == '' ? '' : canonical_ipv4(source.gateway, `${ipath}.gateway`);
		let subnet = source.subnet == '' ? '' : canonical_ipv4_or_cidr(source.subnet, `${ipath}.subnet`);
		let mark = validate_token(source.mark, MARK_RE, `${ipath}.mark`, 'mark string');
		let table = validate_token(source.table, TOKEN_RE, `${ipath}.table`, 'table string');

		let index_key = `${source.index}`;
		if (object_has(seen_indices, index_key))
			fail(`${ipath}: duplicate WAN index ${source.index}`);
		seen_indices[index_key] = true;

		push(interfaces, {
			index: source.index,
			ifname,
			gateway,
			subnet,
			mark,
			table
		});
	}

	let active_wan_marks = {};
	for (let iface in interfaces)
		active_wan_marks[iface.mark] = true;

	let special_marks = [];
	for (let i = 0; i < length(wan.special_marks); i++) {
		let source = wan.special_marks[i];
		let spath = `${path}.special_marks[${i}]`;
		check_keys(source,
			[ 'protocol', 'destination_port', 'mark' ],
			[ 'protocol', 'destination_port', 'mark' ], spath);
		if (source.protocol != 'tcp' && source.protocol != 'udp')
			fail(`${spath}.protocol: expected tcp or udp`);
		require_int(source.destination_port, `${spath}.destination_port`, 1);
		if (source.destination_port > 65535)
			fail(`${spath}.destination_port: exceeds 65535`);
		let mark = validate_token(source.mark, MARK_RE, `${spath}.mark`, 'mark string');
		if (!object_has(active_wan_marks, mark))
			fail(`${spath}.mark: must identify an active WAN mark`);

		push(special_marks, {
			protocol: source.protocol,
			destination_port: source.destination_port,
			mark
		});
	}

	return {
		enabled: wan.enabled,
		interfaces,
		special_marks
	};
}

function normalize_plan_object(plan, path, allow_mode_key) {
	let allowed = [
		'schema', 'generation', 'enabled', 'ipv6_enabled',
		'shaper', 'wan_balance', 'static_clients', 'lease_clients', 'bypass_v4'
	];
	let required = [
		'schema', 'generation', 'enabled', 'ipv6_enabled',
		'shaper', 'wan_balance', 'static_clients', 'lease_clients', 'bypass_v4'
	];
	if (allow_mode_key)
		push(allowed, 'mode');

	check_keys(plan, allowed, required, path);

	if (type(plan.schema) != 'string' || plan.schema != 'eqos-policy-plan/v1')
		fail(`${path}.schema: expected eqos-policy-plan/v1`);

	require_int(plan.generation, `${path}.generation`, 0);
	require_bool(plan.enabled, `${path}.enabled`);
	require_bool(plan.ipv6_enabled, `${path}.ipv6_enabled`);

	let shaper = normalize_shaper(plan.shaper, `${path}.shaper`);
	let wan_balance = normalize_wan_balance(plan.wan_balance, `${path}.wan_balance`);
	let static_clients = normalize_plan_clients(plan.static_clients, `${path}.static_clients`, true);
	let lease_clients = normalize_plan_clients(plan.lease_clients, `${path}.lease_clients`, false);
	let bypass_v4 = normalize_bypasses(plan.bypass_v4, `${path}.bypass_v4`);

	validate_source_flags(static_clients, 'static');
	validate_source_flags(lease_clients, 'lease');

	let all_clients = [ ...static_clients, ...lease_clients ];
	let ipv4_slot = validate_identities(all_clients);
	validate_bypasses_against_slots(bypass_v4, ipv4_slot);

	return {
		schema: 'eqos-policy-plan/v1',
		generation: plan.generation,
		enabled: plan.enabled,
		ipv6_enabled: plan.ipv6_enabled,
		shaper,
		wan_balance,
		static_clients,
		lease_clients,
		bypass_v4
	};
}

function validate_prod_mode(mode, path) {
	if (type(mode) != 'string' ||
	    (mode != 'install' && mode != 'leases' && mode != 'stop'))
		fail(`${path}: expected install, leases, or stop`);
	return mode;
}

function normalize_document(document) {
	if (type(document) != 'object')
		fail('input: expected a JSON object');

	if (!object_has(document, 'schema'))
		fail("input: missing required key 'schema'");

	if (document.schema == 'eqos-nft/v1')
		return normalize_legacy_document(document);

	if (document.schema == 'eqos-nft-plan/v1') {
		check_keys(document, [ 'schema', 'mode', 'plan' ], [ 'schema', 'mode', 'plan' ], 'input');
		let mode = validate_prod_mode(document.mode, 'input.mode');
		let plan = normalize_plan_object(document.plan, 'input.plan', false);
		if (mode == 'leases' && !plan.enabled)
			fail('input.mode: leases mode is invalid when plan.enabled is false');
		return {
			schema: 'eqos-policy-plan/v1',
			mode,
			plan
		};
	}

	if (document.schema == 'eqos-policy-plan/v1') {
		let mode = 'install';
		if (object_has(document, 'mode'))
			mode = validate_prod_mode(document.mode, 'input.mode');
		let plan = normalize_plan_object(document, 'input', true);
		if (mode == 'leases' && !plan.enabled)
			fail('input.mode: leases mode is invalid when plan.enabled is false');
		return {
			schema: 'eqos-policy-plan/v1',
			mode,
			plan
		};
	}

	fail('input.schema: expected eqos-nft/v1, eqos-policy-plan/v1, or eqos-nft-plan/v1');
}

function new_map_data() {
	return {
		v4_upload: { elements: [], seen: {} },
		v4_download: { elements: [], seen: {} },
		v6_upload: { elements: [], seen: {} },
		v6_download: { elements: [], seen: {} }
	};
}

function add_map_element(map, key, value, label) {
	let value_string = `${value}`;

	if (object_has(map.seen, key)) {
		if (map.seen[key] != value_string)
			fail(`${label}: conflicting map values for '${key}'`);

		return;
	}

	map.seen[key] = value_string;
	push(map.elements, `${key} : ${value_string}`);
}

function lease_map_data(clients) {
	let maps = new_map_data();

	for (let client in clients) {
		let pair = client.slot + 32;

		if (client.ipv4 && client.upload)
			add_map_element(maps.v4_upload, client.ipv4, `jump dscp_slot_${client.slot}`, 'lease IPv4 upload');

		if (client.ipv4 && client.download)
			add_map_element(maps.v4_download, client.ipv4, `jump dscp_slot_${pair}`, 'lease IPv4 download');

		if (client.mac && client.upload)
			add_map_element(maps.v6_upload, client.mac, client.slot, 'lease IPv6 upload');

		if (client.mac && client.download)
			add_map_element(maps.v6_download, client.mac, pair, 'lease IPv6 download');
	}

	return maps;
}

function map_specs(table_inet, table_bridge, include_ipv6) {
	let specs = [
		{ family: 'inet', table: table_inet, name: 'lease_v4_upload', key_type: 'ipv4_addr', value_type: 'verdict', data: 'v4_upload' },
		{ family: 'inet', table: table_inet, name: 'lease_v4_download', key_type: 'ipv4_addr', value_type: 'verdict', data: 'v4_download' }
	];

	if (include_ipv6) {
		push(specs, { family: 'inet', table: table_inet, name: 'lease_v6_upload', key_type: 'ether_addr', value_type: 'mark', data: 'v6_upload' });
		push(specs, { family: 'bridge', table: table_bridge, name: 'lease_v6_download', key_type: 'ether_addr', value_type: 'mark', data: 'v6_download' });
	}

	return specs;
}

function append_dscp_slot_chains(out, table_inet) {
	for (let s = 1; s <= 31; s++) {
		push(out, `add chain inet ${table_inet} dscp_slot_${s}`);
		push(out, `add rule inet ${table_inet} dscp_slot_${s} ip dscp set ${s}`);
	}
	for (let s = 33; s <= 63; s++) {
		push(out, `add chain inet ${table_inet} dscp_slot_${s}`);
		push(out, `add rule inet ${table_inet} dscp_slot_${s} ip dscp set ${s}`);
	}
}

function append_map_objects(out, maps, mode, table_inet, table_bridge, include_ipv6) {
	for (let spec in map_specs(table_inet, table_bridge, include_ipv6)) {
		let data = maps[spec.data];

		if (mode == 'install')
			push(out, `add map ${spec.family} ${spec.table} ${spec.name} { type ${spec.key_type} : ${spec.value_type}; }`);
		else
			push(out, `flush map ${spec.family} ${spec.table} ${spec.name}`);

		if (length(data.elements))
			push(out, `add element ${spec.family} ${spec.table} ${spec.name} { ${join(', ', data.elements)} }`);
	}
}

function append_rule(out, seen, family, table, chain, rule) {
	let line = `add rule ${family} ${table} ${chain} ${rule}`;

	if (object_has(seen, line))
		return;

	seen[line] = true;
	push(out, line);
}

function render_legacy_install(input) {
	let out = [
		'# INACTIVE EQoS native renderer candidate; ordinary chains only, no hooks.',
		'# Caller must install/attach in reviewed order and reset br-lan DSCP before classification.',
		`add table inet ${LEGACY_TABLE_INET}`,
		`add table bridge ${LEGACY_TABLE_BRIDGE}`,
		`add chain inet ${LEGACY_TABLE_INET} ipv4_upload`,
		`add chain inet ${LEGACY_TABLE_INET} ipv4_download`,
		`add chain inet ${LEGACY_TABLE_INET} ipv6_upload`,
		`add chain bridge ${LEGACY_TABLE_BRIDGE} ipv6_download`
	];
	let seen_rules = {};

	append_dscp_slot_chains(out, LEGACY_TABLE_INET);
	append_map_objects(out, lease_map_data(input.lease_clients), 'install', LEGACY_TABLE_INET, LEGACY_TABLE_BRIDGE, true);

	for (let client in input.static_clients) {
		let pair = client.slot + 32;

		if (client.ipv4) {
			let upload_action = client.upload ? `ip dscp set ${client.slot} ` : '';
			let download_action = client.download ? `ip dscp set ${pair} ` : '';

			append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv4_upload',
				`ip saddr ${client.ipv4} ${upload_action}return`);
			append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv4_download',
				`ip daddr ${client.ipv4} ${download_action}return`);
		}

		if (client.mac) {
			let upload_action = client.upload ? `meta mark set ${client.slot} ` : '';
			let download_action = client.download ? `meta mark set ${pair} ` : '';

			append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv6_upload',
				`meta nfproto ipv6 ether saddr ${client.mac} ${upload_action}return`);
			append_rule(out, seen_rules, 'bridge', LEGACY_TABLE_BRIDGE, 'ipv6_download',
				`ether type ip6 ether daddr ${client.mac} ${download_action}return`);
		}
	}

	for (let bypass in input.bypass_v4) {
		if (bypass.upload)
			append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv4_upload',
				`ip saddr ${bypass.ipv4} meta mark set 0x99 return`);

		if (bypass.download)
			append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv4_download',
				`ip daddr ${bypass.ipv4} meta mark set 0x99 return`);
	}

	append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv4_upload',
		`ip saddr vmap @lease_v4_upload`);
	append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv4_download',
		`ip daddr vmap @lease_v4_download`);
	append_rule(out, seen_rules, 'inet', LEGACY_TABLE_INET, 'ipv6_upload',
		`meta nfproto ipv6 meta mark set ether saddr map @lease_v6_upload`);
	append_rule(out, seen_rules, 'bridge', LEGACY_TABLE_BRIDGE, 'ipv6_download',
		`ether type ip6 meta mark set ether daddr map @lease_v6_download`);

	return join('\n', out) + '\n';
}

function render_legacy_leases(input) {
	let out = [
		'# INACTIVE EQoS lease update candidate; updates only owned dynamic maps.',
		'# Does not clear static rules, chains, tables, or any ruleset.'
	];

	append_map_objects(out, lease_map_data(input.lease_clients), 'leases', LEGACY_TABLE_INET, LEGACY_TABLE_BRIDGE, true);
	return join('\n', out) + '\n';
}

function render_prod_stop(plan, mode) {
	let out = [
		`# eqos-owner:v1 mode=${mode} generation=${plan.generation} enabled=${plan.enabled ? 'true' : 'false'}`,
		`add table inet ${PROD_TABLE_INET}`,
		`delete table inet ${PROD_TABLE_INET}`,
		`add table bridge ${PROD_TABLE_BRIDGE}`,
		`delete table bridge ${PROD_TABLE_BRIDGE}`
	];

	return join('\n', out) + '\n';
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

function unique_marks(plan) {
	let marks = [];
	let seen = {};
	function add(mark) {
		if (mark != null && !object_has(seen, `${mark}`)) {
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

function mark_match(values) {
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

function render_prod_install(plan) {
	let ipv6 = plan.ipv6_enabled;
	let owned_marks = unique_marks(plan);
	let out = [
		`# eqos-owner:v1 mode=install generation=${plan.generation} ipv6=${ipv6 ? 'true' : 'false'}`,
		`add table inet ${PROD_TABLE_INET}`,
		`flush table inet ${PROD_TABLE_INET}`,
		`add chain inet ${PROD_TABLE_INET} eqos_owner_v1`
	];

	if (ipv6) {
		push(out, `add table bridge ${PROD_TABLE_BRIDGE}`);
		push(out, `flush table bridge ${PROD_TABLE_BRIDGE}`);
		push(out, `add chain bridge ${PROD_TABLE_BRIDGE} eqos_owner_v1`);
	}
	else {
		push(out, `add table bridge ${PROD_TABLE_BRIDGE}`);
		push(out, `delete table bridge ${PROD_TABLE_BRIDGE}`);
	}

	push(out, `add chain inet ${PROD_TABLE_INET} ipv4_upload`);
	push(out, `add chain inet ${PROD_TABLE_INET} ipv4_download`);
	if (ipv6) {
		push(out, `add chain inet ${PROD_TABLE_INET} ipv6_upload`);
		push(out, `add chain bridge ${PROD_TABLE_BRIDGE} ipv6_download`);
	}

	let seen_rules = {};

	append_dscp_slot_chains(out, PROD_TABLE_INET);
	append_map_objects(out, lease_map_data(plan.lease_clients), 'install', PROD_TABLE_INET, PROD_TABLE_BRIDGE, ipv6);

	for (let client in plan.static_clients) {
		let pair = client.slot + 32;

		if (client.ipv4) {
			let upload_action = client.upload ? `ip dscp set ${client.slot} ` : '';
			let download_action = client.download ? `ip dscp set ${pair} ` : '';

			append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv4_upload',
				`ip saddr ${client.ipv4} ${upload_action}return`);
			append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv4_download',
				`ip daddr ${client.ipv4} ${download_action}return`);
		}

		if (ipv6 && client.mac) {
			let upload_action = client.upload ? `meta mark set ${client.slot} ` : '';
			let download_action = client.download ? `meta mark set ${pair} ` : '';

			append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv6_upload',
				`meta nfproto ipv6 ether saddr ${client.mac} ${upload_action}return`);
			append_rule(out, seen_rules, 'bridge', PROD_TABLE_BRIDGE, 'ipv6_download',
				`ether type ip6 ether daddr ${client.mac} ${download_action}return`);
		}
	}

	for (let bypass in plan.bypass_v4) {
		if (bypass.upload)
			append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv4_upload',
				`ip saddr ${bypass.ipv4} meta mark set 0x99 return`);

		if (bypass.download)
			append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv4_download',
				`ip daddr ${bypass.ipv4} meta mark set 0x99 return`);
	}

	append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv4_upload',
		`ip saddr vmap @lease_v4_upload`);
	append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv4_download',
		`ip daddr vmap @lease_v4_download`);
	if (ipv6) {
		append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'ipv6_upload',
			`meta nfproto ipv6 meta mark set ether saddr map @lease_v6_upload`);
		append_rule(out, seen_rules, 'bridge', PROD_TABLE_BRIDGE, 'ipv6_download',
			`ether type ip6 meta mark set ether daddr map @lease_v6_download`);
	}

	/* Single owned forward chain at mangle + 1: DSCP reset strictly precedes classification. */
	push(out, `add chain inet ${PROD_TABLE_INET} forward { type filter hook forward priority mangle + 1; policy accept; }`);
	append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
		`iifname "br-lan" ip dscp set cs0`);
	append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
		`oifname "br-lan" ip dscp set cs0`);
	if (ipv6) {
		append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
			`iifname "br-lan" ip6 dscp set cs0`);
		append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
			`oifname "br-lan" ip6 dscp set cs0`);
	}
	append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
		`jump ipv4_upload`);
	append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
		`jump ipv4_download`);
	if (ipv6) {
		append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'forward',
			`jump ipv6_upload`);
		push(out, `add chain bridge ${PROD_TABLE_BRIDGE} postrouting { type filter hook postrouting priority 300; policy accept; }`);
		append_rule(out, seen_rules, 'bridge', PROD_TABLE_BRIDGE, 'postrouting',
			`jump ipv6_download`);
	}

	if (has_prerouting_marks(plan)) {
		push(out, `add chain inet ${PROD_TABLE_INET} prerouting { type filter hook prerouting priority mangle; policy accept; }`);

		for (let client in plan.static_clients) {
			let client_mark = wan_mark_for_client(client.interfacename, plan.wan_balance.interfaces);
			if (client_mark != null && client.ipv4 != null)
				append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'prerouting',
					`iifname "br-lan" meta nfproto ipv4 ip saddr ${client.ipv4} ct state new ct mark 0 ct mark set ${client_mark} meta mark set ${client_mark}`);
		}

		if (plan.wan_balance.enabled) {
			for (let sm in plan.wan_balance.special_marks) {
				append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'prerouting',
					`iifname "br-lan" meta nfproto ipv4 ct state new ct mark 0 ${sm.protocol} dport ${sm.destination_port} ct mark set ${sm.mark} meta mark set ${sm.mark}`);
			}

			for (let iface in plan.wan_balance.interfaces) {
				if (iface.subnet)
					append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'prerouting',
						`iifname "br-lan" meta nfproto ipv4 ct state new ct mark 0 ip daddr ${iface.subnet} ct mark set ${iface.mark} meta mark set ${iface.mark}`);
			}

			let n_wan = length(plan.wan_balance.interfaces);
			if (n_wan > 0) {
				let map_elems = [];
				for (let bucket = 0; bucket < n_wan; bucket++)
					push(map_elems, `${bucket} : ${plan.wan_balance.interfaces[bucket].mark}`);
				let generic_wan_rule = `iifname "br-lan" meta nfproto ipv4 ct state new ct mark 0 ct mark set numgen inc mod ${n_wan} map { ${join(', ', map_elems)} } meta mark set ct mark`;
				append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'prerouting', generic_wan_rule);
			}
		}

		if (length(owned_marks))
			append_rule(out, seen_rules, 'inet', PROD_TABLE_INET, 'prerouting',
				`iifname "br-lan" meta nfproto ipv4 ${mark_match(owned_marks)} meta mark set ct mark`);
	}

	return join('\n', out) + '\n';
}

function render_prod_leases(plan) {
	let ipv6 = plan.ipv6_enabled;
	let out = [
		`# eqos-owner:v1 mode=leases generation=${plan.generation} ipv6=${ipv6 ? 'true' : 'false'}`
	];

	append_map_objects(out, lease_map_data(plan.lease_clients), 'leases', PROD_TABLE_INET, PROD_TABLE_BRIDGE, ipv6);
	return join('\n', out) + '\n';
}

function render(input) {
	if (input.schema == 'eqos-nft/v1')
		return input.mode == 'install' ? render_legacy_install(input) : render_legacy_leases(input);

	if (input.mode == 'stop' || !input.plan.enabled)
		return render_prod_stop(input.plan, input.mode);

	if (input.mode == 'leases')
		return render_prod_leases(input.plan);

	return render_prod_install(input.plan);
}

if (length(ARGV) != 1) {
	warn('usage: ucode eqos-nft-render.uc INPUT.json\n');
	exit(2);
}

let fp = open(ARGV[0]);
if (!fp)
	fail(`cannot open input: ${error()}`);

let raw = fp.read('all');
fp.close();

let document;
try {
	document = json(raw);
} catch (e) {
	fail(`invalid JSON: ${e}`);
}

/* Build/validate completely before the sole stdout write. */
let output = render(normalize_document(document));
print(output);
exit(0);
