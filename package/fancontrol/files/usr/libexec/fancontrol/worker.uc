#!/usr/bin/env ucode
// Copyright (C) 2026 ImmortalWrt.org
// Smart thermal control loop worker for unified fancontrol

'use strict';

import * as fs from 'fs';

unshift(REQUIRE_SEARCH_PATH, '/usr/share/ucode/*.uc');
let __sp = sourcepath(0, true);
if (__sp)
	unshift(REQUIRE_SEARCH_PATH, __sp + '/../../share/ucode/*.uc');
let __fc_lib = getenv('FANCTRL_UCODE_DIR');
if (__fc_lib)
	unshift(REQUIRE_SEARCH_PATH, __fc_lib + '/*.uc');

let fc = require('fancontrol.common');

const GEN = (length(ARGV) > 0 && ARGV[0] != '') ? ARGV[0] : 'boot';
const SELF_PID = fc.read_self_pid();
if (!SELF_PID) {
	warn('fancontrol worker: unable to resolve self PID\n');
	exit(1);
}

const SLEEP_TOTAL_MS = fc.clamp_int(getenv('FANCTRL_SLEEP_MS'), 20, 30000, 4000);

let active_fan = null;
let handover_manual_done = false;
let smart_loop_active = false;
let restore_called = false;
let restore_rc = 0;
let stop_requested = false;

function read_owner() {
	let op = fc.rpath(fc.OWNER_PATH);
	if (!fs.access(op, 'r'))
		return null;
	let raw = fs.readfile(op);
	if (!raw || trim(raw) == '')
		return null;
	try {
		let obj = json(raw);
		return (type(obj) == 'object') ? obj : null;
	}
	catch (e) {
		return null;
	}
}

function owner_matches_self() {
	let o = read_owner();
	return (o && `${o.pid}` == SELF_PID && `${o.generation}` == GEN);
}

function is_still_active_smart_owner() {
	let ready_val = fc.readfile_trim(fc.READY_PATH);
	if (ready_val == 'manual' || ready_val == 'kernel')
		return false;

	if (smart_loop_active) {
		let loaded = fc.load_config();
		if (loaded.exists && loaded.cfg.mode != 'smart')
			return false;
	}

	let pid_val = fc.readfile_trim(fc.PID_PATH);
	if (pid_val != SELF_PID)
		return false;

	if (fs.access(fc.rpath(fc.OWNER_PATH)) && !owner_matches_self())
		return false;

	if (ready_val != null && ready_val != SELF_PID)
		return false;

	return true;
}

function cleanup_runtime_files(keep_ready) {
	if (fc.readfile_trim(fc.PID_PATH) == SELF_PID)
		fc.unlink_safe(fc.PID_PATH);
	if (owner_matches_self())
		fc.unlink_safe(fc.OWNER_PATH);
	if (!keep_ready && fc.readfile_trim(fc.READY_PATH) == SELF_PID)
		fc.unlink_safe(fc.READY_PATH);
}

function restore_kernel_thermal(full_speed) {
	if (!is_still_active_smart_owner())
		return true;

	restore_called = true;
	let ok = true;

	if (full_speed) {
		if (!active_fan) {
			let l = fc.load_config();
			let res = fc.discover_fan(l.cfg);
			if (res.ok)
				active_fan = res;
		}
		if (!fc.write_verified('/sys/class/thermal/thermal_zone0/mode', 'disabled\n'))
			ok = false;
		if (active_fan && active_fan.enable_path) {
			if (!fc.write_verified(active_fan.enable_path, '1\n'))
				ok = false;
		}
		if (!active_fan || !fc.write_fan_logical(active_fan, 255))
			ok = false;
		if (!fc.write_verified('/sys/class/thermal/thermal_zone0/mode', 'enabled\n'))
			ok = false;
	}
	else {
		if (!fc.write_verified('/sys/class/thermal/thermal_zone0/mode', 'enabled\n'))
			ok = false;
		if (active_fan && active_fan.enable_path) {
			if (!fc.write_verified(active_fan.enable_path, '2\n'))
				ok = false;
		}
	}

	if (!ok) {
		restore_rc = 1;
		system(['logger', '-t', 'fancontrol', '-p', 'daemon.err', `restore_kernel_thermal failed (full_speed=${full_speed ? 1 : 0})`]);
		fc.write_atomic_verified(fc.ERROR_PATH, 'thermal_restore_failed\n');
		return false;
	}

	restore_rc = 0;
	let ready_val = fc.readfile_trim(fc.READY_PATH);
	if (ready_val && ready_val != SELF_PID) {
		fc.unlink_safe(fc.ERROR_PATH);
		return true;
	}
	fc.unlink_safe(fc.READY_PATH);
	fc.unlink_safe(fc.ERROR_PATH);
	return true;
}

