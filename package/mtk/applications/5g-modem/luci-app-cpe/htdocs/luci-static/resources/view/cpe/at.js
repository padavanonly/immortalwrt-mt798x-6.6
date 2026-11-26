'use strict';
'require view';
'require rpc';
'require uci';

var callExecAt = rpc.declare({
	object: 'luci.cpe',
	method: 'exec_at',
	params: ['port', 'command'],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return uci.load('custom').catch(function() {
			return {};
		});
	},

	sendCmd: function() {
		var dropEl = document.getElementById('drop1');
		var cmdEl = document.getElementById('atcmd');
		var txtEl = document.getElementById('attxt');
		if (!dropEl || !cmdEl || !txtEl)
			return false;

		var port = dropEl.value;
		var cmd = (cmdEl.value || '').trim();
		var prev = txtEl.value || '';

		if (cmd.length === 0) {
			txtEl.value = _('Please enter an AT command!') + '\n' + prev;
			return false;
		}

		callExecAt(port, cmd).then(function(rv) {
			if (rv && rv.code === 0) {
				var pd = rv.result || '';
				txtEl.value = pd + '\n' + prev;
			} else {
				var errMsg = (rv && rv.error) ? rv.error : _('AT command error');
				txtEl.value = errMsg + '\n' + prev;
			}
		}).catch(function(err) {
			txtEl.value = String(err) + '\n' + prev;
		});

		return false;
	},

	sendClean: function() {
		var txtEl = document.getElementById('attxt');
		if (txtEl)
			txtEl.value = '';
		return false;
	},

	render: function() {
		var self = this;
		var portOptions = [];
		for (var i = 0; i <= 12; i++) {
			var optAttrs = { 'value': String(i) };
			if (i === 2)
				optAttrs.selected = 'selected';
			portOptions.push(E('option', optAttrs, 'ttyUSB' + i));
		}

		var portSelect = E('select', {
			'style': 'width:200px;',
			'name': 'atdrop',
			'id': 'drop1',
			'class': 'cbi-input-select'
		}, portOptions);

		var cmdInput = E('input', {
			'type': 'text',
			'name': 'atcmdlck',
			'id': 'atcmd',
			'class': 'cbi-input-text',
			'maxlength': '200',
			'style': 'width:500px;',
			'keydown': function(ev) {
				if (ev.key === 'Enter') {
					ev.preventDefault();
					self.sendCmd();
				}
			}
		});

		var sendBtn = E('button', {
			'type': 'button',
			'id': 'sendat',
			'class': 'btn cbi-button cbi-button-apply',
			'click': function(ev) {
				ev.preventDefault();
				self.sendCmd();
			}
		}, _('Send'));

		var cleanBtn = E('button', {
			'type': 'button',
			'id': 'sendclean',
			'class': 'btn cbi-button cbi-button-reset',
			'style': 'margin-left:0.5rem;',
			'click': function(ev) {
				ev.preventDefault();
				self.sendClean();
			}
		}, _('Clear'));

		var outputArea = E('textarea', {
			'readonly': 'readonly',
			'name': 'attxt',
			'id': 'attxt',
			'rows': '30',
			'style': 'width:100%;background:#373737;border:none;color:#FFF;padding:10px;font-family:monospace;'
		});

		return E('div', { 'class': 'cbi-map', 'id': 'cbi-misc' }, [
			E('h2', { 'id': 'content', 'name': 'content' }, _('AT Command Tool')),
			E('div', { 'class': 'cbi-map-descr' }, _('AT Command Tool')),
			E('fieldset', { 'class': 'cbi-section', 'id': 'cbi-term' }, [
				E('legend', {}, _('AT Command Terminal')),
				E('table', { 'width': '100%', 'border': '0', 'id': 'droptxt', 'class': 'table' }, [
					E('tbody', {}, [
						E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td right', 'width': '12%' }, _('Modem Port: ')),
							E('td', { 'class': 'td left', 'width': '18%' }, [portSelect]),
							E('td', { 'class': 'td right', 'width': '12%' }, _('AT Command: ')),
							E('td', { 'class': 'td left', 'width': '38%' }, [cmdInput]),
							E('td', { 'class': 'td left', 'width': '20%' }, [sendBtn, cleanBtn])
						])
					])
				]),
				outputArea
			])
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
