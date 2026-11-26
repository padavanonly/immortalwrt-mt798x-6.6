# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

# The order-only guard is not fail-closed under make -i. Refuse it before rules.mk
# and package.mk can alter MAKEFLAGS or any package recipe can run.
ifneq ($(origin MAKEFLAGS),file)
  $(error Vendor Wi-Fi guarded packages reject explicit MAKEFLAGS overrides)
endif
ifneq ($(flavor MAKEFLAGS),recursive)
  $(error Vendor Wi-Fi guarded packages reject immediate MAKEFLAGS assignments)
endif
ifneq ($(findstring i,$(filter-out --% %=%,$(firstword $(MAKEFLAGS)))),)
  $(error Vendor Wi-Fi guarded packages refuse make -i/--ignore-errors)
endif
ifneq ($(filter --ignore-errors,$(MAKEFLAGS)),)
  $(error Vendor Wi-Fi guarded packages refuse make -i/--ignore-errors)
endif
ifneq ($(origin CURDIR),file)
  $(error mt_wifi7 requires CURDIR from GNU make)
endif
ifneq ($(origin USE_SOURCE_DIR),undefined)
  $(error mt_wifi7 rejects USE_SOURCE_DIR before package.mk expansion)
endif
ifneq ($(origin USE_GIT_TREE),undefined)
  $(error mt_wifi7 rejects USE_GIT_TREE before package.mk expansion)
endif
ifneq ($(origin USE_GIT_SRC_CHECKOUT),undefined)
  $(error mt_wifi7 rejects USE_GIT_SRC_CHECKOUT before package.mk expansion)
endif
ifneq ($(origin KERNEL_BUILD_DIR),undefined)
  ifneq ($(origin KERNEL_BUILD_DIR),file)
    $(error Vendor Wi-Fi guarded packages require KERNEL_BUILD_DIR from the package build rules)
  endif
endif
ifneq ($(origin BUILD_DIR_BASE),undefined)
  $(error Vendor Wi-Fi guarded packages reject BUILD_DIR_BASE overrides)
endif
ifneq ($(origin BUILD_DIR),undefined)
  $(error Vendor Wi-Fi guarded packages reject BUILD_DIR overrides)
endif
ifneq ($(origin PKG_NAME),undefined)
  $(error mt_wifi7 rejects PKG_NAME overrides before package includes)
endif
ifneq ($(origin PKG_BUILD_DIR),undefined)
  $(error mt_wifi7 rejects PKG_BUILD_DIR overrides before package includes)
endif

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt_wifi7
ifneq ($(origin PKG_NAME),file)
  $(error mt_wifi7 rejects command-line/environment identity override)
endif
P4REV:=
PKG_VERSION:=TEST
PKG_RELEASE:=21
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)$(if $(BUILD_VARIANT),-$(BUILD_VARIANT)/$(PKG_NAME))
ifneq ($(origin PKG_BUILD_DIR),file)
  $(error mt_wifi7 rejects command-line/environment build path override)
endif

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
	UNIFIED_COUNTRY_CONFIG_SUPPORT

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MTK_WIFI7_$c),CONFIG_$(c)))

PKG_BUILD_PARALLEL:=1

