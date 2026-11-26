# SPDX-License-Identifier: Apache-2.0
#
# Ported for MediaTek Filogic IMM 24.10 without modifying feeds/luci.
# Original-Author: coolsnowwolf <coolsnowwolf@gmail.com>
# Base-Author: Jo-Philipp Wich <jo@mein.io> (rpcd-mod-luci lease & netlink discovery)
# Upstream: https://github.com/coolsnowwolf/luci (openwrt-25.12)
# Cherry-picked commits:
#   8ac42d9a355f89ccd7414e17f65a6bff5ce62d95
#   22f66d0d9261b8ffd41062ec3ea40dc1d38332ed
#   725ddc051fe44eac07cf39b70708e24bf1539bcc
#   5cfaf67fed77d4a9407c60c31a5abf610c7bb5ad
#   e853b2b146ac0aceac0788b93ad0049e65f1a9d0
#   d7d7e0849df0892541f86186dbde61c99056bf1a

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcd-mod-mtk-clients
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_MAINTAINER:=coolsnowwolf <coolsnowwolf@gmail.com>

PKG_LICENSE:=Apache-2.0

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include

CMAKE_OPTIONS += \
	-DLIBNL_LIBS=-lnl-tiny

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/rpcd-mod-mtk-clients
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Per-client conntrack rate and webprobe rpcd plugin (LEDE port)
  DEPENDS:=+rpcd +libubox +libubus +libuci +libnl-tiny +kmod-nf-conntrack-netlink
endef

define Package/rpcd-mod-mtk-clients/description
  Standalone rpcd plugin providing ubus objects luci.client-rates and
  luci.client-web ported from coolsnowwolf/luci without modifying feeds/luci.
  Original author: coolsnowwolf <coolsnowwolf@gmail.com>
endef

define Package/rpcd-mod-mtk-clients/install
	$(INSTALL_DIR) $(1)/usr/lib/rpcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mtk_clients.so $(1)/usr/lib/rpcd/
endef

define Package/rpcd-mod-mtk-clients/postinst
#!/bin/sh
if [ -z "$$IPKG_INSTROOT" ]; then
	[ -x /etc/init.d/rpcd ] && /etc/init.d/rpcd reload >/dev/null 2>&1 || true
fi
exit 0
endef

$(eval $(call BuildPackage,rpcd-mod-mtk-clients))
