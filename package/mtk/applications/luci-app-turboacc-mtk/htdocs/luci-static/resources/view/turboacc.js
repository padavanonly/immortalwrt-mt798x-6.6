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
	expect: { '': {} }
});

var getFullConeStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFullConeStat',
	expect: { '': {} }
});

var getTCPCCAStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getTCPCCAStat',
	expect: { '': {} }
});

var getMTKPPEStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getMTKPPEStat',
	expect: { '': {} }
});

function getServiceStatus() {
	return Promise.all([
		L.resolveDefault(getFastPathStat(), {}),
		L.resolveDefault(getFullConeStat(), {}),
		L.resolveDefault(getTCPCCAStat(), {})
	]);
}

function getMTKPPEStatus() {
	return L.resolveDefault(getMTKPPEStat(), {});
}

function trimValue(value) {
	return value == null ? '' : String(value).replace(/^\s+|\s+$/g, '');
}

function parseInteger(value) {
	var parsed = parseInt(value, 10);

	return isNaN(parsed) ? null : parsed;
}

function boolValue(value) {
	return value === true || value === '1' || value === 1;
}

function displayValue(value) {
	return (value != null && value !== '') ? value : '--';
}

function compactChildren(children) {
	return (children || []).filter(function(child) {
		return child != null;
	});
}

function parseTokenList(value) {
	var tokens = trimValue(value).split(/\s+/).filter(function(token) {
		return token !== '';
	});

	tokens.sort();
	return tokens;
}

function normalizeFastpathValue(value) {
	value = trimValue(value);

	return (!value || value === 'none') ? 'disabled' : value;
}

function configUsesEngine(config, key) {
	return normalizeFastpathValue(config && config.fastpath) === key;
}

function isEngineAvailable(flag, config, key) {
	return !!flag || configUsesEngine(config, key);
}

function getDefaultFullcone(features) {
	return features && features.hasXTFULLCONENAT ? '1' : '2';
}

function getConfigState(features) {
	var fastpath = normalizeFastpathValue(uci.get('turboacc', 'config', 'fastpath'));

	if (fastpath === 'flow_offloading' && !(features && features.hasFLOWOFFLOADING))
		fastpath = 'disabled';

	return {
		fastpath: fastpath,
		fastpath_fo_hw: boolValue(uci.get('turboacc', 'config', 'fastpath_fo_hw')),
		fastpath_fc_br: boolValue(uci.get('turboacc', 'config', 'fastpath_fc_br')),
		fastpath_fc_ipv6: boolValue(uci.get('turboacc', 'config', 'fastpath_fc_ipv6')),
		fastpath_mh_eth_hnat: boolValue(uci.get('turboacc', 'config', 'fastpath_mh_eth_hnat')),
		fastpath_mh_eth_hnat_v6: boolValue(uci.get('turboacc', 'config', 'fastpath_mh_eth_hnat_v6')),
		fastpath_mh_eth_hnat_bind_rate: trimValue(uci.get('turboacc', 'config', 'fastpath_mh_eth_hnat_bind_rate')) || '30',
		fullcone: trimValue(uci.get('turboacc', 'config', 'fullcone')) || getDefaultFullcone(features),
		tcpcca: trimValue(uci.get('turboacc', 'config', 'tcpcca')) || 'cubic'
	};
}

function getAvailableEngines(features, config) {
	var engines = [];

	if (isEngineAvailable(features.hasFLOWOFFLOADING, config, 'flow_offloading'))
		engines.push({ key: 'flow_offloading', label: _('流量分载') });
	if (isEngineAvailable(features.hasFASTCLASSIFIER, config, 'fast_classifier'))
		engines.push({ key: 'fast_classifier', label: _('快速分类器') });
	if (isEngineAvailable(features.hasSHORTCUTFECM, config, 'shortcut_fe_cm'))
		engines.push({ key: 'shortcut_fe_cm', label: _('SFE 连接管理器') });
	if (isEngineAvailable(features.hasMEDIATEKHNAT, config, 'mediatek_hnat'))
		engines.push({ key: 'mediatek_hnat', label: _('MediaTek HNAT') });

	return engines;
}

function getEngineLabel(value) {
	switch (normalizeFastpathValue(value)) {
	case 'flow_offloading':
		return _('流量分载');
	case 'fast_classifier':
		return _('快速分类器');
	case 'shortcut_fe_cm':
		return _('SFE 连接管理器');
	case 'mediatek_hnat':
		return _('MediaTek HNAT');
	default:
		return _('已禁用');
	}
}

function getEngineShortLabel(value) {
	switch (normalizeFastpathValue(value)) {
	case 'flow_offloading':
		return 'FLOW';
	case 'fast_classifier':
		return 'FAST';
	case 'shortcut_fe_cm':
		return 'SFE';
	case 'mediatek_hnat':
		return 'HNAT';
	default:
		return 'OFF';
	}
}

function getEngineDescription(value) {
	switch (normalizeFastpathValue(value)) {
	case 'flow_offloading':
		return _('软件路由/NAT 分载，通用性最好，但加速深度偏保守。');
	case 'fast_classifier':
		return _('面向 SFE 的快速分类器，适合希望继续启用桥接加速的场景。');
	case 'shortcut_fe_cm':
		return _('SFE 的轻量连接管理器，路径简单，但平台覆盖面更窄。');
	case 'mediatek_hnat':
		return _('mt798x 在 24.10 下的主力有线加速通路，可配合 PPE 实时观察绑定情况。');
	default:
		return _('当前未启用主加速引擎。');
	}
}

function getRuntimeLabel(token) {
	token = trimValue(token);

	switch (token) {
	case 'Flow Offloading':
	case 'Flow offloading':
		return _('流量分载');
	case 'Fast classifier':
		return _('快速分类器');
	case 'Shortcut-FE CM':
		return _('SFE 连接管理器');
	case 'MediaTek HWNAT':
	case 'MediaTek HNAT':
		return _('MediaTek HNAT');
	case 'xt_FULLCONENAT':
		return _('XT_FULLCONE_NAT');
	case 'Boardcom Fullcone':
		return _('Boardcom_FULLCONE_NAT');
	case 'Ethernet HNAT Disabled':
		return _('以太网 HNAT 未启用');
	case 'Wireless HNAT Disabled':
		return _('无线 HNAT 未启用');
	default:
		return token;
	}
}

function getRuntimeEngineKey(value) {
	value = trimValue(value);

	if (!value)
		return 'disabled';

	if (value.indexOf('MediaTek HNAT') >= 0 || value.indexOf('MediaTek HWNAT') >= 0)
		return 'mediatek_hnat';
	if (value.indexOf('Fast classifier') >= 0)
		return 'fast_classifier';
	if (value.indexOf('Shortcut-FE CM') >= 0)
		return 'shortcut_fe_cm';
	if (value.indexOf('Flow offloading') >= 0 || value.indexOf('Flow Offloading') >= 0)
		return 'flow_offloading';

	return 'disabled';
}

function buildStatusMeta(value, emptyDetail, activeDetail) {
	var parts = trimValue(value).split(' / ').map(function(part) {
		return getRuntimeLabel(part);
	}).filter(function(part) {
		return part !== '';
	});

	return {
		primary: parts.length ? parts[0] : _('已禁用'),
		warnings: parts.length > 1 ? parts.slice(1) : [],
		detail: parts.length > 1 ? parts.slice(1).join(' · ') : (parts.length ? (activeDetail || parts[0]) : (emptyDetail || _('未检测到活动加速通路。')))
	};
}

function getFullconeConfigLabel(value) {
	switch (trimValue(value)) {
	case '1':
		return _('XT_FULLCONE_NAT(更佳的兼容性)');
	case '2':
		return _('Boardcom_FULLCONE_NAT(旧实现兼容)');
	default:
		return _('已禁用');
	}
}

function getIPv6ModeText(config, features) {
	var fastpath = normalizeFastpathValue(config.fastpath);

	if (!features.hasIPV6)
		return _('不可用');

	switch (fastpath) {
	case 'fast_classifier':
		return config.fastpath_fc_ipv6 ? _('已启用') : _('已禁用');
	case 'mediatek_hnat':
		return (config.fastpath_mh_eth_hnat && config.fastpath_mh_eth_hnat_v6) ? _('已启用') : _('已禁用');
	case 'disabled':
		return _('已禁用');
	default:
		return _('跟随主通路');
	}
}

function getIPv6ModeDetail(config, features) {
	var fastpath = normalizeFastpathValue(config.fastpath);

	if (!features.hasIPV6)
		return _('当前平台未开放 IPv6 加速能力。');

	switch (fastpath) {
	case 'fast_classifier':
		return config.fastpath_fc_ipv6 ? _('Fast classifier 已打开 IPv6 加速。') : _('Fast classifier 可单独启用 IPv6 加速。');
	case 'mediatek_hnat':
		return config.fastpath_mh_eth_hnat_v6 ? _('有线 IPv6 会话可进入 HNAT 通路。') : _('当前仅加速 IPv4，会保留 IPv6 走常规栈。');
	case 'disabled':
		return _('主通路关闭时不会有独立 IPv6 加速。');
	default:
		return _('当前引擎没有单独的 IPv6 开关。');
	}
}

function getPPESummary(ppeStats) {
	var count = parseInteger(ppeStats.PPE_NUM);
	var index;
	var cards = [];
	var totalBound = 0;
	var totalAll = 0;
	var bound;
	var all;

	if (count == null) {
		return {
			count: null,
			cards: [],
			totalBound: null,
			totalAll: null
		};
	}

	if (count <= 0) {
		return {
			count: 0,
			cards: [],
			totalBound: null,
			totalAll: null
		};
	}

	for (index = 0; index < count; index++) {
		bound = parseInteger(ppeStats['BIND_PPE' + index]) || 0;
		all = parseInteger(ppeStats['ALL_PPE' + index]) || 0;
		totalBound += bound;
		totalAll += all;
		cards.push({
			index: index,
			bound: bound,
			all: all,
			percent: all > 0 ? Math.round((bound / all) * 100) : 0
		});
	}

	return {
		count: count,
		cards: cards,
		totalBound: totalBound,
		totalAll: totalAll
	};
}

function formatSessions(bound, all) {
	if (bound == null || all == null)
		return '--';

	return String(bound) + ' / ' + String(all);
}

function isDarkTheme() {
	if (typeof window === 'undefined' || typeof document === 'undefined' || !document.body)
		return false;

	var html = document.documentElement;
	var htmlClass = html ? (html.className || '') : '';
	var bodyClass = document.body.className || '';
	var htmlTheme = html ? (html.getAttribute('data-theme') || '') : '';
	var bodyTheme = document.body.getAttribute('data-theme') || '';
	var background = window.getComputedStyle(document.body).backgroundColor || '';
	var channels = background.match(/\d+(?:\.\d+)?/g);
	var luminance;

	if (/\b(?:dark|mode-dark|argon-dark)\b/i.test(htmlClass) || /\b(?:dark|mode-dark|argon-dark)\b/i.test(bodyClass))
		return true;

	if (/dark/i.test(htmlTheme) || /dark/i.test(bodyTheme))
		return true;

	if (/light/i.test(htmlTheme) || /light/i.test(bodyTheme))
		return false;

	if (!channels || channels.length < 3)
		return false;

	luminance = (Number(channels[0]) * 299 + Number(channels[1]) * 587 + Number(channels[2]) * 114) / 1000;
	return luminance < 140;
}

