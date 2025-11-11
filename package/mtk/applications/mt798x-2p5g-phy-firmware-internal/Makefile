include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt798x-2p5g-phy-firmware-internal
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt798x-2p5g-phy
  SECTION:=kernel
  SUBMENU:=Network Devices
  TITLE:=MediaTek MT798x 2.5G PHY driver
  DEPENDS:=@TARGET_mediatek +mt798x-2p5g-phy-firmware-internal
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
  TITLE:=MT798x 2.5Gphy firmware's internal version
endef

define Package/mt798x-2p5g-phy-firmware-internal/description
  MT798x 2.5Gphy firmware's internal version
endef

define Package/mt798x-2p5g-phy-firmware-internal/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7988
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7987

	cp ./files/mt7988/i2p5ge-phy-pmb.bin  $(1)/lib/firmware/mediatek/mt7988/i2p5ge-phy-pmb.bin

	cp ./files/mt7987/i2p5ge-phy-pmb.bin  $(1)/lib/firmware/mediatek/mt7987/i2p5ge-phy-pmb.bin
	cp ./files/mt7987/i2p5ge-phy-DSPBitTb.bin  $(1)/lib/firmware/mediatek/mt7987/i2p5ge-phy-DSPBitTb.bin
endef

$(eval $(call BuildPackage,mt798x-2p5g-phy-firmware-internal))
