include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt798x-2p5g-phy-firmware-internal
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_mediatek_filogic_hwifi \
	CONFIG_TARGET_mediatek_filogic_hwifi_a73 \
	CONFIG_WARP_CHIPSET

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt798x-2p5g-phy
  SECTION:=kernel
  SUBMENU:=Network Devices
  TITLE:=MediaTek MT798x 2.5G PHY driver
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +mt798x-2p5g-phy-firmware-internal
  KCONFIG:=CONFIG_MEDIATEK_2P5GE_PHY=m
  FILES:=$(LINUX_DIR)/drivers/net/phy/mediatek/mtk-2p5ge.ko
  AUTOLOAD:=$(call AutoLoad,73,mtk-2p5ge)
endef

define KernelPackage/mt798x-2p5g-phy/description
  MediaTek MT798x 2.5G PHY kernel driver with auto-load support
endef

define Build/Compile
endef

$(eval $(call KernelPackage,mt798x-2p5g-phy))


define Package/mt798x-2p5g-phy-firmware-internal
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73)
  CONFLICTS:=mt7988-2p5g-phy-firmware
  TITLE:=MT798x 2.5Gphy firmware's internal version
endef

define Package/mt798x-2p5g-phy-firmware-internal/description
  MT798x 2.5Gphy firmware's internal version
endef

define Package/mt798x-2p5g-phy-firmware-internal/install
	if [ "$(CONFIG_TARGET_mediatek_filogic_hwifi)" = "y" ]; then \
		$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7987 $(1)/lib/firmware/mediatek/mt7988; \
		cp ./files/mt7987/i2p5ge-phy-pmb.bin $(1)/lib/firmware/mediatek/mt7987/; \
		cp ./files/mt7987/i2p5ge-phy-DSPBitTb.bin $(1)/lib/firmware/mediatek/mt7987/; \
		cp ./files/mt7988/i2p5ge-phy-pmb.bin $(1)/lib/firmware/mediatek/mt7988/; \
	elif [ "$(CONFIG_WARP_CHIPSET)" = "mt7988" -o "$(CONFIG_TARGET_mediatek_filogic_hwifi_a73)" = "y" ]; then \
		$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7988; \
		cp ./files/mt7988/i2p5ge-phy-pmb.bin $(1)/lib/firmware/mediatek/mt7988/; \
	elif [ "$(CONFIG_WARP_CHIPSET)" = "mt7987" ]; then \
		$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7987; \
		cp ./files/mt7987/i2p5ge-phy-pmb.bin $(1)/lib/firmware/mediatek/mt7987/; \
		cp ./files/mt7987/i2p5ge-phy-DSPBitTb.bin $(1)/lib/firmware/mediatek/mt7987/; \
	fi
endef

$(eval $(call BuildPackage,mt798x-2p5g-phy-firmware-internal))
