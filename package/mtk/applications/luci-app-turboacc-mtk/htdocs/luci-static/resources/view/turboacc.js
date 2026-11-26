/* Copyright (C) 2022 ImmortalWrt.org */

'use strict';
'require dom';
'require form';
'require poll';
'require rpc';
'require uci';
'require view';

var getSystemFeatures = rpc.declare({
	object: 'luci.turboacc',
	method: 'getSystemFeatures',
	expect: { '': {} }
});

var getFastPathStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFastPathStat',
	expect: { '': {} },
	reject: true
});

var getFullConeStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFullConeStat',
	expect: { '': {} },
	reject: true
});

var getTCPCCAStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getTCPCCAStat',
	expect: { '': {} },
	reject: true
});

var getMTKPPEStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getMTKPPEStat',
	expect: { '': {} },
	reject: true
});

var getMultiTierTelemetryStatus = rpc.declare({
	object: 'luci.turboacc',
	method: 'getMultiTierTelemetryStatus',
	expect: { '': {} },
	reject: true
});

var getRuntimeStatus = rpc.declare({
	object: 'luci.turboacc',
	method: 'getRuntimeStatus',
	expect: { '': {} },
	reject: true
});

function queryStatus(fn) {
	return Promise.resolve().then(fn).then(function(data) {
		return { ok: true, data: data || {} };
	}, function() {
		return { ok: false };
	});
}

function trimValue(value) {
	return value == null ? '' : String(value).replace(/^\s+|\s+$/g, '');
}

function normalizeFastpathValue(value) {
	value = trimValue(value);

	return (!value || value === 'none') ? 'disabled' : value;
}

function parseTokenList(value) {
	var tokens = trimValue(value).split(/\s+/).filter(function(token) {
		return token !== '';
	});

	tokens.sort();
	return tokens;
}

function getRuntimeLabel(token) {
	token = trimValue(token);

	switch (token) {
	case 'Flow Offloading':
	case 'Flow offloading':
		return _('Flow offloading');
	case 'Fast classifier':
		return _('Fast classifier');
	case 'Shortcut-FE CM':
		return _('SFE connection manager');
	case 'MediaTek HWNAT':
	case 'MediaTek HNAT':
		return _('MediaTek HNAT');
	case 'nft_fullcone':
		return _('NFT_FULLCONE_NAT');
	case 'xt_FULLCONENAT':
		return _('XT_FULLCONE_NAT');
	case 'Boardcom Fullcone':
		return _('Boardcom_FULLCONE_NAT');
	case 'HNAT Disabled':
	case 'Ethernet HNAT Disabled':
		return _('Global HNAT (wired & wireless WHNAT) disabled');
	case 'HNAT Partially Enabled':
		return _('HNAT Partially Enabled');
	default:
		return token;
	}
}

function progressbar(value, max, byte) {
	var vn = parseInt(value, 10) || 0,
		mn = parseInt(max, 10) || 0,
		fv = byte ? String.format('%1024.2mB', vn) : vn,
		fm = byte ? String.format('%1024.2mB', mn) : mn,
		pc = mn > 0 ? Math.min(100, Math.floor((100 * vn) / mn)) : 0;

	return E('div', {
		'class': 'cbi-progressbar',
		'title': '%s / %s (%d%%)'.format(fv, fm, pc)
	}, E('div', { 'style': 'width:%.2f%%'.format(pc) }));
}

function statusLabel(color, label) {
	var style = color === 'none' ? 'color:inherit' : 'color:%s'.format(color);
	return E('em', {}, E('span', { 'style': style }, E('strong', {}, label)));
}

function renderStatus(result) {
	if (!result || !result.ok)
		return statusLabel('red', _('Status unavailable'));

	var stat = result.data || {};
	if (!stat.type)
		return statusLabel('red', _('Disabled'));

	if (stat.type.includes(' / ')) {
		var types = stat.type.split(' / ');
		var nodes = [ statusLabel('green', getRuntimeLabel(types[0])) ];
		for (var i of types.slice(1))
			nodes.push(' / ', statusLabel('red', getRuntimeLabel(i)));
		return E('span', {}, nodes);
	}

	var color = (stat.type === 'HNAT Disabled' || stat.type === 'Ethernet HNAT Disabled') ? 'red' : 'green';
	return statusLabel(color, getRuntimeLabel(stat.type));
}

