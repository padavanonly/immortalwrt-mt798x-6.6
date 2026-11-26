'use strict';
'require view';
'require form';
'require rpc';

var callGetDevices = rpc.declare({
	object: 'luci.hypermodem',
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

		m = new form.Map('hypermodem', _('Hyper Modem Server'), _('Modem Server For OpenWrt'));

		s = m.section(form.TypedSection, 'service', _('Base Setting'));
		s.anonymous = true;
		s.addremove = false;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Flag, 'ipv6', _('Enable IPv6'));
		o.default = '1';
		o.rmempty = false;

		o = s.option(form.Flag, 'network_bridge', _('Enable Network bridge'), _('After checking, enable network interface bridge.'));
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Value, 'device', _('Modem device'));
		o.rmempty = false;
		for (var i = 0; i < devList.length; i++) {
			o.value(devList[i]);
		}

		o = s.option(form.Value, 'apn', _('APN'));
		o.default = '';
		o.rmempty = true;
		o.value('', _('Auto Choose'));
		o.value('cmnet', _('China Mobile'));
		o.value('3gnet', _('China Unicom'));
		o.value('ctnet', _('China Telecom'));
		o.value('cbnet', _('China Broadcast'));
		o.value('5gscuiot', _('Skytone'));

		o = s.option(form.ListValue, 'auth', _('Authentication Type'));
		o.default = 'none';
		o.rmempty = false;
		o.value('none', _('NONE'));
		o.value('both', _('PAP/CHAP (both)'));
		o.value('pap', 'PAP');
		o.value('chap', 'CHAP');

		o = s.option(form.Value, 'username', _('PAP/CHAP Username'));
		o.rmempty = true;
		o.depends('auth', 'both');
		o.depends('auth', 'pap');
		o.depends('auth', 'chap');

		o = s.option(form.Value, 'password', _('PAP/CHAP Password'));
		o.rmempty = true;
		o.password = true;
		o.depends('auth', 'both');
		o.depends('auth', 'pap');
		o.depends('auth', 'chap');

		return m.render();
	}
});
