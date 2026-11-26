#
# Copyright (C) hanwckf <hanwckf@vip.qq.com>
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI wireless integration for MediaTek mtwifi
LUCI_DEPENDS:=@!TARGET_mediatek_filogic +mtwifi-cfg +luci-mod-network
LUCI_PKGARCH:=all

PKG_VERSION:=1
PKG_RELEASE:=13

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
