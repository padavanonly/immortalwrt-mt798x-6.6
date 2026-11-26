# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=switch-rtl837x
PKG_RELEASE:=7

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/RuijieNetworksCommunity/rtl837x-gsw-driver.git
PKG_SOURCE_DATE:=2026-05-25
PKG_SOURCE_VERSION:=a1650c6ac6e658b7ee7594ec8283d94205a001fe
PKG_MIRROR_HASH:=40f2aaaa8d01238ff6840e5e9af4107675229a3e0ea976645f3a3ec14b9282d7

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_PARALLEL:=1
PKG_EXTMOD_SUBDIRS:=src

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL837x switch driver
  FILES:=$(PKG_BUILD_DIR)/src/rtl837x_gsw.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl837x_gsw,1)
  DEPENDS:=+kmod-swconfig +jshn
  CONFLICTS:=kmod-rtl8372n-dsa
endef

define KernelPackage/$(PKG_NAME)/conffiles
/etc/config/rtl837x
endef

define KernelPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/rtl837x $(1)/etc/config/rtl837x
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/rtl837x-tune $(1)/etc/init.d/rtl837x-tune
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/99-rtl837x-init $(1)/etc/uci-defaults/99-rtl837x-init
	$(INSTALL_DIR) $(1)/usr/libexec/rpcd
	$(INSTALL_BIN) ./files/usr/libexec/rpcd/rtl837x $(1)/usr/libexec/rpcd/rtl837x
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/usr/share/rpcd/acl.d/rtl837x.json $(1)/usr/share/rpcd/acl.d/rtl837x.json
	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/status/include
	$(INSTALL_DATA) ./files/www/luci-static/resources/view/status/include/25_rtl837x.js $(1)/www/luci-static/resources/view/status/include/25_rtl837x.js
	if [ -x "$(STAGING_DIR_HOSTPKG)/bin/po2lmo" ]; then \
		$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n; \
		$(STAGING_DIR_HOSTPKG)/bin/po2lmo ./po/zh_Hans/rtl837x.po $(1)/usr/lib/lua/luci/i18n/rtl837x.zh-cn.lmo; \
	fi
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_RTL837x_GSW=m \
		CONFIG_RTL837x_GSW_PORT_MIB_FEATURE=y \
		modules
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
