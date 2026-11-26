// Copyright (C) 2026 ImmortalWrt.org
// Shared ucode fancontrol hardware, config, sensor, and curve engine

'use strict';

import * as fs from 'fs';

let libubus = null;
try {
	libubus = require('ubus');
}
catch (e) {
	libubus = null;
}

const ROOT = getenv('FANCTRL_ROOT') || '';
const CONFIG_PATH = '/etc/config/fancontrol';
const PID_PATH = '/var/run/fancontrol.pid';
const OWNER_PATH = '/var/run/fancontrol.owner.json';
const READY_PATH = '/var/run/fancontrol.ready';
const ERROR_PATH = '/var/run/fancontrol.error';
const WPS_ACTIVE_PATH = '/tmp/fancontrol.wps_active';
const LUCIMSG_PATH = '/tmp/fancontrol.lucimsg';

const MODE_TABLE = {
	stop:     { pwm: 64,   label: 'Silent Mode' },
	low:      { pwm: 128,  label: 'Low Speed Mode' },
	normal:   { pwm: 192,  label: 'Normal Mode' },
	turbo:    { pwm: 255,  label: 'Turbo Mode' },
	smart:    { pwm: null, label: 'Smart Thermal Mode' },
	kernel:   { pwm: null, label: 'Kernel Thermal Mode' },
	stepless: { pwm: null, label: 'Stepless Mode' }
};

const VALID_SOURCES = {
	cpu: true,
	modem: true,
	max: true
};

const VALID_DRIVERS = {
	auto: true,
	hwmon_pwm: true,
	hwmon_gpio: true,
	gpio_duty: true,
	sysfs_pwm: true
};

const VALID_INVERTED = {
	auto: true,
	'0': true,
	'1': true
};

const DEFAULT_CONFIG = {
	mode: 'smart',
	stepless_pwm: 192,
	temp_source: 'cpu',
	driver: 'auto',
	inverted: 'auto',
	hwmon_path: '',
	pwm_chip: '0',
	pwm_channel: '0',
	pwm_period: 40000,
	start_temp: 40,
	full_temp: 75,
	min_pwm: 64,
	max_pwm: 255,
	hysteresis: 2,
	slew_rate: 12,
	wps_turbo: 0,
	board_init: 0,
	migrated: 0
};

function rpath(path) {
	if (!ROOT || ROOT == '')
		return path;
	if (index(path, ROOT) === 0)
		return path;
	return ROOT + path;
};

function ensure_parent_dir(full_path) {
	let slash = rindex(full_path, '/');
	if (slash <= 0)
		return;
	let dir = substr(full_path, 0, slash);
	let parts = split(dir, '/');
	let cur = '';
	for (let p in parts) {
		if (p == '')
			continue;
		cur += '/' + p;
		if (!fs.stat(cur))
			fs.mkdir(cur, 0755);
	}
};

function readfile_trim(path) {
	let fp = rpath(path);
	if (!fs.access(fp, 'r'))
		return null;
	let s = fs.readfile(fp);
	return (s != null) ? trim(s) : null;
};

function write_verified(path, body) {
	let fp = rpath(path);
	ensure_parent_dir(fp);
	let n;
	try {
		n = fs.writefile(fp, body);
	}
	catch (e) {
		return false;
	}
	if (n == null || n != length(body))
		return false;

	if (!fs.access(fp, 'r'))
		return true;

	let got = fs.readfile(fp);
	return got != null && trim(got) == trim(body);
};

function write_writeonly(path, body) {
	let fp = rpath(path);
	ensure_parent_dir(fp);
	let n;
	try {
		n = fs.writefile(fp, body);
	}
	catch (e) {
		return false;
	}
	return n != null && n == length(body);
};

function write_atomic_verified(path, body) {
	let fp = rpath(path);
	ensure_parent_dir(fp);

	if (fs.access(fp) && !fs.access(fp, 'w'))
		return false;

	let tmp_fp = fp + '.tmp';
	let n;
	try {
		n = fs.writefile(tmp_fp, body);
	}
	catch (e) {
		return false;
	}

	if (n == null || n != length(body)) {
		if (fs.access(tmp_fp))
			fs.unlink(tmp_fp);
		return false;
	}

	let got_tmp = fs.readfile(tmp_fp);
	if (got_tmp == null || trim(got_tmp) != trim(body)) {
		if (fs.access(tmp_fp))
			fs.unlink(tmp_fp);
		return false;
	}

	if (!fs.rename(tmp_fp, fp)) {
		if (fs.access(tmp_fp))
			fs.unlink(tmp_fp);
		return false;
	}

	let got = fs.readfile(fp);
	return got != null && trim(got) == trim(body);
};

