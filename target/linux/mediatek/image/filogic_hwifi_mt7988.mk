define Build/mt7988-bl2
	cat $(STAGING_DIR_IMAGE)/mt7988-$1-bl2.img >> $@
endef

define Build/mt7988-bl31-uboot
	cat $(STAGING_DIR_IMAGE)/mt7988_$1-u-boot.fip >> $@
endef

MTK_WIFI7_MT7992_BE7200_PKGS := \
	kmod-warp-mt7988 kmod-mt_wifi_cmn kmod-mt_wifi7-mt7992-be7200 kmod-mt_hwifi \
	kmod-mtk_pci kmod-mtk_wed kmod-connac_if kmod-mt7992 kmod-mt799a \
	mtwifi-cfg luci-app-mtwifi-cfg

MTK_WIFI7_MT7990_BE19000_PKGS := \
	kmod-warp-mt7988 kmod-mt_wifi_cmn kmod-mt_wifi7-mt7990-be19000 kmod-mt_hwifi \
	kmod-mtk_pci kmod-mtk_wed kmod-connac_if kmod-mt7990 kmod-mt7991 \
	mtwifi-cfg luci-app-mtwifi-cfg

define Device/adtran_smartrg
  DEVICE_VENDOR := Adtran
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := e2fsprogs f2fsck mkf2fs kmod-hwmon-pwmfan
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/arcadyan_mozart
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := Mozart
  DEVICE_DTS := mt7988a-arcadyan-mozart
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_PACKAGES := kmod-hwmon-pwmfan e2fsprogs f2fsck mkf2fs $(MTK_WIFI7_MT7990_BE19000_PKGS)
  KERNEL_LOADADDR := 0x46000000
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
        fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  KERNEL_INITRAMFS_SUFFIX := .itb
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
  ARTIFACTS := emmc-preloader.bin emmc-bl31-uboot.fip emmc-gpt.bin
  ARTIFACT/emmc-gpt.bin := mt798x-gpt emmc
  ARTIFACT/emmc-preloader.bin	:= mt7988-bl2 emmc-comb
  ARTIFACT/emmc-bl31-uboot.fip	:= mt7988-bl31-uboot arcadyan_mozart
  SUPPORTED_DEVICES += arcadyan,mozart
endef
TARGET_DEVICES += arcadyan_mozart

define Device/bananapi_bpi-r4
  DEVICE_MODEL := BPi-R4
  DEVICE_DTS := mt7988a-bananapi-bpi-r4
  DEVICE_DTS_CONFIG := config-mt7988a-bananapi-bpi-r4
  $(call Device/bananapi_bpi-r4-common)
endef
TARGET_DEVICES += bananapi_bpi-r4

define Device/bananapi_bpi-r4-common
  DEVICE_VENDOR := Bananapi
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_DTS_OVERLAY:= mt7988a-bananapi-bpi-r4-emmc mt7988a-bananapi-bpi-r4-rtc mt7988a-bananapi-bpi-r4-sd mt7988a-bananapi-bpi-r4-wifi-mt7996a
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_PACKAGES := kmod-hwmon-pwmfan kmod-i2c-mux-pca954x kmod-eeprom-at24 \
		     kmod-rtc-pcf8563 kmod-sfp kmod-usb3 e2fsprogs f2fsck mkf2fs mt7988-wo-firmware \
		     cpufreq luci-app-cpufreq luci-app-fancontrol $(MTK_WIFI7_MT7990_BE19000_PKGS)
  IMAGES := sysupgrade.itb
  KERNEL_LOADADDR := 0x46000000
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  ARTIFACTS := \
	       emmc-preloader.bin emmc-bl31-uboot.fip \
	       sdcard.img.gz \
	       snand-preloader.bin snand-bl31-uboot.fip
  ARTIFACT/emmc-preloader.bin	:= mt7988-bl2 emmc-comb
  ARTIFACT/emmc-bl31-uboot.fip	:= mt7988-bl31-uboot $$(DEVICE_NAME)-emmc
  ARTIFACT/snand-preloader.bin	:= mt7988-bl2 spim-nand-ubi-comb
  ARTIFACT/snand-bl31-uboot.fip	:= mt7988-bl31-uboot $$(DEVICE_NAME)-snand
  ARTIFACT/sdcard.img.gz	:= mt798x-gpt sdmmc |\
				   pad-to 17k | mt7988-bl2 sdmmc-comb |\
				   pad-to 6656k | mt7988-bl31-uboot $$(DEVICE_NAME)-sdmmc |\
				   pad-to 44M | mt7988-bl2 spim-nand-ubi-comb |\
				   pad-to 45M | mt7988-bl31-uboot $$(DEVICE_NAME)-snand |\
				   pad-to 51M | mt7988-bl2 emmc-comb |\
				   pad-to 52M | mt7988-bl31-uboot $$(DEVICE_NAME)-emmc |\
				   pad-to 56M | mt798x-gpt emmc |\
				$(if $(CONFIG_TARGET_ROOTFS_SQUASHFS),\
				   pad-to 64M | append-image squashfs-sysupgrade.itb | check-size |\
				) \
				  gzip
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  KERNEL			:= kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef

