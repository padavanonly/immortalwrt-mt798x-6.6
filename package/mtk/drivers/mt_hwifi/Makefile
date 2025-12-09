# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt_hwifi
WIFI_DRIVER_NAME:=mt_hwifi
P4REV:=
PKG_VERSION:=TEST
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(WIFI_DRIVER_NAME)
PKG_KCONFIG:= \
	SUPPORT_OPENWRT \
	CCN34_SPLIT_MAC_SUPPORT \
        CCN67_SPLIT_MAC_SUPPORT \

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MTK_WIFI7_$c),CONFIG_$(c)))

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

define KernelPackage/mt_hwifi
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HW abstraction layer driver
	DEPENDS:=+kmod-mt_wifi7 +kmod-mt_wifi_cmn
	FILES:=$(PKG_BUILD_DIR)/wlan_hwifi/mtk_hwifi.ko
	AUTOLOAD:=$(call AutoLoad,62,mtk_hwifi)
	SUBMENU:=Drivers
	MENU:=1
endef

define KernelPackage/mt_hwifi/description
	Provide HW abstraction layer for CONNAC3/5 interface, mac80211 driver
endef

define KernelPackage/mt_hwifi/config
	source "$(SOURCE)/config.in"
endef

define KernelPackage/mtk_pci
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI PCI driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_PCI_SUPPORT +kmod-mt_wifi_cmn
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mtk_pci.ko
	AUTOLOAD:=$(call AutoLoad,63,mtk_pci)
	SUBMENU:=Drivers
endef

define KernelPackage/mtk_wed
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI WED driver
	DEPENDS:=kmod-mt_hwifi kmod-warp kmod-mtk_pci @MTK_HWIFI_WED_SUPPORT
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mtk_wed.ko
	AUTOLOAD:=$(call AutoLoad,64,mtk_wed)
	SUBMENU:=Drivers
endef

define KernelPackage/en_npu
	CATEGORY:=MTK Properties
	TITLE:=AIROHA HWIFI NPU driver
	DEPENDS:=kmod-mt_hwifi kmod-warp kmod-mtk_pci kmod-mtk_wed @EN_HWIFI_NPU_SUPPORT
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/en_npu.ko
	AUTOLOAD:=$(call AutoLoad,65,en_npu)
	SUBMENU:=Drivers
endef

define KernelPackage/connac_if
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI CONNAC3/5 interface layer driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_CONNAC_IF_SUPPORT
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/connac_if.ko
	AUTOLOAD:=$(call AutoLoad,63,connac_if)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7915_
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7915 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT7915
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7915.ko \
	       $(PKG_BUILD_DIR)/wlan_hwifi/mt7915_dbg.ko
	AUTOLOAD:=$(call AutoLoad,66,mt7915)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7916_
        CATEGORY:=MTK Properties
        TITLE:=MediaTek HWIFI MT7916 driver
        DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT7916
        FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7916.ko
        AUTOLOAD:=$(call AutoLoad,66,mt7916)
        SUBMENU:=Drivers
endef

define KernelPackage/bellwether
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI Bellwether driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_BELLWETHER
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/bellwether.ko
	AUTOLOAD:=$(call AutoLoad,66,bellwether)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7990
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7990 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT7990
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7990.ko
	MODPARAMS.mt7990:=option_type=$(CONFIG_MTK_HWIFI_MT7990_OPTION_TYPE) \
			  rro_enable=$(CONFIG_MTK_HWIFI_MT7990_RRO_SUPPORT) \
			  intr_option_set=$(CONFIG_MTK_HWIFI_MT7990_INTR_OPTION_SET)
	AUTOLOAD:=$(call AutoLoad,66,mt7990)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7990/config
	if PACKAGE_kmod-mt7990

	config MTK_HWIFI_MT7990_RRO_SUPPORT
		string "HWIFI MT7990 RRO Support"
		default Y
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_MT7990_OPTION_TYPE
		int "HWIFI MT7990 Option Type"
		default 1
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_MT7990_INTR_OPTION_SET
		int "HWIFI MT7990 Interrupt Option Set"
		default 0
		depends on PACKAGE_kmod-mt_hwifi

	endif #PACKAGE_kmod-mt7990#
