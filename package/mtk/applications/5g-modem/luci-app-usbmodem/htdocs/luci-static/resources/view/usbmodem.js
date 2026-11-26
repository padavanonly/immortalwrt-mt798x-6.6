'use strict';
'require view';
'require form';
'require rpc';

var callGetDevices = rpc.declare({
	object: 'luci.usbmodem',
	method: 'getDevices',
	expect: { devices: [] }
});

return view.extend({
	load: function() {
		return L.resolveDefault(callGetDevices(), []);
	},

	render: function(devices) {
		var m, s, o;
		var devList = Array.isArray(devices) ? devices : [];

		m = new form.Map('usbmodem', _('USB Modem Server'), _('Modem Server For OpenWrt'));

		s = m.section(form.TypedSection, 'service', _('Base Setting'));
		s.anonymous = true;
		s.addremove = false;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Value, 'device', _('Modem device'));
		o.rmempty = false;
		for (var i = 0; i < devList.length; i++) {
			o.value(devList[i]);
		}

		o = s.option(form.Value, 'apn', _('APN'));
		o.rmempty = true;

		o = s.option(form.Value, 'pincode', _('PIN'));
		o.rmempty = true;

		o = s.option(form.Value, 'username', _('PAP/CHAP username'));
		o.rmempty = true;

		o = s.option(form.Value, 'password', _('PAP/CHAP password'));
		o.rmempty = true;

		o = s.option(form.Value, 'auth', _('Authentication Type'));
		o.rmempty = true;
		o.value('', _('-- Please choose --'));
		o.value('both', 'PAP/CHAP (both)');
		o.value('pap', 'PAP');
		o.value('chap', 'CHAP');
		o.value('none', 'NONE');

		o = s.option(form.Value, 'tool', _('Tools'));
		o.value('quectel-CM', 'quectel-CM');
		o.rmempty = true;

		return m.render();
	}
});
