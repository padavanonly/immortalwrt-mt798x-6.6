# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wlan_hwifi
P4REV:=
PKG_VERSION:=1.0
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=
PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)

define KernelPackage/wlan_hwifi
  CATEGORY:=MTK Properties
  TITLE:=MTK HWIFI driver
  DEPENDS:= +kmod-warp
  DEPENDS+= +kmod-wlan
  KBUILD_EXTRA_SYMBOLS+=$(KERNEL_BUILD_DIR)/symvers/warp.symvers
  KBUILD_EXTRA_SYMBOLS+=$(KERNEL_BUILD_DIR)/symvers/wlan.symvers
  FILES:=$(PKG_BUILD_DIR)/mtk_hwifi.ko \
	$(PKG_BUILD_DIR)/mtk_pci.ko \
	$(PKG_BUILD_DIR)/mtk_wed.ko \
	$(PKG_BUILD_DIR)/connac_if.ko \
	$(PKG_BUILD_DIR)/mt7915.ko \
	$(PKG_BUILD_DIR)/mt7915_dbg.ko
  AUTOLOAD:=$(call AutoProbe, mtk_hwifi mtk_wed mtk_pci connac_if mt7915 mt7915_dbg)
#  AUTOLOAD:=$(call AutoProbe, mtk_hwifi mtk_pci connac_if mt7915 mt7915_dbg)
  SUBMENU:=Drivers
  MENU:=1
endef

define KernelPackage/wlan_hwifi/config
	source "$(SOURCE)/config.in"
endef

#ifeq ($(MTK_RELEASE_VERSION),trunk)
USE_SOURCE_DIR:=$(TOPDIR)/../mtk/package/kernel/wlan_driver/jedi/wlan_hwifi
define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	$(if $(wildcard $(USE_SOURCE_DIR)/*),,@echo "Error: USE_SOURCE_DIR=$(USE_SOURCE_DIR) path not found"; false)
	mkdir $(KERNEL_BUILD_DIR)/wlan_hwifi
	cp -rf $(USE_SOURCE_DIR)/* $(KERNEL_BUILD_DIR)/wlan_hwifi/
	rm -rf $(PKG_BUILD_DIR)/.git
endef
#endif

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c)))) \
		SRC_DIR=$(PKG_BUILD_DIR)/ \
		WM_RAM=rebb \
		MTK_WIFI_MAC=$(PKG_BUILD_DIR)/../wlan \
		MTK_WED=$(PKG_BUILD_DIR)/../warp \
		KBUILD_EXTRA_SYMBOLS="$(KBUILD_EXTRA_SYMBOLS)" \
		modules
endef

TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))

MAKE_FLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$c)))

TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y


define KernelPackage/wlan_hwifi/install
endef

$(eval $(call KernelPackage,wlan_hwifi))


