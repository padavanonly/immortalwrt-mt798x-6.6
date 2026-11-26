#!/usr/bin/env ucode
/*
 * Copyright (C) 2023 hanwckf <hanwckf@vip.qq.com>
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	The Free Software Foundation; either version 2 of the License, or
 * 	(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

import * as fs from 'fs';

let mtwifi_utils = {};

let DAT_NOSTRIP_KEYS = {
	"SSID": true,
	"SSID1": true,
	"SSID2": true,
	"SSID3": true,
	"SSID4": true,
	"SSID5": true,
	"SSID6": true,
	"SSID7": true,
	"SSID8": true,
	"SSID9": true,
	"SSID10": true,
	"SSID11": true,
	"SSID12": true,
	"SSID13": true,
	"SSID14": true,
	"SSID15": true,
	"SSID16": true,
	"WPAPSK": true,
	"WPAPSK1": true,
	"WPAPSK2": true,
	"WPAPSK3": true,
	"WPAPSK4": true,
	"WPAPSK5": true,
	"WPAPSK6": true,
	"WPAPSK7": true,
	"WPAPSK8": true,
	"WPAPSK9": true,
	"WPAPSK10": true,
	"WPAPSK11": true,
	"WPAPSK12": true,
	"WPAPSK13": true,
	"WPAPSK14": true,
	"WPAPSK15": true,
	"WPAPSK16": true,
	"ApCliSsid": true,
	"ApCliWPAPSK": true,
};

function __kvc_ltrim(s) {
	if (s == null) return "";
	return replace("" + s, /^[ \t]+/, "");
}

function __kvc_rtrim(s) {
	if (s == null) return "";
	return replace("" + s, /[ \t]+$/, "");
}

function __kvc_trim(s) {
	if (s == null) return "";
	return __kvc_rtrim(__kvc_ltrim(s));
}

function __split_kvc_lines(content) {
	let lines = [];
	let len = length(content);
	if (len == 0)
		return lines;

	let pos = 0;
	while (pos < len) {
		let rem = substr(content, pos);
		let m = match(rem, /\r\n|\r|\n/);
		if (!m) {
			push(lines, rem);
			break;
		}
		let eol_idx = index(rem, m[0]);
		push(lines, substr(rem, 0, eol_idx));
		pos += eol_idx + length(m[0]);
	}
	return lines;
}

mtwifi_utils.esc = function(x) {
	return replace("" + (x ?? ""), /[][\\^$.|?*+(){}]/g, "\\$&");
};

mtwifi_utils.trim = function(s) {
	if (s != null)
		return trim("" + s);
	return null;
};

mtwifi_utils.exists = function(path) {
	if (!path)
		return false;
	let root = getenv("MTWIFI_ROOT") || getenv("MTWIFI_SYSFS_ROOT") || "";
	let target = (root != "" && substr(path, 0, 1) == "/") ? (root + path) : path;
	let st = fs.stat(target);
	return st != null;
};

mtwifi_utils.read_sysfs_net = function(ifname, attr) {
	if (!ifname || !attr || match("" + ifname, /[\/\x00]/) || match("" + attr, /[\/\x00]/))
		return null;
	let root = getenv("MTWIFI_ROOT") || getenv("MTWIFI_SYSFS_ROOT") || "";
	let path = root + "/sys/class/net/" + ifname + "/" + attr;
	let content = fs.readfile(path);
	return (content != null) ? trim(content) : null;
};

mtwifi_utils.list_netdevs = function() {
	let root = getenv("MTWIFI_ROOT") || getenv("MTWIFI_SYSFS_ROOT") || "";
	let entries = fs.lsdir(root + "/sys/class/net");
	let devs = [];
	if (type(entries) != "array")
		return devs;
	for (let name in entries) {
		if (name != "." && name != "..")
			push(devs, name);
	}
	return devs;
};

mtwifi_utils.read_pipe = function(pipe) {
	let root = getenv("MTWIFI_ROOT") || getenv("MTWIFI_SYSFS_ROOT") || "";
	if (root != "")
		pipe = replace(pipe, "/sys", root + "/sys");
	let retry_count = 10;
	let txt = "";
	while (retry_count > 0) {
		let fp = fs.popen(pipe, "r");
		if (fp) {
			txt = fp.read("all") ?? "";
			fp.close();
			break;
		}
		retry_count--;
	}
	return txt;
};

mtwifi_utils.exec = function(cmd) {
	if (type(cmd) == "array")
		return system(cmd);
	if (type(cmd) == "string") {
		let parts = split(trim(cmd), /\s+/);
		if (length(parts) > 0 && parts[0] != "")
			return system(parts);
	}
	return -1;
};

mtwifi_utils.syslog = function(level, msg) {
	try {
		let logmod = require("log");
		if (logmod && logmod.syslog) {
			let prio = (level == "err") ? (logmod.LOG_ERR ?? 3) : (logmod.LOG_INFO ?? 6);
			logmod.syslog(prio, "%s", msg);
			return;
		}
	} catch (e) {}
};

mtwifi_utils.__cfg2list = function(str) {
	// delimiter == ";"
	if (str == null)
		return [];
	let list = [];
	for (let k in split("" + str, ";")) {
		if (length(k) > 0)
			push(list, k);
	}
	return list;
};

mtwifi_utils.token_set = function(str, n, v) {
	// n starts from 1
	// delimiter == ";"
	if (str == null || str === false)
		return null;
	if (v == null)
		return null;
	if (type(v) != "string" && type(v) != "int" && type(v) != "double") {
		mtwifi_utils.syslog("err", "invalid value type in token_set, " + type(v));
		return null;
	}
	let num = int(n);
	let tmp = mtwifi_utils.__cfg2list("" + str);
	let val_str = "" + v;
	if (length(tmp) < num) {
		for (let i = length(tmp); i < num; i++) {
			if (tmp[i] == null)
				tmp[i] = val_str;
		}
	} else if (num >= 1) {
		tmp[num - 1] = val_str;
	}
	let res = join(";", tmp);
	res = replace(res, /^;+/, "");
	res = replace(res, /;+$/, "");
	res = replace(res, /;+/g, ";");
	return res;
};

mtwifi_utils.token_get = function(str, n, v) {
	// n starts from 1
	// v is the backup in case token n is nil
	if (str == null || str === false)
		return v;
	let tmp = mtwifi_utils.__cfg2list("" + str);
	let idx = int(n) - 1;
	if (idx >= 0 && idx < length(tmp) && tmp[idx] != null)
		return tmp[idx];
	return v;
};

mtwifi_utils.set_MultiField = function(dats, key, idx, val) {
	if (dats == null || key == null || val == null)
		return;
	dats[key] = mtwifi_utils.token_set(dats[key] ?? "", idx, val);
};

// Native ucode bitwise helpers
mtwifi_utils.bit = {
	band: function(a, b) { return int(a) & int(b); },
	bor: function(a, b) { return int(a) | int(b); },
	bxor: function(a, b) { return int(a) ^ int(b); },
	bnot: function(a) { return ~int(a); },
	lshift: function(a, n) { return int(a) << int(n); },
	rshift: function(a, n) { return int(a) >> int(n); },
};

// Hex and MAC address helpers
mtwifi_utils.str2hex = function(s) {
	if (s == null) return "";
	return hexenc("" + s);
};

mtwifi_utils.hex2str = function(h) {
	if (h == null) return "";
	return hexdec("" + h) ?? "";
};

mtwifi_utils.is_valid_mac = function(mac) {
	if (type(mac) != "string") return false;
	return match(mac, /^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$/) != null;
};

// Pure-ucode .dat parser/writer matching libkvcutil + libdatconf 100%
mtwifi_utils.load_profile = function(path) {
	if (!path)
		return null;
	let fd = fs.open(path, "r");
	if (!fd)
		return null;
	let content = fd.read("all") ?? "";
	fd.close();

	let cfgs = {};
	let lines = __split_kvc_lines(content);
	for (let line in lines) {
		let pkey = __kvc_ltrim(line);
		let eq_pos = index(pkey, "=");
		if (substr(pkey, 0, 1) == "#" || eq_pos < 0)
			continue;
		let key = __kvc_rtrim(substr(pkey, 0, eq_pos));
		let val = substr(pkey, eq_pos + 1);
		if (!DAT_NOSTRIP_KEYS[key])
			val = __kvc_trim(val);
		cfgs[key] = val;
	}
	return cfgs;
};

mtwifi_utils.save_profile = function(cfgs, path) {
	if (!cfgs || !path)
		return false;

	let datconf_bin = getenv("MTWIFI_DATCONF_BIN");
	if (datconf_bin && fs.stat(datconf_bin)) {
		let qbin = "'" + replace("" + datconf_bin, "'", "'\\''") + "'";
		let qpath = "'" + replace("" + path, "'", "'\\''") + "'";
		let fp = fs.popen(qbin + " -f " + qpath + " batch", "w");
		if (fp) {
			for (let k, v in cfgs) {
				if (v != null && (type(v) == "string" || type(v) == "int" || type(v) == "double"))
					fp.write(k + "=" + v + "\n");
			}
			fp.close();
			mtwifi_utils.exec(["sync"]);
			return true;
		}
	}

	let fd = fs.open(path, "r");
	if (!fd)
		return false;
	let content = fd.read("all") ?? "";
	fd.close();

	let og_items = [];
	let lines = __split_kvc_lines(content);
	for (let line in lines) {
		let pkey = __kvc_ltrim(line);
		let eq_pos = index(pkey, "=");
		if (substr(pkey, 0, 1) == "#" || eq_pos < 0) {
			push(og_items, {
				invalid: true,
				key: line,
				value: null,
				nostrip: false
			});
		} else {
			let key = __kvc_rtrim(substr(pkey, 0, eq_pos));
			let nostrip = DAT_NOSTRIP_KEYS[key] ? true : false;
			let val = substr(pkey, eq_pos + 1);
			if (!nostrip)
				val = __kvc_trim(val);
			push(og_items, {
				invalid: false,
				key: key,
				value: val,
				nostrip: nostrip
			});
		}
	}

	let ng_items = [];
	let keys = [];
	for (let k, v in cfgs)
		push(keys, k);
	sort(keys);

	for (let raw_k in keys) {
		let raw_v = cfgs[raw_k];
		if (raw_v == null)
			continue;
		let vt = type(raw_v);
		if (vt != "string" && vt != "int" && vt != "double")
			continue;

		let k = __kvc_trim("" + raw_k);
		let v = "" + raw_v;
		let nostrip = DAT_NOSTRIP_KEYS[k] ? true : false;
		if (!nostrip)
			v = __kvc_trim(v);

		let found = false;
		for (let item in og_items) {
			if (!item.invalid && item.key == k) {
				item.value = v;
				found = true;
				break;
			}
		}
		if (!found) {
			for (let item in ng_items) {
				if (item.key == k) {
					item.value = v;
					found = true;
					break;
				}
			}
		}
		if (!found) {
			push(ng_items, {
				invalid: false,
				key: k,
				value: v,
				nostrip: nostrip
			});
		}
	}

	let out = fs.open(path, "w");
	if (!out)
		return false;

	for (let item in og_items) {
		if (item.invalid)
			out.write(item.key + "\n");
		else
			out.write(item.key + "=" + item.value + "\n");
	}
	for (let item in ng_items) {
		out.write(item.key + "=" + item.value + "\n");
	}
	out.close();

	mtwifi_utils.exec(["sync"]);
	return true;
};

mtwifi_utils.sleep = function(s) {
	let ms = int(s) * 1000;
	if (ms > 0)
		sleep(ms);
};

mtwifi_utils.log2file = function(str) {
	let logfile_path = getenv("MTWIFI_LOGFILE") || "/tmp/mtwifi.log";
	let logfile = fs.open(logfile_path, "a");
	if (logfile) {
		let lt = localtime();
		let ts = sprintf("%02d:%02d:%02d", lt.hour, lt.min, lt.sec);
		logfile.write(ts + " " + str + "\n");
		logfile.close();
	}
};

return mtwifi_utils;
