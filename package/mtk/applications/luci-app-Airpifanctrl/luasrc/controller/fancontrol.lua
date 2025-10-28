module("luci.controller.fancontrol", package.seeall)

function index()
    entry({"admin", "status", "fancontrol"}, template("Airpifanctrl/fancontrol"), _("风扇控制"), 94)
    entry({"admin", "fancontrol", "fanstop"}, call("action_fanstop"))
    entry({"admin", "fancontrol", "fanst1"}, call("action_fanst1"))
    entry({"admin", "fancontrol", "fanst2"}, call("action_fanst2"))

    -- 免认证 fanst3
    local e_fanst3 = entry({"admin", "fancontrol", "fanst3"}, call("action_fanst3"))
    e_fanst3.sysauth = false
    e_fanst3.leaf = true

    -- 免认证 fanst4
    local e_fanst4 = entry({"admin", "fancontrol", "fanst4"}, call("action_fanst4"))
    e_fanst4.sysauth = false
    e_fanst4.leaf = true

    entry({"admin", "fancontrol", "fansttp"}, call("action_fansttp"))
    entry({"admin", "fancontrol", "fanst"}, call("action_fanst"))
    entry({"admin", "fancontrol", "fansvm"}, call("action_fansvm"))
    entry({"admin", "fancontrol", "fansvc"}, call("action_fansvc"))
    entry({"admin", "fancontrol", "fanswj"}, call("action_fanswj"))
    entry({"admin", "fancontrol", "fanswj2"}, call("action_fanswj2"))

    -- 免认证 msg
    local e_msg = entry({"admin", "fancontrol", "msg"}, call("action_msg"))
    e_msg.sysauth = false
    e_msg.leaf = true
end


function action_msg()
    local rv = {}
    local p = luci.http.formvalue("p")
    local set = luci.http.formvalue("set")
    local fixed = set
    local port = string.gsub(p, "\"", "~")
    rv["at"] = fixed 
    rv["port"] = port

    local filepath = "/tmp/lucimsg.file"
    local file = io.open(filepath, "r")
    if file then
        local content = file:read("*a")
        file:close()
        rv["lucimsg"] = content
        os.remove(filepath)
    end

    luci.http.prepare_content("application/json")
    luci.http.write_json(rv)
end

function action_fanswj()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end
	os.execute("echo 999 > /etc/fanvall")
	os.execute("echo "..port.." > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1")
	os.execute("echo "..port.." > /usr/bin/fanspeed.conf")
	rv["result"] = "fanswj"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_fanswj2()
    local rv = {}
    local file
    local p = luci.http.formvalue("p")
    local set = luci.http.formvalue("set")
    fixed = set
    port = string.gsub(p, "\"", "~")
    rv["at"] = fixed 
    rv["port"] = port
    local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end

    os.execute("echo 999 > /etc/fanvall")
    local conf_handle = io.open("/usr/bin/fanspeed.conf", "r")
    if conf_handle then
        local conf_content = conf_handle:read("*a")
        conf_handle:close()
        rv["result"] = conf_content
    else
        rv["result"] = "无法读取 /usr/bin/fanspeed.conf"
    end

    luci.http.prepare_content("application/json")
    luci.http.write_json(rv)
end


function action_fanst()
    local rv = {}
    local file_path = "/usr/bin/fanspeed.conf"
    local file = io.open(file_path, "r")
    if file then
        local content = file:read("*a")
        file:close()
        rv["fanspd"] = content
    else
        rv["fanspd"] = "未获取到转速"
    end
    
    local process_check = io.popen("pgrep -f fancts.sh")
    local process_result = process_check:read("*a")
    process_check:close()

    local fanvall_file = "/etc/fanvall"
    local fanvall_check = io.open(fanvall_file, "r")
    local fanvall_content = nil
    if fanvall_check then
        fanvall_content = fanvall_check:read("*a")
        fanvall_check:close()
    end
    
    if fanvall_content and fanvall_content:match("^%s*999%s*$") then
        rv["fancts"] = "无极"
    elseif process_result ~= "" then
        rv["fancts"] = "智能"
    else
        rv["fancts"] = "手动"
    end

    local p = luci.http.formvalue("p")
    local set = luci.http.formvalue("set")
    local fixed = set
    local port = string.gsub(p, "\"", "~")
    rv["at"] = fixed 
    rv["port"] = port
    
    luci.http.prepare_content("application/json")
    luci.http.write_json(rv)
