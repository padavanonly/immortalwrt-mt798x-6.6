/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#ifndef __MTK_VENDOR_NL80211_H
#define __MTK_VENDOR_NL80211_H
/*
 * This header file defines the userspace API to the wireless stack. Please
 * be careful not to break things - i.e. don't move anything around or so
 * unless you can demonstrate that it breaks neither API nor ABI.
 *
 */

#include <linux/types.h>

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define MTK_NL80211_VENDOR_ID	0x0ce7

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

/**
 * enum mtk_nl80211_vendor_commands - supported mtk nl80211 vendor commands
 *
 * @MTK_NL80211_VENDOR_SUBCMD_UNSPEC: Reserved value 0
 * @MTK_NL80211_VENDOR_SUBCMD_TEST: Test for nl80211command/event
 * @MTK_NL80211_VENDOR_SUBCMD_AMNT_CTRL = 0x000000ae:
 * @MTK_NL80211_VENDOR_SUBCMD_CSI_CTRL = 0x000000c2:
 * @MTK_NL80211_VENDOR_SUBCMD_AP_RFEATURE: command to set radio information
 * @MTK_NL80211_VENDOR_SUBCMD_AP_WIRELESS: command to set wireless information
 * @MTK_NL80211_VENDOR_SUBCMD_SET_DOT11V_WNM:
 * @MTK_NL80211_VENDOR_SUBCMD_GET_CAP: command to get capability information
 *  from wifi driver, it requres mtk_nl80211_vendor_attr_get_cap attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_GET_RUNTIME_INFO: command to get run time information
 *  from wifi driver, it requres mtk_nl80211_vendor_attr_get_runtime_info attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_GET_STATISTIC: command to get statistic information
 *  from wifi driver, it requres mtk_nl80211_vendor_attr_get_static_info attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_GET_STATIC_INFO:
 * @MTK_NL80211_VENDOR_SUBCMD_MAC: command to set or get mac register
 *  information to/from wifi driver, require mtk_nl80211_vendor_attrs_mac
 *  attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_STATISTICS: command to get statistic information
 *  in string from wifi driver, this command is used to be compatible with
 *  old iwpriv stat command, it requres mtk_nl80211_vendor_attrs_statistics
 *  attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_VENDOR_SET: command to set old iwpriv set command
 *  string to wifi driver, it requires mtk_nl80211_vendor_attrs_vendor_set attributes,
 *  please note that this command is just used to be compatible with old iwpriv
 *  set command, and it will be discarded in some time.
 * @MTK_NL80211_VENDOR_SUBCMD_VENDOR_SHOW: command to set old iwpriv show command
 *  string to wifi driver, require mtk_nl80211_vendor_attrs_vendor_show attributes,
 *  please note that this command is just used to be compatible with old iwpriv
 *  show command, and it will be discarded in some time.
 * @MTK_NL80211_VENDOR_SUBCMD_WAPP_REQ: command to set or get wapp requred
 *  information from wifi driver, require mtk_nl80211_vendor_attr_wapp_req attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_AP_SECURITY: command to set ap security configurations
 *  to a specific bss in wifi driver, it requires mtk_nl80211_vendor_attrs_ap_security
 *  attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_AP_VOW:  command to set ap vow configurations
 *  it requires mtk_nl80211_vendor_attrs_ap_vow attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_MWDS: command to set ap/apcli MWDS configuration
 *  it requeris mtk_nl80211_vendor_attrs_mwds_set attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS:  command to set ap bss configurations
 *  it requires mtk_nl80211_vendor_attrs_ap_bss attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO: command to set ap phy or global configurations
 *  it requires mtk_nl80211_vendor_attrs_ap_radio attributes.
 * @MTK_NL80211_VENDOR_CMD_MAX: highest used command number
 * @__MTK_NL80211_VENDOR_CMD_AFTER_LAST: internal use
 * @MTK_NL80211_VENDOR_SUBCMD_SET_WMM:  command to set wmm configurations
 *  it requires mtk_nl80211_vendor_attrs_wmm attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_FT: command to set ft configurations
 *  it requires mtk_nl80211_vendor_attrs_ft attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_ACTION:  command to set action
 *  it requires mtk_nl80211_vendor_attrs_set_action attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_QOS:  command used by QoS
 *  it requires mtk_nl80211_vendor_attrs_qos attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_OFFCHANNEL_INFO: command to set offchannel_info
 *  it requires mtk_nl80211_vendor_attrs_offchannel_info attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_DFS:  command used by DFS
 *  it requires mtk_nl80211_vendor_attrs_dfs attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_EASYMESH:  command used by Easymesh
 *  it requires mtk_nl80211_vendor_attr_easymesh attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR:  command to set ap monitor
 *  it requires mtk_nl80211_vendor_attrs_ap_monitor attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_VLAN:  command to set vlan configurations
 *  it requires mtk_nl80211_vendor_attrs_vlan attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_BA:  command to set block ack configurations
 *  it requires mtk_nl80211_vendor_attrs_ap_ba attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_MLO_PRESET_LINK(0xe3): command to preset the mlo linkid
 *  it requires mtk_nl80211_vendor_attrs_mlo_preset_link attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_TXPOWER: command to set tx power configurations.
 *  it requires mtk_nl80211_vendor_attrs_tx_power attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_EDCA: command to set tx burst configurations.
 *  it requires mtk_nl80211_vendor_attrs_edca attribures
 * @MTK_NL80211_VENDOR_SUBCMD_SET_MULTICAST_SNOOPING: command to set AP multicast
 *  snooping related configurations, it requires mtk_nl80211_vendor_attrs_mcast_snoop attributes
 * @MTK_NL80211_VENDOR_SUBCMD_GET_RADIO_STATS: command to get per radio stats,
 *  it requires MTK_NL80211_VENDOR_ATTR_RADIO_STATS attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_RADIO_STATS: command to set per radio stats,
 *  it requires MTK_NL80211_VENDOR_ATTR_RADIO_STATS attributes
 * @MTK_NL80211_VENDOR_SUBCMD_GET_SSID_STATS: command to get  per BSS stats
 *  it requires MTK_NL80211_VENDOR_ATTR_SSID_STATS attributes
 * @MTK_NL80211_VENDOR_SUBCMD_GET_STA: command to get STA's information and stats
 *  it requires MTK_NL80211_VENDOR_ATTR_STA attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_MBO: command to set STA's Non preferred channel information
 *  it requires MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC attributes
 * @MTK_NL80211_VENDOR_SUBCMD_HQA: command to forward qatool hqa cmd to set/get
 *  it requires MTK_NL80211_VENDOR_ATTR_HQA attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_MLO_IE: command to set MLO IE configurations
 *  it requires mtk_nl80211_vendor_attrs_mlo_ie attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_PMKID: command to set PMKID configurations
 *  it requires mtk_nl80211_vendor_attrs_pmkid attributes.
 * @MTK_NL80211_VENDOR_SUBCMD_SET_V10CONVERTER: command to set v10converter
 *  it requires mtk_nl80211_vendor_attr_v10convertor attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_APPROXYREFRESH: command to set v10converter
 *  it requires mtk_nl80211_vendor_attr_apropxyrefresh attributes
 * @MTK_NL80211_VENDOR_SUBCMD_E2P: command to set/get e2p from driver buffer
 *  it requires mtk_nl80211_vendor_attr_apropxyrefresh attributes
 * @MTK_NL80211_VENDOR_SUBCMD_TESTENGINE: command to set/get AT cmd to fw
 *  it requires mtk_nl80211_vendor_attr_apropxyrefresh attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_COSR_IE: command to set cosr info to beacon
 *  it requires mtk_nl80211_vendor_attr_set_cosr_ie attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_COSR_INFO: command to set sta/ap cosr info to fw
 *  it requires mtk_nl80211_vendor_attr_set_cosr_info attributes
 * @MTK_NL80211_VENDOR_SUBCMD_GET_APCLI_MLD: command to get apcli mld information
 * @MTK_NL80211_VENDOR_SUBCMD_SHOW_MLO_INFO: the old ailias of MTK_NL80211_VENDOR_SUBCMD_GET_APCLI_MLD
 * @MTK_NL80211_VENDOR_SUBCMD_GET_AP_MLD: command to get ap mld information
 * @MTK_NL80211_VENDOR_SUBCMD_GET_BSS_MLO_INFO: the old alias of MTK_NL80211_VENDOR_SUBCMD_GET_AP_MLD
 * @MTK_NL80211_VENDOR_SUBCMD_OFFLOAD_ACS: command to offload acs
 *  it requires mtk_nl80211_vendor_attr_offload_acs attributes
 * @MTK_NL80211_VENDOR_SUBCMD_DEL_MLO_STA_ENTRY: command to clear old mlo sta entry
 * it requires mtk_nl80211_vendor_attrs_del_mlo_sta_entry attributes
 * @MTK_NL80211_VENDOR_SUBCMD_FTM: command to set fine time mesurement process parameters
 * it requires mtk_nl80211_vendor_attrs_ftm attributes
 * @MTK_NL80211_VENDOR_SUBCMD_FTM_STAT: command to query fine time mesurement process status
 * it requires mtk_nl80211_vendor_attrs_ftm_stat attributes
 * @MTK_NL80211_VENDOR_SUBCMD_CONFIG: command to config setting before interface up
 * @MTK_NL80211_VENDOR_SUBCMD_GET_CONNECTED_STA_MLD: command to get connected sta mld
 * it requires mtk_nl80211_vendor_attrs_sta_mld attributes
 * @MTK_NL80211_VENDOR_SUBCMD_GET_AP_BSS: command to get configurations of ap bss
 * @MTK_NL80211_VENDOR_SUBCMD_SET_SWACI_LNA: command to set software aci lna
 * it requires mtk_nl80211_vendor_attrs_swaci_lna attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_NPCA: command to config npca
 * it requires mtk_nl80211_vendor_attrs_npca attributes
 * @MTK_NL80211_VENDOR_SUBCMD_SET_DSO: command to config dso
 * it requires mtk_nl80211_vendor_attrs_dso attributes
 */
enum mtk_nl80211_vendor_commands {
	MTK_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
	MTK_NL80211_VENDOR_SUBCMD_TEST = 1,

/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_SUBCMD_AMNT_CTRL = 0x000000ae,
	MTK_NL80211_VENDOR_SUBCMD_CSI_CTRL = 0x000000c2,
	MTK_NL80211_VENDOR_SUBCMD_AP_RFEATURE,
	MTK_NL80211_VENDOR_SUBCMD_AP_WIRELESS,
	MTK_NL80211_VENDOR_SUBCMD_SET_DOT11V_WNM,
	MTK_NL80211_VENDOR_SUBCMD_GET_CAP,
	MTK_NL80211_VENDOR_SUBCMD_GET_RUNTIME_INFO,
	MTK_NL80211_VENDOR_SUBCMD_GET_STATISTIC,
	MTK_NL80211_VENDOR_SUBCMD_GET_STATIC_INFO,
	MTK_NL80211_VENDOR_SUBCMD_MAC,
	MTK_NL80211_VENDOR_SUBCMD_STATISTICS,
	MTK_NL80211_VENDOR_SUBCMD_VENDOR_SET,
	MTK_NL80211_VENDOR_SUBCMD_VENDOR_SHOW,
	MTK_NL80211_VENDOR_SUBCMD_WAPP_REQ,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_SECURITY,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_VOW,
	MTK_NL80211_VENDOR_SUBCMD_SET_MWDS,
	MTK_NL80211_VENDOR_SUBCMD_SET_COUNTRY,
	MTK_NL80211_VENDOR_SUBCMD_SET_CHANNEL,
	MTK_NL80211_VENDOR_SUBCMD_SET_AUTO_CH_SEL,
	MTK_NL80211_VENDOR_SUBCMD_SET_SCAN,
	MTK_NL80211_VENDOR_SUBCMD_SET_ACL,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO,
	MTK_NL80211_VENDOR_SUBCMD_SET_WMM,
	MTK_NL80211_VENDOR_SUBCMD_SET_FT,
	MTK_NL80211_VENDOR_SUBCMD_SET_ACTION,
	MTK_NL80211_VENDOR_SUBCMD_QOS,
	MTK_NL80211_VENDOR_SUBCMD_OFFCHANNEL_INFO,
	MTK_NL80211_VENDOR_SUBCMD_DFS,
	MTK_NL80211_VENDOR_SUBCMD_EASYMESH,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR,
	MTK_NL80211_VENDOR_SUBCMD_SET_VLAN,
	MTK_NL80211_VENDOR_SUBCMD_SET_BA,
	MTK_NL80211_VENDOR_SUBCMD_SET_MLO_PRESET_LINK,
	MTK_NL80211_VENDOR_SUBCMD_SET_TXPOWER,
	MTK_NL80211_VENDOR_SUBCMD_SET_EDCA,
	MTK_NL80211_VENDOR_SUBCMD_SET_MULTICAST_SNOOPING,
	MTK_NL80211_VENDOR_SUBCMD_GET_RADIO_STATS,
	MTK_NL80211_VENDOR_SUBCMD_SET_RADIO_STATS,
	MTK_NL80211_VENDOR_SUBCMD_GET_BSS_STATS,
	MTK_NL80211_VENDOR_SUBCMD_GET_STA,
	MTK_NL80211_VENDOR_SUBCMD_SET_MBO,
	MTK_NL80211_VENDOR_SUBCMD_HQA,
	MTK_NL80211_VENDOR_SUBCMD_SET_ATE,
	MTK_NL80211_VENDOR_SUBCMD_SET_MLO_IE,
	MTK_NL80211_VENDOR_SUBCMD_SET_PMKID,
	MTK_NL80211_VENDOR_SUBCMD_SET_V10CONVERTER,
	MTK_NL80211_VENDOR_SUBCMD_SET_APPROXYREFRESH,
	MTK_NL80211_VENDOR_SUBCMD_E2P,
	MTK_NL80211_VENDOR_SUBCMD_TESTENGINE,
	MTK_NL80211_VENDOR_SUBCMD_SET_COSR_IE,
	MTK_NL80211_VENDOR_SUBCMD_SET_COSR_INFO,
	MTK_NL80211_VENDOR_SUBCMD_GET_APCLI_MLD,
	MTK_NL80211_VENDOR_SUBCMD_SHOW_MLO_INFO = MTK_NL80211_VENDOR_SUBCMD_GET_APCLI_MLD,
	MTK_NL80211_VENDOR_SUBCMD_SET_MLO_SWITCH,
	MTK_NL80211_VENDOR_SUBCMD_GET_AP_MLD,
	MTK_NL80211_VENDOR_SUBCMD_GET_BSS_MLO_INFO = MTK_NL80211_VENDOR_SUBCMD_GET_AP_MLD,
	MTK_NL80211_VENDOR_SUBCMD_SET_MAC,
	MTK_NL80211_VENDOR_SUBCMD_SEQ_NUM,
	MTK_NL80211_VENDOR_SUBCMD_OFFLOAD_ACS,
	MTK_NL80211_VENDOR_SUBCMD_BH_STATUS,
	MTK_NL80211_VENDOR_SUBCMD_GET_BAND_INFO,
	MTK_NL80211_VENDOR_SUBCMD_DEL_MLO_STA_ENTRY,
	MTK_NL80211_VENDOR_SUBCMD_FTM,
	MTK_NL80211_VENDOR_SUBCMD_FTM_STAT,
	MTK_NL80211_VENDOR_SUBCMD_CONFIG,
	MTK_NL80211_VENDOR_SUBCMD_GET_CONNECTED_STA_MLD,
	MTK_NL80211_VENDOR_SUBCMD_GET_AP_BSS,
	MTK_NL80211_VENDOR_SUBCMD_CLEAN_STATION,
	MTK_NL80211_VENDOR_SUBCMD_SET_SWACI_LNA,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MLD,
	MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
	MTK_NL80211_VENDOR_SUBCMD_SET_DSO,
	/* add new commands above here */
	/* used to define NL80211_CMD_MAX below */
	__MTK_NL80211_VENDOR_CMD_AFTER_LAST,
	MTK_NL80211_VENDOR_CMD_MAX = __MTK_NL80211_VENDOR_CMD_AFTER_LAST - 1
};

