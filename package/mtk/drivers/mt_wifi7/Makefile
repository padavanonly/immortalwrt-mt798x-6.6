# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt_wifi7
P4REV:=
PKG_VERSION:=TEST
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

PKG_KCONFIG:= \
	AP_SUPPORT \
	RT_FIRST_CARD \
	RT_SECOND_CARD \
	RT_FIRST_IF_RF_OFFSET \
	RT_SECOND_IF_RF_OFFSET \
	MT_WIFI \
	WIFI_BASIC_FUNC \
	MT_WIFI_PATH \
	FIRST_IF_NONE \
	FIRST_IF_EEPROM_FLASH \
	FIRST_IF_EEPROM_EFUSE \
	RT_FIRST_CARD_EEPROM \
	SECOND_IF_NONE \
	SECOND_IF_EEPROM_FLASH \
	SECOND_IF_EEPROM_PROM \
	SECOND_IF_EEPROM_EFUSE \
	RT_SECOND_CARD_EEPROM \
	MULTI_INF_SUPPORT \
	WIFI_BASIC_FUNC \
	WIRELESS_EXT \
	WEXT_SPY \
	WEXT_PRIV \
	DOT11_N_SUPPORT \
	DOT11_VHT_AC \
	DOT11_HE_AX \
	DOT11_EHT_BE \
	CFG_SUPPORT_FALCON_MURU \
	CFG_SUPPORT_FALCON_TXCMD_DBG \
	CFG_SUPPORT_FALCON_SR \
	CFG_SUPPORT_FALCON_PP \
	WIFI_DRIVER \
	G_BAND_256QAM_SUPPORT \
	BRCM_256QAM_SUPPORT \
	ICAP_SUPPORT \
	MT_AP_SUPPORT \
	BACKGROUND_SCAN_SUPPORT \
	SMART_CARRIER_SENSE_SUPPORT \
	SCS_FW_OFFLOAD \
	THERMAL_PROTECT_SUPPORT \
	MT_DFS_SUPPORT \
	MT_BAND4_DFS_SUPPORT \
	HDR_TRANS_TX_SUPPORT \
	CHIP_MT7615E \
	HDR_TRANS_RX_SUPPORT \
	DBDC_MODE \
	MULTI_PROFILE_SUPPORT \
	DEFAULT_5G_PROFILE \
	SUPPORT_DYNAMIC_TXOP \
	WSC_INCLUDED \
	MT_STA_SUPPORT \
	WSC_V2_SUPPORT \
	DOT11W_PMF_SUPPORT \
	PASSPOINT_R2 \
	PROXYARP_SUPPORT \
	OFFCHANNEL_SUPPORT \
	BS_MAP_BL_SUPPORT \
	TXBF_SUPPORT \
	IGMP_SNOOP_SUPPORT \
	RATE_ADAPTION \
	RATE_ADAPT_AGBS_SUPPORT \
	RTMP_FLASH_SUPPORT \
	ATE_SUPPORT \
	FPGA_MODE \
	WLAN_SERVICE \
	UAPSD \
	RTMP_PCI_SUPPORT \
	RTMP_USB_SUPPORT \
	RTMP_RBUS_SUPPORT \
	WIFI_MODE_AP \
	WIFI_MODE_STA \
	WIRELESS_EXT \
	WEXT_SPY \
	WEXT_PRIV \
	WDS_SUPPORT \
	MBSS_SUPPORT \
	APCLI_SUPPORT \
	APCLI_CERT_SUPPORT \
	APCLI_CONNECTION_TRIAL \
	MAC_REPEATER_SUPPORT \
	RALINK_RT6352 \
	RALINK_MT7620 \
	RALINK_MT7603E \
	CON_WPS_SUPPORT \
	VOW_SUPPORT \
	BAND_STEERING \
	TXOP_ARBITER \
	CFG_SUPPORT_DYNAMIC_TXOP \
	WIFI_MODE_BOTH \
	CHIP_MT7603E \
	CHIP_MT7615E \
	MT_MAC \
	RATE_ADAPTION \
	SUPPORT_OPENWRT \
	SDK_USER_LIGHTY \
	MUMIMO_SUPPORT \
	MU_RA_SUPPORT \
	LED_CONTROL_SUPPORT \
	RA_HW_NAT \
	RA_HW_NAT_WIFI_NEW_ARCH \
	CFG80211_SUPPORT \
	HOSTAPD_MBSS_SUPPORT \
	SER_SUPPORT \
	GREENAP_SUPPORT \
	RADIUS_ACCOUNTING_SUPPORT \
	TPC_SUPPORT \
	RLM_CAL_CACHE_SUPPORT \
	CAL_BIN_FILE_SUPPORT \
	RF_LOCKDOWN_SUPPORT \
	PASSPOINT_R2 \
	RED_SUPPORT \
	FIRST_IF_EPAELNA \
	FIRST_IF_IPAILNA \
	FIRST_IF_IPAELNA \
	FIRST_IF_EPAILNA \
	SECOND_IF_EPAELNA \
	SECOND_IF_IPAILNA \
	SECOND_IF_IPAELNA \
	SECOND_IF_EPAILNA \
	THIRD_IF_EPAELNA \
	THIRD_IF_IPAILNA \
	THIRD_IF_IPAELNA \
	THIRD_IF_EPAILNA \
	WIFI_PKT_FWD \
	DOT11K_RRM_SUPPORT \
	DOT11R_FT_SUPPORT \
	ENTERPRISE_AP_SUPPORT \
	WIFI_EAP_FEATURE \
	TXRX_STAT_SUPPORT \
	ANTENNA_CONTROL_SUPPORT \
	MGMT_TXPWR_CTRL \
	DATA_TXPWR_CTRL \
	TXD_MGMT_TXPWR_CTRL \
	CHUTIL_SUPPORT \
	NF_SUPPORT \
	RA_PHY_RATE_SUPPORT \
	MBSS_DTIM_SUPPORT \
	AMPDU_CONF_SUPPORT \
	ACK_CTS_TIMEOUT_SUPPORT \
	TX_LATENCY_SUPPORT \
	RADIUS_MAC_AUTH_SUPPORT \
	VLAN_SUPPORT \
	VLAN_GTK_SUPPORT \
	DYNAMIC_VLAN_SUPPORT \
	CUSTOMISED_HOSTAPD_SUPPORT \
	APCLI_SUPPLICANT_SUPPORT \
	WDS_STA_SUPPORT \
	MBSS_AS_WDS_AP_SUPPORT \
	MBO_SUPPORT \
	MAP_SUPPORT \
	MAP_R2_VER_SUPPORT \
	MAP_R3_VER_SUPPORT \
	MAP_R4_VER_SUPPORT \
	MAP_R5_VER_SUPPORT \
	MAP_R2_6E_SUPPORT \
	MAP_R3_6E_SUPPORT \
	MAP_VENDOR_SUPPORT \
	MAP_R6_SUPPORT \
	MAP_HOSTAPD_SUPPORT \
	REPORT_CH_TXPWR_SUPPORT \
	WPA3_SUPPORT \
	OWE_SUPPORT \
	WIFI_PKT_FWD_V1 \
	FIRST_IF_MT7915 \
	FIRST_IF_MT7990 \
	FIRST_IF_BELLWETHER \
	FIRST_IF_MT7992 \
	FIRST_IF_MT7993 \
	FIRST_IF_BLACKHAWK \
	SECOND_IF_MT7915 \
	SECOND_IF_MT7990 \
	SECOND_IF_BELLWETHER \
	SECOND_IF_MT7992 \
	SECOND_IF_MT7993 \
	SECOND_IF_BLACKHAWK \
	THIRD_IF_NONE \
	THIRD_IF_MT7915 \
	THIRD_IF_MT7990 \
	THIRD_IF_BELLWETHER \
	THIRD_IF_MT7992 \
	THIRD_IF_MT7993 \
	THIRD_IF_BLACKHAWK \
	CHIP_MT7915 \
	CHIP_MT7990 \
	CHIP_BELLWETHER \
	CHIP_MT7992 \
	CHIP_MT7993 \
	CHIP_BLACKHAWK \
	RT_THIRD_CARD \
	RT_THIRD_IF_RF_OFFSET \
	THIRD_IF_EEPROM_FLASH \
	THIRD_IF_EEPROM_PROM \
	THIRD_IF_EEPROM_EFUSE \
	RT_THIRD_CARD_EEPROM \
	SPECTRUM_SUPPORT \
	PHY_ICS_SUPPORT \
	MULTI_PROFILE_SUPPORT \
	PRE_CAL_TRX_SET1_SUPPORT \
	MWDS \
	MCAST_RATE_SPECIFIC \
	WLAN_HOOK \
	COEX_SUPPORT \
	EASY_SETUP_SUPPORT \
	EVENT_NOTIFIER_SUPPORT \
	AIR_MONITOR \
	OFFCHANNEL_SCAN_FEATURE \
	WNM_SUPPORT \
	INTERWORKING \
	LINUX_NET_TXQ_SUPPORT \
	MEMORY_SHRINK \
	RPS_EFFICIENCY \
	WHNAT_SUPPORT \
	DEBUGGER_SUPPORT \
	FAST_NAT_SUPPORT \
	PRE_CAL_TRX_SET2_SUPPORT \
	LINK_TEST_SUPPORT \
	TCP_RACK_SUPPORT \
	FQ_SCH_SUPPORT \
	BRCM_256QAM_SUPPORT \
	VHT_TXBF_2G_EPIGRAM_IE_SUPPORT \
	DSCP_QOS_MAP_SUPPORT \
	DSCP_PRI_SUPPORT \
	QOS_R1_SUPPORT \
	VERIFICATION_MODE \
	PCIE_ASPM_DYM_CTRL_SUPPORT \
	MIN_PHY_RATE_SUPPORT \
	FAST_UP_RATE_SUPPORT \
	TXRX_STAT_SUPPORT \
	VENDOR_FEATURE11_SUPPORT \
	WIFI_TWT_SUPPORT \
	CTXD_MEM_CPY_SUPPORT \
	CTXD_SCATTER_AND_GATHER_SUPPORT \
	SINGLE_SKU \
	SNIFFER_SUPPORT \
	SNIFFER_RADIOTAP_SUPPORT \
	WIFI_SYSDVT \
	WARP_V2 \
	OCE_SUPPORT \
	WIFI_FW_BIN_LOAD \
	CONNINFRA_APSOC \
	HWIFI_SUPPORT \
	UNIFIED_COMMAND \
	6G_SUPPORT \
	6G_AFC_SUPPORT \
	ADIE_MT7977 \
	SKU_TYPE \
	SEL_DUAL_BAND_SUPPORT \
	FW_LOG_TYPE \
	ADIE_TYPE \
	TR181_SUPPORT \
	MLME_MULTI_QUEUE_SUPPORT \
	EXPERIMENTAL_FEATURES \
	IGMP_SNOOP_SUPPORT \
	SPECIAL_RPS_SUPPORT \
	BA_REORDER_ARRAY_SUPPORT \
	CSI_SUPPORT \
	RTMP_WLAN_HOOK_SUPPORT \
	CCN34_SPLIT_MAC_SUPPORT \
	CCN67_SPLIT_MAC_SUPPORT \
	EAP_QUICK_CHANNEL_SWITCH_WITHOUT_CSA \
	SW_CONNECT_SUPPORT \
	DFS_SLAVE_SUPPORT \
	ANDLINK_SUPPORT \
	GTB_APP_SUPPORT \
	EXT_BUILD_CHANNEL_LIST \
	MEMORY_SLIM_SUPPORT \
	HIGH_PRIO_QUEUE_SUPPORT \
	BCN_EXTCAP_VAR_LEN \
	FTM_SUPPORT \
	ZERO_PKT_LOSS_SUPPORT \
	IWCOMMAND_CFG80211_SUPPORT \
	CFG80211_FULL_OPS_SUPPORT \
	PWHM_SUPPORT \
	RSNO_SUPPORT \
	ICVERR_RESET_RRO \
	NON_WIFI_FREQ_5MHZ_SHIFT_SUPPORT \
	MLD_GROUP_RESTORE \
	MTK_WIFI7_UNIFIED_COUNTRY_CONFIG_SUPPORT

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MTK_WIFI7_$c),CONFIG_$(c)))

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk


TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

BACKPORT_VERSION:=$(lastword $(subst /, , $(shell find $(KERNEL_BUILD_DIR) -type 'd' -name 'backports*')))

define KernelPackage/mt_wifi7
  CATEGORY:=MTK Properties
  TITLE:=MTK wifi7 AP driver
  DEPENDS:=blkid +MTK_WIFI7_CFG80211_SUPPORT:kmod-cfg80211 +kmod-mt_wifi_cmn
ifeq ($(CONFIG_MTK_CONNINFRA_APSOC),y)
  DEPENDS+=+kmod-conninfra
endif
ifneq ($(CONFIG_MTK_WIFI7_FAST_NAT_SUPPORT), )
ifneq ($(CONFIG_MTK_WIFI7_WHNAT_SUPPORT), )
  DEPENDS+=+kmod-mediatek_hnat
endif
endif
  FILES:=$(PKG_BUILD_DIR)/mt_wifi_ap/mt_wifi.ko
  AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
ifneq ($(CONFIG_MTK_WIFI7_DEBUGGER_SUPPORT), )
  FILES+=$(PKG_BUILD_DIR)/mt_wifi/os/linux/plug_in/debugger/debugger_wifi.ko
endif

ifeq ($(CONFIG_MTK_CONNINFRA_APSOC),y)
ifeq ($(CONFIG_FPGA_EARLY_PORTING),y)
  AUTOLOAD:=
