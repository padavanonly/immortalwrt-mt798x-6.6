'use strict';
'require view';
'require rpc';
'require uci';

var callGetUserLists = rpc.declare({
	object: 'luci.sms_tool',
	method: 'get_user_lists',
	params: [],
	expect: { '': {} }
});

var callSendSms = rpc.declare({
	object: 'luci.sms_tool',
	method: 'send_sms',
	params: ['port', 'number', 'message'],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('sms_tool'),
			callGetUserLists().catch(function() { return {}; })
		]);
	},

	normalizeDiacritics: function(val) {
		return String(val || '')
			.replace(/ą/g, 'a').replace(/Ą/g, 'A')
			.replace(/ć/g, 'c').replace(/Ć/g, 'C')
			.replace(/ę/g, 'e').replace(/Ę/g, 'E')
			.replace(/ł/g, 'l').replace(/Ł/g, 'L')
			.replace(/ń/g, 'n').replace(/Ń/g, 'N')
			.replace(/ó/g, 'o').replace(/Ó/g, 'O')
			.replace(/ś/g, 's').replace(/Ś/g, 'S')
			.replace(/ż/g, 'z').replace(/Ż/g, 'Z')
			.replace(/ź/g, 'z').replace(/Ź/g, 'Z');
	},

	render: function(loadResults) {
		var self = this;
		var lists = loadResults[1] || {};
		var phonebook = Array.isArray(lists.phonebook) ? lists.phonebook : [];

		var pon = uci.get('sms_tool', 'general', 'prefix') || '1';
		var pnumber = uci.get('sms_tool', 'general', 'pnumber') || '48';
		var info = uci.get('sms_tool', 'general', 'information') || '1';

		var pbOptions = [
			E('option', { 'value': (pon === '1') ? pnumber : '' }, '>')
		];
		for (var i = 0; i < phonebook.length; i++) {
			var entry = phonebook[i];
			if (entry && entry.name && entry.value)
				pbOptions.push(E('option', { 'value': entry.value }, entry.name));
		}

		var pbSelect = E('select', {
			'name': 'ussd',
			'id': 'pb',
			'class': 'cbi-input-select',
			'change': function() {
				var numEl = document.getElementById('numer');
				var odpEl = document.getElementById('odp');
				if (numEl)
					numEl.value = pbSelect.value;
				if (odpEl)
					odpEl.textContent = '';
			}
		}, pbOptions);

		var numerInput = E('input', {
			'type': 'text',
			'id': 'numer',
			'class': 'cbi-input-text',
			'placeholder': _('PHONE_NUMBER'),
			'minlength': '3',
			'maxlength': '24',
			'size': '11',
			'value': (pon === '1') ? pnumber : '',
			'keyup': function() {
				var odpEl = document.getElementById('odp');
				if (odpEl)
					odpEl.textContent = '';
			}
		});

		var counterEl = E('div', { 'id': 'counter' }, '160');

		var smsTxt = E('textarea', {
			'id': 'smstxt',
			'rows': '5',
			'style': 'height:120px;min-width:100%;resize:vertical;',
			'keyup': function() {
				smsTxt.value = self.normalizeDiacritics(smsTxt.value);
				counterEl.textContent = String(160 - smsTxt.value.length);
				var odpEl = document.getElementById('odp');
				if (odpEl)
					odpEl.textContent = '';
			}
		});

		var statusEl = E('div', { 'class': 'td left', 'id': 'odp' }, '');

		var sendBtn = E('button', {
			'type': 'button',
			'id': 'sendsms',
			'class': 'btn cbi-button cbi-button-neutral',
			'style': 'margin-right:5%;',
			'click': function() {
				var ph = (numerInput.value || '').trim();
				if (ph.length < 3) {
					statusEl.textContent = '';
					window.alert(_('Please enter phone number'));
					return false;
				}
				if (!/^\+?[0-9]{3,24}$/.test(ph)) {
					window.alert(_('Please correct phone number'));
					return false;
				}
				var tm = smsTxt.value || '';
				if (tm.length === 0) {
					window.alert(_('Please enter a message text'));
					return false;
				}

				callSendSms('', ph, tm).then(function(res) {
					var reply = (res && res.reply) ? String(res.reply) : '';
					if (reply.indexOf('sms sent sucessfully') !== -1 || (res && res.code === 0)) {
						statusEl.textContent = _('SMS sent sucessfully');
					} else {
						statusEl.textContent = (res && res.error) ? res.error : reply;
					}
					smsTxt.value = '';
					counterEl.textContent = '160';
					numerInput.value = (pon === '1') ? pnumber : '';
				});
				return true;
			}
		}, _('Send Message'));

		var infoBanner = (info === '1')
			? E('div', { 'class': 'alert-message notice', 'style': 'margin-bottom:1rem;' },
				_('The phone number should be preceded by the country prefix (for Poland it is 48, without +). If the number is 5, 4 or 3 characters, it is treated as.. short and should not be preceded by a country prefix.'))
			: E('span');

		return E('div', { 'class': 'cbi-map', 'id': 'cbi-sms-sendsms' }, [
			E('h2', {}, _('SMS Messages')),
			E('div', { 'class': 'cbi-map-descr' }, [
				_('Web UI for sending messages via sms_tool. More information about the sms-tool on the'),
				' ',
				E('a', { 'href': 'https://eko.one.pl/?p=openwrt-sms_tool', 'target': '_blank' }, _('eko.one.pl forum')),
				'.'
			]),
			infoBanner,
			E('h4', {}, _('Send Messages')),
			E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('User Phonebook') + ':'),
					E('div', { 'class': 'td left' }, [pbSelect]),
					E('div', { 'class': 'td left' })
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('Send to') + ':'),
					E('div', { 'class': 'td left' }, [numerInput]),
					E('div', { 'class': 'td left' })
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('Message text') + ':'),
					E('div', { 'class': 'td left' }, [smsTxt, counterEl]),
					E('div', { 'class': 'td left' })
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, _('Status') + ':'),
					statusEl
				])
			]),
			E('div', { 'class': 'table' }, [
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
