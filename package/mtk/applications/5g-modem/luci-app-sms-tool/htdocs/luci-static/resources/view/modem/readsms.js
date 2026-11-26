'use strict';
'require view';
'require rpc';
'require uci';
'require ui';

var callReadSms = rpc.declare({
	object: 'luci.sms_tool',
	method: 'read_sms',
	params: ['port', 'storage'],
	expect: { '': {} }
});

var callDeleteSms = rpc.declare({
	object: 'luci.sms_tool',
	method: 'delete_sms',
	params: ['port', 'index', 'all'],
	expect: { '': {} }
});

var callSmsCount = rpc.declare({
	object: 'luci.sms_tool',
	method: 'sms_count',
	params: ['port', 'storage'],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('sms_tool'),
			callReadSms('', '').catch(function() { return {}; })
		]);
	},

	mergeAndSortMessages: function(rawMessages, mergesms, algorithm, direction) {
		var array = Array.isArray(rawMessages) ? rawMessages.slice() : [];
		if (String(mergesms) !== '1') {
			return array.sort(function(a, b) {
				return new Date(b.timestamp) - new Date(a.timestamp);
			});
		}

		if (algorithm === 'Advanced') {
			var sortedParts = array.slice();
			if (direction === 'End') {
				sortedParts.sort(function(a, b) {
					if (a.timestamp === b.timestamp && a.sender === b.sender)
						return (Number(b.part) || 0) - (Number(a.part) || 0);
					else if (a.timestamp === b.timestamp)
						return String(a.sender || '').localeCompare(String(b.sender || ''));
					else
						return String(a.timestamp || '').localeCompare(String(b.timestamp || ''));
				});
			} else {
				sortedParts.sort(function(a, b) {
					if (a.timestamp === b.timestamp && a.sender === b.sender)
						return (Number(a.part) || 0) - (Number(b.part) || 0);
					else if (a.timestamp === b.timestamp)
						return String(a.sender || '').localeCompare(String(b.sender || ''));
					else
						return String(a.timestamp || '').localeCompare(String(b.timestamp || ''));
				});
			}

			var sortedByTime = sortedParts.sort(function(a, b) {
				return new Date(b.timestamp) - new Date(a.timestamp);
			});

			var combined = {};
			var keys = [];
			for (var i = 0; i < sortedByTime.length; i++) {
				var p = sortedByTime[i];
				var key = String(p.sender) + '-' + String(p.timestamp) + '-' + String(p.total);
				if (combined[key]) {
					combined[key].content += String(p.content || '');
					combined[key].index += '-' + String(p.index);
				} else {
					combined[key] = {
						sender: p.sender,
						timestamp: p.timestamp,
						total: p.total,
						content: String(p.content || ''),
						index: String(p.index)
					};
					keys.push(key);
				}
			}
			return keys.map(function(k) { return combined[k]; });
		}

		// Simple algorithm
		var sortedData = array.sort(function(a, b) {
			return new Date(b.timestamp) - new Date(a.timestamp);
		});
		var bySender = Object.create(null);
		var result = [];
		sortedData.forEach(function(o) {
			var s = o.sender;
			if (!bySender[s]) {
				bySender[s] = {
					index: String(o.index),
					sender: o.sender,
					timestamp: o.timestamp,
					part: o.part,
					total: o.total,
					content: String(o.content || '')
				};
				result.push(bySender[s]);
				return;
			}
			if (bySender[s].total === o.total && Number(bySender[s].part) > 0) {
				bySender[s].content += String(o.content || '');
				bySender[s].index += '-' + String(o.index);
			} else {
				bySender[s] = {
					index: String(o.index),
					sender: o.sender,
					timestamp: o.timestamp,
					part: o.part,
					total: o.total,
					content: String(o.content || '')
				};
				result.push(bySender[s]);
			}
		});
		return result;
	},

	formatUsage: function(useVal, allVal) {
		var vn = parseInt(useVal, 10) || 0;
		var mn = parseInt(allVal, 10) || 100;
		var pc = mn > 0 ? Math.floor((100 / mn) * vn) : 0;
		return vn + ' / ' + mn + ' (' + pc + '%)';
	},

	toggleAll: function(sourceCb) {
		var checkboxes = document.querySelectorAll('input[name="smsn"]');
		for (var i = 0; i < checkboxes.length; i++)
			checkboxes[i].checked = sourceCb.checked;
	},

	renderRows: function(tableEl, messages) {
		while (tableEl.rows.length > 1)
			tableEl.deleteRow(tableEl.rows.length - 1);

		var iconUrl = L.resource('icons/delsms.png');
		for (var d = 0; d < messages.length; d++) {
			var msg = messages[d];
			var tr = E('tr', { 'class': 'tr cbi-rowstyle-' + ((d % 2) + 1) }, [
				E('td', { 'class': 'td left', 'width': '7%' }, [
					E('input', {
						'type': 'checkbox',
						'name': 'smsn',
						'id': 'btn' + String(msg.index)
					}),
					' ',
					E('img', { 'src': iconUrl, 'alt': 'SMS' })
				]),
				E('td', { 'class': 'td left', 'width': '11%' }, String(msg.sender || '')),
				E('td', { 'class': 'td left', 'width': '15%' }, String(msg.timestamp || '')),
				E('td', { 'class': 'td left', 'width': '66%' }, String(msg.content || ''))
			]);
			tableEl.appendChild(tr);
		}
	},

	handleDelete: function(allMax) {
		var self = this;
		var allItems = document.querySelectorAll('input[name="smsn"]');
		var checkedItems = document.querySelectorAll('input[name="smsn"]:checked');
		var tableEl = document.getElementById('smsTable');
		var usseEl = document.getElementById('usse');

		if (checkedItems.length === 0) {
			window.alert(_('Please select the message(s) to be deleted'));
			return false;
		}

		if (allItems.length > 0 && checkedItems.length === allItems.length) {
			if (!window.confirm(_('Delete all the messages?')))
				return false;

			return callDeleteSms('', 'all', true).then(function() {
				if (tableEl) {
					while (tableEl.rows.length > 1)
						tableEl.deleteRow(tableEl.rows.length - 1);
				}
				if (usseEl)
					usseEl.textContent = '0 / ' + (allMax || '50') + ' (0%)';
				return callSmsCount('', '');
			});
		}

		var selectedIds = [];
		for (var i = 0; i < checkedItems.length; i++)
			selectedIds.push(checkedItems[i].id);
		var smsindex = selectedIds.join('-');

		if (!window.confirm(_('Delete selected message(s)?')))
			return false;

		return callDeleteSms('', smsindex, false).then(function() {
			if (tableEl) {
				var idx = 1;
				while (idx < tableEl.rows.length) {
					var cb = tableEl.rows[idx].querySelector('input[name="smsn"]');
					if (cb && cb.checked)
						tableEl.deleteRow(idx);
					else
						idx++;
				}
			}
			return callSmsCount('', '').then(function(sim) {
				if (sim && usseEl)
					usseEl.textContent = self.formatUsage(sim.use, sim.all);
			});
		});
	},

	render: function(loadResults) {
		var self = this;
		var smsData = loadResults[1] || {};
		var mergesms = uci.get('sms_tool', 'general', 'mergesms') || smsData.mergesms || '0';
		var algorithm = uci.get('sms_tool', 'general', 'algorithm') || smsData.algorithm || 'Simple';
		var direction = uci.get('sms_tool', 'general', 'direction') || smsData.direction || 'Start';
		var storage = uci.get('sms_tool', 'general', 'storage') || smsData.storage || 'SM';
		var locationLabel = (storage === 'ME') ? _('Modem memory') : _('SIM card');

		var mergedList = this.mergeAndSortMessages(smsData.messages || [], mergesms, algorithm, direction);
		var usageText = this.formatUsage(smsData.use || 0, smsData.all || 50);

		var allCheck = E('input', {
			'type': 'checkbox',
			'id': 'allcheck',
			'click': function(ev) {
				self.toggleAll(ev.target);
			}
		});

		var smsTable = E('table', { 'id': 'smsTable', 'width': '100%', 'class': 'table' }, [
			E('tr', { 'class': 'tr cbi-rowstyle-2' }, [
				E('td', { 'class': 'td left', 'width': '7%' }, [allCheck]),
				E('td', { 'class': 'td left', 'width': '11%' }, [E('strong', {}, _('From'))]),
				E('td', { 'class': 'td left', 'width': '15%' }, [E('strong', {}, _('Received'))]),
				E('td', { 'class': 'td center', 'width': '66%' }, [E('strong', {}, _('Message'))])
			])
		]);

		this.renderRows(smsTable, mergedList);

		return E('div', { 'class': 'cbi-map', 'id': 'cbi-sms-readsms' }, [
			E('h2', {}, _('SMS Messages')),
			E('div', { 'class': 'cbi-map-descr' }, [
				_('Web UI for receiveling messages via sms_tool. More information about the sms-tool on the'),
				' ',
				E('a', { 'href': 'https://eko.one.pl/?p=openwrt-sms_tool', 'target': '_blank' }, _('eko.one.pl forum')),
				'.'
			]),
			E('h4', {}, _('Received Messages')),
			E('div', { 'class': 'table', 'width': '100%' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left', 'width': '33%' }, _('Messages store in') + ':'),
					E('div', { 'class': 'td left', 'id': 'msg_location' }, locationLabel)
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left', 'width': '33%' }, _('Messages (Inbox / Maximum)') + ':'),
					E('div', { 'class': 'td left', 'id': 'usse' }, usageText)
				])
			]),
			smsTable,
			E('div', { 'class': 'table', 'width': '100%' }, [
				E('div', { 'class': 'tr cbi-rowstyle-2' }, [
					E('div', { 'class': 'td right', 'style': 'width:100%;' }, [
						E('button', {
							'type': 'button',
							'id': 'delbtn',
							'class': 'btn cbi-button cbi-button-neutral',
							'click': function() {
								window.location.reload();
							}
						}, _('Refresh SMS')),
						' ',
						E('button', {
							'type': 'button',
							'id': 'delabtn',
							'class': 'btn cbi-button cbi-button-neutral',
							'click': function() {
								self.handleDelete(smsData.all || '50');
							}
						}, _('Delete message(s)'))
					])
				])
			])
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
