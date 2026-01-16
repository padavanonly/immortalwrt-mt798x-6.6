'use strict';
'require view';
'require form';
'require uci';
'require rpc';
'require dom';

// RPC: 安全读文件
var callReadFile = rpc.declare({
    object: 'file',
    method: 'read',
    params: ['path'],
    expect: { data: '' }
});

// 这里的CSS只负责排版布局（左右分栏），完全不涉及颜色和背景
// 颜色和边框统统交给你的主题去决定喵！
var css = `
    .fan-control-container {
        display: flex;
        flex-wrap: wrap;
        align-items: flex-start; /* 顶部对齐 */
        margin: -10px; /* 抵消一点padding，让布局更紧凑 */
    }
    
    /* 监控面板 - 左侧 */
    .fan-status-container {
        flex: 1;
        min-width: 250px;
        padding: 10px;
        box-sizing: border-box;
    }

    /* 设置表单 - 右侧 */
    .fan-form-container {
        flex: 2;
        min-width: 320px;
        padding: 10px;
        box-sizing: border-box;
    }
    
    /* 简单的状态列表样式，保持原生风格 */
    .status-item {
        margin-bottom: 10px;
        padding-bottom: 10px;
        border-bottom: 1px solid #eee; /* 这里用个很淡的线条，主题通常能兼容 */
        display: flex;
        align-items: center;
    }
    /* 适配暗色主题的线条颜色 */
    @media (prefers-color-scheme: dark) {
        .status-item { border-bottom-color: #444; }
    }
    
    .status-item:last-child {
        border-bottom: none;
        margin-bottom: 0;
        padding-bottom: 0;
    }
    
    .status-icon {
        font-size: 20px;
        margin-right: 15px;
        width: 24px;
        text-align: center;
        opacity: 0.8;
    }
    
    .status-text label {
        display: block;
        font-size: 12px;
        opacity: 0.7;
    }
    
    .status-text strong {
        font-size: 16px;
    }
`;

return view.extend({
    load: function () {
        return Promise.all([ uci.load('fancontrol') ]);
    },

    render: function (data) {
        var m, s;

        // 注入布局CSS
        var style_tag = E('style', { id: 'fancontrol-style', type: 'text/css' }, css);
        dom.append(document.head, style_tag);
        
        // 创建Flex布局容器
        var container = E('div', { 'class': 'fan-control-container' }, [
            // 左侧容器：放监控
            E('div', { 'class': 'fan-status-container' }), 
            // 右侧容器：放表单
            E('div', { 'class': 'fan-form-container' })      
        ]);

        // --- 左侧：监控面板 ---
        // 使用 'cbi-section' 类，这样它就会拥有和系统一模一样的边框和背景
        var status_panel = E('div', { 'class': 'cbi-section' }, [
            E('h3', {}, _('Live Status')),
            
            E('div', { 'class': 'cbi-section-node', 'style': 'padding: 1rem;' }, [
                // 服务状态
                E('div', { 'class': 'status-item' }, [
                    E('div', { 'class': 'status-icon' }, '⚡'),
                    E('div', { 'class': 'status-text' }, [
                        E('label', {}, _('Service Status')),
                        E('strong', { 'id': 'status_enabled' }, _('Loading...'))
                    ])
                ]),
                // CPU温度
                E('div', { 'class': 'status-item' }, [
                    E('div', { 'class': 'status-icon' }, '🌡️'),
                    E('div', { 'class': 'status-text' }, [
                        E('label', {}, _('CPU Temperature')),
                        E('strong', { 'id': 'status_temp' }, _('Loading...'))
                    ])
                ]),
                // 风扇转速
                E('div', { 'class': 'status-item' }, [
                    E('div', { 'class': 'status-icon' }, '💨'),
                    E('div', { 'class': 'status-text' }, [
                        E('label', {}, _('Fan Speed Level')),
                        E('strong', { 'id': 'status_speed' }, _('Loading...'))
                    ])
                ])
            ])
        ]);
        
        // 把原生的面板放入左侧容器
        container.querySelector('.fan-status-container').appendChild(status_panel);

        // --- 右侧：设置表单 ---
        m = new form.Map('fancontrol', _('Fan Control Settings'), _('Configure the parameters for the fan control service.'));
        
        s = m.section(form.TypedSection, 'fancontrol', _('General'));
        s.anonymous = true;
        
        s.option(form.Flag, 'enabled', _('Enable Service'));
        s.option(form.Value, 'thermal_file', _('Thermal File Path'));
        s.option(form.Value, 'fan_file', _('Fan Control File Path'));
        s.option(form.Value, 'start_speed', _('Initial Speed'));
        s.option(form.Value, 'max_speed', _('Max Speed'));
        s.option(form.Value, 'start_temp', _('Start Temperature (°C)'));

        // 渲染表单
        m.render().then(function (rendered_form) {
            // 直接把渲染出来的原生表单放入右侧容器
            // 不再包裹任何自定义的 div，确保样式纯正
            container.querySelector('.fan-form-container').appendChild(rendered_form);

            // --- 数据更新逻辑 (保持不变) ---
            var isEnabled = uci.get('fancontrol', 'settings', 'enabled') == '1';
            var enabled_span = document.getElementById('status_enabled');
            if (enabled_span) {
                enabled_span.innerHTML = isEnabled 
                    ? '<span style="color:green">' + _('Running') + '</span>' 
                    : '<span style="color:red">' + _('Stopped') + '</span>';
            }

            var thermal_file = uci.get('fancontrol', 'settings', 'thermal_file');
            var fan_file = uci.get('fancontrol', 'settings', 'fan_file');

            var promises = [];
            if (thermal_file) promises.push(L.resolveDefault(callReadFile(thermal_file), ''));
            if (fan_file) promises.push(L.resolveDefault(callReadFile(fan_file), ''));

            Promise.all(promises).then(function (results) {
                var temp_str = results[0];
                var temp_span = document.getElementById('status_temp');
                if (temp_span && temp_str && temp_str.trim() !== '') {
                    var temp = parseInt(temp_str);
                    var temp_div = uci.get('fancontrol', 'settings', 'temp_div') || 1000;
                    temp_span.innerText = !isNaN(temp) ? (temp / temp_div).toFixed(1) + ' °C' : _('Invalid');
                } else if (temp_span) {
                    temp_span.innerText = _('N/A');
                }

                var speed_str = results[1] || results[0];
                var speed_span = document.getElementById('status_speed');
                if (speed_span && speed_str && speed_str.trim() !== '') {
                    var speed = parseInt(speed_str);
                    speed_span.innerText = !isNaN(speed) ? speed : _('Invalid');
                } else if (speed_span) {
                    speed_span.innerText = _('N/A');
                }
            });
        });

        return container;
    },

    dispatch: function () {
        var style_tag = document.getElementById('fancontrol-style');
        if (style_tag && style_tag.parentNode)
            style_tag.parentNode.removeChild(style_tag);
    }
});
