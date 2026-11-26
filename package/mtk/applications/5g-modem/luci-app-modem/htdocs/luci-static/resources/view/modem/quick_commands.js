'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require poll';
'require ui';

var callGetQuickCommands = rpc.declare({
	object: 'luci.modem',
	method: 'getQuickCommands',
	params: [],
	expect: { '': {} }
});

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('custom_at_commands'),
			uci.load('modem'),
			callGetQuickCommands().catch(function() { return {}; })
		]);
	},

	render: function() {
		var m = new form.Map('custom_at_commands', _('Custom quick commands'), _('Customize your quick commands'));

		var sGlobal = m.section(form.TypedSection, 'global', _('Quick Option'));
		sGlobal.uciconfig = 'modem';
		sGlobal.anonymous = true;
		sGlobal.addremove = false;

		var oQuickOpt = sGlobal.option(form.ListValue, 'quick_option', _('Quick Option'));
		oQuickOpt.uciconfig = 'modem';
		oQuickOpt.default = 'auto';
		oQuickOpt.rmempty = true;
		oQuickOpt.value('auto', _('Auto'));
		oQuickOpt.value('custom', _('Custom'));
		oQuickOpt.value('default', _('General'));

		var s = m.section(form.TableSection, 'custom-commands', _('Custom Commands'));
		s.anonymous = true;
		s.addremove = true;
		s.sortable = true;

		var oDesc = s.option(form.Value, 'description', _('Description'));
		oDesc.placeholder = _('Not null');
		oDesc.rmempty = false;
		oDesc.optional = false;

		var oCmd = s.option(form.Value, 'command', _('Command'));
		oCmd.placeholder = _('Not null');
		oCmd.rmempty = false;
		oCmd.optional = false;

		return m.render().then(function(node) {
			node.appendChild(E('div', { 'class': 'cbi-page-actions' }, [
				E('a', {
					'class': 'btn cbi-button cbi-button-link',
					'href': L.url('admin', 'modem', 'modem_debug')
				}, _('Return to modem debug'))
			]));
			return node;
		});
	}
});
