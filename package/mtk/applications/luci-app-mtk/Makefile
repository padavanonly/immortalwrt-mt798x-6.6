#
# Copyright (C) Hua Shao <nossiac@163.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=MediaTek Proprietary Configuration.
LUCI_DEPENDS:=@!(TARGET_mediatek_filogic||TARGET_mediatek_filogic_mtwifi||TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +datconf-lua
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
