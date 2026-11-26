# SPDX-License-Identifier: MIT

include $(TOPDIR)/rules.mk

PKG_NAME:=fancontrol
PKG_VERSION:=2.0
PKG_RELEASE:=1
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/fancontrol
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Unified Thermal Fan Controller for OpenWrt
  DEPENDS:=+ucode +ucode-mod-fs +ucode-mod-ubus +rpcd
  PKGARCH:=all
endef

define Package/fancontrol/description
  Unified ucode/procd thermal fan control daemon and rpcd backend supporting
  hwmon PWM (including tach RPM and inverted polarity), gpio-fan, GPIO duty,
  and multi-sensor CPU/5G modem thermal regulation across MediaTek Filogic boards.
endef

define Package/fancontrol/conffiles
/etc/config/fancontrol
endef

define Build/Compile
endef

define Package/fancontrol/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/fancontrol $(1)/etc/config/fancontrol
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/fancontrol $(1)/etc/init.d/fancontrol
	$(INSTALL_DIR) $(1)/etc/hotplug.d/button
	$(INSTALL_BIN) ./files/etc/hotplug.d/button/50-fancontrol-wps $(1)/etc/hotplug.d/button/50-fancontrol-wps
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/90-fancontrol-migrate $(1)/etc/uci-defaults/90-fancontrol-migrate
	$(INSTALL_DIR) $(1)/usr/share/ucode/fancontrol
	$(INSTALL_DATA) ./files/usr/share/ucode/fancontrol/common.uc $(1)/usr/share/ucode/fancontrol/common.uc
	$(INSTALL_DIR) $(1)/usr/libexec/fancontrol
	$(INSTALL_BIN) ./files/usr/libexec/fancontrol/worker.uc $(1)/usr/libexec/fancontrol/worker.uc
	$(INSTALL_DIR) $(1)/usr/libexec/rpcd
	$(INSTALL_BIN) ./files/usr/libexec/rpcd/luci.fancontrol $(1)/usr/libexec/rpcd/luci.fancontrol
endef

$(eval $(call BuildPackage,fancontrol))
