ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
TOP_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../mt_wifi
PARENT_DIR := ../mt_wifi

else
ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
TOP_DIR = drivers/net/wireless/mediatek
else
TOP_DIR = drivers/net/wireless
endif
endif


#TOP_BIN_DIR = parent directory of bin folder, used by bin2h
TOP_BIN_DIR ?= $(shell cd ..;pwd)
TOP_OS_DIR := $(TOP_DIR)/os

#WIFI_MODE - AP, STA, APSTA
WIFI_MODE ?= AP

#WIFI_HIF - pcie | usb | ut
WIFI_HIF ?= pcie

# CHIPSET
ifeq ($(CONFIG_FIRST_IF_MT7915),y)
CHIPSET = mt7915
endif
ifeq ($(CONFIG_FIRST_IF_BELLWETHER),y)
CHIPSET = bellwether
endif
ifeq ($(CONFIG_FIRST_IF_MT7990),y)
CHIPSET = mt7990
endif
ifeq ($(CONFIG_FIRST_IF_MT7992),y)
CHIPSET = mt7992
endif
ifeq ($(CONFIG_FIRST_IF_MT7993),y)
CHIPSET = mt7993
endif

# WM RAM TYPE
#WM_RAM - mobile | rebb
WM_RAM ?= rebb

TARGET ?= LINUX

ifneq ($(CONFIG_GCC_VERSION), )
ifeq ($(shell expr $(CONFIG_GCC_VERSION) \>= 8.4.0), 1)
# this option only have after 8.x.x
# workaround of bin compare on preflight will get the .rodata.str1.8 and it's abs path of __FILE__
# will false the compare due to build server folder is not the same
ccflags-y += -fmacro-prefix-map=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))=
endif
endif

#Using Driver Native feature configuration or not
ifneq ($(CONFIG_SUPPORT_OPENWRT),y)
CONFIG_NATIVE_CFG ?= y
endif
MODULE = $(word 1, $(CHIPSET))

DRV_NAME = mt_wifi

include $(TOP_DIR)/mk/config/config.mk

SERV_H_PATH := $(TOP_DIR)/ate/wlan_service
SERV_DIR := ../mt_wifi/ate/wlan_service

obj_ap :=
obj_ate :=
obj_chip :=
obj_common :=
obj_feature :=
obj_hif :=
obj_hw_ctrl :=
obj_os :=
obj_sta :=
obj_protocol :=
obj_dvt:=
obj_profiles :=

include $(TOP_DIR)/mk/obj/ap.mk
include $(TOP_DIR)/mk/obj/ate.mk
include $(TOP_DIR)/mk/obj/chip.mk
include $(TOP_DIR)/mk/obj/common.mk
include $(TOP_DIR)/mk/obj/feature.mk
include $(TOP_DIR)/mk/obj/hif.mk
include $(TOP_DIR)/mk/obj/hw_ctrl.mk
include $(TOP_DIR)/mk/obj/os.mk
include $(TOP_DIR)/mk/obj/sta.mk
include $(TOP_DIR)/mk/obj/protocol.mk
include $(TOP_DIR)/mk/obj/dvt.mk
include $(TOP_DIR)/mk/obj/profiles.mk

ccflags-y += $(WFLAGS)
ccflags-y += -Wno-error=misleading-indentation
ccflags-y += -Wno-error=attribute-warning
ccflags-y += -Wno-error=enum-conversion
ccflags-y += -Wno-error=enum-int-mismatch
ccflags-y += -Wno-error=cast-function-type
ccflags-y += -Wno-error=implicit-fallthrough
ccflags-y += -Wno-error=discarded-qualifiers

ifeq ($(CONFIG_CFG80211_SUPPORT),y)
    GLOBAL_INCLUDES_NL80211 = include/uapi/linux/mtk_nl80211_inc
    ccflags-y += -I$(GLOBAL_INCLUDES_NL80211)
endif

ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ccflags-y += -DCONFIG_SUPPORT_OPENWRT
endif

ccflags-y += -DNEED_ROM_PATCH

ifeq ($(CONFIG_INTERWORKING),y)
ccflags-y += -DCONFIG_DOT11U_INTERWORKING
endif

ifeq ($(CONFIG_PRE_CAL_TRX_SET1_SUPPORT),y)
    ccflags-y += -DPRE_CAL_TRX_SET1_SUPPORT
endif

ifeq ($(CONFIG_RLM_CAL_CACHE_SUPPORT),y)
    ccflags-y += -DRLM_CAL_CACHE_SUPPORT
endif

ifeq ($(CONFIG_PRE_CAL_TRX_SET2_SUPPORT),y)
    ccflags-y += -DPRE_CAL_TRX_SET2_SUPPORT
endif

ifeq ($(CONFIG_CAL_BIN_FILE_SUPPORT),y)
    ccflags-y += -DCAL_BIN_FILE_SUPPORT
