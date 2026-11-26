'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require fs';

var callGetPorts = rpc.declare({
	object: 'luci.sms_tool',
	method: 'get_ports',
	params: [],
	expect: { '': {} }
});

var callGetUserLists = rpc.declare({
	object: 'luci.sms_tool',
	method: 'get_user_lists',
	params: [],
	expect: { '': {} }
});

var callSetUserLists = rpc.declare({
	object: 'luci.sms_tool',
	method: 'set_user_lists',
	params: ['phonebook', 'ussd', 'atcmds'],
	expect: { '': {} }
});

var callLedCtrl = rpc.declare({
	object: 'luci.sms_tool',
	method: 'led_ctrl',
	params: ['enable', 'lednotify', 'smsled', 'ledtype'],
	expect: { '': {} }
});

var USSD_FILE_PATH = '/etc/config/ussd.user';
var PHB_FILE_PATH = '/etc/config/phonebook.user';
var AT_FILE_PATH = '/etc/config/atcmds.user';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('sms_tool'),
			callGetPorts().catch(function() { return {}; }),
			callGetUserLists().catch(function() { return {}; })
		]);
	},

	populatePortOption: function(opt, ports) {
		for (var i = 0; i < ports.length; i++)
			opt.value(ports[i], ports[i]);
	},

	render: function(loadResults) {
		var self = this;
		var portData = loadResults[1] || {};
		var userLists = loadResults[2] || {};
		var ports = Array.isArray(portData.ports) ? portData.ports : ['/dev/ttyUSB0', '/dev/ttyUSB1', '/dev/ttyUSB2', '/dev/ttyUSB3'];
		var leds = Array.isArray(portData.leds) ? portData.leds : [];

		var m = new form.Map('sms_tool', _('Configuration sms-tool'),
			_('Configuration panel for sms_tool and gui application.'));

		var s = m.section(form.NamedSection, 'general', 'sms_tool', '');
		s.anonymous = true;
		s.tab('sms', _('SMS Settings'));
		s.tab('ussd', _('USSD Codes Settings'));
		s.tab('at', _('AT Commands Settings'));
		s.tab('info', _('Notification Settings'));

		// Tab: SMS Settings
		var readport = s.taboption('sms', form.Value, 'readport', _('SMS Reading Port'));
		this.populatePortOption(readport, ports);

		var storage = s.taboption('sms', form.ListValue, 'storage', _('Message storage area'),
			_('Messages are stored in a specific location (for example, on the SIM card or modem memory), but other areas may also be available depending on the type of device.'));
		storage.default = 'SM';
		storage.value('SM', _('SIM card'));
		storage.value('ME', _('Modem memory'));
		storage.rmempty = true;

		var mergesms = s.taboption('sms', form.Flag, 'mergesms', _('Merge split messages'),
			_('Checking this option will make it easier to read the messages, but it will cause a discrepancy in the number of messages shown and received.'));
		mergesms.rmempty = false;

		var algorithm = s.taboption('sms', form.ListValue, 'algorithm', _('Merge algorithm'));
		algorithm.default = 'Simple';
		algorithm.value('Simple', _('Simple (merge without sorting)'));
		algorithm.value('Advanced', _('Advanced (merges with sorting)'));
		algorithm.depends('mergesms', '1');
		algorithm.rmempty = true;

		var direction = s.taboption('sms', form.ListValue, 'direction', _('Direction of message merging'));
		direction.default = 'Start';
		direction.value('Start', _('From beginning to end'));
		direction.value('End', _('From end to beginning'));
		direction.depends('algorithm', 'Advanced');
		direction.rmempty = true;

		var sendport = s.taboption('sms', form.Value, 'sendport', _('SMS Sending Port'));
		this.populatePortOption(sendport, ports);

		var pnumber = s.taboption('sms', form.Value, 'pnumber', _('Prefix Number'),
			_("The phone number should be preceded by the country prefix (for Poland it is 48, without '+'). If the number is 5, 4 or 3 characters, it is treated as 'short' and should not be preceded by a country prefix."));
		pnumber.default = '48';
		pnumber.rmempty = true;

		var prefix = s.taboption('sms', form.Flag, 'prefix', _('Add Prefix to Phone Number'),
			_('Automatically add prefix to the phone number field.'));
		prefix.rmempty = false;

		var information = s.taboption('sms', form.Flag, 'information', _('Explanation of number and prefix'),
			_('In the tab for sending SMSes, show an explanation of the prefix and the correct phone number.'));
		information.rmempty = false;

		var phonebook = s.taboption('sms', form.TextValue, 'phonebook', _('User Phonebook'),
			_("Each line must have the following format: 'Contact name;Phone number'. Save to file '/etc/config/phonebook.user'."));
		phonebook.rows = 7;
		phonebook.rmempty = false;
		phonebook.cfgvalue = function() {
			return userLists.raw_phonebook || '';
		};
		phonebook.write = function(section_id, value) {
			var normalized = String(value || '').replace(/\r\n/g, '\n');
			userLists.raw_phonebook = normalized;
			return callSetUserLists(normalized, null, null).catch(function() {
				return fs.write(PHB_FILE_PATH, normalized);
			});
		};

		// Tab: USSD Codes Settings
		var ussdport = s.taboption('ussd', form.Value, 'ussdport', _('USSD Sending Port'));
		this.populatePortOption(ussdport, ports);

		var ussd = s.taboption('ussd', form.Flag, 'ussd', _('Sending USSD Code in plain text'),
			_('Send the USSD code in plain text. Command is not being coded to the PDU.'));
		ussd.rmempty = false;

		var pdu = s.taboption('ussd', form.Flag, 'pdu', _('Receive message without PDU decoding'),
			_('Receive and display the message without decoding it as a PDU.'));
		pdu.rmempty = false;

		var userussd = s.taboption('ussd', form.TextValue, 'userussd', _('User USSD Codes'),
			_("Each line must have the following format: 'Code name;Code'. Save to file '/etc/config/ussd.user'."));
		userussd.rows = 7;
		userussd.rmempty = true;
		userussd.cfgvalue = function() {
			return userLists.raw_ussd || '';
		};
		userussd.write = function(section_id, value) {
			var normalized = String(value || '').replace(/\r\n/g, '\n');
			userLists.raw_ussd = normalized;
			return callSetUserLists(null, normalized, null).catch(function() {
				return fs.write(USSD_FILE_PATH, normalized);
			});
		};

		// Tab: AT Commands Settings
		var atport = s.taboption('at', form.Value, 'atport', _('AT Commands Sending Port'));
		this.populatePortOption(atport, ports);

		var atlist = s.taboption('at', form.TextValue, 'atlist', _('User AT Commands'),
			_("Each line must have the following format: 'AT Command name;AT Command'. Save to file '/etc/config/atcmds.user'."));
		atlist.rows = 20;
		atlist.rmempty = true;
		atlist.cfgvalue = function() {
			return userLists.raw_atcmds || '';
		};
		atlist.write = function(section_id, value) {
			var normalized = String(value || '').replace(/\r\n/g, '\n');
			userLists.raw_atcmds = normalized;
			return callSetUserLists(null, null, normalized).catch(function() {
				return fs.write(AT_FILE_PATH, normalized);
			});
		};

		// Tab: Notification Settings
		var lednotify = s.taboption('info', form.Flag, 'lednotify', _('Notify new messages'),
			_('The LED informs about a new message. Before activating this function, please config and save the SMS reading port, time to check SMS inbox and select the notification LED.'));
		lednotify.rmempty = false;
		lednotify.write = function(section_id, value) {
			uci.set('sms_tool', section_id, 'lednotify', value);
			var ledOpt = this.section.children.filter(function(o) { return o.option == 'smsled'; })[0];
			var typeOpt = this.section.children.filter(function(o) { return o.option == 'ledtype'; })[0];
			var curLed = (ledOpt && ledOpt.formvalue(section_id)) || uci.get('sms_tool', section_id, 'smsled') || '';
			var curType = (typeOpt && typeOpt.formvalue(section_id)) || uci.get('sms_tool', section_id, 'ledtype') || 'D';
			return callLedCtrl(String(value), String(value), curLed, curType);
		};

		var checktime = s.taboption('info', form.Value, 'checktime', _('Check inbox every minute(s)'),
			_('Specify how many minutes you want your inbox to be checked.'));
		checktime.rmempty = false;
		checktime.maxlength = 2;
		checktime.default = '5';
		checktime.validate = function(section_id, value) {
			var n = Number(value);
			if (!isNaN(n) && n > 0 && n < 60)
				return true;
			return _('Specify a value between 1 and 59');
		};

		var prestart = s.taboption('info', form.ListValue, 'prestart', _('Restart the inbox checking process every'),
			_('The process will restart at the selected time interval. This will eliminate the delay in checking your inbox.'));
		prestart.default = '6';
		prestart.value('4', _('4h'));
		prestart.value('6', _('6h'));
		prestart.value('8', _('8h'));
		prestart.value('12', _('12h'));
		prestart.rmempty = true;

		var smsled = s.taboption('info', form.Value, 'smsled', _('Notification LED'),
			_('Select the notification LED.'));
		for (var j = 0; j < leds.length; j++)
			smsled.value(leds[j], leds[j]);

		var ledtype = s.taboption('info', form.ListValue, 'ledtype', _('The diode is dedicated only to these notifications'),
			_("Select 'No' in case the router has only one LED or if the LED is multi-tasking."));
		ledtype.default = 'D';
		ledtype.value('S', _('No'));
		ledtype.value('D', _('Yes'));
		ledtype.rmempty = true;

		var ledtimeon = s.taboption('info', form.Value, 'ledtimeon', _('Turn on the LED for seconds(s)'),
			_('Specify for how long the LED should be on.'));
		ledtimeon.rmempty = false;
		ledtimeon.maxlength = 3;
		ledtimeon.default = '1';

		var ledtimeoff = s.taboption('info', form.Value, 'ledtimeoff', _('Turn off the LED for seconds(s)'),
			_('Specify for how long the LED should be off.'));
		ledtimeoff.rmempty = false;
		ledtimeoff.maxlength = 3;
		ledtimeoff.default = '5';

		return m.render();
	}
});
