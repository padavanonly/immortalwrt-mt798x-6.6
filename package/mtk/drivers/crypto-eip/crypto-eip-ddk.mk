# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 MediaTek Inc.
#
# Author: Chris.Chou <chris.chou@mediatek.com>
#         Ren-Ting Wang <ren-ting.wang@mediatek.com>

# Configure for crypto-eip DDK makefile
EIP_KERNEL_PKGS+= \
	crypto-eip-ddk \
	crypto-eip-ddk-ksupport \
	crypto-eip-ddk-ctrl \
	crypto-eip-ddk-ctrl-app \
	crypto-eip-ddk-engine

ifneq ($(CONFIG_PACKAGE_kmod-crypto-eip),)
EXTRA_KCONFIG+= \
	CONFIG_RAMBUS_DDK=m

EXTRA_CFLAGS+= \
	-I$(PKG_BUILD_DIR)/ddk/inc \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/configs \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/shdevxs \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/umdevxs \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/device \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/device/lkm \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/device/lkm/of \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/dmares \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/firmware_api \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/kit/builder/sa \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/kit/builder/token \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/kit/eip197 \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/kit/iotoken \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/kit/list \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/kit/ring \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/libc \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/log \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/slad \
	-I$(PKG_BUILD_DIR)/ddk/inc/crypto-eip/ddk/slad/lkm \
	-DEIP197_BUS_VERSION_AXI3 \
	-DDRIVER_64BIT_HOST \
	-DDRIVER_64BIT_DEVICE \
	-DADAPTER_AUTO_TOKENBUILDER
endif

# crypto-eip-ddk kernel package configuration
define KernelPackage/crypto-eip-ddk
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK EIP DDK
  DEFAULT:=n
  DEPENDS:= \
	kmod-crypto-eip
endef

define KernelPackage/crypto-eip-ddk/description
  Porting DDK source code to package.
endef

define KernelPackage/crypto-eip-ddk-ksupport
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK EIP DDK Kernel Support
  DEFAULT:=n
  FILES+=$(PKG_BUILD_DIR)/ddk/build/ksupport/crypto-eip-ddk-ksupport.ko
  DEPENDS:= \
	@CRYPTO_OFFLOAD_INLINE \
	kmod-crypto-eip
endef

define KernelPackage/crypto-eip-ddk-ksupport/description
  Porting DDK source code to package.
endef

define KernelPackage/crypto-eip-ddk-ctrl
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK EIP DDK Global Control Driver
  DEFAULT:=n
  FILES+=$(PKG_BUILD_DIR)/ddk/build/ctrl/crypto-eip-ddk-ctrl.ko
  DEPENDS:= \
	@CRYPTO_OFFLOAD_INLINE \
	kmod-crypto-eip-ddk-ksupport
endef

define KernelPackage/crypto-eip-ddk-ctrl/description
  Porting DDK source code to package.
endef

define KernelPackage/crypto-eip-ddk-ctrl-app
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK EIP DDK Global Control App
  DEFAULT:=n
  FILES+=$(PKG_BUILD_DIR)/ddk/build/app/crypto-eip-ddk-ctrl-app.ko
  DEPENDS:= \
	@CRYPTO_OFFLOAD_INLINE \
	kmod-crypto-eip-ddk-ctrl
endef

define KernelPackage/crypto-eip-ddk-ctrl-app/description
  Porting DDK source code to package.
endef

define KernelPackage/crypto-eip-ddk-engine
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:= MTK EIP DDK engine
  DEFAULT:=n
  FILES+=$(PKG_BUILD_DIR)/ddk/build/engine/crypto-eip-ddk-engine.ko
  DEPENDS:= \
	@CRYPTO_OFFLOAD_INLINE \
	kmod-crypto-eip-ddk-ctrl-app
endef

define KernelPackage/crypto-eip-ddk-engine/description
  Porting DDK source code to package.
endef
