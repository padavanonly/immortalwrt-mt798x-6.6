# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=warp
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)$(if $(BUILD_VARIANT),-$(BUILD_VARIANT))

MT7981_NEW_WOFW_DIR := mt7981-fw-20231228
MT7986_NEW_WOFW_DIR := mt7986-fw-20230808

ifeq ($(CONFIG_TARGET_mediatek_filogic_mtwifi),y)
PKG_VERSION:=20221209-3e6ae1
PKG_SOURCE:=warp_$(PKG_VERSION)-src.tar.xz
PKG_SOURCE_URL:=
PKG_HASH:=985364fe76b1a79dbee2c9dd2392dbc71fa1573b1380b096fda2dc0fda2b9bac
PATCH_DIR:=./patches-v2
PKG_KCONFIG:= \
	WARP_VERSION \
	WARP_ATC_SUPPORT \
	WARP_DBG_SUPPORT \
	WARP_WDMA_RECYCLE_SUPPORT \
	WED_HW_RRO_SUPPORT \
	MTK_MEMORY_SHRINK \
	WARP_CHIPSET \
	WARP_WO_EMBEDDED_LOAD \
	WARP_MEMORY_LEAK_DBG \
	WARP_NEW_FW
else
P4REV:=
PKG_VERSION:=
PKG_RELEASE:=3
PKG_SOURCE:=
WARP_SRC_DIR:=./src
PATCH_DIR:=./patches
PKG_KCONFIG:= \
	WARP_CONFIG_MODE \
	WARP_VERSION \
	WARP_DBG_SUPPORT \
	WED_HW_RRO_SUPPORT \
	MTK_MEMORY_SHRINK \
	WARP_CHIPSET \
	WARP_WO_EMBEDDED_LOAD \
	WARP_MEMORY_LEAK_DBG \
	TARGET_mediatek
endif

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c))) CONFIG_TARGET_mediatek_filogic_mtwifi

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_TARGET_mediatek_filogic_mtwifi),y)
TAR_CMD=$(HOST_TAR) -C $(1) --strip-components=1 $(TAR_OPTIONS)
else
TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)
endif

# Validate the package source/build boundary before package.mk's prepared/clean rm -rf.
override VENDOR_WIFI_GUARD := $(TOPDIR)/package/mtk/drivers/vendor-wifi-guard.sh
ifeq ($(CONFIG_TARGET_mediatek_filogic_mtwifi),y)
override VENDOR_WIFI_SOURCE_DIR := $(CURDIR)
else
override VENDOR_WIFI_SOURCE_DIR := $(CURDIR)/src
endif
.PHONY: vendor-wifi-predelete-guard
vendor-wifi-predelete-guard:
	@$(VENDOR_WIFI_GUARD) "$(TOPDIR)" "$(VENDOR_WIFI_SOURCE_DIR)" "$(KERNEL_BUILD_DIR)" "$(PKG_BUILD_DIR)" "warp" "$(if $(filter-out undefined,$(origin USE_SOURCE_DIR)),active,)" "$(if $(filter-out undefined,$(origin USE_GIT_TREE)),active,)" "$(if $(filter-out undefined,$(origin USE_GIT_SRC_CHECKOUT)),active,)" "$(BUILD_VARIANT)"
STAMP_PREPARED_DEPENDS += | vendor-wifi-predelete-guard
force-clean-build: vendor-wifi-predelete-guard

