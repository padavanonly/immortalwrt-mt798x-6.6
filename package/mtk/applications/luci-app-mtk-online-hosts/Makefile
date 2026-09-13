# SPDX-License-Identifier: Apache-2.0
#
# Ported for MediaTek Filogic IMM 24.10 without modifying feeds/luci.
# Original-Author: coolsnowwolf <coolsnowwolf@gmail.com>
# Base-Author: OpenWrt LuCI Team (luci-mod-status)
# Upstream: https://github.com/coolsnowwolf/luci (openwrt-25.12)
# Cherry-picked commits:
#   8ac42d9a355f89ccd7414e17f65a6bff5ce62d95
#   22f66d0d9261b8ffd41062ec3ea40dc1d38332ed
#   086d8c870dfb0c2e3f60dbc8f78b56d190502745
#   725ddc051fe44eac07cf39b70708e24bf1539bcc
#   348b9af4860fe811bc40623c27abfb23b6911d13
#   edf9a7f6ef6b8cce16e2648df4cba806d9fb1b48
#   6211a8f0630f72753f7e3223b8f02547083af425
#   5cfaf67fed77d4a9407c60c31a5abf610c7bb5ad
#   393cc750ae526773c4f66349742abd57cd6bca31
#   e853b2b146ac0aceac0788b93ad0049e65f1a9d0
#   d7d7e0849df0892541f86186dbde61c99056bf1a

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mtk-online-hosts
PKG_RELEASE:=2
PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=coolsnowwolf <coolsnowwolf@gmail.com>

LUCI_TITLE:=LuCI Status Overview Online Hosts Monitor
LUCI_DEPENDS:=+luci-mod-status +rpcd-mod-mtk-clients +rpcd-mod-rrdns +ucode +ucode-mod-fs +ucode-mod-ubus +ucode-mod-socket
LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/description
  Port of coolsnowwolf/luci Status Overview Online Hosts table with live
  per-client upload/download rates, boot-session totals, connection counts,
  LAN web-port links, boot history, and static lease creation, packaged without
  modifying feeds/luci.
  Original author: coolsnowwolf <coolsnowwolf@gmail.com>
endef

define Build/Prepare/$(PKG_NAME)
	chmod 0755 $(PKG_BUILD_DIR)/root/etc/init.d/luci-client-history
	chmod 0755 $(PKG_BUILD_DIR)/root/usr/libexec/luci-client-history
	chmod 0755 $(PKG_BUILD_DIR)/root/usr/libexec/luci-arp
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$$IPKG_INSTROOT" ]; then
	for f in \
		/etc/init.d/luci-client-history \
		/usr/libexec/luci-client-history \
		/usr/libexec/luci-arp
	do
		[ -f "$$f" ] && chmod 0755 "$$f"
	done
	rm -rf /tmp/luci-modulecache/ /var/luci-modulecache/
	rm -f /tmp/luci-indexcache /tmp/luci-indexcache.* /var/luci-indexcache.* 2>/dev/null
	[ -x /etc/init.d/luci-client-history ] && {
		/etc/init.d/luci-client-history enable >/dev/null 2>&1 || true
		/etc/init.d/luci-client-history restart >/dev/null 2>&1 || true
	}
	[ -x /etc/init.d/rpcd ] && /etc/init.d/rpcd reload >/dev/null 2>&1 || true
fi
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