function path_absent_verified(fp) {
	return fs.lstat(fp) == null && fs.error() == 'No such file or directory';
};

function unlink_safe(path) {
	let fp = rpath(path);
	if (fs.access(fp))
		fs.unlink(fp);
};

function unlink_verified(path) {
	let fp = rpath(path);
	if (path_absent_verified(fp))
		return true;
	fs.unlink(fp);
	return path_absent_verified(fp);
};

function restore_file_or_unlink(path, old_body) {
	let fp = rpath(path);
	if (old_body != null) {
		let st = fs.lstat(fp);
		if (st && st.type == 'file' && fs.readfile(fp) == old_body)
			return true;
		return write_atomic_verified(path, old_body);
	}
	return unlink_verified(path);
};

function clamp_int(val, lo, hi, fallback) {
	if (val == null || !match(`${val}`, /^-?\d+$/))
		return fallback;
	let n = int(val);
	if (n < lo)
		return lo;
	if (n > hi)
		return hi;
	return n;
};

function pwm_to_percent(pwm) {
	return int((pwm / 255.0) * 100.0 + 0.5);
};

function normalize_config(raw_cfg) {
	let cfg = {};
	for (let k, v in DEFAULT_CONFIG)
		cfg[k] = v;

	if (type(raw_cfg) == 'object') {
		if (raw_cfg.mode in MODE_TABLE)
			cfg.mode = raw_cfg.mode;
		if (raw_cfg.temp_source in VALID_SOURCES)
			cfg.temp_source = raw_cfg.temp_source;
		if (raw_cfg.driver in VALID_DRIVERS)
			cfg.driver = raw_cfg.driver;
		if (`${raw_cfg.inverted}` in VALID_INVERTED)
			cfg.inverted = `${raw_cfg.inverted}`;
		if (type(raw_cfg.hwmon_path) == 'string') {
			let hp = trim(raw_cfg.hwmon_path);
			if (hp == '' || (index(hp, chr(0)) < 0 && match(hp, /^\/[A-Za-z0-9_\/\.\-]+$/)))
				cfg.hwmon_path = hp;
		}
		if (raw_cfg.pwm_chip != null && match(`${raw_cfg.pwm_chip}`, /^\d+$/))
			cfg.pwm_chip = `${raw_cfg.pwm_chip}`;
		if (raw_cfg.pwm_channel != null && match(`${raw_cfg.pwm_channel}`, /^\d+$/))
			cfg.pwm_channel = `${raw_cfg.pwm_channel}`;
		cfg.pwm_period = clamp_int(raw_cfg.pwm_period, 1000, 10000000, cfg.pwm_period);
		cfg.stepless_pwm = clamp_int(raw_cfg.stepless_pwm, 0, 255, cfg.stepless_pwm);
		cfg.start_temp = clamp_int(raw_cfg.start_temp, 20, 95, cfg.start_temp);
		cfg.full_temp = clamp_int(raw_cfg.full_temp, 30, 115, cfg.full_temp);
		if (cfg.full_temp <= cfg.start_temp)
			cfg.full_temp = (cfg.start_temp + 10 <= 115) ? (cfg.start_temp + 10) : 115;
		cfg.min_pwm = clamp_int(raw_cfg.min_pwm, 0, 255, cfg.min_pwm);
		cfg.max_pwm = clamp_int(raw_cfg.max_pwm, 1, 255, cfg.max_pwm);
		if (cfg.max_pwm < cfg.min_pwm)
			cfg.max_pwm = cfg.min_pwm;
		cfg.hysteresis = clamp_int(raw_cfg.hysteresis, 0, 15, cfg.hysteresis);
		cfg.slew_rate = clamp_int(raw_cfg.slew_rate, 1, 255, cfg.slew_rate);
		cfg.wps_turbo = (`${raw_cfg.wps_turbo}` == '1') ? 1 : 0;
		cfg.board_init = (`${raw_cfg.board_init}` == '1') ? 1 : 0;
		cfg.migrated = (`${raw_cfg.migrated}` == '1') ? 1 : 0;
	}
	return cfg;
};