struct GNU_PACKED mtk_andlink_radio_info {
	signed char snr;
	signed char rssi;
	signed char noise;
};
/**
 * enum mtk_nl80211_vendor_attrs_ssid_stats - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_GET_RADIO_STATS.
 * Information in these attributes is used to get per BSS stats
 * and counters.
 *
 * @MTK_NL80211_VENDOR_ATTR_RADIO_STATS: per radio stats, struct wifi_radio_stats.
 * @MTK_NL80211_VENDOR_ATTR_GET_RADIO_CPU_TEMPRETURE: to get radio tempreture
 * @MTK_NL80211_VENDOR_ATTR_GET_RADIO_BSS_INFO_ANDLINK_FORMAT: per radio info in andlink format, struct mtk_andlink_radio_info
 * @MTK_NL80211_VENDOR_ATTR_GET_RADIO_RSSI: to get radio rssi
 */
enum mtk_nl80211_vendor_attrs_radio_stats {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_RADIO_STATS_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_RADIO_STATS,
	MTK_NL80211_VENDOR_ATTR_GET_RADIO_CPU_TEMPRETURE,
	MTK_NL80211_VENDOR_ATTR_GET_RADIO_RSSI,
	/* add attributes here, update the policy in nl80211.c */
	MTK_NL80211_VENDOR_ATTR_GET_RADIO_BSS_INFO_ANDLINK_FORMAT,

	__MTK_NL80211_VENDOR_ATTR_RADIO_STATS_LAST,
	MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_RADIO_STATS_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_set_radio_stats - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_RADIO_STATS.
 * Information in these attributes is used to get per BSS stats
 * and counters.
 *
 * @MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_MEASURING_METHOD: set per radio measure method
 * @MTK_NL80211_VENDOR_ATTR_RADIO_SET_MEASURE_ENABEL: enable/disable radio measure
 */
enum mtk_nl80211_vendor_attrs_set_radio_stats {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_MEASURING_METHOD,
	MTK_NL80211_VENDOR_ATTR_RADIO_SET_MEASURE_ENABEL,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_LAST,
	MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_ssid_stats - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_GET_SSID_STATS.
 * Information in these attributes is used to get per BSS stats
 * and counters.
 *
 * @MTK_NL80211_VENDOR_ATTR_SSID_STATS: per BSS stats, struct wifi_bss_stats.
 */
enum mtk_nl80211_vendor_attrs_bss_stats {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_BSS_STATS_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_BSS_STATS,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_BSS_STATS_LAST,
	MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_BSS_STATS_LAST - 1
};

struct mlo_link_information {
	unsigned char valid;
	unsigned char link_address[6];
	unsigned long long rx_bytes;
	unsigned long long tx_bytes;
	unsigned long rx_packets;
	unsigned long tx_packets;
	signed char rssi[4];
	unsigned char snr[4];
	unsigned long tx_rate;
	unsigned long rx_rate;
};

struct station_information {
	unsigned char mac[6];
	unsigned char tx_phymode;
	unsigned char rx_phymode;
	unsigned char tx_bw;
	unsigned char rx_bw;
	unsigned char tx_nss;
	unsigned char tx_mcs;
	unsigned char rx_nss;
	unsigned char rx_mcs;
	unsigned long long rx_bytes;
	unsigned long long tx_bytes;
	unsigned long rx_packets;
	unsigned long tx_packets;
	signed char rssi[4];
	unsigned char snr[4];
	unsigned long tx_rate;
	unsigned long rx_rate;
	unsigned char cap_phymode;
	unsigned char mlo_enable;
	unsigned char mld_mac[6];
	signed char SignalStrength;
	unsigned long ErrorsSent;
	unsigned long ErrorsReceived;
	unsigned long RetryCount;
	struct mlo_link_information mlo_link[3];
};

#define ANDLINK_MAX_ASSOC_NUM 20
#define SSID_LEN 33
#define MAC_ADDR_LEN 6
#define ANDLINK_MAX_WLAN_NEIGHBOR 24
#define ANDLINK_SEC_LEN 32
#define LEN_PSK 64

enum {
	ANDLINK_IF5,
	ANDLINK_IF6,
	ANDLINK_IF8,
	ANDLINK_IF_MAX
};
struct GNU_PACKED mtk_andlink_sta_entry {
	unsigned char  mac_addr[MAC_ADDR_LEN];
	unsigned char  vmac_addr[MAC_ADDR_LEN];
	unsigned char mlo_en;
	unsigned char mlo_idx;
	unsigned char setup_link;
	char	rssi;
	char	bw;
	unsigned char	mode;
	unsigned long long uptime;
	unsigned long tx_rate;/* Mbps */
	unsigned long rx_rate;/* Mbps */
	unsigned long rx_rate_rt;
	unsigned long tx_rate_rt;
	unsigned long avg_tx_rate[ANDLINK_IF_MAX];
	unsigned long avg_rx_rate[ANDLINK_IF_MAX];
	unsigned long max_tx_rate[ANDLINK_IF_MAX];
	unsigned long max_rx_rate[ANDLINK_IF_MAX];
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
	unsigned long long tx_pkts;
	unsigned long long rx_pkts;
};

struct GNU_PACKED mtk_andlink_wifi_sta_info {
	unsigned int sta_cnt;
	unsigned int offset;
	struct mtk_andlink_sta_entry sta_entry[ANDLINK_MAX_ASSOC_NUM];
};
/**
 * enum mtk_nl80211_vendor_attrs_sta - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_GET_STA.
 * Information in these attributes is used to get STA information
 * and counters.
 *
 * @MTK_NL80211_VENDOR_ATTR_STA: STA's information and counters, struct wifi_station.
 * @MTK_NL80211_VENDOR_ATTR_STA_MAC: the mac address of station to be queried.
 * @MTK_NL80211_VENDOR_ATTR_STA_INFO: the queried station information structure, struct station_information.
 * @MTK_NL80211_VENDOR_ATTR_STA_INFO_GTBAPP_FORMAT: the queried station information structure for gtb application, struct wifi_sta_info.
 * @MTK_NL80211_VENDOR_ATTR_STA_INFO_ANDLINK_FORMAT:query all sta info, struct GNU_PACKED mtk_andlink_wifi_sta_info
 */
enum mtk_nl80211_vendor_attrs_sta {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_STA_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_STA,
	/* add attributes here, update the policy in nl80211.c */
	MTK_NL80211_VENDOR_ATTR_STA_MAC,
	MTK_NL80211_VENDOR_ATTR_STA_INFO,
	MTK_NL80211_VENDOR_ATTR_STA_INFO_GTBAPP_FORMAT,
	MTK_NL80211_VENDOR_ATTR_STA_INFO_ANDLINK_FORMAT,

	__MTK_NL80211_VENDOR_ATTR_STA_LAST,
	MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_STA_LAST - 1
};


/**
 * enum mtk_nl80211_vendor_attrs_ftm - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_FTM
 * Information in these attributes is used to set
 * FTM (Fine Time Measuremnt) parameters
 *
 * @MTK_NL80211_VENDOR_ATTR_FTM_RANG_REQ: for iSTA to request FTM range request.
 * @MTK_NL80211_VENDOR_ATTR_FTM_ENABLE: enable iSTA/rSTA, set bit=1 in extended capabilities field.
 * @MTK_NL80211_VENDOR_ATTR_FTM_BURST_EXP: number of burst exponent.
 * @MTK_NL80211_VENDOR_ATTR_FTM_BURST_DUR: indicates the duration of burst duration.
 * @MTK_NL80211_VENDOR_ATTR_FTM_MIN_DELTA: minimum time between two FTM frames.
 * @MTK_NL80211_VENDOR_ATTR_FTM_PARTIAL_TSF: indicate the preferred start of first burst instance.
 * @MTK_NL80211_VENDOR_ATTR_FTM_PTSF_NO_PREFERENCE: indicate the perference of PTFS of iSTA
 * @MTK_NL80211_VENDOR_ATTR_FTM_ASAP: execute FTM process as soon as possible.
 * @MTK_NL80211_VENDOR_ATTR_FTM_NUM: iSTA request number of FTM frames per burst instance.
 * @MTK_NL80211_VENDOR_ATTR_FTM_FMT_AND_BW: requested or allocated PPDU format and bandwidth.
 * @MTK_NL80211_VENDOR_ATTR_FTM_BURST_PERIOD: the interval between two consecutive burst instance.
 * @MTK_NL80211_VENDOR_ATTR_FTM_TARGET_MAC: MAC address of target rSTA.
 * @MTK_NL80211_VENDOR_ATTR_FTM_DEBUG: FTM debug cmd for FW.
 * @MTK_NL80211_VENDOR_ATTR_FTM_ROLE: select iSTA or rSTA role to set parameters.
 * @MTK_NL80211_VENDOR_ATTR_FTM_TOAE_CFG: TOAE Calibration for FTM.
 * @MTK_NL80211_VENDOR_ATTR_FTM_TESTMODE: cmd for FTM test mode.
 * @MTK_NL80211_VENDOR_ATTR_FTM_FTMR_TIMEOUT: set rSTA 2nd FTMR timeout value.
 * @MTK_NL80211_VENDOR_ATTR_FTM_NON_ASAP_DELAY_TIME: non-ASAP iFTMR delay time (for Partial TSF).
 * @MTK_NL80211_VENDOR_ATTR_FTM_DUMP_PARAMS: dump all FTM setting.
 * @MTK_NL80211_VENDOR_ATTR_FTM_NTB_RANGING_PARAMS: Params in Ranging Parameters element for iFTMR.
 * @MTK_NL80211_VENDOR_ATTR_FTM_NTB_MEAS_EXP: Min/Max time between measurement in NTB Specific subelement for iFTMR.
 * @MTK_NL80211_VENDOR_ATTR_FTM_NTB_RANG_REQ: for iSTA to request 11az NTB range request.
 */
enum mtk_nl80211_vendor_attrs_ftm {
	MTK_NL80211_VENDOR_ATTR_FTM_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_FTM_RANG_REQ,
	MTK_NL80211_VENDOR_ATTR_FTM_ENABLE,
	MTK_NL80211_VENDOR_ATTR_FTM_BURST_EXP,
	MTK_NL80211_VENDOR_ATTR_FTM_BURST_DUR,
	MTK_NL80211_VENDOR_ATTR_FTM_MIN_DELTA,
	MTK_NL80211_VENDOR_ATTR_FTM_PARTIAL_TSF,
	MTK_NL80211_VENDOR_ATTR_FTM_PTSF_NO_PREFERENCE,
	MTK_NL80211_VENDOR_ATTR_FTM_ASAP,
	MTK_NL80211_VENDOR_ATTR_FTM_NUM,
	MTK_NL80211_VENDOR_ATTR_FTM_FMT_AND_BW,
	MTK_NL80211_VENDOR_ATTR_FTM_BURST_PERIOD,
	MTK_NL80211_VENDOR_ATTR_FTM_TARGET_MAC,
	MTK_NL80211_VENDOR_ATTR_FTM_DEBUG,
	MTK_NL80211_VENDOR_ATTR_FTM_ROLE,
	MTK_NL80211_VENDOR_ATTR_FTM_TOAE_CFG,
	MTK_NL80211_VENDOR_ATTR_FTM_TESTMODE,
	MTK_NL80211_VENDOR_ATTR_FTM_FTMR_TIMEOUT,
	MTK_NL80211_VENDOR_ATTR_FTM_NON_ASAP_DELAY_TIME,
	MTK_NL80211_VENDOR_ATTR_FTM_DUMP_PARAMS,
	MTK_NL80211_VENDOR_ATTR_FTM_NTB_RANGING_PARAMS,
	MTK_NL80211_VENDOR_ATTR_FTM_NTB_MEAS_EXP,
	MTK_NL80211_VENDOR_ATTR_FTM_NTB_RANG_REQ,

	__MTK_NL80211_VENDOR_ATTR_FTM_LAST,
	MTK_NL80211_VENDOR_ATTR_FTM_MAX = __MTK_NL80211_VENDOR_ATTR_FTM_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_ftm_stat - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_FTM_STAT.
 * Information in these attributes is used to get ftm information
 * to/from driver from/to user application.
 * @MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR: show the tSTA/iSTA information of FTM process.
 */
enum mtk_nl80211_vendor_attrs_ftm_stat {
	MTK_NL80211_VENDOR_ATTR_FTM_STAT_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR,

	__MTK_NL80211_VENDOR_ATTR_FTM_STAT_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_FTM_STAT_MAX = __MTK_NL80211_VENDOR_ATTR_FTM_STAT_AFTER_LAST - 1
};
struct GNU_PACKED andlink_wifi_ch_info {
	char sec_mode[ANDLINK_SEC_LEN];/*security mode*/
	char pwd[LEN_PSK];
	char ssid[SSID_LEN];
	unsigned short max_sta_num;
	unsigned char is_hidden;
	unsigned char ifname[16];
};

struct GNU_PACKED _andlink_event_data {
	struct andlink_wifi_ch_info wifi_ch_info;
};

struct GNU_PACKED mtk_andlink_event {
	unsigned char len;
	unsigned char event_id;
	unsigned int ifindex;
	struct GNU_PACKED _andlink_event_data data;
};

/**
 * enum mtk_nl80211_vendor_events - MediaTek nl80211 asynchoronous event identifiers
 *
 * @MTK_NL80211_VENDOR_EVENT_UNSPEC: Reserved value 0
 *
 * @MTK_NL80211_VENDOR_EVENT_TEST: Test for nl80211command/event
 *  it requires mtk_nl80211_vendor_attr_test attributes.
 * @MTK_NL80211_VENDOR_EVENT_RSP_WAPP_EVENT: command used by wapp event
 *  it requires mtk_nl80211_vendor_attr_event_rsp_wapp_event attributes.
 * @MTK_NL80211_VENDOR_EVENT_OFFCHANNEL_INFO: command by Easymesh
 *  it requires mtk_nl80211_vendor_attr_event_offchannel_info attributes.
 * @MTK_NL80211_VENDOR_EVENT_SEND_ML_INFO: command used by wifi7 multilink operation
 *  it requires mtk_nl80211_vendor_attr_event_send_ml_info attributes.
 * @MTK_NL80211_VENDOR_EVENT_MLO_EVENT: command by uplayer app such as supplicant
 *  it requires NL80211_ATTR_VENDOR_DATA attributes.
 * @MTK_NL80211_VENDOR_EVENT_STA_PROFILE_EVENT: event used by hostapd to get per-STA
 *  profile for MLO FT
 * @MTK_NL80211_VENDOR_EVENT_DISC_STA: event used by customer to get per-STA statisc
 * info when STA disconnect, it require mtk_nl80211_vendor_attr_event_disconnect_sta attribute.
 * @MTK_NL80211_VENDOR_EVENT_COSR: event used for wapp, which include recive cosr action
 * frame, sta rssi change and cosr ap found event,
 * it require mtk_nl80211_vendor_attr_event_cosr.
 * @MTK_NL80211_VENDOR_EVENT_RSP_AFC_EVENT: command used by afc event
 *  it requires mtk_nl80211_vendor_attr_event_rsp_afc_event attributes.
 * @MTK_NL80211_VENDOR_EVENT_ACS_COMPLETE_EVENT: command by uplayer app such as hostapd
 *  it requires mtk_nl80211_vendor_attr_event_acs_complete_event attributes.
 * @MTK_NL80211_VENDOR_EVENT_RX_T2LM_STOP_DISASSOC_TIMER: event to notify hostapd
 *  to stop disassoc timer when t2lm req received.
 * @MTK_NL80211_VENDOR_EVENT_ACS_PER_CH_INFO: event used by uplayer app such as User's ACS daemon
 *  it requires various types of busytime per channel while doing ACS attributes.
 * @MTK_NL80211_VENDOR_EVENT_SEND_MLO_STA_LINK_MAC: event used by uplayer app such as hostapd
 * to update link mac in case of MLO-FT over the DS.
 * @MTK_NL80211_VENDOR_EVENT_ANDLINK: event used by uplayer app such as rmt_mgmtd
 * to update struct GNU_PACKED mtk_andlink_event when ssid and password changed.
 */
enum mtk_nl80211_vendor_events {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_EVENT_UNSPEC = 0,
	MTK_NL80211_VENDOR_EVENT_TEST,

