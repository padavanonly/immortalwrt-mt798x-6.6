#!/usr/bin/ucode
// SPDX-License-Identifier: GPL-2.0-only
'use strict';

import * as fs from 'fs';

let STATE_DIR = getenv('EQOS_STATE_DIR') || '/var/run/eqos';
let IP_BIN = getenv('EQOS_IP_BIN') || 'ip';

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

function ip_del_ignore_missing(cmd) {
	let res = run_cmd(cmd);
	if (res.rc == 0)
		return true;
	let out = res.stdout || '';
	let ign = [
		'No such file or directory',
		'Cannot find device',
		'Cannot delete qdisc with handle of zero',
		'Invalid argument',
		'does not exist',
		'No such device',
		'No such process'
	];
	for (let s in ign) {
		if (index(out, s) >= 0)
			return true;
	}
	if (res.rc == 2 && trim(out) == '')
		return true;
	return false;
}

function parse_rule_line(line) {
	let s = trim(line);
	if (!s || s == '')
		return null;
	let tokens = split(s, /[ \t]+/);
	if (!length(tokens))
		return null;
	let m_prio = match(tokens[0], /^([0-9]+):?$/);
	if (!m_prio)
		return null;
	let priority = int(m_prio[1]);
	let mark_info = null;
	let table = null;
	for (let i = 1; i < length(tokens); i++) {
		if (tokens[i] == 'fwmark' && i + 1 < length(tokens)) {
			let p = split(tokens[i + 1], '/');
			let val = int(p[0], 0);
			let mask = (length(p) > 1) ? int(p[1], 0) : 0xffffffff;
			mark_info = { val, mask, raw: tokens[i + 1] };
		}
		if ((tokens[i] == 'lookup' || tokens[i] == 'table') && i + 1 < length(tokens)) {
			table = tokens[i + 1];
		}
	}
	return { priority, mark: mark_info, table, raw: s };
}

function get_live_rules() {
	let res = run_cmd(`${sh_quote(IP_BIN)} -4 rule show 2>&1`);
	if (res.rc != 0) {
		return {
			ok: false,
			exists: false,
			objects: [],
			error: trim(res.stdout) || 'ip -4 rule show failed',
			rc: res.rc
		};
	}
	let lines = split(res.stdout || '', '\n');
	let rules = [];
	for (let line in lines) {
		let r = parse_rule_line(line);
		if (r)
			push(rules, r);
	}
	return {
		ok: true,
		exists: length(rules) > 0,
		objects: rules,
		error: '',
		rc: 0
	};
}

function parse_route_line(line) {
	let s = trim(line);
	if (!s || s == '')
		return null;
	let tokens = split(s, /[ \t]+/);
	if (!length(tokens))
		return null;
	let destination = tokens[0];
	let dev = '';
	let via = '';
	for (let i = 1; i < length(tokens); i++) {
		if (tokens[i] == 'dev' && i + 1 < length(tokens))
			dev = tokens[i + 1];
		if (tokens[i] == 'via' && i + 1 < length(tokens))
			via = tokens[i + 1];
	}
	return { destination, dev, via, raw: s };
}

function get_live_routes(table) {
	let res = run_cmd(`${sh_quote(IP_BIN)} -4 route show table ${sh_quote(table)} 2>&1`);
	if (res.rc != 0) {
		if (index(res.stdout || '', 'FIB table does not exist') >= 0) {
			return {
				ok: true,
				exists: false,
				objects: [],
				error: '',
				rc: 0
			};
		}
		return {
			ok: false,
			exists: false,
			objects: [],
			error: trim(res.stdout) || sprintf('ip -4 route show table %s failed (rc=%d)', table, res.rc),
			rc: res.rc
		};
	}
	let lines = split(res.stdout || '', '\n');
	let routes = [];
	for (let line in lines) {
		let r = parse_route_line(line);
		if (r)
			push(routes, r);
	}
	return {
		ok: true,
		exists: length(routes) > 0,
		objects: routes,
		error: '',
		rc: 0
	};
}