define KernelPackage/warp
  CATEGORY:=MTK Properties
  TITLE:=MTK warp driver
  DEPENDS:=@(TARGET_mediatek_filogic_mtwifi||TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +kmod-mediatek_hnat
  FILES:=$(PKG_BUILD_DIR)/mtk_warp.ko
  AUTOLOAD:=$(call AutoLoad,60,mtk_warp,1)
  SUBMENU:=Drivers
  MENU:=1
  VARIANT:=custom
  DEFAULT_VARIANT:=1
  PROVIDES:=kmod-warp-hwifi
  CONFLICTS:=kmod-warp-mt7981 kmod-warp-mt7986 kmod-warp-mt7987 kmod-warp-mt7988
endef

define KernelPackage/warp/config
	source "$(SOURCE)/config.in"
endef

define KernelPackage/warp-mt7981
$(call KernelPackage/warp)
  TITLE+= (MT7981)
  DEPENDS:=@TARGET_mediatek_filogic_mtwifi +kmod-mediatek_hnat
  VARIANT:=mt7981
  DEFAULT_VARIANT:=
  PROVIDES:=
  CONFLICTS:=kmod-warp kmod-warp-mt7986
endef

define KernelPackage/warp-mt7986
$(call KernelPackage/warp)
  TITLE+= (MT7986)
  DEPENDS:=@TARGET_mediatek_filogic_mtwifi +kmod-mediatek_hnat
  VARIANT:=mt7986
  DEFAULT_VARIANT:=
  PROVIDES:=
  CONFLICTS:=kmod-warp kmod-warp-mt7981
endef

define KernelPackage/warp-mt7987
$(call KernelPackage/warp)
  TITLE+= (MT7987)
  DEPENDS:=@TARGET_mediatek_filogic_hwifi +kmod-mediatek_hnat
  VARIANT:=mt7987
  DEFAULT_VARIANT:=
  PROVIDES:=kmod-warp-hwifi
  CONFLICTS:=kmod-warp kmod-warp-mt7988
endef

define KernelPackage/warp-mt7988
$(call KernelPackage/warp)
  TITLE+= (MT7988)
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +kmod-mediatek_hnat
  VARIANT:=mt7988
  DEFAULT_VARIANT:=
  PROVIDES:=kmod-warp-hwifi
  CONFLICTS:=kmod-warp kmod-warp-mt7987
endef

ifeq ($(BUILD_VARIANT),mt7981)
WARP_KCONFIG_FLAGS:= \
	CONFIG_WARP_CHIPSET="mt7981" \
	CONFIG_WARP_VERSION="2" \
	CONFIG_WED_HW_RRO_SUPPORT=y \
	CONFIG_WARP_DBG_SUPPORT=y \
	CONFIG_MTK_MEMORY_SHRINK=y
else ifeq ($(BUILD_VARIANT),mt7986)
WARP_KCONFIG_FLAGS:= \
	CONFIG_WARP_CHIPSET="mt7986" \
	CONFIG_WARP_VERSION="2" \
	CONFIG_WED_HW_RRO_SUPPORT=y \
	CONFIG_WARP_DBG_SUPPORT=y \
	CONFIG_WARP_MEMORY_LEAK_DBG=n \
	CONFIG_WARP_NEW_FW=y
else ifeq ($(BUILD_VARIANT),mt7987)
WARP_KCONFIG_FLAGS:= \
	CONFIG_WARP_CONFIG_MODE=1 \
	CONFIG_WARP_CHIPSET="mt7987" \
	CONFIG_WARP_VERSION="3_1" \
	CONFIG_WARP_DBG_SUPPORT=y \
	CONFIG_TARGET_mediatek=y
else ifeq ($(BUILD_VARIANT),mt7988)
WARP_KCONFIG_FLAGS:= \
	CONFIG_WARP_CONFIG_MODE=0 \
	CONFIG_WARP_CHIPSET="mt7988" \
	CONFIG_WARP_VERSION="3" \
	CONFIG_WED_HW_RRO_SUPPORT=y \
	CONFIG_WARP_DBG_SUPPORT=y \
	CONFIG_TARGET_mediatek=y
else
WARP_KCONFIG_FLAGS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c))))
endif

WARP_KERNEL_KCFLAGS = $(call iremap,$(BUILD_DIR),$(notdir $(BUILD_DIR))) $(filter-out -fno-plt,$(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))) $(call qstrip,$(CONFIG_KERNEL_CFLAGS))

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifeq ($(CONFIG_TARGET_mediatek_filogic_mtwifi),y)
	$(PKG_UNPACK)
else
	$(CP) $(WARP_SRC_DIR)/* $(PKG_BUILD_DIR)/
endif
	$(Build/Patch)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/mtk-warp-$(or $(BUILD_VARIANT),custom)/mcu
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/mtk-warp-$(or $(BUILD_VARIANT),custom)/
	$(CP) $(PKG_BUILD_DIR)/mcu/*.h $(1)/usr/include/mtk-warp-$(or $(BUILD_VARIANT),custom)/mcu/
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" V=1 \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		$(WARP_KCONFIG_FLAGS) \
		KCFLAGS="$(WARP_KERNEL_KCFLAGS)" \
		modules
endef

TARGET_CFLAGS += \
	$(addprefix -D,$(WARP_KCONFIG_FLAGS))

MAKE_FLAGS += \
	$(WARP_KCONFIG_FLAGS)

TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y

ifeq ($(CONFIG_TARGET_mediatek_filogic_mtwifi),y)
ifeq ($(CONFIG_WED_HW_RRO_SUPPORT),y)
define KernelPackage/warp/install
	$(INSTALL_DIR) $(1)/lib/firmware
	if [ "$$(CONFIG_WARP_CHIPSET)" = "mt7981" ]; then \
		if [ "$$(CONFIG_WARP_NEW_FW)" = "y" ]; then \
			cp ./files/$$(MT7981_NEW_WOFW_DIR)/7981*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/; \
		else \
			cp $(PKG_BUILD_DIR)/bin/7981*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/; \
		fi; \
	elif [ "$$(CONFIG_WARP_CHIPSET)" = "mt7986" ]; then \
		if [ "$$(CONFIG_WARP_NEW_FW)" = "y" ]; then \
			cp ./files/$$(MT7986_NEW_WOFW_DIR)/7986*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/; \
		else \
			cp $(PKG_BUILD_DIR)/bin/7986*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/; \
		fi; \
	fi
endef
endif

define KernelPackage/warp-mt7981/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp ./files/$$(MT7981_NEW_WOFW_DIR)/7981*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/
endef

define KernelPackage/warp-mt7986/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp ./files/$$(MT7986_NEW_WOFW_DIR)/7986*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/
endef
else
ifeq ($(CONFIG_WED_HW_RRO_SUPPORT),y)
define KernelPackage/warp/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp $(PKG_BUILD_DIR)/bin/*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/;
endef
endif

define KernelPackage/warp-mt7988/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp $(PKG_BUILD_DIR)/bin/*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/;
endef
endif

$(eval $(call KernelPackage,warp))
$(eval $(call KernelPackage,warp-mt7981))
$(eval $(call KernelPackage,warp-mt7986))
$(eval $(call KernelPackage,warp-mt7987))
$(eval $(call KernelPackage,warp-mt7988))
