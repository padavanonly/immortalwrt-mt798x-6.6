'use strict';
'require view';
'require form';
'require uci';
'require rpc';

var callGetModemEnv = rpc.declare({
	object: 'luci.cpe',
	method: 'get_modem_env',
	params: [],
	expect: { '': {} }
});

var callRestartModem = rpc.declare({
	object: 'luci.cpe',
	method: 'restart_modem',
	params: [],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('modem'),
			callGetModemEnv().catch(function() { return {}; })
		]);
	},

	render: function(loadResults) {
		var env = loadResults[1] || {};
		var isFm650 = Boolean(env.is_fm650);
		var devList = Array.isArray(env.devices) ? env.devices : ['/dev/cdc-wdm0'];

		var m = new form.Map('modem', _('Mobile Network'), _('Modem Server For OpenWrt'));

		if (isFm650) {
			var sFm = m.section(form.TypedSection, 'ndis', _('FMFM650-CN Settings'),
				_('[1]Automatic start upon startup: Check</br>[2] FMFMFM650-CN module default ECM (36) mode. If not, please modify it, otherwise dialing cannot proceed normally</br>'));
			sFm.anonymous = true;
			sFm.addremove = false;

			var oFmEn = sFm.option(form.Flag, 'enabled', _('Enable'));
			oFmEn.rmempty = false;
		} else {
			var s = m.section(form.TypedSection, 'ndis', _('SIM Settings'),
				_('Automatic operation upon startup ooo'));
			s.anonymous = true;
			s.addremove = false;
			s.tab('general', _('General Setup'));
			s.tab('advanced', _('Advanced Settings'));

			var enable = s.taboption('general', form.Flag, 'enabled', _('Enable'));
			enable.rmempty = false;

			var device = s.taboption('general', form.Value, 'device', _('Modem device'));
			device.rmempty = false;
			for (var i = 0; i < devList.length; i++)
				device.value(devList[i], devList[i]);

			var apn = s.taboption('general', form.Value, 'apn', _('APN'));
			apn.rmempty = true;

			var username = s.taboption('general', form.Value, 'username', _('PAP/CHAP Username'));
			username.rmempty = true;

			var password = s.taboption('general', form.Value, 'password', _('PAP/CHAP Password'));
			password.password = true;
			password.rmempty = true;

			var pincode = s.taboption('general', form.Value, 'pincode', _('PIN Code'));
			pincode.rmempty = true;

			var auth = s.taboption('general', form.Value, 'auth', _('Authentication Type'));
			auth.rmempty = true;
			auth.value('', _('-- Please choose --'));
			auth.value('both', 'PAP/CHAP (both)');
			auth.value('pap', 'PAP');
			auth.value('chap', 'CHAP');
			auth.value('none', 'NONE');

			var tool = s.taboption('general', form.Value, 'tool', _('Tools'));
			tool.value('quectel-CM', 'quectel-CM');
			tool.rmempty = true;

			var pdptype = s.taboption('general', form.Value, 'pdptype', _('PdpType'));
			pdptype.value('IPV4', 'IPV4');
			pdptype.value('IPV6', 'IPV6');
			pdptype.value('IPV4V6', 'IPV4V6');
			pdptype.rmempty = true;

			var bandlist = s.taboption('advanced', form.ListValue, 'bandlist', _('Lock Band List'));
			bandlist.default = '0';
			bandlist.value('0', _('Disable'));
			bandlist.value('1', 'LTE BAND1');
			bandlist.value('2', 'LTE BAND2');
			bandlist.value('3', 'LTE BAND3');
			bandlist.value('4', 'LTE BAND4');
			bandlist.value('5', 'LTE BAND5');
			bandlist.value('7', 'LTE BAND7');
			bandlist.value('8', 'LTE BAND8');
			bandlist.value('20', 'LTE BAND20');
			bandlist.value('28', 'LTE BAND28');
			bandlist.value('38', 'LTE BAND38');
			bandlist.value('40', 'LTE BAND40');
			bandlist.value('41', 'LTE BAND41');
			bandlist.value('A', 'AUTO');

			var servertype = s.taboption('advanced', form.ListValue, 'servertype', _('Server Type'));
			servertype.default = '0';
			servertype.value('1', '5G Only');
			servertype.value('5', '4G/5G Only');
			servertype.value('2', '4G Only');
			servertype.value('3', '3G Only');
			servertype.value('4', '2G Only');
			servertype.value('0', 'AUTO');

			var tyyusb = s.taboption('advanced', form.Value, 'tyyusb', _('tyyUSB port'), _('Set tyyUSB port'));
			tyyusb.default = '2';
			tyyusb.value('0', '0');
			tyyusb.value('1', '1');
			tyyusb.value('2', '2');
			tyyusb.value('3', '3');
			tyyusb.value('4', '4');
			tyyusb.rmempty = false;
		}

		var s2 = m.section(form.TypedSection, 'ndis', _('Network Diagnostics'),
			_('Network exception handling: check the network connection in a loop for 5 seconds. If the Ping IP address is not successful, After the network exceeds the abnormal number, restart and search the registered network again.'));
		s2.anonymous = true;
		s2.addremove = false;

		var en = s2.option(form.Flag, 'en', _('Enable'));
		en.rmempty = false;

		var ipaddress = s2.option(form.Value, 'ipaddress', _('Ping IP address'));
		ipaddress.default = '114.114.114.114';
		ipaddress.rmempty = false;

		var an = s2.option(form.Value, 'an', _('Abnormal number'));
		an.default = '15';
		an.value('3', '3');
		an.value('5', '5');
		an.value('10', '10');
		an.value('15', '15');
		an.value('20', '20');
		an.value('25', '25');
		an.value('30', '30');
		an.rmempty = false;

		return m.render();
	},

	handleSaveApply: function(ev, mode) {
		return this.super('handleSaveApply', [ev, mode]).then(function() {
			return callRestartModem().catch(function() {});
		});
	}
});
