'use strict';
'require baseclass';
'require rpc';

var callRtl837xStatus = rpc.declare({
	object: 'rtl837x',
	method: 'get_status',
	expect: { '': {} }
});

var callRtl837xSfp = rpc.declare({
	object: 'rtl837x',
	method: 'get_sfp_info',
	expect: { '': {} }
});

function formatSpeed(speed, duplex) {
	if (!speed || speed === '')
		return _('-');

	var s = speed.replace(/baseT.*/i, '');
	if (s === '10000')
		s = '10 Gbps';
	else if (s === '5000')
		s = '5 Gbps';
	else if (s === '2500')
		s = '2.5 Gbps';
	else if (s === '1000')
		s = '1 Gbps';
	else if (s === '100')
		s = '100 Mbps';
	else if (s === '10')
		s = '10 Mbps';
	else
		s = speed;

	if (duplex === 'full')
		s += ' ' + _('Full');
	else if (duplex === 'half')
		s += ' ' + _('Half');

	return s;
}

function buildSfpCell(sfp) {
	if (!sfp || !sfp.present) {
		return E('span', {
			'style': 'color: #888; font-style: italic;'
		}, _('Empty / Not Present'));
	}

	var vendor = sfp.vendor || _('Generic');
	var part = sfp.part || '';
	var serial = sfp.serial || '';
	var hasDdm = sfp.ddm_supported;

	var tooltipLines = [
		_('Vendor') + ': ' + vendor,
		_('Part Number') + ': ' + (part || '-'),
		_('Serial') + ': ' + (serial || '-')
	];

	if (hasDdm) {
		tooltipLines.push('--- ' + _('DDM Optical Diagnostics') + ' ---');
		if (sfp.temperature)
			tooltipLines.push(_('Temperature') + ': ' + sfp.temperature);
		if (sfp.voltage)
			tooltipLines.push(_('Supply Voltage') + ': ' + sfp.voltage);
		if (sfp.tx_bias)
			tooltipLines.push(_('TX Bias Current') + ': ' + sfp.tx_bias);
		if (sfp.tx_power)
			tooltipLines.push(_('TX Optical Power') + ': ' + sfp.tx_power);
		if (sfp.rx_power)
			tooltipLines.push(_('RX Optical Power') + ': ' + sfp.rx_power);
	} else {
		tooltipLines.push(_('DDM') + ': ' + _('Passive DAC or Non-DDM module'));
	}

	var tooltipText = tooltipLines.join('\n');

	var inlineDdm = [];
	if (hasDdm) {
		if (sfp.temperature)
			inlineDdm.push('🌡️ ' + sfp.temperature);
		if (sfp.tx_power)
			inlineDdm.push('↗️ ' + sfp.tx_power.split(' ')[0]);
		if (sfp.rx_power)
			inlineDdm.push('↘️ ' + sfp.rx_power.split(' ')[0]);
	} else {
		inlineDdm.push(_('DAC / Non-DDM'));
	}

	return E('span', {
		'class': 'cbi-tooltip-container',
		'data-tooltip': tooltipText,
		'title': tooltipText,
		'style': 'cursor: help; display: inline-flex; flex-direction: column; gap: 2px;'
	}, [
		E('span', {
			'style': 'font-weight: bold; color: #1565c0;'
		}, '📦 ' + vendor + (part ? ' ' + part : '')),
		E('span', {
			'style': 'font-size: 85%; color: #388e3c;'
		}, inlineDdm.join(' | '))
	]);
}

return L.Class.extend({
	title: _('RTL8372 Switch & SFP Status'),

	load: function() {
		return Promise.all([
			L.resolveDefault(callRtl837xStatus(), {}),
			L.resolveDefault(callRtl837xSfp(), {})
		]);
	},

	render: function(data) {
		var status = data[0] || {};
		var globalSfp = data[1] || {};

		// If switch device is not present, hide the card
		if (!status || status.error || !Array.isArray(status.ports))
			return null;

		var chip = status.chip || 'RTL8372';
		var swName = status.switch || 'switch0';

		var summary = E('div', {
			'style': 'margin-bottom: 12px; display: flex; flex-wrap: wrap; gap: 16px; font-size: 90%;'
		}, [
			E('span', {}, [
				E('strong', {}, _('Switch Chip: ')),
				chip + ' (' + swName + ')'
			]),
			E('span', {}, [
				E('strong', {}, _('CPU Port: ')),
				'Port ' + status.cpu_port + ' (10G-KR)'
			]),
			E('span', {}, [
				E('strong', {}, _('IGMP Snooping: ')),
				status.igmp_snooping ? '✔️ ' + _('Active') : '❌ ' + _('Disabled')
			]),
			E('span', {}, [
				E('strong', {}, _('IGMP Fast Leave: ')),
				status.igmp_fast_leave ? '✔️ ' + _('Active') : '❌ ' + _('Disabled')
			])
		]);

		var table = E('table', { 'class': 'table' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				E('th', { 'class': 'th' }, _('Port')),
				E('th', { 'class': 'th' }, _('Name')),
				E('th', { 'class': 'th' }, _('Status')),
				E('th', { 'class': 'th' }, _('Speed / Duplex')),
				E('th', { 'class': 'th' }, _('MTU')),
				E('th', { 'class': 'th' }, _('Port Information & SFP DDM Sensors'))
			])
		]);

		cbi_update_table(table, status.ports.map(function(p) {
			var portBadge;
			if (p.link === 'up') {
				portBadge = E('span', {
					'class': 'badge badge-positive',
					'style': 'background-color: #2e7d32; color: #fff; padding: 2px 8px; border-radius: 4px; font-weight: bold; font-size: 85%;'
				}, _('Link Up'));
			} else {
				portBadge = E('span', {
					'class': 'badge',
					'style': 'background-color: #757575; color: #fff; padding: 2px 8px; border-radius: 4px; font-size: 85%;'
				}, _('Link Down'));
			}

			var portSpeed = formatSpeed(p.speed, p.duplex);
			var mtuStr = (p.max_frame_size || 1518) + ' B';

			var detailNode;
			if (p.is_sfp) {
				var sfpObj = p.sfp || globalSfp;
				detailNode = buildSfpCell(sfpObj);
			} else if (p.port === status.cpu_port) {
				detailNode = E('span', {
					'style': 'color: #555;'
				}, _('Internal CPU SerDes Interconnect'));
			} else {
				var copperItems = [_('Copper (2.5G Base-T)')];
				copperItems.push(p.eee ? _('EEE On') : _('EEE Off'));
				copperItems.push((p.stp_state === 3) ? _('STP Fwd') : _('STP Block'));
				if (p.storm_bcast)
					copperItems.push(_('Storm Filter'));
				if (p.isolation)
					copperItems.push(_('Isolated'));
				detailNode = E('span', {
					'style': 'color: #555; font-size: 90%;'
				}, copperItems.join(' | '));
			}

			return [
				'Port ' + p.port,
				p.name || ('Port ' + p.port),
				portBadge,
				portSpeed,
				mtuStr,
				detailNode
			];
		}));

		return E('div', {}, [
			summary,
			table
		]);
	}
});
