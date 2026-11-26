# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl837x-dsa
PKG_VERSION:=0.2.0
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_PARALLEL:=1
PKG_EXTMOD_SUBDIRS:=src

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8372n-dsa
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8372N/RTL8373 DSA switch driver (Linux 6.6 + 802.1Q PPE)
  DEPENDS:=+kmod-dsa +kmod-phylink +kmod-sfp +kmod-regmap-core
  CONFLICTS:=kmod-switch-rtl837x
  KCONFIG:= \
	CONFIG_NET_DSA_TAG_8021Q=y \
	CONFIG_NET_DSA_TAG_RTL8_4=y
  FILES:=$(PKG_BUILD_DIR)/src/rtl8372n_dsa.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl8372n_dsa,1)
endef

define KernelPackage/rtl8372n-dsa/description
  Linux DSA driver for Realtek RTL8372/RTL8372N/RTL8373 multi-gigabit switches,
  combining airjinkela/rtl837x-dsa-driver with hardware-verified register
  sequences from logicog/RTLPlayground (N-series 64B/66B & 8B/10B SerDes
  polarity, SFP 1G/2.5G/10G dynamic rate switching, MAC_L2_PORT_CTRL 802.1Q tag
  mode, QinQ MTU compensation, LED_GLB_IO_EN bit-6 solid-link fix, and L2 LUT
  aging/walk safety) plus native Linux 6.6 rtl837x-8021q tagger for zero-CPU
  MediaTek PPE HNAT hardware offload on TP-Link TL-7DR7299 v1.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) ./src/* $(PKG_BUILD_DIR)/src/
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_NET_DSA_RTL8372N=m \
		modules
endef

$(eval $(call KernelPackage,rtl8372n-dsa))