function renderServiceResult(result) {
	if (!result || !result.ok)
		return statusLabel('red', _('Status unavailable'));

	var status = result.data || {};
	if (!status.available)
		return statusLabel('none', _('No service result is available'));

	switch (status.state) {
	case 'starting':
		return statusLabel('orange', _('TurboACC service apply is in progress'));
	case 'ok':
		return statusLabel('green', _('TurboACC service apply succeeded'));
	case 'stopped':
		return statusLabel('none', _('TurboACC service is stopped'));
	case 'error':
		return statusLabel('red', _('TurboACC service apply failed: %s').format(status.message || _('See the system log')));
	default:
		return statusLabel('none', _('No service result is available'));
	}
}

function syncPPERows(tableEl, ppeResult) {
	if (!tableEl)
		return;

	var ppeStats = (ppeResult && ppeResult.ok && ppeResult.data) ? ppeResult.data : {};
	var ppeNum = (ppeStats.available === false) ? 0 : (parseInt(ppeStats.PPE_NUM, 10) || 0);
	var existing = tableEl.querySelectorAll('tr[data-ppe-row="1"]');
	for (var idx = ppeNum; idx < existing.length; idx++) {
		if (existing[idx] && existing[idx].parentNode)
			existing[idx].parentNode.removeChild(existing[idx]);
	}

	for (var i = 0; i < ppeNum; i++) {
		var cellId = 'ppe' + i + '_entry';
		var cell = tableEl.querySelector('#' + cellId);
		var bar = progressbar(ppeStats['BIND_PPE' + i], ppeStats['ALL_PPE' + i]);
		if (cell) {
			dom.content(cell, bar);
		} else {
			tableEl.appendChild(E('tr', { 'data-ppe-row': '1' }, [
				E('td', { 'width': '33%' }, 'PPE' + i + ' ' + _('Diagnostic state entries')),
				E('td', { 'id': cellId }, [ bar ])
			]));
		}
	}
}

function toInt(value) {
	var n = parseInt(value, 10);
	return isNaN(n) ? 0 : n;
}

function syncMultiTierRows(tableEl, result) {
	if (!tableEl)
		return;

	var mt = result && result.ok ? (result.data || {}) : {};
	var telemetryAvailable = !!(result && result.ok && mt.available);
	var unavailableText = result == null ? _('Collecting data...') :
		(result.ok ? _('Telemetry unavailable') : _('Status unavailable'));
	var bind = toInt(mt.tier1_ppe_bind);
	var total = toInt(mt.tier1_ppe_total);
	var permille = toInt(mt.tier1_occupancy_permille);
	var swFlows = toInt(mt.tier2_sw_flows);
	var swPkts = toInt(mt.tier2_sw_packets);
	var swBytes = toInt(mt.tier2_sw_bytes);
	var defers = toInt(mt.watermark_defers);
	var backoffs = toInt(mt.hw_fail_backoffs);
	var roams = toInt(mt.roam_flushes);
	var serEpoch = toInt(mt.ser_epoch);

	var rows = [
		{
			id: 'mt_tier1_occ',
			label: _('Tier-1 PPE Occupancy'),
			text: telemetryAvailable && mt.ppe_occupancy_available
				? '%d / %d (%d.%d%%)'.format(bind, total, Math.floor(permille / 10), permille % 10)
				: (telemetryAvailable ? _('PPE occupancy unavailable') : unavailableText)
		},
		{
			id: 'mt_tier2_stat',
			label: _('Tier-2 SW Fallback'),
			text: telemetryAvailable ? '%d %s / %d %s / %s'.format(swFlows, _('flows'), swPkts, _('packets'), String.format('%1024.2mB', swBytes)) : unavailableText
		},
		{
			id: 'mt_events_stat',
			label: _('Multi-Tier Events'),
			text: telemetryAvailable ? '%s: %d / %s: %d / %s: %d / %s: %d'.format(
				_('Defers'), defers,
				_('Backoffs'), backoffs,
				_('Roam Flushes'), roams,
				_('SER Epoch'), serEpoch
			) : unavailableText
		}
	];

	for (var i = 0; i < rows.length; i++) {
		var row = rows[i];
		var cell = tableEl.querySelector('#' + row.id);
		if (cell) {
			dom.content(cell, row.text);
		} else {
			tableEl.appendChild(E('tr', { 'data-mt-row': '1' }, [
				E('td', { 'width': '33%' }, row.label),
				E('td', { 'id': row.id }, row.text)
			]));
		}
	}
}

