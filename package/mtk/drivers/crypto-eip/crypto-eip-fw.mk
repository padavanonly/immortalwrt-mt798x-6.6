# SPDX-Liscense-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 MediaTek Inc.
#
# Author: Chris.Chou <chris.chou@mediatek.com>
#         Ren-Ting Wang <ren-ting.wang@mediatek.com>

# Configure for crypto firmware makefile
EIP_PKGS+= \
	crypto-eip-inline-fw

define Package/crypto-eip-inline-fw
  TITLE:=Mediatek EIP Firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@CRYPTO_OFFLOAD_INLINE
endef

define Package/crypto-eip-inline-fw/description
  Load firmware for EIP197 inline mode.
endef

define Package/crypto-eip-inline-fw/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(CP) \
		$(PKG_BUILD_DIR)/firmware/bin/firmware_eip207_ifpp.bin \
		$(PKG_BUILD_DIR)/firmware/bin/firmware_eip207_ipue.bin \
		$(PKG_BUILD_DIR)/firmware/bin/firmware_eip207_ofpp.bin \
		$(PKG_BUILD_DIR)/firmware/bin/firmware_eip207_opue.bin \
		$(1)/lib/firmware/
endef