function applyThemeClass(node, darkClass) {
	function syncThemeClass() {
		node.classList.toggle(darkClass, isDarkTheme());
	}

	var retries = [ 0, 80, 220, 480, 900 ];
	var index;
	var themeObserver;
	var mediaQuery;

	syncThemeClass();

	if (typeof window !== 'undefined') {
		for (index = 0; index < retries.length; index++)
			window.setTimeout(syncThemeClass, retries[index]);

		if (window.requestAnimationFrame)
			window.requestAnimationFrame(syncThemeClass);

		if (typeof MutationObserver !== 'undefined' && document.documentElement) {
			themeObserver = new MutationObserver(syncThemeClass);
			themeObserver.observe(document.documentElement, { attributes: true, attributeFilter: [ 'class', 'style', 'data-theme' ] });

			if (document.body && document.body !== document.documentElement)
				themeObserver.observe(document.body, { attributes: true, attributeFilter: [ 'class', 'style', 'data-theme' ] });
		}

		if (window.matchMedia) {
			mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');

			if (mediaQuery) {
				if (mediaQuery.addEventListener)
					mediaQuery.addEventListener('change', syncThemeClass);
				else if (mediaQuery.addListener)
					mediaQuery.addListener(syncThemeClass);
			}
		}

		window.addEventListener('pageshow', syncThemeClass);
		window.addEventListener('focus', syncThemeClass);
	}

	return node;
}

function renderChip(text, extraClass) {
	if (text == null || text === '')
		return null;

	return E('span', { 'class': 'ta-chip ' + (extraClass || '') }, String(text));
}

function renderSectionPill(label, value, tone) {
	return E('div', { 'class': 'ta-section-pill ' + (tone || '') }, [
		E('span', { 'class': 'ta-section-pill-label' }, label),
		E('strong', { 'class': 'ta-section-pill-value' }, displayValue(value))
	]);
}

function renderStatusStrong(value, enabled) {
	return E('strong', { 'class': 'ta-status-value ' + (enabled ? 'is-enabled' : 'is-disabled') }, displayValue(value));
}

function renderStatCard(title, value, detail, tone, chips) {
	chips = compactChildren(chips);

	return E('div', { 'class': 'ta-stat-card ' + (tone || '') }, compactChildren([
		E('div', { 'class': 'ta-stat-title' }, title),
		E('div', { 'class': 'ta-stat-value' }, displayValue(value)),
		detail ? E('div', { 'class': 'ta-stat-detail' }, detail) : null,
		chips.length ? E('div', { 'class': 'ta-chip-list' }, chips) : null
	]));
}

function renderInsightCard(title, value, detail, tone, rows, chips) {
	rows = (rows || []).filter(function(row) {
		return row[1] != null && row[1] !== '';
	});
	chips = compactChildren(chips);

	return E('div', { 'class': 'ta-insight-card ' + (tone || '') }, compactChildren([
		E('div', { 'class': 'ta-insight-head' }, [
			E('div', { 'class': 'ta-insight-title' }, title),
			E('div', { 'class': 'ta-insight-value' }, displayValue(value))
		]),
		detail ? E('div', { 'class': 'ta-insight-detail' }, detail) : null,
		chips.length ? E('div', { 'class': 'ta-chip-list' }, chips) : null,
		rows.length ? E('div', { 'class': 'ta-kv-list' }, rows.map(function(row) {
			return E('div', { 'class': 'ta-kv-row' }, [
				E('span', { 'class': 'ta-kv-label' }, row[0]),
				E('strong', { 'class': 'ta-kv-value' }, displayValue(row[1]))
			]);
		})) : null
	]));
}

function renderInfoGrid(rows) {
	rows = (rows || []).filter(function(row) {
		return row[1] != null;
	});

	if (!rows.length)
		return null;

	return E('div', { 'class': 'ta-info-grid' }, rows.map(function(row) {
		return E('div', { 'class': 'ta-info-row' }, [
			E('div', { 'class': 'ta-info-label' }, row[0]),
			E('div', { 'class': 'ta-info-value' }, row[1])
		]);
	}));
}

function renderChecklist(title, items, tone) {
	items = (items || []).filter(function(item) {
		return item != null && item !== '';
	});

	if (!items.length)
		return null;

	return E('div', { 'class': 'ta-note-card ' + (tone || '') }, [
		E('div', { 'class': 'ta-note-title' }, title),
		E('ul', { 'class': 'ta-note-list' }, items.map(function(item) {
			return E('li', {}, item);
		}))
	]);
}

function renderEngineCard(engine, configKey, runtimeKey) {
	var chips = [];
	var tone = 'is-neutral';

	if (engine.key === configKey) {
		chips.push(renderChip(_('当前配置'), 'is-info'));
		tone = 'is-selected';
	}

	if (engine.key === runtimeKey) {
		chips.push(renderChip(_('实时生效'), 'is-ok'));
		tone = 'is-live';
	}

	if (!chips.length)
		chips.push(renderChip(_('可切换'), 'is-muted'));

	return E('div', { 'class': 'ta-engine-card ' + tone }, [
		E('div', { 'class': 'ta-engine-top' }, [
			E('div', { 'class': 'ta-engine-code' }, getEngineShortLabel(engine.key)),
			E('div', { 'class': 'ta-engine-name' }, engine.label)
		]),
		E('div', { 'class': 'ta-engine-detail' }, getEngineDescription(engine.key)),
		E('div', { 'class': 'ta-chip-list' }, chips)
	]);
}

function renderPPEPanel(ppe) {
	if (!ppe || !ppe.cards || !ppe.cards.length)
		return null;

	var rows = ppe.cards;

	return E('div', { 'class': 'ta-panel ta-ppe-panel' }, [
		E('div', { 'class': 'ta-panel-head' }, [
			E('div', { 'class': 'ta-panel-title' }, _('PPE 明细')), 
			E('div', { 'class': 'ta-panel-subtitle' }, _('PPE 通道的已绑定连接数。'))
		]),
		E('div', { 'class': 'ta-progress-list' }, rows.map(function(card) {
			var width = card.bound > 0 && card.all > 0 ? Math.max(card.percent, 1) : 0;
			var value = card.bound == null || card.all == null
				? '--'
				: formatSessions(card.bound, card.all) + ' (' + card.percent + '%)';
			var fillClass = 'ta-progress-fill' + (width > 0 ? ' is-active' : '');

			return E('div', { 'class': 'ta-progress-row' }, [
				E('div', { 'class': 'ta-progress-head' }, [
					E('div', { 'class': 'ta-progress-label' }, 'PPE' + card.index),
					E('div', { 'class': 'ta-progress-value' }, value)
				]),
				E('div', { 'class': 'ta-progress-track' }, [
					E('div', { 'class': fillClass, 'style': 'width:' + String(width) + '%' })
				])
			]);
		}))
	]);
}

function renderFeatureTag(flag, label) {
	return renderChip((label ? label + ' · ' : '') + (flag ? _('可用') : _('不可用')), flag ? 'is-ok' : 'is-muted');
}

function getHealthState(state) {
	var features = state.features || {};
	var config = state.config || {};
	var service = state.service || [];
	var ppe = state.ppe || {};
	var configuredKey = normalizeFastpathValue(config.fastpath);
	var runtimeType = trimValue(service[0] && service[0].type);
	var runtimeKey = getRuntimeEngineKey(runtimeType);
	var runtimeMeta = buildStatusMeta(runtimeType, _('未检测到活动加速通路。'), _('当前内核返回的主加速通路。'));
	var fullconeMeta = buildStatusMeta(service[1] && service[1].type, _('已禁用'));
	var tcpccaLabel = trimValue(service[2] && service[2].type) || String(config.tcpcca || '').toUpperCase() || '--';
	var notes = [];
	var pills = [];
	var level = 'is-idle';
	var headline = _('已禁用');
	var headlineTone = 'is-disabled';
	var summary = _('当前页面没有检测到活动的数据通路。');
	var aligned = configuredKey === runtimeKey || (configuredKey === 'disabled' && runtimeKey === 'disabled');

	if (configuredKey === 'disabled' && runtimeKey === 'disabled') {
		level = 'is-idle';
		headline = _('已禁用');
		headlineTone = 'is-disabled';
		summary = _('当前未启用主加速引擎。');
	}
	else if (configuredKey === 'disabled' && runtimeKey !== 'disabled') {
		level = 'is-attention';
		headline = _('已启用');
		headlineTone = 'is-enabled';
		summary = _('实时通路仍在工作，但当前配置已经设为禁用。');
	}
	else if (configuredKey !== 'disabled' && runtimeKey === configuredKey) {
		level = 'is-healthy';
		headline = _('已启用');
		headlineTone = 'is-enabled';
		summary = _('配置与实时通路一致。');
	}
	else if (configuredKey !== 'disabled' && runtimeKey !== 'disabled') {
		level = 'is-attention';
		headline = _('已启用');
		headlineTone = 'is-enabled';
		summary = _('实时通路与配置不一致。');
	}
	else {
		level = 'is-warning';
		headline = _('已禁用');
		headlineTone = 'is-disabled';
		summary = _('已配置，但当前未看到活动通路。');
	}

	pills.push(renderChip(_('配置') + ' · ' + getEngineLabel(configuredKey), 'is-info'));
	pills.push(renderChip(_('实时') + ' · ' + runtimeMeta.primary, runtimeKey === 'disabled' ? 'is-muted' : 'is-ok'));
	if (configuredKey === 'mediatek_hnat' && ppe.totalAll != null)
		pills.push(renderChip(_('PPE') + ' · ' + formatSessions(ppe.totalBound, ppe.totalAll), 'is-info'));

	if (!aligned && configuredKey !== 'disabled')
		notes.push(_('已选择 ') + getEngineLabel(configuredKey) + _('，但实时内核仍显示 ') + runtimeMeta.primary + _('。'));

	if (runtimeMeta.warnings.length)
		notes.push(_('运行告警：') + runtimeMeta.warnings.join(' · '));

	if (configuredKey === 'mediatek_hnat' && !config.fastpath_mh_eth_hnat)
		notes.push(_('已选 MediaTek HNAT，但“启用有线 HNAT”当前未打开。'));

	if (configuredKey === 'mediatek_hnat' && config.fastpath_mh_eth_hnat && features.hasIPV6 && !config.fastpath_mh_eth_hnat_v6)
		notes.push(_('IPv6 当前未进入 HNAT，若有纯 IPv6 场景可视需求再开启。'));

	if (configuredKey === 'fast_classifier' && config.fastpath_fc_br)
		notes.push(_('桥接加速已打开，若桥接模式 VPN 出现异常，应优先回看这一项。'));

	if (configuredKey === 'mediatek_hnat' && ppe.totalAll != null)
		notes.push(_('当前 PPE 总容量：') + formatSessions(ppe.totalBound, ppe.totalAll));

	return {
		level: level,
		headline: headline,
		summary: summary,
		configuredKey: configuredKey,
		runtimeKey: runtimeKey,
		runtimeMeta: runtimeMeta,
		fullconeMeta: fullconeMeta,
		tcpccaLabel: tcpccaLabel,
		headlineTone: headlineTone,
		pills: pills,
		notes: notes
	};
}

