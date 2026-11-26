Package/airoha-en8811h-firmware = $(call Package/firmware-default,Airoha EN8811H 2.5G Ethernet PHY firmware,,LICENSE.airoha)
define Package/airoha-en8811h-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/EthMD32.dm.bin \
		$(PKG_BUILD_DIR)/airoha/EthMD32.DSP.bin \
		$(1)/lib/firmware/airoha
ifneq ($(filter y,$(CONFIG_TARGET_mediatek_filogic) $(CONFIG_TARGET_mediatek_filogic_mtwifi) $(CONFIG_TARGET_mediatek_filogic_hwifi) $(CONFIG_TARGET_mediatek_filogic_hwifi_a73) $(CONFIG_TARGET_mediatek_filogic_a73)),)
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	cat \
		$(PKG_BUILD_DIR)/airoha/EthMD32.dm.bin \
		$(PKG_BUILD_DIR)/airoha/EthMD32.DSP.bin \
		> $(STAGING_DIR_IMAGE)/EthMD32.bin
endif
endef

$(eval $(call BuildPackage,airoha-en8811h-firmware))
