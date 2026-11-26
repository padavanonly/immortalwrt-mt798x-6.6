# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=mtk-qos
PKG_VERSION:=2.0
PKG_RELEASE:=7
PKG_LICENSE:=GPL-2.0-only
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/mtk-qos
  SECTION:=net
  CATEGORY:=Network
  TITLE:=MediaTek NETSYS v3 4-Coprocessor Unified QoS Controller
  PROVIDES:=mtk-npu-sqm
  DEPENDS:=@!TARGET_mediatek_filogic +kmod-mediatek_hnat +kmod-sched-core +kmod-sched-cake +kmod-ifb +ip +kmod-nft-bridge +kmod-nft-connlimit +nftables-json +tc +ucode +ucode-mod-fs +ucode-mod-uci +ucode-mod-ubus
endef

define Package/mtk-qos/description
  Unified 4-coprocessor (PCE + PPE + QDMA + WED) hardware QoS controller for
  MediaTek Filogic NETSYS v3 (MT7987/MT7988). Combines non-mutually-exclusive
  per-IP/MAC EQoS hardware rate limiting & connection limits, zero-copy L7 DPI
  traffic classification, and 13-queue 5-tuple flow-fair SQM across Wired WAN
  (eth1/pppoe-wan/DSA) and 5G WWAN (wwan0_1), with automatic CAKE fallback
  only when HNAT is disabled.
endef

define Package/mtk-qos/conffiles
/etc/config/mtk_qos
endef

define Build/Compile
endef

define Package/mtk-qos/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/mtk_qos.config $(1)/etc/config/mtk_qos
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/mtk-qos $(1)/usr/sbin/mtk-qos
	$(INSTALL_DIR) $(1)/usr/share/mtk-qos
	$(INSTALL_BIN) ./files/render-nft.uc $(1)/usr/share/mtk-qos/render-nft.uc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/mtk_qos.init $(1)/etc/init.d/mtk_qos
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/95-mtk-qos.hotplug $(1)/etc/hotplug.d/iface/95-mtk-qos
	$(INSTALL_DIR) $(1)/etc/hotplug.d/dhcp
	$(INSTALL_DATA) ./files/95-mtk-qos.dhcp $(1)/etc/hotplug.d/dhcp/95-mtk-qos
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/95-mtk-qos-cleanup $(1)/etc/uci-defaults/95-mtk-qos-cleanup
endef

$(eval $(call BuildPackage,mtk-qos))
