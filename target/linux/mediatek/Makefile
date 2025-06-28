# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
BOARDNAME:=MediaTek ARM
SUBTARGETS:=filogic mt7622 mt7623 mt7629
FEATURES:=dt-overlay emmc fpu gpio nand pci pcie rootfs-part separate_ramdisk squashfs usb

KERNEL_PATCHVER:=6.6

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
		kmod-leds-gpio kmod-gpio-button-hotplug autocore kmod-sfp \
		kmod-mediatek_hnat mtk-smp mtkhqos_util ip6tables-nft \
        	ebtables kmod-ebtables kmod-ebtables-ipv4 kmod-ebtables-ipv6 \
        	luci-app-wrtbwmon luci-app-eqos-mtk luci-app-turboacc-mtk \
        	switch

$(eval $(call BuildTarget))
