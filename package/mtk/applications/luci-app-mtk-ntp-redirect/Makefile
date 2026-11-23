# SPDX-License-Identifier: Apache-2.0
#
# Ported for MediaTek Filogic IMM 24.10 without modifying feeds/luci.
# Original-Author: coolsnowwolf <coolsnowwolf@gmail.com>
# Base-Author: OpenWrt LuCI Team (luci-mod-system)
# Upstream: https://github.com/coolsnowwolf/luci (openwrt-25.12)
# Cherry-picked commit: 6797fd11670700deda689334cc737e22fe949647

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mtk-ntp-redirect
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=coolsnowwolf <coolsnowwolf@gmail.com>

LUCI_TITLE:=LuCI LAN NTP Redirect Enhancement for System Settings
LUCI_DEPENDS:=+luci-mod-system +firewall4
LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/description
  Port of coolsnowwolf/luci LAN NTP request redirection UI for admin/system/system
  using native fw4 UCI redirect rules, packaged without modifying feeds/luci.
  Original author: coolsnowwolf <coolsnowwolf@gmail.com>
  Upstream commit: 6797fd11670700deda689334cc737e22fe949647
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$$IPKG_INSTROOT" ]; then
	rm -rf /tmp/luci-modulecache/ /var/luci-modulecache/
	rm -f /tmp/luci-indexcache /tmp/luci-indexcache.* /var/luci-indexcache.* 2>/dev/null
	[ -x /etc/init.d/rpcd ] && /etc/init.d/rpcd reload >/dev/null 2>&1 || true
fi
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
