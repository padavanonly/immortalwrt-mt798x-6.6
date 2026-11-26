'use strict';
'require view';
'require form';
'require poll';
'require rpc';
'require uci';

var callGetStatus = rpc.declare({
	object: 'luci.mwan3helper',
	method: 'getStatus',
	expect: { '': {} }
});

function renderStatus(isRunning) {
	var spanTemp = '<em><b><span style="color:%s">%s %s</span></b></em>';
	var renderHTML;
	if (isRunning) {
		renderHTML = String.format(spanTemp, 'green', _('MWAN3 Helper'), _('RUNNING'));
	} else {
		renderHTML = String.format(spanTemp, 'red', _('MWAN3 Helper'), _('NOT RUNNING'));
	}
	return renderHTML;
}

return view.extend({
	load: function() {
		return Promise.all([
			callGetStatus(),
			uci.load('mwan3helper')
		]);
	},

	render: function(data) {
		var statusRes = data[0] || {};
		var isRunning = Boolean(statusRes.status || statusRes.running);
		var m, s, o;

		m = new form.Map('mwan3helper', _('MWAN3 Helper'), _('MWAN3 Helper generate IPSets'));

		s = m.section(form.TypedSection);
		s.anonymous = true;
		s.render = function() {
			poll.add(function() {
				return L.resolveDefault(callGetStatus()).then(function(res) {
					var viewEl = document.getElementById('mwan3helper_status');
					if (viewEl) {
						var running = Boolean(res && (res.status || res.running));
						viewEl.innerHTML = renderStatus(running);
					}
				});
			});

			return E('div', { class: 'cbi-section', id: 'status_bar' }, [
				E('p', { id: 'mwan3helper_status' }, [
					E('em', {}, [ _('Collecting data...') ])
				])
			]);
		};

		s = m.section(form.TypedSection, 'mwan3helper');
		s.addremove = false;
		s.anonymous = true;

		o = s.option(form.Flag, 'enabled', _('Enable Forwarding'));
		o.rmempty = false;

		o = s.option(form.ListValue, 'mode', _('Traffic Forwarding Mode'));
		o.value('0', _('Foreign Traffic'));
		o.value('1', _('GFWList Traffic'));

		o = s.option(form.Value, 'dns', _('Forward Traffic to IP'));
		o.datatype = 'ip4addr';

		o = s.option(form.Value, 'dev', _('Device'));

		o = s.option(form.Flag, 'dnsen', _('Enable DNS Anti-Pollution'));
		o.rmempty = false;

		o = s.option(form.Value, 'dnsserver', _('Anti-pollution DNS Server'));
		o.datatype = 'ip4addr';

		return m.render().then(function(mapEl) {
			var statusEl = mapEl.querySelector('#mwan3helper_status');
			if (statusEl) {
				statusEl.innerHTML = renderStatus(isRunning);
			}
			return mapEl;
		});
	}
});