function renderHero(state, health) {
	var config = state.config || {};
	var ppe = state.ppe || {};
	var activeKey = health.runtimeKey !== 'disabled'
		? health.runtimeKey
		: normalizeFastpathValue(config.fastpath);
	var hnatProfile = activeKey === 'mediatek_hnat';
	var summaryValue = hnatProfile
		? formatSessions(ppe.totalBound, ppe.totalAll)
		: getEngineLabel(activeKey);

	return E('div', { 'class': 'ta-hero ' + health.level }, [
		E('div', { 'class': 'ta-hero-main' }, [
			E('div', { 'class': 'ta-hero-badge' }, [
				E('span', { 'class': 'ta-hero-badge-code' }, getEngineShortLabel(activeKey)),
				E('span', { 'class': 'ta-hero-badge-dot' })
			]),
			E('div', { 'class': 'ta-hero-copy' }, [
				E('div', { 'class': 'ta-hero-kicker' }, _('TurboACC 控制台')),
				E('div', { 'class': 'ta-hero-title ' + health.headlineTone }, health.headline),
				E('div', { 'class': 'ta-hero-summary' }, health.summary)
			])
		]),
		E('div', { 'class': 'ta-hero-side' }, compactChildren([
			renderSectionPill(_('实时通路'), health.runtimeMeta.primary, health.runtimeKey === 'disabled' ? 'is-muted' : 'is-good'),
			renderSectionPill(hnatProfile ? _('PPE 连接数') : _('当前配置'), summaryValue, hnatProfile ? 'is-accent' : 'is-info')
		]))
	]);
}

function renderStatusStrip(state, health) {
	var features = state.features || {};
	var config = state.config || {};
	var ipv6Text = getIPv6ModeText(config, features);
	var fullconeEnabled = trimValue(config.fullcone) !== '0';
	var ipv6Enabled = ipv6Text !== _('已禁用') && ipv6Text !== _('不可用');

	return E('div', { 'class': 'ta-status-strip' }, [
		E('div', { 'class': 'ta-status-item' }, [
			E('span', {}, _('配置')),
			renderStatusStrong(getEngineLabel(config.fastpath), normalizeFastpathValue(config.fastpath) !== 'disabled')
		]),
		E('div', { 'class': 'ta-status-item' }, [
			E('span', {}, _('NAT')),
			renderStatusStrong(getFullconeConfigLabel(config.fullcone), fullconeEnabled)
		]),
		E('div', { 'class': 'ta-status-item' }, [
			E('span', {}, _('IPv6')),
			renderStatusStrong(ipv6Text, ipv6Enabled)
		]),
		E('div', { 'class': 'ta-status-item' }, [
			E('span', {}, _('TCP')),
			renderStatusStrong(health.tcpccaLabel, health.tcpccaLabel !== '--')
		])
	]);
}

function renderSummaryGrid(state, health) {
	var features = state.features || {};
	var config = state.config || {};
	var ppe = state.ppe || {};
	var ipv6Text = getIPv6ModeText(config, features);
	var availableEngines = getAvailableEngines(features, config);
	var engineCount = availableEngines.length;
	var sessionValue = normalizeFastpathValue(config.fastpath) === 'mediatek_hnat'
		? formatSessions(ppe.totalBound, ppe.totalAll)
		: String(engineCount);
	var sessionDetail = normalizeFastpathValue(config.fastpath) === 'mediatek_hnat'
		? (_('PPE 通道') + ': ' + displayValue(ppe.count) + ' · ' + _('绑定阈值') + ': ' + config.fastpath_mh_eth_hnat_bind_rate + ' pps')
		: (_('当前可切换主引擎数量') + ': ' + String(engineCount));

	return E('div', { 'class': 'ta-summary-grid' }, [
		renderStatCard(_('配置档案'), getEngineLabel(config.fastpath), getEngineDescription(config.fastpath), 'is-primary', [
			renderChip(config.fastpath === 'disabled' ? _('未启用') : _('已选择'), config.fastpath === 'disabled' ? 'is-muted' : 'is-info')
		]),
		renderStatCard(_('实时通路'), health.runtimeMeta.primary, health.runtimeMeta.detail, 'is-good', health.runtimeMeta.warnings.map(function(tag) {
			return renderChip(tag, 'is-warning');
		})),
		renderStatCard(_('NAT / TCP 策略'), getFullconeConfigLabel(config.fullcone), _('TCP CCA') + ' · ' + health.tcpccaLabel, 'is-warning', [
			renderChip(_('IPv6') + ' · ' + ipv6Text, ipv6Text === _('已启用') ? 'is-ok' : 'is-muted')
		]),
		renderStatCard(normalizeFastpathValue(config.fastpath) === 'mediatek_hnat' ? _('PPE 会话') : _('引擎储备'), sessionValue, sessionDetail, 'is-accent', [
			renderChip(_('实时遥测'), 'is-info')
		])
	]);
}

function renderFocusGrid(state, health) {
	var features = state.features || {};
	var config = state.config || {};
	var ppe = state.ppe || {};
	var hnatEnabled = normalizeFastpathValue(config.fastpath) === 'mediatek_hnat';

	return E('div', { 'class': 'ta-focus-grid' }, [
		renderInsightCard(
			_('有线数据通路'),
			hnatEnabled ? (config.fastpath_mh_eth_hnat ? _('已放行') : _('待放行')) : health.runtimeMeta.primary,
			hnatEnabled ? _('MediaTek HNAT 是 mt798x 24.10 上最值得优先确认的主通路。') : _('当前主通路决定了大部分路由/NAT 转发行为。'),
			'is-primary',
			[
				[ _('实时引擎'), health.runtimeMeta.primary ],
				[ _('桥接加速'), config.fastpath_fc_br ? _('已启用') : _('未启用') ],
				[ _('有线 HNAT'), hnatEnabled ? (config.fastpath_mh_eth_hnat ? _('已启用') : _('未启用')) : null ]
			],
			[
				renderChip(hnatEnabled ? _('优先检查') : _('按需切换'), 'is-info')
			]
		),
		renderInsightCard(
			_('游戏与 NAT 行为'),
			getFullconeConfigLabel(config.fullcone),
			_('全锥形 NAT 主要影响联机和端口映射体验，不代表主通路本身一定工作。'),
			'is-warning',
			[
				[ _('实时反馈'), health.fullconeMeta.primary ],
				[ _('风险提示'), trimValue(config.fullcone) === '0' ? _('联机型业务更依赖手动映射') : _('更适合游戏主机与 P2P 场景') ]
			],
			[
				renderChip(_('体验侧'), 'is-warning')
			]
		),
		renderInsightCard(
			_('IPv6 与高级调优'),
			getIPv6ModeText(config, features),
			getIPv6ModeDetail(config, features),
			'is-accent',
			[
				[ _('绑定阈值'), hnatEnabled && config.fastpath_mh_eth_hnat ? (config.fastpath_mh_eth_hnat_bind_rate + ' pps') : null ],
				[ _('PPE 总览'), hnatEnabled ? formatSessions(ppe.totalBound, ppe.totalAll) : null ]
			],
			[
				renderChip(_('高级项'), 'is-muted')
			]
		)
	]);
}

function renderEngineRail(state, health) {
	var availableEngines = getAvailableEngines(state.features || {}, state.config || {});

	if (!availableEngines.length)
		return null;

	return E('div', { 'class': 'ta-panel' }, [
		E('div', { 'class': 'ta-panel-head' }, [
			E('div', { 'class': 'ta-panel-title' }, _('通路候选')), 
			E('div', { 'class': 'ta-panel-subtitle' }, _('把“当前配置”和“实时生效”并排摆出来，避免只看配置误判真实状态。'))
		]),
		E('div', { 'class': 'ta-engine-grid' }, availableEngines.map(function(engine) {
			return renderEngineCard(engine, health.configuredKey, health.runtimeKey);
		}))
	]);
}

function renderTelemetryGrid(state, health) {
	var features = state.features || {};
	var config = state.config || {};
	var ppe = state.ppe || {};

	return E('div', { 'class': 'ta-telemetry-grid' }, compactChildren([
		E('div', { 'class': 'ta-panel' }, [
			E('div', { 'class': 'ta-panel-head' }, [
				E('div', { 'class': 'ta-panel-title' }, _('实时遥测')), 
				E('div', { 'class': 'ta-panel-subtitle' }, _('这里读的是当前内核和 rpcd 的实际返回，不是单纯的表单配置。'))
			]),
			renderInfoGrid([
				[ _('主加速引擎'), health.runtimeMeta.primary ],
				[ _('运行告警'), health.runtimeMeta.warnings.length ? E('div', { 'class': 'ta-chip-list' }, health.runtimeMeta.warnings.map(function(tag) {
					return renderChip(tag, 'is-warning');
				})) : renderChip(_('无'), 'is-ok') ],
				[ _('全锥形 NAT'), health.fullconeMeta.primary ],
				[ _('TCP CCA'), health.tcpccaLabel ],
				[ _('IPv6 加速'), getIPv6ModeText(config, features) ],
				[ _('PPE 通道数'), normalizeFastpathValue(config.fastpath) === 'mediatek_hnat' ? displayValue(ppe.count) : null ],
				[ _('已绑定会话'), normalizeFastpathValue(config.fastpath) === 'mediatek_hnat' ? formatSessions(ppe.totalBound, ppe.totalAll) : null ]
			])
		]),
		E('div', { 'class': 'ta-side-stack' }, compactChildren([
			renderChecklist(_('当前判断'), health.notes.slice(0, 4), health.level),
			E('div', { 'class': 'ta-note-card is-neutral' }, [
				E('div', { 'class': 'ta-note-title' }, _('能力矩阵')), 
				E('div', { 'class': 'ta-capability-grid' }, [
					renderFeatureTag(features.hasFLOWOFFLOADING, _('流量分载')),
					renderFeatureTag(features.hasFASTCLASSIFIER, _('快速分类器')),
					renderFeatureTag(features.hasSHORTCUTFECM, _('SFE')),
					renderFeatureTag(features.hasMEDIATEKHNAT, _('HNAT')),
					renderFeatureTag(features.hasXTFULLCONENAT, _('FullCone')),
					renderFeatureTag(features.hasIPV6, _('IPv6'))
				]),
				renderInfoGrid([
					[ _('流量分载'), renderFeatureTag(features.hasFLOWOFFLOADING) ],
					[ _('快速分类器'), renderFeatureTag(features.hasFASTCLASSIFIER) ],
					[ _('SFE 连接管理器'), renderFeatureTag(features.hasSHORTCUTFECM) ],
					[ _('MediaTek HNAT'), renderFeatureTag(features.hasMEDIATEKHNAT) ],
					[ _('XT_FULLCONE_NAT'), renderFeatureTag(features.hasXTFULLCONENAT) ],
					[ _('内核 IPv6 协议栈'), renderFeatureTag(features.hasIPV6) ]
				])
			])
		]))
	]));
}

