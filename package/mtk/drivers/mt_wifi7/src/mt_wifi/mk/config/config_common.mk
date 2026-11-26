################################################################
# Common Feature Selection
################################################################

# Support ATE/QA function
CONFIG_ATE_SUPPORT=n
# Support wlan service function
CONFIG_WLAN_SERVICE=n

#Support 6G
CONFIG_6G_SUPPORT=n

# Support hwifi hw interface
CONFIG_HWIFI_SUPPORT?=y

#ifdef WSC_INCLUDED
# Support WSC function
CONFIG_WSC_INCLUDED=y
CONFIG_WSC_LED=n
CONFIG_WSC_NFC=n
#endif /* WSC_INCLUDED */

#ifdef TXBF_SUPPORT
CONFIG_TXBF_SUPPORT=y
#endif /* TXBF_SUPPORT */

#ifdef UAPSD_SUPPORT
CONFIG_UAPSD=y
#endif

#ifdef CONFIG_CSO_SUPPORT
CONFIG_CSO_SUPPORT=n
#endif

#ifdef AIR_MONITOR
# Support Air Monitor
CONFIG_AIR_MONITOR=n
#endif // AIR_MONITOR //
#ifdef LED_CONTROL_SUPPORT
CONFIG_LED_CONTROL_SUPPORT=n
#endif /* LED_CONTROL_SUPPORT */

#Support TCP_RACK function
CONFIG_TCP_RACK_SUPPORT=n

#ifdef THERMAL_PROTECT_SUPPORT
CONFIG_THERMAL_PROTECT_SUPPORT=y
#endif

#Support Internal-Capture function
CONFIG_ICAP_SUPPORT=y

#Support Wifi-Spectrum function
CONFIG_SPECTRUM_SUPPORT=y

#Support PHY In Chip Sniffer function
CONFIG_PHY_ICS_SUPPORT=y

# Support LLTD function
CONFIG_LLTD=y

#Support features of Single SKU.
CONFIG_SINGLE_SKU_V2_SUPPORT=y

CONFIG_RLM_CAL_CACHE=n

CONFIG_CAL_FREE_IC_SUPPORT=n


#Support Carrier-Sense function
CONFIG_CS_SUPPORT=n



#Support for Net-SNMP
CONFIG_SNMP_SUPPORT=n

#Support TSSI Antenna Variation
CONFIG_TSSI_ANTENNA_VARIATION=n

#ifdef DOT11R_FT_SUPPORT
#Support for dot11r FT
CONFIG_DOT11R_FT_SUPPORT=n
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#Support for dot11k RRM
CONFIG_DOT11K_RRM_SUPPORT=n
#endif /* DOT11K_RRM_SUPPORT */

#ifdef WH_EZ_SETUP
CONFIG_WH_EASY_SETUP=n
#endif

CONFIG_KTHREAD_SUPPORT=n

CONFIG_MEM_ALLOC_INFO_SUPPORT=n

#ifdef MULTI_CARD
# Support for Multiple Cards
CONFIG_MC_SUPPORT=n
#endif /* MULTI_CARD */

#Support for Bridge Fast Path & Bridge Fast Path function open to other module
CONFIG_BGFP_SUPPORT=n
CONFIG_BGFP_OPEN_SUPPORT=n

#Support GreenAP function
CONFIG_GREENAP_SUPPORT=n

#Support PCIE ASPM dynamic control
CONFIG_PCIE_ASPM_DYM_CTRL_SUPPORT=n

#Support 802.11ax TWT
CONFIG_WIFI_TWT_SUPPORT=y

#ifdef FW_DUMP_SUPPORT
CONFIG_FW_DUMP_SUPPORT=n
#endif

CONFIG_DELAY_INT=n

#ifdef CONFIG_TRACE_SUPPORT
CONFIG_TRACE_SUPPORT=n
#endif

