#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=conninfra
PKG_VERSION:=20231229-f2fa25
PKG_SOURCE:=mt79xx_conninfra_20231229-f2fa25.tar.xz
PKG_SOURCE_URL:=
PKG_HASH:=672929de84ff2828ab8caef7ae56b6b3aac91210d5700c568559ebfa2d3cd176
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)$(if $(BUILD_VARIANT),-$(BUILD_VARIANT)/$(PKG_NAME))
PKG_MAINTAINER:=Kun-Ze Syue<kun-ze.syue@mediatek.com>

PKG_KCONFIG:= \
	MTK_CONNINFRA_APSOC \
	MTK_CONNINFRA_APSOC_MT7986 \
	MTK_CONNINFRA_APSOC_MT7981 \
	CONNINFRA_EMI_SUPPORT \
	FPGA_EARLY_PORTING \
	CONNINFRA_AUTO_UP

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

# Validate the package source/build boundary before package.mk's prepared/clean rm -rf.
override VENDOR_WIFI_GUARD := $(TOPDIR)/package/mtk/drivers/vendor-wifi-guard.sh
override VENDOR_WIFI_SOURCE_DIR := $(CURDIR)
.PHONY: vendor-wifi-predelete-guard
vendor-wifi-predelete-guard:
	@$(VENDOR_WIFI_GUARD) "$(TOPDIR)" "$(VENDOR_WIFI_SOURCE_DIR)" "$(KERNEL_BUILD_DIR)" "$(PKG_BUILD_DIR)" "conninfra" "$(if $(filter-out undefined,$(origin USE_SOURCE_DIR)),active,)" "$(if $(filter-out undefined,$(origin USE_GIT_TREE)),active,)" "$(if $(filter-out undefined,$(origin USE_GIT_SRC_CHECKOUT)),active,)" "$(BUILD_VARIANT)"
STAMP_PREPARED_DEPENDS += | vendor-wifi-predelete-guard
force-clean-build: vendor-wifi-predelete-guard

define KernelPackage/conninfra
	CATEGORY:=MTK Properties
	TITLE:=Conninfra driver
	DEPENDS:=@TARGET_mediatek_filogic_mtwifi
	FILES:=$(PKG_BUILD_DIR)/conninfra.ko
	AUTOLOAD:=$(call AutoLoad,10,conninfra,1)
	SUBMENU:=Drivers
	MENU:=1
	VARIANT:=custom
	DEFAULT_VARIANT:=1
endef

define KernelPackage/conninfra/description
	Support for connectivity conninfra driver.
endef

define KernelPackage/conninfra/config
if PACKAGE_kmod-conninfra
	config MTK_CONNINFRA_APSOC
	bool "Conninfra APSOC Only"
	default y

if MTK_CONNINFRA_APSOC
choice
	prompt "Choose APSOC Chip"
	default MTK_CONNINFRA_APSOC_MT7981 if MTK_FIRST_IF_MT7981
	default MTK_CONNINFRA_APSOC_MT7986

	config MTK_CONNINFRA_APSOC_MT7986
	bool "MT7986"

	config MTK_CONNINFRA_APSOC_MT7981
	bool "MT7981"
endchoice

	config CONNINFRA_EMI_SUPPORT
	bool "EMI Support"
	default y

	config FPGA_EARLY_PORTING
	bool "Only for FPGA Stage"
	default n

	config CONNINFRA_AUTO_UP
	bool "Conninfra Up by Self"
	default y
endif
endif
endef

define KernelPackage/conninfra-mt7981
$(call KernelPackage/conninfra)
	TITLE+= (MT7981)
	DEPENDS:=@TARGET_mediatek_filogic_mtwifi
	VARIANT:=mt7981
	DEFAULT_VARIANT:=
	CONFLICTS:=kmod-conninfra kmod-conninfra-mt7986
endef

define KernelPackage/conninfra-mt7986
$(call KernelPackage/conninfra)
	TITLE+= (MT7986)
	DEPENDS:=@TARGET_mediatek_filogic_mtwifi
	VARIANT:=mt7986
	DEFAULT_VARIANT:=
	CONFLICTS:=kmod-conninfra kmod-conninfra-mt7981
endef

# Transfer local kernel config to compile option
ifeq ($(BUILD_VARIANT),mt7981)
KCONFIG_FLAGS:= \
	CONFIG_MTK_CONNINFRA_APSOC=y \
	CONFIG_MTK_CONNINFRA_APSOC_MT7981=y \
	CONFIG_CONNINFRA_EMI_SUPPORT=y \
	CONFIG_CONNINFRA_AUTO_UP=y
else ifeq ($(BUILD_VARIANT),mt7986)
KCONFIG_FLAGS:= \
	CONFIG_MTK_CONNINFRA_APSOC=y \
	CONFIG_MTK_CONNINFRA_APSOC_MT7986=y \
	CONFIG_CONNINFRA_EMI_SUPPORT=y \
	CONFIG_CONNINFRA_AUTO_UP=y
else
KCONFIG_FLAGS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c))))
endif

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(KCONFIG_FLAGS)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(KCONFIG_FLAGS)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=0, $(patsubst %=n,%,$(filter %=n,$(KCONFIG_FLAGS))))

EXTRA_CFLAGS+=-DEEPROM_NAME=${CONFIG_first_card_EEPROM_name}

CONNINFRA_KERNEL_KCFLAGS = $(call iremap,$(BUILD_DIR),$(notdir $(BUILD_DIR))) $(filter-out -fno-plt,$(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))) $(call qstrip,$(CONFIG_KERNEL_CFLAGS))

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	KCFLAGS="$(CONNINFRA_KERNEL_KCFLAGS)" \
	$(KCONFIG_FLAGS)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,conninfra))
$(eval $(call KernelPackage,conninfra-mt7981))
$(eval $(call KernelPackage,conninfra-mt7986))
