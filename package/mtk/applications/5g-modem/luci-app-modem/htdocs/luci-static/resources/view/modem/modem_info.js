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

var callGetModemInfo = rpc.declare({
	object: 'luci.modem',
	method: 'getModemInfo',
	params: ['port', 'translation'],
	expect: { '': {} }
});

function formatRate(rate) {
	var b = parseFloat(rate);
	if (isNaN(b))
		return rate || '-';
	var kb = b / 1024;
	var mb = kb / 1024;
	if (mb >= 1)
		return mb.toFixed(2) + ' MB/s';
	if (kb >= 1)
		return kb.toFixed(2) + ' KB/s';
	return b + ' B/s';
}

function getPercent(value, min, max) {
	var percent;
	if (value <= min)
		percent = 0;
	else if (value >= max)
		percent = 100;
	else
		percent = ((value - min) / (max - min)) * 100;
	return percent.toFixed(2);
}

function getSignalInfoView(networkType, key, rawVal) {
	var value = parseFloat(rawVal);
	var info = {
		percent: '100.00',
		quality: _('Unknown'),
		color: 'gray',
		style: 'width:100.00%;background-color:gray;'
	};
	if (isNaN(value))
		return info;

	var net = String(networkType || '');
	if (key === 'RSRP' || key === 'rsrp') {
		if (net.indexOf('NR') !== -1)
			info.percent = getPercent(value, -153, 31);
		else if (net.indexOf('LTE') !== -1)
			info.percent = getPercent(value, -140, -44);
		else
			info.percent = getPercent(value, -140, -44);

		if (value >= -70) {
			info.quality = _('Excellent');
			info.color = 'limegreen';
		} else if (value >= -80) {
			info.quality = _('Good');
			info.color = 'lime';
		} else if (value >= -90) {
			info.quality = _('Fair');
			info.color = 'gold';
		} else {
			info.quality = _('Bad');
			info.color = 'red';
		}
	} else if (key === 'RSRQ' || key === 'rsrq') {
		if (net.indexOf('NR') !== -1)
			info.percent = getPercent(value, -43, 20);
		else
			info.percent = getPercent(value, -19.5, -3);

		if (value >= -10) {
			info.quality = _('Excellent');
			info.color = 'limegreen';
		} else if (value >= -15) {
			info.quality = _('Good');
			info.color = 'lime';
		} else if (value >= -20) {
			info.quality = _('Fair');
			info.color = 'gold';
		} else {
			info.quality = _('Bad');
			info.color = 'red';
		}
	} else if (key === 'SINR' || key === 'sinr') {
		if (net.indexOf('NR') !== -1)
			info.percent = getPercent(value, -23, 40);
		else
			info.percent = getPercent(value, -20, 30);

		if (value >= 25) {
			info.quality = _('Excellent');
			info.color = 'limegreen';
		} else if (value >= 15) {
			info.quality = _('Good');
			info.color = 'lime';
		} else if (value >= 10) {
			info.quality = _('Fair');
			info.color = 'gold';
		} else {
			info.quality = _('Bad');
			info.color = 'red';
		}
	} else if (key === 'RxLev' || key === 'RSSI' || key === 'rxlev' || key === 'rssi') {
		if (net.indexOf('NR') !== -1)
			info.percent = getPercent(value, -153, 31);
		else if (net.indexOf('LTE') !== -1)
			info.percent = getPercent(value, -140, -44);
		else if (net.indexOf('WCDMA') !== -1)
			info.percent = getPercent(value, -120, -25);
		else
			info.percent = getPercent(value, -110, -48);

		if (value >= -70) {
			info.quality = _('Excellent');
			info.color = 'limegreen';
		} else if (value >= -80) {
			info.quality = _('Good');
			info.color = 'lime';
		} else if (value >= -90) {
			info.quality = _('Fair');
			info.color = 'gold';
		} else {
			info.quality = _('Bad');
			info.color = 'red';
		}
	}

	info.style = 'width:' + info.percent + '%;background-color:' + info.color + ';';
	if (parseFloat(info.percent) >= 100)
		info.style += 'border-top-right-radius:4px;border-bottom-right-radius:4px;';

	return info;
}

