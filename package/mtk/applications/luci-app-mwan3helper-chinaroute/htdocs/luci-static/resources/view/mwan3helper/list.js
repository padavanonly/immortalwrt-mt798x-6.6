'use strict';
'require view';
'require form';
'require uci';

return view.extend({
	load: function() {
		return uci.load('mwan3helper');
	},

	render: function() {
		var m, s, o;

		m = new form.Map('mwan3helper', _('MWAN3 IPSets'));

		s = m.section(form.TypedSection, 'mwan3helper');
		s.addremove = false;
		s.anonymous = true;

		o = s.option(form.DummyValue, 'gfwlist', _('GFW List'));
		o.description = _('GFW list entries: 5207, IPSet name: gfwlist');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/gfw.txt';
		};

		o = s.option(form.DummyValue, 'all_cn', _('All China IP Addresses'));
		o.description = _('All China IP route entries: 8304, IPSet name: cn');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/all_cn.txt (/etc/mwan3helper/chinaroute.txt)';
		};

		o = s.option(form.DummyValue, 'chinatelecom', _('China Telecom'));
		o.description = _('China Telecom route entries: 1948, IPSet name: ct');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/chinatelecom.txt';
		};

		o = s.option(form.DummyValue, 'unicom_cnc', _('China Unicom / CNC'));
		o.description = _('China Unicom / CNC route entries: 895, IPSet name: cnc');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/unicom_cnc.txt';
		};

		o = s.option(form.DummyValue, 'cmcc', _('China Mobile'));
		o.description = _('China Mobile route entries: 55, IPSet name: cmcc');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/cmcc.txt';
		};

		o = s.option(form.DummyValue, 'crtc', _('China TieTong'));
		o.description = _('China TieTong route entries: 16, IPSet name: crtc');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/crtc.txt';
		};

		o = s.option(form.DummyValue, 'cernet', _('CERNET'));
		o.description = _('CERNET route entries: 162, IPSet name: cernet');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/cernet.txt';
		};

		o = s.option(form.DummyValue, 'gwbn', _('Great Wall Broadband / Dr. Peng'));
		o.description = _('Great Wall Broadband / Dr. Peng entries: 94, IPSet name: gwbn');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/gwbn.txt';
		};

		o = s.option(form.DummyValue, 'othernet', _('Other China ISPs'));
		o.description = _('Other China ISP entries: 5134, IPSet name: othernet');
		o.cfgvalue = function() {
			return '/etc/mwan3helper/othernet.txt';
		};

		return m.render();
	}
});
