'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require poll';
'require ui';

var callGetStatus = rpc.declare({
	object: 'luci.modem',
	method: 'getStatus',
	params: [],
	expect: { '': {} }
});

var callGetModems = rpc.declare({
	object: 'luci.modem',
	method: 'getModems',
	params: ['translation'],
	expect: { '': {} }
});

var callGetDialLogInfo = rpc.declare({
	object: 'luci.modem',
	method: 'getDialLogInfo',
	params: [],
	expect: { '': {} }
});

var callCleanDialLog = rpc.declare({
	object: 'luci.modem',
	method: 'cleanDialLog',
	params: [],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('modem'),
			callGetStatus().catch(function() { return {}; }),
			callGetModems('1').catch(function() { return {}; }),
			callGetDialLogInfo().catch(function() { return {}; })
		]);
	},

	renderModemStatusCards: function(container, modemsData) {
		if (!container)
			return;
		var modems = (modemsData && modemsData.modems) ? modemsData.modems : [];
		var translation = (modemsData && modemsData.translation) ? modemsData.translation : {};
		var field = document.getElementById('modem_status_field');

		while (container.firstChild)
			container.removeChild(container.firstChild);

		var list = [];
		if (Array.isArray(modems)) {
			for (var i = 0; i < modems.length; i++) {
				var entry = modems[i];
				if (entry && entry.name) {
					list.push(entry);
				} else if (entry && typeof entry === 'object') {
					for (var k in entry) {
						if (Object.prototype.hasOwnProperty.call(entry, k) && entry[k])
							list.push(entry[k]);
					}
				}
			}
		}

		if (list.length === 0) {
			container.appendChild(E('strong', {}, _('No modems found')));
			if (field)
				field.style.display = 'none';
			return;
		}

		for (var idx = 0; idx < list.length; idx++) {
			var m = list[idx];
			if (!m.name || !m.mode)
				continue;
			var nameStr = (m.name === 'unknown') ? _(translation[m.name] || 'Unknown') : String(m.name).toUpperCase();
			var modeStr = (m.mode === 'unknown') ? _(translation[m.mode] || 'Unknown') : String(m.mode).toUpperCase();
			var rawStatus = m.connect_status || 'unknown';
			var statusStr = _(translation[rawStatus] || rawStatus);
			var cssClass = 'warning';
			if (rawStatus === 'connect' || rawStatus === 'connected')
				cssClass = 'success';
			else if (rawStatus === 'disconnect' || rawStatus === 'disconnected')
				cssClass = 'danger';

			var secName = m['.name'] || ('modem' + idx);
			var modemNo = secName.replace('modem', '');

			container.appendChild(E('div', {
				'class': 'alert-message ' + cssClass,
				'style': 'display:inline-block;margin:0.75rem;padding:1rem;width:16rem;vertical-align:top;line-height:140%;'
			}, [
				E('div', {}, [E('strong', {}, 'No: '), String(modemNo)]),
				E('div', {}, [E('strong', {}, _('Modem Name') + ': '), nameStr]),
				E('div', {}, [E('strong', {}, _('Manufacturer') + ': '), String(m.manufacturer || '-').toUpperCase()]),
				E('div', {}, [E('strong', {}, _('Data Interface') + ': '), String(m.data_interface || '-').toUpperCase()]),
				E('div', {}, [E('strong', {}, _('Mode') + ': '), modeStr]),
				E('div', {}, [E('strong', {}, _('Mobile Network') + ': '), String(m.network || '-')]),
				E('div', {}, [E('strong', {}, _('Connect Status') + ': '), statusStr])
			]));
		}

		if (field)
			field.style.display = 'block';
	},

	renderDialLogs: function(logData) {
		var tabMenu = document.getElementById('dial_log_tab_menu');
		var logView = document.getElementById('dial_log_view');
		var logField = document.getElementById('modem_dial_log_field');
		if (!tabMenu || !logView || !logField)
			return;

		var dialLogs = (logData && logData.dial_log_info) ? logData.dial_log_info : (logData && logData.logs ? logData.logs : []);
		var modemNames = (logData && logData.modem_name_info) ? logData.modem_name_info : {};
		var translation = (logData && logData.translation) ? logData.translation : {};

		var entries = [];
		if (Array.isArray(dialLogs)) {
			for (var i = 0; i < dialLogs.length; i++) {
				var item = dialLogs[i];
				if (item && typeof item === 'object') {
					for (var k in item) {
						if (Object.prototype.hasOwnProperty.call(item, k))
							entries.push({ key: k, text: item[k] });
					}
				}
			}
		} else if (typeof logData === 'string' && logData.length > 0) {
			entries.push({ key: 'modem0', text: logData });
		}

		if (entries.length === 0) {
			logField.style.display = 'none';
			return;
		}

		var activeKey = this.activeLogTab || entries[0].key;
		var foundActive = false;
		for (var j = 0; j < entries.length; j++) {
			if (entries[j].key === activeKey) {
				foundActive = true;
				break;
			}
		}
		if (!foundActive)
			activeKey = entries[0].key;
		this.activeLogTab = activeKey;

		while (tabMenu.firstChild)
			tabMenu.removeChild(tabMenu.firstChild);
		while (logView.firstChild)
			logView.removeChild(logView.firstChild);

		var self = this;
		entries.forEach(function(entry) {
			var isAct = (entry.key === self.activeLogTab);
			var rawModemName = modemNames[entry.key] || entry.key;
			var displayName = (rawModemName === 'unknown')
				? _(translation[rawModemName] || 'Unknown')
				: (translation[rawModemName] ? _(translation[rawModemName]) : String(rawModemName)).toUpperCase();
			var mTitle = displayName + ' (' + entry.key.replace('modem', '') + ')';

			var li = E('li', {
				'class': isAct ? 'cbi-tab' : 'cbi-tab-disabled',
				'data-tab': entry.key + '_tab'
			}, [
				E('a', {
					'href': '#',
					'click': function(ev) {
						ev.preventDefault();
						self.activeLogTab = entry.key;
						self.renderDialLogs(logData);
					}
				}, mTitle)
			]);
			tabMenu.appendChild(li);

			var pane = E('div', {
				'class': 'cbi-section',
				'data-tab': entry.key + '_tab',
				'style': 'display:' + (isAct ? 'block' : 'none') + ';'
			}, [
				E('textarea', {
					'readonly': 'readonly',
					'id': entry.key + '_log',
					'rows': '16',
					'style': 'background:#373737;border:none;color:#FFF;width:100%;padding:10px;font-family:monospace;'
				}, entry.text || '')
			]);
			logView.appendChild(pane);
		});

		logField.style.display = 'block';
	},

	render: function(loadResults) {
		var self = this;
		var statusData = loadResults[1] || {};
		var modemsData = loadResults[2] || {};
		var dialLogData = loadResults[3] || {};

		var m = new form.Map('modem', _('Dial Overview'), _('Check and add modem dialing configurations'));

		var sGlobal = m.section(form.NamedSection, 'global', 'global', _('Global Config'));
		sGlobal.anonymous = true;
		sGlobal.addremove = false;

		var oEnableDial = sGlobal.option(form.Flag, 'enable_dial', _('Enable'), _('Enable dial configurations'));
		oEnableDial.rmempty = false;
		oEnableDial.default = '1';

		var oServiceStatus = sGlobal.option(form.DummyValue, '_service_status', _('Status'));
		oServiceStatus.rawhtml = true;
		oServiceStatus.cfgvalue = function() {
			var running = (statusData && (statusData.running || statusData.status));
			return running
				? '<span style="color:green;font-weight:bold;">' + _('connect') + '</span>'
				: '<span style="color:goldenrod;font-weight:bold;">' + _('Collecting data...') + '</span>';
		};

		var sGrid = m.section(form.GridSection, 'dial-config', _('Config List'));
		sGrid.anonymous = true;
		sGrid.addremove = true;
		sGrid.sortable = true;
		sGrid.modaltitle = _('Dial Config');

		sGrid.tab('general', _('General Settings'));
		sGrid.tab('advanced', _('Advanced Settings'));

		var oEnable = sGrid.taboption('general', form.Flag, 'enable', _('Enable'));
		oEnable.default = '0';
		oEnable.rmempty = false;
		oEnable.editable = true;

		var oRemarks = sGrid.taboption('general', form.Value, 'remarks', _('Remarks'));
		oRemarks.rmempty = true;

		var oNetwork = sGrid.taboption('general', form.ListValue, 'network', _('Mobile Network'));
		oNetwork.rmempty = false;
		var modemDevices = uci.sections('modem', 'modem-device');
		if (!modemDevices || modemDevices.length === 0) {
			oNetwork.value('', _('Mobile network not found'));
		} else {
			for (var i = 0; i < modemDevices.length; i++) {
				var md = modemDevices[i];
				if (md.network) {
					var mName = md.name ? _(String(md.name).toUpperCase()) : _('Unknown');
					oNetwork.value(md.network, md.network + ' (' + mName + ')');
				}
			}
		}

		var oModemNetwork = sGrid.taboption('general', form.Value, 'modem_network', _('Network'));
		oModemNetwork.modalonly = true;
		oModemNetwork.rmempty = true;

		var oMode = sGrid.taboption('general', form.ListValue, 'mode', _('Mode'), _('Only display the modes available for the adaptation modem'));
		oMode.modalonly = true;
		oMode.rmempty = true;
		oMode.value('', _('Auto Choose'));
		['qmi', 'gobinet', 'ecm', 'mbim', 'rndis', 'ncm'].forEach(function(modeName) {
			oMode.value(modeName, modeName.toUpperCase());
		});

		var oDialTool = sGrid.taboption('advanced', form.ListValue, 'dial_tool', _('Dial Tool'),
			_('After switching the dialing tool, it may be necessary to restart the module or restart the router to recognize the module.'));
		oDialTool.rmempty = true;
		oDialTool.value('', _('Auto Choose'));
		oDialTool.value('quectel-CM', _('quectel-CM'));
		oDialTool.value('mmcli', _('mmcli'));
		oDialTool.value('fibocom-dial', 'fibocom-dial');
		oDialTool.value('meig-cm', 'meig-cm');
		oDialTool.textvalue = function(section_id) {
			var val = uci.get('modem', section_id, 'dial_tool') || '';
			return val === '' ? _('Auto Choose') : _(val);
		};

		var oPdpType = sGrid.taboption('advanced', form.ListValue, 'pdp_type', _('PDP Type'));
		oPdpType.default = 'ipv4v6';
		oPdpType.rmempty = false;
		oPdpType.value('ipv4', _('IPv4'));
		oPdpType.value('ipv6', _('IPv6'));
		oPdpType.value('ipv4v6', _('IPv4/IPv6'));
		oPdpType.textvalue = function(section_id) {
			var v = uci.get('modem', section_id, 'pdp_type') || 'ipv4v6';
			if (v === 'ipv4v6')
				return _('IPv4/IPv6');
			return v.replace('_', '/').toUpperCase().replace('V', 'v');
		};

		var oBridge = sGrid.taboption('advanced', form.Flag, 'network_bridge', _('Network Bridge'),
			_('After checking, enable network interface bridge.'));
		oBridge.default = '0';
		oBridge.rmempty = false;
		oBridge.editable = true;

		var oApn = sGrid.taboption('advanced', form.Value, 'apn', _('APN'));
		oApn.default = '';
		oApn.rmempty = true;
		oApn.value('', _('Auto Choose'));
		oApn.value('cmnet', _('China Mobile'));
		oApn.value('3gnet', _('China Unicom'));
		oApn.value('ctnet', _('China Telecom'));
		oApn.value('cbnet', _('China Broadcast'));
		oApn.value('5gscuiot', _('Skytone'));
		oApn.textvalue = function(section_id) {
			var v = uci.get('modem', section_id, 'apn') || '';
			return v === '' ? _('Auto Choose') : v;
		};

		var oAuth = sGrid.taboption('advanced', form.ListValue, 'auth', _('Authentication Type'));
		oAuth.default = 'none';
		oAuth.rmempty = false;
		oAuth.modalonly = true;
		oAuth.value('none', _('NONE'));
		oAuth.value('both', _('PAP/CHAP (both)'));
		oAuth.value('pap', 'PAP');
		oAuth.value('chap', 'CHAP');

		var oUsername = sGrid.taboption('advanced', form.Value, 'username', _('PAP/CHAP Username'));
		oUsername.rmempty = true;
		oUsername.modalonly = true;
		oUsername.depends('auth', 'both');
		oUsername.depends('auth', 'pap');
		oUsername.depends('auth', 'chap');

		var oPassword = sGrid.taboption('advanced', form.Value, 'password', _('PAP/CHAP Password'));
		oPassword.rmempty = true;
		oPassword.password = true;
		oPassword.modalonly = true;
		oPassword.depends('auth', 'both');
		oPassword.depends('auth', 'pap');
		oPassword.depends('auth', 'chap');

		var oMetric = sGrid.taboption('advanced', form.Value, 'metric', _('Metric'));
		oMetric.datatype = 'uinteger';
		oMetric.rmempty = true;
		oMetric.modalonly = true;

		var oMtu = sGrid.taboption('advanced', form.Value, 'mtu', _('MTU'));
		oMtu.datatype = 'uinteger';
		oMtu.rmempty = true;
		oMtu.modalonly = true;

		var oDns = sGrid.taboption('advanced', form.DynamicList, 'dns', _('DNS'));
		oDns.datatype = 'ipaddr';
		oDns.rmempty = true;
		oDns.modalonly = true;

		var oIpv6 = sGrid.taboption('advanced', form.Flag, 'ipv6', _('IPv6'));
		oIpv6.rmempty = true;
		oIpv6.modalonly = true;

		var oNat = sGrid.taboption('advanced', form.Flag, 'nat', _('NAT'));
		oNat.rmempty = true;
		oNat.modalonly = true;

		var oRa = sGrid.taboption('advanced', form.Flag, 'ra', _('Router Advertisement'));
		oRa.rmempty = true;
		oRa.modalonly = true;

		var oExtendPrefix = sGrid.taboption('advanced', form.Flag, 'extend_prefix', _('Extend Prefix'));
		oExtendPrefix.rmempty = true;
		oExtendPrefix.modalonly = true;

		var oSmode = sGrid.taboption('advanced', form.ListValue, 'smode', _('Network Mode'));
		oSmode.rmempty = true;
		oSmode.modalonly = true;
		oSmode.value('', _('Auto'));
		oSmode.value('LTE', _('LTE Mode'));
		oSmode.value('NR5G-NSA', _('EN-DC Mode'));
		oSmode.value('NR5G-SA', _('NR5G-SA Mode'));

		var oBand = sGrid.taboption('advanced', form.Value, 'band', _('Band'));
		oBand.rmempty = true;
		oBand.modalonly = true;

		var oId = sGrid.taboption('advanced', form.Value, 'id', _('Config ID'));
		oId.rmempty = true;
		oId.modalonly = true;

		return m.render().then(function(mapNode) {
			var statusContainer = E('div', { 'id': 'modem_status_view' }, _('Loading modem status') + '...');
			var statusFieldset = E('fieldset', {
				'id': 'modem_status_field',
				'class': 'cbi-section',
				'style': 'display:none;'
			}, [
				E('h3', {}, _('Modem Status')),
				statusContainer
			]);

			var dialLogFieldset = E('fieldset', {
				'id': 'modem_dial_log_field',
				'class': 'cbi-section',
				'style': 'display:none;'
			}, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', { 'id': 'dial_log_title' }, _('Dial Log'))
				]),
				E('ul', { 'class': 'cbi-tabmenu', 'id': 'dial_log_tab_menu' }),
				E('div', { 'id': 'dial_log_view' }),
				E('div', { 'class': 'cbi-page-actions' }, [
					E('button', {
						'class': 'btn cbi-button cbi-button-link',
						'click': function(ev) {
							ev.preventDefault();
							var key = self.activeLogTab || 'modem0';
							var ta = document.getElementById(key + '_log');
							var text = ta ? ta.value : '';
							var blob = new Blob([text], { type: 'text/plain' });
							var url = URL.createObjectURL(blob);
							var a = document.createElement('a');
							a.href = url;
							a.download = key + '_dial_log.txt';
							a.click();
						}
					}, _('Download Log')),
					' ',
					E('button', {
						'class': 'btn cbi-button cbi-button-reset',
						'click': function(ev) {
							ev.preventDefault();
							var key = self.activeLogTab || 'modem0';
							var ta = document.getElementById(key + '_log');
							if (ta)
								ta.value = '';
							callCleanDialLog().catch(function() {});
						}
					}, _('Clean'))
				])
			]);

			mapNode.appendChild(statusFieldset);
			mapNode.appendChild(dialLogFieldset);

			self.renderModemStatusCards(statusContainer, modemsData);
			self.renderDialLogs(dialLogData);

			poll.add(function() {
				return Promise.all([
					callGetModems('1').catch(function() { return {}; }),
					callGetDialLogInfo().catch(function() { return {}; })
				]).then(function(res) {
					self.renderModemStatusCards(statusContainer, res[0]);
					self.renderDialLogs(res[1]);
				});
			}, 5);

			return mapNode;
		});
	}
});