MTK_DEFAULT_EEPROM := $(CURDIR)/files/eeprom-default/bin/MT7991_MT7976_EEPROM_BE5040_iPAiLNA.bin
MTK_DEFAULT_EEPROM_SHA256 := a500477cbe4a9dc6f3b4d672d58871859af0219d215670a8c7bbdcbf7072287b
PKG_FILE_DEPENDS += $(MTK_DEFAULT_EEPROM)

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	$(call Build/Prepare/Default)
	test -f "$(MTK_DEFAULT_EEPROM)" || { echo "missing pinned default EEPROM: $(MTK_DEFAULT_EEPROM)" >&2; exit 1; }
	eeprom_digest="$$$$($(MKHASH) sha256 "$(MTK_DEFAULT_EEPROM)")" && test "$$$$eeprom_digest" = "$(MTK_DEFAULT_EEPROM_SHA256)" || { echo "pinned default EEPROM source hash mismatch or mkhash failed" >&2; exit 1; }
	$(INSTALL_DIR) "$(PKG_BUILD_DIR)/bin/mt7992/rebb"
	$(CP) "$(MTK_DEFAULT_EEPROM)" "$(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7991_MT7976_EEPROM_BE5040_iPAiLNA.bin"
	eeprom_digest="$$$$($(MKHASH) sha256 "$(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7991_MT7976_EEPROM_BE5040_iPAiLNA.bin")" && test "$$$$eeprom_digest" = "$(MTK_DEFAULT_EEPROM_SHA256)" || { echo "prepared default EEPROM hash mismatch or mkhash failed" >&2; exit 1; }
endef


# Validate the package source/build boundary before package.mk's prepared/clean rm -rf.
override VENDOR_WIFI_GUARD := $(TOPDIR)/package/mtk/drivers/vendor-wifi-guard.sh
override VENDOR_WIFI_SOURCE_DIR := $(CURDIR)/src
.PHONY: vendor-wifi-predelete-guard
vendor-wifi-predelete-guard:
	@$(VENDOR_WIFI_GUARD) "$(TOPDIR)" "$(VENDOR_WIFI_SOURCE_DIR)" "$(KERNEL_BUILD_DIR)" "$(PKG_BUILD_DIR)" "mt_wifi7" "$(if $(filter-out undefined,$(origin USE_SOURCE_DIR)),active,)" "$(if $(filter-out undefined,$(origin USE_GIT_TREE)),active,)" "$(if $(filter-out undefined,$(origin USE_GIT_SRC_CHECKOUT)),active,)" "$(BUILD_VARIANT)"
STAMP_PREPARED_DEPENDS += | vendor-wifi-predelete-guard
force-clean-build: vendor-wifi-predelete-guard


TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

BACKPORT_VERSION:=$(lastword $(subst /, , $(shell find $(KERNEL_BUILD_DIR) -type 'd' -name 'backports*')))

define KernelPackage/mt_wifi7
  CATEGORY:=MTK Properties
  TITLE:=MTK wifi7 AP driver
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +blkid +MTK_WIFI7_CFG80211_SUPPORT:kmod-cfg80211 +kmod-mt_wifi_cmn +wifi-dats +kmod-mediatek_hnat
  FILES:=$(PKG_BUILD_DIR)/mt_wifi_ap/mt_wifi.ko
  AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
ifneq ($(CONFIG_MTK_WIFI7_DEBUGGER_SUPPORT), )
  FILES+=$(PKG_BUILD_DIR)/mt_wifi/os/linux/plug_in/debugger/debugger_wifi.ko
endif

  SUBMENU:=Drivers
  MENU:=1
  VARIANT:=custom
  DEFAULT_VARIANT:=1
  PROVIDES:=kmod-mt_wifi7

ifeq ($(CONFIG_MTK_WIFI7_CFG80211_SUPPORT),y)
	AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
endif
endef
define KernelPackage/mt_wifi7/config
	source "$(SOURCE)/config.in"
endef

define KernelPackage/mt_wifi7-mt7992-be5040
$(call KernelPackage/mt_wifi7)
  TITLE+= (MT7992 BE5040 iPAiLNA)
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +blkid +kmod-mt_wifi_cmn +kmod-mediatek_hnat +kmod-warp-mt7987 +wifi-dats-mt7992
  FILES:=$(PKG_BUILD_DIR)/mt_wifi_ap/mt_wifi.ko
  AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
  VARIANT:=mt7992-be5040
  DEFAULT_VARIANT:=
  PROVIDES:=kmod-mt_wifi7
  CONFLICTS:=kmod-mt_wifi7 kmod-mt_wifi7-mt7992-be7200 kmod-mt_wifi7-mt7990-be19000
endef

define KernelPackage/mt_wifi7-mt7992-be7200
$(call KernelPackage/mt_wifi7)
  TITLE+= (MT7992 BE7200 ePAeLNA)
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +blkid +kmod-mt_wifi_cmn +kmod-mediatek_hnat +kmod-warp-mt7988 +wifi-dats-mt7992
  FILES:=$(PKG_BUILD_DIR)/mt_wifi_ap/mt_wifi.ko
  AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
  VARIANT:=mt7992-be7200
  DEFAULT_VARIANT:=
  PROVIDES:=kmod-mt_wifi7
  CONFLICTS:=kmod-mt_wifi7 kmod-mt_wifi7-mt7992-be5040 kmod-mt_wifi7-mt7990-be19000
endef

define KernelPackage/mt_wifi7-mt7990-be19000
$(call KernelPackage/mt_wifi7)
  TITLE+= (MT7990 BE19000 iPAiLNA)
  DEPENDS:=@(TARGET_mediatek_filogic_hwifi||TARGET_mediatek_filogic_hwifi_a73) +blkid +kmod-mt_wifi_cmn +kmod-mediatek_hnat +kmod-warp-mt7988 +wifi-dats-mt7990
  FILES:=$(PKG_BUILD_DIR)/mt_wifi_ap/mt_wifi.ko
  AUTOLOAD:=$(call AutoLoad,61,mt_wifi)
  VARIANT:=mt7990-be19000
  DEFAULT_VARIANT:=
  PROVIDES:=kmod-mt_wifi7
  CONFLICTS:=kmod-mt_wifi7 kmod-mt_wifi7-mt7992-be5040 kmod-mt_wifi7-mt7992-be7200
endef

ifeq ($(BUILD_VARIANT),mt7992-be5040)
MT_WIFI7_KCONFIG_FLAGS := \
	CONFIG_RT_FIRST_IF_RF_OFFSET=0xc0000 \
	CONFIG_RT_SECOND_IF_RF_OFFSET=0xc8000 \
	CONFIG_MT_WIFI=m \
	CONFIG_MT_WIFI_PATH="mt_wifi" \
	CONFIG_FIRST_IF_EEPROM_FLASH=y \
	CONFIG_MULTI_INF_SUPPORT=y \
	CONFIG_DOT11_N_SUPPORT=y \
	CONFIG_DOT11_VHT_AC=y \
	CONFIG_DOT11_HE_AX=y \
	CONFIG_DOT11_EHT_BE=y \
	CONFIG_CFG_SUPPORT_FALCON_MURU=y \
	CONFIG_CFG_SUPPORT_FALCON_TXCMD_DBG=y \
	CONFIG_CFG_SUPPORT_FALCON_SR=y \
	CONFIG_CFG_SUPPORT_FALCON_PP=y \
	CONFIG_G_BAND_256QAM_SUPPORT=y \
	CONFIG_ICAP_SUPPORT=y \
	CONFIG_MT_AP_SUPPORT=m \
	CONFIG_BACKGROUND_SCAN_SUPPORT=y \
	CONFIG_SMART_CARRIER_SENSE_SUPPORT=y \
	CONFIG_SCS_FW_OFFLOAD=y \
	CONFIG_MT_DFS_SUPPORT=y \
	CONFIG_HDR_TRANS_TX_SUPPORT=y \
	CONFIG_HDR_TRANS_RX_SUPPORT=y \
	CONFIG_DBDC_MODE=y \
	CONFIG_MULTI_PROFILE_SUPPORT=y \
	CONFIG_WSC_INCLUDED=y \
	CONFIG_WSC_V2_SUPPORT=y \
	CONFIG_DOT11W_PMF_SUPPORT=y \
	CONFIG_PROXYARP_SUPPORT=y \
	CONFIG_OFFCHANNEL_SUPPORT=y \
	CONFIG_BS_MAP_BL_SUPPORT=y \
	CONFIG_TXBF_SUPPORT=y \
	CONFIG_IGMP_SNOOP_SUPPORT=y \
	CONFIG_RTMP_FLASH_SUPPORT=y \
	CONFIG_ATE_SUPPORT=y \
	CONFIG_WLAN_SERVICE=y \
	CONFIG_UAPSD=y \
	CONFIG_RTMP_PCI_SUPPORT=y \
	CONFIG_WDS_SUPPORT=y \
	CONFIG_MBSS_SUPPORT=y \
	CONFIG_APCLI_SUPPORT=y \
	CONFIG_APCLI_CERT_SUPPORT=y \
	CONFIG_MAC_REPEATER_SUPPORT=y \
	CONFIG_CON_WPS_SUPPORT=y \
	CONFIG_VOW_SUPPORT=y \
	CONFIG_BAND_STEERING=y \
	CONFIG_MT_MAC=y \
	CONFIG_SUPPORT_OPENWRT=y \
	CONFIG_MUMIMO_SUPPORT=y \
	CONFIG_MU_RA_SUPPORT=y \
	CONFIG_LED_CONTROL_SUPPORT=y \
	CONFIG_GREENAP_SUPPORT=y \
	CONFIG_TPC_SUPPORT=y \
	CONFIG_CAL_BIN_FILE_SUPPORT=y \
	CONFIG_FIRST_IF_IPAILNA=y \
	CONFIG_SECOND_IF_IPAILNA=y \
	CONFIG_DOT11K_RRM_SUPPORT=y \
	CONFIG_DOT11R_FT_SUPPORT=y \
	CONFIG_DATA_TXPWR_CTRL=y \
	CONFIG_VLAN_SUPPORT=y \
	CONFIG_MBO_SUPPORT=y \
	CONFIG_MAP_SUPPORT=y \
	CONFIG_WPA3_SUPPORT=y \
	CONFIG_OWE_SUPPORT=y \
	CONFIG_FIRST_IF_MT7992=y \
	CONFIG_SECOND_IF_MT7992=y \
	CONFIG_THIRD_IF_NONE=y \
	CONFIG_CHIP_MT7992=y \
	CONFIG_SPECTRUM_SUPPORT=y \
	CONFIG_PHY_ICS_SUPPORT=y \
	CONFIG_MWDS=y \
	CONFIG_MCAST_RATE_SPECIFIC=y \
	CONFIG_WLAN_HOOK=y \
	CONFIG_AIR_MONITOR=y \
	CONFIG_OFFCHANNEL_SCAN_FEATURE=y \
	CONFIG_WNM_SUPPORT=y \
	CONFIG_INTERWORKING=y \
	CONFIG_LINUX_NET_TXQ_SUPPORT=y \
	CONFIG_WHNAT_SUPPORT=m \
	CONFIG_FAST_NAT_SUPPORT=y \
	CONFIG_DSCP_PRI_SUPPORT=y \
	CONFIG_QOS_R1_SUPPORT=y \
	CONFIG_PCIE_ASPM_DYM_CTRL_SUPPORT=y \
	CONFIG_WIFI_TWT_SUPPORT=y \
	CONFIG_CTXD_MEM_CPY_SUPPORT=y \
	CONFIG_SINGLE_SKU=y \
	CONFIG_SNIFFER_RADIOTAP_SUPPORT=y \
	CONFIG_WARP_V2=y \
	CONFIG_OCE_SUPPORT=y \
	CONFIG_HWIFI_SUPPORT=y \
	CONFIG_UNIFIED_COMMAND=y \
	CONFIG_SKU_TYPE="BE5040" \
	CONFIG_FW_LOG_TYPE="idx_log" \
	CONFIG_TR181_SUPPORT=y \
	CONFIG_MLME_MULTI_QUEUE_SUPPORT=y \
	CONFIG_RTMP_WLAN_HOOK_SUPPORT=y \
	CONFIG_HIGH_PRIO_QUEUE_SUPPORT=y \
	CONFIG_ZERO_PKT_LOSS_SUPPORT=y \
	CONFIG_UNIFIED_COUNTRY_CONFIG_SUPPORT=y
else ifeq ($(BUILD_VARIANT),mt7992-be7200)
MT_WIFI7_KCONFIG_FLAGS := \
	CONFIG_RT_FIRST_IF_RF_OFFSET=0xc0000 \
	CONFIG_RT_SECOND_IF_RF_OFFSET=0xc8000 \
	CONFIG_MT_WIFI=m \
	CONFIG_MT_WIFI_PATH="mt_wifi" \
	CONFIG_FIRST_IF_EEPROM_FLASH=y \
	CONFIG_MULTI_INF_SUPPORT=y \
	CONFIG_DOT11_N_SUPPORT=y \
	CONFIG_DOT11_VHT_AC=y \
	CONFIG_DOT11_HE_AX=y \
	CONFIG_DOT11_EHT_BE=y \
	CONFIG_CFG_SUPPORT_FALCON_MURU=y \
	CONFIG_CFG_SUPPORT_FALCON_TXCMD_DBG=y \
	CONFIG_CFG_SUPPORT_FALCON_SR=y \
	CONFIG_CFG_SUPPORT_FALCON_PP=y \
	CONFIG_G_BAND_256QAM_SUPPORT=y \
	CONFIG_ICAP_SUPPORT=y \
	CONFIG_MT_AP_SUPPORT=m \
	CONFIG_BACKGROUND_SCAN_SUPPORT=y \
	CONFIG_SMART_CARRIER_SENSE_SUPPORT=y \
	CONFIG_SCS_FW_OFFLOAD=y \
	CONFIG_MT_DFS_SUPPORT=y \
	CONFIG_HDR_TRANS_TX_SUPPORT=y \
	CONFIG_HDR_TRANS_RX_SUPPORT=y \
	CONFIG_DBDC_MODE=y \
	CONFIG_MULTI_PROFILE_SUPPORT=y \
	CONFIG_WSC_INCLUDED=y \
	CONFIG_WSC_V2_SUPPORT=y \
	CONFIG_DOT11W_PMF_SUPPORT=y \
	CONFIG_PROXYARP_SUPPORT=y \
	CONFIG_OFFCHANNEL_SUPPORT=y \
	CONFIG_BS_MAP_BL_SUPPORT=y \
	CONFIG_TXBF_SUPPORT=y \
	CONFIG_IGMP_SNOOP_SUPPORT=y \
	CONFIG_RTMP_FLASH_SUPPORT=y \
	CONFIG_ATE_SUPPORT=y \
	CONFIG_WLAN_SERVICE=y \
	CONFIG_UAPSD=y \
	CONFIG_RTMP_PCI_SUPPORT=y \
	CONFIG_WDS_SUPPORT=y \
	CONFIG_MBSS_SUPPORT=y \
	CONFIG_APCLI_SUPPORT=y \
	CONFIG_APCLI_CERT_SUPPORT=y \
	CONFIG_MAC_REPEATER_SUPPORT=y \
	CONFIG_CON_WPS_SUPPORT=y \
	CONFIG_VOW_SUPPORT=y \
	CONFIG_BAND_STEERING=y \
	CONFIG_MT_MAC=y \
	CONFIG_SUPPORT_OPENWRT=y \
	CONFIG_MUMIMO_SUPPORT=y \
	CONFIG_MU_RA_SUPPORT=y \
	CONFIG_LED_CONTROL_SUPPORT=y \
	CONFIG_GREENAP_SUPPORT=y \
	CONFIG_TPC_SUPPORT=y \
	CONFIG_CAL_BIN_FILE_SUPPORT=y \
	CONFIG_FIRST_IF_EPAELNA=y \
	CONFIG_SECOND_IF_EPAELNA=y \
	CONFIG_DOT11K_RRM_SUPPORT=y \
	CONFIG_DOT11R_FT_SUPPORT=y \
	CONFIG_DATA_TXPWR_CTRL=y \
	CONFIG_VLAN_SUPPORT=y \
	CONFIG_MBO_SUPPORT=y \
	CONFIG_MAP_SUPPORT=y \
	CONFIG_WPA3_SUPPORT=y \
	CONFIG_OWE_SUPPORT=y \
	CONFIG_FIRST_IF_MT7992=y \
	CONFIG_SECOND_IF_MT7992=y \
	CONFIG_THIRD_IF_NONE=y \
	CONFIG_CHIP_MT7992=y \
	CONFIG_SPECTRUM_SUPPORT=y \
	CONFIG_PHY_ICS_SUPPORT=y \
	CONFIG_MWDS=y \
	CONFIG_MCAST_RATE_SPECIFIC=y \
	CONFIG_WLAN_HOOK=y \
	CONFIG_AIR_MONITOR=y \
	CONFIG_OFFCHANNEL_SCAN_FEATURE=y \
	CONFIG_WNM_SUPPORT=y \
	CONFIG_INTERWORKING=y \
	CONFIG_LINUX_NET_TXQ_SUPPORT=y \
	CONFIG_WHNAT_SUPPORT=m \
	CONFIG_FAST_NAT_SUPPORT=y \
	CONFIG_DSCP_PRI_SUPPORT=y \
	CONFIG_QOS_R1_SUPPORT=y \
	CONFIG_PCIE_ASPM_DYM_CTRL_SUPPORT=y \
	CONFIG_WIFI_TWT_SUPPORT=y \
	CONFIG_CTXD_MEM_CPY_SUPPORT=y \
	CONFIG_SINGLE_SKU=y \
	CONFIG_SNIFFER_RADIOTAP_SUPPORT=y \
	CONFIG_WARP_V2=y \
	CONFIG_OCE_SUPPORT=y \
	CONFIG_HWIFI_SUPPORT=y \
	CONFIG_UNIFIED_COMMAND=y \
	CONFIG_SKU_TYPE="BE7200" \
	CONFIG_FW_LOG_TYPE="idx_log" \
	CONFIG_TR181_SUPPORT=y \
	CONFIG_MLME_MULTI_QUEUE_SUPPORT=y \
	CONFIG_RTMP_WLAN_HOOK_SUPPORT=y \
	CONFIG_HIGH_PRIO_QUEUE_SUPPORT=y \
	CONFIG_ZERO_PKT_LOSS_SUPPORT=y \
	CONFIG_UNIFIED_COUNTRY_CONFIG_SUPPORT=y
else ifeq ($(BUILD_VARIANT),mt7990-be19000)
MT_WIFI7_KCONFIG_FLAGS := \
	CONFIG_RT_FIRST_IF_RF_OFFSET=0xc0000 \
	CONFIG_RT_SECOND_IF_RF_OFFSET=0xc8000 \
	CONFIG_MT_WIFI=m \
	CONFIG_MT_WIFI_PATH="mt_wifi" \
	CONFIG_FIRST_IF_EEPROM_FLASH=y \
	CONFIG_MULTI_INF_SUPPORT=y \
	CONFIG_DOT11_N_SUPPORT=y \
	CONFIG_DOT11_VHT_AC=y \
	CONFIG_DOT11_HE_AX=y \
	CONFIG_DOT11_EHT_BE=y \
	CONFIG_CFG_SUPPORT_FALCON_MURU=y \
	CONFIG_CFG_SUPPORT_FALCON_TXCMD_DBG=y \
	CONFIG_CFG_SUPPORT_FALCON_SR=y \
	CONFIG_CFG_SUPPORT_FALCON_PP=y \
	CONFIG_G_BAND_256QAM_SUPPORT=y \
	CONFIG_ICAP_SUPPORT=y \
	CONFIG_MT_AP_SUPPORT=m \
	CONFIG_BACKGROUND_SCAN_SUPPORT=y \
	CONFIG_SMART_CARRIER_SENSE_SUPPORT=y \
	CONFIG_SCS_FW_OFFLOAD=y \
	CONFIG_MT_DFS_SUPPORT=y \
	CONFIG_HDR_TRANS_TX_SUPPORT=y \
	CONFIG_HDR_TRANS_RX_SUPPORT=y \
	CONFIG_DBDC_MODE=y \
	CONFIG_MULTI_PROFILE_SUPPORT=y \
	CONFIG_WSC_INCLUDED=y \
	CONFIG_WSC_V2_SUPPORT=y \
	CONFIG_DOT11W_PMF_SUPPORT=y \
	CONFIG_PASSPOINT_R2=y \
	CONFIG_PROXYARP_SUPPORT=y \
	CONFIG_OFFCHANNEL_SUPPORT=y \
	CONFIG_BS_MAP_BL_SUPPORT=y \
	CONFIG_TXBF_SUPPORT=y \
	CONFIG_IGMP_SNOOP_SUPPORT=y \
	CONFIG_RTMP_FLASH_SUPPORT=y \
	CONFIG_ATE_SUPPORT=y \
	CONFIG_WLAN_SERVICE=y \
	CONFIG_UAPSD=y \
	CONFIG_RTMP_PCI_SUPPORT=y \
	CONFIG_WDS_SUPPORT=y \
	CONFIG_MBSS_SUPPORT=y \
	CONFIG_APCLI_SUPPORT=y \
	CONFIG_APCLI_CERT_SUPPORT=y \
	CONFIG_MAC_REPEATER_SUPPORT=y \
	CONFIG_CON_WPS_SUPPORT=y \
	CONFIG_BAND_STEERING=y \
	CONFIG_MT_MAC=y \
	CONFIG_SUPPORT_OPENWRT=y \
	CONFIG_MUMIMO_SUPPORT=y \
	CONFIG_MU_RA_SUPPORT=y \
	CONFIG_LED_CONTROL_SUPPORT=y \
	CONFIG_GREENAP_SUPPORT=y \
	CONFIG_TPC_SUPPORT=y \
	CONFIG_CAL_BIN_FILE_SUPPORT=y \
	CONFIG_FIRST_IF_IPAILNA=y \
	CONFIG_SECOND_IF_IPAILNA=y \
	CONFIG_THIRD_IF_IPAILNA=y \
	CONFIG_DOT11K_RRM_SUPPORT=y \
	CONFIG_DOT11R_FT_SUPPORT=y \
	CONFIG_ANTENNA_CONTROL_SUPPORT=y \
	CONFIG_MGMT_TXPWR_CTRL=y \
	CONFIG_DATA_TXPWR_CTRL=y \
	CONFIG_RA_PHY_RATE_SUPPORT=y \
	CONFIG_AMPDU_CONF_SUPPORT=y \
	CONFIG_ACK_CTS_TIMEOUT_SUPPORT=y \
	CONFIG_TX_LATENCY_SUPPORT=y \
	CONFIG_VLAN_SUPPORT=y \
	CONFIG_MBO_SUPPORT=y \
	CONFIG_MAP_SUPPORT=y \
	CONFIG_WPA3_SUPPORT=y \
	CONFIG_OWE_SUPPORT=y \
	CONFIG_FIRST_IF_MT7990=y \
	CONFIG_SECOND_IF_MT7990=y \
	CONFIG_THIRD_IF_MT7990=y \
	CONFIG_CHIP_MT7990=y \
	CONFIG_RT_THIRD_IF_RF_OFFSET=0xd0000 \
	CONFIG_SPECTRUM_SUPPORT=y \
	CONFIG_PHY_ICS_SUPPORT=y \
	CONFIG_MWDS=y \
	CONFIG_MCAST_RATE_SPECIFIC=y \
	CONFIG_WLAN_HOOK=y \
	CONFIG_AIR_MONITOR=y \
	CONFIG_OFFCHANNEL_SCAN_FEATURE=y \
	CONFIG_WNM_SUPPORT=y \
	CONFIG_INTERWORKING=y \
	CONFIG_WHNAT_SUPPORT=m \
	CONFIG_FAST_NAT_SUPPORT=y \
	CONFIG_DSCP_PRI_SUPPORT=y \
	CONFIG_QOS_R1_SUPPORT=y \
	CONFIG_PCIE_ASPM_DYM_CTRL_SUPPORT=y \
	CONFIG_WIFI_TWT_SUPPORT=y \
	CONFIG_CTXD_MEM_CPY_SUPPORT=y \
	CONFIG_SINGLE_SKU=y \
	CONFIG_SNIFFER_RADIOTAP_SUPPORT=y \
	CONFIG_WARP_V2=y \
	CONFIG_WIFI_FW_BIN_LOAD=y \
	CONFIG_HWIFI_SUPPORT=y \
	CONFIG_UNIFIED_COMMAND=y \
	CONFIG_6G_SUPPORT=y \
	CONFIG_SKU_TYPE="BE19000" \
	CONFIG_FW_LOG_TYPE="string_log" \
	CONFIG_TR181_SUPPORT=y \
	CONFIG_MLME_MULTI_QUEUE_SUPPORT=y \
	CONFIG_CSI_SUPPORT=y \
	CONFIG_RTMP_WLAN_HOOK_SUPPORT=y \
	CONFIG_EAP_QUICK_CHANNEL_SWITCH_WITHOUT_CSA=y \
	CONFIG_HIGH_PRIO_QUEUE_SUPPORT=y \
	CONFIG_RSNO_SUPPORT=y \
	CONFIG_UNIFIED_COUNTRY_CONFIG_SUPPORT=y
else
MT_WIFI7_KCONFIG_FLAGS := $(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MTK_WIFI7_$c),CONFIG_$(c)=$(CONFIG_MTK_WIFI7_$(c))))
endif

