module("luci.controller.modem", package.seeall)

function index()
	entry({"admin", "modem"}, firstchild(), _("内置蜂窝"), 25).dependent=false
	--entry({"admin", "modem", "nets"}, template("zmode/net_status"), _("信号状态"), 97)
	local file = io.open("/tmp/modconf.conf", "r")
	local template_name = "zmode/net_status"
	if file then
		local content = file:read("*all")
		file:close()
		if content and string.find(content, "RM520") then
			template_name = "zmode/net_status_RM520"
		elseif content and string.find(content, "RM500U") then
			template_name = "zmode/net_status_RM500U"
		elseif content and string.find(content, "MT5700") then
			template_name = "zmode/net_status_MT5700M"	
		end
	end
	entry({"admin", "modem", "smsc"}, template("zmode/smsc"), _("短信管理"), 95)
	entry({"admin", "modem", "nets"}, template(template_name), _("模组状态"), 96)
	entry({"admin", "modem", "at"}, template("zmode/at"), _("串口调试"), 97)
	entry({"admin", "modem", "at2"}, template("zmode/at2"), _("网络调试"), 98)
	entry({"admin", "modem", "modem"}, cbi("modem"), _("模组设定"), 100) 
	entry({"admin", "modem", "get_csq"}, call("action_get_csq"))
	entry({"admin", "modem", "send_atcmd"}, call("action_send_atcmd"))
	entry({"admin", "modem", "smscs"}, call("action_smscs"))
end


function action_smscs()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port

	os.execute("/usr/share/modem/smsc.sh \'" .. port .. "\' \'" .. fixed .. "\'")
	smsc = "/tmp/smsc.at"
	file = io.open(smsc, "r")
	if file ~= nil then
		rv["smsc"] = file:read("*all")
		file:close()
	else
		rv["smsc"] = " "
	end
	os.execute("/usr/share/modem/rsmsc.sh")
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end


function action_send_atcmd()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	--fixed = string.gsub(set, "\"", "~")
	fixed = set
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port

	os.execute("/usr/share/modem/atcmd.sh \'" .. port .. "\' \'" .. fixed .. "\'")
	result = "/tmp/result.at"
	file = io.open(result, "r")
	if file ~= nil then
		rv["result"] = file:read("*all")
		file:close()
	else
		rv["result"] = " "
	end
	os.execute("/usr/share/modem/delatcmd.sh")
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function action_get_csq()
	local file = io.open("/tmp/modconf.conf", "r")
	if file then
		local content = file:read("*all")
		file:close()
		if content and string.find(content, "RM520") then
			rm520_rm500U()			
		elseif content and string.find(content, "RM500U") then
			rm520_rm500U()	
		elseif content and string.find(content, "MT5700") then	
			MT5700()
		end
	end
end

function rm520_rm500U()
	os.execute("/usr/share/modem/zinfo.sh")
	local file
	stat = "/tmp/cpe_cell.file"
	file = io.open(stat, "r")
	local rv ={}
	rv["stsss"] = file:read("*line")
	rv["modem"] = file:read("*line")
	rv["conntype"] = file:read("*line")
	rv["firmware"] = file:read("*line")
	rv["temper"] = file:read("*line")
	rv["date"] = file:read("*line")
	--------------------------------
	rv["simsel"] = file:read("*line")
	rv["cops"] = file:read("*line")
	rv["imei"] = file:read("*line")
	rv["imsi"] = file:read("*line")
	rv["iccid"] = file:read("*line")
	rv["phone"] = file:read("*line")
	--------------------------------
	rv["mode"] = file:read("*line")
	rv["per"] = file:read("*line")
	rv["rssi"] = file:read("*line")
	rv["rsrq"] = file:read("*line")
	rv["rscp"] = file:read("*line")
	rv["sinr"] = file:read("*line")
	-------------------------------
	rv["mcc"] = file:read("*line")
	rv["lac"] = file:read("*line")
	rv["cid"] = file:read("*line")
	rv["band"] = file:read("*line")
	rv["rfcn"] = file:read("*line")
	rv["pci"] = file:read("*line")
	rv["apn"] = file:read("*line")
	rv["down"] = file:read("*line")
	rv["up"] = file:read("*line")
	--------------------------------
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function MT5700()
	os.execute("/usr/share/modem/zinfo_mt5700.sh")
    local file, file2
    local stat = "/tmp/cpe_cell.file"
    local stat2 = "/tmp/stsss_u.file"
    file = io.open(stat, "r")
    file2 = io.open(stat2, "r")
	local rv ={}
	rv["stsss"] = file:read("*line")
	rv["modem"] = file:read("*line")
	rv["conntype"] = file:read("*line")
	rv["firmware"] = file:read("*line")
	rv["temper"] = file:read("*line")
	rv["date"] = file:read("*line")
	--------------------------------
	rv["simsel"] = file:read("*line")
	rv["cops"] = file:read("*line")
	rv["imei"] = file:read("*line")
	rv["imsi"] = file:read("*line")
	rv["iccid"] = file:read("*line")
	rv["phone"] = file:read("*line")
	--------------------------------
	rv["mode"] = file:read("*line")
	rv["per"] = file:read("*line")
	rv["rssi"] = file:read("*line")
	rv["rsrq"] = file:read("*line")
	rv["rscp"] = file:read("*line")
	rv["sinr"] = file:read("*line")
	-------------------------------
	rv["mcc"] = file:read("*line")
	rv["lac"] = file:read("*line")
	rv["cid"] = file:read("*line")
	rv["band"] = file:read("*line")
	rv["rfcn"] = file:read("*line")
	rv["pci"] = file:read("*line")
	rv["apn"] = file:read("*line")
	rv["down"] = file:read("*line")
	rv["up"] = file:read("*line")
	rv["qci"] = file:read("*line")
	rv["zbjh"] = file:read("*line")
	rv["r2cc"] = file:read("*line")
	rv["r3cc"] = file:read("*line")
	--------------------------------
	rv["cell1"] = file:read("*line")
	rv["cell2"] = file:read("*line")
	rv["cell3"] = file:read("*line")
	rv["cell4"] = file:read("*line")
	rv["cell5"] = file:read("*line")
-------------------------------
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

