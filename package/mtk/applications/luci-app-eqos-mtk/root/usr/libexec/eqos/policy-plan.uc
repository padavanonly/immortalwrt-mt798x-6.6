#!/usr/bin/ucode
// SPDX-License-Identifier: GPL-2.0-only
'use strict';

/*
 * Inactive EQoS policy planner. This program consumes one caller-provided
 * snapshot and registry, then emits a complete plan and next registry. It
 * performs no UCI, route, lease-file, nft, tc, debugfs, service, or registry
 * I/O. A future applier owns registry persistence after successful install.
 */

import { open, error } from 'fs';

let MAX_SLOT = 31;
let IPV4_RE = /^(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.(0|[1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$/;
let MAC_RE = /^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/;

function has_key(object, key) {
	if (type(object) != 'object')
		return false;

	for (let candidate in object)
		if (candidate == key)
			return true;

	return false;
}

function fail(code, detail, fields) {
	let result = {
		schema: 'eqos-policy-result/v1',
		status: 'error',
		code,
		detail
	};

	if (type(fields) == 'object')
		for (let key in fields)
			result[key] = fields[key];

	print(sprintf('%J', result) + '\n');
	exit(2);
}

function check_keys(object, allowed, required, path) {
	if (type(object) != 'object')
		fail('invalid_input', `${path}: expected object`);

	for (let key in object) {
		let found = false;
		for (let candidate in allowed)
			if (candidate == key)
				found = true;
		if (!found)
			fail('invalid_input', `${path}: unsupported key '${key}'`);
	}

	for (let key in required)
		if (!has_key(object, key))
			fail('invalid_input', `${path}: missing key '${key}'`);
}

function require_array(value, path) {
	if (type(value) != 'array')
		fail('invalid_input', `${path}: expected array`);
}

function require_bool(value, path) {
	if (type(value) != 'bool')
		fail('invalid_input', `${path}: expected boolean`);
}

function require_int(value, path, minimum) {
	if (type(value) != 'int' || value < minimum)
		fail('invalid_input', `${path}: expected integer >= ${minimum}`);
}

function require_string(value, path, nonempty) {
	if (type(value) != 'string' || (nonempty && value == ''))
		fail('invalid_input', `${path}: expected ${nonempty ? 'nonempty ' : ''}string`);
}

function canonical_ipv4(value, path) {
	if (type(value) != 'string')
		fail('invalid_input', `${path}: expected canonical IPv4 string`);

	let matched = match(value, IPV4_RE);
	if (type(matched) != 'array' || !length(matched) || matched[0] != value)
		fail('invalid_input', `${path}: expected canonical IPv4 without leading zeros`);

	return value;
}

function canonical_mac(value, path) {
	if (type(value) != 'string')
		fail('invalid_input', `${path}: expected six-octet MAC string`);

	let matched = match(value, MAC_RE);
	if (type(matched) != 'array' || !length(matched) || matched[0] != value)
		fail('invalid_input', `${path}: expected six colon-separated hexadecimal octets`);

	return lc(value);
}

function canonical_identity(value, path) {
	if (type(value) != 'string')
		fail('invalid_registry', `${path}: expected identity string`);

	if (substr(value, 0, 4) == 'mac:')
		return `mac:${canonical_mac(substr(value, 4), path)}`;
	if (substr(value, 0, 5) == 'ipv4:')
		return `ipv4:${canonical_ipv4(substr(value, 5), path)}`;

	fail('invalid_registry', `${path}: identity must start with mac: or ipv4:`);
}

function string_cmp(left, right) {
	if (left < right)
		return -1;
	if (left > right)
		return 1;
	return 0;
}

function sorted_unique(values) {
	let sorted = [];
	for (let value in values)
		push(sorted, value);
	sort(sorted, string_cmp);

	let result = [];
	let seen = {};
	for (let value in sorted) {
		if (has_key(seen, value))
			continue;
		seen[value] = true;
		push(result, value);
	}
	return result;
}

function registry_identity_tokens(entry) {
	let values = [ entry.identity ];
	/* Dynamic lease addresses can be reassigned; only static links persist. */
	if (entry.static_reserved)
		for (let alias in entry.aliases)
			push(values, alias);
	return sorted_unique(values);
}

function registry_macs(identity, aliases) {
	let values = [];
	if (substr(identity, 0, 4) == 'mac:')
		push(values, substr(identity, 4));
	for (let alias in aliases)
		if (substr(alias, 0, 4) == 'mac:')
			push(values, substr(alias, 4));
	return sorted_unique(values);
}

function normalize_policy(policy) {
	check_keys(policy,
		[ 'enabled', 'ipv6_enabled', 'shaper', 'wan_balance' ],
		[ 'enabled', 'ipv6_enabled', 'shaper', 'wan_balance' ], 'snapshot.policy');
	require_bool(policy.enabled, 'snapshot.policy.enabled');
	require_bool(policy.ipv6_enabled, 'snapshot.policy.ipv6_enabled');

	check_keys(policy.shaper,
		[ 'download_mbit', 'upload_mbit', 'smartqos' ],
		[ 'download_mbit', 'upload_mbit', 'smartqos' ], 'snapshot.policy.shaper');
	require_int(policy.shaper.download_mbit, 'snapshot.policy.shaper.download_mbit', 0);
	require_int(policy.shaper.upload_mbit, 'snapshot.policy.shaper.upload_mbit', 0);
	require_bool(policy.shaper.smartqos, 'snapshot.policy.shaper.smartqos');

	let wan = policy.wan_balance;
	check_keys(wan, [ 'enabled', 'interfaces', 'special_marks' ],
		[ 'enabled', 'interfaces', 'special_marks' ], 'snapshot.policy.wan_balance');
	require_bool(wan.enabled, 'snapshot.policy.wan_balance.enabled');
	require_array(wan.interfaces, 'snapshot.policy.wan_balance.interfaces');
	require_array(wan.special_marks, 'snapshot.policy.wan_balance.special_marks');

	let interfaces = [];
	let seen_indices = {};
	for (let i = 0; i < length(wan.interfaces); i++) {
		let source = wan.interfaces[i];
		let path = `snapshot.policy.wan_balance.interfaces[${i}]`;
		check_keys(source,
			[ 'index', 'ifname', 'gateway', 'subnet', 'mark', 'table' ],
			[ 'index', 'ifname', 'gateway', 'subnet', 'mark', 'table' ], path);
		require_int(source.index, `${path}.index`, 0);
		require_string(source.ifname, `${path}.ifname`, true);
		/* PPP links commonly expose a default route without a gateway or subnet. */
		require_string(source.gateway, `${path}.gateway`, false);
		require_string(source.subnet, `${path}.subnet`, false);
		require_string(source.mark, `${path}.mark`, true);
		require_string(source.table, `${path}.table`, true);
		let index_key = `${source.index}`;
		if (has_key(seen_indices, index_key))
			fail('invalid_input', `${path}: duplicate WAN index ${source.index}`);
		seen_indices[index_key] = true;
		push(interfaces, {
			index: source.index,
			ifname: source.ifname,
			gateway: source.gateway,
			subnet: source.subnet,
			mark: source.mark,
			table: source.table
		});
	}

	let active_wan_marks = {};
	for (let iface in interfaces)
		active_wan_marks[iface.mark] = true;

	let special_marks = [];
	for (let i = 0; i < length(wan.special_marks); i++) {
		let source = wan.special_marks[i];
		let path = `snapshot.policy.wan_balance.special_marks[${i}]`;
		check_keys(source,
			[ 'protocol', 'destination_port', 'mark' ],
			[ 'protocol', 'destination_port', 'mark' ], path);
		if (source.protocol != 'tcp' && source.protocol != 'udp')
			fail('invalid_input', `${path}.protocol: expected tcp or udp`);
		require_int(source.destination_port, `${path}.destination_port`, 1);
		if (source.destination_port > 65535)
			fail('invalid_input', `${path}.destination_port: exceeds 65535`);
		require_string(source.mark, `${path}.mark`, true);
		if (!has_key(active_wan_marks, source.mark))
			fail('invalid_input', `${path}.mark: must identify an active WAN mark`);
		push(special_marks, {
			protocol: source.protocol,
			destination_port: source.destination_port,
			mark: source.mark
		});
	}

	return {
		enabled: policy.enabled,
		ipv6_enabled: policy.ipv6_enabled,
		shaper: {
			download_mbit: policy.shaper.download_mbit,
			upload_mbit: policy.shaper.upload_mbit,
			smartqos: policy.shaper.smartqos
		},
		wan_balance: {
			enabled: wan.enabled,
			interfaces,
			special_marks
		}
	};
}

function normalize_static_clients(records, lease_ip_owner, static_mac_for_ip) {
	require_array(records, 'snapshot.static_clients');
	let normalized = [];
	let static_ip_owner = {};

	for (let i = 0; i < length(records); i++) {
		let source = records[i];
		let path = `snapshot.static_clients[${i}]`;
		check_keys(source,
			[ 'mac', 'ipv4', 'upload', 'download', 'upload_kbit', 'download_kbit', 'interfacename' ],
			[ 'upload', 'download', 'upload_kbit', 'download_kbit' ], path);

		let mac = has_key(source, 'mac') ? canonical_mac(source.mac, `${path}.mac`) : null;
		let ipv4 = has_key(source, 'ipv4') ? canonical_ipv4(source.ipv4, `${path}.ipv4`) : null;
		if (!mac && !ipv4)
			fail('invalid_input', `${path}: requires mac or ipv4`);
		require_bool(source.upload, `${path}.upload`);
		require_bool(source.download, `${path}.download`);
		require_int(source.upload_kbit, `${path}.upload_kbit`, 0);
		require_int(source.download_kbit, `${path}.download_kbit`, 0);
		let interfacename = null;
		if (has_key(source, 'interfacename')) {
			require_string(source.interfacename, `${path}.interfacename`, false);
			interfacename = source.interfacename;
		}

		let owner_mac = mac;
		if (ipv4) {
			if (has_key(lease_ip_owner, ipv4)) {
				if (owner_mac && owner_mac != lease_ip_owner[ipv4])
					fail('identity_conflict', `${path}: IPv4 lease belongs to a different MAC`, { ipv4, configured_mac: owner_mac, lease_mac: lease_ip_owner[ipv4] });
				owner_mac = lease_ip_owner[ipv4];
			}
			if (has_key(static_mac_for_ip, ipv4)) {
				if (owner_mac && owner_mac != static_mac_for_ip[ipv4])
					fail('identity_conflict', `${path}: static IPv4 is associated with a different MAC`, { ipv4, configured_mac: owner_mac, other_static_mac: static_mac_for_ip[ipv4] });
				owner_mac = static_mac_for_ip[ipv4];
			}
		}

		let identity = owner_mac ? `mac:${owner_mac}` : `ipv4:${ipv4}`;
		let row = {
			identity,
			mac: owner_mac,
			ipv4,
			upload: source.upload,
			download: source.download,
			upload_kbit: source.upload_kbit,
			download_kbit: source.download_kbit,
			interfacename
		};

		if (ipv4) {
			if (has_key(static_ip_owner, ipv4) && static_ip_owner[ipv4] != identity)
				fail('identity_conflict', `${path}: IPv4 appears under different static identities`, { ipv4 });
			static_ip_owner[ipv4] = identity;
		}
		push(normalized, row);
	}

	let grouped = {};
	for (let row in normalized) {
		let group = grouped[row.identity];
		let policy = {
			upload: row.upload,
			download: row.download,
			upload_kbit: row.upload_kbit,
			download_kbit: row.download_kbit,
			interfacename: row.interfacename
		};

		if (!group) {
			group = {
				identity: row.identity,
				mac: row.mac,
				policy,
				addresses: {},
				rows: []
			};
			grouped[row.identity] = group;
		}
		else if (sprintf('%J', group.policy) != sprintf('%J', policy)) {
			fail('identity_conflict', `static identity ${row.identity} has inconsistent directions, rates, or WAN interface`, { identity: row.identity });
		}

		if (row.mac && group.mac && row.mac != group.mac)
			fail('identity_conflict', `static identity ${row.identity} maps to multiple MACs`, { identity: row.identity });
		if (row.mac)
			group.mac = row.mac;
		if (row.ipv4)
			group.addresses[row.ipv4] = true;
		push(group.rows, row);
	}

	let ids = [];
	for (let identity in grouped)
		push(ids, identity);
	sort(ids, string_cmp);

	let result = [];
	for (let identity in ids) {
		let group = grouped[identity];
		let addresses = [];
		for (let address in group.addresses)
			push(addresses, address);
		sort(addresses, string_cmp);
		group.addresses = addresses;
		push(result, group);
	}

	return result;
}

function normalize_leases(records, now_epoch) {
	require_array(records, 'snapshot.active_leases');
	let normalized = [];
	let ip_owner = {};
	let pair_index = {};

	for (let i = 0; i < length(records); i++) {
		let source = records[i];
		let path = `snapshot.active_leases[${i}]`;
		check_keys(source, [ 'mac', 'ipv4', 'expires_epoch' ], [ 'mac', 'ipv4', 'expires_epoch' ], path);
		let mac = canonical_mac(source.mac, `${path}.mac`);
		let ipv4 = canonical_ipv4(source.ipv4, `${path}.ipv4`);
		require_int(source.expires_epoch, `${path}.expires_epoch`, 0);
		if (source.expires_epoch != 0 && source.expires_epoch <= now_epoch)
			fail('invalid_input', `${path}: active lease is expired at supplied now_epoch`, { mac, ipv4, expires_epoch: source.expires_epoch });

		if (has_key(ip_owner, ipv4) && ip_owner[ipv4] != mac)
			fail('identity_conflict', `${path}: IPv4 is claimed by multiple MACs`, { ipv4 });
		ip_owner[ipv4] = mac;

		let pair = `${mac}|${ipv4}`;
		if (has_key(pair_index, pair)) {
			let existing = normalized[pair_index[pair]];
			if (existing.expires_epoch != source.expires_epoch)
				fail('identity_conflict', `${path}: duplicate MAC/IP lease has conflicting expiry`, { mac, ipv4 });
			continue;
		}
		pair_index[pair] = length(normalized);
		push(normalized, { mac, ipv4, expires_epoch: source.expires_epoch });
	}

	return { rows: normalized, ip_owner };
}

function make_group(groups, identity) {
	if (!has_key(groups, identity)) {
		groups[identity] = {
			identity,
			identity_tokens: {},
			mac: null,
			static_present: false,
			static_policy: null,
			static_addresses: {},
			lease_addresses: {},
			lease_expires_epoch: null
		};
	}
	return groups[identity];
}

function add_token(group, token) {
	group.identity_tokens[token] = true;
}

function build_groups(static_groups, leases, include_leases) {
	let groups = {};

	for (let source in static_groups) {
		let group = make_group(groups, source.identity);
		group.static_present = true;
		group.static_policy = source.policy;
		group.mac = source.mac;
		if (source.mac)
			add_token(group, `mac:${source.mac}`);
		for (let address in source.addresses) {
			group.static_addresses[address] = true;
			add_token(group, `ipv4:${address}`);
		}
		if (source.mac)
			add_token(group, `mac:${source.mac}`);
	}

	if (!include_leases)
		return groups;

	for (let lease in leases.rows) {
		let identity = `mac:${lease.mac}`;
		let group = make_group(groups, identity);
		if (group.mac && group.mac != lease.mac)
			fail('identity_conflict', `planner group ${identity} maps to multiple MACs`, { identity });
		group.mac = lease.mac;
		group.lease_addresses[lease.ipv4] = lease.expires_epoch;
		if (group.lease_expires_epoch == null || lease.expires_epoch == 0 ||
		    (group.lease_expires_epoch != 0 && lease.expires_epoch > group.lease_expires_epoch))
			group.lease_expires_epoch = lease.expires_epoch;
		add_token(group, `mac:${lease.mac}`);
		add_token(group, `ipv4:${lease.ipv4}`);
	}

	return groups;
}

function normalize_registry(registry) {
	if (registry == null)
		return { generation: 0, assignments: [], token_owner: {}, was_null: true };

	check_keys(registry, [ 'schema', 'generation', 'assignments' ],
		[ 'schema', 'generation', 'assignments' ], 'prior_registry');
	if (registry.schema != 'eqos-slot-registry/v1')
		fail('invalid_registry', 'prior_registry.schema must be eqos-slot-registry/v1');
	require_int(registry.generation, 'prior_registry.generation', 0);
	require_array(registry.assignments, 'prior_registry.assignments');

	let assignments = [];
	let token_owner = {};
	let used_slots = {};
	for (let i = 0; i < length(registry.assignments); i++) {
		let source = registry.assignments[i];
		let path = `prior_registry.assignments[${i}]`;
		check_keys(source,
			[ 'identity', 'slot', 'aliases', 'static_reserved', 'static_configured', 'lease_expires_epoch', 'tombstone_until_epoch' ],
			[ 'identity', 'slot', 'aliases', 'static_reserved', 'static_configured', 'lease_expires_epoch', 'tombstone_until_epoch' ], path);
		let identity = canonical_identity(source.identity, `${path}.identity`);
		require_int(source.slot, `${path}.slot`, 1);
		if (source.slot > MAX_SLOT)
			fail('invalid_registry', `${path}.slot exceeds ${MAX_SLOT}`, { identity, slot: source.slot });
		require_array(source.aliases, `${path}.aliases`);
		require_bool(source.static_reserved, `${path}.static_reserved`);
		require_bool(source.static_configured, `${path}.static_configured`);
		if (source.static_configured && !source.static_reserved)
			fail('invalid_registry', `${path}: configured static identity must remain reserved`, { identity });
		let lease_expires = source.lease_expires_epoch;
		let tombstone_until = source.tombstone_until_epoch;
		if (lease_expires != null)
			require_int(lease_expires, `${path}.lease_expires_epoch`, 0);
		if (tombstone_until != null)
			require_int(tombstone_until, `${path}.tombstone_until_epoch`, 0);
		if (lease_expires != null && tombstone_until != null)
			fail('invalid_registry', `${path}: active lease and tombstone cannot both be set`, { identity });
		if (source.static_reserved && tombstone_until != null)
			fail('invalid_registry', `${path}: static reservation cannot be a tombstone`, { identity });
		if (!source.static_reserved && lease_expires == null && tombstone_until == null)
			fail('invalid_registry', `${path}: dynamic assignment needs active lease expiry or tombstone deadline`, { identity });
		let slot_key = `${source.slot}`;
		if (has_key(used_slots, slot_key))
			fail('invalid_registry', `duplicate slot ${source.slot}`, { identity });
		used_slots[slot_key] = true;

		let aliases = [];
		for (let j = 0; j < length(source.aliases); j++) {
			let alias = canonical_identity(source.aliases[j], `${path}.aliases[${j}]`);
			if (alias == identity)
				fail('invalid_registry', `${path}: identity duplicated in aliases`, { identity });
			push(aliases, alias);
		}
		aliases = sorted_unique(aliases);
		if (length(registry_macs(identity, aliases)) > 1)
			fail('invalid_registry', `${path}: assignment cannot map multiple MAC identities`, { identity });

		let entry = {
			identity,
			slot: source.slot,
			aliases,
			static_reserved: source.static_reserved,
			static_configured: source.static_configured,
			lease_expires_epoch: lease_expires,
			tombstone_until_epoch: tombstone_until
		};
		let tokens = registry_identity_tokens(entry);
		for (let token in tokens) {
			if (has_key(token_owner, token))
				fail('invalid_registry', `identity token ${token} belongs to multiple assignments`, { identity, token });
			token_owner[token] = length(assignments);
		}
		push(assignments, entry);
	}

	sort(assignments, (a, b) => a.slot - b.slot);
	/* Rebuild token indexes after stable slot sorting. */
	token_owner = {};
	for (let i = 0; i < length(assignments); i++) {
		for (let token in registry_identity_tokens(assignments[i]))
			token_owner[token] = i;
	}
	return { generation: registry.generation, assignments, token_owner, was_null: false };
}

function group_tokens(group) {
	let tokens = [];
	for (let token in group.identity_tokens)
		push(tokens, token);
	if (!has_key(group.identity_tokens, group.identity))
		push(tokens, group.identity);
	return sorted_unique(tokens);
}

function assignment_macs(entry) {
	let macs = [];
	if (substr(entry.identity, 0, 4) == 'mac:')
		push(macs, substr(entry.identity, 4));
	for (let alias in entry.aliases)
		if (substr(alias, 0, 4) == 'mac:')
			push(macs, substr(alias, 4));
	return sorted_unique(macs);
}

function group_macs(group) {
	let macs = [];
	if (group.mac)
		push(macs, group.mac);
	return macs;
}

function normalized_assignment(entry) {
	return {
		identity: entry.identity,
		slot: entry.slot,
		aliases: entry.aliases,
		static_reserved: entry.static_reserved,
		static_configured: entry.static_configured,
		lease_expires_epoch: entry.lease_expires_epoch,
		tombstone_until_epoch: entry.tombstone_until_epoch
	};
}

function planned_aliases(group, assignment_identity, previous) {
	let aliases = [];
	if (previous && previous.static_reserved)
		for (let alias in previous.aliases)
			push(aliases, alias);

	if ((previous && previous.static_reserved) || group.static_present) {
		if (group.mac)
			push(aliases, `mac:${group.mac}`);
		if (group.static_present)
			for (let address in group.static_addresses)
				push(aliases, `ipv4:${address}`);
	}

	let result = [];
	for (let alias in sorted_unique(aliases))
		if (alias != assignment_identity)
			push(result, alias);
	return result;
}

function make_plan_record(group, assignment, ipv4, is_static) {
	let record = {
		identity: assignment.identity,
		slot: assignment.slot,
		upload: is_static ? group.static_policy.upload : (group.static_present ? group.static_policy.upload : true),
		download: is_static ? group.static_policy.download : (group.static_present ? group.static_policy.download : true)
	};
	if (group.mac)
		record.mac = group.mac;
	if (ipv4)
		record.ipv4 = ipv4;
	if (is_static) {
		record.upload_kbit = group.static_policy.upload_kbit;
		record.download_kbit = group.static_policy.download_kbit;
		if (group.static_policy.interfacename != null)
			record.interfacename = group.static_policy.interfacename;
	}
	return record;
}

function validate_input(input) {
	check_keys(input,
		[ 'schema', 'now_epoch', 'tombstone_grace_seconds', 'snapshot', 'prior_registry' ],
		[ 'schema', 'now_epoch', 'tombstone_grace_seconds', 'snapshot', 'prior_registry' ], 'input');
	if (input.schema != 'eqos-planner-input/v1')
		fail('invalid_input', 'input.schema must be eqos-planner-input/v1');
	require_int(input.now_epoch, 'input.now_epoch', 0);
	require_int(input.tombstone_grace_seconds, 'input.tombstone_grace_seconds', 0);
	check_keys(input.snapshot, [ 'policy', 'static_clients', 'active_leases' ],
		[ 'policy', 'static_clients', 'active_leases' ], 'input.snapshot');
	require_array(input.snapshot.static_clients, 'input.snapshot.static_clients');
	require_array(input.snapshot.active_leases, 'input.snapshot.active_leases');
	return {
		now_epoch: input.now_epoch,
		grace: input.tombstone_grace_seconds,
		policy: normalize_policy(input.snapshot.policy),
		static_records: input.snapshot.static_clients,
		lease_records: input.snapshot.active_leases,
		prior: normalize_registry(input.prior_registry)
	};
}

function run_planner(input) {
	let normalized = validate_input(input);
	if (!normalized.policy.enabled) {
		let assignments = [];
		for (let entry in normalized.prior.assignments)
			push(assignments, normalized_assignment(entry));
		let generation = normalized.prior.generation;
		return {
			schema: 'eqos-policy-result/v1',
			status: 'ready',
			registry_changed: false,
			plan: {
				schema: 'eqos-policy-plan/v1',
				generation,
				enabled: false,
				ipv6_enabled: normalized.policy.ipv6_enabled,
				shaper: normalized.policy.shaper,
				wan_balance: normalized.policy.wan_balance,
				static_clients: [],
				lease_clients: [],
				bypass_v4: []
			},
			next_registry: {
				schema: 'eqos-slot-registry/v1',
				generation,
				assignments
			},
			decisions: []
		};
	}

	let leases = normalize_leases(normalized.lease_records, normalized.now_epoch);

	let static_mac_for_ip = {};
	let static_sources = input.snapshot.static_clients;
	require_array(static_sources, 'snapshot.static_clients');
	for (let i = 0; i < length(static_sources); i++) {
		let source = static_sources[i];
		if (!has_key(source, 'mac') || !has_key(source, 'ipv4'))
			continue;
		let mac = canonical_mac(source.mac, `snapshot.static_clients[${i}].mac`);
		let ipv4 = canonical_ipv4(source.ipv4, `snapshot.static_clients[${i}].ipv4`);
		if (has_key(static_mac_for_ip, ipv4) && static_mac_for_ip[ipv4] != mac)
			fail('identity_conflict', `static IPv4 ${ipv4} is associated with multiple MACs`, { ipv4 });
		static_mac_for_ip[ipv4] = mac;
	}

	let static_groups = normalize_static_clients(static_sources, leases.ip_owner, static_mac_for_ip);
	let groups = build_groups(static_groups, leases, normalized.policy.shaper.smartqos);
	let group_ids = [];
	for (let identity in groups)
		push(group_ids, identity);
	sort(group_ids, string_cmp);

	let prior = normalized.prior;
	let effective_prior = [];
	let effective_token_owner = {};
	for (let i = 0; i < length(prior.assignments); i++) {
		let entry = prior.assignments[i];
		let expired = !entry.static_reserved && entry.tombstone_until_epoch != null &&
			entry.tombstone_until_epoch <= normalized.now_epoch;
		if (expired)
			continue;
		let new_index = length(effective_prior);
		push(effective_prior, entry);
		for (let token in registry_identity_tokens(entry))
			effective_token_owner[token] = new_index;
	}

	let group_prior_index = {};
	let matched_prior_group = {};
	for (let identity in group_ids) {
		let group = groups[identity];
		let matching = {};
		for (let token in group_tokens(group))
			if (has_key(effective_token_owner, token))
				matching[`${effective_token_owner[token]}`] = true;

		let match_indices = [];
		for (let key in matching)
			push(match_indices, int(key));
		if (length(match_indices) > 1)
			fail('identity_conflict', `current identity ${identity} overlaps multiple prior registry assignments`, { identity });
		if (!length(match_indices))
			continue;

		let index = match_indices[0];
		let old = effective_prior[index];
		let old_macs = assignment_macs(old);
		let current_macs = group_macs(group);
		if (length(old_macs) && length(current_macs) && old_macs[0] != current_macs[0])
			fail('alias_mac_changed', `known IPv4/MAC alias changed from ${old_macs[0]} to ${current_macs[0]}`, { identity: old.identity, slot: old.slot });
		if (has_key(matched_prior_group, `${index}`))
			fail('identity_conflict', `prior assignment ${old.identity} maps to multiple current identities`, { identity: old.identity, slot: old.slot });
		matched_prior_group[`${index}`] = identity;
		group_prior_index[identity] = index;
	}

	let next_entries = [];
	let used_slots = {};
	let next_identity_by_group = {};
	let decisions = [];

	for (let identity in group_ids) {
		if (!has_key(group_prior_index, identity))
			continue;
		let group = groups[identity];
		let old = effective_prior[group_prior_index[identity]];
		let next_identity = old.identity;
		let aliases = planned_aliases(group, next_identity, old);
		let assignment = {
			identity: next_identity,
			slot: old.slot,
			aliases,
			static_reserved: old.static_reserved || group.static_present,
			static_configured: group.static_present,
			lease_expires_epoch: group.lease_expires_epoch,
			tombstone_until_epoch: null
		};
		if (old.static_configured && !group.static_present)
			push(decisions, { code: 'static_reservation_retained', identity: next_identity, slot: old.slot });
		if (sprintf('%J', assignment.aliases) != sprintf('%J', old.aliases))
			push(decisions, { code: 'identity_alias_added', identity: next_identity, slot: old.slot });
		next_identity_by_group[identity] = next_identity;
		used_slots[`${old.slot}`] = true;
		push(next_entries, assignment);
	}

	for (let i = 0; i < length(effective_prior); i++) {
		if (has_key(matched_prior_group, `${i}`))
			continue;
		let old = effective_prior[i];
		if (old.static_reserved) {
			let retained = normalized_assignment(old);
			retained.static_configured = false;
			retained.lease_expires_epoch = null;
			retained.tombstone_until_epoch = null;
			used_slots[`${old.slot}`] = true;
			push(next_entries, retained);
			if (old.static_configured)
				push(decisions, { code: 'static_reservation_retained', identity: old.identity, slot: old.slot });
			continue;
		}

		let deadline = old.tombstone_until_epoch;
		if (deadline == null)
			deadline = normalized.now_epoch + normalized.grace;
		if (deadline > normalized.now_epoch) {
			let retained = normalized_assignment(old);
			retained.lease_expires_epoch = null;
			retained.tombstone_until_epoch = deadline;
			used_slots[`${old.slot}`] = true;
			push(next_entries, retained);
			if (old.tombstone_until_epoch == null)
				push(decisions, { code: 'lease_tombstone_created', identity: old.identity, slot: old.slot, until_epoch: deadline });
		}
		else {
			push(decisions, { code: 'lease_slot_released', identity: old.identity, slot: old.slot });
		}
	}

	let new_group_ids = [];
	for (let identity in group_ids)
		if (!has_key(group_prior_index, identity))
			push(new_group_ids, identity);
	sort(new_group_ids, (left, right) => {
		let left_static = groups[left].static_present;
		let right_static = groups[right].static_present;
		if (left_static != right_static)
			return left_static ? -1 : 1;
		return string_cmp(left, right);
	});
	let required_total = length(next_entries) + length(new_group_ids);
	let available_slots = 0;
	for (let candidate = 1; candidate <= MAX_SLOT; candidate++)
		if (!has_key(used_slots, `${candidate}`))
			available_slots++;

	for (let identity in new_group_ids) {
		let group = groups[identity];
		let slot = 0;
		for (let candidate = 1; candidate <= MAX_SLOT; candidate++) {
			if (!has_key(used_slots, `${candidate}`)) {
				slot = candidate;
				break;
			}
		}
		if (!slot) {
			fail('slot_capacity', 'no free EQoS slot remains', {
				identity,
				limit: MAX_SLOT,
				required: required_total,
				available: available_slots
			});
		}
		let tokens = group_tokens(group);
		let aliases = [];
		for (let token in tokens)
			if (token != identity)
				push(aliases, token);
		let assignment = {
			identity,
			slot,
			aliases: planned_aliases(group, identity, null),
			static_reserved: group.static_present,
			static_configured: group.static_present,
			lease_expires_epoch: group.lease_expires_epoch,
			tombstone_until_epoch: null
		};
		next_identity_by_group[identity] = identity;
		used_slots[`${slot}`] = true;
		push(next_entries, assignment);
		push(decisions, { code: 'slot_allocated', identity, slot, class: group.static_present ? 'static' : 'lease' });
	}

	sort(next_entries, (left, right) => {
		let slot_order = left.slot - right.slot;
		return slot_order ? slot_order : string_cmp(left.identity, right.identity);
	});
	sort(decisions, (left, right) => {
		let identity_order = string_cmp(left.identity || '', right.identity || '');
		return identity_order ? identity_order : string_cmp(left.code, right.code);
	});

	let previous_assignments = [];
	for (let entry in prior.assignments)
		push(previous_assignments, normalized_assignment(entry));
	sort(previous_assignments, (left, right) => {
		let slot_order = left.slot - right.slot;
		return slot_order ? slot_order : string_cmp(left.identity, right.identity);
	});
	let registry_changed = prior.was_null || sprintf('%J', previous_assignments) != sprintf('%J', next_entries);
	let generation = prior.generation + (registry_changed ? 1 : 0);

	let plan_static = [];
	let plan_leases = [];
	for (let identity in group_ids) {
		let group = groups[identity];
		let assigned_identity = next_identity_by_group[identity];
		let assignment = null;
		for (let candidate in next_entries)
			if (candidate.identity == assigned_identity) {
				assignment = candidate;
				break;
			}
		if (!assignment)
			fail('internal_error', `missing planned assignment for ${identity}`);

		let lease_addresses = [];
		for (let address in group.lease_addresses)
			push(lease_addresses, address);
		sort(lease_addresses, string_cmp);
		let static_addresses = [];
		for (let address in group.static_addresses)
			push(static_addresses, address);
		static_addresses = sorted_unique(static_addresses);

		if (group.static_present) {
			if (length(static_addresses)) {
				for (let address in static_addresses)
					push(plan_static, make_plan_record(group, assignment, address, true));
			}
			else {
				push(plan_static, make_plan_record(group, assignment, null, true));
			}
		}

		for (let address in lease_addresses)
			push(plan_leases, make_plan_record(group, assignment, address, false));
	}

	sort(plan_static, (left, right) => {
		let slot_order = left.slot - right.slot;
		if (slot_order)
			return slot_order;
		return string_cmp(left.ipv4 || '', right.ipv4 || '');
	});
	sort(plan_leases, (left, right) => {
		let slot_order = left.slot - right.slot;
		if (slot_order)
			return slot_order;
		let mac_order = string_cmp(left.mac || '', right.mac || '');
		return mac_order ? mac_order : string_cmp(left.ipv4 || '', right.ipv4 || '');
	});

	let plan = {
		schema: 'eqos-policy-plan/v1',
		generation,
		enabled: normalized.policy.enabled,
		ipv6_enabled: normalized.policy.ipv6_enabled,
		shaper: normalized.policy.shaper,
		wan_balance: normalized.policy.wan_balance,
		static_clients: plan_static,
		lease_clients: plan_leases,
		bypass_v4: []
	};
	let next_registry = {
		schema: 'eqos-slot-registry/v1',
		generation,
		assignments: next_entries
	};

	return {
		schema: 'eqos-policy-result/v1',
		status: 'ready',
		registry_changed,
		plan,
		next_registry,
		decisions
	};
}

if (length(ARGV) != 1)
	fail('usage', 'usage: ucode policy-plan.uc INPUT.json');

let fp = open(ARGV[0]);
if (!fp)
	fail('input_read', `cannot open input: ${error()}`);
let raw = fp.read('all');
fp.close();

let document;
try {
	document = json(raw);
}
catch (e) {
	fail('invalid_json', `${e}`);
}

let result = run_planner(document);
print(sprintf('%J', result) + '\n');
exit(0);
