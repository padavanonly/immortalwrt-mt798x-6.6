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
	kmod-leds-gpio kmod-gpio-button-hotplug ethtool \
	kmod-nft-socket kmod-nft-tproxy kmod-nft-nat kmod-inet-diag \
	kmod-ipt-nat6 kmod-ipt-nat \
	kmod-ipt-physdev kmod-nf-ipvs kmod-veth kmod-wireguard 

$(eval $(call BuildTarget))
