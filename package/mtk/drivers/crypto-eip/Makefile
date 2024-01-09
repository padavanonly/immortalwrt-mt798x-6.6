# SPDX-Liscense-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 MediaTek Inc.
#
# Author: Chris.Chou <chris.chou@mediatek.com>
#         Ren-Ting Wang <ren-ting.wang@mediatek.com>

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=crypto-eip
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-defaults.mk

# For package initialization such as menuconfig or description etc.
EIP_KERNEL_PKGS:=
EIP_PKGS:=

include crypto-eip.mk
include $(filter-out crypto-eip.mk,$(wildcard *.mk))

EXTRA_CFLAGS+= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

EXTRA_CFLAGS+= -Wall -Werror

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/firmware
	$(CP) -r `find ./firmware -maxdepth 1 | grep -v ".git"` $(PKG_BUILD_DIR)/firmware/
	$(CP) -r `find ./src -maxdepth 1 | grep -v ".git"` $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		$(EXTRA_KCONFIG) \
		modules
endef

$(foreach KERNEL_PKG,$(EIP_KERNEL_PKGS),$(eval $(call KernelPackage,$(KERNEL_PKG))))
$(foreach PKG,$(EIP_PKGS),$(eval $(call BuildPackage,$(PKG))))
