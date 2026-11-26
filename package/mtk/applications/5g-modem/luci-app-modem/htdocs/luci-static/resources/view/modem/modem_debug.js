'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require poll';
'require ui';

var callGetATPort = rpc.declare({
	object: 'luci.modem',
	method: 'getATPort',
	params: ['translation'],
	expect: { '': {} }
});

var callGetModeInfo = rpc.declare({
	object: 'luci.modem',
	method: 'getModeInfo',
	params: ['port', 'manufacturer', 'platform', 'translation'],
	expect: { '': {} }
});

var callSetMode = rpc.declare({
	object: 'luci.modem',
	method: 'setMode',
	params: ['port', 'manufacturer', 'platform', 'mode'],
	expect: { '': {} }
});

var callGetNetworkPreferInfo = rpc.declare({
	object: 'luci.modem',
	method: 'getNetworkPreferInfo',
	params: ['port', 'manufacturer'],
	expect: { '': {} }
});

var callSetNetworkPrefer = rpc.declare({
	object: 'luci.modem',
	method: 'setNetworkPrefer',
	params: ['port', 'manufacturer', 'network_prefer'],
	expect: { '': {} }
});

var callSetBandPrefer = rpc.declare({
	object: 'luci.modem',
	method: 'setBandPrefer',
	params: ['port', 'manufacturer', 'prefer_config'],
	expect: { '': {} }
});

var callGetSelfTestInfo = rpc.declare({
	object: 'luci.modem',
	method: 'getSelfTestInfo',
	params: ['port', 'manufacturer'],
	expect: { '': {} }
});

var callGetQuickCommands = rpc.declare({
	object: 'luci.modem',
	method: 'getQuickCommands',
	params: [],
	expect: { '': {} }
});