endif


ifeq ($(CONFIG_RTMP_TEMPERATURE_TX_ALC),y)
    ccflags-y += -DRTMP_TEMPERATURE_TX_ALC
endif

ifeq ($(CONFIG_WIFI_SYSDVT),y)
ccflags-y += -DVERIFICATION_MODE
endif

ifneq ($(CONFIG_CFG80211_SUPPORT),y)
ifeq ($(CONFIG_WIFI_FWD_SUPPORT),y)
ccflags-y += -DCONFIG_WIFI_PKT_FWD
else
ccflags-y += -DMTFWD
endif
else
ifeq ($(CONFIG_NL80211_TESTMODE),y)
ccflags-y += -DHOSTAPD_PMKID_IN_DRIVER_SUPPORT
endif
endif

ccflags-y += -DMTK_FE_RESET_RECOVER

ccflags-y += -DHTC_DECRYPT_IOT

ccflags-y += -Inet/nat
#ccflags-y += -DDBG_STARVATION
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ccflags-y += -DMT_WIFI_MODULE
else
ifeq ($(CONFIG_MT_WIFI),m)
ccflags-y += -DMT_WIFI_MODULE
endif
endif


###############################################################################
#
# config for AP mode
#
###############################################################################
ifeq ($(WIFI_MODE),AP)
MOD_NAME = $(DRV_NAME) #$(MODULE)_ap
endif

###############################################################################
#
# config for STA mode
#
###############################################################################
ifeq ($(WIFI_MODE),STA)
MOD_NAME = $(DRV_NAME) #$(MODULE)_sta
endif

###############################################################################
#
# config for AP/STA mixed mode
#
###############################################################################
ifeq ($(WIFI_MODE), APSTA)
MOD_NAME = $(MODULE)_apsta
endif

###############################################################################
#
# Module Base
#
###############################################################################

MOD_NAME = $(DRV_NAME)

# support built-in/ko for Openwrt/Linux SDK
obj-$(CONFIG_MT_WIFI) += $(MOD_NAME).o

ifeq ($(CONFIG_CPE_SUPPORT),y)
# To avoid build issue "/bin/sh: Argument list too long", merge multi-objs to one
cmm_objs := $(patsubst %,$(M)/%,$(obj_common))
$(M)/$(PARENT_DIR)/cmm_objs.o: $(cmm_objs)
	$(LD) -r -o $@ $^

$(MOD_NAME)-objs := \
	$(obj_ap)\
	$(obj_ate)\
	$(obj_chips)\
	$(obj_feature)\
	$(obj_hif)\
	$(obj_hw_ctrl)\
	$(obj_os)\
	$(obj_sta)\
	$(obj_protocol)\
	$(obj_dvt)\
	$(obj_profiles)\
	$(PARENT_DIR)/cmm_objs.o
else
$(MOD_NAME)-objs := \
	$(obj_ap)\
	$(obj_ate)\
	$(obj_chips)\
	$(obj_common)\
	$(obj_feature)\
	$(obj_hif)\
	$(obj_hw_ctrl)\
	$(obj_os)\
	$(obj_sta)\
	$(obj_protocol)\
	$(obj_dvt)\
	$(obj_profiles)
endif

#
#  openWRT
#
#

ifeq ($(CONFIG_FAST_NAT_SUPPORT),y)
ccflags-y += -DCONFIG_FAST_NAT_SUPPORT
endif
ifeq ($(CONFIG_WLAN_HOOK),y)
ifeq ($(CONFIG_WHNAT_SUPPORT),y)
ccflags-y += -DWHNAT_SUPPORT
ccflags-y += -DWFDMA_WED_COMPATIBLE
else
ifeq ($(CONFIG_WHNAT_SUPPORT),m)
ccflags-y += -DWHNAT_SUPPORT
ccflags-y += -DWFDMA_WED_COMPATIBLE
endif
endif
endif
ccflags-y += -DWPS_UNCONFIG_FEATURE_SUPPORT

ccflags-y += -DPER_PKT_CTRL_FOR_CTMR

ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ifeq ($(CONFIG_HWIFI_SUPPORT),y)
#TODO temporarily added by jing
MODULE_FLAGS=$(ccflags-y)
export MODULE_FLAGS
obj-m += $(PARENT_DIR)/os/linux/plug_in/
else
MODULE_FLAGS=$(ccflags-y)
export MODULE_FLAGS
obj-m += $(PARENT_DIR)/os/linux/plug_in/
endif
endif

ifeq ($(CONFIG_CPE_SUPPORT),y)
ccflags-y += -DCONFIG_CPE_SUPPORT
ifeq ($(CONFIG_WIFI_MD_COEX_SUPPORT),y)
ccflags-y += -DWIFI_MD_COEX_SUPPORT
endif
ccflags-y += -Wframe-larger-than=4096
endif

PHONY := clean install uninstall

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
