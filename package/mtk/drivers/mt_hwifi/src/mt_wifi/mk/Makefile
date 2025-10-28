#WIFI_MODE - AP, STA, APSTA
WIFI_MODE ?= AP

#TARTET - LINUX, NONE, UCOS, ECOS, or THREADX.
TARGET ?= LINUX

#WIFI_HIF - pcie | usb | ut | none
WIFI_HIF ?= pcie

#CHIPSET - mt7915 bellwether mt7990 mt7992 mt7993
CHIPSET ?= mt7990

#SKU - BE7200 BE5040 BE6500 for mt7992
SKU ?= BE7200

#Using Driver Native feature configuration or not
CONFIG_NATIVE_CFG ?= y

#UL_CTRL: Upper Layer daemon contrl (ex. hostapd/wpa_supplicant)
# NONE, CFG80211
UL_CTRL ?= NONE

# Add debug symbol
ADD_DBG_SYM ?= n

MODULE = $(word 1, $(CHIPSET))

#PLATFORM: Target platform
PLATFORM ?= PC

#TOP_DIR = home directory of wlan source code
TOP_DIR ?= $(shell pwd)

#TOP_BIN_DIR = parent directory of bin folder, used by bin2h
TOP_BIN_DIR ?= $(shell cd ..;pwd)
TOP_OS_DIR := $(TOP_DIR)/os

MAKE = make

ifeq ($(TARGET),LINUX)
LINUX_KERNEL_VER ?= $(shell uname -r)
ifeq ($(PLATFORM),PC)
LINUX_SRC ?= /lib/modules/$(LINUX_KERNEL_VER)/build
CROSS_COMPILE ?=
endif
endif

ifeq ($(CCACHE),1)
BIN_CCACHE=$(shell which ccache)
CROSS_COMPILE := $(BIN_CCACHE) $(CROSS_COMPILE)
endif

export TOP_DIR TOP_BIN_DIR TOP_OS_DIR WIFI_MODE LINUX_SRC CROSS_COMPILE PLATFORM CHIPSET MODULE TARGET CONFIG_NATIVE_CFG SKU ADD_DBG_SYM

# The targets that may be used.
PHONY += all build_tools test UCOS THREADX LINUX release prerelease clean uninstall install libwapi

ifeq ($(TARGET),LINUX)
all: build_information build_tools build_power_limit_tables $(TARGET)
else
all: $(TARGET)
endif

build_information:
	@echo =========================================
	@echo        Build Option Infomation
	@echo =========================================
	@echo WIFI_MODE       = $(WIFI_MODE)
	@echo TARGET          = $(TARGET)
	@echo WIFI_HIF        = $(WIFI_HIF)
	@echo CHIPSET         = $(CHIPSET)
	@echo SKU             = $(SKU)
	@echo WM_RAM          = $(WM_RAM)
	@echo PLATFORM        = $(PLATFORM)
	@echo UL_CTRL         = $(UL_CTRL)
	@echo =========================================

build_power_limit_tables:
	$(MAKE) -C $(TOP_DIR)/hw_ctrl/phy/txpwr
	$(TOP_DIR)/hw_ctrl/phy/txpwr/data2h

build_tools:
	$(MAKE) -C $(TOP_DIR)/tools
	$(TOP_DIR)/tools/bin2h

test:
	$(MAKE) -C tools test

LINUX:
	cp -f $(TOP_OS_DIR)/linux/Makefile.6 $(TOP_OS_DIR)/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) M=$(TOP_OS_DIR)/linux modules

ifeq ($(WIFI_MODE),AP)
	cp -f $(TOP_OS_DIR)/linux/$(MODULE)_ap.ko $(TOP_OS_DIR)/linux/mt_single_driver_ap.ko
endif
ifeq ($(WIFI_MODE),STA)
	cp -f $(TOP_OS_DIR)/linux/$(MODULE)_sta.ko $(TOP_OS_DIR)/linux/mt_single_driver_sta.ko
endif

build_plug_in:
	$(MAKE) -C $(LINUX_SRC) M=$(TOP_DIR)/os/linux/plug_in MODULE_FLAGS="$(WFLAGS)"

release: build_tools build_power_limit_tables
	$(MAKE) -C $(TOP_DIR)/striptool -f Makefile.release clean
	$(MAKE) -C $(TOP_DIR)/striptool -f Makefile.release
	$(TOP_DIR)/striptool/striptool.out

clean:
ifeq ($(TARGET), LINUX)
	cp -f $(TOP_OS_DIR)/linux/Makefile.clean $(TOP_OS_DIR)/linux/Makefile
	$(MAKE) -C $(TOP_OS_DIR)/linux clean
	rm -rf $(TOP_OS_DIR)/linux/Makefile
endif

libwapi:
	cp -f embedded/os/linux/Makefile.libwapi.6 $(TOP_OS_DIR)/linux/Makefile	
	$(MAKE) -C  $(LINUX_SRC) M=$(TOP_OS_DIR)/linux modules	

# Declare the contents of the .PHONY variable as phony.  We keep that information in a variable
.PHONY: $(PHONY)



