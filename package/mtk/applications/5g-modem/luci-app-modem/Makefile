# Copyright (C) 2023 Siriling <siriling@qq.com>
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-modem
LUCI_TITLE:=LuCI JS support for 5G Modem
LUCI_PKGARCH:=all
PKG_VERSION:=1.4.5
PKG_RELEASE:=4
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Siriling <siriling@qq.com>
LUCI_DEPENDS:=+ucode +ucode-mod-fs +ucode-mod-uci +ucode-mod-ubus \
		+kmod-usb2 +kmod-usb3 \
		+kmod-usb-net-sierrawireless +kmod-usb-ohci \
		+kmod-usb-serial-option +kmod-usb-serial +kmod-usb-serial-qualcomm \
		+kmod-usb-net +kmod-usb-acm \
		+kmod-usb-wdm +kmod-usb-net-qmi-wwan \
		+kmod-usb-net-cdc-ether \
		+kmod-usb-net-cdc-mbim \
		+kmod-usb-net-rndis \
		+kmod-usb-net-cdc-ncm \
		+usbutils \
		+kmod-pcie_mhi \
		+pciutils \
		+quectel-CM-5G \
		+sms-tool \
		+bc +jq +rdisc6 +ndisc6
DEPENDS+=+ucode +ucode-mod-fs +ucode-mod-uci +ucode-mod-ubus

define Package/luci-app-modem/conffiles
/etc/config/modem
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