	MTK_NL80211_VENDOR_EVENT_RSP_WAPP_EVENT,
	MTK_NL80211_VENDOR_EVENT_OFFCHANNEL_INFO,
	MTK_NL80211_VENDOR_EVENT_SEND_ML_INFO,
	MTK_NL80211_VENDOR_EVENT_MLO_EVENT,
	MTK_NL80211_VENDOR_EVENT_STA_PROFILE_EVENT,
	MTK_NL80211_VENDOR_EVENT_DISC_STA,
	MTK_NL80211_VENDOR_EVENT_COSR,
	MTK_NL80211_VENDOR_EVENT_BH_STATUS,
	MTK_NL80211_VENDOR_EVENT_ACS_COMPLETE_EVENT,
	MTK_NL80211_VENDOR_EVENT_RX_T2LM_STOP_DISASSOC_TIMER,
	MTK_NL80211_VENDOR_EVENT_ACS_PER_CH_INFO,
	MTK_NL80211_VENDOR_EVENT_SEND_MLO_STA_LINK_MAC,
	MTK_NL80211_VENDOR_EVENT_MLO_RECONF,
	MTK_NL80211_VENDOR_EVENT_ANDLINK,
	/*ref: nl80211_radar_event start*/
	MTK_NL80211_VENDOR_EVENT_RADAR_DETECTED,
	MTK_NL80211_VENDOR_EVENT_RADAR_CAC_FINISHED,
	MTK_NL80211_VENDOR_EVENT_RADAR_CAC_ABORTED,
	MTK_NL80211_VENDOR_EVENT_RADAR_NOP_FINISHED,
	MTK_NL80211_VENDOR_EVENT_RADAR_PRE_CAC_EXPIRED,
	MTK_NL80211_VENDOR_EVENT_RADAR_CAC_STARTED,
	/*ref: nl80211_radar_event end*/

	/* add new events above here */
	/* used to define NL80211_EVENT_MAX below */
	__MTK_NL80211_VENDOR_EVENT_AFTER_LAST,
	MTK_NL80211_VENDOR_EVENT_MAX = __MTK_NL80211_VENDOR_EVENT_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_test - Specifies the values for vendor test
 * command MTK_NL80211_VENDOR_ATTR_TEST
 * @MTK_NL80211_VENDOR_ATTR_TEST：enable nl80211 test
 */
enum mtk_nl80211_vendor_attr_test {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_TEST_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_TEST,

	__MTK_NL80211_VENDOR_ATTR_TEST_LAST,
	MTK_NL80211_VENDOR_ATTR_TEST_MAX =
	__MTK_NL80211_VENDOR_ATTR_TEST_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_event_test - Specifies the values for vendor test
 * event MTK_NL80211_VENDOR_ATTR_TEST
 * @MTK_NL80211_VENDOR_ATTR_TEST：receive nl80211 test event
 */
enum mtk_nl80211_vendor_attr_event_test {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_TEST_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_EVENT_TEST,

	__MTK_NL80211_VENDOR_ATTR_EVENT_TEST_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_TEST_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_TEST_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_wnm - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_DOT11V_WNM.
 * Information in these attributes is used to set/get wnm information
 * to/from driver from/to user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_WNM_CMD:
 * @MTK_NL80211_VENDOR_ATTR_WNM_BTM_REQ: BTM request frame
 * @MTK_NL80211_VENDOR_ATTR_WNM_BTM_RSP:
 */
enum mtk_nl80211_vendor_attrs_dot11v_wnm {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_DOT11V_WNM_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_DOT11V_WNM_CMD,
	MTK_NL80211_VENDOR_ATTR_DOT11V_WNM_BTM_REQ,
	MTK_NL80211_VENDOR_ATTR_DOT11V_WNM_BTM_RSP,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_DOT11V_WNM_AFTER_LAST,
	MTK_NL80211_VENDOR_DOT11V_WNM_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_DOT11V_WNM_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_vendor_set - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_VENDOR_SET.
 * Information in these attributes is used to set information
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR: command string
 */
enum mtk_nl80211_vendor_attrs_vendor_set {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_VENDOR_SET_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_VENDOR_SET_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_VENDOR_SET_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_VENDOR_SET_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_vendor_show - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_VENDOR_SHOW.
 * Information in these attributes is used to get information
 * from driver to user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR: command string
 * @MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_RSP_STR: show rsp string buffer
 */
enum mtk_nl80211_vendor_attrs_vendor_show {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR,
	MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_RSP_STR,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_statistics - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_STATISTICS.
 * Information in these attributes is used to get wnm information
 * to/from driver from/to user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_STATISTICS_STR: statistic information string
 */
enum mtk_nl80211_vendor_attrs_statistics {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_STATISTICS_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_STATISTICS_STR,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_STATISTICS_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_STATISTICS_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_STATISTICS_AFTER_LAST - 1
};

/**
 * structure mac_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_MAC_WRITE_PARAM and MTK_NL80211_VENDOR_ATTR_MAC_SHOW_PARAM.
 * Information in this structure is used to get/set mac register information
 * from/to driver.
 *
 * @start: start mac address
 * @end: end mac address
 * @value: value for the mac register
 */
struct GNU_PACKED mac_param {
	unsigned int start;
	unsigned int end;
	unsigned int value;
};

/**
 * enum mtk_nl80211_vendor_attrs_mac - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_MAC.
 * Information in these attributes is used to get/set mac information
 * from/to driver.
 *
 * @MTK_NL80211_VENDOR_ATTR_MAC_WRITE_PARAM: params, refer to struct GNU_PACKED mac_param
 * @MTK_NL80211_VENDOR_ATTR_MAC_SHOW_PARAM: params, refer to struct GNU_PACKED mac_param
 * @MTK_NL80211_VENDOR_ATTR_MAC_RSP_STR: RSP string
 */
enum mtk_nl80211_vendor_attrs_mac {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_MAC_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_MAC_WRITE_PARAM,
	MTK_NL80211_VENDOR_ATTR_MAC_SHOW_PARAM,
	MTK_NL80211_VENDOR_ATTR_MAC_RSP_STR,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_MAC_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_MAC_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_MAC_AFTER_LAST - 1
};

/**
 * enum mtk_vendor_attr_authmode - This enum defines the value of
 * MTK_NL80211_VENDOR_ATTR_AP_SECURITY_AUTHMODE.
 * Information in these attributes is used set auth mode of a specific bss.
 */
enum mtk_vendor_attr_authmode {
	NL80211_AUTH_OPEN,
	NL80211_AUTH_SHARED,
	NL80211_AUTH_WEPAUTO,
	NL80211_AUTH_WPA,
	NL80211_AUTH_WPAPSK,
	NL80211_AUTH_WPANONE,
	NL80211_AUTH_WPA2,
	NL80211_AUTH_WPA2MIX,
	NL80211_AUTH_WPA2PSK,
	NL80211_AUTH_WPA3,
	NL80211_AUTH_WPA3_192,
	NL80211_AUTH_WPA3PSK,
	NL80211_AUTH_WPA2PSKWPA3PSK,
	NL80211_AUTH_WPA2PSKMIXWPA3PSK,
	NL80211_AUTH_WPA1WPA2,
	NL80211_AUTH_WPAPSKWPA2PSK,
	NL80211_AUTH_WPA_AES_WPA2_TKIPAES,
	NL80211_AUTH_WPA_AES_WPA2_TKIP,
	NL80211_AUTH_WPA_TKIP_WPA2_AES,
	NL80211_AUTH_WPA_TKIP_WPA2_TKIPAES,
	NL80211_AUTH_WPA_TKIPAES_WPA2_AES,
	NL80211_AUTH_WPA_TKIPAES_WPA2_TKIPAES,
	NL80211_AUTH_WPA_TKIPAES_WPA2_TKIP,
	NL80211_AUTH_OWE,
	NL80211_AUTH_FILS_SHA256,
	NL80211_AUTH_FILS_SHA384,
	NL80211_AUTH_WAICERT,
	NL80211_AUTH_WAIPSK,
	NL80211_AUTH_DPP,
	NL80211_AUTH_DPPWPA2PSK,
	NL80211_AUTH_DPPWPA3PSK,
	NL80211_AUTH_DPPWPA3PSKWPA2PSK,
	NL80211_AUTH_WPA2_ENT_OSEN
};

/**
 * enum mtk_vendor_attr_encryptype - This enum defines the value of
 * MTK_NL80211_VENDOR_ATTR_AP_SECURITY_ENCRYPTYPE.
 * Information in these attributes is used set encryption type of a specific bss.
 */
enum mtk_vendor_attr_encryptype {
	NL80211_ENCRYPTYPE_NONE,
	NL80211_ENCRYPTYPE_WEP,
	NL80211_ENCRYPTYPE_TKIP,
	NL80211_ENCRYPTYPE_AES,
	NL80211_ENCRYPTYPE_CCMP128,
	NL80211_ENCRYPTYPE_CCMP256,
	NL80211_ENCRYPTYPE_GCMP128,
	NL80211_ENCRYPTYPE_GCMP256,
	NL80211_ENCRYPTYPE_TKIPAES,
	NL80211_ENCRYPTYPE_TKIPCCMP128,
	NL80211_ENCRYPTYPE_WPA_AES_WPA2_TKIPAES,
	NL80211_ENCRYPTYPE_WPA_AES_WPA2_TKIP,
	NL80211_ENCRYPTYPE_WPA_TKIP_WPA2_AES,
	NL80211_ENCRYPTYPE_WPA_TKIP_WPA2_TKIPAES,
	NL80211_ENCRYPTYPE_WPA_TKIPAES_WPA2_AES,
	NL80211_ENCRYPTYPE_WPA_TKIPAES_WPA2_TKIPAES,
	NL80211_ENCRYPTYPE_WPA_TKIPAES_WPA2_TKIP,
	NL80211_ENCRYPTYPE_SMS4
};

#ifndef MAX_WEP_KEY_LEN
#define MAX_WEP_KEY_LEN 16
#endif

/**
 * structure wep_key_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_AP_SECURITY_WEPKEY. Information in this structure
 * is used to set wep key information to a specific bss in wifi driver.
 *
 * @key_idx: key index
 * @key_len: key length
 * @key: key value
 */
struct GNU_PACKED wep_key_param {
	unsigned char key_idx;
	unsigned int key_len;
	unsigned char key[MAX_WEP_KEY_LEN];
};

/**
 * structure vow_group_en_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO &
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO.
 * Information in this structure is used to set vow airtime control enable configuration
 * to a specific group in wifi driver.
 *
 * @group: vow group
 * @en: Enable/Disable
 */
struct GNU_PACKED vow_group_en_param {
	unsigned int group;
	unsigned int en;
};

/**
 * structure vow_group_en_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO &
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO.
 * Information in this structure is used to set vow airtime rate configuration
 * to a specific group in wifi driver.
 *
 * @group: vow group
 * @rate: vow rate
 */
struct GNU_PACKED vow_rate_param {
	unsigned int group;
	unsigned int rate;
};

/**
 * structure vow_group_en_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO &
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO.
 * Information in this structure is used to set vow airtime ratio configuration
 * to a specific group in wifi driver.
 *
 * @group: vow group
 * @ratio: vow ratio
 */
struct GNU_PACKED vow_ratio_param {
	unsigned int group;
	unsigned int ratio;
};

/**
 * structure vow_info - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO
 * Information in this structure is used to get vow configuration
 * to a specific group in wifi driver.
 *
 * @atf_en: ATF Enable/Disable
 * @bw_en: bw ctrl Enable/Disable
 * @group: vow group
 * @ratio: vow ratio
 */

struct GNU_PACKED vow_info {
	unsigned char atf_en;
	unsigned char bw_en;
	unsigned int group;
	unsigned int ratio;
};

/**
 * structure vlan_policy_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO.
 * Information in this structure is used to set vlan policy configuration
 * to a specific group in wifi driver.
 *
 * @direction: vlan direction
 * @policy: vlan policy
 */
struct GNU_PACKED vlan_policy_param {
	unsigned int direction;
	unsigned int policy;
};

/**
 * structure ap_11r_params - This structure defines the payload format of
 * MTK_NL80211_VENDOR_SUBCMD_SET_FT.
 * Information in this structure is used to set ft configuration
 * to wifi driver.
 *
 * @nas_identifier: r0kh-id value
 * @nas_id_len: r0kh-id length
 * @r1_key_holder: r1kh-id value
 * @own_mac: interface's mac addr
 * @reassociation_deadline: reassociation deadline value
 */
struct GNU_PACKED ap_11r_params {
	unsigned char nas_identifier[64];
	int nas_id_len;
	unsigned char r1_key_holder[ETH_ALEN];
	unsigned char own_mac[ETH_ALEN];
	unsigned int reassociation_deadline;
};

/**
 * structure mlo_per_sta_profile - This structure defines the mlo ie format of
 * MTK_NL80211_VENDOR_SUBCMD_SET_MLO_IE.
 * Information in this structure is used to set MLO IE configuration
 * to wifi driver.
 *
 * @link_id: link id
 * @addr: per sta link mac addr
 * @dtim: dtim
 * @beacon_interval: beacon interval
 * @nstr_bmap: nstr bitmap
 * @complete_profile: complete profile bit
 * @mac_addr_present: mac addr present bit
 * @bcn_intvl_present: bcn intvl present bit
 * @dtim_present: dtim present bit
 * @nstr_present: nstr present bit
 */
struct GNU_PACKED mlo_per_sta_profile {
	unsigned char link_id;
	unsigned char addr[6];
	unsigned short dtim;
	unsigned short beacon_interval;
	unsigned short nstr_bmap;
	unsigned int complete_profile:1;
	unsigned int mac_addr_present:1;
	unsigned int bcn_intvl_present:1;
	unsigned int dtim_present:1;
	unsigned int nstr_present:1;
};

/**
 * structure wpa_mlo_ie_parse - This structure defines the mlo ie format of
 * MTK_NL80211_VENDOR_SUBCMD_SET_MLO_IE.
 * Information in this structure is used to set MLO IE configuration
 * to wifi driver.
 *
 * @type: type
 * @ml_addr: peer mld mac addr
 * @common_info_len: mlo ie common info len
 * @link_id: link id
 * @bss_para_change_count: bss para change count
 * @medium_sync_delay: medium sync delay
 * @eml_cap: eml capability
 * @mld_cap: peer mld capability
 * @ml_link_num: all link num
 * @valid: valid bit
 * @link_id_present: link id present bit
 * @bss_para_change_cnt_present: bss para change count bit
 * @medium_sync_delay_present: medium sync delay present bit
 * @eml_cap_present: eml cap present bit
 * @mld_cap_present: mld cap present bit
 * @profiles: per sta profile
 */
struct GNU_PACKED wpa_mlo_ie_parse {
	unsigned char type;
	unsigned char ml_addr[6];
	unsigned char common_info_len;
	unsigned char link_id;
	unsigned char bss_para_change_count;
	unsigned short medium_sync_delay;
	unsigned short eml_cap;
	unsigned short mld_cap;
	unsigned char ml_link_num;
	unsigned int valid:1;
	unsigned int link_id_present:1;
	unsigned int bss_para_change_cnt_present:1;
	unsigned int medium_sync_delay_present:1;
	unsigned int eml_cap_present:1;
	unsigned int mld_cap_present:1;