endif
endif
  SUBMENU:=Drivers
  MENU:=1

ifeq ($(CONFIG_MTK_WIFI7_CFG80211_SUPPORT),y)
	AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
endif
endef
define KernelPackage/mt_wifi7/config
	source "$(SOURCE)/config.in"
endef


NOSTDINC_FLAGS :=
ifeq ($(CONFIG_MTK_WIFI7_CFG80211_SUPPORT),y)
NOSTDINC_FLAGS += \
        $(KERNEL_NOSTDINC_FLAGS) \
        -DBACKPORT_NOSTDINC \
        -I$(PKG_BUILD_DIR) \
        -I$(PKG_BUILD_DIR)/include \
        -I$(STAGING_DIR)/usr/include/mac80211-backport \
        -I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
        -I$(STAGING_DIR)/usr/include/mac80211 \
        -I$(STAGING_DIR)/usr/include/mac80211/uapi \
        -include backport/backport.h
endif

ifeq ($(CONFIG_PACKAGE_kmod-mt_wifi_cmn),y)
NOSTDINC_FLAGS += \
	-DMT_WIFI_COMMON_SUPPORT
endif

ifneq ($(CONFIG_MTK_WIFI7_CCN34_SPLIT_MAC_SUPPORT),y)
ifneq ($(CONFIG_MTK_WIFI7_CCN67_SPLIT_MAC_SUPPORT),y)
ifeq ($(CONFIG_MTK_WIFI7_CFG80211_SUPPORT),y)
KCFLAGS += \
	-DNO_MTK_WEXT_PRIV
