# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hwifi
P4REV:=
PKG_VERSION:=
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=\
	HWIFI \
	HWIFI_HE_SUPPORT \
	HWIFI_HIF_PCI \
	HWIFI_IF_MTK \
	HWIFI_IF_MAC80211 \
	HWIFI_CHIP_MT7915 \
	HWIFI_CHIP_MT7916 \
	HWIFI_CHIP_MT7915_IF_TYPE \
	HWIFI_CHIP_MT7915_DBG \

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)

define KernelPackage/hwifi
  CATEGORY:=MTK Properties
  TITLE:=MTK hwifi driver
  DEPENDS:= +kmod-mt_wifi
  FILES:=$(PKG_BUILD_DIR)/mtk_hwifi.ko
  AUTOLOAD:=$(call AutoProbe,mtk_hwifi)
  SUBMENU:=Drivers
  MENU:=1
endef

define KernelPackage/hwifi/config
	source "$(SOURCE)/config.in"
endef

ifeq ($(MTK_RELEASE_VERSION),trunk)
USE_SOURCE_DIR:=$(MTK_WIFI_DIR)/hwifi/hwifi_drv
define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	$(if $(wildcard $(USE_SOURCE_DIR)/*),,@echo "Error: USE_SOURCE_DIR=$(USE_SOURCE_DIR) path not found"; false)
	cp -rf $(USE_SOURCE_DIR) $(PKG_BUILD_DIR)
	rm -rf $(PKG_BUILD_DIR)/.git
	$(Build/Patch)
	touch $(PKG_BUILD_DIR)/.source_dir
	touch $(PKG_BUILD_DIR)/.no_autorebuild
endef
endif

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c)))) \
		MTK_WIFI_MAC="../mt_wifi/mt_wifi/" \
		modules
endef

TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))

MAKE_FLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$c))) 

TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y


define KernelPackage/hwifi/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/firmware/*.bin $(1)/lib/firmware/mediatek/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/script/* $(1)/etc/wireless/mediatek/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.ko $(1)/lib/modules/4.4.226/
endef

$(eval $(call KernelPackage,hwifi))


