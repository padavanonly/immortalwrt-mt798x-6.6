include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-cpe
PKG_VERSION:=2.0
PKG_RELEASE:=2

LUCI_TITLE:=LuCI Support for CPE
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+sendat +ucode +ucode-mod-fs +ucode-mod-uci +ucode-mod-ubus \
	+kmod-usb-net +kmod-usb-net-cdc-ether +kmod-usb-acm \
	+kmod-usb-net-qmi-wwan +kmod-usb-net-rndis +kmod-usb-serial-qualcomm \
	+kmod-usb-net-sierrawireless +kmod-usb-ohci +kmod-usb-serial \
	+kmod-usb-serial-option +kmod-usb-wdm \
	+kmod-usb2 +kmod-usb3 \
	+quectel-CM-5G +kmod-usb-net-cdc-mbim +usbutils

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