endef

define KernelPackage/mt7990_dbg
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7990 Debug driver
	DEPENDS:=kmod-mt7990 kmod-mtk_pci @MTK_HWIFI_MT7990
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7990_dbg.ko
	SUBMENU:=Drivers
	DEFAULT:=PACKAGE_kmod-mt7990
endef

define KernelPackage/mt7991
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7991 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT7991
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7991.ko
	AUTOLOAD:=$(call AutoLoad,66,mt7991)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7992
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7992 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT7992
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7992.ko
	MODPARAMS.mt7992:=option_type=$(CONFIG_MTK_HWIFI_MT7992_OPTION_TYPE) \
			  rro_mode=$(CONFIG_MTK_HWIFI_MT7992_RRO_MODE) \
			  intr_option_set=$(CONFIG_MTK_HWIFI_MT7992_INTR_OPTION_SET)
	AUTOLOAD:=$(call AutoLoad,66,mt7992)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7992/config
	if PACKAGE_kmod-mt7992

	config MTK_HWIFI_MT7992_RRO_MODE
		int "HWIFI MT7992 RRO Mode"
		default 5
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_MT7992_OPTION_TYPE
		int "HWIFI MT7992 Option Type"
		default 1
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_MT7992_INTR_OPTION_SET
		int "HWIFI MT7992 Interrupt Option Set"
		default 4
		depends on PACKAGE_kmod-mt_hwifi

	endif #PACKAGE_kmod-mt7992#
endef

define KernelPackage/mt7992_dbg
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7992 Debug driver
	DEPENDS:=kmod-mt7992 kmod-mtk_pci @MTK_HWIFI_MT7992
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7992_dbg.ko
	SUBMENU:=Drivers
	DEFAULT:=PACKAGE_kmod-mt7992
endef

define KernelPackage/mt799a
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT799A driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT799A
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt799a.ko
	AUTOLOAD:=$(call AutoLoad,66,mt799a)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7993
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7993 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT7993
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7993.ko
	MODPARAMS.mt7993:=option_type=$(CONFIG_MTK_HWIFI_MT7993_OPTION_TYPE) \
			  rro_mode=$(CONFIG_MTK_HWIFI_MT7993_RRO_MODE) \
			  intr_option_set=$(CONFIG_MTK_HWIFI_MT7993_INTR_OPTION_SET)
	AUTOLOAD:=$(call AutoLoad,66,mt7993)
	SUBMENU:=Drivers
endef

define KernelPackage/mt7993/config
	if PACKAGE_kmod-mt7993

	config MTK_HWIFI_MT7993_RRO_MODE
		int "HWIFI MT7993 RRO Mode"
		default 5
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_MT7993_OPTION_TYPE
		int "HWIFI MT7993 Option Type"
		default 1
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_MT7993_INTR_OPTION_SET
		int "HWIFI MT7993 Interrupt Option Set"
		default 4
		depends on PACKAGE_kmod-mt_hwifi

	endif #PACKAGE_kmod-mt7993#
endef

define KernelPackage/mt7993_dbg
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT7993 Debug driver
	DEPENDS:=kmod-mt7993 kmod-mtk_pci @MTK_HWIFI_MT7993
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt7993_dbg.ko
	SUBMENU:=Drivers
	DEFAULT:=PACKAGE_kmod-mt7993
endef

define KernelPackage/mt799b
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI MT799B driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_MT799B
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/mt799b.ko
	AUTOLOAD:=$(call AutoLoad,66,mt799b)
	SUBMENU:=Drivers
endef

define KernelPackage/blackhawk_a0
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI BLACKHAWK_A0 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_BLACKHAWK_A0
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/blackhawk_a0.ko
	MODPARAMS.blackhawk_a0:=option_type=$(CONFIG_MTK_HWIFI_BLACKHAWK_A0_OPTION_TYPE) \
			  rro_mode=$(CONFIG_MTK_HWIFI_BLACKHAWK_A0_RRO_MODE) \
			  intr_option_set=$(CONFIG_MTK_HWIFI_BLACKHAWK_A0_INTR_OPTION_SET)
	AUTOLOAD:=$(call AutoLoad,66,blackhawk_a0)
	SUBMENU:=Drivers