function load_config() {
	let fp = rpath(CONFIG_PATH);
	let exists = fs.access(fp, 'r');
	let raw_text = exists ? fs.readfile(fp) : null;
	let parsed = {};
	let in_main = false;

	if (raw_text != null) {
		for (let line in split(raw_text, /\r?\n/)) {
			let t = trim(line);
			if (t == '' || substr(t, 0, 1) == '#')
				continue;
			if (match(t, /^config[ \t]+[^ \t]+/)) {
				let cm = match(t, /^config[ \t]+[^ \t]+[ \t]+['"]?([^'" \t]+)['"]?$/);
				let sec_name = cm ? cm[1] : null;
				in_main = (!sec_name || sec_name == 'main' || sec_name == 'fancontrol' || sec_name == 'cooling_control');
				continue;
			}
			if (!in_main)
				continue;
			let om = match(t, /^option[ \t]+([A-Za-z0-9_]+)[ \t]+(.*)$/);
			if (om) {
				let key = om[1];
				let val = trim(om[2]);
				if (length(val) >= 2 &&
				    ((substr(val, 0, 1) == "'" && substr(val, -1) == "'") ||
				     (substr(val, 0, 1) == '"' && substr(val, -1) == '"'))) {
					val = substr(val, 1, length(val) - 2);
				}
				parsed[key] = val;
			}
		}
	}

	return {
		exists: exists,
		raw: raw_text,
		cfg: normalize_config(parsed)
	};
};