end


function action_fansttp()
    local rv = {}
    local p = luci.http.formvalue("p")
    local set = luci.http.formvalue("set")
    local fixed = set
    local port = string.gsub(p, "\"", "~")

    local fansv="温度类型获取中..."
	local temperature = 0
    local conf_file = io.open("/etc/fanvallv.conf", "r")

    if conf_file then
        local config = conf_file:read("*a")
        conf_file:close()

        if config:match("模组温度") then
			fansv="MT5700M-CN模组温度"
            local sendat_command = io.popen("sendat 1 'AT^CHIPTEMP?' | grep 'CHIPTEMP' | sed -n '1p' | cut -d, -f9 | sed '/^$/d'")
            local temp_output = sendat_command:read("*a")
            sendat_command:close()
            local temp_value = tonumber(temp_output)
            if temp_value then
                temperature = temp_value / 10
            else
                temperature = "null"
            end
        else
            fansv="CPU温度"
            local file = io.open("/sys/class/thermal/thermal_zone0/temp", "r")
            if file then
                temperature = file:read("*n")
                file:close()
                temperature = temperature / 1000
            else
                temperature = "null"
            end
        end
    else
		fansv="CPU温度"
        local file = io.open("/sys/class/thermal/thermal_zone0/temp", "r")
        if file then
            temperature = file:read("*n")
            file:close()
            temperature = temperature / 1000
        else
            temperature = "null"
        end
    end

    rv["at"] = fixed 
    rv["port"] = port
    rv["fansttp"] = temperature
	rv["fansv"] = fansv

    luci.http.prepare_content("application/json")
    luci.http.write_json(rv)
end



function action_fanst2()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end
	os.execute("echo 2 > /etc/fanvall")
	os.execute("echo 192 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1")
	os.execute("echo 192 > /usr/bin/fanspeed.conf")
	rv["result"] = "fanst2"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_fanst1()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end
	os.execute("echo 1 > /etc/fanvall")
	os.execute("echo 128 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1")
	os.execute("echo 128 > /usr/bin/fanspeed.conf")
	rv["result"] = "fanst2"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_fanst3()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end
	os.execute("echo 3 > /etc/fanvall")
	os.execute("echo 255 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1")
	os.execute("echo 255 > /usr/bin/fanspeed.conf")
	rv["result"] = "fanst3"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_fanst4()
    local rv = {}
    local p = luci.http.formvalue("p")
    local set = luci.http.formvalue("set")
    local fixed = set
    local port = string.gsub(p, "\"", "~")

    rv["at"] = fixed 
    rv["port"] = port
    local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end
	os.execute("echo 9 > /etc/fanvall")
    os.execute("/usr/bin/fancts.sh &")
    rv["result"] = "fanst4"
    
    luci.http.prepare_content("application/json")
    luci.http.write_json(rv)
end


function action_fanstop()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	local handle = io.popen("pgrep -f fancts.sh")
    local pid = handle:read("*a")
    handle:close()

    if pid then
        pid = pid:match("%d+") 
        if pid then
            os.execute("kill -9 " .. pid)
        end
    end
	os.execute("echo 0 > /etc/fanvall")
	os.execute("echo 64 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1")
	os.execute("echo 64 > /usr/bin/fanspeed.conf")
	rv["result"] = "fanstop"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_fansvm()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	os.execute("echo 9 > /etc/fanvall")
	os.execute("echo 模组温度 > /etc/fanvallv.conf")
	rv["result"] = "fansvm"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_fansvc()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port
	os.execute("echo 9 > /etc/fanvall")
	os.execute("echo CPU温度 > /etc/fanvallv.conf")
	rv["result"] = "fansvc"
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end