function renderCompactHero(state, health) {
	var config = state.config || {};
	var ppe = state.ppe || {};
	var hnatProfile = normalizeFastpathValue(config.fastpath) === 'mediatek_hnat';
	var chips = compactChildren([
		renderChip(_('配置') + ' · ' + getEngineLabel(config.fastpath), 'is-info'),
		renderChip(_('实时') + ' · ' + health.runtimeMeta.primary, health.runtimeKey === 'disabled' ? 'is-muted' : 'is-ok'),
		renderChip(_('TCP') + ' · ' + health.tcpccaLabel, 'is-info'),
		hnatProfile ? renderChip(_('PPE') + ' · ' + formatSessions(ppe.totalBound, ppe.totalAll), 'is-info') : null
	]);

	return E('div', { 'class': 'ta-panel ta-compact-hero ' + health.level }, compactChildren([
		E('div', { 'class': 'ta-compact-hero-top' }, [
			E('div', { 'class': 'ta-compact-hero-copy' }, [
				E('div', { 'class': 'ta-compact-kicker' }, _('TurboACC 控制台')),
				E('div', { 'class': 'ta-compact-headline' }, health.headline),
				E('div', { 'class': 'ta-compact-summary' }, health.summary)
			]),
			E('div', { 'class': 'ta-compact-state ' + health.headlineTone }, health.runtimeMeta.primary)
		]),
		chips.length ? E('div', { 'class': 'ta-chip-list' }, chips) : null
	]));
}

function renderCompactOverviewLegacy(state, health) {
	var features = state.features || {};
	var config = state.config || {};
	var ppe = state.ppe || {};
	var rows = [
		[ _('主加速引擎'), getEngineLabel(config.fastpath) ],
		[ _('实时通路'), health.runtimeMeta.primary ],
		[ _('全锥形 NAT'), health.fullconeMeta.primary ],
		[ _('TCP CCA'), health.tcpccaLabel ],
		[ _('IPv6'), getIPv6ModeText(config, features) ]
	];

	if (normalizeFastpathValue(config.fastpath) === 'mediatek_hnat') {
		rows.push(
			[ _('PPE 通道数'), displayValue(ppe.count) ],
			[ _('PPE 连接数'), formatSessions(ppe.totalBound, ppe.totalAll) ],
			[ _('绑定阈值'), config.fastpath_mh_eth_hnat ? (config.fastpath_mh_eth_hnat_bind_rate + ' pps') : null ]
		);
	}

	return E('div', { 'class': 'ta-compact-grid' }, compactChildren([
		E('div', { 'class': 'ta-panel' }, [
			E('div', { 'class': 'ta-panel-head' }, [
				E('div', { 'class': 'ta-panel-title' }, _('运行状态')),
				E('div', { 'class': 'ta-panel-subtitle' }, _('只保留最常用的运行状态与实时信息。'))
			]),
			renderInfoGrid(rows)
		]),
		renderChecklist(_('提示'), health.notes.slice(0, 3), health.level)
	]));
}

function renderCompactOverview(state, health) {
	var features = state.features || {};
	var config = state.config || {};
	var ppe = state.ppe || {};
	var statusTone = health.level === 'is-healthy'
		? 'is-good'
		: (health.level === 'is-attention' || health.level === 'is-warning')
			? 'is-warning'
			: 'is-accent';
	var ipv6Text = getIPv6ModeText(config, features);
	var runtimeChips = health.runtimeMeta.warnings.length
		? health.runtimeMeta.warnings.map(function(tag) {
			return renderChip(tag, 'is-warning');
		})
		: [ renderChip(_('状态正常'), 'is-ok') ];
	var metricCards = [
		renderStatCard(
			_('总状态'),
			health.headline,
			health.summary,
			statusTone,
			[
				renderChip(_('配置') + ' 路 ' + getEngineLabel(config.fastpath), 'is-info')
			]
		),
		renderStatCard(
			_('实时通路'),
			health.runtimeMeta.primary,
			health.runtimeMeta.detail,
			health.runtimeKey === 'disabled' ? 'is-accent' : 'is-good',
			runtimeChips
		),
		renderStatCard(
			_('NAT / TCP'),
			getFullconeConfigLabel(config.fullcone),
			_('TCP CCA') + ' 路 ' + health.tcpccaLabel,
			'is-warning',
			[
				renderChip(_('IPv6') + ' 路 ' + ipv6Text, ipv6Text === _('已启用') ? 'is-ok' : 'is-muted')
			]
		)
	];
	var rows = [
		[ _('主加速引擎'), getEngineLabel(config.fastpath) ],
		[ _('实时通路'), health.runtimeMeta.primary ],
		[ _('运行告警'), health.runtimeMeta.warnings.length ? health.runtimeMeta.warnings.join(' / ') : _('无') ],
		[ _('全锥形 NAT'), health.fullconeMeta.primary ],
		[ _('TCP CCA'), health.tcpccaLabel ],
		[ _('IPv6'), ipv6Text ]
	];

	if (normalizeFastpathValue(config.fastpath) === 'mediatek_hnat') {
		metricCards.push(renderStatCard(
			_('PPE 连接'),
			formatSessions(ppe.totalBound, ppe.totalAll),
			_('通道') + ' 路 ' + displayValue(ppe.count) + ' 路 ' + _('阈值') + ' 路 ' +
				(config.fastpath_mh_eth_hnat ? (config.fastpath_mh_eth_hnat_bind_rate + ' pps') : '--'),
			'is-accent',
			[
				renderChip(
					config.fastpath_mh_eth_hnat ? _('有线 HNAT 已启用') : _('有线 HNAT 未启用'),
					config.fastpath_mh_eth_hnat ? 'is-ok' : 'is-muted'
				)
			]
		));
		rows.push(
			[ _('PPE 通道数'), displayValue(ppe.count) ],
			[ _('PPE 连接数'), formatSessions(ppe.totalBound, ppe.totalAll) ],
			[ _('绑定阈值'), config.fastpath_mh_eth_hnat ? (config.fastpath_mh_eth_hnat_bind_rate + ' pps') : null ]
		);
	}
	else {
		metricCards.push(renderStatCard(
			_('当前配置'),
			getEngineLabel(config.fastpath),
			getEngineDescription(config.fastpath),
			'is-primary',
			[
				renderChip(_('主通路'), 'is-info')
			]
		));
	}

	return E('div', { 'class': 'ta-compact-grid' }, [
		E('div', { 'class': 'ta-panel ta-compact-status-panel' }, [
			E('div', { 'class': 'ta-panel-head' }, [
				E('div', { 'class': 'ta-panel-title' }, _('运行状态')),
				E('div', { 'class': 'ta-panel-subtitle' }, _('把当前配置、实时通路和关键策略拆开看，排查与确认都会更直接。'))
			]),
			E('div', { 'class': 'ta-summary-grid ta-compact-summary-grid' }, metricCards)
		]),
		E('div', { 'class': 'ta-panel ta-compact-detail-panel' }, [
			E('div', { 'class': 'ta-panel-head' }, [
				E('div', { 'class': 'ta-panel-title' }, _('运行明细')),
				E('div', { 'class': 'ta-panel-subtitle' }, _('保留最常用的实时明细，避免页面信息过多又不易定位。'))
			]),
			renderInfoGrid(rows)
		])
	]);
}

function renderOverviewContent(state) {
	var health = getHealthState(state);

	return E('div', { 'class': 'ta-overview' }, compactChildren([
		renderHero(state, health),
		renderStatusStrip(state, health),
		renderPPEPanel(state.ppe)
	]));
}

