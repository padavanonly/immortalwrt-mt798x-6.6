# Copyright (C) 2016-2026 OpenWrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mwan3helper-chinaroute
PKG_VERSION:=2.0
PKG_RELEASE:=1

LUCI_TITLE:=LuCI JS support for MWAN3 Helper chinaroute
LUCI_DEPENDS:=+luci-base +rpcd +ucode +ucode-mod-fs +dnsmasq-full +ipset
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