endef

define KernelPackage/blackhawk_a0/config
	if PACKAGE_kmod-blackhawk_a0

	config MTK_HWIFI_BLACKHAWK_A0_RRO_MODE
		int "HWIFI BLACKHAWK_A0 RRO Mode"
		default 4
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_BLACKHAWK_A0_OPTION_TYPE
		int "HWIFI BLACKHAWK_A0 Option Type"
		default 0
		depends on PACKAGE_kmod-mt_hwifi

	config MTK_HWIFI_BLACKHAWK_A0_INTR_OPTION_SET
		int "HWIFI BLACKHAWK_A0 Interrupt Option Set"
		default 0
		depends on PACKAGE_kmod-mt_hwifi

	endif #PACKAGE_kmod-blackhawk_a0#
endef

define KernelPackage/blackhawk_a0_dbg
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI BLACKHAWK_A0 Debug driver
	DEPENDS:=kmod-blackhawk_a0 kmod-mtk_pci @MTK_HWIFI_BLACKHAWK_A0
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/blackhawk_a0_dbg.ko
	SUBMENU:=Drivers
	DEFAULT:=PACKAGE_kmod-blackhawk_a0
endef

define KernelPackage/blackhawk_a0_pcie1
	CATEGORY:=MTK Properties
	TITLE:=MediaTek HWIFI BLACKHAWK_A0_PCIE1 driver
	DEPENDS:=kmod-mt_hwifi @MTK_HWIFI_BLACKHAWK_A0_PCIE1
	FILES+=$(PKG_BUILD_DIR)/wlan_hwifi/blackhawk_a0_pcie1.ko
	AUTOLOAD:=$(call AutoLoad,66,blackhawk_a0_pcie1)
	SUBMENU:=Drivers
endef


MTK_HWIFI_C_FLAGS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MTK_WIFI7_$c),CONFIG_$(c)=$(CONFIG_MTK_WIFI7_$(c))))

ifeq ($(CONFIG_MTK_HWIFI_WED_SUPPORT),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_HIF_WED=m
else
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_HIF_WED=n
endif

ifeq ($(CONFIG_EN_HWIFI_NPU_SUPPORT),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_HIF_NPU=m
else
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_HIF_NPU=n
endif

ifeq ($(CONFIG_MTK_HWIFI_MD_COEX_SUPPORT),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_MD_COEX_SUPPORT=y
endif

ifeq ($(CONFIG_MTK_HWIFI_SW_AMSDU),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_SW_AMSDU=y
endif

ifeq ($(CONFIG_MTK_HWIFI_RX_PROCESS_WORKQUEUE),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_RX_PROCESS_WORKQUEUE=y
endif

ifeq ($(CONFIG_MTK_HWIFI_MEM_SHRINK),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_MEM_SHRINK=y
endif

ifeq ($(CONFIG_PACKAGE_kmod-mt_wifi_cmn),y)
KCFLAGS += \
	-DMT_WIFI_COMMON_SUPPORT
endif

KCFLAGS += $(if $(CONFIG_MTK_WIFI7_OSAL_SUPPORT), -DMT_WIFI_COMMON_SUPPORT -I$(STAGING_DIR)/usr/include/mt_wifi)

ifneq ($(CONFIG_PACKAGE_kmod-mt7915_),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7915=n
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7915_DBG=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt7916_),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7916=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-bellwether),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_BELLWETHER=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt7990),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7990=n
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7990_DBG=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt7991),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7991=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt7992),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7992=n
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7992_DBG=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt799a),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT799A=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt7993),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7993=n
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT7993_DBG=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt799b),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_MT799B=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-blackhawk_a0),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_BLACKHAWK_A0=n
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_BLACKHAWK_A0_DBG=n
endif

ifneq ($(CONFIG_PACKAGE_kmod-blackhawk_a0_pcie1),y)
	MTK_HWIFI_C_FLAGS+=CONFIG_HWIFI_CHIP_BLACKHAWK_A0_PCIE1=n
endif