	struct GNU_PACKED mlo_per_sta_profile profiles[3];
};

struct GNU_PACKED bss_mlo_info {
	unsigned char link_cnt;
	unsigned char mld_grp_idx;
	unsigned char link_id;
	unsigned char addr[ETH_ALEN];
};

/**
 * enum mtk_nl80211_vendor_attrs_bss_mlo_info - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_GET_AP_MLD.
 * Information in these attributes is used to get per BSS mlo info.
 *
 * @MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO: per BSS mlo info, struct bss_mlo_info.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX: u8, mld index.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS: u8[6], mld mac address
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_DUMP: flag, used to dump all ap mlds.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_APS: a nested array of affiliated APs, each
 *  contains per-ap information.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_BSSID: u8[6], affiliated ap bssid.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_LINKID: u8, affiliated ap linkid.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_DISABLED_SUBCHAN: u16, affiliated ap disabled subchan.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLMR: flag, emlmr is enabled.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLSR: falg, emlsr is enabled.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_STR: flag, str is enabled.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_NSTR: flag, nstr is enabled.
 */
enum mtk_nl80211_vendor_attrs_ap_mld {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_AP_MLD_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_DUMP,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_APS,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_BSSID,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_LINKID,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_DISABLED_SUBCHAN,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLMR,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLSR,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_STR,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_NSTR,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_CONNECTED_STA_MLDS_DUMP,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_TYPE,

	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_AP_MLD_LAST,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_MLD_LAST - 1,
	MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO_ATTR_MAX = MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX
};

/**
 * enum mtk_nl80211_vendor_attrs_set_ap_mld - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AP_MLD.
 * Information in these attributes is used to set AP or MLD.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_INDEX: used to set MLD AP's mld index
 * @MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_ADDRESS: used to set MLD group addr
 * @MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_EML_MODE: used to set MLD AP's EML mode
 */
enum mtk_nl80211_vendor_attrs_set_ap_mld {
	MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_INDEX,
	MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_ADDRESS,
	MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_EML_MODE,

	__MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_LAST,
	MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_LAST - 1,
};
/**
 * structure ba_mactid_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO &
 * MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO &
 * MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO
 * Information in this structure is used to set ft configuration
 * to wifi driver.
 *
 * @mac_addr
 * @tid
 */
struct GNU_PACKED ba_mactid_param {
	unsigned char mac_addr[6];
	unsigned char tid;
};

/**
 * enum mtk_nl80211_vendor_attrs_ap_vow - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AP_VOW.
 * Information in these attributes is used to set vow configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_ATF_EN_INFO: u8, air time fairness enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO: refer to vow_group_en_param
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_EN_INFO: u8, air time bw enalbe attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO: refer to vow_group_en_param
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO: refer to vow_rate_param
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO: refer to vow_rate_param
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO: refer to vow_ratio_param
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO: refer to vow_ratio_param
 * @MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO: refer to vow_info
 */
enum mtk_nl80211_vendor_attrs_ap_vow{
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_AP_VOW_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_ATF_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO,
	__MTK_NL80211_VENDOR_ATTR_AP_VOW_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_VOW_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_VOW_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_ap_monitor - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR.
 * Information in these attributes is used to set ap monitor configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_ENABLE: u8, air monitor enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_RULE: u8 array[3] air monitor rule attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA: u8 array[6], air monitor mac address attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_IDX: u8, air monitor index attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_CLR: None, air monitor clear attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA0: u8 array[6], air monitor mac address with idx 0 attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_STA_STATS: Get air monitor specific stats for target mac address
 */
enum mtk_nl80211_vendor_attrs_ap_monitor {
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_ENABLE,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_RULE,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_IDX,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_CLR,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA0,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA_ID,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_MAX_PKT,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_RESULT,
	MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_STA_STATS,

	__MTK_NL80211_VENDOR_ATTR_AP_MONITOR_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_MONITOR_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_MONITOR_AFTER_LAST - 1
};

/**
 * mtk_nl80211_vendor_attrs_sta_clean - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_CLEAN_STATION.
 * Information in these attributes is used to clear the station entry
 * from driver when requested by application
 *
 * @MTK_NL80211_VENDOR_ATTR_STA_CLEAN_MAC
 */
enum mtk_nl80211_vendor_attrs_sta_clean {
	MTK_NL80211_VENDOR_ATTR_STA_CLEAN_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_STA_CLEAN_MAC,

	__MTK_NL80211_VENDOR_ATTR_STA_CLEAN_AFTER_LAST,
	MTK_NL80211_VENDOR_STA_CLEAN_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_STA_CLEAN_AFTER_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_ap_ba - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_BA.
 * Information in these attributes is used to set bss configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO: u8, BA decline attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_HT_BA_WSIZE_INFO: u16, BA tx and rx window size
 * @MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO: u8, BA enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO: refer to ba_mactid_param
 * @MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO: refer to ba_mactid_param
 * @MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO: refer to ba_mactid_param
 */
enum mtk_nl80211_vendor_attrs_ap_ba {
	MTK_NL80211_VENDOR_ATTR_AP_BA_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_HT_BA_WSIZE_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO,
	__MTK_NL80211_VENDOR_ATTR_AP_BA_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_BA_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_BA_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_ap_bss - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS.
 * Information in these attributes is used to set bss configuration
 * to driver from user application.
 *
 * MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MODE: u8, wireless mode attributes
 * MTK_NL80211_VENDOR_ATTR_HT_TX_STREAM_INFO: u32, ht tx stream attributes
 * MTK_NL80211_VENDOR_ATTR_ASSOCREQ_RSSI_THRES_INFO: char, assocreq rssi thres attributes
 * MTK_NL80211_VENDOR_ATTR_AP_MPDU_DENSITY: u8, mpdu density attributes
 * MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN: u8, enable AMSDU attributes
 * MTK_NL80211_VENDOR_ATTR_AP_CNT_THR: u8, HtBssCoexApCntThr attributes
 * MTK_NL80211_VENDOR_ATTR_AP_HT_EXT_CHA: u8, HtExtcha attributes
 * MTK_NL80211_VENDOR_ATTR_AP_HT_PROTECT: u8, HtProtect attributes
 * MTK_NL80211_VENDOR_ATTR_AP_DISALLOW_NON_VHT:u8, VhtDisallowNonVht attributes
 * MTK_NL80211_VENDOR_ATTR_AP_ETXBF_EN_COND:u8, ETxBfEnCond attributes
 * MTK_NL80211_VENDOR_ATTR_AP_PMF_SHA256:u8, PMF_SHA256 attributes
 * MTK_NL80211_VENDOR_ATTR_AP_BCN_INT:u16, beacon interval attributes
 * MTK_NL80211_VENDOR_ATTR_AP_DTIM_INT:u8, dtim interval attributes
 * MTK_NL80211_VENDOR_ATTR_AP_HIDDEN_SSID:u8, hide ssid attributes
 * MTK_NL80211_VENDOR_ATTR_AP_HT_OP_MODE:u8, ht operation mode attributes
 * MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_DL_EN: u8, muru ofdma dl enable attributes
 * MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_UL_EN: u8, muru ofdma ul enable attributes
 * MTK_NL80211_VENDOR_ATTR_MU_MIMO_DL_EN: u8, mu mimo dl enable attributes
 * MTK_NL80211_VENDOR_ATTR_MU_MIMO_UL_EN: u8, mu mimo ul enable attributes
 * MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_IDLE: u32, bss max idle time(seconds) attributes
 * MTK_NL80211_VENDOR_ATTR_RTS_BW_SIGNALING: u8, RTS BW signaling config attributes
 * MTK_NL80211_VENDOR_ATTR_SET_AP_BSS_COLOR: struct bss_color, set bss color for AP
 * MTK_NL80211_VENDOR_ATTR_AP_ISOLATION: bss isolation
 * MTK_NL80211_VENDOR_ATTR_AP_SET_11AXONLY : set 11ax only
 * MTK_NL80211_VENDOR_ATTR_AP_SET_11BEONLY : set 11be only
 * MTK_NL80211_VENDOR_ATTR_AP_SET_NO_AGMODE : set NO AG Mode
 * MTK_NL80211_VENDOR_ATTR_AP_SET_NO_NMODE : set NO N Mode
 * MTK_NL80211_VENDOR_ATTR_AP_SET_NO_BMODE : set NO B Mode
 */
enum mtk_nl80211_vendor_attrs_ap_bss {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_AP_BSS_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MODE,
	MTK_NL80211_VENDOR_ATTR_HT_TX_STREAM_INFO,
	MTK_NL80211_VENDOR_ATTR_ASSOCREQ_RSSI_THRES_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_MPDU_DENSITY,
	MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN,
	MTK_NL80211_VENDOR_ATTR_AP_CNT_THR,
	MTK_NL80211_VENDOR_ATTR_AP_HT_EXT_CHA,
	MTK_NL80211_VENDOR_ATTR_AP_HT_PROTECT,
	MTK_NL80211_VENDOR_ATTR_AP_DISALLOW_NON_VHT,
	MTK_NL80211_VENDOR_ATTR_AP_ETXBF_EN_COND,
	MTK_NL80211_VENDOR_ATTR_AP_PMF_SHA256,
	MTK_NL80211_VENDOR_ATTR_AP_BCN_INT,
	MTK_NL80211_VENDOR_ATTR_AP_DTIM_INT,
	MTK_NL80211_VENDOR_ATTR_AP_HIDDEN_SSID,
	MTK_NL80211_VENDOR_ATTR_AP_HT_OP_MODE,
	MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_DL_EN,
	MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_UL_EN,
	MTK_NL80211_VENDOR_ATTR_MU_MIMO_DL_EN,
	MTK_NL80211_VENDOR_ATTR_MU_MIMO_UL_EN,
	MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_IDLE,
	MTK_NL80211_VENDOR_ATTR_RTS_BW_SIGNALING,
	MTK_NL80211_VENDOR_ATTR_SET_AP_BSS_COLOR,
	MTK_NL80211_VENDOR_ATTR_SET_AFC_PARAM,
	MTK_NL80211_VENDOR_ATTR_AP_MGMT_RX,
	MTK_NL80211_VENDOR_ATTR_AP_NO_BCN,
	MTK_NL80211_VENDOR_ATTR_AP_ISOLATION,
	MTK_NL80211_VENDOR_ATTR_AP_SET_11AXONLY,
	MTK_NL80211_VENDOR_ATTR_AP_SET_11BEONLY,
	MTK_NL80211_VENDOR_ATTR_AP_SET_NO_AGMODE,
	MTK_NL80211_VENDOR_ATTR_AP_SET_NO_NMODE,
	MTK_NL80211_VENDOR_ATTR_AP_SET_NO_BMODE,
	__MTK_NL80211_VENDOR_ATTR_AP_BSS_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_BSS_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_BSS_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_ap_radio - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO.
 * Information in these attributes is used to set phy or global configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_IEEE80211H_INFO: u8, ieee80211h enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_ACKCTS_TOUT_EN_INFO: u8, ACKCTS timeout config enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_DISTANCE_INFO: u32, ACK timeout by distacne attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_CCK_ACK_TOUT_INFO: u32, ACK timeout by CCK mode attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_OFDM_ACK_TOUT_INFO: u32, ACK timeout by OFDM mode attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_OFDMA_ACK_TOUT_INFO: u32, ACK timeout by OFDMA mode attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_2G_CSA_SUPPORT: u8, 2G CSA support enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_MRU_INFO: u8, MRU enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_AI_ANTINOISE: u16, ANTINOISE enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_STA_NUM:u16, set bss max sta num
 * @MTK_NL80211_VENDOR_ATTR_AP_PHY_MODE: u8, per band phymode support enable attributes
 * @MTK_NL80211_VENDOR_ATTR_AP_CSA_SUPPORT: u8, CSA support enable attributes
 */
enum mtk_nl80211_vendor_attrs_ap_radio {
	MTK_NL80211_VENDOR_ATTR_AP_RADIO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_AP_IEEE80211H_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_ACKCTS_TOUT_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_DISTANCE_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_CCK_ACK_TOUT_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_OFDM_ACK_TOUT_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_OFDMA_ACK_TOUT_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_2G_CSA_SUPPORT,
	MTK_NL80211_VENDOR_ATTR_AP_MRU_INFO,
	MTK_NL80211_VENDOR_ATTR_AP_AI_ANTINOISE,
	MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_STA_NUM,
	MTK_NL80211_VENDOR_ATTR_AP_PHY_MODE,
	MTK_NL80211_VENDOR_ATTR_AP_CSA_SUPPORT,

	__MTK_NL80211_VENDOR_ATTR_AP_RADIO_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_RADIO_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_RADIO_AFTER_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_wmm - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_WMM.
 * Information in these attributes is used to set wmm configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO: array, ap aifsn value, AC0:AC1:AC2:AC3
 * @MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO: array, ap cwmin value, AC0:AC1:AC2:AC3
 * @MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO: array, ap cwmax value, AC0:AC1:AC2:AC3
 * @MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO: array, ap txop value, AC0:AC1:AC2:AC3
 * @MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO: array, ap wmm capability enabled/disabled
 */
enum mtk_nl80211_vendor_attrs_wmm{
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_WMM_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO,
	MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO,
	MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO,
	MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO,
	MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO,

	__MTK_NL80211_VENDOR_ATTR_WMM_AFTER_LAST,
	MTK_NL80211_VENDOR_WMM_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_WMM_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_vlan - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_VLAN.
 * Information in these attributes is used to set vlan configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_VLAN_EN_INFO: u8, vlan enable attributes
 * @MTK_NL80211_VENDOR_ATTR_VLAN_ID_INFO: u16, vlan id attributes
 * @MTK_NL80211_VENDOR_ATTR_VLAN_PRIORITY_INFO: u8, vlan priority attributes
 * @MTK_NL80211_VENDOR_ATTR_VLAN_TAG_INFO: u8, vlan tag attributes
 * @MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO: refer to vlan_policy_param
 */
enum mtk_nl80211_vendor_attrs_vlan{
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_VLAN_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_VLAN_EN_INFO,
	MTK_NL80211_VENDOR_ATTR_VLAN_ID_INFO,
	MTK_NL80211_VENDOR_ATTR_VLAN_PRIORITY_INFO,
	MTK_NL80211_VENDOR_ATTR_VLAN_TAG_INFO,
	MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO,
	__MTK_NL80211_VENDOR_ATTR_VLAN_AFTER_LAST,
	MTK_NL80211_VENDOR_VLAN_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_VLAN_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_mbo - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_MBO.
 * Information in these attributes is used to set MBO configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC to set npc list into unsigned char npc_value[MBO_NPC_MAX_LEN];
 */
enum mtk_nl80211_vendor_attrs_mbo {
	MTK_NL80211_VENDOR_ATTR_MBO_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC,
	__MTK_NL80211_VENDOR_ATTR_MBO_LAST,
	MTK_NL80211_VENDOR_ATTR_MBO_MAX =
	__MTK_NL80211_VENDOR_ATTR_MBO_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_ft - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_FT.
 * Information in these attributes is used to set ft configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_FTIE: struct ap_11r_params, ap ftie value, r0kh-id and r1kh-id
 */
enum mtk_nl80211_vendor_attrs_ft {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_FT_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_FTIE,

