module("luci.controller.cellscan", package.seeall)

function index()
    local page = entry({"admin", "modem", "Smstrun"}, cbi("Smstrun"), _("短信转发"))
    page.dependent = true
end