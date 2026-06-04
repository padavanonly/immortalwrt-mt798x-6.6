define KernelPackage/ata-ahci-mtk
  TITLE:=Mediatek AHCI Serial ATA support
  KCONFIG:=CONFIG_AHCI_MTK
  FILES:= \
	$(LINUX_DIR)/drivers/ata/ahci_mtk.ko \
	$(LINUX_DIR)/drivers/ata/libahci_platform.ko
  AUTOLOAD:=$(call AutoLoad,40,libahci libahci_platform ahci_mtk,1)
  $(call AddDepends/ata)
  DEPENDS+=@(TARGET_mediatek_mt7622||TARGET_mediatek_mt7623)
endef

define KernelPackage/ata-ahci-mtk/description
 Mediatek AHCI Serial ATA host controllers
endef

$(eval $(call KernelPackage,ata-ahci-mtk))

define KernelPackage/btmtkuart
  SUBMENU:=Other modules
  TITLE:=MediaTek HCI UART driver
  DEPENDS:=@TARGET_mediatek_mt7622 +kmod-bluetooth +mt7622bt-firmware
  KCONFIG:=CONFIG_BT_MTKUART
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/btmtkuart.ko
  AUTOLOAD:=$(call AutoProbe,btmtkuart)
endef

$(eval $(call KernelPackage,btmtkuart))

define KernelPackage/iio-mt6577-auxadc
  TITLE:=Mediatek AUXADC driver
  DEPENDS:=@(TARGET_mediatek_mt7622||TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_MEDIATEK_MT6577_AUXADC
  FILES:= \
	$(LINUX_DIR)/drivers/iio/adc/mt6577_auxadc.ko
  AUTOLOAD:=$(call AutoProbe,mt6577_auxadc)
  $(call AddDepends/iio)
endef
$(eval $(call KernelPackage,iio-mt6577-auxadc))

define KernelPackage/air-an8855-phy
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Airoha AN8855 PHY driver
  DEPENDS:=@(TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_AIR_AN8855_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/air_an8855.ko
  AUTOLOAD:=$(call AutoProbe,air_an8855)
endef

define KernelPackage/air-an8855-phy/description
  Airoha AN8855 10G PHY driver
endef

$(eval $(call KernelPackage,air-an8855-phy))

define KernelPackage/an8855-gsw
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Airoha AN8855 GSW driver
  DEPENDS:=@(TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_AN8855_GSW
  FILES:=$(LINUX_DIR)/drivers/net/phy/airoha/an8855/an8855.ko
  AUTOLOAD:=$(call AutoProbe,an8855)
endef

define KernelPackage/an8855-gsw/description
  Airoha AN8855 gigabit switch driver
endef

$(eval $(call KernelPackage,an8855-gsw))

define KernelPackage/mdio-an8855
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Airoha AN8855 MDIO driver
  DEPENDS:=@(TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_MDIO_AN8855
  FILES:=$(LINUX_DIR)/drivers/net/mdio/mdio-an8855.ko
  AUTOLOAD:=$(call AutoProbe,mdio-an8855)
endef

define KernelPackage/mdio-an8855/description
  Airoha AN8855 MDIO interface driver
endef

$(eval $(call KernelPackage,mdio-an8855))

define KernelPackage/mfd-airoha-an8855
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Airoha AN8855 MFD driver
  DEPENDS:=@(TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_MFD_AIROHA_AN8855
  FILES:=$(LINUX_DIR)/drivers/mfd/airoha-an8855.ko
  AUTOLOAD:=$(call AutoProbe,airoha-an8855)
endef

define KernelPackage/mfd-airoha-an8855/description
  Airoha AN8855 multi-function device driver
endef

$(eval $(call KernelPackage,mfd-airoha-an8855))

define KernelPackage/net-dsa-an8855
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Airoha AN8855 DSA driver
  DEPENDS:=@(TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_NET_DSA_AN8855
  FILES:=$(LINUX_DIR)/drivers/net/dsa/an8855.ko
  AUTOLOAD:=$(call AutoProbe,an8855)
endef

define KernelPackage/net-dsa-an8855/description
  Airoha AN8855 DSA switch driver
endef

$(eval $(call KernelPackage,net-dsa-an8855))

define KernelPackage/nvmem-an8855-efuse
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Airoha AN8855 eFuse NVMEM driver
  DEPENDS:=@(TARGET_mediatek_filogic||TARGET_mediatek_filogic_a73)
  KCONFIG:=CONFIG_NVMEM_AN8855_EFUSE
  FILES:=$(LINUX_DIR)/drivers/nvmem/an8855-efuse.ko
  AUTOLOAD:=$(call AutoProbe,an8855_efuse)
endef

define KernelPackage/nvmem-an8855-efuse/description
  Airoha AN8855 eFuse NVMEM driver
endef

$(eval $(call KernelPackage,nvmem-an8855-efuse))
