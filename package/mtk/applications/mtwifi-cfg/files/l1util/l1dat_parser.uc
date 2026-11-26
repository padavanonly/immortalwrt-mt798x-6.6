#!/usr/bin/env ucode
/*
 * A ucode library to manipulate mtk's wifi driver.
 *
 * Copyright (C) 2016 MTK <support@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

import * as fs from 'fs';

let l1dat_parser = {
	L1_DAT_PATH: getenv("L1DAT_PATH") || getenv("L1_DAT_PATH") || "/etc/wireless/l1profile.dat",
	IF_RINDEX: "ifname_ridx",
	DEV_RINDEX: "devname_ridx",
	MAX_NUM_APCLI: 1,
	MAX_NUM_WDS: 4,
	MAX_NUM_MESH: 1,
	MAX_NUM_EXTIF: 16,
	MAX_NUM_DBDC_BAND: 3,
};

let l1cfg_options = {
	ext_ifname: "",
	apcli_ifname: "apcli",
	wds_ifname: "wds",
	mesh_ifname: "mesh",
};

function __resolve_l1_path(path) {
	if (path != null && path != l1dat_parser.L1_DAT_PATH)
		return path;
	return getenv("L1DAT_PATH") || getenv("L1_DAT_PATH") || path || l1dat_parser.L1_DAT_PATH;
}

l1dat_parser.__trim = function(s) {
	if (s != null)
		return trim("" + s);
	return null;
};

l1dat_parser.__cfg2list = function(str) {
	if (str == null)
		return [];
	let list = [];
	for (let k in split("" + str, ";")) {
		if (length(k) > 0)
			push(list, k);
	}
	return list;
};

l1dat_parser.token_get = function(str, n, v) {
	// n starts from 1
	// v is the backup in case token n is nil
	if (str == null || str === false)
		return v;
	let tmp = l1dat_parser.__cfg2list(str);
	let idx = int(n) - 1;
	if (idx >= 0 && idx < length(tmp) && tmp[idx] != null)
		return tmp[idx];
	return v;
};

l1dat_parser.add_default_value = function(l1cfg) {
	let idx = 1;
	while (l1cfg[idx] != null) {
		let v = l1cfg[idx];
		for (let opt, default_val in l1cfg_options) {
			if (opt == "ext_ifname") {
				if (v[opt] == null || v[opt] === false)
					v[opt] = (v["main_ifname"] ?? "") + "_";
			} else {
				if (v[opt] == null || v[opt] === false)
					v[opt] = default_val + idx + "_";
			}
		}
		idx++;
	}
	return l1cfg;
};

l1dat_parser.get_value_by_idx = function(devidx, mainidx, subidx, key) {
	if (devidx == null || mainidx == null || key == null)
		return null;

	let devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH);
	if (!devs)
		return null;

	let dev_ridx = l1dat_parser.DEV_RINDEX;
	let sidx = subidx ?? 1;
	let devname1 = devidx + "_" + mainidx;
	let devname2 = devidx + "_" + mainidx + "_" + sidx;

	if (devs[dev_ridx][devname2] && devs[dev_ridx][devname2][key] != null)
		return devs[dev_ridx][devname2][key];
	if (devs[dev_ridx][devname1] && devs[dev_ridx][devname1][key] != null)
		return devs[dev_ridx][devname1][key];
	return null;
};

l1dat_parser.l1_ifname_to_datpath = function(ifname) {
	if (!ifname)
		return null;

	let devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH);
	if (!devs)
		return null;

	let ridx = l1dat_parser.IF_RINDEX;
	return devs[ridx][ifname] ? devs[ridx][ifname].profile_path : null;
};

l1dat_parser.l1_ifname_to_zone = function(ifname) {
	if (!ifname)
		return null;

	let devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH);
	if (!devs)
		return null;

	let ridx = l1dat_parser.IF_RINDEX;
	return devs[ridx][ifname] ? devs[ridx][ifname].nvram_zone : null;
};

l1dat_parser.l1_zone_to_ifname = function(zone) {
	if (!zone)
		return null;

	let devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH);
	if (!devs)
		return null;

	let zone_dev = null;
	for (let k, dev in devs[l1dat_parser.DEV_RINDEX]) {
		if (dev.nvram_zone == zone)
			zone_dev = dev;
	}

	if (!zone_dev)
		return null;

	return [
		zone_dev.main_ifname,
		zone_dev.ext_ifname,
		zone_dev.apcli_ifname,
		zone_dev.wds_ifname,
		zone_dev.mesh_ifname
	];
};

l1dat_parser.load_l1_profile = function(path) {
	let real_path = __resolve_l1_path(path);
	let fd = fs.open(real_path, "r");
	if (!fd)
		return null;

	let devs = {};
	let chipset_num = {};
	let line;

	while ((line = fd.read("line")) != null) {
		line = l1dat_parser.__trim(line);
		if (substr(line, 0, 1) != "#") {
			let eq_pos = index(line, "=");
			if (eq_pos >= 0) {
				let k = l1dat_parser.__trim(substr(line, 0, eq_pos));
				let v = l1dat_parser.__trim(substr(line, eq_pos + 1));
				let m = match(k, /INDEX([0-9]+)_(.+)/);
				if (m) {
					let k1 = int(m[1]) + 1;
					let k2 = m[2];
					if (!devs[k1])
						devs[k1] = {};
					devs[k1][k2] = v ?? "";
				} else {
					let m2 = match(k, /INDEX([0-9]+)/);
					if (m2) {
						let k1 = int(m2[1]) + 1;
						if (!devs[k1])
							devs[k1] = {};
						devs[k1]["INDEX"] = v;
						chipset_num[v] = (!chipset_num[v] ? 1 : chipset_num[v] + 1);
						devs[k1]["mainidx"] = chipset_num[v];
					}
				}
			}
		}
	}

	fd.close();

	l1dat_parser.add_default_value(devs);

	let ridx = l1dat_parser.IF_RINDEX;
	let dridx = l1dat_parser.DEV_RINDEX;
	let band_num = l1dat_parser.MAX_NUM_DBDC_BAND;
	devs[ridx] = {};
	devs[dridx] = {};

	let dev_idx = 1;
	while (devs[dev_idx] != null) {
		let dev = devs[dev_idx];
		let dbdc_if = {};
		if (l1dat_parser.token_get(dev.main_ifname, 2, null)) {
			let cur_band_num = 0;
			for (let i = 1; i <= band_num; i++) {
				let ifname = l1dat_parser.token_get(dev.main_ifname, i, null);
				if (!ifname)
					break;
				dbdc_if[i] = ifname;
				cur_band_num = i;
			}
			for (let i = 1; i <= cur_band_num; i++) {
				devs[ridx][dbdc_if[i]] = {};
				devs[ridx][dbdc_if[i]]["subidx"] = i;

				for (let k, v in dev) {
					if (k == "INDEX" || k == "EEPROM_offset" || k == "EEPROM_size" || k == "mainidx") {
						devs[ridx][dbdc_if[i]][k] = v;
					} else {
						devs[ridx][dbdc_if[i]][k] = l1dat_parser.token_get(v, i, "");
					}
				}
				let devname = dev.INDEX + "_" + dev.mainidx + "_" + devs[ridx][dbdc_if[i]]["subidx"];
				devs[dridx][devname] = devs[ridx][dbdc_if[i]];
			}

			let apcli_if = {}, wds_if = {}, ext_if = {}, mesh_if = {};
			for (let i = 1; i <= cur_band_num; i++) {
				ext_if[i] = l1dat_parser.token_get(dev.ext_ifname, i, null);
				apcli_if[i] = l1dat_parser.token_get(dev.apcli_ifname, i, null);
				wds_if[i] = l1dat_parser.token_get(dev.wds_ifname, i, null);
				mesh_if[i] = l1dat_parser.token_get(dev.mesh_ifname, i, null);
			}

			for (let i = 1; i <= l1dat_parser.MAX_NUM_EXTIF - 1; i++) {
				for (let j = 1; j <= cur_band_num; j++) {
					devs[ridx][ext_if[j] + i] = devs[ridx][dbdc_if[j]];
				}
			}

			for (let i = 0; i <= l1dat_parser.MAX_NUM_APCLI - 1; i++) {
				for (let j = 1; j <= cur_band_num; j++) {
					devs[ridx][apcli_if[j] + i] = devs[ridx][dbdc_if[j]];
				}
			}

			for (let i = 0; i <= l1dat_parser.MAX_NUM_WDS - 1; i++) {
				for (let j = 1; j <= cur_band_num; j++) {
					devs[ridx][wds_if[j] + i] = devs[ridx][dbdc_if[j]];
				}
			}

			for (let i = 0; i <= l1dat_parser.MAX_NUM_MESH - 1; i++) {
				for (let j = 1; j <= cur_band_num; j++) {
					if (mesh_if[j]) {
						devs[ridx][mesh_if[j] + i] = devs[ridx][dbdc_if[j]];
					}
				}
			}
		} else {
			devs[ridx][dev.main_ifname] = dev;

			let devname = dev.INDEX + "_" + dev.mainidx;
			devs[dridx][devname] = dev;

			for (let i = 1; i <= l1dat_parser.MAX_NUM_EXTIF - 1; i++) {
				devs[ridx][dev.ext_ifname + i] = dev;
			}

			for (let i = 0; i <= l1dat_parser.MAX_NUM_APCLI - 1; i++) {
				devs[ridx][dev.apcli_ifname + i] = dev;
			}

			for (let i = 0; i <= l1dat_parser.MAX_NUM_WDS - 1; i++) {
				devs[ridx][dev.wds_ifname + i] = dev;
			}

			for (let i = 0; i <= l1dat_parser.MAX_NUM_MESH - 1; i++) {
				devs[ridx][dev.mesh_ifname + i] = dev;
			}
		}
		dev_idx++;
	}

	return devs;
};

return l1dat_parser;