	__MTK_NL80211_VENDOR_ATTR_FT_LAST,
	MTK_NL80211_VENDOR_ATTR_FT_MAX =
	__MTK_NL80211_VENDOR_ATTR_FT_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_mlo_ie - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_MLO_IE.
 * Information in these attributes is used to set mlo ie configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_AUTH_MLO_IE: struct wpa_ml_ie_parse, ap mlo ie value
 */
enum mtk_nl80211_vendor_attrs_mlo_ie {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_MLO_IE_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_AUTH_MLO_IE,

	__MTK_NL80211_VENDOR_ATTR_MLO_IE_LAST,
	MTK_NL80211_VENDOR_ATTR_MLO_IE_MAX =
	__MTK_NL80211_VENDOR_ATTR_MLO_IE_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_del_mlo_sta_entry - This enum defines
 * attributes required for MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY.
 * Information in these attributes is used to clear old sta entry and
 * start mlo connection for over_ds case from Hostapd.
 *
 * @MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY: Parse FT Action frame.
 */
enum mtk_nl80211_vendor_attrs_del_mlo_sta_entry {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY,

	__MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_LAST,
	MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_MAX =
		__MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_pmkid - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_PMKID.
 * Information in these attributes is used to set pmkid cache configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_SUBCMD_SET_PMKSA_CACHE
 */
enum mtk_nl80211_vendor_attrs_pmkid {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_PMKID_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_PMKSA_CACHE,

	__MTK_NL80211_VENDOR_ATTR_PMKID_LAST,
	MTK_NL80211_VENDOR_ATTR_PMKID_MAX =
	__MTK_NL80211_VENDOR_ATTR_PMKID_LAST - 1
};
/**
 * mtk_nl80211_vendor_attrs_sta_mac - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_MAC.
 * Information in these attributes is used to set mac addr configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_SUBCMD_SET_STA_MAC
 */
enum mtk_nl80211_vendor_attrs_sta_mac {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_SET_MAC_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_STA_MAC,

	__MTK_NL80211_VENDOR_ATTR_SET_MAC_LAST,
	MTK_NL80211_VENDOR_ATTR_SET_MAC_MAX =
	__MTK_NL80211_VENDOR_ATTR_SET_MAC_LAST - 1
};

/**
 * mtk_nl80211_vendor_attrs_sta_seq_num - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_SEQ_NUM.
 * Information in these attributes is used to set seq num configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM
 */
enum mtk_nl80211_vendor_attrs_sta_seq_num {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM,

	__MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_LAST,
	MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_MAX =
	__MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_mlo_preset_link - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_MLO_PRESET_LINK.
 * Information in these attributes is used to set mlo link id
 * to driver from user application such as wpa_supplicant.
 *
 * @MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINK_INFO: u8, set mlo link id value, 0, 1 or other
 */
enum mtk_nl80211_vendor_attrs_mlo_preset_link{
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_MLO_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINKID_INFO,
	__MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINKID_AFTER_LAST,
	MTK_NL80211_VENDOR_MLO_PRESET_LINKID_ATTR_MAX =
	__MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINKID_AFTER_LAST - 1
};


/**
 * enum mtk_nl80211_vendor_attrs_ap_security - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AP_SECURITY.
 * Information in these attributes is used to set security information
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_AUTHMODE:  u32, auth mode attributes, refer to mtk_vendor_attr_authmode
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_ENCRYPTYPE: u32, encryptype, refer to mtk_vendor_attr_encryptype
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_REKEYINTERVAL: u32, rekey interval in seconds
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_REKEYMETHOD: u8, 0-by time, 1-by pkt count
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_DEFAULTKEYID: u8, default key index
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_WEPKEY: refer to wep_key_param
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PASSPHRASE: string
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PMF: u8 1-support pmf, 0-not support pmf
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PMF_REQUIRE: u8 1-pmf is required, 0-pmf is not required
 * @MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PMF_SHA256: u8 1-pmfsha256 is desired, 0-pmfsha256 is not desired
 */
enum mtk_nl80211_vendor_attrs_ap_security {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_AUTHMODE,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_ENCRYPTYPE,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_REKEYINTERVAL,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_REKEYMETHOD,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_DEFAULTKEYID,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_WEPKEY,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PASSPHRASE,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PMF_CAPABLE,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PMF_REQUIRE,
	MTK_NL80211_VENDOR_ATTR_AP_SECURITY_PMF_SHA256,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_AP_SECURITY_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_SECURITY_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_SECURITY_AFTER_LAST - 1
};


/**
 * enum mtk_vendor_attr_ap_wireless - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_AP_WIRELESS.
 * Information in these attributes is used to set wireless information
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_MCS:  u8, fixed mcs
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_OFDMA: u8,  0-disable, 1-DL, 2-UL
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_PPDU_TX_TYPE: u8, 0-SU, 1-MU, 2-LEGACY
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_NUSERS_OFDMA: u8, NumUsersOFDMA
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_BA_BUFFER_SIZE:  u8, ba req buffer size
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MIMO: u8, 0-DL, 1-UL
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMPDU: u8, 0-disable, 1-enable
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMSDU: u8, 0-disable, 1-enable
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT: u8 cert mode, 0-disable, 1-enable
 * @MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_HE_TXOP_RTS_THLD: u16, HE TXOP RTS threshold
 */
enum mtk_vendor_attr_ap_wireless {
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_MCS,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_OFDMA,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_PPDU_TX_TYPE,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_NUSERS_OFDMA,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_BA_BUFFER_SIZE,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MIMO,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMPDU,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMSDU,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_HE_TXOP_RTS_THLD,

	/* keep last */
	__MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MAX =
	__MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AFTER_LAST - 1
};



/**
 * enum mtk_nl80211_vendor_attrs_ap_rfeature - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_AP_RFEATURE.
 * Information in these attributes is used to set radio information
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_GI:  u8, he gi attributes, 0-0.8; 1-1.6; 2-3.2
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_LTF: u8, he ltf attribute, 0-3.2; 1-6.4; 2-12.8
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE: string,  "<en>,<type>",
 *  en: 0-disable,1-enable; type: 0~7
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE_EN: u8, 0-disable, 1-enable
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_ACK_PLCY: u8, 0-4
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_PPDU_TYPE: u8, 0-4
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_TXOP_RTS_THLD: u16, HE TXOP RTS threshold
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_EXTRA_LTF_DISABLE: u8, 0-disable, 1-enable
 * @MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_EXTRA_LTF: u8, 4 or 8
 */
enum mtk_nl80211_vendor_attrs_ap_rfeature {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_GI,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_LTF,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_ACK_PLCY,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_PPDU_TYPE,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_TXOP_RTS_THLD,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_EXTRA_LTF_DISABLE,
	MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_EXTRA_LTF,
	__MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_RFEATURE_ATTR_MAX =
	__MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_AFTER_LAST - 1
};


/**
 * enum mtk_nl80211_vendor_attr_get_static_info - Specifies the vendor attribute values
 * to get static info
 */
enum mtk_nl80211_vendor_attr_get_static_info {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_CHIP_ID,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_DRIVER_VER,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_COEXISTENCE,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_VER,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WAPP_SUPPORT_VER,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_AFC_DATA,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_OPERATING_INFO,

	__MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_get_band_info - Specifies the vendor attribute values
 * to get band info
 */
enum mtk_nl80211_vendor_attr_get_band_info {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND,
	MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST,
	MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BANDWIDTH,
	MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_CHANNEL,

	__MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_get_runtime_info - Specifies the vendor attribute values
 * to get runtime info
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_ACS_REFRESH_PERIOD: u32, ACS check period.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANNEL_LAST_CHANGE: u32, The accumulated
 *   time since the current Channel came into use. unit:us.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_SSIDS: u8, Max number of ssid.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_EXTENSION_CHANNEL: u8, Secondary extension
 *   channel position. 0:none; 1:above; 3:below
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_TRANSMITPOWER: u8, the current transmit
 *   power level as a percentage of full power.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MLO_MLD_INFO: u8, MLD Group Address
 *   and MLD Index for the BSS.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BASIC_RATE: u64, basic rates.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SUPP_RATE: max length 12 bytes, support rates.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CURRENT_CHANNEL_INFO: customized channel stats.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BW: run time bw.
 * @MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_COUNTRYCODE: countrycode get.
 */
enum mtk_nl80211_vendor_attr_get_runtime_info {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_STA,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHAN_LIST,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSS_INFO,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_NOP_CHANNEL_LIST,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WAPP_WSC_PROFILES,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_PMK_BY_PEER_MAC,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_802_11_AUTHENTICATION_MODE,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_802_11_MAC_ADDRESS,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_802_11_CURRENTCHANNEL,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_ACS_REFRESH_PERIOD,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANNEL_LAST_CHANGE,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_SSIDS,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_EXTENSION_CHANNEL,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_TRANSMITPOWER,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_80211H,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MLO_MLD_INFO,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BASIC_RATE,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SUPP_RATE,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CURRENT_CHANNEL_INFO,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BW,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_COUNTRYCODE,

	__MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_LAST - 1
};

struct GNU_PACKED mtk_wifi_uplink_info {
	char ssid[SSID_LEN];
	unsigned char channel;
	signed char noise;
	signed char snr;
	signed char rssi;
	unsigned int tx_rate;
	unsigned int rx_rate;
	unsigned int tx_rate_rt;
	unsigned int rx_rate_rt;
	unsigned long mcBytesTx;
	unsigned long bcBytesTx;
	unsigned int avg_tx_rate[ANDLINK_IF_MAX];
	unsigned int avg_rx_rate[ANDLINK_IF_MAX];
	unsigned int max_tx_rate[ANDLINK_IF_MAX];
	unsigned int max_rx_rate[ANDLINK_IF_MAX];
};
/**
 * enum mtk_nl80211_vendor_attr_get_statistic - Specifies the vendor attribute values
 * to get statistic info
 * @MTK_NL80211_VENDOR_ATTR_GET_UPLINK_STATISTICS_ANDLINK_FORMAT: get uplink info, add struct GNU_PACKED mtk_wifi_uplink_info
 */
enum mtk_nl80211_vendor_attr_get_statistic {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_GET_802_11_STATISTICS,
	MTK_NL80211_VENDOR_ATTR_GET_TX_PWR,
	MTK_NL80211_VENDOR_ATTR_GET_AP_METRICS,
	MTK_NL80211_VENDOR_ATTR_GET_802_11_PER_BSS_STATISTICS,
	MTK_NL80211_VENDOR_ATTR_GET_CPU_TEMPERATURE,
	MTK_NL80211_VENDOR_ATTR_GET_UPLINK_STATISTICS_ANDLINK_FORMAT,

	_MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_LAST,
	MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_MAX =
	_MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_wapp_req - Specifies the vendor attribute values
 * to request wifi info
 */
enum mtk_nl80211_vendor_attr_wapp_req {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_WAPP_REQ_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_WAPP_REQ,

	__MTK_NL80211_VENDOR_ATTR_WAPP_REQ_LAST,
	MTK_NL80211_VENDOR_ATTR_WAPP_REQ_MAX =
	__MTK_NL80211_VENDOR_ATTR_WAPP_REQ_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_event_rsp_afc_event - Specifies the vendor attribute values
 * to get afc event
 */
enum mtk_nl80211_vendor_attr_event_rsp_afc_event {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_AFC_EVENT,

	__MTK_NL80211_VENDOR_ATTR_EVENT_RSP_AFC_EVENT_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_RSP_AFC_EVENT_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_RSP_AFC_EVENT_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_event_rsp_wapp_event - Specifies the vendor attribute values
 * to get wifi info event
 */
enum mtk_nl80211_vendor_attr_event_rsp_wapp_event {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_RSP_WAPP_EVENT_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_EVENT_RSP_WAPP_EVENT,

	__MTK_NL80211_VENDOR_ATTR_EVENT_RSP_WAPP_EVENT_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_RSP_WAPP_EVENT_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_RSP_WAPP_EVENT_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_event_mlo_reconf - Specifies the vendor attribute values
 * to get mlo reconfiguration event
 */
enum mtk_nl80211_vendor_attr_event_mlo_reconf {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_MLO_RECONF_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_EVENT_MLO_RECONF_SM,

	__MTK_NL80211_VENDOR_ATTR_EVENT_MLO_RECONF_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_MLO_RECONF_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_MLO_RECONF_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_event_attr_acs_complete_event - Specifies the vendor attribute values
 * to get acs event
 */
enum mtk_nl80211_vendor_attr_event_acs_complete_event {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_PRIMARY_FREQUENCY,
/*	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_PRIMARY_CHANNEL, */
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_SECONDARY_FREQUENCY,
/*	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_SECONDARY_CHANNEL, */
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_HW_MODE,
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_EDMG_CHANNEL,
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_VHT_SEG0_CENTER_CHANNEL,
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_VHT_SEG1_CENTER_CHANNEL,
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_CHWIDTH,
	__MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_EVENT_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_EVENT_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_EVENT_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_get_cap - Specifies the vendor attribute values
 * to get capability info
 */
enum mtk_nl80211_vendor_attr_get_cap {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_CAC_CAP,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MISC_CAP,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HT_CAP,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_VHT_CAP,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HE_CAP,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WF6_CAPABILITY,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_QUERY_11H_CAPABILITY,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_QUERY_RRM_CAPABILITY,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_QUERY_KVR_CAPABILITY,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_EHT_CAP,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MLO_CAP,
	/*To get WDEV CAP of AP/APCLI wireless mode*/
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WDEV,

	__MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_mwds_set - Specifies the vendor attribute values
 * MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE: u8, enable/disable MWDS attribute
 * MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE: u8, clear/show MWDS autotes result attribute
 */
enum mtk_nl80211_vendor_attrs_mwds_set {
	MTK_NL80211_VENDOR_ATTR_MWDS_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE,
	MTK_NL80211_VENDOR_ATTR_MWDS_INFO,

	__MTK_NL80211_VENDOR_ATTR_MWDS_LAST,
	MTK_NL80211_VENDOR_MWDS_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_MWDS_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_country_set - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_COUNTRY.
 * Information in these attributes is used to set country code/region/string
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE: string, Country code
 * @MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_REGION: u32, Region code
 * @MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME: string, Country string
 */
enum mtk_nl80211_vendor_attrs_country_set {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_COUNTRY_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE,
	MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_REGION,
	MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME,

	__MTK_NL80211_VENDOR_ATTR_COUNTRY_LAST,
	MTK_NL80211_VENDOR_ATTR_COUNTRY_MAX = __MTK_NL80211_VENDOR_ATTR_COUNTRY_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_cosr_ie - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_COSR_IE.
 * Information in these attributes is used to set cosr ie configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_COSR_IE
 */
enum mtk_nl80211_vendor_attrs_set_cosr_ie {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_COSR_IE_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_AP_COSR_IE,

	__MTK_NL80211_VENDOR_ATTR_SET_COSR_IE_LAST,
	MTK_NL80211_VENDOR_ATTR_SET_COSR_IE_MAX =
	__MTK_NL80211_VENDOR_ATTR_SET_COSR_IE_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_cosr_info - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_COSR_INFO.
 * Information in these attributes is used to set sta/ap info configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_COSR_IE
 * @MTK_NL80211_VENDOR_ATTR_COSR_SET_RSSI_TH: set cosr ap rssi th
 */
enum mtk_nl80211_vendor_attrs_set_cosr_info {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_COSR_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_COSR_STAINFO,
	MTK_NL80211_VENDOR_ATTR_SET_COSR_APINFO,
	MTK_NL80211_VENDOR_ATTR_COSR_SET_RSSI_TH,

