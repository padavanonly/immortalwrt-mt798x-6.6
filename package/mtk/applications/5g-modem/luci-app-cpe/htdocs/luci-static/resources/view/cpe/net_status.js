'use strict';
'require view';
'require rpc';
'require poll';

var callGetCsq = rpc.declare({
	object: 'luci.cpe',
	method: 'get_csq',
	params: [],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return callGetCsq().catch(function() {
			return {};
		});
	},

	updateTelemetry: function(rv) {
		if (!rv || typeof rv !== 'object')
			return;

		var fields = [
			['date', rv.date],
			['csq', rv.csq],
			['per', rv.per],
			['rssi', rv.rssi],
			['modem', rv.modem],
			['cops', rv.cops],
			['mode', rv.mode],
			['lac', rv.lac],
			['cid', rv.cid],
			['lacn', rv.lacn],
			['cidn', rv.cidn],
			['mcc', rv.mcc],
			['mnc', rv.mnc],
			['rnc', rv.rnc],
			['rncn', rv.rncn],
			['down', rv.down],
			['up', rv.up],
			['ecio', rv.ecio],
			['rscp', rv.rscp],
			['ecio1', rv.ecio1],
			['rscp1', rv.rscp1],
			['conntype', rv.conntype],
			['chan', rv.channel],
			['lband', rv.lband],
			['conmon', rv.netmode],
			['tempur', rv.tempur],
			['pci', rv.pci],
			['sinr', rv.sinr],
			['imei', rv.imei],
			['imsi', rv.imsi],
			['iccid', rv.iccid],
			['phone', rv.phone],
			['lat', rv.lat],
			['long', rv.long]
		];

		for (var i = 0; i < fields.length; i++) {
			var el = document.getElementById(fields[i][0]);
			if (el) {
				var val = fields[i][1];
				el.innerHTML = (val !== undefined && val !== null) ? String(val) : '-';
			}
		}

		var gpsSec = document.getElementById('cbi-gps');
		if (gpsSec)
			gpsSec.style.display = (Number(rv.havegps) === 1) ? 'block' : 'none';

		var simWarn = document.getElementById('simwarn');
		var simMsg = document.getElementById('simsg');
		if (simWarn && simMsg) {
			var simerr = String(rv.simerr || '0');
			if (simerr === '0') {
				simWarn.style.display = 'none';
			} else {
				simWarn.style.display = 'block';
				simMsg.style.color = 'red';
				if (simerr === '1')
					simMsg.textContent = _('SIM card is locked and no SIM PIN is configured in profile!');
				else if (simerr === '2')
					simMsg.textContent = _('Incorrect PIN to unlock SIM card');
				else if (simerr === '3')
					simMsg.textContent = _('Invalid SIM card');
				else
					simMsg.textContent = _('SIM card is not locked. Invalid SIM card');
			}
		}
	},

	render: function(initialData) {
		var self = this;
		var rv = initialData || {};
		var haveGps = (Number(rv.havegps) === 1);

		var root = E('div', { 'class': 'cbi-map', 'id': 'cbi-modem' }, [
			E('h2', { 'id': 'content', 'name': 'content' }, _('Signal Status / Modem Information')),
			E('div', { 'class': 'cbi-map-descr' },
				_('Please note that not all 5G modems are supported by this plugin.')),

			E('fieldset', { 'class': 'cbi-section', 'id': 'simwarn', 'style': 'display:none;' }, [
				E('legend', {}, _('SIM Warning')),
				E('table', { 'width': '550', 'border': '0' }, [
					E('tbody', {}, [
						E('tr', {}, [
							E('td', { 'width': '10%' }),
							E('td', { 'width': '60%' }, [
								E('div', { 'align': 'left', 'id': 'simsg', 'style': 'font-size:1.875em;font-weight:bold;' })
							]),
							E('td', { 'width': '30%' })
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-mod' }, [
				E('legend', {}, _('General Information')),
				E('table', { 'width': '100%', 'cellspacing': '10', 'class': 'table' }, [
					E('tbody', {}, [
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Modem: ')),
							E('td', { 'class': 'td left', 'id': 'modem' }, rv.modem || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Manufacturer: ')),
							E('td', { 'class': 'td left', 'id': 'conntype' }, rv.conntype || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Temperature: ')),
							E('td', { 'class': 'td left', 'id': 'tempur' }, rv.tempur || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Update Time: ')),
							E('td', { 'class': 'td left', 'id': 'date' }, rv.date || '-'),
							E('td', { 'class': 'td left' })
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-msinfo' }, [
				E('legend', {}, _('Cellular Module / SIM Card Information')),
				E('table', { 'width': '100%', 'cellspacing': '10', 'class': 'table' }, [
					E('tbody', {}, [
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Carrier: ')),
							E('td', { 'class': 'td left', 'id': 'cops' }, rv.cops || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('IMEI: ')),
							E('td', { 'class': 'td left', 'id': 'imei' }, rv.imei || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('IMSI: ')),
							E('td', { 'class': 'td left', 'id': 'imsi' }, rv.imsi || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('ICCID: ')),
							E('td', { 'class': 'td left', 'id': 'iccid' }, rv.iccid || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('SIM Card Number: ')),
							E('td', { 'class': 'td left', 'id': 'phone' }, rv.phone || '-'),
							E('td', { 'class': 'td left' })
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-sig' }, [
				E('legend', {}, _('Signal Status')),
				E('table', { 'width': '100%', 'cellspacing': '10', 'class': 'table' }, [
					E('tbody', {}, [
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Cellular Network Type: ')),
							E('td', { 'class': 'td left', 'id': 'mode' }, rv.mode || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('CSQ: ')),
							E('td', { 'class': 'td left', 'id': 'csq' }, rv.csq || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Signal Strength: ')),
							E('td', { 'class': 'td left', 'id': 'per' }, rv.per || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Received Signal Strength Indicator (RSSI): ')),
							E('td', { 'class': 'td left', 'id': 'rssi' }, rv.rssi || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Reference Signal Received Quality (RSRQ): ')),
							E('td', { 'class': 'td left' }, [
								E('span', { 'id': 'ecio', 'class': 'r' }, rv.ecio || '-'),
								E('span', { 'id': 'ecio1', 'class': 'r' }, rv.ecio1 || '')
							]),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Reference Signal Received Power (RSRP): ')),
							E('td', { 'class': 'td left' }, [
								E('span', { 'id': 'rscp', 'class': 'r' }, rv.rscp || '-'),
								E('span', { 'id': 'rscp1', 'class': 'r' }, rv.rscp1 || '')
							]),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Signal-to-Noise Ratio (SINR): ')),
							E('td', { 'class': 'td left', 'id': 'sinr' }, rv.sinr || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Connection Status Monitor: ')),
							E('td', { 'class': 'td left', 'id': 'conmon' }, rv.netmode || '-'),
							E('td', { 'class': 'td left' })
						])
					])
				])
			]),

			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-cell' }, [
				E('legend', {}, _('Cell Station Information')),
				E('table', { 'width': '100%', 'cellspacing': '10', 'class': 'table' }, [
					E('tbody', {}, [
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('MCC / MNC: ')),
							E('td', { 'class': 'td left', 'id': 'mcc' }, rv.mcc || '-'),
							E('td', { 'class': 'td left', 'id': 'mnc' }, rv.mnc || '-')
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('eNB ID: ')),
							E('td', { 'class': 'td left' }, [
								E('span', { 'id': 'rnc', 'class': 'r' }, rv.rnc || '-'),
								E('span', { 'id': 'rncn', 'class': 'r' }, rv.rncn || '')
							]),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('TAC: ')),
							E('td', { 'class': 'td left' }, [
								E('span', { 'id': 'lac', 'class': 'r' }, rv.lac || '-'),
								E('span', { 'id': 'lacn', 'class': 'r' }, rv.lacn || '')
							]),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Cell ID: ')),
							E('td', { 'class': 'td left' }, [
								E('span', { 'id': 'cid', 'class': 'r' }, rv.cid || '-'),
								E('span', { 'id': 'cidn', 'class': 'r' }, rv.cidn || '')
							]),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Band: ')),
							E('td', { 'class': 'td left', 'id': 'lband' }, rv.lband || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Channel: ')),
							E('td', { 'class': 'td left', 'id': 'chan' }, rv.channel || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Physical Cell ID (PCI): ')),
							E('td', { 'class': 'td left', 'id': 'pci' }, rv.pci || '-'),
							E('td', { 'class': 'td left' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'width': '20%' }, _('Maximum QoS Level: ')),
							E('td', { 'class': 'td left' }, [
								E('span', { 'id': 'down', 'class': 'r' }, rv.down || '-'),
								E('span', { 'id': 'up', 'class': 'r' }, rv.up || '')
							]),
							E('td', { 'class': 'td left' })
						])
					])
				])
			]),

			E('fieldset', {
				'class': 'cbi-section',
				'id': 'cbi-gps',
				'style': haveGps ? 'display:block;' : 'display:none;'
			}, [
				E('legend', {}, _('GPS Location')),
				E('table', { 'width': '550', 'border': '0', 'class': 'table' }, [
					E('tbody', {}, [
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td right', 'width': '30%' }, _('Latitude: ')),
							E('td', { 'class': 'td left', 'id': 'lat' }, rv.lat || '-'),
							E('td', { 'class': 'td left', 'width': '1%' })
						]),
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td right', 'width': '30%' }, _('Longitude: ')),
							E('td', { 'class': 'td left', 'id': 'long' }, rv.long || '-'),
							E('td', { 'class': 'td left', 'width': '1%' })
						])
					])
				])
			])
		]);

		poll.add(function() {
			return callGetCsq().then(function(data) {
				self.updateTelemetry(data);
			});
		}, 2);

		return root;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