var callSendATCommand = rpc.declare({
	object: 'luci.modem',
	method: 'sendATCommand',
	params: ['port', 'command'],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('modem'),
			uci.load('custom_at_commands'),
			callGetATPort('1').catch(function() { return {}; }),
			callGetQuickCommands().catch(function() { return {}; })
		]);
	},

	getModemMetadataByPort: function(atPort) {
		var sections = uci.sections('modem', 'modem-device') || [];
		for (var i = 0; i < sections.length; i++) {
			if (sections[i].at_port === atPort) {
				return {
					manufacturer: sections[i].manufacturer || 'quectel',
					platform: sections[i].platform || 'qualcomm',
					modes: sections[i].modes || ['qmi', 'ecm', 'mbim', 'rndis', 'ncm']
				};
			}
		}
		return {
			manufacturer: 'quectel',
			platform: 'qualcomm',
			modes: ['qmi', 'ecm', 'mbim', 'rndis', 'ncm']
		};
	},

	populateATPortSelect: function(selectEl, atPorts, translation) {
		var selected = selectEl.value;
		while (selectEl.options.length > 0)
			selectEl.remove(0);

		if (Array.isArray(atPorts)) {
			for (var i = 0; i < atPorts.length; i++) {
				var entry = atPorts[i];
				for (var port in entry) {
					if (!Object.prototype.hasOwnProperty.call(entry, port))
						continue;
					var label = entry[port] || port;
					if (label.indexOf('unknown') !== -1)
						label = label.replace('unknown', port);
					else if (translation && translation[label])
						label = _(translation[label]);
					selectEl.appendChild(E('option', { 'value': port }, label));
				}
			}
		}

		for (var j = 0; j < selectEl.options.length; j++) {
			if (selectEl.options[j].value === selected) {
				selectEl.selectedIndex = j;
				break;
			}
		}
	},

	populateQuickCommands: function(selectEl, quickOption, rpcQuickCommands) {
		var selected = selectEl.value;
		while (selectEl.options.length > 0)
			selectEl.remove(0);

		if (quickOption === 'custom') {
			var customList = uci.sections('custom_at_commands', 'custom-commands') || [];
			for (var i = 0; i < customList.length; i++) {
				var c = customList[i];
				if (c.description && c.command)
					selectEl.appendChild(E('option', { 'value': c.command }, c.description));
			}
		} else {
			var list = (rpcQuickCommands && rpcQuickCommands.quick_commands) ? rpcQuickCommands.quick_commands : [];
			if (Array.isArray(list) && list.length > 0) {
				for (var j = 0; j < list.length; j++) {
					var item = list[j];
					for (var desc in item) {
						if (Object.prototype.hasOwnProperty.call(item, desc))
							selectEl.appendChild(E('option', { 'value': item[desc] }, desc));
					}
				}
			} else {
				var fallback = uci.sections('custom_at_commands', 'custom-commands') || [];
				for (var k = 0; k < fallback.length; k++) {
					if (fallback[k].description && fallback[k].command)
						selectEl.appendChild(E('option', { 'value': fallback[k].command }, fallback[k].description));
				}
			}
		}

		for (var m = 0; m < selectEl.options.length; m++) {
			if (selectEl.options[m].value === selected) {
				selectEl.selectedIndex = m;
				break;
			}
		}
		var atInput = document.getElementById('at_command');
		if (atInput && selectEl.value)
			atInput.value = selectEl.value;
	},

	updateModeTab: function(modeData, fallbackModes) {
		var info = (modeData && modeData.mode_info) ? modeData.mode_info : (modeData || {});
		var currentMode = info.mode || 'qmi';
		var modes = Array.isArray(info.modes) ? info.modes : (fallbackModes || ['qmi', 'ecm', 'mbim', 'rndis', 'ncm']);

		var curEl = document.getElementById('current_mode');
		if (curEl && currentMode !== 'unknown')
			curEl.textContent = String(currentMode).toUpperCase();

		var modeContainer = document.getElementById('mode_option');
		if (modeContainer && this.firstCache) {
			while (modeContainer.firstChild)
				modeContainer.removeChild(modeContainer.firstChild);

			var normalizedCurrent = (currentMode === 'gobinet') ? 'qmi' : currentMode;
			var hasGobinet = modes.indexOf('gobinet') !== -1;

			modes.forEach(function(m) {
				if (m === 'gobinet')
					return;
				var label = (m === 'qmi' && hasGobinet) ? 'QMI/GobiNet' : m.toUpperCase();
				var radio = E('input', {
					'type': 'radio',
					'name': 'mode_option',
					'id': 'mode_option_' + m,
					'value': m
				});
				if (m === normalizedCurrent)
					radio.checked = true;
				modeContainer.appendChild(E('label', { 'class': 'cbi-radio', 'style': 'margin-right:1rem;' }, [
					radio, ' ', E('span', {}, label)
				]));
			});
		}
	},

	updateNetworkPreferTab: function(preferData) {
		var self = this;
		var info = (preferData && preferData.network_prefer_info) ? preferData.network_prefer_info : (preferData || {});
		var preferList = info.network_prefer || [
			{ '3G': { enable: 1, band: [] } },
			{ '4G': { enable: 1, band: [] } },
			{ '5G': { enable: 1, band: [] } }
		];

		var currentView = [];
		var checkContainer = document.getElementById('network_type_check');
		if (checkContainer && this.firstCache) {
			while (checkContainer.firstChild)
				checkContainer.removeChild(checkContainer.firstChild);
		}

		var enabledCount = 0;
		var totalCount = 0;
		var hasAnyBand = false;

		if (Array.isArray(preferList)) {
			for (var i = 0; i < preferList.length; i++) {
				var entry = preferList[i];
				for (var netKey in entry) {
					if (!Object.prototype.hasOwnProperty.call(entry, netKey))
						continue;
					totalCount++;
					var cfg = entry[netKey] || {};
					var enabled = Boolean(cfg.enable);
					if (enabled) {
						currentView.push(netKey);
						enabledCount++;
					}
					var ratLc = netKey.toLowerCase();
					if (checkContainer && this.firstCache) {
						var cb = E('input', {
							'type': 'checkbox',
							'id': 'prefer_config_' + ratLc,
							'class': 'cbi-input-checkbox',
							'value': netKey
						});
						if (enabled)
							cb.checked = true;
						checkContainer.appendChild(E('label', { 'class': 'cbi-checkbox', 'style': 'margin-right:1rem;' }, [
							cb, ' ', E('span', {}, netKey)
						]));
					}

					// Band preferences per RAT
					var bands = Array.isArray(cfg.band) ? cfg.band : [];
					var bandCheckContainer = document.getElementById(ratLc + '_band_check');
					var currentBandView = [];
					var bandEnabledCount = 0;
					var bandTotalCount = 0;

					if (bandCheckContainer && this.firstCache) {
						while (bandCheckContainer.firstChild)
							bandCheckContainer.removeChild(bandCheckContainer.firstChild);
					}

					for (var b = 0; b < bands.length; b++) {
						var bObj = bands[b];
						if (!bObj || typeof bObj !== 'object')
							continue;
						for (var bandName in bObj) {
							if (!Object.prototype.hasOwnProperty.call(bObj, bandName))
								continue;
							bandTotalCount++;
							var bEnabled = Boolean(bObj[bandName]);
							if (bEnabled) {
								currentBandView.push(bandName);
								bandEnabledCount++;
							}
							if (bandCheckContainer && this.firstCache) {
								var bCb = E('input', {
									'type': 'checkbox',
									'id': 'band_prefer_config_' + bandName.toLowerCase(),
									'class': 'cbi-input-checkbox',
									'value': bandName,
									'data-band': bandName
								});
								if (bEnabled)
									bCb.checked = true;
								bandCheckContainer.appendChild(E('label', { 'class': 'cbi-checkbox', 'style': 'margin-right:0.75rem; display:inline-block;' }, [
									bCb, ' ', E('span', {}, bandName)
								]));
							}
						}
					}

					var curBandEl = document.getElementById(ratLc + '_current_prefer');
					if (curBandEl) {
						curBandEl.textContent = (currentBandView.length > 0) ? currentBandView.join(' ') : 'NONE';
					}

					if (this.firstCache) {
						var autoRadio = document.getElementById(ratLc + '_prefer_option_auto');
						var customRadio = document.getElementById(ratLc + '_prefer_option_custom');
						if (autoRadio && customRadio) {
							if (bandTotalCount > 0 && bandEnabledCount === bandTotalCount) {
								autoRadio.checked = true;
								if (bandCheckContainer) {
									bandCheckContainer.querySelectorAll('input[type="checkbox"]').forEach(function(c) {
										c.disabled = true;
									});
								}
							} else {
								customRadio.checked = true;
								if (bandCheckContainer) {
									bandCheckContainer.querySelectorAll('input[type="checkbox"]').forEach(function(c) {
										c.disabled = false;
									});
								}
							}
						}
					}

					var ratSection = document.getElementById('cbi-' + ratLc + '_band_info');
					if (ratSection) {
						if (self.activeTab === 'network_prefer_tab' && enabled && bands.length > 0) {
							ratSection.style.display = 'block';
							hasAnyBand = true;
						} else {
							ratSection.style.display = 'none';
						}
					}
				}
			}
		}

		var curPrefEl = document.getElementById('current_prefer');
		if (curPrefEl) {
			curPrefEl.textContent = (totalCount > 0 && enabledCount === totalCount)
				? 'AUTO'
				: (currentView.join(' ') || 'NONE');
		}

		var bandBtn = document.getElementById('band_prefer_button');
		if (bandBtn) {
			bandBtn.style.display = (self.activeTab === 'network_prefer_tab' && hasAnyBand) ? '' : 'none';
		}
	},

	updateSelfTestTab: function(testData) {
		var info = (testData && testData.self_test_info) ? testData.self_test_info : (testData || {});
		var voltage = parseFloat(info.voltage);
		var temperature = parseFloat(info.temperature);

		var curVoltEl = document.getElementById('current_voltage');
		var voltStatusEl = document.getElementById('voltage_status');
		if (curVoltEl && !isNaN(voltage))
			curVoltEl.textContent = (voltage / 1000).toFixed(2) + ' V';

		if (voltStatusEl) {
			var vState = _('Unknown');
			var vColor = 'goldenrod';
			if (!isNaN(voltage)) {
				if (voltage < 3135 || voltage >= 4400) {
					vState = _('Abnormal');
					vColor = 'red';
				} else if (voltage < 3700) {
					vState = _('Low');
					vColor = 'goldenrod';
				} else {
					vState = _('Normal');
					vColor = 'green';
				}
			}
			voltStatusEl.textContent = vState;
			voltStatusEl.style.color = vColor;
		}

		var curTempEl = document.getElementById('current_temperature');
		var tempStatusEl = document.getElementById('temperature_status');
		if (curTempEl && !isNaN(temperature))
			curTempEl.textContent = temperature + ' °C';

		if (tempStatusEl) {
			var tState = _('Unknown');
			var tColor = 'goldenrod';
			if (!isNaN(temperature)) {
				if (temperature < 45) {
					tState = _('Normal');
					tColor = 'green';
				} else if (temperature < 75) {
					tState = _('Somewhat High');
					tColor = 'goldenrod';
				} else if (temperature < 105) {
					tState = _('Excessively High');
					tColor = 'red';
				} else {
					tState = _('Abnormal');
					tColor = 'red';
				}
			}
			tempStatusEl.textContent = tState;
			tempStatusEl.style.color = tColor;
		}
	},

	refreshDebugTab: function(atPort) {
		if (!atPort)
			return Promise.resolve();
		var self = this;
		var meta = this.getModemMetadataByPort(atPort);
		var activeTab = this.activeTab || 'mode_tab';

		if (activeTab === 'mode_tab') {
			return callGetModeInfo(atPort, meta.manufacturer, meta.platform, '1').then(function(res) {
				self.updateModeTab(res, meta.modes);
				self.firstCache = false;
			}).catch(function() {});
		} else if (activeTab === 'network_prefer_tab') {
			return callGetNetworkPreferInfo(atPort, meta.manufacturer).then(function(res) {
				self.updateNetworkPreferTab(res);
				self.firstCache = false;
			}).catch(function() {});
		} else if (activeTab === 'self_test_tab') {
			return callGetSelfTestInfo(atPort, meta.manufacturer).then(function(res) {
				self.updateSelfTestTab(res);
				self.firstCache = false;
			}).catch(function() {});
		}
		return Promise.resolve();
	},

	switchTab: function(tabId) {
		this.activeTab = tabId;
		this.firstCache = true;
		var tabs = ['mode_tab', 'network_prefer_tab', 'self_test_tab', 'at_command_tab'];
		tabs.forEach(function(id) {
			var li = document.querySelector('li[data-tab="' + id + '"]');
			var pane = document.querySelector('div[data-tab="' + id + '"]');
			if (li)
				li.className = (id === tabId) ? 'cbi-tab' : 'cbi-tab-disabled';
			if (pane)
				pane.style.display = (id === tabId) ? 'block' : 'none';
		});
		if (tabId !== 'network_prefer_tab') {
			['5g', '4g', '3g', '2g'].forEach(function(rat) {
				var el = document.getElementById('cbi-' + rat + '_band_info');
				if (el)
					el.style.display = 'none';
			});
			var bandBtn = document.getElementById('band_prefer_button');
			if (bandBtn)
				bandBtn.style.display = 'none';
		}
		var selectEl = document.getElementById('modem_select');
		if (selectEl && selectEl.value)
			this.refreshDebugTab(selectEl.value);
	},

	render: function(loadResults) {
		var self = this;
		var initialAT = loadResults[2] || {};
		var initialQuick = loadResults[3] || {};
		this.activeTab = 'mode_tab';
		this.firstCache = true;

		var modemSelect = E('select', {
			'name': 'modem_select',
			'id': 'modem_select',
			'class': 'cbi-input-select',
			'change': function() {
				self.firstCache = true;
				self.refreshDebugTab(modemSelect.value);
			}
		});
		this.populateATPortSelect(modemSelect, initialAT.at_ports || [], initialAT.translation || {});

		var commandSelect = E('select', {
			'name': 'command_select',
			'id': 'command_select',
			'class': 'cbi-input-select',
			'change': function() {
				var input = document.getElementById('at_command');
				if (input)
					input.value = commandSelect.value;
			},
			'click': function() {
				var input = document.getElementById('at_command');
				if (input)
					input.value = commandSelect.value;
			}
		});
		this.populateQuickCommands(commandSelect, 'auto', initialQuick);

		var root = E('div', { 'class': 'cbi-map', 'id': 'cbi-modem-debug' }, [
			E('h2', { 'id': 'content', 'name': 'content' }, _('Modem Debug')),
			E('div', { 'class': 'cbi-map-descr' }, _('Debug Your Module')),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-info', 'style': 'display:none;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Message')),
					E('div', { 'id': 'info_message' }, _('Loading modem') + '...')
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-modem', 'style': 'display:block;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Modem Select')),
					E('div', { 'class': 'cbi-section-node' }, [
						E('div', { 'class': 'cbi-value' }, [
							E('label', { 'class': 'cbi-value-title' }, _('Modem Name')),
							E('div', { 'class': 'cbi-value-field' }, [
								modemSelect,
								E('div', { 'class': 'cbi-value-description' }, _('Select a modem for debugging'))
							])
						])
					])
				])
			]),

			E('ul', { 'class': 'cbi-tabmenu', 'id': 'tab_menu' }, [
				E('li', { 'class': 'cbi-tab', 'data-tab': 'mode_tab' }, [
					E('a', { 'href': '#', 'click': function(ev) { ev.preventDefault(); self.switchTab('mode_tab'); } }, _('Mode'))
				]),
				E('li', { 'class': 'cbi-tab-disabled', 'data-tab': 'network_prefer_tab' }, [
					E('a', { 'href': '#', 'click': function(ev) { ev.preventDefault(); self.switchTab('network_prefer_tab'); } }, _('Network Preferences'))
				]),
				E('li', { 'class': 'cbi-tab-disabled', 'data-tab': 'self_test_tab' }, [
					E('a', { 'href': '#', 'click': function(ev) { ev.preventDefault(); self.switchTab('self_test_tab'); } }, _('Self Test'))
				]),
				E('li', { 'class': 'cbi-tab-disabled', 'data-tab': 'at_command_tab' }, [
					E('a', { 'href': '#', 'click': function(ev) { ev.preventDefault(); self.switchTab('at_command_tab'); } }, _('AT Command'))
				])
			]),

			E('div', { 'class': 'cbi-section-node cbi-section-node-tabbed', 'id': 'tab_context' }, [
				E('div', { 'class': 'cbi-section', 'data-tab': 'mode_tab', 'style': 'display:block;' }, [
					E('table', { 'class': 'table cbi-section-table' }, [
						E('tbody', {}, [
							E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Current')),
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Config')),
								E('th', { 'class': 'th cbi-section-table-cell cbi-section-actions' }, '')
							]),
							E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-1' }, [
								E('td', { 'class': 'td cbi-value-field', 'id': 'current_mode' }, '-'),
								E('td', { 'class': 'td cbi-value-field', 'id': 'mode_option' }),
								E('td', { 'class': 'td' }, [
									E('button', {
										'class': 'btn cbi-button cbi-button-apply',
										'id': 'mode_button',
										'click': function(ev) {
											ev.preventDefault();
											var checked = document.querySelector('input[name="mode_option"]:checked');
											var port = modemSelect.value;
											if (!checked || !port)
												return;
											var meta = self.getModemMetadataByPort(port);
											callSetMode(port, meta.manufacturer, meta.platform, checked.value).then(function(res) {
												if (res && res.code && res.code !== 0) {
													ui.addNotification(null, E('p', {}, res.error || _('Failed to set mode')), 'danger');
												}
												return self.refreshDebugTab(port);
											}).catch(function(err) {
												ui.addNotification(null, E('p', {}, (err && err.message) || _('Failed to set mode')), 'danger');
											});
										}
									}, _('Apply'))
								])
							])
						])
					])
				]),

				E('div', { 'class': 'cbi-section', 'data-tab': 'network_prefer_tab', 'style': 'display:none;' }, [
					E('table', { 'class': 'table cbi-section-table' }, [
						E('tbody', {}, [
							E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Current')),
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Option')),
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Config')),
								E('th', { 'class': 'th cbi-section-table-cell cbi-section-actions' }, '')
							]),
							E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-1' }, [
								E('td', { 'class': 'td cbi-value-field', 'id': 'current_prefer' }, 'AUTO'),
								E('td', { 'class': 'td cbi-value-field', 'id': 'prefer_option' }, [
									E('label', { 'class': 'cbi-radio', 'style': 'margin-right:1rem;' }, [
										E('input', {
											'type': 'radio',
											'name': 'prefer_option',
											'id': 'prefer_option_auto',
											'value': 'auto',
											'checked': 'checked',
											'change': function() {
												var cbs = document.querySelectorAll('#network_type_check input[type="checkbox"]');
												cbs.forEach(function(cb) { cb.checked = true; cb.disabled = true; });
											}
										}),
										' ',
										E('span', {}, _('Auto'))
									]),
									E('label', { 'class': 'cbi-radio' }, [
										E('input', {
											'type': 'radio',
											'name': 'prefer_option',
											'id': 'prefer_option_custom',
											'value': 'custom',
											'change': function() {
												var cbs = document.querySelectorAll('#network_type_check input[type="checkbox"]');
												cbs.forEach(function(cb) { cb.disabled = false; });
											}
										}),
										' ',
										E('span', {}, _('Custom'))
									])
								]),
								E('td', { 'class': 'td cbi-value-field', 'id': 'prefer_custom_config' }, [
									E('div', { 'id': 'network_type_check' })
								]),
								E('td', { 'class': 'td' }, [
									E('button', {
										'class': 'btn cbi-button cbi-button-apply',
										'id': 'network_prefer_button',
										'click': function(ev) {
											ev.preventDefault();
											var port = modemSelect.value;
											if (!port)
												return;
											var meta = self.getModemMetadataByPort(port);
											var cbs = document.querySelectorAll('#network_type_check input[type="checkbox"]');
											var selectedNets = [];
											cbs.forEach(function(cb) {
												if (cb.checked)
													selectedNets.push(cb.value.toUpperCase());
											});
											var prefStr = selectedNets.join(':') || 'AUTO';
											callSetNetworkPrefer(port, meta.manufacturer, prefStr).then(function(res) {
												if (res && res.code && res.code !== 0) {
													ui.addNotification(null, E('p', {}, res.error || _('Failed to set network preference')), 'danger');
												}
												return self.refreshDebugTab(port);
											}).catch(function(err) {
												ui.addNotification(null, E('p', {}, (err && err.message) || _('Failed to set network preference')), 'danger');
											});
										}
									}, _('Apply')),
									' ',
									E('button', {
										'class': 'btn cbi-button cbi-button-apply',
										'id': 'band_prefer_button',
										'style': 'display:none; margin-left:0.5rem;',
										'click': function(ev) {
											ev.preventDefault();
											var port = modemSelect.value;
											if (!port)
												return;
											var meta = self.getModemMetadataByPort(port);
											var preferConfig = {};
											var ratCbs = document.querySelectorAll('#network_type_check input[type="checkbox"]');
											ratCbs.forEach(function(ratCb) {
												var ratKey = ratCb.value.toUpperCase();
												var ratLc = ratCb.value.toLowerCase();
												var ratObj = {
													enable: ratCb.checked ? 1 : 0,
													band: []
												};
												var bandCbs = document.querySelectorAll('#' + ratLc + '_band_check input[type="checkbox"]');
												bandCbs.forEach(function(bCb) {
													var bandKey = bCb.getAttribute('data-band') || bCb.value;
													var item = {};
													item[bandKey] = bCb.checked ? 1 : 0;
													ratObj.band.push(item);
												});
												preferConfig[ratKey] = ratObj;
											});

											callSetBandPrefer(port, meta.manufacturer, preferConfig).then(function(res) {
												if (res && res.code !== 0) {
													ui.addNotification(null, E('p', {}, res.error || _('Failed to apply band preferences')), 'danger');
												}
												return self.refreshDebugTab(port);
											}).catch(function(err) {
												ui.addNotification(null, E('p', {}, (err && err.message) || _('Failed to apply band preferences')), 'danger');
											});
										}
									}, _('Apply Band Prefer'))
								])
							])
						])
					]),
					(function() {
						function renderBandFieldset(rat, title) {
							var ratLc = rat.toLowerCase();
							return E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-' + ratLc + '_band_info', 'style': 'display:none; margin-top:1rem;' }, [
								E('div', { 'class': 'cbi-section fade-in' }, [
									E('h3', {}, title),
									E('table', { 'class': 'table cbi-section-table' }, [
										E('tbody', {}, [
											E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
												E('th', { 'class': 'th cbi-section-table-cell' }, _('Current')),
												E('th', { 'class': 'th cbi-section-table-cell' }, _('Option')),
												E('th', { 'class': 'th cbi-section-table-cell' }, _('Config')),
												E('th', { 'class': 'th cbi-section-table-cell cbi-section-actions' }, '')
											]),
											E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-1' }, [
												E('td', { 'class': 'td cbi-value-field', 'id': ratLc + '_current_prefer' }, 'NONE'),
												E('td', { 'class': 'td cbi-value-field', 'id': ratLc + '_prefer_option' }, [
													E('label', { 'class': 'cbi-radio', 'style': 'margin-right:1rem;' }, [
														E('input', {
															'type': 'radio',
															'name': ratLc + '_prefer_option',
															'id': ratLc + '_prefer_option_auto',
															'value': 'auto',
															'checked': 'checked',
															'change': function() {
																var cbs = document.querySelectorAll('#' + ratLc + '_band_check input[type="checkbox"]');
																cbs.forEach(function(cb) { cb.checked = true; cb.disabled = true; });
															}
														}),
														' ',
														E('span', {}, _('Auto'))
													]),
													E('label', { 'class': 'cbi-radio' }, [
														E('input', {
															'type': 'radio',
															'name': ratLc + '_prefer_option',
															'id': ratLc + '_prefer_option_custom',
															'value': 'custom',
															'change': function() {
																var cbs = document.querySelectorAll('#' + ratLc + '_band_check input[type="checkbox"]');
																cbs.forEach(function(cb) { cb.disabled = false; });
															}
														}),
														' ',
														E('span', {}, _('Custom'))
													])
												]),
												E('td', { 'class': 'td cbi-value-field', 'id': ratLc + '_prefer_custom_config' }, [
													E('div', { 'id': ratLc + '_band_check' })
												]),
												E('td', { 'class': 'td' }, [])
											])
										])
									])
								])
							]);
						}
						return [
							renderBandFieldset('5G', _('5G Band Preferences')),
							renderBandFieldset('4G', _('4G Band Preferences')),
							renderBandFieldset('3G', _('3G Band Preferences')),
							renderBandFieldset('2G', _('2G Band Preferences'))
						];
					})()
				]),

				E('div', { 'class': 'cbi-section', 'data-tab': 'self_test_tab', 'style': 'display:none;' }, [
					E('table', { 'class': 'table cbi-section-table' }, [
						E('tbody', {}, [
							E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Item')),
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Current')),
								E('th', { 'class': 'th cbi-section-table-cell' }, _('Status'))
							]),
							E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-1' }, [
								E('td', { 'class': 'td cbi-value-field', 'id': 'voltage_label' }, _('Voltage')),
								E('td', { 'class': 'td cbi-value-field', 'id': 'current_voltage' }, '-'),
								E('td', { 'class': 'td cbi-value-field', 'id': 'voltage_status' }, '-')
							]),
							E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-2' }, [
								E('td', { 'class': 'td cbi-value-field', 'id': 'temperature_label' }, _('Temperature')),
								E('td', { 'class': 'td cbi-value-field', 'id': 'current_temperature' }, '-'),
								E('td', { 'class': 'td cbi-value-field', 'id': 'temperature_status' }, '-')
							])
						])
					])
				]),

				E('div', { 'class': 'cbi-section', 'data-tab': 'at_command_tab', 'style': 'display:none;' }, [
					E('h3', { 'id': 'at_command_title' }, _('AT Command')),
					E('table', { 'class': 'table', 'id': 'at_command_info' }, [
						E('tbody', {}, [
							E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left', 'style': 'width:33%;' }, _('Quick Option')),
								E('td', { 'class': 'td left', 'id': 'quick_option_td' }, [
									E('label', { 'class': 'cbi-radio', 'style': 'margin-right:1rem;' }, [
										E('input', {
											'type': 'radio',
											'name': 'quick_option',
											'value': 'auto',
											'checked': 'checked',
											'change': function() {
												self.populateQuickCommands(commandSelect, 'auto', initialQuick);
											}
										}),
										' ',
										E('span', {}, _('Auto'))
									]),
									E('label', { 'class': 'cbi-radio' }, [
										E('input', {
											'type': 'radio',
											'name': 'quick_option',
											'value': 'custom',
											'change': function() {
												self.populateQuickCommands(commandSelect, 'custom', initialQuick);
											}
										}),
										' ',
										E('span', {}, _('Custom'))
									])
								])
							]),
							E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left' }, _('Quick Commands')),
								E('td', { 'class': 'td left' }, [commandSelect])
							]),
							E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left' }, _('Enter Command')),
								E('td', { 'class': 'td left' }, [
									E('div', { 'style': 'margin-bottom:0.5rem;' }, [
										E('input', { 'type': 'text', 'id': 'at_command', 'class': 'cbi-input-text' })
									]),
									E('div', {}, [
										E('button', {
											'class': 'btn cbi-button cbi-button-apply',
											'click': function(ev) {
												ev.preventDefault();
												var port = modemSelect.value;
												var cmdEl = document.getElementById('at_command');
												var cmd = cmdEl ? cmdEl.value : '';
												if (!port || !cmd)
													return;
												callSendATCommand(port, cmd).then(function(res) {
													var respEl = document.getElementById('response');
													if (respEl && res) {
														var timeStr = res.time ? (res.time + ' ') : '';
														if (res.code && res.code !== 0) {
															ui.addNotification(null, E('p', {}, res.error || _('AT command execution failed')), 'danger');
															respEl.value += timeStr + 'ERROR: ' + (res.error || '') + ' ' + (res.response || '') + '\n';
														} else {
															respEl.value += timeStr + (res.response || '') + '\n';
														}
														respEl.scrollTop = respEl.scrollHeight;
													}
												}).catch(function(err) {
													ui.addNotification(null, E('p', {}, (err && err.message) || _('AT command execution failed')), 'danger');
												});
											}
										}, _('Send')),
										' ',
										E('button', {
											'class': 'btn cbi-button cbi-button-reset',
											'click': function(ev) {
												ev.preventDefault();
												var cmdEl = document.getElementById('at_command');
												if (cmdEl)
													cmdEl.value = '';
											}
										}, _('Clean'))
									])
								])
							]),
							E('tr', { 'class': 'tr' }, [
								E('td', { 'colspan': '2', 'class': 'td left' }, [
									E('div', { 'id': 'response_label' }, _('Response')),
									E('textarea', {
										'readonly': 'readonly',
										'id': 'response',
										'rows': '16',
										'style': 'background:#373737;border:none;color:#FFF;width:100%;padding:10px;font-family:monospace;'
									}),
									E('div', { 'class': 'cbi-page-actions' }, [
										E('a', {
											'class': 'btn cbi-button cbi-button-link',
											'href': L.url('admin', 'modem', 'quick_commands')
										}, _('Custom quick commands')),
										' ',
										E('button', {
											'class': 'btn cbi-button cbi-button-reset',
											'click': function(ev) {
												ev.preventDefault();
												var respEl = document.getElementById('response');
												if (respEl)
													respEl.value = '';
											}
										}, _('Clean'))
									])
								])
							])
						])
					])
				])
			])
		]);

		poll.add(function() {
			return callGetATPort('1').then(function(atData) {
				var ports = (atData && atData.at_ports) ? atData.at_ports : [];
				self.populateATPortSelect(modemSelect, ports, (atData && atData.translation) || {});
				if (modemSelect.value)
					return self.refreshDebugTab(modemSelect.value);
			});
		}, 5);

		return root;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
