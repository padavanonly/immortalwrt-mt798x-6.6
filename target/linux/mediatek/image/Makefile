# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016-2017 LEDE project

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += NETGEAR_ENC_MODEL NETGEAR_ENC_REGION NETGEAR_ENC_HW_ID_LIST NETGEAR_ENC_MODEL_LIST

loadaddr-$(CONFIG_TARGET_mediatek_mt7622) := 0x44000000
loadaddr-$(CONFIG_TARGET_mediatek_mt7623) := 0x80008000
loadaddr-$(CONFIG_TARGET_mediatek_mt7629) := 0x40008000
loadaddr-$(CONFIG_TARGET_mediatek_filogic) := 0x48000000

# default all platform image(fit) build
define Device/Default
  PROFILES = Default $$(DEVICE_NAME)
  KERNEL_NAME := Image
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd
  KERNEL_LOADADDR = $(loadaddr-y)
  FILESYSTEMS := squashfs
  DEVICE_DTS_DIR := $(DTS_DIR)
  NETGEAR_ENC_MODEL :=
  NETGEAR_ENC_REGION :=
  NETGEAR_ENC_HW_ID_LIST :=
  NETGEAR_ENC_MODEL_LIST :=
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-rootfs | \
	pad-rootfs | append-metadata
endef

define Image/get_fw_ar_ver
	$(shell sed -n 's/^FW_AR_VER\t:=\t*//p' $(TOPDIR)/$(call qstrip,$(CONFIG_AUTO_AR_CONF)))
endef

define Image/gen-fw-ar-ver
	rm -rf $(TOPDIR)/$(call qstrip,$(CONFIG_AUTO_AR_CONF))
	$(STAGING_DIR_HOST)/bin/ar-tool fw_ar_table create_ar_conf \
		$(TOPDIR)/$(call qstrip,$(CONFIG_ANTI_ROLLBACK_TABLE)) \
		$(TOPDIR)/$(call qstrip,$(CONFIG_AUTO_AR_CONF))
endef

include $(SUBTARGET).mk
-include $(SUBTARGET)-extra.mk

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Build/append-opteenode
	$(STAGING_DIR_HOSTPKG)/bin/fdtput $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb "/reserved-memory/secmon@43000000" \
		-tx reg 0 0x43000000 0 $(shell printf "%#x" $$((0x570000 \
		$(if $(CONFIG_OPTEE_CORE_RESERVED_SHMEM),+ $(CONFIG_OPTEE_RESERVED_SHMEM_SIZE),))))
	$(STAGING_DIR_HOSTPKG)/bin/fdtput $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /firmware -cp
	$(STAGING_DIR_HOSTPKG)/bin/fdtput $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /firmware/optee -cp
	$(STAGING_DIR_HOSTPKG)/bin/fdtput $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /firmware/optee -ts compatible "linaro,optee-tz"
	$(STAGING_DIR_HOSTPKG)/bin/fdtput $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /firmware/optee -ts method "smc"
	$(STAGING_DIR_HOSTPKG)/bin/fdtput $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /firmware/optee -ts status "okay"
endef

# build squashfs-hashed
define Build/squashfs-hashed
	$(TOPDIR)/scripts/make-squashfs-hashed.sh \
		$(ROOTFS/$(FILESYSTEMS)/$(DEVICE_NAME)) \
		$(STAGING_DIR_HOSTPKG) \
		$(ROOTFS/$(FILESYSTEMS)/$(DEVICE_NAME))-hashed-summary
	fdt-patch-dm-verify $(ROOTFS/$(FILESYSTEMS)/$(DEVICE_NAME))-hashed-summary \
		$(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb
endef

define hkdf
	HOST_BIN=$(STAGING_DIR_HOST)/bin \
	HOSTPKG_BIN=$(STAGING_DIR_HOSTPKG)/bin \
	$(TOPDIR)/scripts/fw-enc-hkdf.sh -i $(1) -o $(2) -t $(3)
endef

define Image/fw-enc-key-derive
	$(call hkdf,$(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/$(CONFIG_PLAT_KEY_NAME).bin), \
		    $(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/roe_key.bin), \
		    $(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/$(CONFIG_ROE_SALT)))

	$(call hkdf,$(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/roe_key.bin), \
		    $(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/kernel_key.bin), \
		    $(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/$(CONFIG_KERNEL_SALT)))

	$(call hkdf,$(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/roe_key.bin), \
		    $(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/rootfs_key.bin), \
		    $(TOPDIR)/$(call qstrip,$(CONFIG_SBC_KEY_DIR)/$(CONFIG_ROOTFS_SALT)))
endef

define Build/change-rootdev-to-ramdisk
	$(STAGING_DIR_HOSTPKG)/bin/fdtget -t s \
		$(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /chosen bootargs | \
		sed "s/root=[^ ]*/root=\/dev\/ram/" | xargs -I {} \
	$(STAGING_DIR_HOSTPKG)/bin/fdtput -t s \
		$(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb /chosen bootargs "{}"
endef

define Build/secure-boot
	$(if $(CONFIG_MTK_SECURE_BOOT), \
		$(if $(CONFIG_MTK_FW_ENC_USE_RAMDISK),,$(call Build/squashfs-hashed)))
	$(if $(CONFIG_PACKAGE_optee-mediatek),$(call Build/append-opteenode))
	$(if $(CONFIG_MTK_FW_ENC_USE_RAMDISK),$(call Build/change-rootdev-to-ramdisk))
endef

define Build/secure-boot-initramfs
	$(if $(CONFIG_PACKAGE_optee-mediatek),$(call Build/append-opteenode))
endef

$(eval $(call BuildImage))

