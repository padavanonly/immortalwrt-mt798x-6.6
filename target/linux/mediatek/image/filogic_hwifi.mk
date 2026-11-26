DTS_DIR := $(DTS_DIR)/mediatek

define Image/Prepare
	# For UBI we want only one extra block
	rm -f $(KDIR)/ubi_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/ubi_mark
	$(if $(CONFIG_MTK_FW_ENC),$(call Image/fw-enc-key-derive))
	$(if $(CONFIG_MTK_ANTI_ROLLBACK),$(call Image/gen-fw-ar-ver))
endef

define Build/mt7981-bl2
	cat $(STAGING_DIR_IMAGE)/mt7981-$1-bl2.img >> $@
endef

define Build/mt7981-bl31-uboot
	cat $(STAGING_DIR_IMAGE)/mt7981_$1-u-boot.fip >> $@
endef

define Build/mt7986-bl2
	cat $(STAGING_DIR_IMAGE)/mt7986-$1-bl2.img >> $@
endef

define Build/mt7986-bl31-uboot
	cat $(STAGING_DIR_IMAGE)/mt7986_$1-u-boot.fip >> $@
endef

define Build/mt798x-gpt
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -g -o $@.tmp -a 1 -l 1024 \
		$(if $(findstring sdmmc,$1), \
			-H \
			-t 0x83	-N bl2		-r	-p 4079k@17k \
		) \
			-t 0x83	-N ubootenv	-r	-p 512k@4M \
			-t 0x83	-N factory	-r	-p 2M@4608k \
			-t 0xef	-N fip		-r	-p 4M@6656k \
				-N recovery	-r	-p 32M@12M \
		$(if $(findstring sdmmc,$1), \
				-N install	-r	-p 20M@44M \
			-t 0x2e -N production		-p $(CONFIG_TARGET_ROOTFS_PARTSIZE)M@64M \
		) \
		$(if $(findstring emmc,$1), \
			-t 0x2e -N production		-p $(CONFIG_TARGET_ROOTFS_PARTSIZE)M@64M \
		)
	cat $@.tmp >> $@
	rm $@.tmp
endef

metadata_gl_json = \
	'{ $(if $(IMAGE_METADATA),$(IMAGE_METADATA)$(comma)) \
		"metadata_version": "1.1", \
		"compat_version": "$(call json_quote,$(compat_version))", \
		$(if $(DEVICE_COMPAT_MESSAGE),"compat_message": "$(call json_quote,$(DEVICE_COMPAT_MESSAGE))"$(comma)) \
		$(if $(filter-out 1.0,$(compat_version)),"new_supported_devices": \
			[$(call metadata_devices,$(SUPPORTED_DEVICES))]$(comma) \
			"supported_devices": ["$(call json_quote,$(legacy_supported_message))"]$(comma)) \
		$(if $(filter 1.0,$(compat_version)),"supported_devices":[$(call metadata_devices,$(SUPPORTED_DEVICES))]$(comma)) \
		"version": { \
			"release": "$(call json_quote,$(VERSION_NUMBER))", \
			"date": "$(shell TZ='Asia/Chongqing' date '+%Y%m%d%H%M%S')", \
			"dist": "$(call json_quote,$(VERSION_DIST))", \
			"version": "$(call json_quote,$(VERSION_NUMBER))", \
			"revision": "$(call json_quote,$(REVISION))", \
			"target": "$(call json_quote,$(TARGETID))", \
			"board": "$(call json_quote,$(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)))" \
		} \
	}'

define Build/append-gl-metadata
	$(if $(SUPPORTED_DEVICES),-echo $(call metadata_gl_json,$(SUPPORTED_DEVICES)) | fwtool -I - $@)
	sha256sum "$@" | cut -d" " -f1 > "$@.sha256sum"
	[ ! -s "$(BUILD_KEY)" -o ! -s "$(BUILD_KEY).ucert" -o ! -s "$@" ] || { \
		cp "$(BUILD_KEY).ucert" "$@.ucert" ;\
		usign -S -m "$@" -s "$(BUILD_KEY)" -x "$@.sig" ;\
		ucert -A -c "$@.ucert" -x "$@.sig" ;\
		fwtool -S "$@.ucert" "$@" ;\
	}
endef

define Build/append-openwrt-one-eeprom
	dd if=$(STAGING_DIR_IMAGE)/mt7981_eeprom_mt7976_dbdc.bin >> $@
endef

