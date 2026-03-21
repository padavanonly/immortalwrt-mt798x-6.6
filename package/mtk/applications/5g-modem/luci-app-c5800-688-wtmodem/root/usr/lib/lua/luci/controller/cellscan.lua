module("luci.controller.cellscan", package.seeall)

function index()
    -- entry({"admin", "modem"}, firstchild(), _("蜂窝"), 25).dependent=false
    local file = io.open("/tmp/modconf.conf", "r")
	local template_name = "cellscan/cellscan_null"
	if file then
		local content = file:read("*all")
		file:close()
		if content and string.find(content, "RM520") then
			template_name = "cellscan/cellscan"
		elseif content and string.find(content, "RM500U") then
			template_name = "cellscan/cellscan_null"
        elseif content and string.find(content, "MT5700") then
			template_name = "cellscan/cellscan_mt5700m"    
		end
	end
    --entry({"admin", "modem", "cellscan"}, template("cellscan/cellscan"), _("邻区扫描"), 80).dependent = true
    entry({"admin", "modem", "cellscan"}, template(template_name), _("邻区扫描"), 80).dependent = true
    entry({"admin", "modem", "cellscan", "switch2"}, call("action_switch2"), nil)
    entry({"admin", "modem", "cellscan", "switch5g"}, call("action_switch5g"), nil)
    entry({"admin", "modem", "cellscan", "switch4g"}, call("action_switch4g"), nil)
end


function action_switch2()
    local sys = require "luci.sys"
    local http = require "luci.http"
    local confirm = http.formvalue("confirm")
    
    if confirm and confirm == "yes" then
        --sys.call("./usr/share/modem/keyPairCellScan.sh")
        luci.http.redirect(luci.dispatcher.build_url("admin", "modem", "cellscan"))
        os.execute("/usr/share/modem/keyPairCellScan.sh")
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "modem", "cellscan"))
    end
end
function action_switch5g()
    local sys = require "luci.sys"
    local http = require "luci.http"
    local confirm = http.formvalue("confirm")
    
    if confirm and confirm == "yes" then
        luci.http.redirect(luci.dispatcher.build_url("admin", "modem", "cellscan"))
        local file = io.open("/tmp/modconf.conf", "r")
        if file then
            local content = file:read("*all")
            file:close()
            if content and string.find(content, "RM520") then
                os.execute("/usr/share/modem/keyPairCellScan.sh 5")
            elseif content and string.find(content, "MT5700") then
                os.execute("/usr/share/modem/keyPairCellScan_mt5700.sh")
            end
        end    
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "modem", "cellscan"))
    end
end
function action_switch4g()
    local sys = require "luci.sys"
    local http = require "luci.http"
    local confirm = http.formvalue("confirm")
    
    if confirm and confirm == "yes" then
        --sys.call("./usr/share/modem/keyPairCellScan.sh")
        luci.http.redirect(luci.dispatcher.build_url("admin", "modem", "cellscan"))
        local file = io.open("/tmp/modconf.conf", "r")
        if file then
            local content = file:read("*all")
            file:close()
            if content and string.find(content, "RM520") then
                os.execute("/usr/share/modem/keyPairCellScan.sh 4")
            elseif content and string.find(content, "MT5700") then
                os.execute("/usr/share/modem/keyPairCellScan_mt5700.sh 4")
            end
        end 
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "modem", "cellscan"))
    end
end


function parse_results()
    local results = {}
    local controller = {}
    -- Read and parse cellinfo file
    local cellinfo = io.open("/tmp/kpcellinfo", "r")
    if cellinfo then
        for line in cellinfo:lines() do
            local mode, operator, band, earfcn, pci, rsrp, rsrq = line:match('+QSCAN: "(.-)",(.-),(.-),(.-),(.-),(.-),(.+)')
            if mode and operator and earfcn and pci and rsrp and rsrq then
                table.insert(controller, {
                    mode = mode,
                    operator = operator,
                    band = band,
                    earfcn = earfcn,
                    pci = pci,
                    rsrp = rsrp,
                    rsrq = rsrq
                })
            end
        end
        cellinfo:close()
    else
        table.insert(controller, {
            mode = "wait for ctrl...",
            operator = "",
            band = "",
            earfcn = "",
            pci = "",
            rsrp = "",
            rsrq = ""
        })
    end
    return controller
end

function cellscan_run_time()
    local file = io.open("/tmp/cellscan_run_time", "r")
    cellscan_time="1"
    if file then
        cellscan_time=file:read("*all")
        io.close(file)
    end
    return cellscan_time
end