function finish_clean(sig_exit) {
	if (!handover_manual_done && is_still_active_smart_owner()) {
		if (!restore_called) {
			if (!restore_kernel_thermal(false)) {
				cleanup_runtime_files(true);
				exit(1);
			}
		}
		else if (restore_rc != 0) {
			cleanup_runtime_files(true);
			exit(1);
		}
	}

	cleanup_runtime_files(handover_manual_done || restore_rc != 0);
	exit(sig_exit ? 0 : (restore_rc != 0 ? 1 : 0));
}

signal('SIGTERM', function() { stop_requested = true; });
signal('SIGINT', function() { stop_requested = true; });

// Check existing lock PID
let old_pid = fc.readfile_trim(fc.PID_PATH);
if (old_pid && old_pid != SELF_PID && fc.is_confirmed_worker_pid(old_pid)) {
	warn(`fancontrol worker already running as PID ${old_pid}\n`);
	exit(1);
}

fc.unlink_safe(fc.READY_PATH);
fc.unlink_safe(fc.OWNER_PATH);

let start_ticks = fc.proc_start_ticks(SELF_PID);
if (!start_ticks)
	exit(1);

if (!fc.write_atomic_verified(fc.PID_PATH, `${SELF_PID}\n`))
	exit(1);

if (!fc.write_atomic_verified(fc.OWNER_PATH, sprintf('%J\n', { generation: GEN, pid: int(SELF_PID), start_ticks: start_ticks }))) {
	cleanup_runtime_files(false);
	exit(1);
}

let cfg_state = fc.load_config();
let cfg = cfg_state.cfg;

let fan_res = fc.discover_fan(cfg);
if (!fan_res.ok || !fc.check_thermal_rw()) {
	restore_kernel_thermal(false);
	cleanup_runtime_files(true);
	exit(1);
}
active_fan = fan_res;

let init_t = fc.read_temps(cfg);
if (!init_t.ok) {
	restore_kernel_thermal(true);
	cleanup_runtime_files(true);
	exit(1);
}

let init_pwm;
if (cfg.mode == 'turbo')
	init_pwm = 255;
else if (cfg.mode == 'normal')
	init_pwm = 192;
else if (cfg.mode == 'low')
	init_pwm = 128;
else if (cfg.mode == 'stop')
	init_pwm = 64;
else if (cfg.mode == 'stepless')
	init_pwm = cfg.stepless_pwm;
else
	init_pwm = fc.compute_smart_pwm(init_t.active_mC, 0, cfg);

// Step 1: Write initial PWM, disable kernel thermal, switch pwm_enable=1, verify readback
if (!fc.write_fan_logical(active_fan, init_pwm)) {
	restore_kernel_thermal(false);
	cleanup_runtime_files(true);
	exit(1);
}

if (!fc.write_verified('/sys/class/thermal/thermal_zone0/mode', 'disabled\n')) {
	restore_kernel_thermal(false);
	cleanup_runtime_files(true);
	exit(1);
}

if (active_fan.enable_path) {
	if (!fc.write_verified(active_fan.enable_path, '1\n')) {
		restore_kernel_thermal(false);
		cleanup_runtime_files(true);
		exit(1);
	}
}

if (!fc.write_fan_logical(active_fan, init_pwm)) {
	restore_kernel_thermal(false);
	cleanup_runtime_files(true);
	exit(1);
}

if (!fc.write_atomic_verified(fc.READY_PATH, `${SELF_PID}\n`)) {
	restore_kernel_thermal(false);
	cleanup_runtime_files(true);
	exit(1);
}
fc.unlink_safe(fc.ERROR_PATH);

if (cfg.mode != 'smart') {
	handover_manual_done = true;
	finish_clean(false);
}

// Step 2: Zero-flash-write smart control loop
smart_loop_active = true;
let cur_pwm = init_pwm;

while (!stop_requested) {
	let tinfo = fc.read_temps(cfg);
	if (!tinfo.ok) {
		restore_kernel_thermal(true);
		cleanup_runtime_files(true);
		exit(1);
	}

	let target_pwm = fc.compute_smart_pwm(tinfo.active_mC, cur_pwm, cfg);
	let hw_pwm = fc.read_fan_logical(active_fan, cur_pwm);
	if (target_pwm != cur_pwm || hw_pwm != target_pwm) {
		if (!fc.write_fan_logical(active_fan, target_pwm)) {
			restore_kernel_thermal(true);
			cleanup_runtime_files(true);
			exit(1);
		}
	}
	cur_pwm = target_pwm;

	let slept = 0;
	while (!stop_requested && slept < SLEEP_TOTAL_MS) {
		let step = (SLEEP_TOTAL_MS - slept > 50) ? 50 : (SLEEP_TOTAL_MS - slept);
		sleep(step);
		slept += step;
	}
}

finish_clean(true);
