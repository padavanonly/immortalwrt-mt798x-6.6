#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtwifi-cfg
PKG_LICENSE:=GPL-2.0+
PKG_VERSION:=1
PKG_RELEASE:=8

include $(INCLUDE_DIR)/package.mk

define Package/mtwifi-cfg
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  TITLE:=mtwifi configure scripts for mt798x
  DEPENDS:=@!TARGET_mediatek_filogic +iwinfo +wifi-scripts +ucode +ucode-mod-fs +ucode-mod-log +ucode-mod-uci +ucode-mod-ubus +datconf @!PACKAGE_luci-app-mtk @!PACKAGE_wifi-profile
endef

define Build/Compile
endef

define Package/mtwifi-cfg/install
	$(INSTALL_DIR) $(1)/sbin $(1)/usr/share/ucode/mtwifi
	$(INSTALL_DIR) $(1)/lib/netifd/wireless/ $(1)/lib/wifi/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/net/

	$(INSTALL_BIN) ./files/netifd/mtwifi.sh $(1)/lib/netifd/wireless/
	$(INSTALL_BIN) ./files/mtwifi.sh $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/mtwifi-cfg/mtwifi_cfg $(1)/sbin/
	$(INSTALL_DATA) ./files/mtwifi-cfg/mtwifi_defs.uc $(1)/usr/share/ucode/mtwifi/
	$(INSTALL_DATA) ./files/mtwifi-cfg/mtwifi_utils.uc $(1)/usr/share/ucode/mtwifi/
	$(INSTALL_BIN) ./files/l1util/l1util $(1)/sbin/
	$(LN) l1util $(1)/sbin/l1dat
	$(INSTALL_DATA) ./files/l1util/l1dat_parser.uc $(1)/usr/share/ucode/mtwifi/
	$(INSTALL_DATA) ./files/hotplug/10-mtwifi-detect $(1)/etc/hotplug.d/net/
endef

$(eval $(call BuildPackage,mtwifi-cfg))