define Build/zyxel-nwa-fit-filogic
	$(TOPDIR)/scripts/mkits-zyxel-fit-filogic.sh \
		$@.its $@ "80 e1 ff ff ff ff ff ff ff ff"
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv $@.new $@
endef

define Build/cetron-header
	$(eval magic=$(word 1,$(1)))
	$(eval model=$(word 2,$(1)))
	( \
		dd if=/dev/zero bs=856 count=1 2>/dev/null; \
		printf "$(model)," | dd bs=128 count=1 conv=sync 2>/dev/null; \
		md5sum $@ | cut -f1 -d" " | dd bs=32 count=1 2>/dev/null; \
		printf "$(magic)" | dd bs=4 count=1 conv=sync 2>/dev/null; \
		cat $@; \
	) > $@.tmp
	fw_crc=$$(gzip -c $@.tmp | tail -c 8 | od -An -N4 -tx4 --endian little | tr -d ' \n'); \
	printf "$$(echo $$fw_crc | sed 's/../\\x&/g')" | cat - $@.tmp > $@
	rm $@.tmp
endef

MTK_WIFI7_MT7992_BE5040_PKGS := \
	kmod-warp-mt7987 kmod-mt_wifi_cmn kmod-mt_wifi7-mt7992-be5040 kmod-mt_hwifi \
	kmod-mtk_pci kmod-mtk_wed kmod-connac_if kmod-mt7992 kmod-mt799a \
	mtwifi-cfg luci-app-mtwifi-cfg

define Device/bananapi_bpi-r4-lite
  DEVICE_VENDOR := Bananapi
  DEVICE_MODEL := BPi-R4 Lite
  DEVICE_DTS := mt7987a-bananapi-bpi-r4-lite
  DEVICE_DTS_OVERLAY:= \
	mt7987-spim-nand \
	mt7987-spidev \
	mt7987-spim-nor \
	mt7987-emmc \
	mt7987-sd \
	mt7987a-bananapi-bpi-r4-lite-1pcie-2L \
	mt7987a-bananapi-bpi-r4-lite-2pcie-1L
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTC_FLAGS := --pad 4096
  DEVICE_DTS_LOADADDR := 0x4ff00000
  DEVICE_PACKAGES := \
  mt798x-2p5g-phy-firmware-internal kmod-sfp blkid \
  kmod-i2c-mux-pca954x kmod-eeprom-at24 kmod-rtc-pcf8563 \
  kmod-usb3 kmod-hwmon-pwmfan luci-app-fancontrol
  KERNEL_LOADADDR := 0x40000000
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
        fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGES := sysupgrade.itb
  KERNEL_INITRAMFS_SUFFIX := .itb
  KERNEL_IN_UBI := 1
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-with-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += bananapi_bpi-r4-lite

define Device/hiveton-h5000m
  DEVICE_VENDOR := Hiveton
  DEVICE_MODEL := H5000M
  DEVICE_DTS := mt7987a-hiveton-h5000m
  DEVICE_DTS_DIR := ../dts
  KERNEL_LOADADDR := 0x40080000
  DEVICE_PACKAGES := \
	mt798x-2p5g-phy-firmware-internal $(MTK_WIFI7_MT7992_BE5040_PKGS) mtk-qos luci-app-mtk-qos luci-app-eqos-mtk luci-app-turboacc-mtk cpufreq luci-app-cpufreq \
	kmod-gpio-button-hotplug kmod-leds-gpio kmod-hwmon-pwmfan luci-light luci-app-fancontrol \
	kmod-crypto-hw-safexcel \
	kmod-usb3 luci-app-modem sms-tool quectel-CM-5G kmod-usb-acm kmod-usb-net kmod-usb-net-cdc-ether kmod-usb-net-cdc-mbim kmod-usb-net-cdc-ncm kmod-usb-net-huawei-cdc-ncm kmod-usb-net-qmi-wwan kmod-qmi_wwan_q kmod-qmi_wwan_f kmod-usb-net-rndis kmod-usb-serial kmod-usb-serial-option kmod-usb-serial-qualcomm kmod-usb-serial-wwan kmod-usb-wdm
  SUPPORTED_DEVICES += hiveton,h5000m
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += hiveton-h5000m

define Device/mediatek_mt7987a-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7987A rfb (DT-overlay)
  DEVICE_DTS := mt7987a-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/
  KERNEL_LOADADDR := 0x40080000
  DEVICE_PACKAGES := mt798x-2p5g-phy-firmware-internal kmod-sfp blkid
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mediatek_mt7987a-rfb

include filogic_hwifi_mt7988.mk

