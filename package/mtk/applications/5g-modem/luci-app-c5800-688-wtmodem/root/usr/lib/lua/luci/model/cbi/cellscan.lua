module("luci.controller.cellscan", package.seeall)

function index()
    local page = entry({"admin", "modem", "cellscan"}, cbi("cellscan"), _("邻区扫描"))
    page.dependent = true
end

