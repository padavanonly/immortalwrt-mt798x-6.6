'use strict';
'require view';
'require rpc';

var callGetUserLists = rpc.declare({
	object: 'luci.sms_tool',
	method: 'get_user_lists',
	params: [],
	expect: { '': {} }
});

var callSendAt = rpc.declare({
	object: 'luci.sms_tool',
	method: 'send_at',
	params: ['port', 'code'],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return callGetUserLists().catch(function() { return {}; });
	},

	render: function(lists) {
		var atList = (lists && Array.isArray(lists.atcmds)) ? lists.atcmds : [];

		var options = [];
		for (var i = 0; i < atList.length; i++) {
			var a = atList[i];
			if (a && a.name && a.value)
				options.push(E('option', { 'value': a.value }, a.name));
		}

		var codeInput = E('input', {
			'type': 'text',
			'id': 'code',
			'class': 'cbi-input-text'
		});

		var replyPre = E('pre', { 'id': 'odp', 'style': 'visibility:hidden;' }, '');

		var atSelect = E('select', {
			'name': 'ussd',
			'id': 'pl',
			'class': 'cbi-input-select',
			'change': function() {
				replyPre.style.visibility = 'hidden';
				replyPre.textContent = '';
				codeInput.value = atSelect.value;
			},
			'click': function() {
				if (atSelect.value) {
					replyPre.style.visibility = 'hidden';
					replyPre.textContent = '';
					codeInput.value = atSelect.value;
				}
			}
		}, options);

		var sendBtn = E('button', {
			'type': 'button',
			'id': 'sendcmd',
			'class': 'btn cbi-button cbi-button-neutral',
			'style': 'margin-right:5%;',
			'click': function() {
				var s = (codeInput.value || '').trim();
				if (s.length === 0) {
					replyPre.textContent = '';
					window.alert(_('Please enter a AT Command'));
					return false;
				}
				codeInput.value = '';
				replyPre.style.visibility = 'visible';

				callSendAt('', s).then(function(res) {
					var aStr = (res && res.code === 0) ? String(res.reply || '') : String((res && res.error) || '');
					var bStr = aStr.replace(/^[\s\t]*(\r\n|\n|\r)/gm, '');
					replyPre.textContent = bStr;
				});
				return true;
			}
		}, _('Send Command'));

		return E('div', { 'class': 'cbi-map', 'id': 'cbi-sms-atcommands' }, [
			E('h2', {}, _('AT Commands')),
			E('div', { 'class': 'cbi-map-descr' }, [
				_('Web UI for handling AT commands via sms_tool. More information about the sms-tool on the'),
				' ',
				E('a', { 'href': 'https://eko.one.pl/?p=openwrt-sms_tool', 'target': '_blank' }, _('eko.one.pl forum')),
				'.'
			]),
			E('h4', {}, _('Sending commands to modem')),
			E('div', { 'class': 'table', 'width': '100%' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('User AT Commands') + ':'),
					E('div', { 'class': 'td left' }, [atSelect]),
					E('div', { 'class': 'td left' })
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('Command to send') + ':'),
					E('div', { 'class': 'td left' }, [codeInput]),
					E('div', { 'class': 'td left' })
				])
			]),
			E('div', { 'class': 'table', 'width': '100%' }, [
				E('div', { 'class': 'td left' }, [
					_('Reply') + ':',
					E('p'),
					replyPre
				]),
				E('div', { 'class': 'tr cbi-rowstyle-2' }, [
					E('div', { 'class': 'td right' }, [sendBtn])
				])
			])
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