NOSTDINC_FLAGS :=
ifeq ($(filter custom,$(or $(BUILD_VARIANT),custom)),custom)
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
endif

ifneq ($(CONFIG_PACKAGE_kmod-mt_wifi_cmn),)
NOSTDINC_FLAGS += \
	-DMT_WIFI_COMMON_SUPPORT
endif

ifeq ($(filter custom,$(or $(BUILD_VARIANT),custom)),custom)
ifneq ($(CONFIG_MTK_WIFI7_CCN34_SPLIT_MAC_SUPPORT),y)
ifneq ($(CONFIG_MTK_WIFI7_CCN67_SPLIT_MAC_SUPPORT),y)
ifeq ($(CONFIG_MTK_WIFI7_CFG80211_SUPPORT),y)
MT_WIFI7_KCFLAGS += \
	-DNO_MTK_WEXT_PRIV
endif
endif
endif
MT_WIFI7_KCFLAGS += $(if $(CONFIG_MTK_WIFI7_OSAL_SUPPORT), -DMT_WIFI_COMMON_SUPPORT -DMT_WIFI_OSAL_SUPPORT -I$(STAGING_DIR)/usr/include/mt_wifi -I$(STAGING_DIR)/usr/include/mt_wifi/os)
endif

MT_WIFI7_KERNEL_KCFLAGS = $(call iremap,$(BUILD_DIR),$(notdir $(BUILD_DIR))) $(filter-out -fno-plt,$(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))) $(call qstrip,$(CONFIG_KERNEL_CFLAGS)) $(MT_WIFI7_KCFLAGS)

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" V=1 \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/mt_wifi_ap" \
		LINUX_DIR="$(KERNEL_BUILD_DIR)" \
		$(MT_WIFI7_KCONFIG_FLAGS) \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		KCFLAGS="$(MT_WIFI7_KERNEL_KCFLAGS)" \
		modules
	touch $(PKG_BUILD_DIR)/.no_autorebuild
	cp $(PKG_BUILD_DIR)/mt_wifi_ap/Module.symvers $(PKG_BUILD_DIR)/Module.symvers
