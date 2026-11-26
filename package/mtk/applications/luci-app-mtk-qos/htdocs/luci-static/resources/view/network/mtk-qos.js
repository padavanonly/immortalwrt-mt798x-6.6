/* Copyright (C) 2026 ImmortalWrt.org */

'use strict';
'require form';
'require poll';
'require rpc';
'require view';

var callGetStatus = rpc.declare({
	object: 'luci.mtk_qos',
	method: 'getStatus',
	expect: { '': {} }
});

var callGetHints = rpc.declare({
	object: 'luci.mtk_qos',
	method: 'getHints',
	expect: { '': {} }
});

function fmtPair(pair) {
	if (!Array.isArray(pair) || pair.length < 2)
		return '0 / 0';
	return (pair[0] || 0) + ' / ' + (pair[1] || 0);
}

function fmtSch(sch) {
	if (!sch)
		return '--';
	return (sch.enabled ? _('Enabled') : _('Disabled')) +
		' (' + (sch.alg || 'WRR') + ', ' + (sch.rate || 0) + ' kbit/s)';
}

function fmtWanKind(kind) {
	switch (kind) {
	case 'cellular_5g':
		return _('5G Cellular (WWAN EXT)');
	case 'dual_wan':
		return _('Dual-WAN (Wired + 5G)');
	case 'wired':
		return _('Wired WAN (Ethernet / PPPoE)');
	default:
		return kind || '--';
	}
}

function updateStatusDOM(st) {
	if (!st || st.error)
		return;

	var elState = document.getElementById('mtk-qos-state');
	var elApply = document.getElementById('mtk-qos-last-apply');
	var elWan = document.getElementById('mtk-qos-wan');
	var elCoproc = document.getElementById('mtk-qos-coproc');
	var elDl = document.getElementById('mtk-qos-dl');
	var elUl = document.getElementById('mtk-qos-ul');
	var elDpi = document.getElementById('mtk-qos-dpi');

	if (elState) {
		if (st.conflict)
			elState.textContent = _('Blocked (Legacy EQoS / mtkhqos active)');
		else if (st.active && st.mode === 'coprocessor_4x')
			elState.textContent = _('Active (4-Coprocessor Zero-CPU: PCE + PPE + QDMA + WED)');
		else if (st.active)
			elState.textContent = _('Active (Software CAKE Fallback)');
		else
			elState.textContent = _('Stopped');
	}

	if (elApply)
		elApply.textContent = st.last_apply === 'success' ? _('Success') :
			(st.last_apply === 'failed' ? _('Failed') : '--');

	if (elWan)
		elWan.textContent = (st.wan_devs || st.wan_dev || '--') + ' [' + fmtWanKind(st.wan_kind) + ']';

	if (elCoproc) {
		var pce = st.pce || {};
		var pceText;
		if (pce.supported === 0)
			pceText = _('PPE FOE QID Hardware Lock (MT7987)');
		else
			pceText = pce.loaded ? (_('Loaded') + ' (' + (pce.active_cls || 0) + ' CLS)') : _('Unloaded');
		elCoproc.textContent =
			'PPE ' + _('Bound') + ': ' + (st.ppe_bound != null ? st.ppe_bound : '--') +
			(st.ppe_total > 0 ? '/' + st.ppe_total : '') + ' (@' + (st.fast_bind_pkt || 5) + ' pps)' +
			' | PCE: ' + pceText +
			' | WED: ' + (st.wed_present ? _('Active') : _('Inactive')) +
			' | hook=' + (st.hook_toggle ? 1 : 0) +
			' qos=' + (st.qos_toggle || 0) +
			' sqm=' + (st.sqm_toggle ? 1 : 0);
	}

	var q = st.queues || {};
	if (elDl) {
		elDl.textContent = fmtSch(st.sch3) +
			' | EQoS Q33-47 [' + fmtPair(q.dl_eqos) + ']' +
			' | RT Q48 [' + fmtPair(q.dl_rt) + ']' +
			' | Fair Q49-61 [' + fmtPair(q.dl_fair) + ']' +
			' | Video Q62 [' + fmtPair(q.dl_video) + ']' +
			' | P2P Q63 [' + fmtPair(q.dl_p2p) + ']';
	}

	if (elUl) {
		elUl.textContent = fmtSch(st.sch2) +
			' | EQoS Q1-15 [' + fmtPair(q.ul_eqos) + ']' +
			' | RT Q16 [' + fmtPair(q.ul_rt) + ']' +
			' | Fair Q17-29 [' + fmtPair(q.ul_fair) + ']' +
			' | Video Q30 [' + fmtPair(q.ul_video) + ']' +
			' | P2P Q31 [' + fmtPair(q.ul_p2p) + ']';
	}

	var ds = st.dpi_stats || {};
	if (elDpi) {
		elDpi.textContent =
			_('Realtime') + ': ' + (ds.realtime_pkts || 0) +
			' | ' + _('Flow-Fair') + ': ' + (ds.fair_pkts || 0) +
			' | ' + _('Video/QUIC') + ': ' + (ds.video_pkts || 0) +
			' | ' + _('P2P/BT') + ': ' + (ds.p2p_pkts || 0) +
			' | ' + _('EQoS Flows') + ': ' + (ds.eqos_flows || 0) +
			((st.cake && st.cake.length > 0) ? (' | CAKE: ' + st.cake) : '');
	}
}

