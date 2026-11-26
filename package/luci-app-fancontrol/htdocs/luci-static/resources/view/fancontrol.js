/* Copyright (C) 2026 ImmortalWrt.org */

'use strict';
'require dom';
'require form';
'require poll';
'require rpc';
'require ui';
'require view';

var callGetStatus = rpc.declare({
	object: 'luci.fancontrol',
	method: 'getStatus',
	expect: { '': {} }
});

var callSetMode = rpc.declare({
	object: 'luci.fancontrol',
	method: 'setMode',
	params: [ 'mode', 'pwm' ],
	expect: { '': {} }
});

var callSetTempSource = rpc.declare({
	object: 'luci.fancontrol',
	method: 'setTempSource',
	params: [ 'source' ],
	expect: { '': {} }
});

var callApplyConfig = rpc.declare({
	object: 'luci.fancontrol',
	method: 'applyConfig',
	params: [
		'mode', 'pwm', 'source',
		'inverted', 'start_temp', 'full_temp',
		'min_pwm', 'max_pwm', 'hysteresis',
		'slew_rate', 'wps_turbo'
	],
	expect: { '': {} }
});

var MODE_LABELS = {
	'stop': _('Silent Mode'),
	'low': _('Low Speed Mode'),
	'normal': _('Normal Mode'),
	'turbo': _('Turbo Mode'),
	'smart': _('Smart Thermal Mode'),
	'kernel': _('Kernel Thermal Mode'),
	'stepless': _('Stepless Mode')
};

var SOURCE_LABELS = {
	'cpu': _('CPU Temperature'),
	'modem': _('5G Modem Temperature'),
	'max': _('Max (CPU & 5G Modem)')
};

function formatTemp(val) {
	return (val != null) ? (val + ' °C') : '--';
}

function formatDriverText(res) {
	var drv = res.driver_label || res.driver || '--';
	if (res.driver && res.driver !== 'none') {
		var pol = res.inverted_effective ? _('Inverted') : _('Normal');
		return drv + ' (' + pol + ')';
	}
	return drv;
}

function updateStatusDOM(res) {
	if (!res)
		return;

	var elTemp = document.getElementById('fan-temp-val');
	var elCpuTemp = document.getElementById('fan-cpu-temp-val');
	var elModemTemp = document.getElementById('fan-modem-temp-val');
	var elSpeed = document.getElementById('fan-speed-val');
	var elRpm = document.getElementById('fan-rpm-val');
	var elMode = document.getElementById('fan-mode-val');
	var elSrc = document.getElementById('fan-src-val');
	var elDriver = document.getElementById('fan-driver-val');
	var elErr = document.getElementById('fan-error-row');
	var elErrVal = document.getElementById('fan-error-val');

	if (elTemp)
		elTemp.textContent = formatTemp(res.temp);
	if (elCpuTemp)
		elCpuTemp.textContent = formatTemp(res.cpu_temp);
	if (elModemTemp)
		elModemTemp.textContent = formatTemp(res.modem_temp);
	if (elSpeed) {
		elSpeed.textContent = (res.speed_pct != null)
			? (res.speed_pct + ' % (PWM ' + (res.pwm != null ? res.pwm : '--') + '/255)')
			: '--';
	}
	if (elRpm)
		elRpm.textContent = (res.rpm != null) ? (res.rpm + ' RPM') : '--';
	if (elMode && res.mode != null)
		elMode.textContent = MODE_LABELS[res.mode] || res.mode_label || res.mode;
	if (elSrc) {
		var srcKey = res.source_used || res.temp_source;
		elSrc.textContent = srcKey ? (SOURCE_LABELS[srcKey] || srcKey) : '--';
	}
	if (elDriver)
		elDriver.textContent = formatDriverText(res);
	if (elErr && elErrVal) {
		if (res.error) {
			elErr.style.display = '';
			elErrVal.textContent = res.error;
		}
		else {
			elErr.style.display = 'none';
			elErrVal.textContent = '';
		}
	}
}

function dismissBannerElement(msg) {
	if (!msg)
		return;
	msg.classList.add('fade-out');
	msg.classList.remove('fade-in');
	window.setTimeout(function() {
		if (msg.parentNode)
			msg.parentNode.removeChild(msg);
	}, 180);
}