function query_active_wans_from_csv(csv) {
	if (!csv || csv == '')
		return [];
	let parts = split(csv, /[,\s]+/);
	let res = run_cmd(`${sh_quote(IP_BIN)} -4 route show table main 2>&1`);
	if (res.rc != 0) {
		let err = {
			status: 'error',
			code: 'runtime_helper_failed',
			detail: trim(res.stdout) || 'ip -4 route show table main failed'
		};
		print(sprintf('%J\n', err));
		exit(1);
	}
	let lines = split(res.stdout || '', '\n');
	let interfaces = [];
	for (let i = 0; i < length(parts); i++) {
		let ifname = trim(parts[i]);
		if (ifname == '')
			continue;
		let gateway = '';
		let found = false;
		for (let line in lines) {
			let r = parse_route_line(line);
			if (r && r.destination == 'default' && r.dev == ifname) {
				found = true;
				gateway = r.via;
				break;
			}
		}
		if (found) {
			push(interfaces, {
				index: i,
				ifname,
				gateway,
				subnet: '',
				mark: `2${i}`,
				table: `2${i}0`
			});
		}
	}
	return interfaces;
}

function parse_mark_info(m) {
	if (m == null || m == '')
		return null;
	let p = split(`${m}`, '/');
	let val = int(p[0], 0);
	let mask = (length(p) > 1) ? int(p[1], 0) : 0xffffffff;
	return { val, mask };
}

function parse_mark_val(m) {
	let info = parse_mark_info(m);
	return info ? info.val : null;
}

let stage = 'all';
let raw_arg = '';

for (let i = 0; i < length(ARGV); i++) {
	let arg = ARGV[i];
	if (arg == '--stage' && i + 1 < length(ARGV)) {
		stage = ARGV[++i];
	}
	else if (substr(arg, 0, 8) == '--stage=') {
		stage = substr(arg, 8);
	}
	else if (raw_arg == '' && substr(arg, 0, 1) != '-') {
		raw_arg = trim(arg);
	}
}

let desired_interfaces = [];

if (raw_arg == '' || raw_arg == '[]' || raw_arg == '""') {
	desired_interfaces = [];
}
else if (substr(raw_arg, 0, 1) == '[' || substr(raw_arg, 0, 1) == '{') {
	let parsed = null;
	try {
		parsed = json(raw_arg);
	}
	catch (e) {
		parsed = null;
	}
	if (type(parsed) == 'array') {
		desired_interfaces = parsed;
	}
	else if (type(parsed) == 'object' && type(parsed.interfaces) == 'array') {
		desired_interfaces = parsed.interfaces;
	}
	else {
		let err = {
			status: 'error',
			code: 'runtime_helper_failed',
			detail: 'Invalid route-reconcile JSON argument'
		};
		print(sprintf('%J\n', err));
		exit(1);
	}
}
else if (fs.stat(raw_arg)) {
	let parsed = null;
	try {
		parsed = json(fs.readfile(raw_arg));
	}
	catch (e) {
		parsed = null;
	}
	if (type(parsed) == 'array') {
		desired_interfaces = parsed;
	}
	else if (type(parsed) == 'object' && type(parsed.interfaces) == 'array') {
		desired_interfaces = parsed.interfaces;
	}
	else {
		let err = {
			status: 'error',
			code: 'runtime_helper_failed',
			detail: 'Invalid route-reconcile JSON file'
		};
		print(sprintf('%J\n', err));
		exit(1);
	}
}
else {
	desired_interfaces = query_active_wans_from_csv(raw_arg);
}

if (!mkdir_p(STATE_DIR)) {
	let err = {
		status: 'error',
		code: 'state_write_failed',
		detail: `Failed to create state dir ${STATE_DIR}`
	};
	print(sprintf('%J\n', err));
	exit(1);
}

let state_file = `${STATE_DIR}/route-state.json`;
let prior_state = [];
if (fs.stat(state_file)) {
	let parsed = null;
	try {
		parsed = json(fs.readfile(state_file));
	}
	catch (e) {
		parsed = null;
	}
	if (type(parsed) != 'array') {
		let err = {
			status: 'error',
			code: 'invalid_route_state',
			detail: 'Failed to parse route-state.json'
		};
		print(sprintf('%J\n', err));
		exit(1);
	}
	prior_state = parsed;
}

