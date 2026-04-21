# Copyright (C) 2026
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-c5800-688-wtmodem
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0-or-later
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=LuCI support for C5800-688 WTModem
  DEFAULT:=y
  DEPENDS:=+luci-compat +python3-light +libc +libgcc
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/apninfo
/etc/config/modem
endef

define Package/$(PKG_NAME)/description
 C5800-688 5G modem control plugin.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(CP) ./root/. $(1)/
	rm -f $(1)/usr/share/modem/mopdu
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
