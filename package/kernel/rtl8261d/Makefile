# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8261d
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/RuijieNetworksCommunity/rtl8261-phy-driver.git
PKG_SOURCE_DATE:=2026-04-21
PKG_SOURCE_VERSION:=19fa6fa8347026f35b6cb41a5785b76e0e315b14
PKG_MIRROR_HASH:=a8dcba9ca42920f54bbf1f7e532d6547d6ddd64e2005fa80a20772d38f33cdcc

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_PARALLEL:=1
PKG_EXTMOD_SUBDIRS:=$(if $(filter backport,$(BUILD_VARIANT)),backport-src,src)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/phy-rtl8261d
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8261D 10 Gigabit PHY driver (Vendor legacy)
  VARIANT:=legacy
  DEFAULT_VARIANT:=1
  DEPENDS:=@TARGET_mediatek +kmod-libphy +kmod-hwmon-core
  CONFLICTS:=kmod-phy-rtl8261d-backport
  FILES:=$(PKG_BUILD_DIR)/src/rtl8261d.ko
  AUTOLOAD:=$(call AutoLoad,18,rtl8261d,1)
endef

define KernelPackage/phy-rtl8261d/description
  Vendor legacy Realtek RTL8261D 10 Gigabit PHY driver with signed MDIO
  error propagation, EDPD u16 tunable, config_intr, and hwmon fixes.
endef

define KernelPackage/phy-rtl8261d-backport
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8261C/CE/D 10GbE PHY driver (C45 backport)
  VARIANT:=backport
  DEPENDS:=@TARGET_mediatek +kmod-libphy +kmod-hwmon-core
  CONFLICTS:=kmod-phy-rtl8261d
  FILES:=$(PKG_BUILD_DIR)/backport-src/rtl8261d.ko
  AUTOLOAD:=$(call AutoLoad,18,rtl8261d,1)
endef

define KernelPackage/phy-rtl8261d-backport/description
  Clause-45 Realtek RTL8261C/CE/D 10GbE PHY driver backport with graceful
  optional firmware loading, USXGMII/10GBASE-R rate matching and SerDes
  resync, optional DT SerDes polarity inversion, hwmon, and PHY LEDs.
endef

ifeq ($(BUILD_VARIANT),backport)
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/backport-src
	$(CP) ./backport-src/* $(PKG_BUILD_DIR)/backport-src/
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/backport-src" \
		CONFIG_REALTEK_PHY_RTL8261D_BACKPORT=m \
		modules
endef
else
define Build/Prepare
	$(call Build/Prepare/Default)
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_REALTEK_PHY_RTL8261D=m \
		modules
endef
endif

$(eval $(call KernelPackage,phy-rtl8261d))
$(eval $(call KernelPackage,phy-rtl8261d-backport))