endef

define mt_wifi7_install_base_scripts
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifi_e2p.init $(1)/etc/init.d/wifi_e2p
	$(INSTALL_BIN) ./files/wifi_testmode.init $(1)/etc/init.d/wifi_testmode
	$(INSTALL_BIN) ./files/wifi_fw_path.init $(1)/etc/init.d/wifi_fw_path
	$(INSTALL_BIN) ./files/wifi_msp.init $(1)/etc/init.d/wifi_msp
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/wifi-testmode.sh $(1)/sbin/testmode.sh
	$(INSTALL_BIN) ./files/wifi-launch_testmode.sh $(1)/sbin/launch_testmode.sh
endef

define mt_wifi7_install_mt7992
	rm -rf $(1)/lib/firmware/
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7975_MT7979_EEPROM_BE7200_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7978_MT7979_EEPROM_BE6500_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7976_MT7977_EEPROM_BE7200_ePAeLNA_midFEM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7992_MT7975_MT7977_EEPROM_BE7200_2i5e_sky_nonlinear.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7991_MT7976_EEPROM_BE5040_iPAiLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7992/rebb/MT7991_MT7976_EEPROM_BE5040_ePAeLNA.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt_wifi/tools/fw_parser/get_consys_log.sh $(1)/etc/wireless/mediatek/
	$(call mt_wifi7_install_base_scripts,$(1))
