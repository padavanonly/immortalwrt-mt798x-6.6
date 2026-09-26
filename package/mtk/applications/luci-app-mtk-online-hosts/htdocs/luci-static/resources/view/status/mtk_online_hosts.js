'use strict';
'require baseclass';
'require rpc';
'require fs';
'require uci';
'require network';
'require validation';
'require poll';

const callLuciDHCPLeases = rpc.declare({
	object: 'luci-rpc',
	method: 'getDHCPLeases',
	expect: { '': {} }
});
 
const callClientRates = rpc.declare({
	object: 'luci.client-rates',
	method: 'get',
	params: [ 'addresses' ],
	expect: { '': {} }
});

const callClientWeb = rpc.declare({
	object: 'luci.client-web',
	method: 'get',
	expect: { '': {} }
});

const callUfpList = rpc.declare({
	object: 'fingerprint',
	method: 'fingerprint',
});

return baseclass.extend({
	title: _('Online Hosts'),
	deferFirstLoad: true,
	disableCache: true,

	// Plain hostname rendering (without luci-app-oui icon lookup).
	renderHostname(host) {
		return E('span', { 'style': 'display:block;text-align:left' }, [ document.createTextNode(host || '-') ]);
	},

	isMACStatic: {},
	isDUIDStatic: {},
	isDUIDIAIDStatic: {},

	normalizeRateAddress(ip) {
		ip = ip.replace(/\/\d+$/, '');
		const v4 = validation.parseIPv4(ip);
		if (v4)
			return v4.join('.');
		const v6 = validation.parseIPv6(ip);
		return v6 ? v6.map(word => word.toString(16)).join(':') : null;
	},

	clientAddresses(lease, hints) {
		if (lease.rateAddresses)
			return lease.rateAddresses;
		const host = hints.hosts?.[lease.macaddr?.toUpperCase()] || {};
		return Array.from(new Set([
			lease.ipaddr, ...L.toArray(lease.ipaddrs), lease.ip6addr, ...L.toArray(lease.ip6addrs),
			...L.toArray(host.ipaddrs || host.ipv4), ...L.toArray(host.ip6addrs || host.ipv6)
		].filter(Boolean).map(ip => this.normalizeRateAddress(ip)).filter(Boolean)));
	},

	renderRate(lease, hints, data, direction) {
		return this.rateValue(this.clientAddresses(lease, hints), data, direction);
	},

	rateValue(addresses, data, direction, mac, online) {
		if (direction == 'connections') {
			if (online === false)
				return [0, '0'];
			const value = data?.connections?.[mac?.toUpperCase()];
			return value != null ? [Number(value), String(value)] : [-1, '-'];
		}
		if (direction == 'total') {
			const value = data?.totals?.[mac?.toUpperCase()];
			return value != null ? [Number(value), '%1024.2mB'.format(value)] : [-1, '-'];
		}
		let total = 0;
		for (const ip of addresses) {
			const rate = data?.rates?.[ip];
			if (!rate?.ready || (mac && rate.mac && rate.mac != mac.toUpperCase()))
				return [ -1, '-' ];
			total += Number(rate[direction] || 0);
		}
		return addresses.length ? [ total, '%1024.1mB/s'.format(total) ] : [ -1, '-' ];
	},

	rateCell(lease, hints, direction, online) {
		const addresses = this.clientAddresses(lease, hints);
		const value = this.rateValue(addresses, this.rateData, direction, lease.macaddr, online);
		return [ value[0], E('span', {
			'class': 'luci-client-rate',
			'data-addresses': JSON.stringify(addresses),
			'data-direction': direction,
			'data-online': online === false ? '0' : '1',
			'data-mac': lease.macaddr || ''
		}, value[1]) ];
	},


	clientURL(lease, data) {
		const ip = validation.parseIPv4(lease.ipaddr || '')?.join('.');
		const client = data?.clients?.[ip];
		if (!ip || !client?.ready || client.mac != lease.macaddr?.toUpperCase())
			return null;
		const port = [80, 8080, 5666, 443, 4430, 5667].find(port => client.ports?.includes(port));
		if (!port)
			return null;
		const scheme = [80, 8080, 5666].includes(port) ? 'http' : 'https';
		return scheme + '://' + ip + (port == 80 || port == 443 ? '' : ':' + port) + '/';
	},

	renderClientIP(lease, data) {
		const url = this.clientURL(lease, data);
		return url ? E('a', { 'href': url, 'target': '_blank', 'rel': 'noopener noreferrer', 'style': 'text-decoration:underline', 'data-value': lease.ipaddr }, lease.ipaddr) : lease.ipaddr;
	},

	initLeaseTable(table) {
		const widget = new L.ui.Table(table);
		const update = widget.update;
		widget.update = function(...args) {
			const result = update.apply(this, args);
			this.node.querySelectorAll('.cbi-section-actions').forEach(cell => {
				cell.style.setProperty('text-align', 'center', 'important');
			});
			return result;
		};
		const derive = widget.deriveSortKey;
		widget.deriveSortKey = function(value, index) {
			return Array.isArray(value) ? Number(value[0]) : derive.call(this, value, index);
		};
		const totalIndex = Array.from(table.querySelectorAll('th')).findIndex(th => th.dataset.totalTraffic);
		if (!widget.getActiveSortState())
			widget.sortState = [totalIndex, true];
		L.dom.bindClassInstance(table, widget);
	},

	updateLeaseOrder(table) {
		const widget = L.dom.findClassInstance(table);
		if (!widget)
			return;
		// Keep the table widget's raw keys current for subsequent header clicks.
		for (const row of widget.data || [])
			for (const value of row)
				if (Array.isArray(value) && value[1]?.matches?.('.luci-client-rate'))
					value[0] = Number(value[1].closest('td')?.dataset.value ?? -1);
		const sorting = widget.getActiveSortState();
		if (sorting) {
			const rows = Array.from(table.querySelectorAll('tr')).filter(row => row.querySelector('.luci-client-rate'));
			const key = row => {
				const cell = row.children[sorting[0]];
				return cell.hasAttribute('data-value') ? Number(cell.dataset.value) : widget.deriveSortKey(cell.querySelector('a[data-value]') || cell, sorting[0]);
			};
			const sorted = rows.slice().sort((a, b) => {
				const av = key(a), bv = key(b);
				const cmp = typeof av == 'number' && typeof bv == 'number' ? av - bv : L.naturalCompare(av, bv);
				return sorting[1] ? -cmp : cmp;
			});
			if (sorted.some((row, i) => row !== rows[i]))
				for (const row of sorted)
					row.parentElement.appendChild(row);
		}
	},

	refreshRates() {
		const cells = Array.from(document.querySelectorAll('.luci-client-rate'));
		const addresses = Array.from(new Set(cells.flatMap(cell => JSON.parse(cell.dataset.addresses))));
		if (!cells.length)
			return Promise.resolve();
		return L.resolveDefault(callClientRates(addresses.slice(0, 1024)), {}).then(data => {
			this.rateData = data;
			// Query again: the normal overview refresh may have replaced the rows.
			document.querySelectorAll('.luci-client-rate').forEach(cell => {
				const value = this.rateValue(JSON.parse(cell.dataset.addresses), data, cell.dataset.direction, cell.dataset.mac, cell.dataset.online !== '0');
				cell.textContent = value[1];
				cell.closest('td')?.setAttribute('data-value', value[0]);
			});
			document.querySelectorAll('#status_leases').forEach(table => this.updateLeaseOrder(table));
		});
	},

	load() {
		return Promise.all([
			callLuciDHCPLeases(),
			network.getHostHints(),
			L.hasSystemFeature('ufpd') ? callUfpList() : null,
			L.resolveDefault(uci.load('dhcp')),
			L.resolveDefault(callClientWeb(), {}),
			L.resolveDefault(fs.exec_direct('/usr/libexec/luci-arp'), ''),
			L.resolveDefault(fs.read('/tmp/luci-client-history.json').then(JSON.parse), {})
		]);
	},

	render([dhcp_leases, host_hints, ufp_list, dhcp_config, web, arp, history]) {
		if (!this.ratePoll) {
			this.ratePoll = L.bind(this.refreshRates, this);
			poll.add(this.ratePoll, 2);
		}
		if (L.hasSystemFeature('dnsmasq') || L.hasSystemFeature('odhcpd'))
			return this.renderLeases(dhcp_leases, host_hints, ufp_list, web, arp, history);

		return null;
	},

	handleCreateStaticLease(lease, ev) {
		ev.currentTarget.classList.add('spinning');
		ev.currentTarget.disabled = true;
		ev.currentTarget.blur();

		const cfg = uci.add('dhcp', 'host');
		uci.set('dhcp', cfg, 'name', lease.hostname);
		uci.set('dhcp', cfg, 'ip', lease.ipaddr);
		uci.set('dhcp', cfg, 'mac', [lease.macaddr.toLowerCase()]);

		return uci.save()
			.then(L.bind(L.ui.changes.init, L.ui.changes))
			.then(L.bind(L.ui.changes.displayChanges, L.ui.changes));
	},

	handleCreateStaticLease6(lease, ev) {
		ev.currentTarget.classList.add('spinning');
		ev.currentTarget.disabled = true;
		ev.currentTarget.blur();

		const cfg = uci.add('dhcp', 'host');
		const ip6addr = lease.ip6addrs?.[0]?.replace(/\/128$/, '');
		const ip6arr = ip6addr ? validation.parseIPv6(ip6addr) : null;

		// Combine DUID and IAID if both available
		// (note that we know that lease.duid is set here)
		let duid_iaid = lease.duid.toLowerCase();
		if (lease.iaid)
			duid_iaid += `%${lease.iaid}`.toLowerCase();

		uci.set('dhcp', cfg, 'name', lease.hostname);
		uci.set('dhcp', cfg, 'duid', [duid_iaid]);
		if (lease.macaddr)
			uci.set('dhcp', cfg, 'mac', [lease.macaddr.toLowerCase()]);
		if (ip6arr)
			uci.set('dhcp', cfg, 'hostid', (ip6arr[6] * 0xFFFF + ip6arr[7]).toString(16));

		return uci.save()
			.then(L.bind(L.ui.changes.init, L.ui.changes))
			.then(L.bind(L.ui.changes.displayChanges, L.ui.changes));
	},

	arpLeases(arp) {
		const leases = [];
		for (const line of String(arp || '').trim().split(/\n/)) {
			const fields = line.trim().split(/\s+/);
			if (fields.length != 6 || fields[5] != 'br-lan' ||
			    !(parseInt(fields[2], 16) & 2) || !validation.parseIPv4(fields[0]) ||
			    !/^(?:[0-9a-f]{2}:){5}[0-9a-f]{2}$/i.test(fields[3]) ||
			    fields[3] == '00:00:00:00:00:00' || (parseInt(fields[3].slice(0, 2), 16) & 1))
				continue;
			leases.push({ ipaddr: fields[0], macaddr: fields[3].toUpperCase(), interface: fields[5] });
		}
		return leases;
	},

	mergeLeases(leases, leases6, hints) {
		const clients = new Map();
		const owners = new Map();
		const normalizeMAC = mac => /^(?:[0-9a-f]{2}:){5}[0-9a-f]{2}$/i.test(mac || '') &&
			mac != '00:00:00:00:00:00' && !(parseInt(mac.slice(0, 2), 16) & 1)
			? mac.toUpperCase() : null;
		const addresses = lease => [lease.ipaddr, ...L.toArray(lease.ipaddrs),
			lease.ip6addr, ...L.toArray(lease.ip6addrs)].filter(Boolean);
		const addOwner = (ip, mac) => {
			const address = this.normalizeRateAddress(ip);
			if (!address || !mac)
				return;
			if (!owners.has(address))
				owners.set(address, new Set());
			owners.get(address).add(mac);
		};
		for (const [mac, host] of Object.entries(hints.hosts || {}))
			for (const ip of [...L.toArray(host.ipaddrs || host.ipv4), ...L.toArray(host.ip6addrs || host.ipv6)])
				addOwner(ip, normalizeMAC(mac));
		for (const lease of [...leases, ...leases6])
			for (const ip of addresses(lease))
				addOwner(ip, normalizeMAC(lease.macaddr));

		for (const [family, list] of [[4, leases], [6, leases6]]) {
			for (const lease of list) {
				let mac = normalizeMAC(lease.macaddr);
				if (!mac) {
					const matches = new Set();
					for (const ip of addresses(lease))
						for (const owner of owners.get(this.normalizeRateAddress(ip)) || [])
							matches.add(owner);
					if (matches.size == 1)
						mac = matches.values().next().value;
				}
				// DUID/IAID only deduplicate the same unidentified lease and its history;
				// they never join it to a different IPv4 record based on hostname.
				const unknown = lease.duid ? [family, lease.duid.toLowerCase(), lease.iaid?.toLowerCase(), lease.interface] :
					addresses(lease).length ? [family, ...addresses(lease).map(ip => this.normalizeRateAddress(ip))] : null;
				const key = mac || (unknown ? JSON.stringify(unknown) : Symbol());
				if (!clients.has(key))
					clients.set(key, { macaddr: mac, hostname: lease.hostname,
						ipaddrs: [], ip6addrs: [], interfaces: [], leases: [], leases6: [], activeAddresses: [] });
				const client = clients.get(key);
				client.hostname ||= lease.hostname;
				if (lease.interface && !client.interfaces.includes(lease.interface))
					client.interfaces.push(lease.interface);
				const ips = family == 4 ? client.ipaddrs : client.ip6addrs;
				for (const ip of addresses(lease)) {
					const address = this.normalizeRateAddress(ip);
					if (address && !lease._historical && !client.activeAddresses.includes(address))
						client.activeAddresses.push(address);
					if (address && !ips.some(value => this.normalizeRateAddress(value) == address))
						ips.push(ip);
				}
				const records = family == 4 ? client.leases : client.leases6;
				if (!lease._historical && !records.some(record => family == 4 ? record.ipaddr == lease.ipaddr :
					record.duid?.toLowerCase() == lease.duid?.toLowerCase() && record.iaid == lease.iaid))
					records.push(Object.assign({}, lease, { macaddr: mac }));
			}
		}
		for (const client of clients.values()) {
			const current = { macaddr: client.macaddr, ipaddrs: client.activeAddresses };
			client.rateAddresses = client.activeAddresses.length ? this.clientAddresses(current, hints) : [];
		}
		return Array.from(clients.values());
	},

	leaseActions(client) {
		const actions = [];
		for (const [family, leases] of [[4, client.leases], [6, client.leases6]]) {
			for (const lease of leases) {
				const mac = lease.macaddr?.toLowerCase();
				const duid = lease.duid?.toLowerCase();
				const iaid = lease.iaid?.toLowerCase();
				if (family == 4 ? !mac : !duid)
					continue;
				const disabled = family == 4 ? this.isMACStatic[mac] :
					this.isDUIDStatic[duid] || (iaid && this.isDUIDIAIDStatic[`${duid}%${iaid}`]);
				actions.push(E('button', {
					'class': 'cbi-button cbi-button-apply',
					'click': L.bind(family == 4 ? this.handleCreateStaticLease : this.handleCreateStaticLease6, this, lease),
					'data-tooltip': _('Reserve a specific IP address for this device'),
					'disabled': disabled || null
				}, [_('Reserve IP') + ` (IPv${family})` + (leases.length > 1 ? ` ${family == 4 ? lease.ipaddr : lease.ip6addrs?.[0] || lease.ip6addr || ''}` : '')]));
			}
		}
		return E('div', { 'style': 'display:flex;flex-direction:column;gap:4px;align-items:center' }, actions);
	},

	renderLeases(dhcp_leases, host_hints, macaddr, web, arp, history) {
		const arpClients = this.arpLeases(arp);
		const leases = [...(Array.isArray(dhcp_leases.dhcp_leases) ? dhcp_leases.dhcp_leases : []),
			...arpClients,
			...(Array.isArray(history?.dhcp_leases) ? history.dhcp_leases : []).map(lease => ({ ...lease, _historical: true }))];
		const leases6 = [...(Array.isArray(dhcp_leases.dhcp6_leases) ? dhcp_leases.dhcp6_leases : []),
			...(Array.isArray(history?.dhcp6_leases) ? history.dhcp6_leases : []).map(lease => ({ ...lease, _historical: true }))];
		if (leases.length == 0 && leases6.length == 0)
			return E('em', _('No active leases found'));
		const machints = host_hints.getMACHints(false);
		const isReadonlyView = !L.hasViewPermission();

		this.isMACStatic = {};
		this.isDUIDStatic = {};
		this.isDUIDIAIDStatic = {};
		for (const host of uci.sections('dhcp', 'host')) {

			for (const mac of L.toArray(host.mac).map(m => m.toLowerCase()))
				this.isMACStatic[mac] = true;

			for (const duid_iaid of L.toArray(host.duid).map(m => m.toLowerCase())) {
				const parts = duid_iaid.split('%').length;

				if (parts == 1)
					this.isDUIDStatic[duid_iaid] = true;
				else if (parts == 2)
					this.isDUIDIAIDStatic[duid_iaid] = true;
			}
		};

		const clients = this.mergeLeases(leases, leases6, host_hints);
		// Failed neighbours can retain their MAC in /proc/net/arp with flags 0x0.
		const onlineMACs = new Set(arpClients.map(client => client.macaddr));
		const table = E('table', { 'id': 'status_leases', 'class': 'table leases' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				E('th', { 'class': 'th' }, _('Online')),
				E('th', { 'class': 'th', 'style': 'text-align:left' }, _('Hostname')),
				E('th', { 'class': 'th' }, _('IPv4 address')),
				E('th', { 'class': 'th' }, _('IPv6 addresses')),
				E('th', { 'class': 'th' }, _('MAC address')),
				E('th', { 'class': 'th' }, _('Upload')),
				E('th', { 'class': 'th' }, _('Download')),
				E('th', { 'class': 'th', 'data-total-traffic': '1' }, _('Total traffic')),
				E('th', { 'class': 'th' }, _('Connection count')),
				isReadonlyView ? E([]) : E('th', { 'class': 'th cbi-section-actions center' }, _('Static Lease'))
			])
		]);

		this.initLeaseTable(table);
		cbi_update_table(table, clients.map(client => {
			const hint = machints.find(h => h[0].toUpperCase() == client.macaddr);
			const host = client.hostname || hint?.[1];
			const vendor = macaddr?.[client.macaddr?.toLowerCase()]?.vendor;
			const online = onlineMACs.has(client.macaddr);
			const status = online ? _('Online') : _('Offline');
			const columns = [
				[online ? 1 : 0, E('span', {
					'class': 'luci-client-online',
					'role': 'img', 'aria-label': status, 'title': status,
					'style': 'display:inline-block;width:10px;height:10px;border-radius:50%;background-color:' + (online ? '#28a745' : '#dc3545')
				})],
				this.renderHostname(host)),
				client.ipaddrs.length ? E('div', {}, client.ipaddrs.map(ipaddr =>
					E('div', client.activeAddresses.includes(this.normalizeRateAddress(ipaddr)) ? {} : { 'style': 'opacity:.55', 'title': _('Expired') },
						client.activeAddresses.includes(this.normalizeRateAddress(ipaddr)) ? this.renderClientIP({ ipaddr, macaddr: client.macaddr }, web) : ipaddr))) : '-',
				client.ip6addrs.length ? E('div', { 'style': 'overflow-wrap:anywhere' },
					client.ip6addrs.map(ip => E('div', client.activeAddresses.includes(this.normalizeRateAddress(ip)) ? {} : { 'style': 'opacity:.55', 'title': _('Expired') }, ip))) : '-',
				vendor ? `${client.macaddr} (${vendor})` : client.macaddr || '-',
				this.rateCell(client, host_hints, 'upload'),
				this.rateCell(client, host_hints, 'download'),
				this.rateCell(client, host_hints, 'total'),
				this.rateCell(client, host_hints, 'connections', online)
			];
			if (!isReadonlyView)
				columns.push(this.leaseActions(client));
			return columns;
		}), E('em', _('No active leases found')));
		this.updateLeaseOrder(table);
		return table;
	},

});
