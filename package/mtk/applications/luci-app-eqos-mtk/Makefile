include $(TOPDIR)/rules.mk

PKG_RELEASE:=5

LUCI_TITLE:=LuCI support for Easy QoS
LUCI_DEPENDS:=@!TARGET_mediatek_filogic +luci-base +rpcd +firewall4 +nftables-json +ip +tc +kmod-sched-core +kmod-ifb +bash +ucode +ucode-mod-fs +ucode-mod-uci +ucode-mod-ubus @!PACKAGE_luci-app-eqos

PKG_MAINTAINER:=Jianhui Zhao <jianhuizhao329@gmail.com>
PKG_NAME:=luci-app-eqos-mtk

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
