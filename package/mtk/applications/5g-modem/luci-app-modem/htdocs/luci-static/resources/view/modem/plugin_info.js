'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require poll';
'require ui';

var callGetPluginInfo = rpc.declare({
	object: 'luci.modem',
	method: 'getPluginInfo',
	params: ['translation'],
	expect: { '': {} }
});

var DIAL_TOOLS = [
	{ key: 'modemmanager', label: 'modemmanager' },
	{ key: 'quectel-CM-5G', label: 'quectel-CM-5G' },
	{ key: 'fibocom-dial', label: 'fibocom-dial' },
	{ key: 'meig-cm', label: 'meig-cm' },
	{ key: 'sms_tool_q', label: 'sms_tool_q' },
	{ key: 'atinout', label: 'atinout' },
	{ key: 'sendat', label: 'sendat' }
];

var GENERAL_DRIVERS = [
	{ type: 'USB Network', kmod: 'kmod-usb-net', ko: 'usbnet.ko' },
	{ type: 'Serial Port', kmod: 'kmod-usb-serial-option', ko: 'option.ko' },
	{ type: 'Serial Port', kmod: 'kmod-usb-serial-qualcomm', ko: 'qcserial.ko' }
];

var USB_DRIVERS = [
	{ type: 'General', mode: 'QMI', kmod: 'kmod-usb-net-qmi-wwan', ko: 'qmi_wwan.ko' },
	{ type: 'General', mode: 'GobiNet', kmod: 'kmod-usb-net-gobinet', ko: 'GobiNet.ko' },
	{ type: 'General', mode: 'ECM', kmod: 'kmod-usb-net-cdc-ether', ko: 'cdc_ether.ko' },
	{ type: 'General', mode: 'MBIM', kmod: 'kmod-usb-net-cdc-mbim', ko: 'cdc_mbim.ko' },
	{ type: 'General', mode: 'RNDIS', kmod: 'kmod-usb-net-rndis', ko: 'rndis_host.ko' },
	{ type: 'General', mode: 'NCM', kmod: 'kmod-usb-net-cdc-ncm', ko: 'cdc_ncm.ko' },
	{ type: 'Private (Quectel)', mode: 'QMI', kmod: 'kmod-usb-net-qmi-wwan-q', ko: 'qmi_wwan_q.ko' },
	{ type: 'Private (Fibocom)', mode: 'QMI', kmod: 'kmod-usb-net-qmi-wwan-f', ko: 'qmi_wwan_f.ko' },
	{ type: 'Private (Meig)', mode: 'NCM', kmod: 'kmod-usb-net-meig-cdc', ko: 'meig_cdc_driver.ko' }
];

var PCIE_DRIVERS = [
	{ type: 'General', kmod: 'kmod-mhi-net', ko: 'mhi_net.ko' },
	{ type: 'General', kmod: 'kmod-qrtr-mhi', ko: 'qrtr_mhi.ko' },
	{ type: 'General', kmod: 'kmod-mhi-bus', ko: 'mhi_pci_generic.ko' },
	{ type: 'General', kmod: 'kmod-mhi-wwan-mbim', ko: 'mhi_wwan_mbim.ko' },
	{ type: 'General', kmod: 'kmod-mhi-wwan-ctrl', ko: 'mhi_wwan_ctrl.ko' },
	{ type: 'Private', kmod: 'kmod-pcie_mhi', ko: 'pcie_mhi.ko' },
	{ type: 'Private', kmod: 'kmod-mtk-t7xx', ko: 'mtk_pcie_wwan_m80.ko' }
];

function renderVersionSpan(versionVal, translation) {
	var v = versionVal || 'Unknown';
	var color = 'green';
	var text = 'v' + v;
	if (v === 'Unknown') {
		color = 'goldenrod';
		text = (translation && translation[v]) ? _(translation[v]) : _('Unknown');
	} else if (v === 'Not installed') {
		color = 'red';
		text = (translation && translation[v]) ? _(translation[v]) : _('Not installed');
	}
	return E('strong', { 'style': 'color:' + color + ';' }, text);
}

function renderStatusSpan(statusVal, translation) {
	var st = statusVal || 'Not loaded';
	var color = (st === 'Loaded') ? 'green' : 'red';
	var text = (translation && translation[st]) ? _(translation[st]) : _(st);
	return E('span', { 'style': 'color:' + color + ';font-weight:bold;' }, text);
}

