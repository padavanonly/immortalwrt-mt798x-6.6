'use strict';
'require view';
'require rpc';

var callGetUserLists = rpc.declare({
	object: 'luci.sms_tool',
	method: 'get_user_lists',
	params: [],
	expect: { '': {} }
});

var callSendUssd = rpc.declare({
	object: 'luci.sms_tool',
	method: 'send_ussd',
	params: ['port', 'code'],
	expect: { '': {} }
});

var USSD_ERROR_MAP = {
	'error: 0': _('Phone/Modem failure.'),
	'error: 1': _('No connection to phone.'),
	'error: 2': _('Phone/Modem adapter link reserved.'),
	'error: 3': _('Operation not allowed.'),
	'error: 4': _('Operation not supported.'),
	'error: 5': _('PH_SIM PIN required.'),
	'error: 6': _('PH_FSIM PIN required.'),
	'error: 7': _('PH_FSIM PUK required.'),
	'error: 10': _('SIM not inserted.'),
	'error: 11': _('SIM PIN required.'),
	'error: 12': _('SIM PUK required.'),
	'error: 13': _('SIM failure.'),
	'error: 14': _('SIM busy.'),
	'error: 15': _('SIM wrong.'),
	'error: 16': _('Incorrect password.'),
	'error: 17': _('SIM PIN2 required.'),
	'error: 18': _('SIM PUK2 required.'),
	'error: 20': _('Memory full.'),
	'error: 21': _('Invalid index.'),
	'error: 22': _('Not found.'),
	'error: 23': _('Memory failure.'),
	'error: 24': _('Text string too long.'),
	'error: 25': _('Invalid characters in text string.'),
	'error: 26': _('Dial string too long.'),
	'error: 27': _('Invalid characters in dial string.'),
	'error: 30': _('No network service.'),
	'error: 31': _('Network timeout.'),
	'error: 32': _('Network not allowed, emergency calls only.'),
	'error: 40': _('Network personalization PIN required.'),
	'error: 41': _('Network personalization PUK required.'),
	'error: 42': _('Network subset personalization PIN required.'),
	'error: 43': _('Network subset personalization PUK required.'),
	'error: 44': _('Service provider personalization PIN required.'),
	'error: 45': _('Service provider personalization PUK required.'),
	'error: 46': _('Corporate personalization PIN required.'),
	'error: 47': _('Corporate personalization PUK required.'),
	'error: 48': _('PH-SIM PUK required.'),
	'error: 100': _('Unknown error.'),
	'error: 103': _('Illegal MS.'),
	'error: 106': _('Illegal ME.'),
	'error: 107': _('GPRS services not allowed.'),
	'error: 111': _('PLMN not allowed.'),
	'error: 112': _('Location area not allowed.'),
	'error: 113': _('Roaming not allowed in this location area.'),
	'error: 126': _('Operation temporary not allowed.'),
	'error: 132': _('Service operation not supported.'),
	'error: 133': _('Requested service option not subscribed.'),
	'error: 134': _('Service option temporary out of order.'),
	'error: 148': _('Unspecified GPRS error.'),
	'error: 149': _('PDP authentication failure.'),
	'error: 150': _('Invalid mobile class.'),
	'error: 256': _('Operation temporarily not allowed.'),
	'error: 257': _('Call barred.'),
	'error: 258': _('Phone/Modem is busy.'),
	'error: 259': _('User abort.'),
	'error: 260': _('Invalid dial string.'),
	'error: 261': _('SS not executed.'),
	'error: 262': _('SIM Blocked.'),
	'error: 263': _('Invalid block.'),
	'error: 527': _('Please wait, and retry your selection later (Specific Modem Sierra).'),
	'error: 528': _('Location update failure – emergency calls only (Specific Modem Sierra).'),
	'error: 529': _('Selection failure – emergency calls only (Specific Modem Sierra).'),
	'error: 772': _('SIM powered down.')
};

return view.extend({
	load: function() {
		return callGetUserLists().catch(function() { return {}; });
	},

	translateReply: function(rawReply) {
		var cut = String(rawReply || '');
		for (var errKey in USSD_ERROR_MAP) {
			if (Object.prototype.hasOwnProperty.call(USSD_ERROR_MAP, errKey)) {
				if (cut.indexOf(errKey) !== -1)
					return USSD_ERROR_MAP[errKey];
			}
		}
		return cut;
	},

	render: function(lists) {
		var self = this;
		var ussdList = (lists && Array.isArray(lists.ussd)) ? lists.ussd : [];

		var options = [];
		for (var i = 0; i < ussdList.length; i++) {
			var u = ussdList[i];
			if (u && u.name && u.value)
				options.push(E('option', { 'value': u.value }, u.name));
		}

		var codeInput = E('input', {
			'type': 'text',
			'id': 'code',
			'class': 'cbi-input-text',
			'size': '20'
		});

		var replyPre = E('pre', { 'id': 'odp', 'style': 'visibility:hidden;' }, '');

		var ussdSelect = E('select', {
			'name': 'ussd',
			'id': 'pl',
			'class': 'cbi-input-select',
			'change': function() {
				replyPre.style.visibility = 'hidden';
				replyPre.textContent = '';
				codeInput.value = ussdSelect.value;
			},
			'click': function() {
				if (ussdSelect.value) {
					replyPre.style.visibility = 'hidden';
					replyPre.textContent = '';
					codeInput.value = ussdSelect.value;
				}
			}
		}, options);

		var sendBtn = E('button', {
			'type': 'button',
			'id': 'sendussd',
			'class': 'btn cbi-button cbi-button-neutral',
			'style': 'margin-right:5%;',
			'click': function() {
				var s = (codeInput.value || '').trim();
				if (s.length === 0) {
					replyPre.textContent = '';
					window.alert(_('Please enter a USSD code'));
					return false;
				}
				codeInput.value = '';
				replyPre.style.visibility = 'visible';

				callSendUssd('', s).then(function(res) {
					if (res && res.code === 0)
						replyPre.textContent = self.translateReply(res.reply || '');
					else
						replyPre.textContent = (res && res.error) ? res.error : _('Unknown error.');
				});
				return true;
			}
		}, _('Send Code'));

		return E('div', { 'class': 'cbi-map', 'id': 'cbi-sms-ussd' }, [
			E('h2', {}, _('USSD Codes')),
			E('div', { 'class': 'cbi-map-descr' }, [
				_('Web UI for handling USSD codes via sms_tool. More information about the sms-tool on the'),
				' ',
				E('a', { 'href': 'https://eko.one.pl/?p=openwrt-sms_tool', 'target': '_blank' }, _('eko.one.pl forum')),
				'.'
			]),
			E('h4', {}, _('Sending USSD codes')),
			E('div', { 'class': 'table', 'width': '100%' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left', 'width': '25%' }, _('User USSD Codes') + ':'),
					E('div', { 'class': 'td left' }, [ussdSelect]),
					E('div', { 'class': 'td left' })
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('Code to send') + ':'),
					E('div', { 'class': 'td left' }, [codeInput]),
					E('div', { 'class': 'td left' })
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('Reply') + ':'),
					E('div', { 'class': 'td left' }, [replyPre]),
					E('div', { 'class': 'td left' })
				])
			]),
			E('div', { 'class': 'table', 'width': '100%' }, [
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