function applyStatusSnapshot(tableEl, res) {
	if (!tableEl || !res)
		return;

	var ids = [ 'fastpath_state', 'fullcone_state', 'tcpcca_state' ];
	for (var i = 0; i < ids.length; i++) {
		var node = tableEl.querySelector('#' + ids[i]);
		if (node)
			dom.content(node, renderStatus(res[i]));
	}

	var serviceNode = tableEl.querySelector('#service_apply_state');
	if (serviceNode)
		dom.content(serviceNode, renderServiceResult(res[4]));

	syncMultiTierRows(tableEl, res[3]);
	syncPPERows(tableEl, res[5]);
}

function updateTurboaccStatus() {
	var tableEl = document.getElementById('turboacc_status');
	if (!tableEl)
		return;

	return Promise.all([
		queryStatus(getFastPathStat),
		queryStatus(getFullConeStat),
		queryStatus(getTCPCCAStat),
		queryStatus(getMultiTierTelemetryStatus),
		queryStatus(getRuntimeStatus),
		queryStatus(getMTKPPEStat)
	]).then(function(res) {
		applyStatusSnapshot(tableEl, res);
	});
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('turboacc'),
			L.resolveDefault(getSystemFeatures(), {}),
			queryStatus(getFastPathStat),
			queryStatus(getFullConeStat),
			queryStatus(getTCPCCAStat),
			queryStatus(getMultiTierTelemetryStatus),
			queryStatus(getRuntimeStatus),
			queryStatus(getMTKPPEStat)
		]);
	},

	render: function(data) {
		var m, s, o;
		var features = data[1] || {};
		var initialStatus = data.slice(2);
		var savedFastpath = normalizeFastpathValue(uci.get('turboacc', 'config', 'fastpath'));
		var savedFullcone = trimValue(uci.get('turboacc', 'config', 'fullcone'));
		var savedTcpcca = trimValue(uci.get('turboacc', 'config', 'tcpcca')) || 'cubic';
		var legacyFlowOffloading = !features.hasFLOWOFFLOADING && savedFastpath === 'flow_offloading';

		m = new form.Map('turboacc', _('TurboACC settings'),
			_('Open source flow offloading engine (fast path or hardware NAT).'));

		s = m.section(form.TypedSection);
		s.anonymous = true;
		s.render = function () {
			var acc_status = E('table', { 'id': 'turboacc_status', 'class': 'table', 'width': '100%', 'cellspacing': '10' }, [
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('FastPath Engine')),
					E('td', { 'id': 'fastpath_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('Full Cone NAT')),
					E('td', { 'id': 'fullcone_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('TCP CCA')),
					E('td', { 'id': 'tcpcca_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('Last TurboACC service run')),
					E('td', { 'id': 'service_apply_state' }, E('em', {}, _('Collecting data...')))
				])
			]);

			applyStatusSnapshot(acc_status, initialStatus);
			poll.add(updateTurboaccStatus, 3);

			return E('fieldset', { 'class': 'cbi-section' }, [
				E('legend', {}, _('Acceleration Status')),
				acc_status
			]);
		};

		s = m.section(form.NamedSection, 'config', 'turboacc');

		o = s.option(form.ListValue, 'fastpath', _('Fastpath engine'),
			_('The offloading engine for routing/NAT.'));
		o.value('disabled', _('Disable'));
		if (features.hasFLOWOFFLOADING || legacyFlowOffloading)
			o.value('flow_offloading', legacyFlowOffloading
				? _('Flow offloading (unsupported legacy)')
				: _('Flow offloading'));
		if (features.hasFASTCLASSIFIER)
			o.value('fast_classifier', _('Fast classifier'));
		if (features.hasSHORTCUTFECM)
			o.value('shortcut_fe_cm', _('SFE connection manager'));
		if (features.hasMEDIATEKHNAT || savedFastpath === 'mediatek_hnat')
			o.value('mediatek_hnat', _('MediaTek HNAT'));
		o.default = 'disabled';
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return normalizeFastpathValue(uci.get('turboacc', section_id, 'fastpath'));
		};
		o.write = function(section_id, value) {
			return uci.set('turboacc', section_id, 'fastpath', value === 'disabled' ? 'none' : value);
		};
		o.onchange = function(ev, section_id, value) {
			var desc = ev.target && ev.target.nextElementSibling;
			if (!desc)
				return;
			if (value === 'flow_offloading')
				desc.innerHTML = _('Software based offloading for routing/NAT.');
			else if (value === 'fast_classifier')
				desc.innerHTML = _('Fast classifier connection manager for the shortcut forwarding engine.');
			else if (value === 'shortcut_fe_cm')
				desc.innerHTML = _('Simple connection manager for the shortcut forwarding engine.');
			else if (value === 'mediatek_hnat')
				desc.innerHTML = _('MediaTek\'s open source hardware offloading engine.');
			else
				desc.innerHTML = _('The offloading engine for routing/NAT.');
		};

		if (features.hasFLOWOFFLOADING) {
			o = s.option(form.Flag, 'fastpath_fo_hw', _('Hardware flow offloading'),
				_('Requires hardware NAT support. Implemented at least for mt7621.'));
			o.default = o.disabled;
			o.rmempty = false;
			o.depends('fastpath', 'flow_offloading');
		}

		o = s.option(form.Flag, 'fastpath_fc_br', _('Bridge Acceleration'),
			_('Enable bridge acceleration (may be functional conflict with bridge-mode VPN server).'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'fast_classifier');

		if (features.hasIPV6) {
			o = s.option(form.Flag, 'fastpath_fc_ipv6', _('IPv6 acceleration'),
				_('Enable IPv6 Acceleration.'));
			o.default = o.disabled;
			o.rmempty = false;
			o.depends('fastpath', 'fast_classifier');
		}

		o = s.option(form.Flag, 'fastpath_mh_eth_hnat', _('Enable global HNAT (wired & wireless WHNAT)'),
			_('Enable shared hardware offloading hooks for both wired Ethernet and wireless WHNAT connections.'));
		o.rmempty = false;
		o.default = '1';
		o.depends('fastpath', 'mediatek_hnat');

		o = s.option(form.Flag, 'fastpath_mh_eth_hnat_v6', _('Enable IPv6 HNAT'),
			_('Enable hardware offloading for IPv6 connections when global HNAT is enabled.'));
		o.rmempty = false;
		o.default = '1';
		o.depends({ fastpath: 'mediatek_hnat', fastpath_mh_eth_hnat: '1' });

		o = s.option(form.Value, 'fastpath_mh_eth_hnat_bind_rate', _('HNAT bind rate threshold (pps)'),
			_('The smaller the threshold, the easier it is for the connection to be accelerated.'));
		o.optional = true;
		o.datatype = 'range(1,30)';
		o.default = '5';
		o.placeholder = 5;
		o.depends({ fastpath: 'mediatek_hnat', fastpath_mh_eth_hnat: '1' });

		o = s.option(form.Flag, 'fastpath_mh_sw_fallback', _('Enable software flowtable fallback'),
			_('Keep flows in software fastpath via firewall4 flowtable when PPE occupancy reaches the 80% watermark or hardware offload defers.'));
		o.rmempty = false;
		o.default = '1';
		o.depends({ fastpath: 'mediatek_hnat', fastpath_mh_eth_hnat: '1' });

		o = s.option(form.ListValue, 'fullcone', _('Full cone NAT'),
			_('Full cone NAT (NAT1) can improve gaming performance effectively.'));
		o.value('0', _('Disable'));
		if (features.hasNFTFULLCONE || features.hasXTFULLCONENAT || savedFullcone === '1')
			o.value('1', features.hasNFTFULLCONE
				? _('NFT_FULLCONE_NAT (Recommended / HNAT compatible)')
				: _('XT_FULLCONE_NAT'));
		if (features.hasBCMFullcone || savedFullcone === '2')
			o.value('2', _('Boardcom_FULLCONE_NAT (Kernel expectation / v4+v6 HNAT compatible)'));
		o.default = features.hasNFTFULLCONE ? '1' : (features.hasBCMFullcone ? '2' : (features.hasXTFULLCONENAT ? '1' : '0'));
		o.rmempty = false;

		if (features.hasIPV6 && (features.hasNFTFULLCONE || savedFullcone === '1')) {
			o = s.option(form.Flag, 'fullcone6', _('Enable IPv6 Full Cone NAT'),
				_('Enable nftables Full Cone NAT for IPv6 masqueraded zones (usually unnecessary for IPv6).'));
			o.default = '0';
			o.rmempty = false;
			o.depends('fullcone', '1');
		}

		var tcpccaList = parseTokenList(features.hasTCPCCA || 'cubic');
		if (savedTcpcca && tcpccaList.indexOf(savedTcpcca) < 0)
			tcpccaList.push(savedTcpcca);

		o = s.option(form.ListValue, 'tcpcca', _('TCP CCA'),
			_('TCP congestion control algorithm.'));
		for (var cca of tcpccaList)
			o.value(cca);
		o.default = 'cubic';
		o.rmempty = false;

		return m.render();
	}
});