endif
endif
endif

KCFLAGS += $(if $(CONFIG_MTK_WIFI7_OSAL_SUPPORT), -DMT_WIFI_COMMON_SUPPORT -DMT_WIFI_OSAL_SUPPORT -I$(STAGING_DIR)/usr/include/mt_wifi -I$(STAGING_DIR)/usr/include/mt_wifi/os)

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" V=1 \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/mt_wifi_ap" \
		LINUX_DIR="$(KERNEL_BUILD_DIR)" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MTK_WIFI7_$c),CONFIG_$(c)=$(CONFIG_MTK_WIFI7_$(c)))) \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
	 	KCFLAGS="$(KCFLAGS)" \
		modules
	touch $(PKG_BUILD_DIR)/.no_autorebuild
	cp $(PKG_BUILD_DIR)/mt_wifi_ap/Module.symvers $(PKG_BUILD_DIR)/Module.symvers
endef

define KernelPackage/mt_wifi7/install
	rm -rf $(1)/lib/firmware/
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek
ifeq ($(CONFIG_MTK_WIFI7_WFA_SCRIPTS),y)
	rm -rf $(1)/etc/wireless/mediatek/wfa_scripts
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/APSetting
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/FixedRate
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/handy_commands
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/Performance_BRCM
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/SnifferSetting
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/TX_pwr
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/wfa_scripts/WiFi7_setting
endif
ifeq ($(CONFIG_MTK_WIFI7_CHIP_BELLWETHER),y)
ifeq ($(CONFIG_MTK_WIFI7_ADIE_MT7977),y)
ifeq ($(CONFIG_MTK_WIFI7_DUAL_ADIE_MT7977), y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/BELLWETHER_EEPROM_MT7977D.bin $(1)/lib/firmware/BELLWETHER_EEPROM.bin
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/BELLWETHER_EEPROM_MT7977_5G.bin $(1)/lib/firmware/BELLWETHER_EEPROM.bin
endif
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/bellwether/rebb/BELLWETHER_EEPROM.bin $(1)/lib/firmware/.
ifeq ($(CONFIG_MTK_WIFI7_WFA_SCRIPTS),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/APSetting/* $(1)/etc/wireless/mediatek/wfa_scripts/APSetting
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/FixedRate/* $(1)/etc/wireless/mediatek/wfa_scripts/FixedRate
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/handy_commands/* $(1)/etc/wireless/mediatek/wfa_scripts/handy_commands
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/Performance_BRCM/* $(1)/etc/wireless/mediatek/wfa_scripts/Performance_BRCM
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/SnifferSetting/* $(1)/etc/wireless/mediatek/wfa_scripts/SnifferSetting
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/TX_pwr/* $(1)/etc/wireless/mediatek/wfa_scripts/TX_pwr
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_driver/tools/wfa_scripts/WiFi7_setting/* $(1)/etc/wireless/mediatek/wfa_scripts/WiFi7_setting
endif
endif
endif
ifeq ($(CONFIG_MTK_WIFI7_CHIP_MT7990),y)
ifeq ($(CONFIG_MTK_WIFI7_SKU_TYPE),"BE13000_255")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_EEPROM_255_eFEM.bin $(1)/lib/firmware/.
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_iPAiLNA_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_EEPROM_iFEM233.bin $(1)/lib/firmware/.
endif
ifeq ($(CONFIG_MTK_WIFI7_ADIE_TYPE),"MT7977")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7977/MT7990_7977_EEPROM.bin $(1)/lib/firmware/MT7990_EEPROM.bin
else ifeq ($(CONFIG_MTK_WIFI7_ADIE_TYPE),"MT7979")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/pre8_7979/MT7990_7979_EEPROM.bin $(1)/lib/firmware/MT7990_EEPROM.bin
else ifeq ($(CONFIG_MTK_WIFI7_ADIE_TYPE),"MT7777")
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/uni_e2p_7777/MT7990_7777_EEPROM.bin $(1)/lib/firmware/MT7990_EEPROM.bin
endif

endif
ifeq ($(CONFIG_MTK_WIFI7_CHIP_MT7992),y)
ifeq ($(CONFIG_MTK_WIFI7_SEL_DUAL_BAND_SUPPORT),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_SDB_EEPROM.bin $(1)/lib/firmware/.
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7975_MT7979_EEPROM_BE7200_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7978_MT7979_EEPROM_BE6500_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7976_MT7977_EEPROM_BE7200_ePAeLNA_midFEM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7975_MT7977_EEPROM_BE7200_2i5e_sky_nonlinear.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7991_MT7976_EEPROM_BE5040_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7991_MT7976_EEPROM_BE5040_ePAeLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt_wifi/tools/fw_parser/get_consys_log.sh $(1)/etc/wireless/mediatek/
endif
endif
ifeq ($(CONFIG_MTK_WIFI7_CHIP_MT7993),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/MT7993_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/MT7993_MT7976_EEPROM_BE3600_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7993/rebb/MT7993_MT7976_EEPROM_BE3600_ePAeLNA.bin $(1)/lib/firmware/.
	if [ -d "$(PKG_BUILD_DIR)/wifi_driver" ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt_wifi/tools/fw_parser/get_consys_log.sh $(1)/etc/wireless/mediatek/; \
	else \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt_wifi/tools/fw_parser/get_consys_log.sh $(1)/etc/wireless/mediatek/; \
	fi
endif
ifeq ($(CONFIG_MTK_WIFI7_CHIP_BLACKHAWK),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/blackhawk/rebb/BLACKHAWK_A0_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/blackhawk/rebb/BLACKHAWK_A0_MT7976_EEPROM_BN25000_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/blackhawk/rebb/BLACKHAWK_A0_MT7976_EEPROM_BN25000_ePAeLNA.bin $(1)/lib/firmware/.
endif
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifi_e2p.init $(1)/etc/init.d/wifi_e2p
ifeq ($(CONFIG_MTK_WIFI7_SEL_DUAL_BAND_SUPPORT),y)
	$(INSTALL_BIN) ./files/wifi_e2p_sdb.init $(1)/etc/init.d/wifi_e2p
endif
	$(INSTALL_BIN) ./files/wifi_testmode.init $(1)/etc/init.d/wifi_testmode
	$(INSTALL_BIN) ./files/wifi_fw_path.init $(1)/etc/init.d/wifi_fw_path
	$(INSTALL_BIN) ./files/wifi_msp.init $(1)/etc/init.d/wifi_msp
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/wifi-testmode.sh $(1)/sbin/testmode.sh
	$(INSTALL_BIN) ./files/wifi-launch_testmode.sh $(1)/sbin/launch_testmode.sh
ifeq ($(CONFIG_MTK_WIFI7_SEL_DUAL_BAND_SUPPORT),y)
	$(INSTALL_BIN) ./files/wifi-select_sku.sh $(1)/sbin/select_sku.sh
endif
ifeq ($(CONFIG_MTK_WIFI7_PROFILING_SUPPORT),y)
	$(INSTALL_DIR) $(1)/etc/wireless/
	# Example combination mt7622 + Kite.
	if [ "$$(CONFIG_first_card_name)" = "MT7992" -o "$$(CONFIG_second_card_name)" = "MT7992" ] ; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt_wifi/profiles/mt7992/* $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_WIFI7_SKU_TYPE)" = "BE3600SDB" ] ; then \
			cp $(1)/etc/wireless/l1profile.dat $(1)/etc/wireless/l1profile_3.dat ; \
			sed -i "s/mt7992.1_1.dat/mt7992.1_3.dat/" $(1)/etc/wireless/l1profile_3.dat; \
			sed -i "s/EEPROM_offset="0x0"/EEPROM_offset="0x4000"/" $(1)/etc/wireless/l1profile_3.dat; \
			cp $(1)/etc/wireless/l1profile.dat $(1)/etc/wireless/l1profile_2.dat ; \
			sed -i "s/mt7992.1_1.dat/mt7992.1_2.dat/" $(1)/etc/wireless/l1profile_2.dat; \
			sed -i "s/EEPROM_offset="0x0"/EEPROM_offset="0x2000"/" $(1)/etc/wireless/l1profile_2.dat; \
			cp $(1)/etc/wireless/l1profile.dat $(1)/etc/wireless/l1profile_1.dat ; \
		fi; \
	fi
endif
endef

$(eval $(call KernelPackage,mt_wifi7))
