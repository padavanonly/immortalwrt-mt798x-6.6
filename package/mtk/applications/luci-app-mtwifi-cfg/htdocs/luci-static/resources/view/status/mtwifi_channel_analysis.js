'use strict';
'require view';
'require request';
'require network';
'require ui';
'require rpc';
'require tools.prng as random';

return view.extend({
	callFrequencyList: rpc.declare({
		object: 'iwinfo',
		method: 'freqlist',
		params: [ 'device' ],
		expect: { results: [] }
	}),

	callInfo: rpc.declare({
		object: 'iwinfo',
		method: 'info',
		params: [ 'device' ],
		expect: { }
	}),

	render_signal_badge: function(signalPercent, signalValue) {
		var icon, title, value;

		if (signalPercent < 0)
			icon = L.resource('icons/signal-none.svg');
		else if (signalPercent == 0)
			icon = L.resource('icons/signal-000-000.svg');
		else if (signalPercent < 25)
			icon = L.resource('icons/signal-000-025.svg');
		else if (signalPercent < 50)
			icon = L.resource('icons/signal-025-050.svg');
		else if (signalPercent < 75)
			icon = L.resource('icons/signal-050-075.svg');
		else
			icon = L.resource('icons/signal-075-100.svg');

		value = '%d\xa0%s'.format(signalValue, _('dBm'));
		title = '%s: %d %s'.format(_('Signal'), signalValue, _('dBm'));

		return E('div', {
			'class': 'ifacebadge',
			'title': title,
			'data-signal': signalValue
		}, [
			E('img', { 'src': icon }),
			value
		]);
	},

	parse_htmode_width: function(htmode) {
		if (typeof(htmode) !== 'string' || !htmode)
			return { label: '20 MHz', units: 2 };

		if (htmode.indexOf('320') !== -1)
			return { label: '320 MHz', units: 32 };

		if (htmode.indexOf('160') !== -1)
			return { label: '160 MHz', units: 16 };

		if (htmode.indexOf('80+80') !== -1 || htmode.indexOf('80_80') !== -1 ||
		    htmode.indexOf('80P80') !== -1)
			return { label: '80+80 MHz', units: 8 };

		if (htmode.indexOf('80') !== -1)
			return { label: '80 MHz', units: 8 };

		if (htmode.indexOf('40') !== -1)
			return { label: '40 MHz', units: 4 };

		return { label: '20 MHz', units: 2 };
	},

	add_wifi_to_graph: function(chan_analysis, res, scanCache, channels, channel_width) {
		const offset_tbl = chan_analysis.offset_tbl;
		const height = (chan_analysis.graph.offsetHeight || 400) - 2;
		const step = chan_analysis.col_width;
		const sig = (typeof(res.signal) === 'number' && !isNaN(res.signal)) ? res.signal : -90;
		const height_diff = Math.max(18, Math.min(height - 2, Math.round(sig * -4)));
		const valid_channels = [];

		if (!(step > 0))
			return;

		if (Array.isArray(channels)) {
			for (var i = 0; i < channels.length; i++) {
				var c = channels[i];
				if (c && offset_tbl[c] != null && valid_channels.indexOf(c) === -1)
					valid_channels.push(c);
			}
		}

		var plot_width = (typeof(channel_width) === 'number' && !isNaN(channel_width) && channel_width > 0)
			? channel_width : 2;

		if (valid_channels.length === 0) {
			if (res.channel != null && offset_tbl[res.channel] != null) {
				valid_channels.push(res.channel);
				plot_width = 2;
			} else {
				return;
			}
		}

		var primary_offset = (res.channel != null && offset_tbl[res.channel] != null)
			? offset_tbl[res.channel] : offset_tbl[valid_channels[0]];

		if (scanCache[res.bssid].color == null)
			scanCache[res.bssid].color = random.derive_color(res.bssid);

		if (scanCache[res.bssid].graph == null) {
			const svgns = 'http://www.w3.org/2000/svg';
			const group = document.createElementNS(svgns, 'g');
			const line = document.createElementNS(svgns, 'polyline');
			const text = document.createElementNS(svgns, 'text');
			const color = scanCache[res.bssid].color;

			group.setAttribute('data-ca-dynamic', '1');
			line.setAttribute('style', 'fill:' + color + '4f;stroke:' + color + ';stroke-width:0.5');
			text.setAttribute('style', 'fill:' + color + ';font-size:9pt; font-family:sans-serif; text-shadow:1px 1px 1px #000');
			text.appendChild(document.createTextNode(res.ssid || res.bssid));

			group.appendChild(line);
			group.appendChild(text);

			chan_analysis.graph.firstElementChild.appendChild(group);
			scanCache[res.bssid].graph = { group: group, line: line, lines: [ line ], text: text };
		}

		const g = scanCache[res.bssid].graph;
		const color = scanCache[res.bssid].color;

		if (plot_width > 2 && primary_offset != null) {
			if (!g.main) {
				const main = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');
				main.setAttribute('style', 'fill:url(#GradientVerticalCenteredBlack)');
				g.group.appendChild(main);
				g.main = main;
			}
			const main_points = [
				(primary_offset - (step * 2)) + ',' + height,
				(primary_offset - step) + ',' + height_diff,
				(primary_offset + step) + ',' + height_diff,
				(primary_offset + (step * 2)) + ',' + height
			].join(' ');
			g.main.setAttribute('points', main_points);
			g.main.style.display = '';
		} else if (g.main) {
			g.main.style.display = 'none';
		}

		while (g.lines.length < valid_channels.length) {
			const extraLine = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');
			extraLine.setAttribute('style', 'fill:' + color + '4f;stroke:' + color + ';stroke-width:0.5');
			g.group.insertBefore(extraLine, g.text);
			g.lines.push(extraLine);
		}

		for (var idx = 0; idx < g.lines.length; idx++) {
			if (idx < valid_channels.length) {
				const chan_offset = offset_tbl[valid_channels[idx]];
				const points = [
					(chan_offset - (step * plot_width)) + ',' + height,
					(chan_offset - (step * (plot_width - 1))) + ',' + height_diff,
					(chan_offset + (step * (plot_width - 1))) + ',' + height_diff,
					(chan_offset + (step * plot_width)) + ',' + height
				].join(' ');
				g.lines[idx].setAttribute('points', points);
				g.lines[idx].style.display = '';
			} else {
				g.lines[idx].style.display = 'none';
			}
		}

		g.text.setAttribute('x', primary_offset - step);
		g.text.setAttribute('y', height_diff - 2);
		g.group.style.zIndex = Math.round(sig * -1);
		g.group.style.opacity = res.stale ? '0.5' : null;
	},

	ensure_channel_graph: function(radio) {
		var chan_analysis = radio.graph,
		    chan_graph = chan_analysis.graph,
		    G = chan_graph.firstElementChild,
		    width = chan_graph.offsetWidth;

		if (!G || width <= 2)
			return false;

		if (chan_analysis.builtWidth === width &&
		    Object.keys(chan_analysis.offset_tbl).length > 0)
			return true;

		var dynamicNodes = G.querySelectorAll('[data-ca-dynamic="1"]');
		for (var n = 0; n < dynamicNodes.length; n++)
			if (dynamicNodes[n].parentNode === G)
				G.removeChild(dynamicNodes[n]);

		for (var k in radio.scanCache)
			if (radio.scanCache[k])
				radio.scanCache[k].graph = null;

		chan_analysis.offset_tbl = {};

		var freq_tbl = chan_analysis.freq_tbl,
		    band = chan_analysis.band,
		    columns = (band != 2) ? freq_tbl.length * 4 : freq_tbl.length + 3,
		    step = columns > 0 ? (width - 2) / columns : 0,
		    curr_offset = step;

		if (!(step > 0))
			return false;

		function createGraphHLine(graph, pos, lineWidth, dash) {
			var elem = document.createElementNS('http://www.w3.org/2000/svg', 'line');
			elem.setAttribute('data-ca-dynamic', '1');
			elem.setAttribute('x1', pos);
			elem.setAttribute('y1', 0);
			elem.setAttribute('x2', pos);
			elem.setAttribute('y2', '100%');
			elem.setAttribute('style', 'stroke:black;stroke-width:' + lineWidth + ';stroke-dasharray:' + dash);
			graph.appendChild(elem);
		}

		function createGraphText(graph, pos, text) {
			var elem = document.createElementNS('http://www.w3.org/2000/svg', 'text');
			elem.setAttribute('data-ca-dynamic', '1');
			elem.setAttribute('y', 15);
			elem.setAttribute('style', 'fill:#eee; font-size:9pt; font-family:sans-serif; text-shadow:1px 1px 1px #000');
			elem.setAttribute('x', pos + 5);
			elem.appendChild(document.createTextNode(text));
			graph.appendChild(elem);
		}

		chan_analysis.col_width = step;
		chan_analysis.builtWidth = width;

		createGraphHLine(G, curr_offset, 0.1, 1);
		for (var i = 0; i < freq_tbl.length; i++) {
			var channel = freq_tbl[i];
			chan_analysis.offset_tbl[channel] = curr_offset + step;

			if (band != 2) {
				createGraphHLine(G, curr_offset + step, 0.1, 3);
				if (channel < 100)
					createGraphText(G, curr_offset - (step / 2), channel);
				else
					createGraphText(G, curr_offset - step, channel);
			} else {
				createGraphHLine(G, curr_offset + step, 0.1, 0);
				createGraphText(G, curr_offset + step, channel);
			}
			curr_offset += step;

			if ((band != 2) && freq_tbl[i + 1]) {
				var next_channel = freq_tbl[i + 1];
				/* Check if we are transitioning to another 5/6GHz band range */
				if ((next_channel - channel) == 4) {
					for (var j = 1; j < 4; j++) {
						chan_analysis.offset_tbl[channel + j] = curr_offset + step;
						if (j == 2)
							createGraphHLine(G, curr_offset + step, 0.1, 0);
						else
							createGraphHLine(G, curr_offset + step, 0.1, 1);
						curr_offset += step;
					}
				} else {
					chan_analysis.offset_tbl[channel + 1] = curr_offset + step;
					createGraphHLine(G, curr_offset + step, 0.1, 1);
					curr_offset += step;

					chan_analysis.offset_tbl[next_channel - 2] = curr_offset + step;
					createGraphHLine(G, curr_offset + step, 0.5, 0);
					curr_offset += step;

					chan_analysis.offset_tbl[next_channel - 1] = curr_offset + step;
					createGraphHLine(G, curr_offset + step, 0.1, 1);
					curr_offset += step;
				}
			}
		}
		createGraphHLine(G, curr_offset + step, 0.1, 1);

		return true;
	},

	renderScanData: function(radio, payload) {
		var results = (payload.results || []).slice(),
		    local_wifi = payload.local_wifi,
		    table = radio.table,
		    chan_analysis = radio.graph,
		    scanCache = radio.scanCache,
		    band = radio.band,
		    graphReady = this.ensure_channel_graph(radio),
		    rows = [];

		for (var k in scanCache) {
			if (scanCache[k]?.data)
				scanCache[k].data.stale = true;
		}

		for (var i = 0; i < results.length; i++) {
			if (!results[i] || !results[i].bssid)
				continue;

			if (scanCache[results[i].bssid] == null)
				scanCache[results[i].bssid] = {};

			scanCache[results[i].bssid].data = Object.assign({}, results[i], { stale: false });
		}

		if (local_wifi && local_wifi.bssid &&
		    local_wifi.bssid !== '00:00:00:00:00:00' &&
		    local_wifi.channel &&
		    band + 'g' == radio.dev.get('band')) {
			var localEntry = Object.assign({}, local_wifi, {
				signal: -10,
				ssid: 'Local Interface',
				stale: false
			});

			if (scanCache[local_wifi.bssid] == null)
				scanCache[local_wifi.bssid] = {};

			scanCache[local_wifi.bssid].data = localEntry;
			if (scanCache[local_wifi.bssid].color == null)
				scanCache[local_wifi.bssid].color = random.derive_color(local_wifi.bssid);

			var bwInfo = this.parse_htmode_width(local_wifi.htmode),
			    center_channels = [],
			    chan_width = bwInfo.units;

			if (local_wifi.center_chan1)
				center_channels.push(local_wifi.center_chan1);
			else
				center_channels.push(local_wifi.channel);

			if (local_wifi.center_chan2 && local_wifi.center_chan2 !== local_wifi.center_chan1) {
				center_channels.push(local_wifi.center_chan2);
				if (bwInfo.label === '80+80 MHz')
					chan_width = 8;
			}

			if (graphReady)
				this.add_wifi_to_graph(chan_analysis, localEntry, scanCache, center_channels, chan_width);

			rows.push([
				this.render_signal_badge(100, localEntry.signal),
				[
					E('span', { 'style': 'color:' + scanCache[local_wifi.bssid].color }, '⬤ '),
					localEntry.ssid
				],
				'%d'.format(localEntry.channel),
				'%h'.format(bwInfo.label),
				'%h'.format(localEntry.mode || '-'),
				'%h'.format(localEntry.bssid)
			]);
		}

		var combined = [];
		for (var bssid in scanCache) {
			if (local_wifi && bssid === local_wifi.bssid && band + 'g' == radio.dev.get('band'))
				continue;
			if (scanCache[bssid]?.data)
				combined.push(scanCache[bssid].data);
		}

		combined.sort(function(a, b) {
			var chDiff = (a.channel || 0) - (b.channel || 0);
			if (chDiff)
				return chDiff;

			var aSsid = a.ssid || '';
			var bSsid = b.ssid || '';
			if (aSsid < bSsid)
				return -1;
			if (aSsid > bSsid)
				return 1;

			var aBssid = a.bssid || '';
			var bBssid = b.bssid || '';
			if (aBssid < bBssid)
				return -1;
			if (aBssid > bBssid)
				return 1;

			return 0;
		});

		for (var j = 0; j < combined.length; j++) {
			var res = combined[j],
			    qv = res.quality || 0,
			    qm = res.quality_max || 0,
			    q = (qv > 0 && qm > 0) ? Math.floor((100 / qm) * qv) : 0,
			    s = res.stale ? 'opacity:0.5' : '',
			    c_chans = [res.channel],
			    c_width = 2;

			/* Skip WiFi not supported by the current band */
			if (band != res.band)
				continue;
			if (graphReady && chan_analysis.offset_tbl[res.channel] == null)
				continue;

			res.channel_width = '20 MHz';
			if (res.ht_operation != null) {
				if (res.ht_operation.secondary_channel_offset == 'below') {
					res.channel_width = '40 MHz';
					c_width = 4;
					c_chans[0] -= 2;
				} else if (res.ht_operation.secondary_channel_offset == 'above') {
					res.channel_width = '40 MHz';
					c_width = 4;
					c_chans[0] += 2;
				} else if (res.ht_operation.channel_width == 2040) {
					res.channel_width = '20 MHz (40 MHz Intolerant)';
				}
			}

			if (res.vht_operation?.channel_width > 40) {
				if (res.vht_operation.center_freq_1)
					c_chans[0] = res.vht_operation.center_freq_1;
				if (res.vht_operation.channel_width == 80) {
					c_width = 8;
					res.channel_width = '80 MHz';

					if (res.vht_operation.center_freq_2) {
						var diff = Math.abs(res.vht_operation.center_freq_2 -
						                    res.vht_operation.center_freq_1);

						if (diff == 8) {
							c_width = 16;
							res.channel_width = '160 MHz';
							c_chans[0] = Math.round((res.vht_operation.center_freq_1 +
							                         res.vht_operation.center_freq_2) / 2);
						} else if (diff > 8) {
							c_width = 8;
							res.channel_width = '80+80 MHz';
							c_chans.push(res.vht_operation.center_freq_2);
						}
					}
				} else if (res.vht_operation.channel_width == 8080) {
					res.channel_width = '80+80 MHz';
					c_width = 8;
					if (res.vht_operation.center_freq_2)
						c_chans.push(res.vht_operation.center_freq_2);
				} else if (res.vht_operation.channel_width == 160) {
					res.channel_width = '160 MHz';
					c_width = 16;
				}
			}

			if (res.he_operation?.channel_width > 20) {
				if (res.he_operation.center_freq_1)
					c_chans[0] = res.he_operation.center_freq_1;
				c_width = res.he_operation.channel_width / 10;
				switch (res.he_operation.channel_width) {
				case 40:
					res.channel_width = '40 MHz';
					break;
				case 80:
					res.channel_width = '80 MHz';
					break;
				case 160:
					res.channel_width = '160 MHz';
					if (res.he_operation.center_freq_2)
						c_chans[0] = res.he_operation.center_freq_2;
					break;
				}
			}

			if (res.eht_operation?.channel_width == 320) {
				c_width = 32;
				res.channel_width = '320 MHz';
				if (res.eht_operation.center_freq_2)
					c_chans[0] = res.eht_operation.center_freq_2;
				else if (res.eht_operation.center_freq_1)
					c_chans[0] = res.eht_operation.center_freq_1;
			}

			if (scanCache[res.bssid].color == null)
				scanCache[res.bssid].color = random.derive_color(res.bssid);

			if (graphReady)
				this.add_wifi_to_graph(chan_analysis, res, scanCache, c_chans, c_width);

			rows.push([
				E('span', { 'style': s }, this.render_signal_badge(q, res.signal)),
				E('span', { 'style': s }, [
					E('span', { 'style': 'color:' + scanCache[res.bssid].color }, '⬤ '),
					(res.ssid != null) ? '%h'.format(res.ssid) : E('em', _('hidden'))
				]),
				E('span', { 'style': s }, '%d'.format(res.channel)),
				E('span', { 'style': s }, '%h'.format(res.channel_width)),
				E('span', { 'style': s }, '%h'.format(res.mode)),
				E('span', { 'style': s }, '%h'.format(res.bssid))
			]);
		}

		cbi_update_table(table, rows, E('em', _('No entries available')));
	},

	handleScanRefresh: function(evOrRadio) {
		var radio = (evOrRadio && evOrRadio.graph && evOrRadio.table)
			? evOrRadio : this.radios[this.active_tab];

		if (!radio || radio.scanning)
			return Promise.resolve();

		radio.scanning = true;
		if (!radio.lastData)
			cbi_update_table(radio.table, [], E('em', { class: 'spinning' }, _('Starting wireless scan...')));

		return Promise.all([
			L.resolveDefault(radio.dev.getScanList(), null),
			L.resolveDefault(this.callInfo(radio.dev.getName()), null)
		]).then(L.bind(function(data) {
			var results = Array.isArray(data[0]) ? data[0] : [],
			    local_wifi = data[1];

			radio.loadedOnce = true;
			radio.scanning = false;

			if (data[0] == null && (!local_wifi || !local_wifi.bssid)) {
				cbi_update_table(radio.table, [], E('em', _('No entries available')));
				return;
			}

			radio.lastData = { results: results, local_wifi: local_wifi };
			this.renderScanData(radio, radio.lastData);
		}, this)).catch(L.bind(function(err) {
			radio.scanning = false;
			radio.loadedOnce = true;
			cbi_update_table(radio.table, [], E('em', (err && err.message) ? err.message : _('No entries available')));
		}, this));
	},

	radios: {},
	active_tab: null,

	loadSVG: function(src) {
		return request.get(src).then(function(response) {
			if (!response.ok)
				throw new Error(response.statusText);

			return E('div', {
				'id': 'channel_graph',
				'style': 'width:100%;height:400px;border:1px solid #000;background:#fff'
			}, E(response.text()));
		});
	},

	load: function() {
		return Promise.all([
			this.loadSVG(L.resource('svg/channel_analysis.svg')),
			network.getWifiDevices().then(L.bind(function(data) {
				var tasks = [], ret = {};

				for (var i = 0; i < data.length; i++) {
					ret[data[i].getName()] = { dev: data[i], freq: [] };

					tasks.push(L.resolveDefault(this.callFrequencyList(data[i].getName()), [])
					.then(L.bind(function(radio, freqData) {
						ret[radio.getName()].freq = Array.isArray(freqData) ? freqData : [];
					}, this, data[i])));
				}

				return Promise.all(tasks).then(function() { return ret; });
			}, this))
		]);
	},

	render: function(data) {
		var svg = data[0],
		    wifiDevs = data[1];

		this.radios = {};
		this.active_tab = null;

		var h2 = E('div', { 'class': 'cbi-title-section' }, [
			E('h2', { 'class': 'cbi-title-field' }, [ _('Channel Analysis') ]),
			E('div', { 'class': 'cbi-title-buttons' }, [
				E('button', {
					'class': 'cbi-button cbi-button-edit',
					'click': ui.createHandlerFn(this, 'handleScanRefresh')
				}, [ _('Refresh Channels') ])
			])
		]);

		var tabs = E('div', {}, E('div'));

		for (var ifname in wifiDevs) {
			var bands = {
				[2]: { title: '2.4GHz', channels: [] },
				[5]: { title: '5GHz', channels: [] },
				[6]: { title: '6GHz', channels: [] }
			};

			/* Split FrequencyList in Bands */
			wifiDevs[ifname].freq.forEach(function(freq) {
				if (bands[freq.band] && bands[freq.band].channels.indexOf(freq.channel) === -1)
					bands[freq.band].channels.push(freq.channel);
			});

			for (var band in bands) {
				if (bands[band].channels.length == 0)
					continue;

				bands[band].channels.sort(function(a, b) { return a - b; });

				var tabId = ifname + band,
				    csvg = svg.cloneNode(true),
				    table = E('table', { 'class': 'table' }, [
					E('tr', { 'class': 'tr table-titles' }, [
						E('th', { 'class': 'th col-2 middle center' }, _('Signal')),
						E('th', { 'class': 'th col-4 middle left' }, _('SSID')),
						E('th', { 'class': 'th col-2 middle center hide-xs' }, _('Channel')),
						E('th', { 'class': 'th col-3 middle left' }, _('Channel Width')),
						E('th', { 'class': 'th col-2 middle left hide-xs' }, _('Mode')),
						E('th', { 'class': 'th col-3 middle left hide-xs' }, _('BSSID'))
					])
				]),
				tab = E('div', { 'data-tab': tabId, 'data-tab-title': ifname + ' (' + bands[band].title + ')' },
					[ E('br'), csvg, E('br'), table, E('br') ]),
				graph_data = {
					graph: csvg,
					offset_tbl: {},
					col_width: 0,
					tab: tab,
					freq_tbl: bands[band].channels,
					band: band,
					builtWidth: 0
				};

				this.radios[tabId] = {
					dev: wifiDevs[ifname].dev,
					band: band,
					graph: graph_data,
					table: table,
					scanCache: {},
					loadedOnce: false,
					scanning: false,
					lastData: null
				};

				tab.addEventListener('cbi-tab-active', L.bind(function(id, ev) {
					var activeId = (ev && ev.detail && ev.detail.tab) ? ev.detail.tab : id;
					var r = this.radios[activeId];
					this.active_tab = activeId;
					if (!r)
						return;

					requestAnimationFrame(L.bind(function() {
						this.ensure_channel_graph(r);
						if (r.lastData)
							this.renderScanData(r, r.lastData);
					}, this));

					if (!r.loadedOnce && !r.scanning)
						this.handleScanRefresh(r);
				}, this, tabId));

				cbi_update_table(table, [], E('em', { class: 'spinning' }, _('Starting wireless scan...')));

				tabs.firstElementChild.appendChild(tab);
			}
		}

		if (tabs.firstElementChild.childNodes.length === 0) {
			tabs.firstElementChild.appendChild(E('div', { 'class': 'cbi-section' }, E('em', _('No entries available'))));
		} else {
			ui.tabs.initTabGroup(tabs.firstElementChild.childNodes);
		}

		return E('div', {}, [ h2, tabs ]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
