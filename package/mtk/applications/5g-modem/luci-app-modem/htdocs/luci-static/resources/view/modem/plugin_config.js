'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require poll';
'require ui';

var callModemScan = rpc.declare({
	object: 'luci.modem',
	method: 'modemScan',
	params: ['translation'],
	expect: { '': {} }
});

var SUPPORTED_MODEMS = [
	['rm500q', 'Quectel RM500Q'],
	['rm500u', 'Quectel RM500U'],
	['rm520n', 'Quectel RM520N'],
	['rm530n', 'Quectel RM530N'],
	['rg500q', 'Quectel RG500Q'],
	['rg500u', 'Quectel RG500U'],
	['rg520n', 'Quectel RG520N'],
	['ec20', 'Quectel EC20'],
	['ec200t', 'Quectel EC200T'],
	['em12g', 'Quectel EM12G'],
	['ep06e', 'Quectel EP06E'],
	['fm350-gl', 'Fibocom FM350-GL'],
	['fm650-cn', 'Fibocom FM650-CN'],
	['fm150-ae', 'Fibocom FM150-AE'],
	['fg160-cn', 'Fibocom FG160-CN'],
	['fg650-cn', 'Fibocom FG650-CN'],
	['nl668', 'Fibocom NL668'],
	['l850-gl', 'Fibocom L850-GL'],
	['l860-gl', 'Fibocom L860-GL'],
	['srm815', 'Meig SRM815'],
	['srm825n', 'Meig SRM825N'],
	['slm820', 'Meig SLM820'],
	['slm750', 'Meig SLM750'],
	['mh5000', 'Huawei MH5000'],
	['me909s', 'Huawei ME909S'],
	['mt5710', 'Tdtech MT5710'],
	['sim8200ea', 'Simcom SIM8200EA'],
	['sim7600g', 'Simcom SIM7600G']
];