return view.extend({
	load: function() {
		return callGetPluginInfo('1').catch(function() { return {}; });
	},

	updateTables: function(data) {
		var translation = (data && data.translation) ? data.translation : {};
		var pluginInfo = (data && data.plugin_info) ? data.plugin_info : {};
		var dialToolInfo = (data && data.dial_tool_info) ? data.dial_tool_info : {};
		var generalInfo = (data && data.general_driver_info) ? data.general_driver_info : {};
		var usbInfo = (data && data.usb_driver_info) ? data.usb_driver_info : {};
		var pcieInfo = (data && data.pcie_driver_info) ? data.pcie_driver_info : {};

		var appVerEl = document.getElementById('luci-app-modem_version');
		if (appVerEl) {
			while (appVerEl.firstChild)
				appVerEl.removeChild(appVerEl.firstChild);
			appVerEl.appendChild(renderVersionSpan(pluginInfo['luci-app-modem'] || data.plugin_version, translation));
		}

		DIAL_TOOLS.forEach(function(tool) {
			var el = document.getElementById(tool.key + '_version');
			if (el) {
				while (el.firstChild)
					el.removeChild(el.firstChild);
				el.appendChild(renderVersionSpan(dialToolInfo[tool.key] || 'Not installed', translation));
			}
		});

		var allDrivers = [].concat(GENERAL_DRIVERS, USB_DRIVERS, PCIE_DRIVERS);
		var mergedDriverMap = Object.assign({}, generalInfo, usbInfo, pcieInfo);
		allDrivers.forEach(function(drv) {
			var id = drv.ko.toLowerCase().replace('.ko', '_status');
			var el = document.getElementById(id);
			if (el) {
				while (el.firstChild)
					el.removeChild(el.firstChild);
				var st = mergedDriverMap[drv.ko] || mergedDriverMap[drv.kmod] || 'Not loaded';
				el.appendChild(renderStatusSpan(st, translation));
			}
		});
	},

	render: function(initialData) {
		var self = this;

		var dialToolRows = DIAL_TOOLS.map(function(tool) {
			return E('div', { 'class': 'cbi-value' }, [
				E('label', { 'class': 'cbi-value-title' }, tool.label + ' ' + _('Version')),
				E('div', { 'class': 'cbi-value-field' }, [
					E('div', { 'class': 'version', 'id': tool.key + '_version' }, '-')
				])
			]);
		});

		var generalRows = [
			E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Driver Type')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Kernel Model')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Status'))
			])
		].concat(GENERAL_DRIVERS.map(function(drv, idx) {
			var statusId = drv.ko.toLowerCase().replace('.ko', '_status');
			return E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-' + ((idx % 2) + 1) }, [
				E('td', { 'class': 'td cbi-value-field' }, _(drv.type)),
				E('td', { 'class': 'td cbi-value-field', 'title': drv.kmod }, drv.ko),
				E('td', { 'class': 'td cbi-value-field', 'id': statusId }, '-')
			]);
		}));

		var usbRows = [
			E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Driver Type')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Mode')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Kernel Model')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Status'))
			])
		].concat(USB_DRIVERS.map(function(drv, idx) {
			var statusId = drv.ko.toLowerCase().replace('.ko', '_status');
			return E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-' + ((idx % 2) + 1) }, [
				E('td', { 'class': 'td cbi-value-field' }, _(drv.type)),
				E('td', { 'class': 'td cbi-value-field' }, drv.mode),
				E('td', { 'class': 'td cbi-value-field', 'title': drv.kmod }, drv.ko),
				E('td', { 'class': 'td cbi-value-field', 'id': statusId }, '-')
			]);
		}));

		var pcieRows = [
			E('tr', { 'class': 'tr cbi-section-table-titles anonymous' }, [
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Driver Type')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Kernel Model')),
				E('th', { 'class': 'th cbi-section-table-cell' }, _('Status'))
			])
		].concat(PCIE_DRIVERS.map(function(drv, idx) {
			var statusId = drv.ko.toLowerCase().replace('.ko', '_status');
			return E('tr', { 'class': 'tr cbi-section-table-row cbi-rowstyle-' + ((idx % 2) + 1) }, [
				E('td', { 'class': 'td cbi-value-field' }, _(drv.type)),
				E('td', { 'class': 'td cbi-value-field', 'title': drv.kmod }, drv.ko),
				E('td', { 'class': 'td cbi-value-field', 'id': statusId }, '-')
			]);
		}));

		var root = E('div', { 'class': 'cbi-map', 'id': 'cbi-modem-plugin-info' }, [
			E('h2', { 'id': 'content', 'name': 'content' }, _('Plugin Info')),
			E('div', { 'class': 'cbi-map-descr' }, _('Check the version information of the plugin')),

			E('fieldset', { 'class': 'cbi-section' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Plugin Info')),
					E('div', { 'class': 'cbi-section-node' }, [
						E('div', { 'class': 'cbi-value cbi-value-last' }, [
							E('label', { 'class': 'cbi-value-title' }, _('Plugin Version')),
							E('div', { 'class': 'cbi-value-field' }, [
								E('div', { 'class': 'version', 'id': 'luci-app-modem_version' }, '-')
							])
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Dial Tool Info')),
					E('div', { 'class': 'cbi-section-node' }, dialToolRows)
				])
			]),

			E('fieldset', { 'class': 'cbi-section' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Modem General Driver Info')),
					E('table', { 'class': 'table cbi-section-table' }, [
						E('tbody', {}, generalRows)
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Modem USB Driver Info')),
					E('table', { 'class': 'table cbi-section-table' }, [
						E('tbody', {}, usbRows)
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section' }, [
				E('div', { 'class': 'cbi-section fade-in' }, [
					E('h3', {}, _('Modem PCIE Driver Info')),
					E('table', { 'class': 'table cbi-section-table' }, [
						E('tbody', {}, pcieRows)
					])
				])
			])
		]);

		setTimeout(function() {
			self.updateTables(initialData || {});
		}, 0);

		poll.add(function() {
			return callGetPluginInfo('1').then(function(res) {
				self.updateTables(res || {});
			});
		}, 10);

		return root;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
