/* Copyright (C) 2026 ImmortalWrt.org */

'use strict';
'require form';
'require view';

return view.extend({
	render: function() {
		var m, s, o;

		m = new form.Map('openfi', _('OpenFi Settings'),
			_('Configure OpenFi hardware switch mode and 5G modem reset polarity.'));

		s = m.section(form.NamedSection, 'switch', 'switch', _('Switch Settings'));
		s.anonymous = true;
		s.addremove = false;

		o = s.option(form.ListValue, 'func', _('Switch configuration'));
		o.value('0', _('Off'));
		o.value('1', _('WAN/LAN Switch'));
		o.value('2', _('Modem Power Switch'));
		o.value('3', _('Led Switch'));
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.ListValue, 'lan', _('WAN/LAN Switch'));
		o.depends('func', '0');
		o.depends('func', '2');
		o.depends('func', '3');
		o.value('0', _('WAN'));
		o.value('1', _('LAN'));
		o.default = '1';

		o = s.option(form.ListValue, 'power', _('Modem Power Switch'));
		o.depends('func', '0');
		o.depends('func', '1');
		o.depends('func', '3');
		o.value('0', _('Off'));
		o.value('1', _('On'));
		o.default = '1';

		o = s.option(form.ListValue, 'led', _('Led Switch'));
		o.depends('func', '0');
		o.depends('func', '1');
		o.depends('func', '2');
		o.value('0', _('Off'));
		o.value('1', _('On'));
		o.default = '1';

		s = m.section(form.NamedSection, 'reset', 'modem', _('Modem Reset Settings'));
		s.anonymous = true;
		s.addremove = false;

		o = s.option(form.Flag, 'state', _('Modem reset reverse'));
		o.rmempty = false;

		return m.render();
	}
});