define Device/bananapi_bpi-r4-poe
  DEVICE_MODEL := BPi-R4 2.5GE
  DEVICE_DTS := mt7988a-bananapi-bpi-r4-poe
  DEVICE_DTS_CONFIG := config-mt7988a-bananapi-bpi-r4-poe
  $(call Device/bananapi_bpi-r4-common)
  DEVICE_PACKAGES += mt798x-2p5g-phy-firmware-internal
endef
TARGET_DEVICES += bananapi_bpi-r4-poe

define Device/bananapi_bpi-r4-pro
  DEVICE_MODEL := BPi-R4-PRO
  DEVICE_DTS := mt7988a-bananapi-bpi-r4-pro
  DEVICE_DTS_CONFIG := config-mt7988a-bananapi-bpi-r4-pro
  $(call Device/bananapi_bpi-r4-pro-common)
endef
TARGET_DEVICES += bananapi_bpi-r4-pro

define Device/bananapi_bpi-r4-pro-common
  DEVICE_VENDOR := Bananapi
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_DTS_OVERLAY:= mt7988a-bananapi-bpi-r4-emmc mt7988a-bananapi-bpi-r4-rtc mt7988a-bananapi-bpi-r4-sd mt7988a-bananapi-bpi-r4-wifi-mt7996a
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_PACKAGES := kmod-hwmon-pwmfan kmod-i2c-mux-pca954x kmod-eeprom-at24 $(MTK_WIFI7_MT7990_BE19000_PKGS) \
		     kmod-rtc-pcf8563 kmod-sfp kmod-usb3 e2fsprogs f2fsck mkf2fs mt7988-wo-firmware luci-app-fancontrol
  IMAGES := sysupgrade.itb
  KERNEL_LOADADDR := 0x46000000
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  KERNEL			:= kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef

define Device/mediatek_mt7988a-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7988A rfb
  DEVICE_DTS := mt7988a-rfb
  DEVICE_DTS_OVERLAY:= \
	mt7988a-rfb-emmc \
	mt7988a-rfb-sd \
	mt7988a-rfb-snfi-nand \
	mt7988a-rfb-spim-nand \
	mt7988a-rfb-spim-nand-factory \
	mt7988a-rfb-spim-nand-nmbm \
	mt7988a-rfb-spim-nor \
	mt7988a-rfb-eth0-gsw \
	mt7988a-rfb-eth1-aqr \
	mt7988a-rfb-eth1-an8831x \
	mt7988a-rfb-eth1-cux3410 \
	mt7988a-rfb-eth1-i2p5g-phy \
	mt7988a-rfb-eth1-mxl \
	mt7988a-rfb-eth1-sfp \
	mt7988a-rfb-eth2-aqr \
	mt7988a-rfb-eth2-an8831x \
	mt7988a-rfb-eth2-cux3410 \
	mt7988a-rfb-eth2-mxl \
	mt7988a-rfb-eth2-mxl86252 \
	mt7988a-rfb-eth2-sfp \
	mt7988a-rfb-spidev \
	mt7988a-rfb-4pcie \
	mt7988a-rfb-2pcie \
	mt7988d-rfb-2pcie
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_PACKAGES := mt798x-2p5g-phy-firmware-internal kmod-sfp blkid
  KERNEL_LOADADDR := 0x46000000
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | secure-boot-initramfs | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  KERNEL_INITRAMFS_SUFFIX := .itb
  KERNEL_IN_UBI := 1
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | secure-boot | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += mediatek_mt7988a-rfb

define Device/mediatek_mt7988a-rfb-mxl86252
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7988A rfb mxl86252
  DEVICE_DTS := mt7988a-rfb-mxl86252
  DEVICE_DTS_OVERLAY:= \
	mt7988a-rfb-spim-nand \
	mt7988a-rfb-spim-nand-factory \
	mt7988a-rfb-spim-nand-nmbm
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_PACKAGES := mt798x-2p5g-phy-firmware-internal kmod-sfp blkid
  KERNEL_LOADADDR := 0x46000000
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  KERNEL_INITRAMFS_SUFFIX := .itb
  KERNEL_IN_UBI := 1
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += mediatek_mt7988a-rfb-mxl86252

