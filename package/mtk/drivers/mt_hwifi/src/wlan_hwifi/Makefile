# SPDX-License-Identifier: GPL-2.0
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
	SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../wlan_hwifi
else
	SRC_DIR := $(PWD)
endif

include $(SRC_DIR)/config.mk

ifneq ($(CONFIG_HWIFI_IF_CONNAC), n)
	ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
		MTK_WIFI_MAC := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../mt_wifi
	else
		MTK_WIFI_MAC ?=$(SRC_DIR)/../wifi_driver
	endif
KBUILD_EXTRA_SYMBOLS += $(MTK_WIFI_MAC)/os/linux/Module.symvers
endif

#core
ccflags-y += -Werror -Wall -g \
	     -I$(SRC_DIR) \
	     -I$(MTK_WIFI_MAC)/include \
	     -I$(MTK_WIFI_MAC)/include/lib \
	     -I$(MTK_WIFI_MAC)/include/hw_ctrl

ifeq ($(WM_RAM),mobile)
	ccflags-y += -DCONFIG_HWIFI_MOBILE_SUPPORT
else
	ccflags-y += -DCONFIG_HWIFI_REBB_SUPPORT
endif

ifneq ($(CONFIG_HWIFI_HIF_WED), n)
	MTK_WED ?= $(SRC_DIR)/../warp_driver/warp
	KBUILD_EXTRA_SYMBOLS += $(MTK_WED)/Module.symvers
	ccflags-y += -I$(MTK_WED) -DCONFIG_HWIFI_HIF_WED
endif

ifneq ($(CONFIG_HWIFI_HIF_NPU), n)
	ccflags-y += -DCONFIG_HWIFI_HIF_NPU
endif

ifeq ($(CONFIG_HWIFI_HE_SUPPORT),y)
	ccflags-y += -DCONFIG_HWIFI_HE_SUPPORT
endif

ifeq ($(CONFIG_HWIFI_SW_AMSDU),y)
	ccflags-y += -DCONFIG_HWIFI_SW_AMSDU
endif

ifeq ($(CONFIG_HWIFI_DBG), y)
	ccflags-y += -DDEBUG -DCONFIG_HWIFI_DBG
endif

ifeq ($(CONFIG_HWIFI_DBG_ISR), y)
	ccflags-y += -DCONFIG_HWIFI_DBG_ISR
endif

ifeq ($(CONFIG_HWIFI_USE_PCIE0_TXRING21), y)
	ccflags-y += -DCONFIG_HWIFI_USE_PCIE0_TXRING21
endif

ifeq ($(CONFIG_HWIFI_MD_COEX_SUPPORT), y)
	ccflags-y += -DCONFIG_HWIFI_MD_COEX_SUPPORT
endif

ifeq ($(CONFIG_HWIFI_RX_PROCESS_WORKQUEUE), y)
	ccflags-y += -DCONFIG_HWIFI_RX_PROCESS_WORKQUEUE
endif

ifeq ($(CONFIG_HWIFI_MEM_SHRINK), y)
	ccflags-y += -DCONFIG_HWIFI_MEM_SHRINK
endif

ifeq ($(CONFIG_ARCH_DMA_ADDR_T_64BIT), y)
	ccflags-y += -DCONFIG_HWIFI_64BIT_SUPPORT
endif

ifeq ($(CONFIG_HWIFI_FW_HDR_SUPPORT), y)
	ccflags-y += -DCONFIG_HWIFI_FW_HDR_SUPPORT
endif

#ccflags-y += -DCONFIG_HWIFI_FW_HDR_SUPPORT
ccflags-y += -Wno-error=enum-conversion
ccflags-y += -Wno-error=attribute-warning
ccflags-y += -Wno-error=enum-int-mismatch
ccflags-y += -Wno-error=cast-function-type
ccflags-y += -Wno-error=implicit-fallthrough