function pollStatus() {
	if (!document.getElementById('mtk-qos-state'))
		return Promise.resolve();

	return callGetStatus().then(updateStatusDOM);
}

return view.extend({
	load: function() {
		return Promise.all([
			callGetStatus(),
			callGetHints()
		]);
	},

	render: function(data) {
		var status = (data && data[0]) || {};
		var hints = (data && data[1] && data[1].hosts) || [];

		var m = new form.Map(
			'mtk_qos',
			_('MediaTek NPU 4-Coprocessor Unified QoS'),
			_('Simultaneous hardware EQoS per-host shaping, connection limiting, zero-copy L7 DPI classification, and 13-queue SQM via MT7987/MT7988 PCE + PPE + QDMA + WED across Wired WAN and 5G WWAN.')
		);
		var s = m.section(form.NamedSection, '__status', '_status', _('Realtime 4-Coprocessor Status'));
		s.render = function() {
			var statusTable = E('table', { 'class': 'table' }, [
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('Engine Mode')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-state' }, '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('Last Apply')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-last-apply' }, '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('Bound WAN Interface(s)')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-wan' }, '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('Coprocessor Health')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-coproc' }, '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('Downlink QDMA (Sch3 / Q33-63)')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-dl' }, '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('Uplink QDMA (Sch2 / Q1-31)')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-ul' }, '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '28%' }, _('L7 Hardware DPI & EQoS Hits')),
					E('td', { 'class': 'td left', 'id': 'mtk-qos-dpi' }, '--')
				])
			]);

			var wrap = E('fieldset', { 'class': 'cbi-section' }, [
				E('legend', {}, _('Realtime 4-Coprocessor Status')),
				statusTable
			]);
			window.setTimeout(function() { updateStatusDOM(status); }, 0);
			return wrap;
		};

		s = m.section(form.NamedSection, 'global', 'global', _('Global & Coprocessor Policy'));
		s.tab('engine', _('Coprocessor Engine'));
		s.tab('sqm', _('SQM & L7 DPI Classes'));

		var o = s.taboption('engine', form.Flag, 'enabled', _('Enable Unified MTK QoS'));
		o.rmempty = false;

		o = s.taboption('engine', form.ListValue, 'wan_mode', _('WAN Topology Mode'));
		o.value('auto', _('Auto Detect (Wired WAN / 5G WWAN / Dual-WAN)'));
		o.value('wired', _('Wired Ethernet / PPPoE / DSA'));
		o.value('cellular_5g', _('5G Cellular QMI (wwan0_1)'));
		o.value('dual_wan', _('Dual-WAN Active'));
		o.default = 'auto';
		o.rmempty = false;

		o = s.taboption('engine', form.Value, 'interface', _('Bound WAN Device'));
		o.value('auto', _('Auto Detect Active Default Routes'));
		o.value('eth1', 'eth1 (Wired Ethernet WAN)');
		o.value('pppoe-wan', 'pppoe-wan (PPPoE WAN)');
		o.value('wwan0_1', 'wwan0_1 (5G QMI RM520F)');
		o.value('usb0', 'usb0 (USB NCM/RNDIS)');
		o.default = 'auto';
		o.rmempty = false;

		o = s.taboption('engine', form.Value, 'download_kbps', _('Total Download Bandwidth (kbit/s)'));
		o.datatype = 'range(0,10000000)';
		o.default = '300000';
		o.rmempty = false;
		o.description = _('Recommended: 85-95% of measured line speed. Set 0 to only enforce per-host EQoS limits.');

		o = s.taboption('engine', form.Value, 'upload_kbps', _('Total Upload Bandwidth (kbit/s)'));
		o.datatype = 'range(0,10000000)';
		o.default = '50000';
		o.rmempty = false;

		o = s.taboption('engine', form.Value, 'overhead', _('Link Layer Overhead (bytes)'));
		o.datatype = 'range(0,128)';
		o.default = '0';
		o.rmempty = false;
		o.description = _('Typical: 0 (Raw IP/Ethernet), 8 (PPPoE), 18 (Ethernet+FCS), 44 (VDSL/ATM).');

		o = s.taboption('engine', form.ListValue, 'scheduler', _('QDMA Hardware Scheduler'));
		o.value('wrr', _('WRR (Weighted Round Robin)'));
		o.value('sp', _('SP (Strict Priority)'));
		o.default = 'wrr';
		o.rmempty = false;

		o = s.taboption('engine', form.Flag, 'pce_enabled', _('Enable PCE Hardware L4/L7 Classifier'));
		o.default = '1';
		o.rmempty = false;

		o = s.taboption('engine', form.Flag, 'dpi_enabled', _('Enable Zero-Copy L7 DPI Engine'));
		o.default = '1';
		o.rmempty = false;

		o = s.taboption('sqm', form.Flag, 'sqm_enabled', _('Enable SQM Fair & Prioritized Queueing'));
		o.default = '1';
		o.rmempty = false;

		o = s.taboption('sqm', form.Value, 'realtime_pct', _('Realtime / Gaming / VoIP Floor (%)'));
		o.datatype = 'range(5,100)';
		o.default = '25';
		o.rmempty = false;
		o.description = _('Minimum guaranteed bandwidth on Q16/Q48 (weight 15) for DNS, NTP, SSH, STUN/WebRTC, ZeroTier, WireGuard, small UDP game packets, and EF/CS5-7 DSCP.');

		o = s.taboption('sqm', form.Value, 'video_pct', _('Video / QUIC Streaming Ceiling (%)'));
		o.datatype = 'range(5,100)';
		o.default = '45';
		o.rmempty = false;
		o.description = _('Maximum share on Q30/Q62 (weight 6) for QUIC UDP/443 media and recognized streaming TLS SNI.');

		o = s.taboption('sqm', form.Value, 'p2p_pct', _('P2P / BitTorrent Scavenger Ceiling (%)'));
		o.datatype = 'range(1,100)';
		o.default = '15';
		o.rmempty = false;
		o.description = _('Strict ceiling on Q31/Q63 (weight 1) for BitTorrent/DHT/eDonkey and CS1/LE bulk traffic.');

		o = s.taboption('sqm', form.Flag, 'ack_prio', _('Prioritize Pure TCP Control / ACK Packets'));
		o.default = '1';
		o.rmempty = false;

		o = s.taboption('sqm', form.ListValue, 'cake_diffserv', _('Fallback CAKE DiffServ (only when HNAT=0)'));
		o.value('diffserv4', 'diffserv4');
		o.value('diffserv3', 'diffserv3');
		o.value('diffserv8', 'diffserv8');
		o.value('besteffort', 'besteffort');
		o.default = 'diffserv4';
		o.rmempty = false;

		o = s.taboption('sqm', form.Flag, 'cake_ack_filter', _('Fallback CAKE ACK Filter (only when HNAT=0)'));
		o.default = '1';
		o.rmempty = false;

		var eqosGlobal = m.section(form.NamedSection, 'global', 'global');
		o = eqosGlobal.option(form.Flag, 'eqos_enabled', _('Enable Simultaneous Per-Host EQoS & Connection Limits'));
		o.default = '1';
		o.rmempty = false;

		s = m.section(form.TableSection, 'device', _('Per-Host Hardware EQoS Rules (Up to 15 Concurrent Hosts: UL Q1-15 / DL Q33-47)'));
		s.addremove = true;
		s.anonymous = true;
		s.sortable = true;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = '1';
		o.rmempty = false;

		o = s.option(form.Value, 'ip', _('IPv4 / IPv6 Address'));
		o.datatype = 'ipaddr';
		o.placeholder = '192.168.59.x';
		hints.forEach(function(h) {
			if (h.ip)
				o.value(h.ip, h.name ? (h.ip + ' (' + h.name + ')') : h.ip);
		});

		o = s.option(form.Value, 'mac', _('MAC Address'));
		o.datatype = 'macaddr';
		o.placeholder = 'aa:bb:cc:dd:ee:ff';
		hints.forEach(function(h) {
			if (h.mac)
				o.value(h.mac, h.name ? (h.mac + ' (' + h.name + ')') : (h.mac + ' (' + h.ip + ')'));
		});

		o = s.option(form.Value, 'download_kbps', _('Download Limit (kbit/s)'));
		o.datatype = 'range(0,10000000)';
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Value, 'upload_kbps', _('Upload Limit (kbit/s)'));
		o.datatype = 'range(0,10000000)';
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Value, 'max_conn', _('Max New Connections'));
		o.datatype = 'range(0,65535)';
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Value, 'comment', _('Comment'));
		o.placeholder = _('Device note');

		poll.add(pollStatus, 10);
		return m.render();
	},
	handleReset: null
});