define Device/mediatek_mt7988d-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7988D rfb
  DEVICE_DTS := mt7988d-rfb
  DEVICE_DTS_OVERLAY:= \
	mt7988a-rfb-emmc \
	mt7988a-rfb-sd \
	mt7988a-rfb-snfi-nand \
	mt7988a-rfb-spim-nand \
	mt7988a-rfb-spim-nand-factory \
	mt7988a-rfb-spim-nand-nmbm \
	mt7988a-rfb-spim-nor \
	mt7988a-rfb-eth1-i2p5g-phy \
	mt7988a-rfb-eth2-aqr \
	mt7988a-rfb-eth2-mxl \
	mt7988a-rfb-spidev \
	mt7988d-rfb-eth2-sfp \
	mt7988d-rfb-eth0-gsw
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_PACKAGES := mt798x-2p5g-phy-firmware-internal kmod-sfp blkid
  KERNEL_LOADADDR := 0x46000000
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | secure-boot-initramfs | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  KERNEL_INITRAMFS_SUFFIX := .itb
  KERNEL_IN_UBI := 1
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | secure-boot | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += mediatek_mt7988d-rfb

define Device/smartrg_sdg-8733
$(call Device/adtran_smartrg)
  DEVICE_MODEL := SDG-8733
  DEVICE_DTS := mt7988a-smartrg-SDG-8733
  DEVICE_PACKAGES += $(MTK_WIFI7_MT7990_BE19000_PKGS) kmod-phy-aquantia kmod-usb3 mt7988-wo-firmware
endef
TARGET_DEVICES += smartrg_sdg-8733

define Device/smartrg_sdg-8733a
$(call Device/adtran_smartrg)
  DEVICE_MODEL := SDG-8733A
  DEVICE_DTS := mt7988d-smartrg-SDG-8733A
  DEVICE_PACKAGES += mt7988-2p5g-phy-firmware $(MTK_WIFI7_MT7990_BE19000_PKGS) kmod-phy-aquantia mt7988-wo-firmware
endef
TARGET_DEVICES += smartrg_sdg-8733a

define Device/smartrg_sdg-8734
$(call Device/adtran_smartrg)
  DEVICE_MODEL := SDG-8734
  DEVICE_DTS := mt7988a-smartrg-SDG-8734
  DEVICE_PACKAGES += $(MTK_WIFI7_MT7990_BE19000_PKGS) kmod-phy-aquantia kmod-sfp kmod-usb3 mt7988-wo-firmware
endef
TARGET_DEVICES += smartrg_sdg-8734

define Device/tplink_tl-7dr7230-rev1.0-sp2
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TL-7DR7230
  DEVICE_VARIANT := rev1.0-sp2
  DEVICE_DTS := mt7988d-tplink-tl-7dr7230-rev1.0-sp2
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS_LOADADDR := 0x45f00000
  DEVICE_PACKAGES := mt798x-2p5g-phy-firmware-internal $(MTK_WIFI7_MT7992_BE7200_PKGS) \
			 mt7988-wo-firmware kmod-phy-airoha-en8811h airoha-en8811h-firmware
  KERNEL_LOADADDR := 0x46000000
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  UBOOTENV_IN_UBI := 1
  IMAGES := sysupgrade.itb
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | \
	fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += tplink_tl-7dr7230-rev1.0-sp2

define Device/tplink_tl-7dr7299-v1
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TL-7DR7299
  DEVICE_VARIANT := v1
  DEVICE_DTS := mt7988a-tplink-tl-7dr7299-v1
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := \
	mt798x-2p5g-phy-firmware-internal mt7988-wo-firmware kmod-mt798x-2p5g-phy kmod-phy-rtl8261d kmod-switch-rtl837x swconfig kmod-sfp kmod-usb3 automount \
	mtk-qos luci-app-mtk-qos luci-app-eqos-mtk luci-app-turboacc-mtk cpufreq luci-app-cpufreq $(MTK_WIFI7_MT7992_BE7200_PKGS) \
	kmod-gpio-button-hotplug kmod-leds-gpio luci-light \
	kmod-crypto-hw-safexcel
  KERNEL_LOADADDR := 0x48000000
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  UBOOTENV_IN_UBI := 1
  IMAGES := sysupgrade.itb
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | \
	fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += tplink_tl-7dr7299-v1

define Device/tplink_tl-7dr7299-v1-dsa
  DEFAULT := n
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TL-7DR7299
  DEVICE_VARIANT := v1-dsa
  DEVICE_DTS := mt7988a-tplink-tl-7dr7299-v1-dsa
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := \
	mt798x-2p5g-phy-firmware-internal mt7988-wo-firmware kmod-mt798x-2p5g-phy kmod-phy-rtl8261d-backport kmod-rtl8372n-dsa kmod-sfp kmod-usb3 automount \
	mtk-qos luci-app-mtk-qos luci-app-eqos-mtk luci-app-turboacc-mtk cpufreq luci-app-cpufreq $(MTK_WIFI7_MT7992_BE7200_PKGS) \
	kmod-gpio-button-hotplug kmod-leds-gpio luci-light \
	kmod-crypto-hw-safexcel
  KERNEL_LOADADDR := 0x48000000
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  UBOOTENV_IN_UBI := 1
  IMAGES := sysupgrade.itb
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | \
	fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += tplink_tl-7dr7299-v1-dsa
