# SPDX-Identifier-License: GPL-3.0-only
#
# Copyright (C) 2018 Lean <coolsnowwolf@gmail.com>
# Copyright (C) 2019-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPL-3.0-only

LUCI_TITLE:=LuCI support for MTK HNAT
LUCI_DEPENDS:= \
	+kmod-tcp-bbr \
	+kmod-nft-socket +kmod-nft-tproxy +kmod-nft-nat +kmod-inet-diag \
	+kmod-ipt-nat6 +kmod-ipt-nat +kmod-netlink-diag \
	+kmod-ipt-physdev +kmod-nf-ipvs +kmod-veth +kmod-wireguard \
	+kmod-ipt-fullconenat +kmod-fs-btrfs +luci-app-ttyd +kmod-bonding \
	@!PACKAGE_luci-app-turboacc
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
