# SPDX-Identifier-License: GPL-3.0-only
#
# Copyright (C) 2018 Lean <coolsnowwolf@gmail.com>
# Copyright (C) 2019-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-turboacc-mtk
PKG_LICENSE:=GPL-3.0-only

LUCI_TITLE:=LuCI support for MTK HNAT
LUCI_DEPENDS:= \
	+luci-base \
	+rpcd \
	+kmod-mediatek_hnat \
	+luci-lua-runtime \
	+kmod-tcp-bbr \
	+kmod-nft-socket +kmod-nft-tproxy +kmod-nft-nat +kmod-inet-diag \
	+kmod-ipt-nat6 +kmod-ipt-nat +kmod-netlink-diag \
	+kmod-ipt-physdev +kmod-nf-ipvs +kmod-veth +kmod-wireguard \
	+kmod-ipt-fullconenat +kmod-fs-btrfs +luci-app-ttyd +kmod-bonding \
	@!PACKAGE_luci-app-turboacc
LUCI_PKGARCH:=all

define Build/Prepare/$(PKG_NAME)
	chmod 0755 $(PKG_BUILD_DIR)/root/etc/init.d/turboacc
	chmod 0755 $(PKG_BUILD_DIR)/root/etc/uci-defaults/turboacc
	chmod 0755 $(PKG_BUILD_DIR)/root/usr/libexec/rpcd/luci.turboacc
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$$IPKG_INSTROOT" ]; then
	for runtime_file in \
		/etc/init.d/turboacc \
		/etc/uci-defaults/turboacc \
		/usr/libexec/rpcd/luci.turboacc
	do
		[ -f "$$runtime_file" ] && chmod 0755 "$$runtime_file"
	done
	rm -rf /tmp/luci-modulecache/ /var/luci-modulecache/
	rm -f /tmp/luci-indexcache /tmp/luci-indexcache.* /var/luci-indexcache.* 2>/dev/null
	[ -x /etc/init.d/turboacc ] && /etc/init.d/turboacc enable >/dev/null 2>&1 || true
	[ -x /etc/init.d/turboacc ] && /etc/init.d/turboacc restart >/dev/null 2>&1 || true
	[ -x /etc/init.d/rpcd ] && /etc/init.d/rpcd reload >/dev/null 2>&1 || true
fi
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
