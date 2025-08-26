include $(TOPDIR)/rules.mk

PKG_NAME:=mt798x-2p5g-phy-firmware-internal
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/mt798x-2p5g-phy-firmware-internal
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Misc
  TITLE:=Ethernet Phy Utilities
  VERSION:=$(PKG_RELEASE)-$(REVISION)
endef

define Package/mt798x-2p5g-phy-firmware-internal
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=MT798x 2.5Gphy firmware's internal version
endef

define Package/mt798x-2p5g-phy-firmware-internal/description
  MT798x 2.5Gphy firmware's internal version
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/mt798x-2p5g-phy-firmware-internal/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7988
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek/mt7987

	cp ./files/mt7988/i2p5ge-phy-pmb.bin  $(1)/lib/firmware/mediatek/mt7988/i2p5ge-phy-pmb.bin

	cp ./files/mt7987/i2p5ge-phy-pmb.bin  $(1)/lib/firmware/mediatek/mt7987/i2p5ge-phy-pmb.bin
	cp ./files/mt7987/i2p5ge-phy-DSPBitTb.bin  $(1)/lib/firmware/mediatek/mt7987/i2p5ge-phy-DSPBitTb.bin
endef

$(eval $(call BuildPackage,mt798x-2p5g-phy-firmware-internal))
