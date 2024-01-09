# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2023, Mediatek Inc. All Rights Reserved.
# Author: Ren-Ting.Wang <ren-ting.wang@mediatek.com>

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pce
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-defaults.mk

EXTRA_KCONFIG+= \
	CONFIG_MTK_PCE=m

EXTRA_CFLAGS+= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

EXTRA_CFLAGS+= -Wall -Werror

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		$(EXTRA_KCONFIG) \
		modules
endef

# pce kernel package configuration
define KernelPackage/pce
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK Packet Classification Engine Driver
  FILES+=$(PKG_BUILD_DIR)/pce.ko
  KCONFIG:=
  DEPENDS:= \
	kmod-mediatek_hnat
endef

define KernelPackage/pce/description
  Support for Mediatek Packet Classification Engine. The HW provides abilities
  to redirect specific packet type to different Mediatek HW offload engine.
  In addition, the HW can be configured with multiple rules, such as IP protocol,
  destination IP address, Layer 4 header or data, to match with the inbound packet.
  Support HW offload engine includes tunnel offload engine, inline crypto offload
  engine.
endef

define KernelPackage/pce-autoload
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK Packet Classification Engine Auto Load
  AUTOLOAD:=$(call AutoLoad,50,pce)
  KCONFIG:=
  DEPENDS:= \
	kmod-pce
endef

define KernelPackage/pce-autoload/description
  Support for Mediatek Packet Classification Engine auto load on system
  boot process.
endef

$(eval $(call KernelPackage,pce))
$(eval $(call KernelPackage,pce-autoload))