function buildForm(features, config) {
	var m = new form.Map('turboacc', _('TurboACC 配置面板'),
		_('只保留常用配置项，更改后保存并应用即可。'));
	var s = m.section(form.NamedSection, 'config', 'turboacc');
	var o;
	var tcpccaOptions = parseTokenList(features.hasTCPCCA);
	var showFlowOffloading = !!features.hasFLOWOFFLOADING;
	var showFastClassifier = isEngineAvailable(features.hasFASTCLASSIFIER, config, 'fast_classifier');
	var showShortcutFeCm = isEngineAvailable(features.hasSHORTCUTFECM, config, 'shortcut_fe_cm');
	var showMediatekHnat = true;

	s.tab('engine', _('主通路'), _('选择主加速引擎。'));
	s.tab('experience', _('体验优化'), _('NAT 与 TCP 设置。'));

	if (showMediatekHnat)
		s.tab('hnat', _('HNAT 高级项'), _('MediaTek HNAT 专用设置。'));

	o = s.taboption('engine', form.ListValue, 'fastpath', _('主加速引擎'),
		_('选择当前使用的加速通路。'));
	o.value('disabled', _('禁用'));
	if (showFlowOffloading)
		o.value('flow_offloading', _('流量分载'));
	if (showFastClassifier)
		o.value('fast_classifier', _('快速分类器'));
	if (showShortcutFeCm)
		o.value('shortcut_fe_cm', _('SFE 连接管理器'));
	o.value('mediatek_hnat', features.hasMEDIATEKHNAT ? _('MediaTek HNAT') : _('MediaTek HNAT（尝试加载）'));
	o.default = config.fastpath || 'disabled';
	o.widget = 'select';
	o.rmempty = false;
	o.cfgvalue = function(section_id) {
		var value = normalizeFastpathValue(uci.get('turboacc', section_id, 'fastpath'));

		return value === 'flow_offloading' && !showFlowOffloading ? 'disabled' : value;
	};
	o.write = function(section_id, value) {
		return uci.set('turboacc', section_id, 'fastpath', value === 'disabled' ? 'none' : value);
	};
	o.validate = function(section_id, value) {
		return value === 'disabled' || (showFlowOffloading && value === 'flow_offloading') || value === 'fast_classifier' ||
			value === 'shortcut_fe_cm' || value === 'mediatek_hnat'
				? true
				: _('无效的主加速引擎');
	};

	if (showFlowOffloading) {
		o = s.taboption('engine', form.Flag, 'fastpath_fo_hw', _('启用硬件流量分载'),
			_('硬件支持时可开启。'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'flow_offloading');
	}

	if (showFastClassifier) {
		o = s.taboption('engine', form.Flag, 'fastpath_fc_br', _('启用桥接加速'),
			_('可能影响桥接 VPN。'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'fast_classifier');

		if (features.hasIPV6) {
			o = s.taboption('engine', form.Flag, 'fastpath_fc_ipv6', _('为 Fast classifier 打开 IPv6'),
				_('启用 IPv6 加速。'));
			o.default = o.disabled;
			o.rmempty = false;
			o.depends('fastpath', 'fast_classifier');
		}
	}

	o = s.taboption('experience', form.ListValue, 'fullcone', _('全锥形 NAT'),
		_('游戏或 P2P 需要时开启。'));
	o.value('0', _('禁用'));
	if (features.hasXTFULLCONENAT || config.fullcone === '1')
		o.value('1', _('XT_FULLCONE_NAT'));
	o.value('2', _('Boardcom_FULLCONE_NAT'));
	o.default = config.fullcone || getDefaultFullcone(features);
	o.widget = 'select';
	o.rmempty = false;

	if (tcpccaOptions.indexOf(config.tcpcca) < 0)
		tcpccaOptions.push(config.tcpcca);

	o = s.taboption('experience', form.ListValue, 'tcpcca', _('TCP 拥塞控制算法'),
		_('选择 TCP 拥塞控制算法。BBR 适合高带宽链路，CUBIC 为内核默认，Reno 兼容性最佳。'));
	tcpccaOptions.forEach(function(item) {
		var label;
		switch (String(item).toLowerCase()) {
		case 'bbr':
			label = 'BBR';
			break;
		case 'cubic':
			label = 'CUBIC';
			break;
		case 'reno':
			label = 'Reno';
			break;
		default:
			label = String(item).toUpperCase();
		}
		o.value(item, label);
	});
	o.default = config.tcpcca || 'cubic';
	o.widget = 'select';
	o.rmempty = false;

	if (showMediatekHnat) {
		o = s.taboption('hnat', form.Flag, 'fastpath_mh_eth_hnat', _('启用有线 HNAT'),
			_('启用 MediaTek HNAT hook_toggle；保存并应用后由 turboacc 服务写入内核开关。'));
		o.default = o.enabled;
		o.rmempty = false;
		o.depends('fastpath', 'mediatek_hnat');

		o = s.taboption('hnat', form.Flag, 'fastpath_mh_eth_hnat_v6', _('启用有线 IPv6 HNAT'),
			_('启用 IPv6 HNAT。'));
		o.default = o.enabled;
		o.rmempty = false;
		o.depends({ fastpath: 'mediatek_hnat', fastpath_mh_eth_hnat: '1' });

		o = s.taboption('hnat', form.Value, 'fastpath_mh_eth_hnat_bind_rate', _('HNAT 绑定速率阈值（pps）'),
			_('默认 30。'));
		o.optional = true;
		o.datatype = 'range(1,30)';
		o.placeholder = '30';
		o.depends({ fastpath: 'mediatek_hnat', fastpath_mh_eth_hnat: '1' });
	}

	return m;
}

function renderStyle() {
	return E('style', [
		/* ===== Core variables – light ===== */
		'.ta-page{display:flex;flex-direction:column;gap:16px;padding-top:14px;--ta-bg:linear-gradient(180deg,#f3f6fb,#eef3f8);--ta-panel-bg:linear-gradient(180deg,rgba(255,255,255,.96),rgba(250,252,255,.98));--ta-panel-border:rgba(210,222,237,.95);--ta-panel-border-soft:rgba(225,233,243,.96);--ta-shadow:0 8px 18px rgba(114,131,159,.08);--ta-text:#1f3150;--ta-text-strong:#0d1a33;--ta-text-muted:#6f84a4;--ta-chip:#304762;--ta-chip-bg:rgba(237,242,248,.95);--ta-chip-border:rgba(214,224,237,.95);--ta-info:#5878a6;--ta-info-bg:rgba(241,245,250,.96);--ta-good:#17786a;--ta-good-bg:rgba(220,244,234,.98);--ta-warn:#b87812;--ta-warn-bg:rgba(255,243,214,.98);--ta-danger:#b0374f;--ta-danger-bg:rgba(255,228,234,.95);--ta-accent:#5c6ddb;--ta-accent-bg:rgba(232,237,255,.98);--ta-hero-orb:none;--ta-hero-badge:linear-gradient(145deg,#2d5ce9,#4c96ef);--ta-button-bg:linear-gradient(180deg,#ffffff,#f7f9fc);--ta-button-primary:linear-gradient(90deg,#19959c,#5470ef);--ta-button-shadow:0 10px 18px rgba(84,112,239,.16)}',

		/* ===== Dark mode – .ta-dark class ===== */
		'.ta-page.ta-dark,.ta-dark{--ta-bg:linear-gradient(180deg,rgba(10,16,28,.99),rgba(8,13,23,.99));--ta-panel-bg:linear-gradient(180deg,rgba(19,29,44,.98),rgba(13,22,35,.99));--ta-panel-border:rgba(120,146,188,.22);--ta-panel-border-soft:rgba(120,146,188,.14);--ta-shadow:0 14px 30px rgba(0,0,0,.28);--ta-text:#cbd8ea;--ta-text-strong:#eef4ff;--ta-text-muted:#8fa4c0;--ta-chip:#e0ecff;--ta-chip-bg:rgba(114,157,232,.16);--ta-chip-border:rgba(114,157,232,.22);--ta-info:#b0cdf0;--ta-info-bg:rgba(114,157,232,.12);--ta-good:#bbf7d0;--ta-good-bg:rgba(16,185,129,.20);--ta-warn:#fde68a;--ta-warn-bg:rgba(245,158,11,.20);--ta-danger:#fecaca;--ta-danger-bg:rgba(239,68,68,.20);--ta-accent:#ede9fe;--ta-accent-bg:rgba(139,92,246,.20);--ta-hero-orb:none;--ta-hero-badge:linear-gradient(145deg,#1e3a8a,#2563eb 52%,#0891b2);--ta-button-bg:linear-gradient(180deg,rgba(30,42,60,.98),rgba(18,28,42,.98));--ta-button-primary:linear-gradient(90deg,#0f766e,#2563eb);--ta-button-shadow:0 12px 22px rgba(0,0,0,.28)}',

		/* ===== Force dark via Argon body/html selectors (higher specificity) ===== */
		'body.dark .ta-page,html.dark .ta-page,body.mode-dark .ta-page,body.argon-dark .ta-page,html[data-theme="dark"] .ta-page,body[data-theme="dark"] .ta-page{--ta-bg:linear-gradient(180deg,rgba(10,16,28,.99),rgba(8,13,23,.99));--ta-panel-bg:linear-gradient(180deg,rgba(19,29,44,.98),rgba(13,22,35,.99));--ta-panel-border:rgba(120,146,188,.22);--ta-panel-border-soft:rgba(120,146,188,.14);--ta-shadow:0 14px 30px rgba(0,0,0,.28);--ta-text:#cbd8ea;--ta-text-strong:#eef4ff;--ta-text-muted:#8fa4c0;--ta-chip:#e0ecff;--ta-chip-bg:rgba(114,157,232,.16);--ta-chip-border:rgba(114,157,232,.22);--ta-info:#b0cdf0;--ta-info-bg:rgba(114,157,232,.12);--ta-good:#bbf7d0;--ta-good-bg:rgba(16,185,129,.20);--ta-warn:#fde68a;--ta-warn-bg:rgba(245,158,11,.20);--ta-danger:#fecaca;--ta-danger-bg:rgba(239,68,68,.20);--ta-accent:#ede9fe;--ta-accent-bg:rgba(139,92,246,.20);--ta-hero-orb:none;--ta-hero-badge:linear-gradient(145deg,#1e3a8a,#2563eb 52%,#0891b2);--ta-button-bg:linear-gradient(180deg,rgba(30,42,60,.98),rgba(18,28,42,.98));--ta-button-primary:linear-gradient(90deg,#0f766e,#2563eb);--ta-button-shadow:0 12px 22px rgba(0,0,0,.28)}',

		/* ===== Dark: LuCI form elements ===== */
		'.ta-dark .cbi-section,.ta-dark .cbi-section-descr,.ta-dark .cbi-section-node,.ta-dark .cbi-map-descr,.ta-dark .cbi-value-field,.ta-dark .cbi-value-title,.ta-dark .cbi-value-description,.ta-dark .cbi-tab-descr{color:var(--ta-text)!important}',

		'.ta-dark .cbi-dropdown,.ta-dark select,.ta-dark .cbi-input-select,.ta-dark .cbi-input-text,.ta-dark input[type="text"],.ta-dark input[type="number"]{background:rgba(255,255,255,.06)!important;color:var(--ta-text-strong)!important;border-color:var(--ta-panel-border)!important}',

		'.ta-dark .cbi-input-checkbox,.ta-dark .cbi-input-radio{accent-color:var(--ta-info)!important}',

		'.ta-dark .cbi-tabmenu li,.ta-dark .cbi-tabmenu a{background:rgba(255,255,255,.03)!important;color:var(--ta-text)!important;border-color:var(--ta-panel-border)!important}',

		'.ta-dark .cbi-tabmenu .cbi-tab,.ta-dark .cbi-tabmenu a.cbi-tab,.ta-dark .cbi-tabmenu li.active{background:var(--ta-info-bg)!important;color:var(--ta-info)!important;border-color:rgba(59,130,246,.30)!important}',

		'.ta-dark .cbi-button{background:var(--ta-button-bg)!important;color:var(--ta-text)!important;border-color:var(--ta-panel-border)!important}',

		'.ta-dark .cbi-button-save,.ta-dark .cbi-button-apply,.ta-dark .cbi-button-positive{background:var(--ta-button-primary)!important;color:#fff!important;border-color:transparent!important;box-shadow:var(--ta-button-shadow)!important}',

		'.ta-dark .cbi-button-reset{background:rgba(239,68,68,.18)!important;color:var(--ta-danger)!important;border-color:rgba(239,68,68,.30)!important}',

		'.ta-dark .ta-hero-title{box-shadow:0 10px 18px rgba(0,0,0,.18)}',

		'.ta-dark .ta-hero:before{background:radial-gradient(circle,rgba(255,255,255,.06),transparent 62%)}',

		'.ta-dark .ta-chip.is-muted{background:rgba(148,163,184,.10);color:var(--ta-text-muted)}',

		'.ta-dark .ta-config-shell .cbi-page-actions .cbi-button-reset{background:rgba(239,68,68,.22)!important;color:var(--ta-danger)!important}',

		/* ===== Top accent bar ===== */
		'.ta-page:before{content:"";position:absolute;top:-8px;left:0;right:0;height:6px;border-radius:999px;background:linear-gradient(90deg,#4c63e6,#6f78eb);opacity:.92}',

		/* ===== Layout ===== */
		'.ta-overview{display:flex;flex-direction:column;gap:18px}',

		'.ta-hero,.ta-panel,.ta-stat-card,.ta-insight-card,.ta-note-card,.ta-config-shell{position:relative;border-radius:24px;border:1px solid var(--ta-panel-border);background:var(--ta-panel-bg);box-shadow:var(--ta-shadow);color:var(--ta-text);overflow:hidden}',

		'.ta-hero{display:grid;grid-template-columns:minmax(0,1.6fr) minmax(310px,.92fr);gap:28px;padding:24px 28px;border-color:rgba(177,225,218,.95);background:linear-gradient(90deg,rgba(226,235,253,.96),rgba(244,241,251,.96) 58%,rgba(239,248,250,.96))}',

		'.ta-dark .ta-hero{border-color:rgba(120,146,188,.24);background:linear-gradient(90deg,rgba(18,30,52,.96),rgba(16,28,48,.96) 58%,rgba(13,23,42,.96))}',

		'.ta-hero:before{display:none}',

		'.ta-hero.is-healthy{border-color:rgba(13,148,136,.28)}',

		'.ta-hero.is-attention{border-color:rgba(245,158,11,.28)}',

		'.ta-hero.is-warning{border-color:rgba(239,68,68,.28)}',

		'.ta-hero-main{display:flex;gap:22px;align-items:flex-start;min-width:0}',

		'.ta-hero-badge{position:relative;flex:none;display:flex;align-items:center;justify-content:center;width:98px;height:98px;border-radius:28px;background:var(--ta-hero-badge);box-shadow:none}',

		'.ta-hero-badge-code{color:#fff;font-size:1.16rem;font-weight:800;letter-spacing:.03em}',

		'.ta-hero-badge-dot{position:absolute;right:12px;bottom:12px;width:18px;height:18px;border-radius:50%;background:#fff;box-shadow:0 0 0 7px rgba(255,255,255,.36)}',

		'.ta-hero-copy{display:flex;flex-direction:column;gap:12px;min-width:0;padding-top:2px}',

		'.ta-hero-kicker{font-size:.92rem;font-weight:700;letter-spacing:0;text-transform:uppercase;color:#66778f}',

		'.ta-dark .ta-hero-kicker{color:var(--ta-text-muted)}',

		'.ta-hero-title{display:inline-flex;align-items:center;justify-content:center;width:fit-content;max-width:100%;padding:14px 38px;border:1px solid #b9d8ca;border-radius:28px;background:linear-gradient(180deg,#edf7f3,#dcebdf);color:#146d61;font-size:1.18rem;line-height:1.2;font-weight:800;letter-spacing:0;box-shadow:none}',

		'.ta-hero-title.is-enabled{color:var(--ta-good);border-color:rgba(34,197,94,.24);background:linear-gradient(180deg,rgba(34,197,94,.12),rgba(34,197,94,.06))}',

		'.ta-hero-title.is-disabled{border-color:#f0c1cb;background:linear-gradient(180deg,#fff3f5,#ffe3e7);color:#b0374f}',

		'.ta-dark .ta-hero-title.is-disabled{border-color:rgba(239,68,68,.24);background:linear-gradient(180deg,rgba(239,68,68,.16),rgba(239,68,68,.08));color:var(--ta-danger)}',

		'.ta-hero-summary{font-size:1rem;line-height:1.62;color:#16304c;max-width:36rem}',

		'.ta-dark .ta-hero-summary{color:var(--ta-text)}',

		'.ta-hero-side{display:grid;gap:16px;align-content:start}',

		'.ta-section-pill{display:grid;gap:4px;padding:20px 24px;border-radius:20px;background:rgba(252,253,255,.97);border:1px solid #e2eaf4;backdrop-filter:none}',

		'.ta-dark .ta-section-pill{background:rgba(255,255,255,.03);border-color:var(--ta-panel-border-soft)}',

		'.ta-section-pill-label{font-size:1rem;font-weight:700;color:#6b7f9b;letter-spacing:0}',

		'.ta-dark .ta-section-pill-label{color:var(--ta-text-muted)}',

		'.ta-section-pill-value{font-size:1.18rem;font-weight:800;line-height:1.3;color:#0d1a33}',

		'.ta-dark .ta-section-pill-value{color:var(--ta-text-strong)}',

		'.ta-chip-list{display:flex;flex-wrap:wrap;gap:8px}',

		'.ta-chip{display:inline-flex;align-items:center;max-width:100%;padding:6px 11px;border-radius:999px;background:var(--ta-chip-bg);border:1px solid var(--ta-chip-border);color:var(--ta-chip);font-size:.8rem;font-weight:700;line-height:1.2;word-break:break-word}',

		'.ta-chip.is-ok{background:var(--ta-good-bg);color:var(--ta-good)}',

		'.ta-chip.is-warning{background:var(--ta-warn-bg);color:var(--ta-warn)}',

		'.ta-chip.is-info{background:var(--ta-info-bg);color:var(--ta-info)}',

		'.ta-chip.is-muted{background:rgba(148,163,184,.14);color:var(--ta-text-muted)}',

		'.ta-status-strip{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:16px}',

		'.ta-status-item{display:flex;align-items:flex-start;justify-content:flex-start;flex-direction:column;gap:8px;min-width:0;min-height:110px;padding:20px 24px;border:1px solid #dde6f1;border-radius:20px;background:rgba(252,253,255,.98);box-shadow:none;color:var(--ta-text)}',

		'.ta-dark .ta-status-item{border-color:var(--ta-panel-border);background:var(--ta-panel-bg)}',

		'.ta-status-item span{font-size:1rem;font-weight:700;color:#6a82a6;white-space:nowrap}',

		'.ta-dark .ta-status-item span{color:var(--ta-text-muted)}',

		'.ta-status-item strong{width:100%;font-size:1.15rem;line-height:1.3;color:#0a1730;text-align:right;word-break:break-word}',

		'.ta-dark .ta-status-item strong{color:var(--ta-text-strong)}',

		'.ta-summary-grid,.ta-focus-grid,.ta-engine-grid{display:grid;gap:14px}',

		'.ta-summary-grid{grid-template-columns:repeat(auto-fit,minmax(220px,1fr))}',

		'.ta-focus-grid{grid-template-columns:repeat(auto-fit,minmax(260px,1fr))}',

		'.ta-engine-grid{grid-template-columns:repeat(auto-fit,minmax(210px,1fr))}',

		'.ta-stat-card{display:flex;flex-direction:column;gap:10px;padding:18px;min-height:182px}',

		'.ta-stat-card.is-primary{border-color:rgba(37,99,235,.22)}',

		'.ta-stat-card.is-good{border-color:rgba(13,148,136,.24)}',

		'.ta-stat-card.is-warning{border-color:rgba(245,158,11,.24)}',

		'.ta-stat-card.is-accent{border-color:rgba(124,58,237,.24)}',

		'.ta-stat-title{font-size:.82rem;font-weight:700;letter-spacing:.05em;text-transform:uppercase;color:var(--ta-text-muted)}',

		'.ta-stat-value{font-size:1.34rem;font-weight:800;line-height:1.25;color:var(--ta-text-strong)}',

		'.ta-stat-detail{margin-top:auto;font-size:.88rem;line-height:1.55;color:var(--ta-text-muted)}',

		'.ta-insight-card{display:flex;flex-direction:column;gap:12px;padding:18px}',

		'.ta-insight-head{display:flex;align-items:flex-start;justify-content:space-between;gap:12px}',

		'.ta-insight-title{font-size:1rem;font-weight:800;color:var(--ta-text-strong)}',

		'.ta-insight-value{font-size:1rem;font-weight:800;color:var(--ta-text)}',

		'.ta-insight-detail{font-size:.9rem;line-height:1.6;color:var(--ta-text-muted)}',

		'.ta-kv-list{display:grid;gap:10px}',

		'.ta-kv-row{display:grid;grid-template-columns:minmax(112px,42%) 1fr;gap:10px;padding:10px 12px;border-radius:14px;background:var(--ta-info-bg);border:1px solid var(--ta-panel-border-soft)}',

		'.ta-kv-label{font-size:.84rem;color:var(--ta-text-muted);line-height:1.45}',

		'.ta-kv-value{font-size:.9rem;line-height:1.45;color:var(--ta-text-strong);text-align:right;word-break:break-word}',

		'.ta-panel{padding:22px 28px}',

		'.ta-panel-head{display:flex;flex-direction:column;gap:6px;margin-bottom:18px}',

		'.ta-panel-title{font-size:1.08rem;font-weight:800;color:var(--ta-text-strong)}',

		'.ta-panel-subtitle{font-size:1rem;line-height:1.6;color:#6f84a4}',

		'.ta-dark .ta-panel-subtitle{color:var(--ta-text-muted)}',

		'.ta-engine-card{display:flex;flex-direction:column;gap:12px;padding:16px;border-radius:18px;background:var(--ta-info-bg);border:1px solid var(--ta-panel-border-soft)}',

		'.ta-engine-card.is-selected{border-color:rgba(37,99,235,.25);background:rgba(37,99,235,.06)}',

		'.ta-engine-card.is-live{border-color:rgba(13,148,136,.26);background:rgba(13,148,136,.08)}',

		'.ta-engine-top{display:flex;align-items:center;gap:12px}',

		'.ta-engine-code{display:inline-flex;align-items:center;justify-content:center;min-width:48px;height:34px;padding:0 10px;border-radius:12px;background:var(--ta-hero-badge);color:#fff;font-size:.78rem;font-weight:800;letter-spacing:.06em}',

		'.ta-engine-name{font-size:.96rem;font-weight:800;color:var(--ta-text-strong)}',

		'.ta-engine-detail{font-size:.88rem;line-height:1.55;color:var(--ta-text-muted)}',

		'.ta-telemetry-grid{display:grid;grid-template-columns:minmax(0,1.45fr) minmax(280px,.9fr);gap:14px}',

		'.ta-side-stack{display:grid;gap:14px}',

		'.ta-note-card{padding:16px}',

		'.ta-note-title{font-size:.92rem;font-weight:800;color:var(--ta-text-strong);margin-bottom:10px}',

		'.ta-note-list{margin:0;padding-left:18px;display:grid;gap:8px;color:var(--ta-text);line-height:1.55}',

		'.ta-note-card.is-healthy{border-color:rgba(13,148,136,.24)}',

		'.ta-note-card.is-attention{border-color:rgba(245,158,11,.24)}',

		'.ta-note-card.is-warning{border-color:rgba(239,68,68,.24)}',

		'.ta-info-grid{display:grid;gap:10px}',

		'.ta-info-row{display:grid;grid-template-columns:minmax(120px,42%) 1fr;gap:12px;align-items:start;padding:11px 12px;border-radius:14px;background:var(--ta-info-bg);border:1px solid var(--ta-panel-border-soft)}',

		'.ta-info-label{font-size:.84rem;color:var(--ta-text-muted);line-height:1.45}',

		'.ta-info-value{display:flex;align-items:flex-start;justify-content:flex-end;gap:8px;font-size:.9rem;color:var(--ta-text-strong);font-weight:700;line-height:1.45;text-align:right;word-break:break-word}',

		'.ta-info-value .ta-chip-list{justify-content:flex-end}',

		'.ta-capability-grid{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:12px}',

		'.ta-ppe-panel .ta-panel-head{margin-bottom:16px}',

		'.ta-progress-list{display:grid;gap:18px}',

		'.ta-progress-row{display:grid;gap:8px}',

		'.ta-progress-head{display:flex;align-items:flex-end;justify-content:space-between;gap:12px}',

		'.ta-progress-label{font-size:1.02rem;font-weight:700;color:var(--ta-text-strong)}',

		'.ta-progress-value{font-size:.95rem;color:#6b7f9b;white-space:nowrap}',

		'.ta-dark .ta-progress-value{color:var(--ta-text-muted)}',

		'.ta-progress-track{height:12px;border-radius:999px;background:#e8edf4;overflow:hidden;border:1px solid #e2eaf2}',

		'.ta-dark .ta-progress-track{background:rgba(148,163,184,.14);border-color:var(--ta-panel-border-soft)}',

		'.ta-progress-fill{position:relative;height:100%;border-radius:999px;background:linear-gradient(90deg,#44c15b,#3ebf78);box-shadow:none}',

		'.ta-progress-fill.is-active:before{content:"";position:absolute;left:0;top:50%;width:14px;height:14px;border-radius:50%;background:#44c15b;transform:translateY(-50%);box-shadow:0 0 0 2px rgba(68,193,91,.18)}',

		/* ===== Config panel (form) ===== */
		'.ta-config-shell{padding:18px}',

		'.ta-config-shell>.cbi-map{margin:0;background:transparent;border:none;box-shadow:none;padding:0}',

		'.ta-config-shell h2{display:block;margin:0 0 16px;padding:18px 22px;border-radius:16px;border:1px solid #edf2f8;background:rgba(255,255,255,.98);font-size:1.28rem;font-weight:800;color:#0b1730}',

		'.ta-dark .ta-config-shell h2{background:rgba(255,255,255,.03);border-color:var(--ta-panel-border-soft);color:var(--ta-text-strong)}',

		'.ta-config-shell .cbi-map-descr{margin:0 0 18px;padding:0 8px;color:#6f84a4;font-size:1rem}',

		'.ta-dark .ta-config-shell .cbi-map-descr{color:var(--ta-text-muted)}',

		'.ta-config-shell .cbi-section{margin-top:0;padding:0 8px}',

		'.ta-config-shell .cbi-section-node{border:1px solid #edf2f8;border-radius:16px;background:rgba(255,255,255,.98);padding:0;overflow:visible}',

		'.ta-dark .ta-config-shell .cbi-section-node{border-color:var(--ta-panel-border-soft);background:rgba(255,255,255,.02)}',

		'.ta-config-shell .cbi-tabmenu{margin:0;padding:0 8px 12px;gap:12px;display:flex;flex-wrap:wrap;border-bottom:none;list-style:none}',

		'.ta-config-shell .cbi-tabmenu li{display:inline-flex;align-items:stretch;justify-content:center;flex:0 1 148px;min-width:128px;margin:0;float:none;border:1px solid #dfe7f2;border-radius:16px!important;min-height:44px;background:#fff!important;box-shadow:none!important;overflow:hidden;transition:background-color .18s ease,color .18s ease,border-color .18s ease}',

		'.ta-dark .ta-config-shell .cbi-tabmenu li{background:rgba(255,255,255,.04)!important;border-color:var(--ta-panel-border)!important}',

		'.ta-config-shell .cbi-tabmenu li a{display:inline-flex;align-items:center;justify-content:center;width:100%;min-height:44px;padding:12px 18px;border:none!important;border-radius:0!important;background:none!important;color:var(--ta-chip)!important;-webkit-text-fill-color:var(--ta-chip);font-size:1rem;font-weight:700;line-height:1.2;text-align:center;box-shadow:none!important;transition:background-color .18s ease,color .18s ease,border-color .18s ease}',

		'.ta-config-shell .cbi-tabmenu li.cbi-tab,.ta-config-shell .cbi-tabmenu li.active{background:linear-gradient(90deg,#1d9a9c,#4771eb)!important;border-color:transparent!important;box-shadow:none!important}',

		'.ta-config-shell .cbi-tabmenu li.cbi-tab a,.ta-config-shell .cbi-tabmenu li.active a{color:#fff!important;-webkit-text-fill-color:#fff!important}',

		'.ta-config-shell .cbi-tab-descr,.ta-config-shell .cbi-section-descr,.ta-config-shell .cbi-value-description{font-size:1rem;line-height:1.6;color:var(--ta-text-muted)!important;-webkit-text-fill-color:var(--ta-text-muted);opacity:1}',

		'.ta-config-shell label,.ta-config-shell .cbi-value-title,.ta-config-shell .cbi-section-node{color:var(--ta-text)}',

		'.ta-dark .ta-config-shell label,.ta-dark .ta-config-shell .cbi-value-title{color:var(--ta-text)!important}',

		'.ta-config-shell input[type="text"],.ta-config-shell input:not([type]),.ta-config-shell select,.ta-config-shell textarea,.ta-config-shell .cbi-dropdown{min-height:46px;box-sizing:border-box;border-radius:14px!important;background:var(--ta-button-bg)!important;border:1px solid var(--ta-panel-border)!important;color:var(--ta-text)!important;-webkit-text-fill-color:var(--ta-text);box-shadow:0 0 0 1px rgba(255,255,255,.02) inset!important}',

		'.ta-config-shell .cbi-dropdown>ul:not(.dropdown),.ta-config-shell .cbi-dropdown ul.preview{background:var(--ta-button-bg)!important;border-radius:14px!important}',

		'.ta-config-shell .cbi-dropdown ul.dropdown{z-index:10000!important;background:var(--ta-button-bg)!important;border:1px solid var(--ta-panel-border)!important;color:var(--ta-text)!important}',

		'.ta-config-shell .cbi-value{padding:20px 24px;border-top:1px solid #eff4f9}',

		'.ta-dark .ta-config-shell .cbi-value{border-top-color:var(--ta-panel-border-soft)}',

		'.ta-config-shell .cbi-value:first-child{border-top:none}',

		'.ta-config-shell .cbi-value-title{font-size:1rem;font-weight:700;color:#29405e}',

		'.ta-dark .ta-config-shell .cbi-value-title{color:var(--ta-text-strong)!important}',

		'.ta-config-shell .cbi-page-actions{display:flex;flex-wrap:wrap;gap:10px;align-items:center;justify-content:flex-end;margin:18px 8px 0;padding:0;background:transparent;border:none;box-shadow:none}',

		'.ta-config-shell .cbi-page-actions>*{margin:0!important;float:none!important}',

		'.ta-config-shell .cbi-page-actions .cbi-button{display:inline-flex;align-items:center;justify-content:center;min-height:52px;padding:0 24px;border-radius:10px!important;border:1px solid var(--ta-panel-border-soft)!important;background:var(--ta-button-bg)!important;color:var(--ta-text)!important;box-shadow:none!important;transition:transform .16s ease,filter .16s ease}',

		'.ta-config-shell .cbi-page-actions .cbi-button:hover{transform:translateY(-1px);filter:brightness(1.02)}',

		'.ta-config-shell .cbi-page-actions .cbi-button-apply,.ta-config-shell .cbi-page-actions .cbi-button-save{background:linear-gradient(90deg,#5672ef,#5d67ea)!important;border-color:transparent!important;color:#fff!important;box-shadow:none!important}',

		'.ta-config-shell .cbi-page-actions .cbi-button-reset{background:linear-gradient(180deg,#ef4c70,#ea3758)!important;color:#fff!important;border-color:transparent!important}',

		/* ===== Compact overview variants ===== */
		'.ta-compact-hero{display:grid;gap:14px;padding:18px}',

		'.ta-compact-hero-top{display:flex;align-items:flex-start;justify-content:space-between;gap:14px}',

		'.ta-compact-hero-copy{display:grid;gap:8px;min-width:0}',

		'.ta-compact-kicker{font-size:.76rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase;color:var(--ta-text-muted)}',

		'.ta-compact-headline{font-size:1.18rem;font-weight:800;line-height:1.3;color:var(--ta-text-strong)}',

		'.ta-compact-summary{font-size:.92rem;line-height:1.6;color:var(--ta-text-muted)}',

		'.ta-compact-state{display:inline-flex;align-items:center;justify-content:center;min-width:132px;min-height:42px;padding:0 18px;border-radius:999px;font-size:.92rem;font-weight:800;line-height:1.2;text-align:center;box-shadow:0 8px 16px rgba(15,23,42,.08)}',

		'.ta-compact-state.is-enabled{background:var(--ta-good-bg);color:var(--ta-good)}',

		'.ta-compact-state.is-disabled{background:var(--ta-danger-bg);color:var(--ta-danger)}',

		'.ta-compact-grid{display:grid;grid-template-columns:minmax(0,1.35fr) minmax(240px,.8fr);gap:14px}',

		'.ta-compact-summary-grid{grid-template-columns:repeat(2,minmax(0,1fr));align-items:stretch}',

		'.ta-compact-summary-grid .ta-stat-card{min-height:0;padding:16px}',

		'.ta-compact-detail-panel .ta-info-grid{height:100%}',

		/* ===== Screenshot-matched Argon compact skin ===== */
		'.ta-page{gap:14px;padding:12px 0 0;--ta-shadow:0 6px 16px rgba(73,99,135,.08);--ta-panel-border:#dfe7f1;--ta-panel-border-soft:#edf2f7;--ta-panel-bg:linear-gradient(180deg,rgba(255,255,255,.98),rgba(249,252,255,.99));--ta-text:#263b58;--ta-text-strong:#0b1830;--ta-text-muted:#6b809c;--ta-button-bg:linear-gradient(180deg,#fbfdff,#f5f8fc)}',

		'.ta-page:before{top:-10px;height:8px;background:#5b6fee;border-radius:0 0 999px 999px}',

		'.ta-hero,.ta-panel,.ta-config-shell{border-radius:18px;border-color:var(--ta-panel-border);box-shadow:var(--ta-shadow);background:var(--ta-panel-bg)}',

		'.ta-hero{min-height:142px;padding:22px 24px;grid-template-columns:minmax(0,1.45fr) minmax(300px,.95fr);gap:26px;background:linear-gradient(100deg,#e9f1ff 0%,#f8fbff 52%,#eefbff 100%);border-color:#bfe8e8}',

		'.ta-dark .ta-hero{background:linear-gradient(100deg,rgba(22,34,58,.98),rgba(17,27,48,.98) 52%,rgba(14,34,45,.98));border-color:rgba(115,151,190,.34)}',

		'.ta-hero-main{gap:18px;align-items:flex-start}',

		'.ta-hero-badge{width:74px;height:74px;border-radius:20px;background:linear-gradient(145deg,#2563eb,#62a5ff)}',

		'.ta-hero-badge-code{font-size:1rem}',

		'.ta-hero-badge-dot{right:9px;bottom:9px;width:11px;height:11px;box-shadow:0 0 0 5px rgba(255,255,255,.42)}',

		'.ta-hero-copy{gap:10px;padding-top:0}',

		'.ta-hero-kicker{font-size:.72rem;letter-spacing:.02em;color:#6a7d98}',

		'.ta-hero-title{min-height:34px;padding:7px 18px;border-radius:999px;font-size:.95rem;background:linear-gradient(180deg,#eefaf4,#dbefe5);border-color:#bde5d2;color:#177a69}',

		'.ta-hero-summary{font-size:.88rem;line-height:1.5;color:#16304c}',

		'.ta-hero-side{gap:10px}',

		'.ta-section-pill{min-height:58px;padding:13px 18px;border-radius:16px;background:rgba(255,255,255,.72);border-color:#e0eaf5}',

		'.ta-section-pill-label{font-size:.78rem;font-weight:700;color:#6b7f9b}',

		'.ta-section-pill-value{font-size:.98rem;color:#0b1830}',

		'.ta-status-strip{gap:10px;grid-template-columns:repeat(4,minmax(0,1fr))}',

		'.ta-status-item{min-height:58px;padding:13px 16px;border-radius:14px;border-color:#dee7f1;background:#fff}',

		'.ta-status-item span{font-size:.78rem;color:#6a7f9d}',

		'.ta-status-item strong{font-size:.92rem;text-align:right;color:#0b1830}',

		'.ta-panel{padding:18px 22px}',

		'.ta-panel-head{gap:4px;margin-bottom:14px}',

		'.ta-panel-title{font-size:1rem}',

		'.ta-panel-subtitle{font-size:.84rem;line-height:1.45;color:#7387a2}',

		'.ta-progress-list{gap:13px}',

		'.ta-progress-row{gap:6px}',

		'.ta-progress-label{font-size:.9rem}',

		'.ta-progress-value{font-size:.78rem}',

		'.ta-progress-track{height:8px;border:none;background:#e8edf4}',

		'.ta-progress-fill.is-active:before{width:9px;height:9px;box-shadow:0 0 0 2px rgba(68,193,91,.18)}',

		'.ta-config-shell{padding:14px}',

		'.ta-config-shell h2{margin:0 0 16px;padding:15px 16px;border-radius:4px;border:none;background:#fff;font-size:1.12rem;color:#0b1830}',

		'.ta-config-shell .cbi-map-descr{margin:0 0 18px;padding:0 12px;font-size:.82rem;color:#7187a3}',

		'.ta-config-shell .cbi-section{padding:0}',

		'.ta-config-shell .cbi-section-node{border-radius:0;border:none;background:#fff;box-shadow:0 1px 8px rgba(54,76,108,.04)}',

		'.ta-config-shell .cbi-tabmenu{padding:0 14px 12px;gap:8px;background:#fff}',

		'.ta-config-shell .cbi-tabmenu li{flex:0 1 132px;min-width:116px;min-height:42px;border-radius:8px!important;border-color:#dfe7f1;background:#fff!important}',

		'.ta-config-shell .cbi-tabmenu li a{min-height:42px;padding:10px 14px;font-size:.86rem}',

		'.ta-config-shell .cbi-tabmenu li.cbi-tab,.ta-config-shell .cbi-tabmenu li.active{background:linear-gradient(90deg,#168f8f,#3d70f0)!important}',

		'.ta-config-shell .cbi-tab-descr,.ta-config-shell .cbi-section-descr,.ta-config-shell .cbi-value-description{font-size:.78rem;line-height:1.45;color:#7187a3!important;-webkit-text-fill-color:#7187a3}',

		'.ta-config-shell .cbi-value{padding:18px 20px;border-top:1px solid #f0f4f8}',

		'.ta-config-shell .cbi-value-title{font-size:.84rem;color:#314865}',

		'.ta-config-shell select,.ta-config-shell input[type="text"],.ta-config-shell input[type="number"],.ta-config-shell input:not([type]){min-height:34px;min-width:220px;max-width:100%;padding:6px 12px;border-radius:10px!important;border:1px solid #dce6f1!important;background:linear-gradient(180deg,#fbfdff,#f4f8fd)!important;color:#263b58!important;-webkit-text-fill-color:#263b58;font-size:.84rem;box-shadow:none!important}',

		'.ta-config-shell select:focus,.ta-config-shell input:focus{border-color:#7aa8ff!important;box-shadow:0 0 0 3px rgba(91,111,238,.12)!important;outline:none}',

		'.ta-config-shell select option{color:#1f3150;background:#fff}',

		'.ta-config-shell .cbi-page-actions{margin:16px 0 0;gap:10px;align-items:center;justify-content:flex-end}',

		'.ta-config-shell .cbi-page-actions .cbi-button{box-sizing:border-box;min-width:96px;min-height:40px;padding:0 18px;border-radius:10px!important;font-size:.86rem;font-weight:700;line-height:1.2;white-space:nowrap}',

		'.ta-config-shell .cbi-page-actions .cbi-button-apply,.ta-config-shell .cbi-page-actions .cbi-button-save{background:#5b6fee!important}',

		'.ta-config-shell .cbi-page-actions .cbi-button-reset{background:#ef4565!important}',

		'.ta-dark .ta-status-item,.ta-dark .ta-config-shell h2,.ta-dark .ta-config-shell .cbi-section-node,.ta-dark .ta-config-shell .cbi-tabmenu{background:rgba(255,255,255,.04)}',

		'.ta-dark .ta-section-pill{background:rgba(255,255,255,.04)}',

		'.ta-dark .ta-config-shell select,.ta-dark .ta-config-shell input[type="text"],.ta-dark .ta-config-shell input[type="number"],.ta-dark .ta-config-shell input:not([type]){background:rgba(255,255,255,.06)!important;color:var(--ta-text-strong)!important;-webkit-text-fill-color:var(--ta-text-strong);border-color:rgba(148,163,184,.24)!important}',

		/* ===== State text colors ===== */
		'.ta-hero-title.is-enabled,.ta-compact-state.is-enabled,.ta-status-value.is-enabled,.ta-section-pill.is-good .ta-section-pill-value,.ta-chip.is-ok{color:var(--ta-good)!important;-webkit-text-fill-color:var(--ta-good)}',

		'.ta-hero-title.is-disabled,.ta-compact-state.is-disabled,.ta-status-value.is-disabled,.ta-section-pill.is-muted .ta-section-pill-value,.ta-chip.is-muted{color:var(--ta-text-muted)!important;-webkit-text-fill-color:var(--ta-text-muted)}',

		'.ta-hero-title.is-disabled,.ta-compact-state.is-disabled{border-color:rgba(148,163,184,.30);background:rgba(148,163,184,.12)}',

		/* ===== Responsive ===== */
		'@media (max-width:1040px){.ta-hero{grid-template-columns:1fr}.ta-telemetry-grid{grid-template-columns:1fr}.ta-status-strip{grid-template-columns:repeat(2,minmax(0,1fr))}.ta-compact-grid{grid-template-columns:1fr}}',

		'@media (max-width:760px){.ta-page{padding-top:10px}.ta-page:before{top:-4px;height:4px}.ta-hero{padding:20px}.ta-hero-main{flex-direction:column}.ta-hero-badge{width:84px;height:84px;border-radius:24px}.ta-hero-title{padding:12px 24px}.ta-section-pill,.ta-status-item,.ta-panel{padding:18px 20px}.ta-status-strip{grid-template-columns:1fr}.ta-status-item strong{text-align:left}.ta-kv-row,.ta-info-row{grid-template-columns:1fr}.ta-kv-value,.ta-info-value,.ta-info-value .ta-chip-list{text-align:left;justify-content:flex-start}.ta-progress-head{flex-direction:column;align-items:flex-start}.ta-progress-value{white-space:normal}.ta-compact-hero-top{flex-direction:column;align-items:flex-start}.ta-compact-state{width:100%;justify-content:flex-start}.ta-compact-summary-grid{grid-template-columns:1fr}.ta-config-shell{padding:14px}.ta-config-shell h2{padding:16px 18px}.ta-config-shell .cbi-tabmenu{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px;padding:0 0 12px}.ta-config-shell .cbi-tabmenu li{width:100%;min-width:0}.ta-config-shell .cbi-tabmenu li a{padding:10px 8px}.ta-config-shell .cbi-page-actions{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}.ta-config-shell .cbi-page-actions .cbi-button{width:100%;min-width:0;min-height:42px;padding:0 12px;justify-content:center}.ta-config-shell .cbi-page-actions .cbi-button-apply{grid-column:1/-1;order:-1}}',

		'@media (max-width:380px){.ta-config-shell .cbi-tabmenu,.ta-config-shell .cbi-page-actions{grid-template-columns:1fr}.ta-config-shell .cbi-page-actions .cbi-button{min-height:40px}.ta-config-shell .cbi-page-actions .cbi-button-apply{grid-column:auto}}'
	].join('\n'));
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('turboacc'),
			L.resolveDefault(getSystemFeatures(), {}),
			L.resolveDefault(getServiceStatus(), []),
			L.resolveDefault(getMTKPPEStatus(), {})
		]);
	},

	render: function(data) {
		var features = data[1] || {};
		var service = data[2] || [];
		var ppeStats = data[3] || {};
		var overviewRoot = E('div', { 'class': 'ta-overview-root' });
		var page = applyThemeClass(E('div', { 'class': 'ta-page' }, [
			renderStyle(),
			overviewRoot
		]), 'ta-dark');
		var map = buildForm(features, getConfigState(features));

		function refreshOverview(nextService, nextPpe) {
			dom.content(overviewRoot, renderOverviewContent({
				features: features,
				config: getConfigState(features),
				service: nextService,
				ppe: getPPESummary(nextPpe || {})
			}));
		}

		refreshOverview(service, ppeStats);

		return map.render().then(function(mapNode) {
			page.appendChild(E('div', { 'class': 'ta-config-shell' }, [ mapNode ]));

			poll.add(function() {
				return Promise.all([
					L.resolveDefault(getServiceStatus(), []),
					L.resolveDefault(getMTKPPEStatus(), {})
				]).then(function(nextData) {
					refreshOverview(nextData[0], nextData[1]);
				});
			}, 3);

			return page;
		});
	}
});
