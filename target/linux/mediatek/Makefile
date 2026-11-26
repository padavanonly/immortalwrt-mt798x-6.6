# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
BOARDNAME:=MediaTek ARM
SUBTARGETS:=filogic filogic_mtwifi filogic_hwifi filogic_hwifi_a73 mt7622 mt7623 mt7629
FEATURES:=dt-overlay emmc fpu gpio nand pci pcie rootfs-part separate_ramdisk squashfs usb

KERNEL_PATCHVER:=6.6

include $(INCLUDE_DIR)/target.mk

ifeq ($(TARGET_BUILD),1)
ifneq ($(filter filogic_mtwifi filogic_hwifi filogic_hwifi_a73,$(SUBTARGET)),)
  SUBTARGET_PATCH_DIR := $(PLATFORM_DIR)/vendor-patches-$(KERNEL_PATCHVER)
  SUBTARGET_FILES_DIR := "$(PLATFORM_DIR)/vendor-files-$(KERNEL_PATCHVER)"
endif
endif
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug ethtool \
	kmod-nft-socket kmod-nft-tproxy kmod-nft-nat kmod-inet-diag \
	kmod-ipt-nat6 kmod-ipt-nat \
	kmod-ipt-physdev kmod-nf-ipvs kmod-veth kmod-wireguard 

$(eval $(call BuildTarget))
