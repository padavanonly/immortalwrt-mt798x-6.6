// SPDX-License-Identifier: Apache-2.0
// Retain discovered clients in RAM for this boot, independently of the browser.
export function arp_leases(text) {
	let leases = [];
	for (let line in split(text || '', '\n')) {
		let f = split(trim(line), /\s+/);
		if (length(f) != 6 || f[5] != 'br-lan' || !(int(f[2], 16) & 2) ||
		    !match(f[0], /^\d+\.\d+\.\d+\.\d+$/) ||
		    !match(f[3], /^([0-9a-f]{2}:){5}[0-9a-f]{2}$/i) ||
		    f[3] == '00:00:00:00:00:00' || (int(substr(f[3], 0, 2), 16) & 1))
			continue;
		push(leases, { ipaddr: f[0], macaddr: uc(f[3]), interface: f[5] });
	}
	return leases;
};

export function update_history(previous, current, boot_id) {
	if (type(previous) != 'object' || previous.version != 1 || previous.boot_id != boot_id)
		previous = {};
	let result = { version: 1, boot_id, dhcp_leases: [], dhcp6_leases: [] };
	for (let family in ['dhcp_leases', 'dhcp6_leases']) {
		let records = {};
		for (let source in [previous, current]) {
			for (let lease in type(source[family]) == 'array' ? source[family] : []) {
				if (type(lease) != 'object')
					continue;
				let mac = type(lease.macaddr) == 'string' ? uc(lease.macaddr) : '';
				let identity = mac || lc(lease.duid || '');
				let key = sprintf('%J', [identity, lease.interface || '',
					family == 'dhcp_leases' ? lease.ipaddr : [lease.duid || '', lease.iaid || '', lease.ip6addr || '']]);
				let old = records[key] || {};
				let record = { ...old, ...lease };
				if (mac)
					record.macaddr = mac;
				if (family == 'dhcp6_leases') {
					let ips = [];
					for (let item in [old, lease])
						for (let ip in type(item.ip6addrs) == 'array' ? item.ip6addrs : [])
							if (index(ips, ip) < 0)
								push(ips, ip);
					record.ip6addrs = ips;
				}
				records[key] = record;
			}
		}
		result[family] = values(records);
	}
	return result;
};