function format_config(cfg_in) {
	let cfg = normalize_config(cfg_in);
	let order = [
		'mode', 'stepless_pwm', 'temp_source',
		'driver', 'inverted', 'hwmon_path',
		'pwm_chip', 'pwm_channel', 'pwm_period',
		'start_temp', 'full_temp', 'min_pwm', 'max_pwm',
		'hysteresis', 'slew_rate', 'wps_turbo',
		'board_init', 'migrated'
	];
	let out = "config fancontrol 'main'\n";
	for (let k in order) {
		let v = replace(`${cfg[k]}`, /'/g, '');
		out += `\toption ${k} '${v}'\n`;
	}
	return out;
};

function proc_start_ticks(pid) {
	let pid_str = (pid != null) ? trim(`${pid}`) : '';
	if (!match(pid_str, /^[0-9]+$/))
		return null;

	let stat = null;
	let p_stat = rpath('/proc/' + pid_str + '/stat');
	if (fs.access(p_stat, 'r'))
		stat = fs.readfile(p_stat);
	else if (fs.access('/proc/' + pid_str + '/stat', 'r'))
		stat = fs.readfile('/proc/' + pid_str + '/stat');

	if (stat == null)
		return null;

	let rp = rindex(stat, ')');
	if (rp < 0)
		return null;

	let rest = trim(substr(stat, rp + 1));
	if (rest == '')
		return null;

	let parts = split(rest, /\s+/);
	if (length(parts) <= 19)
		return null;

	let ticks = parts[19];
	if (!ticks || !match(`${ticks}`, /^[0-9]+$/))
		return null;
	return `${ticks}`;
};

function read_self_pid() {
	let link = fs.readlink('/proc/self');
	if (link && match(link, /^[0-9]+$/))
		return link;
	let stat = fs.readfile('/proc/self/stat');
	if (stat) {
		let sp = index(stat, ' ');
		if (sp > 0) {
			let pid = trim(substr(stat, 0, sp));
			if (match(pid, /^[0-9]+$/))
				return pid;
		}
	}
	return null;
};

function is_confirmed_worker_pid(pid_str) {
	if (!pid_str || !match(`${pid_str}`, /^\d+$/))
		return false;

	if (system(['kill', '-0', `${pid_str}`]) != 0)
		return false;

	let stat = null;
	let p_stat = rpath('/proc/' + pid_str + '/stat');
	if (fs.access(p_stat, 'r'))
		stat = fs.readfile(p_stat);
	else if (fs.access('/proc/' + pid_str + '/stat', 'r'))
		stat = fs.readfile('/proc/' + pid_str + '/stat');

	if (stat != null && index(stat, ') Z') >= 0)
		return false;

	let cmdline = null;
	let p_cmd = rpath('/proc/' + pid_str + '/cmdline');
	if (fs.access(p_cmd, 'r'))
		cmdline = fs.readfile(p_cmd);
	else if (fs.access('/proc/' + pid_str + '/cmdline', 'r'))
		cmdline = fs.readfile('/proc/' + pid_str + '/cmdline');

	if (cmdline != null && index(cmdline, 'worker.uc') >= 0)
		return true;

	let comm = null;
	let p_comm = rpath('/proc/' + pid_str + '/comm');
	if (fs.access(p_comm, 'r'))
		comm = fs.readfile(p_comm);
	else if (fs.access('/proc/' + pid_str + '/comm', 'r'))
		comm = fs.readfile('/proc/' + pid_str + '/comm');

	if (comm != null && index(comm, 'worker.uc') >= 0)
		return true;

	return false;
};

function detect_dt_inverted(hw_dir) {
	let candidates = [];
	if (hw_dir)
		push(candidates, hw_dir + '/device/of_node/cooling-levels');
	push(candidates, rpath('/proc/device-tree/pwm-fan/cooling-levels'));
	push(candidates, rpath('/proc/device-tree/fan/cooling-levels'));

	for (let p in candidates) {
		if (!fs.access(p, 'r'))
			continue;
		let raw = fs.readfile(p);
		if (raw == null)
			continue;
		let n = length(raw);
		if (n < 8 || (n % 4) != 0)
			continue;
		let first_val = (ord(raw, 0) * 16777216) + (ord(raw, 1) * 65536) + (ord(raw, 2) * 256) + ord(raw, 3);
		let o = n - 4;
		let last_val = (ord(raw, o) * 16777216) + (ord(raw, o + 1) * 65536) + (ord(raw, o + 2) * 256) + ord(raw, o + 3);
		return first_val > last_val;
	}
	return false;
};

function resolve_inverted(cfg, hw_dir) {
	if (`${cfg.inverted}` == '1')
		return true;
	if (`${cfg.inverted}` == '0')
		return false;
	return detect_dt_inverted(hw_dir);
};

function discover_fan(cfg_in) {
	let cfg = normalize_config(cfg_in);

	if (cfg.hwmon_path && cfg.hwmon_path != '') {
		let p = rpath(cfg.hwmon_path);
		if (!fs.access(p, 'r') || !fs.access(p, 'w'))
			return { ok: false, error: 'Configured hwmon_path is unavailable or not writable' };
		let sl = rindex(p, '/');
		let dir = (sl > 0) ? substr(p, 0, sl) : null;
		let rpm_p = (dir && fs.access(dir + '/fan1_input', 'r')) ? (dir + '/fan1_input') : null;
		let en_p = fs.access(p + '_enable', 'w') ? (p + '_enable') : null;
		return {
			ok: true,
			kind: 'hwmon_pwm',
			driver_label: 'hwmon-pwmfan (custom)',
			path: p,
			enable_path: en_p,
			rpm_path: rpm_p,
			max_val: 255,
			inverted: resolve_inverted(cfg, dir),
			writable_only: false
		};
	}

	if (cfg.driver == 'auto' || cfg.driver == 'hwmon_pwm') {
		let names = fs.glob(rpath('/sys/class/hwmon/hwmon*/name')) || [];
		let found_dirs = [];
		for (let np in names) {
			if (!fs.access(np, 'r'))
				continue;
			let nm = trim(fs.readfile(np) || '');
			if (nm != 'pwmfan')
				continue;
			let sl = rindex(np, '/');
			if (sl > 0)
				push(found_dirs, substr(np, 0, sl));
		}

		if (length(found_dirs) > 1)
			return { ok: false, error: 'Ambiguous multiple pwmfan hwmon devices' };

		if (length(found_dirs) == 1) {
			let dir = found_dirs[0];
			let node = dir + '/pwm1';
			if (!fs.access(node, 'r') || !fs.access(node, 'w'))
				return { ok: false, error: 'pwmfan pwm1 sysfs node is not read-writable' };
			let rpm_p = fs.access(dir + '/fan1_input', 'r') ? (dir + '/fan1_input') : null;
			let en_p = fs.access(node + '_enable', 'w') ? (node + '_enable') : null;
			return {
				ok: true,
				kind: 'hwmon_pwm',
				driver_label: 'hwmon-pwmfan',
				path: node,
				enable_path: en_p,
				rpm_path: rpm_p,
				max_val: 255,
				inverted: resolve_inverted(cfg, dir),
				writable_only: false
			};
		}

		if (cfg.driver == 'hwmon_pwm')
			return { ok: false, error: 'No pwmfan hwmon device found' };
	}

	if (cfg.driver == 'auto' || cfg.driver == 'hwmon_gpio') {
		let cdevs = fs.glob(rpath('/sys/class/thermal/cooling_device*/type')) || [];
		let found_cdev = null;
		for (let tp in cdevs) {
			if (!fs.access(tp, 'r'))
				continue;
			let tname = trim(fs.readfile(tp) || '');
			if (tname != 'gpio-fan')
				continue;
			let sl = rindex(tp, '/');
			let cdir = (sl > 0) ? substr(tp, 0, sl) : null;
			if (!cdir)
				continue;
			let cur_p = cdir + '/cur_state';
			let max_p = cdir + '/max_state';
			if (!fs.access(cur_p, 'r') || !fs.access(cur_p, 'w') || !fs.access(max_p, 'r'))
				continue;
			let ms = clamp_int(trim(fs.readfile(max_p) || ''), 1, 255, 0);
			if (ms <= 0)
				continue;
			found_cdev = { dir: cdir, cur_p: cur_p, max_state: ms };
			break;
		}
		if (found_cdev) {
			return {
				ok: true,
				kind: 'hwmon_gpio',
				driver_label: 'gpio-fan (cooling_device)',
				path: found_cdev.cur_p,
				enable_path: null,
				rpm_path: null,
				max_val: found_cdev.max_state,
				inverted: (`${cfg.inverted}` == '1'),
				writable_only: false
			};
		}
		if (cfg.driver == 'hwmon_gpio')
			return { ok: false, error: 'No gpio-fan cooling_device found' };
	}

	if (cfg.driver == 'auto' || cfg.driver == 'gpio_duty') {
		let dp = rpath('/sys/kernel/duty_cycle');
		if (fs.access(dp, 'w')) {
			return {
				ok: true,
				kind: 'gpio_duty',
				driver_label: 'Airpi-gpio-fan',
				path: dp,
				enable_path: null,
				rpm_path: null,
				max_val: 255,
				inverted: (`${cfg.inverted}` == '1'),
				writable_only: !fs.access(dp, 'r')
			};
		}
		if (cfg.driver == 'gpio_duty')
			return { ok: false, error: '/sys/kernel/duty_cycle not available' };
	}

	if (cfg.driver == 'sysfs_pwm') {
		let base = rpath(`/sys/class/pwm/pwmchip${cfg.pwm_chip}/pwm${cfg.pwm_channel}`);
		let dc = base + '/duty_cycle';
		if (fs.access(dc, 'w')) {
			let per_p = base + '/period';
			let per = cfg.pwm_period;
			if (fs.access(per_p, 'r')) {
				let cur_p = trim(fs.readfile(per_p) || '');
				if (match(cur_p, /^\d+$/) && int(cur_p) >= 1000)
					per = int(cur_p);
			}
			return {
				ok: true,
				kind: 'sysfs_pwm',
				driver_label: `sysfs-pwm (pwm${cfg.pwm_channel})`,
				path: dc,
				enable_path: fs.access(base + '/enable', 'w') ? (base + '/enable') : null,
				rpm_path: null,
				max_val: per,
				inverted: (`${cfg.inverted}` == '1'),
				writable_only: !fs.access(dc, 'r')
			};
		}
		return { ok: false, error: `Sysfs PWM path unavailable: ${dc}` };
	}

	return { ok: false, error: 'No supported fan hardware found' };
};

function logical_to_raw(logical_pwm, fan) {
	let l = clamp_int(logical_pwm, 0, 255, 0);
	let eff = fan.inverted ? (255 - l) : l;
	if (fan.kind == 'hwmon_gpio')
		return int((eff * fan.max_val) / 255.0 + 0.5);
	if (fan.kind == 'sysfs_pwm')
		return int((eff * fan.max_val) / 255.0 + 0.5);
	return eff;
};

function raw_to_logical(raw_val, fan) {
	let r = clamp_int(raw_val, 0, fan.max_val || 255, 0);
	let eff;
	if (fan.kind == 'hwmon_gpio' || fan.kind == 'sysfs_pwm')
		eff = int((r * 255.0) / (fan.max_val || 1) + 0.5);
	else
		eff = r;
	eff = clamp_int(eff, 0, 255, 0);
	return fan.inverted ? (255 - eff) : eff;
};

function write_fan_logical(fan, logical_pwm) {
	let raw = logical_to_raw(logical_pwm, fan);
	let body = `${raw}\n`;
	if (fan.writable_only)
		return write_writeonly(fan.path, body);
	return write_verified(fan.path, body);
};

function read_fan_logical(fan, fallback) {
	if (!fan || !fan.path || !fs.access(fan.path, 'r'))
		return fallback;
	let raw = trim(fs.readfile(fan.path) || '');
	if (!match(raw, /^-?\d+$/))
		return fallback;
	return raw_to_logical(int(raw), fan);
};

function read_fan_raw(fan) {
	if (!fan || !fan.path || !fs.access(fan.path, 'r'))
		return null;
	let raw = trim(fs.readfile(fan.path) || '');
	if (!match(raw, /^-?\d+$/))
		return null;
	return int(raw);
};

function write_fan_raw_exact(fan, raw_int) {
	if (!fan || !fan.path)
		return false;
	let body = `${int(raw_int)}\n`;
	if (fan.writable_only)
		return write_writeonly(fan.path, body);
	return write_verified(fan.path, body);
};

function read_fan_rpm(fan) {
	if (!fan || !fan.rpm_path || !fs.access(fan.rpm_path, 'r'))
		return null;
	let raw = trim(fs.readfile(fan.rpm_path) || '');
	if (!match(raw, /^\d+$/))
		return null;
	return int(raw);
};

function check_thermal_rw() {
	let p = rpath('/sys/class/thermal/thermal_zone0/mode');
	return fs.access(p, 'r') && fs.access(p, 'w');
};

function read_cpu_temp_mC() {
	let raw = readfile_trim('/sys/class/thermal/thermal_zone0/temp');
	if (raw == null || !match(raw, /^-?\d+(\.\d+)?$/))
		return null;
	let n = int(+raw);
	if (n <= 0)
		return null;
	let mC = (n < 200) ? (n * 1000) : n;
	if (mC <= 0 || mC > 150000)
		return null;
	return mC;
};

function parse_qtemp_max(output) {
	let best = null;
	for (let line in split(output || '', /\r?\n/)) {
		if (index(line, '+QTEMP:') < 0)
			continue;
		let m = match(line, /([+-]?[0-9]+(\.[0-9]+)?)"?\s*$/);
		if (!m)
			continue;
		let v = +m[1];
		if (v > 0 && v <= 130 && (best == null || v > best))
			best = v;
	}
	return (best != null) ? int(best * 1000.0 + 0.5) : null;
};

function parse_chiptemp_val(output) {
	for (let line in split(output || '', /\r?\n/)) {
		if (index(line, 'CHIPTEMP') < 0)
			continue;
		let fields = split(line, ',');
		let cand = null;
		if (length(fields) >= 9) {
			cand = trim(fields[8]);
		}
		else {
			let m = match(line, /CHIPTEMP[^:]*:\s*([0-9.+\-]+)/);
			if (m)
				cand = trim(m[1]);
			else if (length(fields) > 0)
				cand = trim(fields[length(fields) - 1]);
		}
		if (cand != null && match(cand, /^-?\d+(\.\d+)?$/)) {
			let num = +cand;
			if (num > 10000)
				return int(num);
			if (num > 150)
				return int(num * 100.0 + 0.5);
			if (num > 0)
				return int(num * 1000.0 + 0.5);
		}
	}
	return null;
};

function detect_at_ports() {
	let ports = [];
	let seen = {};
	let add = function(p) {
		if (!p || p == '' || seen[p] || !match(p, /^\/[A-Za-z0-9_\/\.\-]+$/))
			return;
		let fp = rpath(p);
		if (fs.access(fp)) {
			seen[p] = true;
			push(ports, fp);
		}
	};
	let qcfg = rpath('/etc/config/qmodem');
	if (fs.access(qcfg, 'r')) {
		let txt = fs.readfile(qcfg) || '';
		for (let line in split(txt, /\r?\n/)) {
			let m = match(line, /option[ \t]+at_port[ \t]+['"]?([^'" \t]+)['"]?/);
			if (m)
				add(m[1]);
		}
	}
	for (let p in ['/dev/ttyUSB3', '/dev/ttyUSB2', '/dev/ttyUSB1', '/dev/mhi_DUN'])
		add(p);
	return ports;
};

function query_at_daemon(port, cmd) {
	if (ROOT != '' || !libubus)
		return null;
	let b = libubus.connect(null, 3);
	if (!b)
		return null;
	let reply = b.call('at-daemon', 'sendat', { at_port: port, at_cmd: cmd, timeout: 2 });
	let err = libubus.error();
	if (err != null || type(reply) != 'object')
		return null;
	return reply.response || reply.result || null;
};

function run_cmd_capture_argv(argv) {
	if (type(argv) != 'array' || length(argv) == 0)
		return '';
	let quoted = [];
	for (let a in argv)
		push(quoted, "'" + replace(`${a}`, "'", "'\\''") + "'");
	let pipe = fs.popen(join(' ', quoted) + ' 2>/dev/null', 'r');
	if (!pipe)
		return '';
	let out = pipe.read('all') || '';
	pipe.close();
	return out;
};

function read_modem_temp_mC() {
	let mock_file = getenv('FANCTRL_MODEM_TEMP_FILE');
	if (mock_file && mock_file != '' && fs.access(mock_file, 'r')) {
		let raw = trim(fs.readfile(mock_file) || '');
		if (match(raw, /^-?\d+(\.\d+)?$/)) {
			let n = int(+raw);
			if (n > 0) {
				let mC = (n < 200) ? (n * 1000) : n;
				if (mC > 0 && mC <= 150000)
					return mC;
			}
		}
		return null;
	}

	for (let p in detect_at_ports()) {
		let dq = parse_qtemp_max(query_at_daemon(p, 'AT+QTEMP'));
		if (dq != null)
			return dq;
		let dc = parse_chiptemp_val(query_at_daemon(p, 'AT^CHIPTEMP?'));
		if (dc != null)
			return dc;
	}

	let sms_bin = fs.access(rpath('/usr/bin/sms_tool'), 'x')
		? rpath('/usr/bin/sms_tool')
		: ((ROOT == '' && fs.access('/usr/bin/sms_tool', 'x')) ? '/usr/bin/sms_tool' : null);
	if (sms_bin) {
		for (let p in detect_at_ports()) {
			let q_val = parse_qtemp_max(run_cmd_capture_argv([sms_bin, '-d', p, 'at', 'AT+QTEMP']));
			if (q_val != null)
				return q_val;
			let c_val = parse_chiptemp_val(run_cmd_capture_argv([sms_bin, '-d', p, 'at', 'AT^CHIPTEMP?']));
			if (c_val != null)
				return c_val;
		}
	}

	let sendat_bin = fs.access(rpath('/usr/bin/sendat'), 'x')
		? rpath('/usr/bin/sendat')
		: ((ROOT == '' && fs.access('/usr/bin/sendat', 'x')) ? '/usr/bin/sendat' : null);
	if (sendat_bin) {
		for (let idx in ['1', '2']) {
			let q_val = parse_qtemp_max(run_cmd_capture_argv([sendat_bin, idx, 'AT+QTEMP']));
			if (q_val != null)
				return q_val;
		}
		let c_val = parse_chiptemp_val(run_cmd_capture_argv([sendat_bin, '1', 'AT^CHIPTEMP?']));
		if (c_val != null)
			return c_val;
	}

	return null;
};

function mC_to_C(mC) {
	if (mC == null)
		return null;
	return int((mC / 100.0) + 0.5) / 10.0;
};

function read_temps(cfg_in) {
	let cfg = normalize_config(cfg_in);
	let cpu_mC = read_cpu_temp_mC();
	let modem_mC = (cfg.temp_source == 'modem' || cfg.temp_source == 'max') ? read_modem_temp_mC() : null;

	let active_mC = null;
	let source_used = cfg.temp_source;

	if (cfg.temp_source == 'cpu') {
		active_mC = cpu_mC;
		source_used = 'cpu';
	}
	else if (cfg.temp_source == 'modem') {
		if (modem_mC != null) {
			active_mC = modem_mC;
			source_used = 'modem';
		}
		else {
			active_mC = cpu_mC;
			source_used = 'cpu';
		}
	}
	else {
		if (cpu_mC != null && modem_mC != null) {
			if (modem_mC >= cpu_mC) {
				active_mC = modem_mC;
				source_used = 'modem';
			}
			else {
				active_mC = cpu_mC;
				source_used = 'cpu';
			}
		}
		else if (cpu_mC != null) {
			active_mC = cpu_mC;
			source_used = 'cpu';
		}
		else {
			active_mC = modem_mC;
			source_used = 'modem';
		}
	}

	return {
		ok: active_mC != null && active_mC > 0 && active_mC <= 150000,
		active_mC: active_mC,
		active_temp: mC_to_C(active_mC),
		cpu_temp: mC_to_C(cpu_mC),
		modem_temp: mC_to_C(modem_mC),
		source_used: source_used
	};
};

function compute_smart_pwm(temp_mC, prev_pwm, cfg_in) {
	let cfg = normalize_config(cfg_in);
	let t = int(temp_mC);
	let prev = (prev_pwm != null) ? int(prev_pwm) : 0;
	let start_mC = cfg.start_temp * 1000;
	let full_mC = cfg.full_temp * 1000;
	let min_pwm = cfg.min_pwm;
	let max_pwm = cfg.max_pwm;

	if (t >= full_mC)
		return max_pwm;

	let raw;
	if (cfg.start_temp == 40 && cfg.full_temp == 75 && min_pwm == 64 && max_pwm == 255) {
		if (t >= 68000)
			raw = 200 + int(((t - 68000) * 55) / 7000);
		else if (t >= 55000)
			raw = 112 + int(((t - 55000) * 88) / 13000);
		else if (t >= 40000)
			raw = 64 + int(((t - 40000) * 48) / 15000);
		else
			raw = 64;
	}
	else if (t <= start_mC) {
		raw = min_pwm;
	}
	else {
		raw = min_pwm + int(((t - start_mC) * (max_pwm - min_pwm)) / (full_mC - start_mC));
	}

	if (raw < min_pwm)
		raw = min_pwm;
	if (raw > max_pwm)
		raw = max_pwm;

	if (prev > 0) {
		let db = (cfg.hysteresis <= 0) ? 0 : ((cfg.hysteresis == 2 && cfg.slew_rate == 12) ? 3 : max(1, int(cfg.hysteresis * 1.5)));
		let slew_up = cfg.slew_rate;
		let slew_dn = max(1, int(slew_up / 2));
		let diff = raw - prev;

		if (diff > 0) {
			if (db > 0 && diff <= db && raw < max_pwm)
				raw = prev;
			else if (diff > slew_up)
				raw = prev + slew_up;
		}
		else if (diff < 0) {
			if (db > 0 && diff >= -db && raw > min_pwm)
				raw = prev;
			else if (diff < -slew_dn)
				raw = prev - slew_dn;
		}
	}

	return clamp_int(raw, min_pwm, max_pwm, min_pwm);
};

return {
	ROOT: ROOT,
	CONFIG_PATH: CONFIG_PATH,
	PID_PATH: PID_PATH,
	OWNER_PATH: OWNER_PATH,
	READY_PATH: READY_PATH,
	ERROR_PATH: ERROR_PATH,
	WPS_ACTIVE_PATH: WPS_ACTIVE_PATH,
	LUCIMSG_PATH: LUCIMSG_PATH,
	MODE_TABLE: MODE_TABLE,
	VALID_SOURCES: VALID_SOURCES,
	VALID_DRIVERS: VALID_DRIVERS,
	VALID_INVERTED: VALID_INVERTED,
	DEFAULT_CONFIG: DEFAULT_CONFIG,
	rpath: rpath,
	ensure_parent_dir: ensure_parent_dir,
	readfile_trim: readfile_trim,
	write_verified: write_verified,
	write_writeonly: write_writeonly,
	write_atomic_verified: write_atomic_verified,
	path_absent_verified: path_absent_verified,
	unlink_safe: unlink_safe,
	unlink_verified: unlink_verified,
	restore_file_or_unlink: restore_file_or_unlink,
	clamp_int: clamp_int,
	pwm_to_percent: pwm_to_percent,
	normalize_config: normalize_config,
	load_config: load_config,
	format_config: format_config,
	proc_start_ticks: proc_start_ticks,
	read_self_pid: read_self_pid,
	is_confirmed_worker_pid: is_confirmed_worker_pid,
	discover_fan: discover_fan,
	logical_to_raw: logical_to_raw,
	raw_to_logical: raw_to_logical,
	write_fan_logical: write_fan_logical,
	read_fan_logical: read_fan_logical,
	read_fan_raw: read_fan_raw,
	write_fan_raw_exact: write_fan_raw_exact,
	read_fan_rpm: read_fan_rpm,
	check_thermal_rw: check_thermal_rw,
	read_cpu_temp_mC: read_cpu_temp_mC,
	read_modem_temp_mC: read_modem_temp_mC,
	mC_to_C: mC_to_C,
	read_temps: read_temps,
	compute_smart_pwm: compute_smart_pwm
};
