'use strict';
'require baseclass';
'require rpc';
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

const callUfpList = rpc.declare({
	object: 'fingerprint',
	method: 'fingerprint',
});

return baseclass.extend({
	title: _('DHCP Leases'),
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
		const host = hints.hosts?.[lease.macaddr?.toUpperCase()] || {};
		return Array.from(new Set([
			lease.ipaddr, lease.ip6addr, ...L.toArray(lease.ip6addrs),
			...L.toArray(host.ipaddrs || host.ipv4), ...L.toArray(host.ip6addrs || host.ipv6)
		].filter(Boolean).map(ip => this.normalizeRateAddress(ip)).filter(Boolean)));
	},

	renderRate(lease, hints, data, direction) {
		return this.rateValue(this.clientAddresses(lease, hints), data, direction);
	},

	rateValue(addresses, data, direction) {
		let total = 0;
		for (const ip of addresses) {
			const rate = data?.rates?.[ip];
			if (!rate?.ready)
				return [ -1, '-' ];
			total += Number(rate[direction] || 0);
		}
		return addresses.length ? [ total, '%1024.1mB/s'.format(total) ] : [ -1, '-' ];
	},

	rateCell(lease, hints, direction) {
		const addresses = this.clientAddresses(lease, hints);
		const value = this.rateValue(addresses, this.rateData, direction);
		return [ value[0], E('span', {
			'class': 'luci-client-rate',
			'data-addresses': JSON.stringify(addresses),
			'data-direction': direction
		}, value[1]) ];
	},

	refreshRates() {
		const cells = Array.from(document.querySelectorAll('.luci-client-rate'));
		const addresses = Array.from(new Set(cells.flatMap(cell => JSON.parse(cell.dataset.addresses))));
		if (!addresses.length)
			return Promise.resolve();
		return L.resolveDefault(callClientRates(addresses.slice(0, 1024)), {}).then(data => {
			this.rateData = data;
			// Query again: the normal overview refresh may have replaced the rows.
			document.querySelectorAll('.luci-client-rate').forEach(cell => {
				const value = this.rateValue(JSON.parse(cell.dataset.addresses), data, cell.dataset.direction);
				cell.textContent = value[1];
				cell.closest('td')?.setAttribute('data-value', value[0]);
			});
		});
	},

	load() {
		return Promise.all([
			callLuciDHCPLeases(),
			network.getHostHints(),
			L.hasSystemFeature('ufpd') ? callUfpList() : null,
			L.resolveDefault(uci.load('dhcp'))
		]);
	},

	render([dhcp_leases, host_hints, ufp_list]) {
		if (!this.ratePoll) {
			this.ratePoll = L.bind(this.refreshRates, this);
			poll.add(this.ratePoll, 2);
		}
		if (L.hasSystemFeature('dnsmasq') || L.hasSystemFeature('odhcpd'))
			return this.renderLeases(dhcp_leases, host_hints, ufp_list);

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

	renderLeases(dhcp_leases, host_hints, macaddr) {
		const leases = Array.isArray(dhcp_leases.dhcp_leases) ? dhcp_leases.dhcp_leases : [];
		const leases6 = Array.isArray(dhcp_leases.dhcp6_leases) ? dhcp_leases.dhcp6_leases : [];
		if (leases.length == 0 && leases6.length == 0)
			return E('em', _('No active leases found'));
		const machints = host_hints.getMACHints(false);
		const isReadonlyView = !L.hasViewPermission();

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

		const table = E('table', { 'id': 'status_leases', 'class': 'table leases' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				L.hasSystemFeature('odhcpd', 'dhcpv4') ? E('th', { 'class': 'th' }, _('Interface')) : E([]),
				E('th', { 'class': 'th', 'style': 'text-align:left' }, _('Hostname')),
				E('th', { 'class': 'th' }, _('IPv4 address')),
				E('th', { 'class': 'th' }, _('MAC address')),
				E('th', { 'class': 'th' }, _('Upload')),
				E('th', { 'class': 'th' }, _('Download')),
				E('th', { 'class': 'th' }, _('Remaining time')),
				isReadonlyView ? E([]) : E('th', { 'class': 'th cbi-section-actions' }, _('Static Lease'))
			])
		]);

		cbi_update_table(table, leases.map(L.bind(function(lease) {
			let exp;
			let vendor;

			if (lease.expires === false)
				exp = E('em', _('unlimited'));
			else if (lease.expires <= 0)
				exp = E('em', _('expired'));
			else
				exp = '%t'.format(lease.expires);

			const hint = lease.macaddr ? machints.filter(function(h) { return h[0] == lease.macaddr })[0] : null;
			let host = null;

			if (hint && lease.hostname && lease.hostname != hint[1])
				host = '%s (%s)'.format(lease.hostname, hint[1]);
			else if (lease.hostname)
				host = lease.hostname;

			if (macaddr)
				vendor = macaddr[lease.macaddr.toLowerCase()]?.vendor ?? null;

			const columns = [
				this.renderHostname(host),
				lease.ipaddr,
				vendor ? lease.macaddr + ` (${vendor})` : lease.macaddr,
				this.rateCell(lease, host_hints, 'upload'),
				this.rateCell(lease, host_hints, 'download'),
				exp,
			];

			if (L.hasSystemFeature('odhcpd', 'dhcpv4'))
				columns.unshift(lease.interface || '-');

			if (!isReadonlyView && lease.macaddr != null) {
				columns.push(E('button', {
					'class': 'cbi-button cbi-button-apply',
					'click': L.bind(this.handleCreateStaticLease, this, lease),
					'data-tooltip': _('Reserve a specific IP address for this device'),
					'disabled': this.isMACStatic[lease.macaddr.toLowerCase()]
				}, [ _('Reserve IP') ]));
			}

			return columns;
		}, this)), E('em', _('No active leases found')));

		const table6 = E('table', { 'id': 'status_leases6', 'class': 'table leases6' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				L.hasSystemFeature('odhcpd', 'dhcpv6') ? E('th', { 'class': 'th' }, _('Interface')) : E([]),
				E('th', { 'class': 'th', 'style': 'text-align:left' }, _('Hostname')),
				E('th', { 'class': 'th' }, _('IPv6 addresses')),
				E('th', { 'class': 'th' }, _('Upload')),
				E('th', { 'class': 'th' }, _('Download')),
				E('th', { 'class': 'th' }, _('Remaining time')),
				isReadonlyView ? E([]) : E('th', { 'class': 'th cbi-section-actions' }, _('Static Lease'))
			])
		]);

		cbi_update_table(table6, leases6.map(L.bind(function(lease) {
			let exp;

			if (lease.expires === false)
				exp = E('em', _('unlimited'));
			else if (lease.expires <= 0)
				exp = E('em', _('expired'));
			else
				exp = '%t'.format(lease.expires);

			const hint = lease.macaddr ? machints.filter(function(h) { return h[0] == lease.macaddr })[0] : null;
			let host = null;

			if (hint && lease.hostname && lease.hostname != hint[1] && lease.ip6addr != hint[1])
				host = '%s (%s)'.format(lease.hostname, hint[1]);
			else if (lease.hostname)
				host = lease.hostname;
			else if (hint)
				host = hint[1];

			const duid = lease.duid?.toLowerCase();
			const iaid = lease.iaid?.toLowerCase();

			// Note: "disabled: false" doesn't work
			let disabled = null;
			if (!duid)
				disabled = true;
			else if (duid && this.isDUIDStatic[duid])
				disabled = true;
			else if (duid && iaid && this.isDUIDIAIDStatic[`${duid}%${iaid}`])
				disabled = true;

			const columns = [
				this.renderHostname(host),
				lease.ip6addrs ? lease.ip6addrs.join('<br />') : lease.ip6addr,
				this.rateCell(lease, host_hints, 'upload'),
				this.rateCell(lease, host_hints, 'download'),
				exp
			];

			if (L.hasSystemFeature('odhcpd', 'dhcpv6'))
				columns.unshift(lease.interface || '-');

			if (!isReadonlyView && lease.duid) {
				columns.push(E('button', {
					'class': 'cbi-button cbi-button-apply',
					'click': L.bind(this.handleCreateStaticLease6, this, lease),
					'data-tooltip': _('Reserve a specific IP address for this device'),
					'disabled': disabled
				}, [ _('Reserve IP') ]));
			}

			return columns;
		}, this)), E('em', _('No active leases found')));

		return E([
			E('h3', _('Active DHCPv4 Leases')),
			table,
			E('h3', _('Active DHCPv6 Leases')),
			table6
		]);
	},

});