let rules_res = get_live_rules();
if (!rules_res.ok) {
	let err = {
		status: 'error',
		code: 'runtime_helper_failed',
		detail: rules_res.error
	};
	print(sprintf('%J\n', err));
	exit(1);
}
let live_rules = rules_res.objects;

function find_owned_record(prior_records, entry) {
	let target_prio = 30000 + int(entry.index);
	let target_tbl = `${entry.table}`;
	let target_mark_val = int(entry.mark, 0);
	for (let owned in prior_records) {
		if (type(owned) == 'object' && int(owned.index) == int(entry.index) &&
		    owned.rule && `${owned.rule.table}` == target_tbl &&
		    int(owned.rule.priority) == target_prio) {
			let minfo = parse_mark_info(owned.rule.fwmark);
			if (minfo && minfo.val == target_mark_val && minfo.mask == 0xffffffff)
				return owned;
		}
	}
	return null;
}

function remove_state_entry(arr, idx, tbl) {
	let out = [];
	for (let item in arr) {
		if (type(item) == 'object' && int(item.index) == int(idx) &&
		    item.rule && `${item.rule.table}` == `${tbl}`)
			continue;
		push(out, item);
	}
	return out;
}

function upsert_state_entry(arr, rec) {
	let out = [];
	let replaced = false;
	for (let item in arr) {
		if (type(item) == 'object' && int(item.index) == int(rec.index) &&
		    item.rule && `${item.rule.table}` == `${rec.rule.table}`) {
			push(out, rec);
			replaced = true;
		}
		else {
			push(out, item);
		}
	}
	if (!replaced)
		push(out, rec);
	return out;
}