function renderSignalBar(networkType, key, value, unit, translation) {
	var sig = getSignalInfoView(networkType, key, value);
	var qualText = (translation && translation[sig.quality]) ? _(translation[sig.quality]) : sig.quality;
	var label = value + ' ' + unit + ' | ' + qualText + ' (' + sig.percent + '%)';
	return E('div', { 'class': 'signal-view', 'style': 'position:relative;min-width:11rem;height:1.5rem;border:thin solid #999;background:#eee;border-radius:4px;overflow:hidden;' }, [
		E('div', { 'class': 'signal-percent', 'style': 'height:100%;border-top-left-radius:4px;border-bottom-left-radius:4px;' + sig.style }, [
			E('div', { 'class': 'signal-context', 'style': 'font-family:monospace;font-weight:bold;line-height:1.5rem;position:absolute;top:0;right:0;bottom:0;left:0;text-align:center;white-space:pre;text-overflow:ellipsis;overflow:hidden;' }, [
				E('small', { 'title': label }, label)
			])
		])
	]);
}

function translateKey(key, translation) {
	if (translation && translation[key])
		return _(translation[key]);
	return _(key);
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('modem'),
			callGetATPort('1').catch(function() { return {}; })
		]);
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
					if (label.indexOf('unknown') !== -1) {
						if (translation && translation[label])
							label = _(translation[label]);
						else if (translation && translation['unknown'])
							label = label.replace('unknown', _(translation['unknown']));
						else
							label = label.replace('unknown', _('Unknown'));
					}
					var opt = E('option', { 'value': port }, label);
					selectEl.appendChild(opt);
				}
			}
		} else if (atPorts && typeof atPorts === 'object') {
			for (var k in atPorts) {
				if (!Object.prototype.hasOwnProperty.call(atPorts, k))
					continue;
				var text = atPorts[k] || k;
				var optEl = E('option', { 'value': k }, text);
				selectEl.appendChild(optEl);
			}
		}

		for (var j = 0; j < selectEl.options.length; j++) {
			if (selectEl.options[j].value === selected) {
				selectEl.selectedIndex = j;
				break;
			}
		}
	},

	renderKeyValueRows: function(tableEl, listOrObj, translation, formatter) {
		while (tableEl.firstChild)
			tableEl.removeChild(tableEl.firstChild);

		var tbody = E('tbody', {});
		if (Array.isArray(listOrObj)) {
			for (var i = 0; i < listOrObj.length; i++) {
				var item = listOrObj[i];
				var fullName = item.full_name || '';
				for (var key in item) {
					if (!Object.prototype.hasOwnProperty.call(item, key) || key === 'full_name')
						continue;
					var val = item[key];
					if (val === '-' || val === '' || val === null || val === undefined)
						continue;
					var displayNode = formatter ? formatter(key, val, translation) : String(val);
					if (displayNode === null)
						continue;
					tbody.appendChild(E('tr', { 'class': 'tr' }, [
						E('td', { 'class': 'td left', 'title': fullName, 'style': 'width:33%;' }, translateKey(key, translation)),
						E('td', { 'class': 'td left', 'id': key }, displayNode)
					]));
					break;
				}
			}
		} else if (listOrObj && typeof listOrObj === 'object') {
			for (var k in listOrObj) {
				if (!Object.prototype.hasOwnProperty.call(listOrObj, k))
					continue;
				var v = listOrObj[k];
				if (v === '-' || v === '' || v === null || v === undefined)
					continue;
				var node = formatter ? formatter(k, v, translation) : String(v);
				if (node === null)
					continue;
				tbody.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'style': 'width:33%;' }, translateKey(k, translation)),
					E('td', { 'class': 'td left', 'id': k }, node)
				]));
			}
		}
		tableEl.appendChild(tbody);
	},

	formatCellEntry: function(networkType, key, val, translation) {
		var net = String(networkType || '');
		if (key === 'Band' || key === 'band') {
			if (net.indexOf('NR') !== -1)
				return 'N' + val;
			if (net.indexOf('LTE') !== -1 || net.indexOf('WCDMA') !== -1)
				return 'B' + val;
			return String(val);
		}
		if (key === 'UL Bandwidth' || key === 'DL Bandwidth' || key === 'ul_bandwidth' || key === 'dl_bandwidth')
			return val + ' MHz';
		if (key === 'RSRP' || key === 'rsrp')
			return renderSignalBar(net, 'RSRP', val, 'dBm', translation);
		if (key === 'RxLev' || key === 'RSSI' || key === 'rxlev' || key === 'rssi')
			return renderSignalBar(net, 'RxLev', val, 'dBm', translation);
		if (key === 'RSRQ' || key === 'rsrq')
			return renderSignalBar(net, 'RSRQ', val, 'dB', translation);
		if (key === 'SINR' || key === 'sinr')
			return renderSignalBar(net, 'SINR', val, 'dB', translation);
		if (key === 'TX Power' || key === 'tx_power')
			return val + ' dBm';
		if (key === 'RSSNR' || key === 'Srxlev' || key === 'Ec/Io' || key === 'rssnr' || key === 'srxlev')
			return val + ' dB';
		if (key === 'SCS' || key === 'scs')
			return val + ' kHz';
		return String(val);
	},

	updateModemView: function(data) {
		var modemInfo = (data && data.modem_info) ? data.modem_info : (data || {});
		var translation = (data && data.translation) ? data.translation : {};
		var deviceInfo = modemInfo.device_info || modemInfo;
		var moreInfo = modemInfo.more_info || modemInfo;
		var baseInfo = moreInfo.base_info || modemInfo;

		var mergedBase = Object.assign({}, deviceInfo, baseInfo);
		var baseFields = [
			'manufacturer', 'revision', 'data_interface', 'mode', 'at_port',
			'network', 'temperature', 'voltage', 'connect_status', 'update_time',
			'isp', 'imei', 'imsi', 'iccid', 'phone_number'
		];
		for (var i = 0; i < baseFields.length; i++) {
			var f = baseFields[i];
			var el = document.getElementById(f);
			if (el && mergedBase[f] !== undefined)
				el.textContent = mergedBase[f];
		}

		var infoSection = document.getElementById('cbi-info');
		var infoMessage = document.getElementById('info_message');
		var baseSection = document.getElementById('cbi-baseinfo');
		var simSection = document.getElementById('cbi-siminfo');
		var netSection = document.getElementById('cbi-networkinfo');
		var cellSection = document.getElementById('cbi-cellinfo');

		var manufacturer = mergedBase.manufacturer || 'unknown';
		if (manufacturer === 'unknown') {
			if (infoMessage) {
				infoMessage.innerHTML = '';
				infoMessage.appendChild(E('strong', {}, _('Not adapted to this modem')));
				infoMessage.appendChild(document.createTextNode(' '));
				infoMessage.appendChild(E('a', {
					'href': 'https://github.com/Siriling/5G-Modem-Support/blob/main/luci-app-modem/README.md',
					'target': '_blank'
				}, _('(Check adapted modem)')));
			}
			if (infoSection) infoSection.style.display = 'block';
			if (baseSection) baseSection.style.display = 'block';
			if (simSection) simSection.style.display = 'none';
			if (netSection) netSection.style.display = 'none';
			if (cellSection) cellSection.style.display = 'none';
			return;
		}

		if (infoSection) infoSection.style.display = 'none';
		if (baseSection) baseSection.style.display = 'block';

		var simInfo = moreInfo.sim_info || {
			'SIM Status': mergedBase.sim_status || 'ready',
			'ISP': mergedBase.isp || '',
			'SIM Slot': mergedBase.sim_slot || '',
			'SIM Number': mergedBase.phone_number || '',
			'IMEI': mergedBase.imei || '',
			'IMSI': mergedBase.imsi || '',
			'ICCID': mergedBase.iccid || ''
		};
		var simTable = document.getElementById('sim_info');
		if (simTable) {
			this.renderKeyValueRows(simTable, simInfo, translation, function(k, v) {
				if (v === 'ready')
					return null;
				return String(v);
			});
		}

		var simStatus = 'ready';
		if (Array.isArray(simInfo) && simInfo.length > 0 && simInfo[0]['SIM Status'])
			simStatus = simInfo[0]['SIM Status'];
		else if (simInfo && simInfo['SIM Status'])
			simStatus = simInfo['SIM Status'];
		else if (mergedBase.sim_status)
			simStatus = mergedBase.sim_status;

		if (simStatus === 'unknown') {
			if (infoMessage)
				infoMessage.innerHTML = '<strong>' + _('Unknown SIM card status') + '</strong>';
			if (infoSection) infoSection.style.display = 'block';
		} else if (simStatus === 'miss') {
			if (infoMessage)
				infoMessage.innerHTML = '<strong>' + _('SIM card not inserted') + '</strong>';
			if (infoSection) infoSection.style.display = 'block';
		} else if (simStatus !== 'ready') {
			if (infoMessage)
				infoMessage.innerHTML = '<strong>' + simStatus + '</strong>';
			if (infoSection) infoSection.style.display = 'block';
		}
		if (simSection) simSection.style.display = 'block';

		var connectStatus = mergedBase.connect_status || 'connect';
		if (simStatus !== 'ready' || connectStatus !== 'connect') {
			if (netSection) netSection.style.display = 'none';
			if (cellSection) cellSection.style.display = 'none';
			return;
		}

		var networkInfo = moreInfo.network_info || {
			'Network Type': mergedBase.network_type || '',
			'COPS': mergedBase.cops || mergedBase.isp || '',
			'AMBR UL': mergedBase.ambr_ul || '',
			'AMBR DL': mergedBase.ambr_dl || '',
			'Tx Rate': mergedBase.tx_rate || '',
			'Rx Rate': mergedBase.rx_rate || '',
			'TX Power': mergedBase.tx_power || ''
		};
		var netTable = document.getElementById('network_info');
		if (netTable) {
			this.renderKeyValueRows(netTable, networkInfo, translation, function(k, v) {
				if (k === 'AMBR UL' || k === 'AMBR DL' || k === 'ambr_ul' || k === 'ambr_dl')
					return v + ' Mbps';
				if (k === 'Tx Rate' || k === 'Rx Rate' || k === 'tx_rate' || k === 'rx_rate')
					return formatRate(v);
				return String(v);
			});
		}

		var cellInfo = moreInfo.cell_info || {
			'NR5G-SA Mode': [
				{ 'MCC': mergedBase.mcc || '' },
				{ 'MNC': mergedBase.mnc || '' },
				{ 'Duplex Mode': mergedBase.duplex_mode || '' },
				{ 'Cell ID': mergedBase.cell_id || '' },
				{ 'Physical Cell ID': mergedBase.pcid || '' },
				{ 'TAC': mergedBase.tac || '' },
				{ 'ARFCN': mergedBase.arfcn || '' },
				{ 'Band': mergedBase.band || '' },
				{ 'UL Bandwidth': mergedBase.ul_bandwidth || '' },
				{ 'DL Bandwidth': mergedBase.dl_bandwidth || '' },
				{ 'RSRP': mergedBase.rsrp || '' },
				{ 'RSRQ': mergedBase.rsrq || '' },
				{ 'SINR': mergedBase.sinr || '' },
				{ 'RxLev': mergedBase.rssi || mergedBase.rxlev || '' },
				{ 'SCS': mergedBase.scs || '' },
				{ 'CQI': mergedBase.cqi || '' },
				{ 'MCS': mergedBase.mcs || '' }
			]
		};
		var cellTable = document.getElementById('cell_info');
		if (cellTable && cellInfo && typeof cellInfo === 'object') {
			while (cellTable.firstChild)
				cellTable.removeChild(cellTable.firstChild);

			var modeKeys = Object.keys(cellInfo);
			if (modeKeys.length > 0) {
				var networkMode = modeKeys[0];
				cellTable.appendChild(E('caption', {}, translateKey(networkMode, translation)));
				var tbody = E('tbody', {});
				var modeData = cellInfo[networkMode];
				var self = this;

				var appendEntries = function(entries, netType) {
					if (!Array.isArray(entries))
						return;
					for (var idx = 0; idx < entries.length; idx++) {
						var infoObj = entries[idx];
						var fName = infoObj.full_name || '';
						for (var ck in infoObj) {
							if (!Object.prototype.hasOwnProperty.call(infoObj, ck) || ck === 'full_name')
								continue;
							var cv = infoObj[ck];
							if (cv === '-' || cv === '' || cv === null || cv === undefined)
								continue;
							tbody.appendChild(E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left', 'title': fName, 'style': 'width:33%;' }, translateKey(ck, translation)),
								E('td', { 'class': 'td left', 'id': ck }, self.formatCellEntry(netType, ck, cv, translation))
							]));
							break;
						}
					}
				};

				if (networkMode === 'EN-DC Mode' && Array.isArray(modeData)) {
					if (modeData[0] && modeData[0].LTE) {
						tbody.appendChild(E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'colspan': '2' }, 'LTE')
						]));
						appendEntries(modeData[0].LTE, 'LTE');
					}
					if (modeData[1] && modeData[1]['NR5G-NSA']) {
						tbody.appendChild(E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'colspan': '2' }, 'NR5G-NSA')
						]));
						appendEntries(modeData[1]['NR5G-NSA'], 'NR');
					}
				} else {
					appendEntries(modeData, networkMode);
				}
				cellTable.appendChild(tbody);
			}
		}

		if (netSection) netSection.style.display = 'block';
		if (cellSection) cellSection.style.display = 'block';
	},

	showNoModemsView: function() {
		var infoSection = document.getElementById('cbi-info');
		var infoMessage = document.getElementById('info_message');
		var baseSection = document.getElementById('cbi-baseinfo');
		var simSection = document.getElementById('cbi-siminfo');
		var netSection = document.getElementById('cbi-networkinfo');
		var cellSection = document.getElementById('cbi-cellinfo');

		if (infoMessage) {
			infoMessage.innerHTML = '';
			infoMessage.appendChild(E('strong', {}, _('No modems found')));
			infoMessage.appendChild(document.createTextNode(' '));
			infoMessage.appendChild(E('a', {
				'href': 'https://blog.siriling.com:1212/2023/03/18/openwrt-5g-modem/#zi-dong-sao-miao-shi-bie-gong-neng',
				'target': '_blank'
			}, _('(Check the reason)')));
		}
		if (infoSection) infoSection.style.display = 'block';
		if (baseSection) baseSection.style.display = 'none';
		if (simSection) simSection.style.display = 'none';
		if (netSection) netSection.style.display = 'none';
		if (cellSection) cellSection.style.display = 'none';
	},

	render: function(loadResults) {
		var self = this;
		var initialATData = loadResults[1] || {};
		var translationFlag = '1';

		var modemSelect = E('select', {
			'name': 'modem_select',
			'id': 'modem_select',
			'class': 'cbi-input-select',
			'change': function() {
				self.refreshModemInfo(modemSelect.value, translationFlag);
			}
		});

		var translationToggle = E('input', {
			'type': 'checkbox',
			'id': 'translation_toggle',
			'checked': 'checked',
			'change': function(ev) {
				translationFlag = ev.target.checked ? '1' : '0';
				self.refreshModemInfo(modemSelect.value, translationFlag);
			}
		});

		this.populateATPortSelect(modemSelect, initialATData.at_ports || [], initialATData.translation || {});

		var viewNode = E('div', { 'class': 'cbi-map', 'id': 'cbi-modem' }, [
			E('h2', { 'name': 'content' }, _('Modem Information')),
			E('div', { 'class': 'cbi-map-descr' }, _('Check the information of the adapted modem')),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-info', 'style': 'display:block;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Message')),
					E('table', { 'class': 'table' }, [
						E('tbody', { 'id': 'message' }, [
							E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left' }, [
									E('div', { 'id': 'info_message', 'style': 'font-size:1.4em;' }, _('Loading modem information') + '...')
								]),
								E('td', { 'class': 'td right' }, [
									E('label', { 'style': 'margin-right:0.5rem;' }, [
										translationToggle,
										' ',
										_('Translation')
									])
								])
							])
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-baseinfo', 'style': 'display:none;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Base Information')),
					E('table', { 'class': 'table', 'id': 'base_info' }, [
						E('tbody', {}, [
							E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left', 'style': 'width:33%;' }, _('Modem Name')),
								E('td', { 'class': 'td left', 'id': 'modem_name' }, [modemSelect])
							]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Manufacturer')), E('td', { 'class': 'td left', 'id': 'manufacturer' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Revision')), E('td', { 'class': 'td left', 'id': 'revision' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Data Interface')), E('td', { 'class': 'td left', 'id': 'data_interface' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Mode')), E('td', { 'class': 'td left', 'id': 'mode' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('AT Port')), E('td', { 'class': 'td left', 'id': 'at_port' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Mobile Network')), E('td', { 'class': 'td left', 'id': 'network' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Temperature')), E('td', { 'class': 'td left', 'id': 'temperature' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Voltage')), E('td', { 'class': 'td left', 'id': 'voltage' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Connect Status')), E('td', { 'class': 'td left', 'id': 'connect_status' }, '-')]),
							E('tr', { 'class': 'tr' }, [E('td', { 'class': 'td left' }, _('Update Time')), E('td', { 'class': 'td left', 'id': 'update_time' }, '-')])
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-siminfo', 'style': 'display:none;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('SIM Information')),
					E('table', { 'class': 'table', 'id': 'sim_info' })
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-networkinfo', 'style': 'display:none;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Network Information')),
					E('table', { 'class': 'table', 'id': 'network_info' })
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-cellinfo', 'style': 'display:none;' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Cell Information')),
					E('table', { 'class': 'table', 'id': 'cell_info' })
				])
			])
		]);

		poll.add(function() {
			return callGetATPort(translationFlag).then(function(atData) {
				var ports = (atData && atData.at_ports) ? atData.at_ports : [];
				var trans = (atData && atData.translation) ? atData.translation : {};
				self.populateATPortSelect(modemSelect, ports, trans);

				if (!ports || (Array.isArray(ports) && ports.length === 0) || (typeof ports === 'object' && Object.keys(ports).length === 0)) {
					self.showNoModemsView();
					return;
				}
				return self.refreshModemInfo(modemSelect.value, translationFlag);
			});
		}, 5);

		return viewNode;
	},

	refreshModemInfo: function(port, translationFlag) {
		if (!port)
			return Promise.resolve();
		var self = this;
		return callGetModemInfo(port, translationFlag).then(function(infoData) {
			if (infoData)
				self.updateModemView(infoData);
		}).catch(function() {});
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
