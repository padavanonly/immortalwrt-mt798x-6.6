module("luci.controller.Smstrun", package.seeall)
local fs = require "nixio.fs"
function index()
    entry({"admin", "modem", "Smstrun"}, template("Smstrun/settings"), _("短信转发"), 80).dependent = true
    entry({"admin", "modem", "Smstrun", "set_token"}, call("set_token"), nil).leaf = true
    entry({"admin", "modem", "Smstrun", "set_title"}, call("set_title"), nil).leaf = true
    entry({"admin", "modem", "Smstrun", "check_status"}, call("check_status"), nil).leaf = true
    entry({"admin", "modem", "Smstrun", "redhis"}, call("redhis"), nil).leaf = true
end
function set_token()
    local token = luci.http.formvalue("ppsToken")
    if token then
        fs.writefile("/usr/bin/smstrun.conf", token)
        local output = luci.sys.exec("/usr/bin/setppstoken.sh")
        luci.http.prepare_content("application/json")
        luci.http.write_json({ result = true, output = output })
        luci.sys.exec("python3 /usr/bin/smstrun.py")
    else
        luci.http.status(400, "Bad Request")
    end
end

function set_title()
    local title = luci.http.formvalue("smsTitle")
    if title then
        fs.writefile("/usr/bin/smstrun-title.conf", title)
        local output = luci.sys.exec("/usr/bin/setsmstitle.sh")
        luci.http.prepare_content("application/json")
        luci.http.write_json({ result = true, output = output })
    else
        luci.http.status(400, "Bad Request")
    end
end


function check_status()
    local script = "/usr/bin/smstrun.py"
    local token_file = "/usr/bin/smstrun.conf"
    local title_file = "/usr/bin/smstrun-title.conf"
    local is_running = luci.sys.exec("pgrep -f " .. script) ~= ""
    local token_content = luci.sys.exec("cat " .. token_file) or ""
    local title_content = luci.sys.exec("cat " .. title_file) or ""
    luci.http.prepare_content("application/json")
    if is_running then
        luci.http.write_json({ status = "running", token = token_content, title = title_content })
    else
        luci.http.write_json({ status = "stopped" })
    end
end

function redhis()
    local output = luci.sys.exec("cat /tmp/smstrunsum.conf") or "" 
    if output == "" then
        output = "未发现转发记录，请核对。"
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json({ result = true, output = output })
end