define Build/Compile
	$(Build/Compile/mt7990)
	$(Build/Compile/mt7992)
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" V=1 \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/wlan_hwifi" \
		LINUX_DIR="$(KERNEL_BUILD_DIR)" \
		$(MTK_HWIFI_C_FLAGS) \
	 	KCFLAGS="$(KCFLAGS)" \
		modules
endef

define KernelPackage/mt_hwifi/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
ifeq ($(CONFIG_MTK_HWIFI_MT7915),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7915/rebb/7915_WACPU_RAM_CODE_release.bin $(1)/lib/firmware/mediatek/mt7915_wa.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7915/rebb/WIFI_RAM_CODE_7915_E2.bin $(1)/lib/firmware/mediatek/mt7915_wm.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7915/rebb/mt7915_patch_e2_hdr.bin $(1)/lib/firmware/mediatek/mt7915_rom_patch.bin
endif

ifeq ($(CONFIG_MTK_HWIFI_BELLWETHER),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/bellwether_WACPU_RAM_CODE_release.bin $(1)/lib/firmware/bellwether_WACPU_RAM_CODE_release.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/WIFI_RAM_CODE_bellwether.bin $(1)/lib/firmware/WIFI_RAM_CODE_bellwether.bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/mtbellwether_patch_e1_hdr.bin $(1)/lib/firmware/mediatek/bellwether_rom_patch.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/wf_rom.bin $(1)/lib/firmware/wf_rom.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/wf_rom_sram.bin $(1)/lib/firmware/wf_rom_sram.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/WIFI_RAM_CODE_bellwether_TESTMODE_MT7976.bin $(1)/lib/firmware/WIFI_RAM_CODE_bellwether_TESTMODE_MT7976.bin
ifeq ($(CONFIG_MTK_WIFI7_ADIE_MT7977),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/WIFI_RAM_CODE_bellwether_MT7977.bin $(1)/lib/firmware/WIFI_RAM_CODE_bellwether.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/wf_rom_7977.bin $(1)/lib/firmware/wf_rom.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/wf_rom_sram_7977.bin $(1)/lib/firmware/wf_rom_sram.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/WIFI_RAM_CODE_bellwether_TESTMODE_MT7977.bin $(1)/lib/firmware/WIFI_RAM_CODE_bellwether_TESTMODE_MT7977.bin
endif
ifeq ($(CONFIG_MTK_WIFI7_DUAL_ADIE_MT7977),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/WIFI_RAM_CODE_bellwether_MT7977D.bin $(1)/lib/firmware/WIFI_RAM_CODE_bellwether.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/wf_rom_7977D.bin $(1)/lib/firmware/wf_rom.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/wf_rom_sram_7977D.bin $(1)/lib/firmware/wf_rom_sram.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/WIFI_RAM_CODE_bellwether_TESTMODE_MT7977D.bin $(1)/lib/firmware/WIFI_RAM_CODE_bellwether_TESTMODE_MT7977.bin
endif
endif

ifeq ($(CONFIG_MTK_HWIFI_MT7990),y)
ifeq ($(CONFIG_MTK_WIFI7_CCN34_SPLIT_MAC_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/ccn34/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/ccn34/WIFI_RAM_CODE_MT7990_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/ccn34/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_PHY_RAM_CODE_1_1.bin
else
ifeq ($(CONFIG_MTK_WIFI7_2ADIE_TRIBAND),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/2adie_triband/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/2adie_triband/WIFI_RAM_CODE_MT7990_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/2adie_triband/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/2adie_triband/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_PHY_RAM_CODE_1_1.bin
else ifeq ($(CONFIG_MTK_WIFI7_ADIE_TYPE),"MT7977")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7977/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7977/WIFI_RAM_CODE_MT7990_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7977/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7977/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_PHY_RAM_CODE_1_1.bin
else ifeq ($(CONFIG_MTK_WIFI7_ADIE_TYPE),"MT7979")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/pre8_7979/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/pre8_7979/WIFI_RAM_CODE_MT7990_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/pre8_7979/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/pre8_7979/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_PHY_RAM_CODE_1_1.bin
else ifeq ($(CONFIG_MTK_WIFI7_ADIE_TYPE),"MT7777")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7777/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7777/WIFI_RAM_CODE_MT7990_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7777/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7777/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_PHY_RAM_CODE_1_1.bin
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_RAM_CODE_MT7990_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/WIFI_MT7990_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7990_PHY_RAM_CODE_1_1.bin
endif
endif
endif

ifeq ($(CONFIG_MTK_HWIFI_MT7992),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PHY_RAM_CODE_1_1.bin $(1)/lib/firmware/WIFI_MT7992_PHY_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin

ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_idx_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
endif

ifeq ($(CONFIG_MTK_WIFI7_FW_CASAN_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_1_1_casan.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_WACPU_RAM_CODE_1_1_casan.bin $(1)/lib/firmware/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PHY_RAM_CODE_1_1_casan.bin $(1)/lib/firmware/WIFI_MT7992_PHY_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_1_1_hdr_casan.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
endif

ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE5040")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_WACPU_RAM_CODE_2_1.bin $(1)/lib/firmware/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PHY_RAM_CODE_2_1.bin $(1)/lib/firmware/WIFI_MT7992_PHY_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_2_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_2_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin

ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_idx_2_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_2_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
endif

ifeq ($(CONFIG_MTK_WIFI7_FW_CASAN_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_2_1_casan.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_WACPU_RAM_CODE_2_1_casan.bin $(1)/lib/firmware/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PHY_RAM_CODE_2_1_casan.bin $(1)/lib/firmware/WIFI_MT7992_PHY_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_2_1_hdr_casan.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
endif

endif
ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE6500")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_WACPU_RAM_CODE_3_1.bin $(1)/lib/firmware/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PHY_RAM_CODE_3_1.bin $(1)/lib/firmware/WIFI_MT7992_PHY_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_3_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_3_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin

ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_idx_3_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_3_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
endif

ifeq ($(CONFIG_MTK_WIFI7_FW_CASAN_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_3_1_casan.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_WACPU_RAM_CODE_3_1_casan.bin $(1)/lib/firmware/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PHY_RAM_CODE_3_1_casan.bin $(1)/lib/firmware/WIFI_MT7992_PHY_RAM_CODE_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_3_1_hdr_casan.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
endif

endif
ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE3600SDB")
ifeq ($(CONFIG_MTK_WIFI7_CCN34_SPLIT_MAC_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_RAM_CODE_MT7992_4_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_RAM_CODE_MT7992_4_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_4_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_RAM_CODE_MT7992_5_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_5_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_RAM_CODE_MT7992_6_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_6_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_MT7992_PATCH_MCU_5_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_5_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/ccn34/WIFI_MT7992_PATCH_MCU_6_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_6_1_hdr.bin
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_4_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_4_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_4_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_5_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_5_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_6_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_6_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_5_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_5_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_MT7992_PATCH_MCU_6_1_hdr.bin $(1)/lib/firmware/WIFI_MT7992_PATCH_MCU_6_1_hdr.bin
endif
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_4_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_4_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_4_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_5_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_5_1_TESTMODE.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/WIFI_RAM_CODE_MT7992_6_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7992_6_1_TESTMODE.bin
endif
endif

ifeq ($(CONFIG_MTK_HWIFI_MT7993),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/WIFI_MT7993_PATCH_MCU_1_1_hdr.bin $(1)/lib/firmware/WIFI_MT7993_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/WIFI_RAM_CODE_MT7993_1_1.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7993_1_1.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/WIFI_RAM_CODE_MT7993_1_1_TESTMODE.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7993_1_1_TESTMODE.bin
ifeq ($(CONFIG_MTK_WIFI7_FW_CASAN_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/WIFI_MT7993_PATCH_MCU_1_1_hdr_casan.bin $(1)/lib/firmware/WIFI_MT7993_PATCH_MCU_1_1_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/WIFI_RAM_CODE_MT7993_1_1_casan.bin $(1)/lib/firmware/WIFI_RAM_CODE_MT7993_1_1.bin
endif
endif

ifeq ($(CONFIG_MTK_HWIFI_BLACKHAWK_A0),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/blackhawk/rebb/WIFI_BLACKHAWK_A0_PATCH_MCU_hdr.bin $(1)/lib/firmware/WIFI_BLACKHAWK_A0_PATCH_MCU_hdr.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/blackhawk/rebb/WIFI_RAM_CODE_BLACKHAWK_A0.bin $(1)/lib/firmware/WIFI_RAM_CODE_BLACKHAWK_A0.bin
endif

endef

define Build/Compile/mt7992
#Copy Kite FW header for differnet SKUs
ifeq ($(CONFIG_MTK_WIFI7_CHIP_MT7992),y)
ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE7200")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr.h
ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
else
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
endif
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1.h
endif

ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"AX6000")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr.h
ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
else
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
endif
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1.h
endif

ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE5040")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_2_1_hdr_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr.h
ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_2_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
else
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_2_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
endif
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_2_1_TESTMODE_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_2_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1.h
endif

ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE6500")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_3_1_hdr_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr.h
ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_3_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
else
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_3_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
endif
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_3_1_TESTMODE_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_3_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1.h
endif

ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE3600SDB")
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_4_1_TESTMODE_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_5_1_TESTMODE_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_6_1_TESTMODE_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE_sdb.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.h
ifeq ($(CONFIG_MTK_WIFI7_CCN34_SPLIT_MAC_SUPPORT),y)
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_4_1_hdr_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_5_1_hdr_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_6_1_hdr_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr_sdb_ccn34.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr.h
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_4_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_5_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_6_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1_sdb_ccn34.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1.h
ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_4_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_5_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_6_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_1_1_sdb_ccn34.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
else
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_4_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_5_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_6_1_ccn34.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_sdb_ccn34.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
endif
else
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_4_1_hdr_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_5_1_hdr_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_6_1_hdr_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr_sdb.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_PATCH_MCU_1_1_hdr.h
ifeq ($(CONFIG_MTK_WIFI7_FW_LOG_TYPE),"idx_log")
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_4_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_5_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_6_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_idx_1_1_sdb.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
else
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_4_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_5_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_6_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1_sdb.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7992_1_1.h
endif
endif
	cat $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_4_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_5_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_6_1_.h \
		$(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1_sdb.h > $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7992_WACPU_RAM_CODE_1_1.h
endif
endif
endef

define Build/Compile/mt7990
#Copy Eagle FW header for differnet SKUs
ifeq ($(CONFIG_MTK_HWIFI_MT7990),y)
ifeq ($(CONFIG_MTK_WIFI7_2ADIE_TRIBAND),y)
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_PATCH_MCU_1_1_hdr_2adie.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_PATCH_MCU_1_1_hdr.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_WACPU_RAM_CODE_1_1_2adie.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_WACPU_RAM_CODE_1_1.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_2adie.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_TESTMODE_2adie.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.h
else
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_PATCH_MCU_1_1_hdr_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_PATCH_MCU_1_1_hdr.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_WACPU_RAM_CODE_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_MT7990_WACPU_RAM_CODE_1_1.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1.h
	cp -rf $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_TESTMODE_.h $(PKG_BUILD_DIR)/mt_wifi/include/hw_ctrl/mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.h
endif
endif
endef

$(eval $(call KernelPackage,mt_hwifi))
$(eval $(call KernelPackage,mtk_pci))
$(eval $(call KernelPackage,mtk_wed))
$(eval $(call KernelPackage,en_npu))
$(eval $(call KernelPackage,connac_if))
$(eval $(call KernelPackage,mt7915_))
$(eval $(call KernelPackage,mt7916_))
$(eval $(call KernelPackage,bellwether))
$(eval $(call KernelPackage,mt7990))
$(eval $(call KernelPackage,mt7990_dbg))
$(eval $(call KernelPackage,mt7991))
$(eval $(call KernelPackage,mt7992))
$(eval $(call KernelPackage,mt7992_dbg))
$(eval $(call KernelPackage,mt799a))
$(eval $(call KernelPackage,mt7993))
$(eval $(call KernelPackage,mt7993_dbg))
$(eval $(call KernelPackage,mt799b))
$(eval $(call KernelPackage,blackhawk_a0))
$(eval $(call KernelPackage,blackhawk_a0_dbg))
$(eval $(call KernelPackage,blackhawk_a0_pcie1))