function showBanner(title, children, timeoutMs, ...classes) {
	var msg = (timeoutMs && timeoutMs > 0)
		? ui.addTimeLimitedNotification(title, children, timeoutMs, ...classes)
		: ui.addNotification(title, children, ...classes);
	if (msg) {
		msg.querySelectorAll('.btn').forEach(function(btn) {
			btn.addEventListener('click', function() {
				dismissBannerElement(msg);
			});
		});
	}
	return msg;
}

var activeView = null;
var pollGeneration = 0;

function pollFanStatus() {
	var viewSelf = activeView;
	var generation = pollGeneration;
	if (!viewSelf || viewSelf.applyInProgress || !document.getElementById('fan-temp-val'))
		return Promise.resolve();

	return callGetStatus().then(function(res) {
		if (!res || res.code !== 0)
			throw new Error((res && res.error) ? res.error : 'Invalid status response');
		if (activeView !== viewSelf || viewSelf.applyInProgress || generation !== pollGeneration)
			return;

		updateStatusDOM(res);

		if (res.lucimsg)
			showBanner(null, E('p', _(res.lucimsg)), 3500, 'info');
	});
}

function intOrDefault(val, defVal) {
	var n = parseInt(val, 10);
	return isNaN(n) ? defVal : n;
}

return view.extend({
	load: function() {
		return callGetStatus();
	},

	render: function(status) {
		status = status || {};
		activeView = this;
		this.initialStatus = status;

		var steplessVal = (status.stepless_pwm != null) ? status.stepless_pwm : 192;
		this.formData = {
			cooling_control: {
				mode: status.mode || 'smart',
				stepless_pwm: String(steplessVal),
				temp_source: status.temp_source || 'cpu'
			},
			thermal_curve: {
				start_temp: String(status.start_temp != null ? status.start_temp : 40),
				full_temp: String(status.full_temp != null ? status.full_temp : 75),
				min_pwm: String(status.min_pwm != null ? status.min_pwm : 64),
				max_pwm: String(status.max_pwm != null ? status.max_pwm : 255),
				hysteresis: String(status.hysteresis != null ? status.hysteresis : 2),
				slew_rate: String(status.slew_rate != null ? status.slew_rate : 12),
				inverted: String(status.inverted_cfg || 'auto'),
				wps_turbo: (status.wps_turbo === 1 || status.wps_turbo === '1') ? '1' : '0'
			}
		};

		var m = new form.JSONMap(
			this.formData,
			_('Fan Control'),
			_('Configure cooling fan operating mode, temperature sensor source, and hardware thermal curve.')
		);
		this.map = m;

		var s = m.section(form.NamedSection, '__status', '_status', _('Realtime Status'));
		s.render = function() {
			var curSpeed = (status.speed_pct != null)
				? (status.speed_pct + ' % (PWM ' + (status.pwm != null ? status.pwm : '--') + '/255)')
				: '--';
			var curMode = (status.mode && MODE_LABELS[status.mode])
				? MODE_LABELS[status.mode]
				: (status.mode_label || status.mode || '--');
			var srcKey = status.source_used || status.temp_source;
			var curSrc = srcKey ? (SOURCE_LABELS[srcKey] || srcKey) : '--';

			var statusTable = E('table', { 'class': 'table' }, [
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Active Temperature')),
					E('td', { 'class': 'td left', 'id': 'fan-temp-val' }, formatTemp(status.temp))
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('CPU Temperature')),
					E('td', { 'class': 'td left', 'id': 'fan-cpu-temp-val' }, formatTemp(status.cpu_temp))
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('5G Modem Temperature')),
					E('td', { 'class': 'td left', 'id': 'fan-modem-temp-val' }, formatTemp(status.modem_temp))
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Fan Speed')),
					E('td', { 'class': 'td left', 'id': 'fan-speed-val' }, curSpeed)
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Fan RPM')),
					E('td', { 'class': 'td left', 'id': 'fan-rpm-val' },
						(status.rpm != null) ? (status.rpm + ' RPM') : '--')
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Operating Mode')),
					E('td', { 'class': 'td left', 'id': 'fan-mode-val' }, curMode)
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Active Sensor Source')),
					E('td', { 'class': 'td left', 'id': 'fan-src-val' }, curSrc)
				]),
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Hardware Driver')),
					E('td', { 'class': 'td left', 'id': 'fan-driver-val' }, formatDriverText(status))
				]),
				E('tr', { 'class': 'tr', 'id': 'fan-error-row', 'style': status.error ? '' : 'display:none' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, _('Controller Status')),
					E('td', { 'class': 'td left', 'id': 'fan-error-val', 'style': 'color:#c00;font-weight:bold' },
						status.error || '')
				])
			]);

			if (status.lucimsg)
				showBanner(null, E('p', _(status.lucimsg)), 3500, 'info');

			poll.remove(pollFanStatus);
			poll.add(pollFanStatus, 3);

			return E('fieldset', { 'class': 'cbi-section' }, [
				E('legend', {}, _('Realtime Status')),
				statusTable
			]);
		};

		s = m.section(form.NamedSection, 'cooling_control', 'cooling_control', _('Cooling Policy Settings'));

		var o = s.option(form.ListValue, 'mode', _('Operating Mode'));
		o.value('smart', _('Smart Thermal Mode (Auto)'));
		o.value('kernel', _('Kernel Thermal Mode (Native Driver)'));
		o.value('stop', _('Silent Mode (PWM 64 / 25%)'));
		o.value('low', _('Low Speed Mode (PWM 128 / 50%)'));
		o.value('normal', _('Normal Mode (PWM 192 / 75%)'));
		o.value('turbo', _('Turbo Mode (PWM 255 / 100%)'));
		o.value('stepless', _('Stepless Custom PWM Mode'));
		o.default = 'smart';
		o.rmempty = false;

		o = s.option(form.Value, 'stepless_pwm', _('Stepless PWM Duty (0 - 255)'));
		o.depends('mode', 'stepless');
		o.datatype = 'range(0,255)';
		o.rmempty = false;
		o.default = '192';
		o.description = _('Set fixed PWM value between 0 (0%) and 255 (100%).');

		o = s.option(form.ListValue, 'temp_source', _('Temperature Sensor Source'));
		o.value('cpu', _('CPU Thermal Zone (thermal_zone0)'));
		o.value('modem', _('5G Modem Chip Temperature (AT+QTEMP / AT^CHIPTEMP)'));
		o.value('max', _('Max (CPU & 5G Modem, fallback to CPU)'));
		o.default = 'cpu';
		o.rmempty = false;

		var adv = m.section(form.NamedSection, 'thermal_curve', 'thermal_curve', _('Thermal Curve & Hardware Settings'));

		var oStart = adv.option(form.Value, 'start_temp', _('Start Temperature (°C)'));
		oStart.datatype = 'range(20,95)';
		oStart.default = '40';
		oStart.rmempty = false;
		oStart.description = _('Temperature threshold where smart fan ramp begins (20 - 95 °C).');

		var oFull = adv.option(form.Value, 'full_temp', _('Full Speed Temperature (°C)'));
		oFull.datatype = 'range(30,115)';
		oFull.default = '75';
		oFull.rmempty = false;
		oFull.description = _('Temperature threshold where smart fan reaches Maximum PWM (must exceed Start Temperature).');
		oFull.validate = function(section_id, value) {
			var startV = intOrDefault(oStart.formvalue(section_id), 40);
			var fullV = parseInt(value, 10);
			if (isNaN(fullV) || fullV <= startV)
				return _('Full Speed Temperature must be greater than Start Temperature.');
			return true;
		};

		var oMinPwm = adv.option(form.Value, 'min_pwm', _('Minimum PWM (0 - 255)'));
		oMinPwm.datatype = 'range(0,255)';
		oMinPwm.default = '64';
		oMinPwm.rmempty = false;

		var oMaxPwm = adv.option(form.Value, 'max_pwm', _('Maximum PWM (0 - 255)'));
		oMaxPwm.datatype = 'range(0,255)';
		oMaxPwm.default = '255';
		oMaxPwm.rmempty = false;
		oMaxPwm.validate = function(section_id, value) {
			var minV = intOrDefault(oMinPwm.formvalue(section_id), 64);
			var maxV = parseInt(value, 10);
			if (isNaN(maxV) || maxV < minV)
				return _('Maximum PWM must be greater than or equal to Minimum PWM.');
			return true;
		};

		o = adv.option(form.Value, 'hysteresis', _('Hysteresis (°C)'));
		o.datatype = 'range(0,20)';
		o.default = '2';
		o.rmempty = false;
		o.description = _('Deadband margin to suppress rapid fan speed oscillation (0 - 20).');

		o = adv.option(form.Value, 'slew_rate', _('Slew Rate (PWM Step / Cycle)'));
		o.datatype = 'range(1,255)';
		o.default = '12';
		o.rmempty = false;
		o.description = _('Maximum PWM change per smart thermal control cycle (1 - 255).');

		o = adv.option(form.ListValue, 'inverted', _('PWM Polarity'));
		o.value('auto', _('Auto Detect (Devicetree Cooling Levels)'));
		o.value('0', _('Normal (0 = Off, 255 = Full Speed)'));
		o.value('1', _('Inverted (0 = Full Speed, 255 = Off)'));
		o.default = 'auto';
		o.rmempty = false;

		o = adv.option(form.Flag, 'wps_turbo', _('WPS Button Turbo Toggle'));
		o.enabled = '1';
		o.disabled = '0';
		o.default = '0';
		o.rmempty = false;
		o.description = _('Allow the WPS hardware button to toggle between Turbo Mode and Smart Thermal Mode.');

		return m.render();
	},

	handleSaveApply: function(ev, mode) {
		var viewSelf = this;
		if (viewSelf.applyInProgress)
			return Promise.resolve();

		viewSelf.applyInProgress = true;
		pollGeneration++;
		poll.remove(pollFanStatus);
		var shouldApply = false;

		return Promise.resolve().then(function() {
			return viewSelf.map.save(null, true);
		}).then(function() {
			var cfg = (viewSelf.formData && viewSelf.formData.cooling_control)
				? viewSelf.formData.cooling_control : {};
			var curve = (viewSelf.formData && viewSelf.formData.thermal_curve)
				? viewSelf.formData.thermal_curve : {};

			var targetMode = cfg.mode || 'smart';
			var targetPwm = intOrDefault(cfg.stepless_pwm, 192);
			var targetSource = cfg.temp_source || 'cpu';
			var targetInv = curve.inverted || 'auto';
			var targetStart = intOrDefault(curve.start_temp, 40);
			var targetFull = intOrDefault(curve.full_temp, 75);
			var targetMinPwm = intOrDefault(curve.min_pwm, 64);
			var targetMaxPwm = intOrDefault(curve.max_pwm, 255);
			var targetHyst = intOrDefault(curve.hysteresis, 2);
			var targetSlew = intOrDefault(curve.slew_rate, 12);
			var targetWps = (curve.wps_turbo === '1' || curve.wps_turbo === 1 || curve.wps_turbo === true) ? 1 : 0;

			var init = viewSelf.initialStatus || {};
			var needsInit = (init.uninitialized === true);
			shouldApply = needsInit ||
				targetMode !== (init.mode || 'smart') ||
				(targetMode === 'stepless' && targetPwm !== intOrDefault(init.stepless_pwm, 192)) ||
				targetSource !== (init.temp_source || 'cpu') ||
				targetInv !== String(init.inverted_cfg || 'auto') ||
				targetStart !== intOrDefault(init.start_temp, 40) ||
				targetFull !== intOrDefault(init.full_temp, 75) ||
				targetMinPwm !== intOrDefault(init.min_pwm, 64) ||
				targetMaxPwm !== intOrDefault(init.max_pwm, 255) ||
				targetHyst !== intOrDefault(init.hysteresis, 2) ||
				targetSlew !== intOrDefault(init.slew_rate, 12) ||
				targetWps !== intOrDefault(init.wps_turbo, 0);

			if (!shouldApply)
				return;

			return callApplyConfig(
				targetMode, targetPwm, targetSource,
				targetInv, targetStart, targetFull,
				targetMinPwm, targetMaxPwm, targetHyst,
				targetSlew, targetWps
			).then(function(res) {
				if (!res || res.code !== 0)
					throw new Error((res && res.error) ? res.error : 'RPC error');
			});
		}).then(function() {
			return callGetStatus().then(function(newStatus) {
				if (!newStatus || newStatus.code !== 0)
					throw new Error((newStatus && newStatus.error) ? newStatus.error : 'RPC error');

				viewSelf.initialStatus = newStatus;
				updateStatusDOM(newStatus);
				showBanner(null, E('p', _('Configuration has been applied.')), 3500, 'info');
			}).catch(function(err) {
				var msg = shouldApply
					? _('Configuration was applied, but status refresh failed: %s').format(err.message || err)
					: _('Fan status refresh failed: %s').format(err.message || err);
				showBanner(null, E('p', msg), 0, 'error');
			});
		}).catch(function(err) {
			showBanner(null, E('p', _('Failed to apply configuration: %s').format(err.message || err)), 0, 'error');
		}).then(function() {
			viewSelf.applyInProgress = false;
			poll.add(pollFanStatus, 3);
		});
	},

	handleSave: null,
	handleReset: null
});