endef

define KernelPackage/mt_wifi7-mt7992-be5040/install
	$(call mt_wifi7_install_mt7992,$(1))
endef

define KernelPackage/mt_wifi7-mt7992-be7200/install
	$(call mt_wifi7_install_mt7992,$(1))
endef

define KernelPackage/mt_wifi7-mt7990-be19000/install
	rm -rf $(1)/lib/firmware/
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_iPAiLNA_EEPROM.bin $(1)/lib/firmware/.
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/mt7990/rebb/MT7990_EEPROM_iFEM233.bin $(1)/lib/firmware/.
	$(call mt_wifi7_install_base_scripts,$(1))
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
	$(call mt_wifi7_install_base_scripts,$(1))
ifeq ($(CONFIG_MTK_WIFI7_SEL_DUAL_BAND_SUPPORT),y)
	$(INSTALL_BIN) ./files/wifi_e2p_sdb.init $(1)/etc/init.d/wifi_e2p
	$(INSTALL_BIN) ./files/wifi-select_sku.sh $(1)/sbin/select_sku.sh
endif
ifeq ($(CONFIG_MTK_WIFI7_PROFILING_SUPPORT),y)
	$(INSTALL_DIR) $(1)/etc/wireless/
	if [ "$$(CONFIG_first_card_name)" = "MT7990" -o "$$(CONFIG_second_card_name)" = "MT7990" ] ; then \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/mt_wifi/profiles/mt7990/* $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_WIFI7_SKU_TYPE)" = "BE13000_255" ] ; then \
			$(INSTALL_DATA) $(PKG_BUILD_DIR)/mt_wifi/profiles/mt7990/mt7990.b1.5g.dat $(1)/etc/wireless/mediatek/mt7990.b1.dat; \
			$(INSTALL_DATA) $(PKG_BUILD_DIR)/mt_wifi/profiles/mt7990/mt7990.b2.5g.dat $(1)/etc/wireless/mediatek/mt7990.b2.dat; \
		fi; \
		if [ "$$(CONFIG_MTK_WIFI7_2ADIE_TRIBAND)" = "y" ] ; then \
			sed -i 's/DfsZeroWait=0/DfsZeroWait=1/g' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
			sed -i 's/DfsDedicatedZeroWait=1/DfsDedicatedZeroWait=0/g' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
			sed -i 's/DfsZeroWaitDefault=1/DfsZeroWaitDefault=0/g' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
			sed -i '/DfsZeroWaitDefault/a\DfsNopExpireSetChPolicy=0' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
		fi; \
	fi

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

	# griffin
	if [ "$$(CONFIG_first_card_name)" = "MT7993" -o "$$(CONFIG_second_card_name)" = "MT7993" ] ; then \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/mt_wifi/profiles/griffin/* $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_WIFI7_SKU_TYPE)" = "BE3600_26" ] ; then \
			sed -i "s/mt7993.b1.dat/mt7993.b2.dat/" $(1)/etc/wireless/mediatek/mt7993.1.dat ; \
		fi; \
	fi
endif
endef

$(eval $(call KernelPackage,mt_wifi7))
$(eval $(call KernelPackage,mt_wifi7-mt7992-be5040))
$(eval $(call KernelPackage,mt_wifi7-mt7992-be7200))
$(eval $(call KernelPackage,mt_wifi7-mt7990-be19000))
