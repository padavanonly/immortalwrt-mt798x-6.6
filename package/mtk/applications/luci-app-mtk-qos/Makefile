# SPDX-License-Identifier: Apache-2.0

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mtk-qos
PKG_RELEASE:=3
PKG_LICENSE:=Apache-2.0

LUCI_TITLE:=LuCI support for MediaTek NETSYS v3 4-Coprocessor Unified QoS
LUCI_DEPENDS:=@!TARGET_mediatek_filogic +mtk-qos +luci-base +rpcd +ucode +ucode-mod-fs +ucode-mod-uci
LUCI_PKGARCH:=all

PROVIDES:=luci-app-mtk-npu-sqm

define Build/Prepare/$(PKG_NAME)
	chmod 0755 $(PKG_BUILD_DIR)/root/usr/libexec/rpcd/luci.mtk_qos
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$$IPKG_INSTROOT" ]; then
	[ -f /usr/libexec/rpcd/luci.mtk_qos ] && chmod 0755 /usr/libexec/rpcd/luci.mtk_qos
	rm -rf /tmp/luci-modulecache/ /var/luci-modulecache/
	rm -f /tmp/luci-indexcache /tmp/luci-indexcache.* /var/luci-indexcache.* 2>/dev/null
	[ -x /etc/init.d/mtk_qos ] && /etc/init.d/mtk_qos enable >/dev/null 2>&1 || true
	[ -x /etc/init.d/rpcd ] && /etc/init.d/rpcd reload >/dev/null 2>&1 || true
fi
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