	__MTK_NL80211_VENDOR_ATTR_SET_COSR_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_SET_COSR_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_SET_COSR_INFO_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_event_cosr - This enum defines the value of
 * MTK_NL80211_VENDOR_EVENT_COSR.
 * Information in these attributes is used to notify upper application cosr event.
 */
enum mtk_nl80211_vendor_attr_event_cosr {
	/* don`t change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_COSR_INVALLD = 0,

	MTK_NL80211_VENDOR_ATTR_STA_RSSI_CHANGE,
	MTK_NL80211_VENDOR_ATTR_COAP_UPDATED,
	MTK_NL80211_VENDOR_ATTR_COAP_FOUND,
	MTK_NL80211_VENDOR_ATTR_COSR_FRAME,

	__MTK_NL80211_VENDOR_ATTR_COSR_LAST,
	MTK_NL80211_VENDOR_ATTR_COSR_MAX =
	__MTK_NL80211_VENDOR_ATTR_COSR_LAST - 1
};

enum mtk_nl80211_vendor_attr_radar_notify {
	MTK_NL80211_VENDOR_ATTR_RADAR_INVALLD = 0,
	MTK_NL80211_VENDOR_ATTR_RADAR_FREQ,
	MTK_NL80211_VENDOR_ATTR_RADAR_FREQ_OFFSET,
	MTK_NL80211_VENDOR_ATTR_RADAR_CHANNEL_TYPE,
	MTK_NL80211_VENDOR_ATTR_RADAR_CHANNEL_WIDTH,
	MTK_NL80211_VENDOR_ATTR_RADAR_CHANNEL_CENTER_FREQ1,
	MTK_NL80211_VENDOR_ATTR_RADAR_CHANNEL_CENTER_FREQ2,

	__MTK_NL80211_VENDOR_ATTR_RADAR_LAST,
	MTK_NL80211_VENDOR_ATTR_RADAR_MAX =
	__MTK_NL80211_VENDOR_ATTR_RADAR_LAST - 1
};

#ifndef MAX_NUM_OF_CHANNELS
#ifdef NON_WIFI_FREQ_5MHZ_SHIFT_SUPPORT
#define MAX_NUM_OF_CHANNELS 300
#else
#define MAX_NUM_OF_CHANNELS 59
#endif
#endif
/**
 * structure ch_list_info - This structure defines the payload format of
 * some attributes which related to channel list operation
 * Information in this structure is used to do channel list operation
 * @ch_list: ch list
 * @num_of_ch: num of ch
 */
struct GNU_PACKED ch_list_info {
	unsigned char ch_list[MAX_NUM_OF_CHANNELS];
	unsigned char num_of_ch;
};

/**
 * enum mtk_nl80211_vendor_attrs_chan_set - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_CHANNEL.
 * Information in these attributes are used to set channel and bandwidth
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_CHAN_SET_NUM: u8, channel ID
 * @MTK_NL80211_VENDOR_ATTR_CHAN_SET_FREQ: u32, frequency
 * @MTK_NL80211_VENDOR_ATTR_CHAN_SET_BW: u32, channel bandwidth
 * @MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_EXTCHAN: u8, HT 40 extension channel
 * @MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_COEX: u8, HT 20/40 coex
 * @MTK_NL80211_VENDOR_ATTR_CHAN_SET_BYPASS_CAC: u8, bypass CAC check or not
 */
enum mtk_nl80211_vendor_attrs_chan_set {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_CHAN_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_CHAN_SET_NUM,
	MTK_NL80211_VENDOR_ATTR_CHAN_SET_FREQ,
	MTK_NL80211_VENDOR_ATTR_CHAN_SET_BW,
	MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_EXTCHAN,
	MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_COEX,
	MTK_NL80211_VENDOR_ATTR_CHAN_SET_BYPASS_CAC,

	__MTK_NL80211_VENDOR_ATTR_CHAN_LAST,
	MTK_NL80211_VENDOR_ATTR_CHAN_MAX = __MTK_NL80211_VENDOR_ATTR_CHAN_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_auto_ch_sel - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_AUTO_CH_SEL.
 * Information in these attributes are used to trigger acs operation
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL: u8, the algorithm used for acs
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME: u32, the check time for periodic acs
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC: u8, enable/disable 6g psc acs
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL: u8, enable/disable partial scan acs
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL: u16, set the scanning dwell time
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL: u16, set the restoring dwell time
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM: u16, set the continuously scanning channel number
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_PREFER_LIST: refer to ch_list_info
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_SKIP_LIST: refer to ch_list_info
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_CH_UTIL_THR: u8, channel utilization threshold for
 *	periodic ACS executing, in unit percentage; Channel utilization ratio is the ratio
 *	utilized by others; Periodic ACS scan will be triggered only when the current channel
 *	utilization ratio higher than the threshold.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_STA_NUM_THR: u16, connected station number threshold
 *	for periodic ACS executing; Periodic ACS scan can be triggered only when the current
 *	connected station number less than the threshold; A zero setting for the station
 *	number threshold means the station number won't be considered.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_DATA_RATE_WT: u16, The weight of data rate in calculating scores.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_PRIO_WT: u16, The weight of channel priority in calculating scores.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_TX_POWER_CONS: u8, whether consider TX power when calculating the data rate.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_MAX_ACS_TIMES: u8, maximum times of execute ACS in every 8 hours.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_SWITCH_THR: u8, If the score of the channel selected
 *	by ACS does not exceed the threshold compared.to the score of the original
 *	working channel, it will not switch.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_ICE_CH_UTIL_THR: u8, If the percentage of busy time
 *	in the measurement exceeds the threshold, the channel is considered busy in emergency case.
 * @MTK_NL80211_VENDOR_ATTR_AUTO_CH_OBSS_WT: u16, The weight of other BSS in calculating scores.
 */
enum mtk_nl80211_vendor_attr_auto_ch_sel {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_PREFER_LIST,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_SKIP_LIST,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_CH_UTIL_THR,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_STA_NUM_THR,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_DATA_RATE_WT,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_PRIO_WT,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_TX_POWER_CONS,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_MAX_ACS_TIMES,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_SWITCH_THR,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_ICE_CH_UTIL_THR,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_OBSS_WT,

	__MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_LAST,
	MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_MAX =
	__MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_LAST - 1
};

struct GNU_PACKED wifi_scan_info_entry {
	char ssid[SSID_LEN];
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char channel;
	char rssi;
	unsigned int bandwidth;
	unsigned char load_rate;
	char wifistandard;
	char authmode[32];
};

struct GNU_PACKED mtk_wifi_scan_info {
	int num;
	int offset;
	struct wifi_scan_info_entry scan_entry[ANDLINK_MAX_WLAN_NEIGHBOR];
};
/**
 * enum mtk_nl80211_vendor_attr_scan - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_SCAN.
 * Information in these attributes are used to trigger scan operation
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SCAN_TYPE: u8, the type of scan
 * @MTK_NL80211_VENDOR_ATTR_SCAN_CLEAR: without value, to clear scan result
 * @MTK_NL80211_VENDOR_ATTR_SCAN_SSID: ssid, the ssid for full scan
 * @MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_INTERVAL: u32, the interval of continous partial scans
 * @MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH: u8, the scanned ch num before restoring for partial scan
 * @MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH: u32, the target ch for offch scan
 * @MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE: u8, passive or active for offch scan
 * @MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION: u32, the duration for offch scan
 * @MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX: u32, first bss indx for scan dump
 * @MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT: scan result information string
 * @MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN: u8, enable or disable 6G PSC scan
 * @MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT_ANDLINK_FORMAT: scan result, struct mtk_wifi_scan_info
 */
enum mtk_nl80211_vendor_attr_scan{
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_SCAN_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SCAN_TYPE,
	MTK_NL80211_VENDOR_ATTR_SCAN_CLEAR,
	MTK_NL80211_VENDOR_ATTR_SCAN_SSID,
	MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_INTERVAL,
	MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH,
	MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH,
	MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE,
	MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION,
	MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX,
	MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT,
	MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN,
	MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT_ANDLINK_FORMAT,

	__MTK_NL80211_VENDOR_ATTR_SCAN_LAST,
	MTK_NL80211_VENDOR_ATTR_SCAN_MAX =
	__MTK_NL80211_VENDOR_ATTR_SCAN_LAST - 1
};

/**
 * enum mtk_vendor_attr_scantype - This enum defines the value of
 * MTK_NL80211_VENDOR_ATTR_SCAN_TYPE.
 * Information in these attributes is used to set the scan type.
 */
enum mtk_vendor_attr_scantype{
	NL80211_FULL_SCAN,
	NL80211_PARTIAL_SCAN,
	NL80211_OFF_CH_SCAN,
	NL80211_2040_OVERLAP_SCAN
};

/**
 * enum mtk_nl80211_vendor_attr_set_action - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_ACTION
 * Information in these attributes are used to trigger action
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_ACTION_OFFCHAN_ACTION_FRAME: without value, to
 * trigger offchannel action
 * @MTK_NL80211_VENDOR_ATTR_SET_ACTION_QOS_ACTION_FRAME: without value, to set qos
 * @MTK_NL80211_VENDOR_ATTR_SET_ACTION_STATIC_PUNCTURE_BITMAP: to set static punctured bitmap
 * @MTK_NL80211_VENDOR_ATTR_SET_ACTION_COSR_ACTION_FRAME: wihtout value, to set cosr
 * @MTK_NL80211_VENDOR_ATTR_SET_ACTION_SEND_T2LM_REQUEST: trigger to send t2lm request
 * @@MTK_NL80211_VENDOR_ATTR_SET_ACTION_SET_T2LM_MAPPING: set t2lm mapping to driver
 */
enum mtk_nl80211_vendor_attr_set_action {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_ACTION_OFFCHAN_ACTION_FRAME,
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_QOS_ACTION_FRAME,
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_STATIC_PUNCTURE_BITMAP,
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_COSR_ACTION_FRAME,
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_SEND_T2LM_REQUEST,
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_SET_T2LM_MAPPING,

	__MTK_NL80211_VENDOR_ATTR_SET_ACTION_LAST,
	MTK_NL80211_VENDOR_ATTR_SET_ACTION_MAX =
	__MTK_NL80211_VENDOR_ATTR_SET_ACTION_LAST - 1
};

/**
* enum mtk_nl80211_vendor_attr_qos - This enum defines
* attributes required for MTK_NL80211_VENDOR_SUBCMD_QOS
* Information in these attributes are used by QoS
* to driver from user application.
*
* @MTK_NL80211_VENDOR_ATTR_QOS_UP_TUPLE_EXPIRED_NOTIFY:
* @MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC:
* @MTK_NL80211_VENDOR_ATTR_QOS_CHARATERISTICS_IE:
* @MTK_NL80211_VENDOR_ATTR_EPCS_STATE_SYNC:
*/
enum mtk_nl80211_vendor_attr_qos {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_QOS_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_QOS_UP_TUPLE_EXPIRED_NOTIFY,
	MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC,
	MTK_NL80211_VENDOR_ATTR_QOS_CHARATERISTICS_IE,
	MTK_NL80211_VENDOR_ATTR_EPCS_STATE_SYNC,

	__MTK_NL80211_VENDOR_ATTR_QOS_LAST,
	MTK_NL80211_VENDOR_ATTR_QOS_MAX =
	__MTK_NL80211_VENDOR_ATTR_QOS_LAST - 1
};

/**
* enum mtk_nl80211_vendor_attr_subcmd_offchannel_info - This enum defines
* attributes required for MTK_NL80211_VENDOR_SUBCMD_OFFCHANNEL_INFO
* Information in these attributes are used by offchannel_feature
* to driver from user application.
*
* @MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO:
*/
enum mtk_nl80211_vendor_attr_subcmd_offchannel_info {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO,

	__MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_LAST - 1
};

/**
* enum mtk_nl80211_vendor_attr_event_offchannel_info - This enum defines
* attributes required for MTK_NL80211_VENDOR_SUBCMD_EVENT_OFFCHANNEL_INFO
* Information in these attributes are used by driver generate event send to user application
*
* @MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO: scan result to app
*/
enum mtk_nl80211_vendor_attr_event_offchannel_info {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO,

	__MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO_LAST - 1
};

/**
* enum mtk_nl80211_vendor_attr_dfs - This enum defines
* attributes required for MTK_NL80211_VENDOR_SUBCMD_DFS
* Information in these attributes are used by DFS feature
*
* @MTK_NL80211_VENDOR_ATTR_DFS_SET_ZERO_WAIT: set zero wait
* @MTK_NL80211_VENDOR_ATTR_DFS_GET_ZERO_WAIT: get zero wait info
* @MTK_NL80211_VENDOR_ATTR_DFS_CAC_STOP: set cac stop
*/
enum mtk_nl80211_vendor_attr_dfs {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_DFS_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_DFS_SET_ZERO_WAIT,
	MTK_NL80211_VENDOR_ATTR_DFS_GET_ZERO_WAIT,
	MTK_NL80211_VENDOR_ATTR_DFS_CAC_STOP,

	__MTK_NL80211_VENDOR_ATTR_DFS_LAST,
	MTK_NL80211_VENDOR_ATTR_DFS_MAX =
	__MTK_NL80211_VENDOR_ATTR_DFS_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_easymesh - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_EASYMESH
 * Information in these attributes are used by Easymesh
 *
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_SSID: string, set easymesh ssid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PSK: psk, set easymesh psk
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WIRELESS_MODE, u8, set easymesh wireless mode
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_CHANNEL, u8, set easymesh channel
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBW, u8, set easymesh htbw
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VHTBW, u8, set easymesh vhtbw
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HIDDEN_SSID, u8, set easymesh hidden ssid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTH_MODE, string, set easymesh autho mode
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_ENCRYP_TYPE, string, set easymesh encryp type
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DEFAULT_KEY_ID, u8, set easymesh default keyid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_KEY1, string, set easymesh key1
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WPA_PSK, binary64, set easymesh wpa_psk
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_RADIO_ON, u32, set easymesh radio onoff
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DISCONNECT_STA, binary6, set easymesh disconnect sta
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_VID, u16, set easymesh ts bh primary vid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_PCP, u8, set easymesh ts bh primary pcp
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_VID, string, set easymesh ts bh vid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_FH_VID, u16, set easymesh ts fh vid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TRANSPARENT_VID, string, set easymesh transparent vid
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BCN_REQ, string, set easymesh bcn req payload
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBSSCOEX, u8, set easymesh ht bss coex
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMFMFPC, u8, set easymesh pmfmfpc
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTO_ROAMING, u8, set easymesh auto roaming
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DSCP_POLICY_ENABLE, u8, set easymesh dscp policy enable
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_QOS_MAP_CAPA, u8, set easymesh qos map capa
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMK: pmk, set easymesh pmk
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_RESET_DSCP2UP_TABLE, reset dscp2up table to default.
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME: get info
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME: get info
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_WAPP_IE: ie, set ie from wapp
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_CANCEL_ROC: without value, to cancel roc
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_DEL_CCE_IE: without value, to delete cce ie
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_DPP_URI: url, dpp url
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_1905_SEC_ENABLED: bool, set security of 1905
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_ONBOARDING_TYPE: bool, set onboarding type
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BHBSS: char, set bhbss enable/disable
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_FHBSS: char, set fhbss enable/disable
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_START_ROC: roc req data , to start ROC
 * @MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ: srg info , to get spt reuse req
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_CH: map_ch , set channel for Easymesh
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_ENABLE: char , set MAP  enable/disable
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DPP_STAMAC: MAC address , set sta mac address for DPP
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_EHTBW: set easymesh EHT bw
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_CREATE: to add mld group
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_DESTROY: to delete mld group
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_ADD: to add bss in mld group
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_DEL:to delete bss from  mld group
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_TRANSFER:to transfer bss to other mld group
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SR_ENABLE: to enable or disable SR
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_SR_BITMAP: to set Backhaul Bitmap for SR
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_NOP_STATE: to set or clear NOP state.
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_CH_PREF_STATE:to set channel priority.
 * @MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_RECONF: to del link with recofig option
 */
enum mtk_nl80211_vendor_attr_easymesh {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EASYMESH_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_SSID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PSK,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WIRELESS_MODE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_CHANNEL,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBW,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VHTBW,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HIDDEN_SSID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTH_MODE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_ENCRYP_TYPE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DEFAULT_KEY_ID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_KEY1,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WPA_PSK,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_RADIO_ON,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DISCONNECT_STA,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_VID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_PCP,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_VID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_FH_VID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TRANSPARENT_VID,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BCN_REQ,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBSSCOEX,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMFMFPC,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTO_ROAMING,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DSCP_POLICY_ENABLE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_QOS_MAP_CAPA,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMK,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_RESET_DSCP2UP_TABLE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_WAPP_IE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_CANCEL_ROC,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_DEL_CCE_IE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_DPP_URI,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_1905_SEC_ENABLED,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_ONBOARDING_TYPE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BHBSS,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_FHBSS,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_START_ROC,
	MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_CH,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_ENABLE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DPP_STAMAC,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_ASSOC_CTRL,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_EHTBW,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_CREATE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_DESTROY,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_ADD,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_DEL,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_TRANSFER,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SR_ENABLE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_SR_BITMAP,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_NOP_STATE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_CH_PREF_STATE,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_RECONF,