return view.extend({
	load: function() {
		return uci.load('modem');
	},

	render: function() {
		var m = new form.Map('modem', _('Plugin Config'), _('Check and modify the plugin configuration'));

		var sGlobal = m.section(form.TypedSection, 'global', _('Global Config'));
		sGlobal.anonymous = true;
		sGlobal.addremove = false;

		var oScan = sGlobal.option(form.Button, 'modem_scan', _('Modem Scan'),
			_('The automatic configuration modem is triggered only at modem startup, otherwise, manual scanning is necessary'));
		oScan.inputtitle = _('Scan');
		oScan.inputstyle = 'apply';
		oScan.onclick = function() {
			ui.showModal(_('Modem Scan'), [
				E('p', { 'class': 'spinning' }, _('Scaning modem...'))
			]);
			return callModemScan('1').then(function() {
				ui.hideModal();
				window.location.reload();
			}).catch(function(err) {
				ui.hideModal();
				ui.addNotification(null, E('p', {}, _('Modem Scan') + ': ' + String(err)));
			});
		};

		var oManualCfg = sGlobal.option(form.Flag, 'manual_configuration',
			_('Manual Configuration'),
			_('Enable the manual configuration of modem information') + ' ' +
			_('(After enable, the automatic scanning and configuration function for modem information will be disabled)'));
		oManualCfg.rmempty = false;
		oManualCfg.default = '0';

		var oAlias = sGlobal.option(form.Flag, 'network_interface_alias',
			_('Network Interface Alias'),
			_('Enable automatic creation of network interface alias (Effective upon reinstatement of dialing configuration)'));
		oAlias.rmempty = false;
		oAlias.default = '1';

		var sModem = m.section(form.GridSection, 'modem-device', _('Modem Config'));
		sModem.anonymous = true;
		sModem.addremove = true;
		sModem.sortable = true;
		sModem.modaltitle = _('Modem Config');

		sModem.handleCreate = function(ev) {
			var existing = uci.sections('modem', 'modem-device') || [];
			var nextName = 'modem' + existing.length;
			var sid = uci.add('modem', 'modem-device', nextName);
			uci.set('modem', sid, 'manual', '1');
			return this.renderMoreOptionsModal(sid);
		};

		var oManual = sModem.option(form.Flag, 'manual', _('Manual'));
		oManual.default = '1';
		oManual.rmempty = false;
		oManual.modalonly = true;

		var oDataIf = sModem.option(form.ListValue, 'data_interface', _('Data Interface'));
		oDataIf.default = 'usb';
		oDataIf.rmempty = true;
		oDataIf.value('usb', 'USB');
		oDataIf.value('pcie', 'PCIe');

		var oNetwork = sModem.option(form.Value, 'network', _('Mobile Network'));
		oNetwork.rmempty = true;
		oNetwork.placeholder = _('Not null');
		['usb0', 'usb1', 'wwan0', 'wwan0_1', 'eth1', 'eth2', 'mhi_hwip0', 'rmnet_mhi0', 'rmnet_mhi0.1'].forEach(function(net) {
			oNetwork.value(net, net);
		});

		var oName = sModem.option(form.Value, 'name', _('Modem Name'));
		oName.placeholder = _('Not null');
		oName.rmempty = false;
		for (var i = 0; i < SUPPORTED_MODEMS.length; i++) {
			oName.value(SUPPORTED_MODEMS[i][0], SUPPORTED_MODEMS[i][1]);
		}
		oName.textvalue = function(section_id) {
			var v = uci.get('modem', section_id, 'name') || '';
			return _(v.toUpperCase());
		};

		var oManufacturer = sModem.option(form.ListValue, 'manufacturer', _('Manufacturer'));
		oManufacturer.rmempty = true;
		oManufacturer.modalonly = true;
		oManufacturer.value('quectel', 'Quectel');
		oManufacturer.value('fibocom', 'Fibocom');
		oManufacturer.value('meig', 'Meig');
		oManufacturer.value('huawei', 'Huawei');
		oManufacturer.value('tdtech', 'TD Tech');
		oManufacturer.value('simcom', 'SIMCom');
		oManufacturer.value('unknown', _('Unknown'));

		var oPlatform = sModem.option(form.ListValue, 'platform', _('Platform'));
		oPlatform.rmempty = true;
		oPlatform.modalonly = true;
		oPlatform.value('qualcomm', 'Qualcomm');
		oPlatform.value('unisoc', 'UNISOC');
		oPlatform.value('mediatek', 'MediaTek');
		oPlatform.value('intel', 'Intel');
		oPlatform.value('hisilicon', 'HiSilicon');
		oPlatform.value('lte', 'LTE');
		oPlatform.value('unknown', _('Unknown'));

		var oDefConnect = sModem.option(form.ListValue, 'define_connect', _('Define Connect'));
		oDefConnect.rmempty = true;
		oDefConnect.modalonly = true;
		oDefConnect.value('1', 'PDP Context 1');
		oDefConnect.value('3', 'PDP Context 3');

		var oModes = sModem.option(form.DynamicList, 'modes', _('Mode'));
		oModes.rmempty = true;
		oModes.modalonly = true;
		['qmi', 'gobinet', 'ecm', 'mbim', 'rndis', 'ncm'].forEach(function(md) {
			oModes.value(md, md.toUpperCase());
		});

		var oAtPort = sModem.option(form.Value, 'at_port', _('AT Port'));
		oAtPort.placeholder = _('Not null');
		oAtPort.rmempty = false;
		['/dev/ttyUSB2', '/dev/ttyUSB3', '/dev/ttyACM0', '/dev/mhi_DUN', '/dev/wwan0at0', '/dev/wwan0at1'].forEach(function(p) {
			oAtPort.value(p, p);
		});

		var oGpsPort = sModem.option(form.Value, 'gps_port', _('GPS Port'));
		oGpsPort.rmempty = true;
		oGpsPort.modalonly = true;

		var oNetLed = sModem.option(form.Value, 'net_led', _('Net LED'));
		oNetLed.rmempty = true;
		oNetLed.modalonly = true;

		return m.render();
	}
});