obj-$(CONFIG_HWIFI) += mtk_hwifi.o
mtk_hwifi-y := main.o bus.o mac_if.o chips.o utility.o
mtk_hwifi-y += chips/fmac.o
mtk_hwifi-y += chips/fmac_connac.o
mtk_hwifi-y += chips/bmac.o
mtk_hwifi-y += chips/bmac_connac.o
mtk_hwifi-y += wsys.o
mtk_hwifi-y += hwres_mgmt.o tk_mgmt.o
mtk_hwifi-y += mac_ops.o mac_ops_connac.o
mtk_hwifi-y += debugfs.o
mtk_hwifi-y += mcu/mcu.o
mtk_hwifi-y += mcu/mcu_wm.o
mtk_hwifi-y += mcu/mcu_wa.o
mtk_hwifi-y += mcu/mcu_wo.o
mtk_hwifi-y += mcu/mcu_dsp.o
mtk_hwifi-y += mcu/mcu_ops.o
mtk_hwifi-y += debug.o

#hif
obj-$(CONFIG_HWIFI_HIF_PCI) += mtk_pci.o
mtk_pci-y := bus/mtk_pci.o
mtk_pci-y += bus/mtk_pci_dma.o
mtk_pci-$(CONFIG_HWIFI_RRO) += bus/mtk_rro.o

#wed
obj-$(CONFIG_HWIFI_HIF_WED) += mtk_wed.o
mtk_wed-y := bus/mtk_wed.o

#airoha npu
obj-$(CONFIG_HWIFI_HIF_NPU) += en_npu.o
en_npu-y := bus/en_npu.o

#interface

ccflags-y += -DPER_PKT_CTRL_FOR_CTMR

#connac_if
obj-$(CONFIG_HWIFI_IF_CONNAC) += connac_if.o
connac_if-y := interface/connac_if.o
connac_if-$(CONFIG_HWIFI_MD_COEX_SUPPORT) += interface/connac_if_md_coex.o

#mac80211_if
obj-$(CONFIG_HWIFI_IF_MAC80211) += mac80211_if.o
mac80211_if-y := interface/mac80211_if.o
mac80211_if-y += interface/mac80211_if_core.o
mac80211_if-y += interface/mac_rx.o
mac80211_if-y += interface/mac_cmd.o
mac80211_if-y += interface/eeprom.o

#chip
obj-$(CONFIG_HWIFI_CHIP_MT7915) += mt7915.o
mt7915-y := chips/mt7915/mt7915.o

obj-$(CONFIG_HWIFI_CHIP_MT7916) += mt7916.o
mt7916-y := chips/mt7915/mt7916.o

obj-$(CONFIG_HWIFI_CHIP_MT7990) += mt7990.o
mt7990-y := chips/mt7990/mt7990.o

obj-$(CONFIG_HWIFI_CHIP_MT7991) += mt7991.o
mt7991-y := chips/mt7990/mt7991.o

obj-$(CONFIG_HWIFI_CHIP_BELLWETHER) += bellwether.o
bellwether-y := chips/bellwether/bellwether.o

obj-$(CONFIG_HWIFI_CHIP_MT7992) += mt7992.o
mt7992-y := chips/mt7992/mt7992.o

obj-$(CONFIG_HWIFI_CHIP_MT799A) += mt799a.o
mt799a-y := chips/mt7992/mt799a.o

obj-$(CONFIG_HWIFI_CHIP_MT7993) += mt7993.o
mt7993-y := chips/mt7993/mt7993.o

obj-$(CONFIG_HWIFI_CHIP_MT799B) += mt799b.o
mt799b-y := chips/mt7993/mt799b.o

#tools
obj-$(CONFIG_HWIFI_CHIP_MT7915_DBG) += mt7915_dbg.o
mt7915_dbg-y := tools/mt7915_dbg.o

obj-$(CONFIG_HWIFI_CHIP_MT7990_DBG) += mt7990_dbg.o
mt7990_dbg-y := tools/mt7990_dbg.o

obj-$(CONFIG_HWIFI_CHIP_MT7992_DBG) += mt7992_dbg.o
mt7992_dbg-y := tools/mt7992_dbg.o

obj-$(CONFIG_HWIFI_CHIP_MT7993_DBG) += mt7993_dbg.o
mt7993_dbg-y := tools/mt7993_dbg.o

#fpga mode
ccflags-y += -DCONFIG_HWIFI_CHIP_MT7993_FPGA