	__MTK_NL80211_VENDOR_ATTR_EASYMESH_LAST,
	MTK_NL80211_VENDOR_ATTR_EASYMESH_MAX =
	__MTK_NL80211_VENDOR_ATTR_EASYMESH_LAST - 1
};
/**
 * mtk_nl80211_vendor_attr_acl_mode - Specifies the vendor cmd mode for ACL
 *MTK_NL80211_VENDOR_ATTR_ACL_POLICY: u8, the mode of acl, 0 /1 /2 (disable, black list, white list)
 *MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC: array, add acl entry, the format like this, [mac_addr][mac_addr][...]
 *MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC: array, del acl entry, the format like this, [mac_addr][mac_addr][...]
 *MTK_NL80211_VENDOR_ATTR_ACL_SHOW_ALL: without value, send to driver and query list content.
 *MTK_NL80211_VENDOR_ATTR_ACL_CLEAR_ALL: without value, clear all stas in list
 *MTK_NL80211_VENDOR_ATTR_ACL_LIST_INFO: str, contain acl policy and all stas' mac in the list.
 *MTK_NL80211_VENDOR_ATTR_ACL_BL_ADD_MAC Add Entry in MAP BL Table to use BS2.0 force Steering
 *MTK_NL80211_VENDOR_ATTR_ACL_BL_DEL_MAC delete  Entry in MAP BL Table to use BS2.0 force Steering
 */
enum mtk_nl80211_vendor_attr_acl_mode {
	MTK_NL80211_VENDOR_ATTR_ACL_INVALID,
	MTK_NL80211_VENDOR_ATTR_ACL_POLICY = 1,
	MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC = 2,
	MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC = 3,
	MTK_NL80211_VENDOR_ATTR_ACL_SHOW_ALL = 4,
	MTK_NL80211_VENDOR_ATTR_ACL_CLEAR_ALL = 5,
	MTK_NL80211_VENDOR_ATTR_ACL_LIST_INFO = 6,
	MTK_NL80211_VENDOR_ATTR_ACL_BL_ADD_MAC = 7,
	MTK_NL80211_VENDOR_ATTR_ACL_BL_DEL_MAC = 8,
	__MTK_NL80211_VENDOR_ATTR_AP_ACL_AFTER_LAST,
	MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_AP_ACL_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_acl_policy - Specifies the vendor attribute values
 * to set acl policy.
 *MTK_NL80211_VENDOR_ATTR_ACL_DISABLE: u8, disable the acl
 *MTK_NL80211_VENDOR_ATTR_ACL_ENABLE_WHITE_LIST: u8, enter the white list mode
 *MTK_NL80211_VENDOR_ATTR_ACL_ENABLE_BLACK_LIST: u8, enter the black list mode
 */
enum mtk_nl80211_vendor_attr_acl_policy {
	MTK_NL80211_VENDOR_ATTR_ACL_DISABLE = 0,
	MTK_NL80211_VENDOR_ATTR_ACL_ENABLE_WHITE_LIST = 1,
	MTK_NL80211_VENDOR_ATTR_ACL_ENABLE_BLACK_LIST = 2,
};
/**
 * enum mtk_nl80211_vendor_attrs_tx_power - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_TXPOWER.
 * Information in these attributes is used to set AP Power configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE: u8, Power Drop Percentage, work after channel set.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_MAX: u8, set max tx power.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_INFO: u8, set cmd to fw for show pwr info.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE_EN: u8, enable power percentage.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_DROP_CTRL: u8, power drop by percentage, work immediatelly.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_DECREASE: u8, power drop by specific value.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_CTRL: u8, sku function enable/disable.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_INFO: u8, show sku information.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_MU: struct, MU power setting.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT: u8, management frame power specific.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_GET_MGMT: u8, get management frame power.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_PSU_SKU_CTRL: u8, psu sku function enable/disable.
 * @MTK_NL80211_VENDOR_ATTR_TXPWR_PSU_SKU_TABLE_IDX: u8, psu sku table index.
 */
enum mtk_nl80211_vendor_attrs_tx_power{
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_TXPWR_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE,
	MTK_NL80211_VENDOR_ATTR_TXPWR_MAX,
	MTK_NL80211_VENDOR_ATTR_TXPWR_INFO,
	MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE_EN,
	MTK_NL80211_VENDOR_ATTR_TXPWR_DROP_CTRL,
	MTK_NL80211_VENDOR_ATTR_TXPWR_DECREASE,
	MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_CTRL,
	MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_INFO,
	MTK_NL80211_VENDOR_ATTR_TXPWR_MU,
	MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT,
	MTK_NL80211_VENDOR_ATTR_TXPWR_GET_MGMT,
	MTK_NL80211_VENDOR_ATTR_TXPWR_PSU_SKU_CTRL,
	MTK_NL80211_VENDOR_ATTR_TXPWR_PSU_SKU_TABLE_IDX,
	__MTK_NL80211_VENDOR_ATTR_TXPWR_AFTER_LAST,
	MTK_NL80211_VENDOR_TXPWR_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_TXPWR_AFTER_LAST - 1
};
/**
 * structure mu_power_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_TXPWR_MU
 * Information in this structure is used to set ft configuration
 * to wifi driver.
 *
 * @en: enable/disable
 * @value: power in dBm:
 */
struct GNU_PACKED mu_power_param {
	unsigned int en;
	unsigned int value;
};
/**
 * enum mtk_nl80211_vendor_attrs_edca - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_EDCA.
 * Information in these attributes is used to set AP Power configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_TX_BURST: u8, Tx Burst enable/disable.

 */
enum mtk_nl80211_vendor_attrs_edca{
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EDCA_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_TX_BURST,
	__MTK_NL80211_VENDOR_ATTR_EDCA_AFTER_LAST,
	MTK_NL80211_VENDOR_EDCA_ATTR_MAX = __MTK_NL80211_VENDOR_ATTR_EDCA_AFTER_LAST - 1
};

/**
  * enum mtk_nl80211_vendor_attrs_mcast_snoop
  * This enum defines attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_MULTICAST_SNOOPING.
  * Information in these attributes is used to set multicast snooping configuration
  * to driver from user application.
  *
  * @MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE: u8, 0-disable, 1-enable
  * @MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_UNKNOWN_PLCY: u8, 0-unknown drop, 1-unknown flooding
  * @MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD: string
  * @MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL: string
  * @MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST: string
  * @MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR: string
  */
enum mtk_nl80211_vendor_attrs_mcast_snoop{
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_UNKNOWN_PLCY,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR,
	MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_CFGPERBANDSIZE,
	__MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_AFTER_LAST,
	MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX =
		__MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_AFTER_LAST - 1
};


/**
 * enum mtk_nl80211_vendor_attr_ate - Specifies the vendor attribute values
 * to request wifi info
 *
 * @MTK_NL80211_VENDOR_ATTR_HQA: struct hqa_frame
 */
enum mtk_nl80211_vendor_attr_hqa {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_HQA_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_HQA,

	__MTK_NL80211_VENDOR_ATTR_HQA_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_HQA_MAX =
		__MTK_NL80211_VENDOR_ATTR_HQA_AFTER_LAST - 1
};


/**
 * enum mtk_nl80211_vendor_attrs_ate - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_ATE.
 * Information in these attributes is used to set security information
 * to driver from user application.
 *
 *
 * @MTK_NL80211_VENDOR_ATTR_ATE_ACTION: u32,  set action mode
 * @MTK_NL80211_VENDOR_ATTR_ATE_MCS: u32, set mcs
 * @MTK_NL80211_VENDOR_ATTR_ATE_NSS: u32, set nss
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXSTBC: u32, set stbc
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXMODE: u32, set txmode
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXGI: u32, set tx gi
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXPE: u32, set tx pe
 * @MTK_NL80211_VENDOR_ATTR_ATE_PAYLOAD: u32, set payload
 * @MTK_NL80211_VENDOR_ATTR_ATE_FIXEDPAYLOAD: u32, set fixed payload
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXPOWER: u32, set tx power
 * @MTK_NL80211_VENDOR_ATTR_ATE_CHANNEL: u32, set channel
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXBW: u32, set tx bw
 * @MTK_NL80211_VENDOR_ATTR_ATE_TXANT: u32, tx ant
 * @MTK_NL80211_VENDOR_ATTR_ATE_RXANT: u32, rx ant
 */
enum mtk_nl80211_vendor_attr_ate {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_ATE_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_ATE_ACTION,
	MTK_NL80211_VENDOR_ATTR_ATE_MCS,
	MTK_NL80211_VENDOR_ATTR_ATE_NSS,
	MTK_NL80211_VENDOR_ATTR_ATE_TXSTBC,
	MTK_NL80211_VENDOR_ATTR_ATE_TXMODE,
	MTK_NL80211_VENDOR_ATTR_ATE_TXGI,
	MTK_NL80211_VENDOR_ATTR_ATE_TXPE,
	MTK_NL80211_VENDOR_ATTR_ATE_PAYLOAD,
	MTK_NL80211_VENDOR_ATTR_ATE_FIXEDPAYLOAD,
	MTK_NL80211_VENDOR_ATTR_ATE_TXPOWER,
	MTK_NL80211_VENDOR_ATTR_ATE_CHANNEL,
	MTK_NL80211_VENDOR_ATTR_ATE_TXBW,
	MTK_NL80211_VENDOR_ATTR_ATE_TXANT,
	MTK_NL80211_VENDOR_ATTR_ATE_RXANT,

	__MTK_NL80211_VENDOR_ATTR_ATE_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_ATE_MAX =
		__MTK_NL80211_VENDOR_ATTR_ATE_AFTER_LAST - 1
};

/**
 * enum mtk_vendor_attr_ateaction - This enum defines the value of
 * MTK_NL80211_VENDOR_ATTR_ATE_ACTION.
 * Information in these attributes is used set ate action mode of testmode
 */

enum mtk_vendor_attr_ateaction {
	NL80211_ATE_ACTION_ATESTART,
	NL80211_ATE_ACTION_ATESTOP,
	NL80211_ATE_ACTION_TXCARR,
	NL80211_ATE_ACTION_TXCONT,
	NL80211_ATE_ACTION_TXFRAME,
	NL80211_ATE_ACTION_TXSTOP,
	NL80211_ATE_ACTION_RXFRAME,
	NL80211_ATE_ACTION_RXSTOP,
};

/**
 * enum mtk_nl80211_vendor_attr_v10convertor - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_V10CONVERTER.
 * Information in these attributes is used to v10converter
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_V10CONVERTER: u8, enable/disable v10convertor
 */
enum mtk_nl80211_vendor_attr_v10converter {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_V10CONVERTER_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_V10CONVERTER,

	__MTK_NL80211_VENDOR_ATTR_V10CONVERTER_LAST,
	MTK_NL80211_VENDOR_ATTR_V10CONVERTER_MAX =
	__MTK_NL80211_VENDOR_ATTR_V10CONVERTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_apropxyrefresh - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_APPROXYREFRESH.
 * Information in these attributes is used to approxyrefresh
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SET_APPROXYREFRESH: u8, enable/disable ap proxy refresh
 */
enum mtk_nl80211_vendor_attr_apropxyrefresh {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_SET_APPROXYREFRESH,

	__MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_LAST,
	MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_MAX =
	__MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_LAST - 1
};

/**
* enum mtk_nl80211_vendor_connect_failed_reason - This enum defines
* the fail reason in Authentication procedure.
* This enum is an extension of kernel enum: nl80211_connect_failed_reason
* enum nl80211_connect_failed_reason {
*	NL80211_CONN_FAIL_MAX_CLIENTS,
*	NL80211_CONN_FAIL_BLOCKED_CLIENT,
* };
* which will be reported by kernel API: cfg80211_conn_failed()
*/
enum mtk_nl80211_vendor_connect_failed_reason {
	MTK_NL80211_VENDOR_CONN_FAIL_MAX_CLIENTS,
	MTK_NL80211_VENDOR_CONN_FAIL_BLOCKED_CLIENT,
	MTK_NL80211_VENDOR_CONN_FAIL_DISALLOW_NEW_CLIENT,
	MTK_NL80211_VENDOR_CONN_FAIL_SANITY_FAIL,
	MTK_NL80211_VENDOR_CONN_FAIL_BSSID_NOT_FOUND,
	MTK_NL80211_VENDOR_CONN_FAIL_AP_NOT_READY,
	MTK_NL80211_VENDOR_CONN_FAIL_BSSID_NOT_READY,
	MTK_NL80211_VENDOR_CONN_FAIL_EZ_CONNECT_DISALLOW,
	MTK_NL80211_VENDOR_CONN_FAIL_WDS_WITH_INVALID_PARAM,
	MTK_NL80211_VENDOR_CONN_FAIL_MONITOR_ENTRY,
	MTK_NL80211_VENDOR_CONN_FAIL_AUHT_FLOOD,
	MTK_NL80211_VENDOR_CONN_FAIL_BND_STRG_CHECK_FAIL,
	MTK_NL80211_VENDOR_CONN_FAIL_NOT_FOUND_IN_RADIUS_ACL,
	MTK_NL80211_VENDOR_CONN_FAIL_PMF_REJ_TEMP,
	MTK_NL80211_VENDOR_CONN_FAIL_MLME_NO_RESOURCE,
	MTK_NL80211_VENDOR_CONN_FAIL_UNDEFINED,
	MTK_NL80211_VENDOR_CONN_FAIL_MAX_REASON,
};

/**
 * enum mtk_nl80211_vendor_attr_event_disconnect_sta - Specifies the values for vendor sta disconntct
 * event MTK_NL80211_VENDOR_EVENT_DISC_STA
 * MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_MAC: u8[6], STA mac address,
 * MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_REASON: u16, Disconnect reason,
 * MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_STATISTIC: u8[0], STA statistic info,
 */
enum mtk_nl80211_vendor_attr_event_disconnect_sta {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_MAC,
	MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_REASON,
	MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_STATISTIC,
	__MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_LAST,
	MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_MAX =
	__MTK_NL80211_VENDOR_ATTR_EVENT_DISC_STA_LAST - 1
};

/**
 * enum mtk_nl80211_verndor_disconnect_reason - This enum defines
 * the sta disconnection reason reported from driver.
 */
enum mtk_nl80211_verndor_disconnect_reason {
	MTK_NL80211_VENDOR_DISC_INIT = 0,
	MTK_NL80211_VENDOR_DISC_NO_DATA_ASSOC_TIMEOUT,
	MTK_NL80211_VENDOR_DISC_NO_DATA_TIMEOUT,
	MTK_NL80211_VENDOR_DISC_CONTINUE_TX_FAIL,
	MTK_NL80211_VENDOR_DISC_RSSI_LOW,
	MTK_NL80211_VENDOR_DISC_STA_LEAVE_DISASSOC,
	MTK_NL80211_VENDOR_DISC_STA_LEAVE_DEAUTH,
	MTK_NL80211_VENDOR_DISC_UNDEFINED,
	MTK_NL80211_VENDOR_DISC_MAX_REASON,
};

/**
 * structure e2p_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_E2P_WRITE_PARAM and MTK_NL80211_VENDOR_ATTR_E2P_SHOW_PARAM.
 * Information in this structure is used to get/set mac register information
 * from/to driver.
 *
 * @start: start e2p address
 * @end: end e2p address
 * @value: value for the e2p offset
 */
struct GNU_PACKED e2p_param {
	unsigned int start;
	unsigned int end;
	unsigned int value;
};

/**
 * enum mtk_nl80211_vendor_attrs_e2p - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_E2P.
 * Information in these attributes is used to get/set e2p bufer value
 * from/to driver.
 *
 * @MTK_NL80211_VENDOR_ATTR_E2P_WRITE_PARAM: params, refer to struct GNU_PACKED e2p_param
 * @MTK_NL80211_VENDOR_ATTR_E2P_SHOW_PARAM: params, refer to struct GNU_PACKED e2p_param
 * @MTK_NL80211_VENDOR_ATTR_E2P_DUMP_ALL_PARAM: params, refer to struct GNU_PACKED e2p_param
 * @MTK_NL80211_VENDOR_ATTR_E2P_RSP_STR: RSP string
 */
enum mtk_nl80211_vendor_attrs_e2p {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_E2P_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_E2P_WRITE_PARAM,
	MTK_NL80211_VENDOR_ATTR_E2P_SHOW_PARAM,
	MTK_NL80211_VENDOR_ATTR_E2P_DUMP_ALL_PARAM,
	MTK_NL80211_VENDOR_ATTR_E2P_RSP_STR,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_E2P_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_E2P_MAX = __MTK_NL80211_VENDOR_ATTR_E2P_AFTER_LAST - 1
};

/**
 * structure testengine_param - This structure defines the payload format of
 * MTK_NL80211_VENDOR_ATTR_TESTENGINE_WRITE_PARAM and MTK_NL80211_VENDOR_ATTR_TESTENGINE_SHOW_PARAM.
 * Information in this structure is used to get/set mac register information
 * from/to driver.
 *
 * @at_idx: auto test index
 * @value: value for the set/get
 */
struct GNU_PACKED testengine_param {
	unsigned int atidx;
	unsigned int value;
};


/**
 * enum mtk_nl80211_vendor_attr_testengine - Specifies the vendor attribute values
 * to request wifi info
 *
 * @MTK_NL80211_VENDOR_ATTR_TESTENGINE_SET: struct testengine_param
 * @MTK_NL80211_VENDOR_ATTR_TESTENGINE_GET: struct testengine_param
 */
enum mtk_nl80211_vendor_attr_testengine {
	/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_TESTENGINE_INVALID = 0,

