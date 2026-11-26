'use strict';
'require view';
'require form';

return view.extend({
	render: function() {
		var m, s, o;

		m = new form.Map('gobinetmodem', _('gobinet Modem Server'), _('Modem Server For OpenWrt'));

		s = m.section(form.TypedSection, 'service', _('Base Setting'));
		s.anonymous = true;
		s.addremove = false;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = '0';
		o.rmempty = false;

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