function recover_pending_ownership() {
	let journal_file = `${STATE_DIR}/journal.json`;
	if (!fs.stat(journal_file)) {
		if (stage == 'recover') {
			print(sprintf('%J\n', { status: 'ok', stage: 'recover', recovered: false }));
			exit(0);
		}
		return;
	}

	let j_raw = fs.readfile(journal_file);
	let j_doc = null;
	try {
		j_doc = json(j_raw);
	}
	catch (e) {
		j_doc = null;
	}
	if (type(j_doc) != 'object') {
		let err = {
			status: 'error',
			code: 'invalid_journal',
			detail: 'Failed to parse journal.json'
		};
		print(sprintf('%J\n', err));
		exit(1);
	}

	let has_route_ops = (type(j_doc.route_ops) == 'array');
	let candidates = has_route_ops
		? j_doc.route_ops
		: ((type(j_doc.desired_wans) == 'array') ? j_doc.desired_wans : []);

	let state_dirty = false;

	for (let jw in candidates) {
		if (type(jw) != 'object' || jw.index == null || jw.table == null || jw.mark == null)
			continue;

		let target_idx = int(jw.index);
		let target_prio = (jw.priority != null) ? int(jw.priority) : (30000 + target_idx);
		let target_tbl = `${jw.table}`;
		let target_mark_val = int(jw.mark, 0);
		let target_mask = (jw.mask != null) ? int(jw.mask, 0) : 0xffffffff;
		let target_dev = jw.dev || jw.ifname || '';
		let target_via = (jw.gateway != null) ? `${jw.gateway}` : '';
		let target_fwmark = (jw.fwmark != null) ? `${jw.fwmark}` : `${target_mark_val}/0xffffffff`;

		let matching_owned = null;
		for (let owned in prior_state) {
			if (type(owned) == 'object' && int(owned.index) == target_idx &&
			    owned.rule && `${owned.rule.table}` == target_tbl &&
			    int(owned.rule.priority) == target_prio) {
				let minfo = parse_mark_info(owned.rule.fwmark);
				if (minfo && minfo.val == target_mark_val && minfo.mask == target_mask) {
					matching_owned = owned;
					break;
				}
			}
		}

		if (matching_owned != null) {
			for (let r in live_rules) {
				if (r.priority == target_prio || r.table == target_tbl || (r.mark && r.mark.val == target_mark_val)) {
					if (!(r.priority == target_prio && r.table == target_tbl &&
					      r.mark != null && r.mark.val == target_mark_val && r.mark.mask == target_mask)) {
						let err = {
							status: 'error',
							code: 'route_conflict',
							detail: sprintf('recovery conflict: rule %s does not match owned selector', r.raw)
						};
						print(sprintf('%J\n', err));
						exit(1);
					}
				}
			}

			let routes_res = get_live_routes(target_tbl);
			if (!routes_res.ok) {
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: routes_res.error
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
			let default_route = null;
			for (let r in routes_res.objects) {
				if (r.destination == 'default') {
					default_route = r;
					break;
				}
			}
			let cur_dev = (matching_owned.route && matching_owned.route.dev != null) ? matching_owned.route.dev : '';
			let cur_via = (matching_owned.route && matching_owned.route.via != null) ? matching_owned.route.via : '';
			if (default_route != null) {
				let matches_target = (default_route.dev == target_dev && default_route.via == target_via);
				let matches_cur = (default_route.dev == cur_dev && default_route.via == cur_via);
				if (!matches_target && !matches_cur) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('recovery conflict: default route in table %s (%s) matches neither owned nor journaled route', target_tbl, default_route.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
				if (matches_target && (cur_dev != target_dev || cur_via != target_via)) {
					matching_owned.route = {
						destination: 'default',
						dev: target_dev,
						via: target_via
					};
					state_dirty = true;
				}
			}
		}
		else {
			let matching_rule = null;
			let conflicting_rule = null;
			for (let r in live_rules) {
				if (r.priority == target_prio || r.table == target_tbl || (r.mark && r.mark.val == target_mark_val)) {
					if (r.priority == target_prio && r.table == target_tbl &&
					    r.mark != null && r.mark.val == target_mark_val && r.mark.mask == target_mask) {
						matching_rule = r;
					}
					else {
						conflicting_rule = r;
					}
				}
			}
			if (conflicting_rule != null) {
				let err = {
					status: 'error',
					code: 'route_conflict',
					detail: sprintf('recovery conflict: rule %s does not match journaled selector', conflicting_rule.raw)
				};
				print(sprintf('%J\n', err));
				exit(1);
			}

			let routes_res = get_live_routes(target_tbl);
			if (!routes_res.ok) {
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: routes_res.error
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
			let default_route = null;
			for (let r in routes_res.objects) {
				if (r.destination == 'default') {
					default_route = r;
					break;
				}
			}
			if (default_route != null && (default_route.dev != target_dev || default_route.via != target_via)) {
				let err = {
					status: 'error',
					code: 'route_conflict',
					detail: sprintf('recovery conflict: default route in table %s (%s) does not match journaled route (dev=%s via=%s)', target_tbl, default_route.raw, target_dev, target_via)
				};
				print(sprintf('%J\n', err));
				exit(1);
			}

			let route_matches = (default_route != null && default_route.dev == target_dev && default_route.via == target_via);
			let allow_no_route = (default_route == null && has_route_ops);
			if (matching_rule != null && (route_matches || allow_no_route)) {
				let rec = {
					index: target_idx,
					ifname: target_dev,
					rule: {
						priority: target_prio,
						fwmark: target_fwmark,
						table: target_tbl
					},
					route: {
						destination: 'default',
						dev: target_dev,
						via: target_via
					}
				};
				prior_state = upsert_state_entry(prior_state, rec);
				state_dirty = true;
			}
		}
	}

	if (state_dirty) {
		if (!write_json_atomic(state_file, prior_state)) {
			let err = {
				status: 'error',
				code: 'state_write_failed',
				detail: 'failed to write recovered route-state.json'
			};
			print(sprintf('%J\n', err));
			exit(1);
		}
	}

	if (stage == 'recover') {
		print(sprintf('%J\n', { status: 'ok', stage: 'recover', recovered: state_dirty }));
		exit(0);
	}
}

recover_pending_ownership();

// Conflict Detection across all desired interfaces (for precheck, add, and all)
if (stage == 'precheck' || stage == 'add' || stage == 'all') {
	for (let entry in desired_interfaces) {
		let target_prio = 30000 + int(entry.index);
		let target_tbl = `${entry.table}`;
		let target_mark_val = int(entry.mark, 0);
		let target_dev = entry.ifname;
		let target_via = entry.gateway || '';

		let owned = find_owned_record(prior_state, entry);

		if (!owned) {
			// No prior record owning this rule/route: inspect live system for conflicting rules
			for (let rule in live_rules) {
				if (rule.priority == target_prio) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('conflicting rule exists at priority %d: %s', target_prio, rule.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
				if (rule.table && rule.table == target_tbl) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('conflicting unowned rule exists for table %s: %s', target_tbl, rule.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
				if (rule.mark && rule.mark.val == target_mark_val) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('conflicting unowned rule exists with fwmark %d: %s', target_mark_val, rule.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
			}

			// Inspect live routes in target_tbl for existing default route
			let routes_res = get_live_routes(target_tbl);
			if (!routes_res.ok) {
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: routes_res.error
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
			for (let route in routes_res.objects) {
				if (route.destination == 'default') {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('conflicting default route exists in table %s: %s', target_tbl, route.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
			}
		}
		else {
			// Owned record exists: ensure live rule at target_prio matches owned rule and no unowned rule conflicts
			for (let rule in live_rules) {
				if (rule.priority == target_prio && (rule.table != target_tbl || !rule.mark || rule.mark.val != target_mark_val || rule.mark.mask != 0xffffffff)) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('rule at priority %d does not match owned rule: %s', target_prio, rule.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
				if (rule.priority != target_prio && rule.table == target_tbl) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('conflicting unowned rule for table %s at priority %d: %s', target_tbl, rule.priority, rule.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
				if (rule.priority != target_prio && rule.mark && rule.mark.val == target_mark_val) {
					let err = {
						status: 'error',
						code: 'route_conflict',
						detail: sprintf('conflicting unowned rule with fwmark %d at priority %d: %s', target_mark_val, rule.priority, rule.raw)
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
			}

			// Ensure any live default route in target_tbl matches either the prior owned route or the desired route
			let owned_dev = (owned.route && owned.route.dev) ? owned.route.dev : '';
			let owned_via = (owned.route && owned.route.via) ? owned.route.via : '';
			let routes_res = get_live_routes(target_tbl);
			if (!routes_res.ok) {
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: routes_res.error
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
			for (let route in routes_res.objects) {
				if (route.destination == 'default') {
					let matches_owned = (route.dev == owned_dev && route.via == owned_via);
					let matches_target = (route.dev == target_dev && route.via == target_via);
					if (!matches_owned && !matches_target) {
						let err = {
							status: 'error',
							code: 'route_conflict',
							detail: sprintf('default route in table %s does not match owned route: %s', target_tbl, route.raw)
						};
						print(sprintf('%J\n', err));
						exit(1);
					}
				}
			}
		}
	}
}

if (stage == 'precheck') {
	print(sprintf('%J\n', { status: 'ok', precheck: 'passed' }));
	exit(0);
}

let current_state = [ ...prior_state ];

// Step 1: Exact deletion of deprecated/removed owned entries (for prune or all)
if (stage == 'prune' || stage == 'all') {
	for (let owned in prior_state) {
		let still_desired = false;
		for (let entry in desired_interfaces) {
			if (find_owned_record([ owned ], entry) != null) {
				still_desired = true;
				break;
			}
		}
		if (!still_desired) {
			let del_rule_cmd = `${sh_quote(IP_BIN)} -4 rule del priority ${owned.rule.priority} fwmark ${owned.rule.fwmark} lookup ${owned.rule.table} 2>&1`;
			if (!ip_del_ignore_missing(del_rule_cmd)) {
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: `failed to delete ip rule: priority ${owned.rule.priority}`
				};
				print(sprintf('%J\n', err));
				exit(1);
			}

			let via_part = (owned.route && owned.route.via && owned.route.via != '') ? `via ${sh_quote(owned.route.via)} ` : '';
			let dev_part = (owned.route && owned.route.dev && owned.route.dev != '') ? `dev ${sh_quote(owned.route.dev)} ` : '';
			let del_route_cmd = `${sh_quote(IP_BIN)} -4 route del default ${via_part}${dev_part}table ${sh_quote(owned.rule.table)} 2>&1`;
			if (!ip_del_ignore_missing(del_route_cmd)) {
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: `failed to delete default route in table ${owned.rule.table}`
				};
				print(sprintf('%J\n', err));
				exit(1);
			}

			current_state = remove_state_entry(current_state, owned.index, owned.rule.table);
			if (!write_json_atomic(state_file, current_state)) {
				let err = {
					status: 'error',
					code: 'state_write_failed',
					detail: 'failed to write route-state.json after removing entry'
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
		}
	}
	if (stage == 'prune') {
		run_cmd(`${sh_quote(IP_BIN)} route flush cache 2>/dev/null || true`);
		print(sprintf('%J\n', { status: 'ok', stage: 'prune' }));
		exit(0);
	}
}

// Step 2: Install or update desired entries (for add or all)
if (stage == 'add' || stage == 'all') {
	let journal_file = `${STATE_DIR}/journal.json`;
	for (let entry in desired_interfaces) {
		let target_idx = int(entry.index);
		let target_prio = 30000 + target_idx;
		let target_tbl = `${entry.table}`;
		let target_mark = `${entry.mark}/0xffffffff`;
		let target_mark_val = int(entry.mark, 0);
		let target_dev = entry.ifname;
		let target_via = entry.gateway || '';

		if (fs.stat(journal_file)) {
			let j_raw = fs.readfile(journal_file);
			let j_doc = null;
			try {
				j_doc = json(j_raw);
			}
			catch (e) {
				j_doc = null;
			}
			if (type(j_doc) == 'object') {
				if (type(j_doc.route_ops) != 'array')
					j_doc.route_ops = [];
				let found_op = false;
				for (let op in j_doc.route_ops) {
					if (type(op) == 'object' && int(op.index) == target_idx && `${op.table}` == target_tbl) {
						found_op = true;
						break;
					}
				}
				if (!found_op) {
					push(j_doc.route_ops, {
						op: 'add',
						index: target_idx,
						ifname: target_dev,
						priority: target_prio,
						mark: target_mark_val,
						mask: 0xffffffff,
						fwmark: target_mark,
						table: target_tbl,
						dev: target_dev,
						gateway: target_via
					});
					if (!write_json_atomic(journal_file, j_doc)) {
						let err = {
							status: 'error',
							code: 'state_write_failed',
							detail: 'failed to update journal.json before mutating route/rule'
						};
						print(sprintf('%J\n', err));
						exit(1);
					}
				}
			}
		}

		let cur_rules_res = get_live_rules();
		if (!cur_rules_res.ok) {
			let err = {
				status: 'error',
				code: 'runtime_helper_failed',
				detail: cur_rules_res.error
			};
			print(sprintf('%J\n', err));
			exit(1);
		}
		let live_rules_cur = cur_rules_res.objects;
		let rule_found = false;
		for (let rule in live_rules_cur) {
			if (rule.priority == target_prio && rule.table == target_tbl &&
			    rule.mark && rule.mark.val == target_mark_val && rule.mark.mask == 0xffffffff) {
				rule_found = true;
				break;
			}
		}
		if (!rule_found) {
			let add_rule_cmd = `${sh_quote(IP_BIN)} -4 rule add priority ${target_prio} fwmark ${target_mark} lookup ${sh_quote(target_tbl)}`;
			let r_res = run_cmd(add_rule_cmd);
			if (r_res.rc != 0) {
				write_json_atomic(state_file, current_state);
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: `failed to add rule: ${r_res.stdout}`
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
		}

		let routes_res = get_live_routes(target_tbl);
		if (!routes_res.ok) {
			let err = {
				status: 'error',
				code: 'runtime_helper_failed',
				detail: routes_res.error
			};
			print(sprintf('%J\n', err));
			exit(1);
		}
		let live_tbl_routes = routes_res.objects;
		let default_route = null;
		for (let r in live_tbl_routes) {
			if (r.destination == 'default') {
				default_route = r;
				break;
			}
		}

		if (default_route) {
			if (default_route.dev != target_dev || default_route.via != target_via) {
				let old_via_part = (default_route.via && default_route.via != '') ? `via ${sh_quote(default_route.via)} ` : '';
				let old_dev_part = (default_route.dev && default_route.dev != '') ? `dev ${sh_quote(default_route.dev)} ` : '';
				if (!ip_del_ignore_missing(`${sh_quote(IP_BIN)} -4 route del default ${old_via_part}${old_dev_part}table ${sh_quote(target_tbl)} 2>&1`)) {
					write_json_atomic(state_file, current_state);
					let err = {
						status: 'error',
						code: 'runtime_helper_failed',
						detail: `failed to delete old default route in table ${target_tbl}`
					};
					print(sprintf('%J\n', err));
					exit(1);
				}

				let new_via_part = (target_via != '') ? `via ${sh_quote(target_via)} ` : '';
				let add_route_cmd = `${sh_quote(IP_BIN)} -4 route add default ${new_via_part}dev ${sh_quote(target_dev)} table ${sh_quote(target_tbl)}`;
				let rt_res = run_cmd(add_route_cmd);
				if (rt_res.rc != 0) {
					ip_del_ignore_missing(`${sh_quote(IP_BIN)} -4 rule del priority ${target_prio} fwmark ${target_mark} lookup ${sh_quote(target_tbl)} 2>&1`);
					current_state = remove_state_entry(current_state, target_idx, target_tbl);
					write_json_atomic(state_file, current_state);
					let err = {
						status: 'error',
						code: 'runtime_helper_failed',
						detail: `failed to add route: ${rt_res.stdout}`
					};
					print(sprintf('%J\n', err));
					exit(1);
				}
			}
		}
		else {
			let new_via_part = (target_via != '') ? `via ${sh_quote(target_via)} ` : '';
			let add_route_cmd = `${sh_quote(IP_BIN)} -4 route add default ${new_via_part}dev ${sh_quote(target_dev)} table ${sh_quote(target_tbl)}`;
			let rt_res = run_cmd(add_route_cmd);
			if (rt_res.rc != 0) {
				ip_del_ignore_missing(`${sh_quote(IP_BIN)} -4 rule del priority ${target_prio} fwmark ${target_mark} lookup ${sh_quote(target_tbl)} 2>&1`);
				current_state = remove_state_entry(current_state, target_idx, target_tbl);
				write_json_atomic(state_file, current_state);
				let err = {
					status: 'error',
					code: 'runtime_helper_failed',
					detail: `failed to add route: ${rt_res.stdout}`
				};
				print(sprintf('%J\n', err));
				exit(1);
			}
		}

		let owned_rec = {
			index: target_idx,
			ifname: target_dev,
			rule: {
				priority: target_prio,
				fwmark: target_mark,
				table: target_tbl
			},
			route: {
				destination: 'default',
				dev: target_dev,
				via: target_via
			}
		};
		current_state = upsert_state_entry(current_state, owned_rec);
		if (!write_json_atomic(state_file, current_state)) {
			let err = {
				status: 'error',
				code: 'state_write_failed',
				detail: 'failed to write route-state.json'
			};
			print(sprintf('%J\n', err));
			exit(1);
		}
	}
}

// Flush route cache
run_cmd(`${sh_quote(IP_BIN)} route flush cache 2>/dev/null || true`);

// Final state write verification
if (!write_json_atomic(state_file, current_state)) {
	let err = {
		status: 'error',
		code: 'state_write_failed',
		detail: 'failed to write route-state.json'
	};
	print(sprintf('%J\n', err));
	exit(1);
}

exit(0);