	MTK_NL80211_VENDOR_ATTR_TESTENGINE_SET,
	MTK_NL80211_VENDOR_ATTR_TESTENGINE_GET,
	MTK_NL80211_VENDOR_ATTR_TESTENGINE_RSP_STR,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_TESTENGINE_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_TESTENGINE_MAX =
		__MTK_NL80211_VENDOR_ATTR_TESTENGINE_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_apcli_mld - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_GET_APCLI_MLD.
 * Information in these attributes is used to get information
 * from driver to user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR: command string
 * @MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_RSP_STR: show rsp string buffer
 * @MTK_NL80211_VENDOR_ATTR_DUMP_APCLI_MLD: flag, dump all apcli mlds.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_AP_MLD_MAC: connected AP mld mac address.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_MAC: sation mld mac address.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STAS: nested flags, it wraps
 * multiple affiliated stations information.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STA_MAC: affiliated station mac address.
 * the first one should be the setup link affiliated station.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLMR_ENABLE: emlmr enable status.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLSR_ENABLE: emlst enable status.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_STR_ENABLE: str enable status.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_NSTR_ENABLE: nstr enable status.
 * @MTK_NL80211_VENDOR_ATTR_APCLI_MLD_LINK_MAC: the mac of AP connected by apcli.
 */
enum mtk_nl80211_vendor_attrs_apcli_mld {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR,
	MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_RSP_STR,
	MTK_NL80211_VENDOR_ATTR_DUMP_APCLI_MLD,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_MAC,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_AP_MLD_MAC,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STAS,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STA_MAC,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLMR_ENABLE,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLSR_ENABLE,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_STR_ENABLE,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_NSTR_ENABLE,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_LINK_MAC,
	/* add attributes here, update the policy in nl80211.c */

	__MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_APCLI_MLD_ATTR_MAX =
		__MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFTER_LAST - 1,
	MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_ATTR_MAX = MTK_NL80211_VENDOR_ATTR_APCLI_MLD_ATTR_MAX
};
/**
 * enum mtk_nl80211_vendor_attrs_vendor_show - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_MLO_SWITCH.
 * Information in these attributes is used to get information
 * from driver to user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_SET:parameter, 1=enable, 0=disable
 */
enum mtk_nl80211_vendor_attrs_mlo_switch {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_SET,
	/* add attributes here, update the policy in nl80211.c */

	_MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_AFTER_LAST,
	_MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_ATTR_MAX =
		_MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attr_offload_acs - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_OFFLOAD_ACS.
 * Information in these attributes is used to get information
 * from driver to user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_SET:parameter, 1=enable, 0=disable
 */
enum mtk_nl80211_vendor_attr_offload_acs {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_SET,
	/* add attributes here, update the policy in nl80211.c */

	_MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_ATTR_MAX =
		_MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_acs_hw_mode - Defines HW mode to be used with the
 * vendor command/event MTK_NL80211_VENDOR_EVENT_AUTO_CH_SEL_EVENT.
 *
 * @MTK_ACS_MODE_IEEE80211B: 802.11b mode
 * @MTK_ACS_MODE_IEEE80211G: 802.11g mode
 * @MTK_ACS_MODE_IEEE80211A: 802.11a mode
 * @MTK_ACS_MODE_IEEE80211AD: 802.11ad mode
 * @MTK_ACS_MODE_IEEE80211ANY: all modes
 * @MTK_ACS_MODE_IEEE80211AX: 802.11ax mode
 */
enum mtk_wlan_vendor_acs_hw_mode {
	MTK_ACS_MODE_IEEE80211B,
	MTK_ACS_MODE_IEEE80211G,
	MTK_ACS_MODE_IEEE80211A,
	MTK_ACS_MODE_IEEE80211AD,
	MTK_ACS_MODE_IEEE80211ANY,
};

/**
 * enum mtk_nl80211_vendor_attrs_dfs_slave - This enum defines
 * attributes required for MTK_NL80211_VENDOR_EVENT_BH_STATUS
 * Information in these attributes is used to.send evnt from driver
 * and send cmd from wapp
 */
enum mtk_nl80211_vendor_attrs_dfs_slave {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS,
	/* add attributes here, update the policy in nl80211.c */

	_MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_MAX =
		_MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_AFTER_LAST - 1
};

/**
 * enum mtk_nl80211_vendor_attrs_config - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_CONFIG.
 * @MTK_NL80211_VENDOR_ATTR_CONFIG_FWDL_ACTION: FW download control
 * @MTK_NL80211_VENDOR_ATTR_CONFIG_MAC_ADDR: Config mac address of inerface
 * @MTK_NL80211_VENDOR_ATTR_CONFIG_SET_DEVICE_CMD_STR: Set config command string for per device
 * @MTK_NL80211_VENDOR_ATTR_CONFIG_SET_BAND_CMD_STR: Set config command string for per band
 * @MTK_NL80211_VENDOR_ATTR_CONFIG_SET_BAND_CMD_STR: Set config command string for per wdev
 */
enum mtk_nl80211_vendor_attrs_config {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_CONFIG_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_CONFIG_FWDL_ACTION,
	MTK_NL80211_VENDOR_ATTR_CONFIG_MAC_ADDR,
	MTK_NL80211_VENDOR_ATTR_CONFIG_SET_DEVICE_CMD_STR,
	MTK_NL80211_VENDOR_ATTR_CONFIG_SET_BAND_CMD_STR,
	MTK_NL80211_VENDOR_ATTR_CONFIG_SET_WDEV_CMD_STR,

	__MTK_NL80211_VENDOR_ATTR_CONFIG_LAST,
	MTK_NL80211_VENDOR_ATTR_CONFIG_MAX = __MTK_NL80211_VENDOR_ATTR_CONFIG_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_andlink_event - This enum defines
 * attributes required for MTK_NL80211_VENDOR_EVENT_ANDLINK
 * Information in these attributes is used to.send evnt from driver
 * to usr application
 * @MTK_NL80211_VENDOR_ANDLINK_WIFI_CHANGE_EVENT:send struct andlink_wifi_ch_info
 * to usr application when ssid and password change
 */

enum mtk_nl80211_vendor_andlink_event {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ANDLINK_EVENT_INVALID = 0,

	MTK_NL80211_VENDOR_ANDLINK_WIFI_CHANGE_EVENT,
	_MTK_NL80211_VENDOR_ANDLINK_EVENT_LAST,
	MTK_NL80211_VENDOR_ANDLINK_EVENT_MAX =
		_MTK_NL80211_VENDOR_ANDLINK_EVENT_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_sta_mld - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_GET_CONNECTED_STA_MLD.
 * @MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP: u8, index of AP MLD to dump STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAC: u8[6], MAC address of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLMR: flag, EMLMR of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLSR: flag, EMLSR of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_STR: flag, STR of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_NSTR: flag, NSTR of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST_CONNECT_TIME: u32, Last connect time of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA: flag, Affiliated STA of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_MAC: u8[6], Affiliated STA MAC of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BSSID: u8[6], Affiliated STA BSSID of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LINKID: u8, Affiliated STA link id of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_UL: u8, uplink direction TID map of current affiliated sta.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_DL: u8, donwlink direction TID map of current affiliated sta.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_SENT: u64, Affiliated STA bytes sent of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_RECEIVED: u64, Affiliated STA bytes received of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_SENT: u32, Affiliated STA packets sent of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_RECEIVED:
 * u32, Affiliated STA packets received of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_ERRORS_SENT: u64, Affiliated STA errors sent of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_RETRIES: u32, Affiliated STA retries sent of the connected STA MLD
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SNR: u8, Affiliated STA SNR of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SIGNAL_STRENGTH: s32, Affiliated STA signal strength of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_DL_RATE: u32, Affiliated STA estimated DL rate of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_UL_RATE: u32, Affiliated STA estimated UL rate of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_DL_RATE: u32, Affiliated STA last DL rate of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_UL_RATE: u32, Affiliated STA last UL rate of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_RECEIVE:
 * u64, Affiliated STA air time receive of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_TRANSMIT:
 * u64, Affiliated STA air time transmit of the connected STA MLD.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_PHY_MODE: u8, Affiliated STA uplink phy mode.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_PHY_MODE: u8, Affiliated STA downlink phy mode.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_BW: u8, Affiliated STA uplink bandwidth.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_BW: u8, Affiliated STA downlink bandwidth.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_MCS: u8, Affiliated STA uplink mcs.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_MCS: u8, Affiliated STA downlink mcs.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_NSS: u8, Affiliated STA uplink nss.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_NSS: u8, Affiliated STA downlink nss.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_SGI: u8, Affiliated STA uplink sgi.
 * @MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_SGI: u8, Affiliated STA downlink sgi.
 */
enum mtk_nl80211_vendor_attrs_sta_mld {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAC,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLMR,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLSR,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_STR,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_NSTR,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST_CONNECT_TIME,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_MAC,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BSSID,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LINKID,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_UL,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_DL,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_SENT,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_RECEIVED,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_SENT,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_RECEIVED,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_ERRORS_SENT,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_RETRIES,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SIGNAL_STRENGTH,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SNR,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_DL_RATE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_UL_RATE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_DL_RATE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_UL_RATE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_RECEIVE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_TRANSMIT,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_PHY_MODE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_PHY_MODE,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_BW,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_BW,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_MCS,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_MCS,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_NSS,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_NSS,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_SGI,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_SGI,
	__MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST,
	MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX = __MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_swaci_lna - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_SWACI_LNA.
 * Information in these attributes is used to set AP Power configuration
 * to driver from user application.
 *
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_SCENARIO, Force ACI L/ AACI/ Normal/ ACI small setting shortcut.
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_DUTY_TH, Duty cycle ACI/AACI TH.
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_SCEN_ADC_RCPI_TH, Cond D : ADC ??RCPI th / ADC valid TH (For interf. Scenario decision)
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_ACI_IB_RCPI_TH, Cond E: IB ??RCPI TH (For ACI).
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_AACI_IB_RCPI_TH, Cond F: RCPI TH (For AACI).
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_INST_IB_RCPI_TH, Cond G: InstIB ??RCPI TH.
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_INTERF_CNT_TH, Total Interf Cnt Thr/ ACIL cnt Thr/ AACI cnt Thr.
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_VALID_RCPI_TH, Valid RCPI TH (By BW flexibility)
 * @MTK_NL80211_VENDOR_ATTR_SWACI_LNA_CONF, config values.
 */
enum mtk_nl80211_vendor_attrs_swaci_lna {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_ENABLE,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_SCENARIO,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_DUTY_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_SCEN_ADC_RCPI_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_ACI_IB_RCPI_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_AACI_IB_RCPI_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_INST_IB_RCPI_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_INTERF_CNT_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_VALID_RCPI_TH,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_CONF,
	__MTK_NL80211_VENDOR_ATTR_SWACI_LNA_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_SWACI_LNA_MAX = __MTK_NL80211_VENDOR_ATTR_SWACI_LNA_AFTER_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_npca - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_NPCA.
 * Information in these attributes is used to set NPCA configuration
 * to driver from user application.
 *
 */
enum mtk_nl80211_vendor_attrs_npca {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_NPCA_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_NPCA_GLB_MODE,
	MTK_NL80211_VENDOR_ATTR_NPCA_GLB_THRES,
	MTK_NL80211_VENDOR_ATTR_NPCA_GLB_CH_SWT,
	MTK_NL80211_VENDOR_ATTR_NPCA_GLB_BYPASS,
	MTK_NL80211_VENDOR_ATTR_NPCA_GLB_SRAM_ADDR,
	MTK_NL80211_VENDOR_ATTR_NPCA_GLB_SRAM_COLOR,
	MTK_NL80211_VENDOR_ATTR_NPCA_STA_COLOR,
	MTK_NL80211_VENDOR_ATTR_NPCA_STA_ADDR,
	__MTK_NL80211_VENDOR_ATTR_NPCA_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_NPCA_MAX = __MTK_NL80211_VENDOR_ATTR_NPCA_AFTER_LAST - 1
};
/**
 * enum mtk_nl80211_vendor_attrs_dso - This enum defines
 * attributes required for MTK_NL80211_VENDOR_SUBCMD_SET_DSO.
 * Information in these attributes is used to set DSO configuration
 * to driver from user application.
 *
 */
enum mtk_nl80211_vendor_attrs_dso {
/* don't change the order or add anything between, this is ABI! */
	MTK_NL80211_VENDOR_ATTR_DSO_INVALID = 0,
	MTK_NL80211_VENDOR_ATTR_DSO_GLB_EN,
	MTK_NL80211_VENDOR_ATTR_DSO_STA_CAP,
	__MTK_NL80211_VENDOR_ATTR_DSO_AFTER_LAST,
	MTK_NL80211_VENDOR_ATTR_DSO_MAX = __MTK_NL80211_VENDOR_ATTR_DSO_AFTER_LAST - 1
};

#endif /* __MTK_VENDOR_NL80211_H */