#ifdef CONFIG_FW_DEBUG
CONFIG_FW_DEBUG_SUPPORT=y
#endif

#ifdef CONFIG_KEEP_ALIVE_OFFLOAD
CONFIG_KEEP_ALIVE_OFFLOAD=y
#endif

#Wifi MTD related access
CONFIG_WIFI_MTD=n

#ifdef CONFIG_CALIBRATION_COLLECTION
CONFIG_CALIBRATION_COLLECTION_SUPPORT=y
#endif /* CONFIG_CALIBRATION_COLLECTION */

#ifdef WIFI_REGION32_HIDDEN_SSID_SUPPORT
CONFIG_WIFI_REGION32_HIDDEN_SSID_SUPPORT=n
#endif /* WIFI_REGION32_HIDDEN_SSID_SUPPORT */

# Falcon MURU
CONFIG_FALCON_MURU_SUPPORT=y

# MU-MIMO
CONFIG_MU_MIMO_SUPPORT=y

# Falcon SR
CONFIG_FALCON_SR_SUPPORT=y

CONFIG_FLASH_SUPPORT=n

#------ IOT Feature Support ------#
#ifdef USB_IOT_WORKAROUND
# This workaround can fix IOT issue with some specific USB host controllers.
# On those host controllers, a USB packet is unexpectedly divided into 2 smaller
# packets.
CONFIG_USB_IOT_WORKAROUND2=n
#endif

#------ CFG80211 Feature Support (Linux Only) ------#
#Please make sure insmod the cfg80211.ko before our driver
ifneq ($(findstring CFG80211,$(UL_CTRL)),)
CONFIG_CFG80211_SUPPORT=y
CONFIG_APCLI_SUPPLICANT_SUPPORT=y
CONFIG_WNM_SUPPORT=y
CONFIG_DOT11K_RRM_SUPPORT=y
else
CONFIG_CFG80211_SUPPORT=n
endif
#smooth the scan signal for cfg80211 based driver
CONFIG_CFG80211_SCAN_SIGNAL_AVG_SUPPORT=n
#Cfg80211-based P2P Support
CONFIG_CFG80211_P2P_SUPPORT=n
#Cfg80211-based P2P Mode Selection (must one be chosen)
CONFIG_CFG80211_P2P_CONCURRENT_DEVICE=n
CONFIG_CFG80211_P2P_SINGLE_DEVICE=n
CONFIG_CFG80211_P2P_STATIC_CONCURRENT_DEVICE=n
CONFIG_CFG80211_P2P_MULTI_CHAN_SUPPORT=n
#For android wifi priv-lib (cfg80211_based wpa_supplicant cmd expansion)
CONFIG_CFG80211_ANDROID_PRIV_LIB_SUPPORT=n
#Support RFKILL hardware block/unblock LINUX-only function
CONFIG_RFKILL_HW_SUPPORT=n

CONFIG_VERIFICATION_MODE=y

CONFIG_WIFI_SYSDVT=n

#DBG_TXCMD
CONFIG_WIFI_DBG_TXCMD=y

#ifdef APCLI_SUPPORT
# Support AP-Client function
CONFIG_BEACON_MISS_ON_PRIMARY_CHANNEL=n
#endif // APCLI_SUPPORT //

### Common HW/SW definitions ###
#ifdef DOT11_N_SUPPORT
CONFIG_DOT11_N_SUPPORT=y
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
CONFIG_DOT11W_PMF_SUPPORT=y
#endif /* DOT11W_PMF_SUPPORT */

#ifdef WIFI_EAP_FEATURE
CONFIG_WIFI_EAP_FEATURE=y
#endif

#ifdef WIFI_GPIO_CTRL
CONFIG_WIFI_GPIO_CTRL=y
#endif

#ifdef CFG_RED_SUPPORT
CONFIG_RED_SUPPORT=n
#endif /* CFG_RED_SUPPORT */


CONFIG_FTM_SUPPORT=n
