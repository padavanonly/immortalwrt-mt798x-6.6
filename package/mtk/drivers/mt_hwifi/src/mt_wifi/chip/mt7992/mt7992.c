/*
 * Copyright (c) [2022], MediaTek Inc. All rights reserved.
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

#ifdef MT7992

#include "rt_config.h"
#include "mt7992_cr.h"
#include "mac_mt/fmac/mt_fmac.h"
#include "mac_mt/bmac/mt_bmac.h"
#include "mt7992_coda/WF_RX_DESCRIPTOR_BESRA_c_header.h"
#include "mt7992_coda/WF_TX_DESCRIPTOR_BESRA_c_header.h"
#include "mt7992_coda/wf_wfdma_mem_dma.h"
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
#include <modules/npu/wifi_mail.h>
#endif


/* iPAiLNA shall always be included as default */
#include "eeprom/mt7992_e2p_iPAiLNA.h"

#ifdef CONFIG_CPE_SUPPORT
#include "eeprom/mt7992_e2p_ePAeLNA.h"
#endif

#if defined(CONFIG_FIRST_IF_EPAELNA) || defined(CONFIG_SECOND_IF_EPAELNA) || defined(CONFIG_THIRD_IF_EPAELNA)
#include "eeprom/mt7992_e2p_ePAeLNA.h"
#endif
#if defined(CONFIG_FIRST_IF_IPAELNA) || defined(CONFIG_SECOND_IF_IPAELNA) || defined(CONFIG_THIRD_IF_IPAELNA)
#include "eeprom/mt7992_e2p_iPAeLNA.h"
#endif
#if defined(CONFIG_FIRST_IF_EPAILNA) || defined(CONFIG_SECOND_IF_EPAILNA) || defined(CONFIG_THIRD_IF_EPAILNA)
#include "eeprom/mt7992_e2p_ePAiLNA.h"
#endif

#define MT7992_WIFI_RAM_BAND_NUM 2
#ifdef DOT11V_MBSSID_SUPPORT
#define MT7992_MAX_TRANSMITTED_NUM 16 /* per band support */
#define MT7992_TRANSMITTED_BSS_BITMAP 0x0000FFFF
#endif /* DOT11V_MBSSID_SUPPORT */

/* interrupt pcie 0 */
/* MT7992 TODO: to support wed or not? */
#define HOST_DMA0_INT_STA_SOURCE \
WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR
#define HOST_DMA0_INT_STA_ENA_ADDR \
WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR
#define HOST_DMA0_INT_STA_TX_COHERENT \
WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_coherent_int_sts_MASK
#define HOST_DMA0_INT_STA_RX_COHERENT \
WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_coherent_int_sts_MASK
#ifdef HW_TX_BYPASS_WA
#define HOST_DMA0_INT_STA_TX_DONE \
(WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_1_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_2_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_3_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_4_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_17_MASK)
#else /* #ifdef HW_TX_BYPASS_WA */
#define HOST_DMA0_INT_STA_TX_DONE \
(WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_17_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_18_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_19_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_20_MASK)
#endif /* !#ifdef HW_TX_BYPASS_WA */
#define HOST_DMA0_INT_STA_RX_DATA_DONE \
(WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK)
#ifdef HW_TX_BYPASS_WA
#define HOST_DMA0_INT_STA_RX_EVENT_DONE \
(WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_0_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK\
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK)
#else /* HW_TX_BYPASS_WA */
#define HOST_DMA0_INT_STA_RX_EVENT_DONE \
(WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_0_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_1_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_2_MASK \
| WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_3_MASK)
#endif /* !HW_TX_BYPASS_WA */
#define HOST_DMA0_INT_STA_SUBSYS \
WF_WFDMA_HOST_DMA0_HOST_INT_STA_subsys_int_sts_MASK
#define HOST_DMA0_INT_STA_MCU2HOST_SW \
WF_WFDMA_HOST_DMA0_HOST_INT_STA_mcu2host_sw_int_sts_MASK

/* onwership control */
#define HOST_SET_FW_OWN_MASK \
CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_SET_FW_OWN_HS_PULSE_MASK
#define HOST_CLR_FW_OWN_MASK \
CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_CLR_FW_OWN_HS_PULSE_MASK
#define HOST_FW_OWN_SYNC_MASK \
CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_OWNER_STATE_SYNC_MASK
#define HOST_FW_OWN_CLR_STAT_MASK \
CONN_HOST_CSR_TOP_WF_BAND0_IRQ_STAT_WF_B0_HOST_LPCR_FW_OWN_CLR_STAT_MASK

#define HOST_DMA0_MCU2HOST_SW_INT_DRV_OWN \
WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_ENA_mcu2host_int_ena_0_MASK

typedef enum _RXV_CONTENT_CAT {
	RXV_CONTENT_CMN1 = 0,
	RXV_CONTENT_USR1,
	RXV_CONTENT_USR2,
	RXV_CONTENT_CMN2,
	RXV_CONTENT_NUM
} RXV_CONTENT_CAT, *P_RXV_CONTENT_CAT;

typedef enum _RXV_DUMP_LIST_TYPE {
	RXV_DUMP_LIST_TYPE_CONTENT = 0,
	RXV_DUMP_LIST_TYPE_BASIC_ENTRY,
	RXV_DUMP_LIST_TYPE_ENTRY,
	RXV_DUMP_LIST_TYPE_NUM
} RXV_DUMP_LIST_TYPE, *P_RXV_DUMP_LIST_TYPE;

struct MT7992_IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC3_L;
	UINT8 ucC4_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC3_M;
	UINT8 ucC4_M;
	UINT8 ucC0_MH;
	UINT8 ucC1_MH;
	UINT8 ucC2_MH;
	UINT8 ucC3_MH;
	UINT8 ucC4_MH;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC3_H;
	UINT8 ucC4_H;
	UINT8 ucC0_UH;
	UINT8 ucC1_UH;
	UINT8 ucC2_UH;
	UINT8 ucC3_UH;
	UINT8 ucC4_UH;
};

struct MT7992_IBF_PHASE_G0_T {
	UINT8 ucG0_R0_UH;
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_MH;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R0_UL;
	UINT8 ucG0_R1_UH;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_MH;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R1_UL;
	UINT8 ucG0_R2_UH;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_MH;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R2_UL;
	UINT8 ucG0_R3_UH;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_MH;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_R3_UL;
	UINT8 ucG0_M_T0_H;
	UINT8 ucG0_M_T1_H;
	UINT8 ucG0_M_T2_H;
};

struct MT7992_IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_UH;
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_MH;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R0_UL;
	UINT8 ucGx_R1_UH;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_MH;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R1_UL;
	UINT8 ucGx_R2_UH;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_MH;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R2_UL;
	UINT8 ucGx_R3_UH;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_MH;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R3_UL;
	UINT8 ucGx_R4_UH;
	UINT8 ucGx_R4_H;
	UINT8 ucGx_R4_MH;
	UINT8 ucGx_R4_M;
	UINT8 ucGx_R4_L;
	UINT8 ucGx_R4_UL;
	UINT8 ucGx_M_T0_H;
	UINT8 ucGx_M_T1_H;
	UINT8 ucGx_M_T2_H;
	UINT8 ucGx_M_T3_H;
};

/*
	0x0_xxxx	RW	0x1840_xxxx
				ex. 0x0_2000 --> 0x1840_2000
				    0x0_3000 --> 0x1840_3000
	0x1_xxxx	RW	0x7403_xxxx
	0x2_xxxx	RW	0x1842_xxxx
	0x3_xxxx	RW	0x1843_xxxx
	0x4_xxxx	RW	0x1844_xxxx
	0x5_xxxx	RW	0x1845_xxxx
	0x6_xxxx	RW	0x1846_xxxx
	0x7_xxxx	RW	0x1847_xxxx WF_UMCA_SYSRAM
	0x8_xxxx	RW	0x1848_xxxx WF_MCU_SYSRAM
	0x9_xxxx	RW	0x1849_xxxx WF_MCU_SYSRAM
	0xa_xxxx	RW	0x184a_xxxx
	0xb_xxxx	RW	0x184b_xxxx
	0xc_xxxx	RW	0x184c_xxxx
	0xd_xxxx	RW	0x1802_xxxx
	0xe_xxxx	RW	0x1806_xxxx
	0xf_xxxx	RO	0x1800_xxxx
*/
const struct PCIE_CHIP_CR_MAPPING mt7992_mac_cr_range[] = {
	{0x54000000, 0x02000, 0x1000}, /* WFDMA_0 (PCIE0 MCU DMA0) */
	{0x55000000, 0x03000, 0x1000}, /* WFDMA_1 (PCIE0 MCU DMA1) */
	{0x56000000, 0x04000, 0x1000}, /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000}, /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000}, /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x1000}, /* WF_UMAC_TOP (PP) */
	{0x74030000, 0x10000, 0x1000}, /* PCIe MAC */
	{0x820e0000, 0x20000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */

	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */

	{0x820f0000, 0xa0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0xa5000, 0x2000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0xa8000, 0x4000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820b0000, 0xae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */

	{0x0, 0x0, 0x0}, /* imply end of search */
};

/*
 * Used for DNL Calibration happen in MP-Line state
 * 3 Channel for G-band L/M/H channel
 * 16 Channel for A-band L/H channel
 */
uint16_t MT7992_DNL_CAL_GBAND_BW20_FREQ[] = {2412, 2442, 2472};
uint16_t MT7992_DNL_CAL_ABAND_BW20_FREQ[] = {
	4960, 5060,/* Group 1 */
	5180, 5240,/* Group 2 */
	5260, 5320,/* Group 3 */
	5340, 5480,/* Group 4 */
	5500, 5560,/* Group 5 */
	5580, 5640,/* Group 6 */
	5660, 5720,/* Group 7 */
	5785, 5845 /* Group 8 */};
uint16_t MT7992_DNL_CAL_BW20_FREQ[] = {
	2412, 2442, 2472,
	4960, 5060, 5180, 5240, 5260, 5320, 5340, 5480,
	5500, 5560, 5580, 5640, 660, 5720, 5785, 5845};

uint16_t MT7992_DNL_CAL_GBAND_BW20_CH[] = {1, 7, 13};
uint16_t MT7992_DNL_CAL_ABAND_BW20_CH[] = {
	192,  12,  36,  48,  52,  64,  68,  96,
	100, 112, 116, 128, 132, 144, 157, 169};
uint16_t MT7992_DNL_CAL_BW20_CH[] = {
	1,   7,  13, 192,  12,  36,  48,  52,  64,  68,
	96, 100, 112, 116, 128, 132, 144, 157, 169};

uint16_t MT7992_DNL_CAL_GBAND_BW20_SIZE =
	(sizeof(MT7992_DNL_CAL_GBAND_BW20_FREQ) / sizeof(uint16_t));
uint16_t MT7992_DNL_CAL_ABAND_BW20_SIZE =
	(sizeof(MT7992_DNL_CAL_ABAND_BW20_FREQ) / sizeof(uint16_t));
uint16_t MT7992_DNL_CAL_BW20_FREQ_SIZE  =
	(sizeof(MT7992_DNL_CAL_BW20_FREQ) / sizeof(uint16_t));

uint16_t MT7992_DNL_CAL_GBAND_BW20_CH_SIZE =
	(sizeof(MT7992_DNL_CAL_GBAND_BW20_CH) / sizeof(uint16_t));
uint16_t MT7992_DNL_CAL_ABAND_BW20_CH_SIZE =
	(sizeof(MT7992_DNL_CAL_ABAND_BW20_CH) / sizeof(uint16_t));
uint16_t MT7992_DNL_CAL_B20_CH_SIZE        =
	(sizeof(MT7992_DNL_CAL_BW20_CH) / sizeof(uint16_t));

static struct rtmp_spe_map mt7992_spe_map[] = {
	/* All */
	{0x0, 0},
	{0xf, 0},
	/* 1 Ant */
	{0x1, 0},	/* Tx0 */
	{0x2, 1},	/* Tx1 */
	{0x4, 3},	/* Tx2 */
	{0x8, 9},	/* Tx3 */
	/* 2 Ant */
	{0x3, 0},
	{0x5, 2},
	{0x9, 8},
	{0x6, 4},
	{0xa, 6},
	{0xc, 16},
	/* 3 Ant */
	{0x7, 0},	/* 0_1_2 */
	{0xb, 10},	/* 0_1_3 */
	{0xd, 12},	/* 0_2_3 */
	{0xe, 18},	/* 1_2_3 */
};

#ifdef PRE_CAL_MT7992_SUPPORT
UINT16 MT7992_PER_CH_A5_BW20_FREQ[] = {
	5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
	5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
	5660, 5680, 5700, 5720, 5745, 5765, 5785, 5805,
	5825, 5845, 5865, 5885};

UINT16 MT7992_PER_CH_A5_BW80_FREQ[] = {
	5210, 5290, 5530, 5610, 5690, 5775,
	5855};

UINT16 MT7992_PER_CH_A5_BW160_FREQ[] = {
	5250, 5570, 5815};

UINT16 MT7992_PER_CH_A5_BW20_BW80_BW160_FREQ[] = {
	5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
	5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
	5660, 5680, 5700, 5720, 5745, 5765, 5785, 5805,
	5825, 5845, 5865, 5885,
	5210, 5290, 5530, 5610, 5690, 5775,
	5855,
	5250, 5570, 5815};

UINT16 MT7992_PER_CH_A5_BW20[] = {
	36, 40, 44, 48, 52, 56, 60, 64,
	100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 144, 149, 153, 157, 161,
	165, 169, 173, 177};

UINT16 MT7992_PER_CH_A5_BW80[] = {
	42, 58, 106, 122, 138, 155,
	171};

UINT16 MT7992_PER_CH_A5_BW160[] = {
	 50, 114, 163};

UINT16 MT7992_PER_CH_A5_BW20_BW80_BW160[] = {
	36, 40, 44, 48, 52, 56, 60, 64,
	100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 144, 149, 153, 157, 161,
	165, 169, 173, 177,
	42, 58, 106, 122, 138, 155,
	171,
	50, 114, 163};

UINT16 MT7992_PER_CH_A6_BW20_FREQ[] = {
	5955, 5975, 5995, 6015, 6035, 6055, 6075, 6095,
	6115, 6135, 6155, 6175, 6195, 6215, 6235, 6255,
	6275, 6295, 6315, 6335, 6355, 6375, 6395, 6415,
	6435, 6455, 6475, 6495, 6515, 6535, 6555, 6575,
	6595, 6615, 6635, 6655, 6675, 6695, 6715, 6735,
	6755, 6775, 6795, 6815, 6835, 6855, 6875, 6895,
	6915, 6935, 6955, 6975, 6995, 7015, 7035, 7055,
	7075, 7095, 7115};

UINT16 MT7992_PER_CH_A6_BW80_FREQ[] = {
	5985, 6065, 6145, 6225, 6305, 6385, 6465, 6545,
	6625, 6705, 6785, 6865, 6945, 7025};

UINT16 MT7992_PER_CH_A6_BW160_FREQ[] = {
	6025, 6185, 6345, 6505, 6665, 6825, 6985};

UINT16 MT7992_PER_CH_A6_BW20_BW80_BW160_FREQ[] = {
	5955, 5975, 5995, 6015, 6035, 6055, 6075, 6095,
	6115, 6135, 6155, 6175, 6195, 6215, 6235, 6255,
	6275, 6295, 6315, 6335, 6355, 6375, 6395, 6415,
	6435, 6455, 6475, 6495, 6515, 6535, 6555, 6575,
	6595, 6615, 6635, 6655, 6675, 6695, 6715, 6735,
	6755, 6775, 6795, 6815, 6835, 6855, 6875, 6895,
	6915, 6935, 6955, 6975, 6995, 7015, 7035, 7055,
	7075, 7095, 7115,
	5985, 6065, 6145, 6225, 6305, 6385, 6465, 6545,
	6625, 6705, 6785, 6865, 6945, 7025,
	6025, 6185, 6345, 6505, 6665, 6825, 6985};

UINT16 MT7992_PER_CH_A6_BW20[] = {
	1, 5, 9, 13, 17, 21, 25, 29,
	33, 37, 41, 45, 49, 53, 57, 61,
	65, 69, 73, 77, 81, 85, 89, 93,
	97, 101, 105, 109, 113, 117, 121, 125,
	129, 133, 137, 141, 145, 149, 153, 157,
	161, 165, 169, 173, 177, 181, 185, 189,
	193, 197, 201, 205, 209, 213, 217, 221,
	225, 229, 233};

UINT16 MT7992_PER_CH_A6_BW80[] = {
	7, 23, 39, 55, 71, 87, 103, 119,
	135, 151, 167, 183, 199, 215};

UINT16 MT7992_PER_CH_A6_BW160[] = {
	15, 47, 79, 111, 143, 175, 207};

UINT16 MT7992_PER_CH_A6_BW20_BW80_BW160[] = {
	1, 5, 9, 13, 17, 21, 25, 29,
	33, 37, 41, 45, 49, 53, 57, 61,
	65, 69, 73, 77, 81, 85, 89, 93,
	97, 101, 105, 109, 113, 117, 121, 125,
	129, 133, 137, 141, 145, 149, 153, 157,
	161, 165, 169, 173, 177, 181, 185, 189,
	193, 197, 201, 205, 209, 213, 217, 221,
	225, 229, 233,
	7, 23, 39, 55, 71, 87, 103, 119,
	135, 151, 167, 183, 199, 215,
	15, 47, 79, 111, 143, 175, 207};

UINT16 MT7992_PER_CH_G_BW20_FREQ[] = {
	2422, 2442, 2462};

UINT16 MT7992_PER_CH_G_BW20[] = {
	3, 7, 11};

UINT16 MT7992_PER_CH_A5_BW20_BW80_BW160_FREQ_SIZE = sizeof(MT7992_PER_CH_A5_BW20_BW80_BW160_FREQ) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A6_BW20_BW80_BW160_FREQ_SIZE = sizeof(MT7992_PER_CH_A6_BW20_BW80_BW160_FREQ) / sizeof(UINT16);
UINT16 MT7992_PER_CH_G_BW20_FREQ_SIZE  = (sizeof(MT7992_PER_CH_G_BW20_FREQ) / sizeof(UINT16));

UINT16 MT7992_PER_CH_A5_BW20_BW80_BW160_SIZE = sizeof(MT7992_PER_CH_A5_BW20_BW80_BW160) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A6_BW20_BW80_BW160_SIZE = sizeof(MT7992_PER_CH_A6_BW20_BW80_BW160) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A5_BW20_SIZE =  sizeof(MT7992_PER_CH_A5_BW20) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A6_BW20_SIZE =  sizeof(MT7992_PER_CH_A6_BW20) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A5_BW80_SIZE =  sizeof(MT7992_PER_CH_A5_BW80) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A6_BW80_SIZE =  sizeof(MT7992_PER_CH_A6_BW80) / sizeof(UINT16);
UINT16 MT7992_PER_CH_G_BW20_SIZE    = sizeof(MT7992_PER_CH_G_BW20) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A5_BW160_SIZE =  sizeof(MT7992_PER_CH_A5_BW160) / sizeof(UINT16);
UINT16 MT7992_PER_CH_A6_BW160_SIZE =  sizeof(MT7992_PER_CH_A6_BW160) / sizeof(UINT16);


#endif /* PRE_CAL_MT7992_SUPPORT */

UINT8 MT7992_SINGLE_SKU_FILL_TABLE_LENGTH[] = {
	SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1
};

#define HIF_PORT_MAX	2
const UINT32 hif_ownsership_cr[HIF_PORT_MAX][OWNERSHIP_CR_TYPE_NUM] = {
	{CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_ADDR, CONN_HOST_CSR_TOP_WF_BAND0_IRQ_STAT_ADDR},
	{CONN_HOST_CSR_TOP_WF_BAND1_LPCTL_ADDR, CONN_HOST_CSR_TOP_WF_BAND1_IRQ_STAT_ADDR},
};

#ifdef TXBF_SUPPORT
#define MT7992_MAX_BFEE_SS 4 /* max nr for bfee */
#endif

static UINT32 mt7992_get_mib_bcn_tx_cnt(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;

	switch (band_idx) {
	case BAND1:
		MAC_IO_READ32(pAd->hdev_ctrl,
			BN1_WF_MIB_TOP_TSCR18_BEACONTXCOUNT_ADDR, &value);
		break;
	default:
		MAC_IO_READ32(pAd->hdev_ctrl,
			BN0_WF_MIB_TOP_TSCR18_BEACONTXCOUNT_ADDR, &value);
		break;
	}

	return (value & BN0_WF_MIB_TOP_TSCR18_BEACONTXCOUNT_MASK);
}

#ifdef PRE_CAL_MT7992_SUPPORT
void mt7992_apply_dpd_flatness_data(
	RTMP_ADAPTER          *pAd,
	MT_SWITCH_CHANNEL_CFG SwChCfg
	)
{
	USHORT			doCal1 = 0;
	UINT8			i = 0;
	UINT8			Band = 0;
	UINT16			CentralFreq = 0;
	UINT8			sendNum = 0;
	UINT8			ofst = 0;
	UINT16			Freqofst = 0;
	UINT16			upper_bound = 0;
	UINT32			eeprom_ofst = 0;
	UINT32			BW80_160_ofst = 0;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) != E2P_FLASH_MODE
		&& PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) != E2P_BIN_MODE) {

		return;
	}

	chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"---------------------DoCAL(%x) Band(%x)---------------------\n",
		doCal1, pAd->BandSel);

	if ((((doCal1 & (1 << DPD5G_PRECAL_INDN_BIT)) != 0) && (pAd->BandSel == BAND_SELECT_5G || pAd->BandSel == BAND_SELECT_5GLOW || pAd->BandSel == BAND_SELECT_5GHIGH)) ||
			(((doCal1 & (1 << DPD2G_PRECAL_INDN_BIT)) != 0) && (pAd->BandSel == BAND_SELECT_2G)) ||
			(((doCal1 & (1 << DPD6G_PRECAL_INDN_BIT)) != 0) && (pAd->BandSel == BAND_SELECT_6G || pAd->BandSel == BAND_SELECT_6GLOW || pAd->BandSel == BAND_SELECT_6GHIGH))) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"DPD Pre-Cal finished, load DPD Pre-Cal data\n");
		if (SwChCfg.CentralChannel == 14) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					 "CH 14 don't need DPD , return!!!\n");
			return;
		} else if ((SwChCfg.CentralChannel < 14) && (SwChCfg.Channel_Band == 0)) {
			Band = GBAND;

			if (SwChCfg.CentralChannel >= 1 && SwChCfg.CentralChannel <= 4)
				CentralFreq = 2422;
			else if (SwChCfg.CentralChannel >= 5 && SwChCfg.CentralChannel <= 9)
				CentralFreq = 2442;
			else if (SwChCfg.CentralChannel >= 10 && SwChCfg.CentralChannel <= 13)
				CentralFreq = 2462;
			else
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						 "can't find cent freq for CH %d , should not happen!!!\n",
						  SwChCfg.CentralChannel);
		} else {
			Band = ABAND;

			if (pAd->BandSel == BAND_SELECT_5G || pAd->BandSel == BAND_SELECT_5GLOW || pAd->BandSel == BAND_SELECT_5GHIGH)
				Freqofst = 5000; /* 5G */
			else
				Freqofst = 5950; /* 6G */
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"--------------Freqofst  %d-----------\n", Freqofst);

			if (SwChCfg.Bw == BW_20) {
				CentralFreq = SwChCfg.CentralChannel * 5 + Freqofst;
			} else if (SwChCfg.Bw == BW_80) {
				CentralFreq = SwChCfg.CentralChannel * 5 + Freqofst;
			} else if (SwChCfg.Bw == BW_160) {
				CentralFreq = SwChCfg.CentralChannel * 5 + Freqofst;
			} else {
				UINT32 Central = SwChCfg.CentralChannel * 5 + Freqofst;
				UINT32 CentralMinus10M = (SwChCfg.CentralChannel - 2) * 5 + Freqofst;

				if (ChannelFreqToGroup(Central) != ChannelFreqToGroup(CentralMinus10M)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
						"==== Different Group Central %d @ group %d Central-10 @ group %d !!\n",
						Central, ChannelFreqToGroup(Central), ChannelFreqToGroup(CentralMinus10M));
					CentralFreq = (SwChCfg.CentralChannel + 2) * 5 + Freqofst;
				} else
					CentralFreq = (SwChCfg.CentralChannel - 2) * 5 + Freqofst;
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"eeprom 0x%2x bit 0 is 0, do runtime cal DOCAL%x BANDSELECT%x\n",
			PRECAL_INDICATION_BYTE, doCal1, pAd->BandSel);
		return;
	}

	/* Find if CentralFreq is exist in DPD+Flatness pre-k table */
	if (Band == GBAND) {/* 2G */
		upper_bound = MT7992_PER_CH_G_BW20_FREQ_SIZE;
		eeprom_ofst = cap->prek_ee_info.dpd_flash_offset_g_begin;
		for (i = 0; i < upper_bound; i++) {
			if (MT7992_PER_CH_G_BW20_FREQ[i] == CentralFreq) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"%d is in DPD-Flatness cal table, index = %d\n",
					CentralFreq, i);
				break;
			}
		}
	} else {
		/* 5G */
		if (pAd->BandSel == BAND_SELECT_5G || pAd->BandSel == BAND_SELECT_5GLOW || pAd->BandSel == BAND_SELECT_5GHIGH) {
			upper_bound = MT7992_PER_CH_A5_BW20_BW80_BW160_FREQ_SIZE;
			eeprom_ofst = cap->prek_ee_info.dpd_flash_offset_a5_begin;
			for (i = 0; i < upper_bound; i++) {
				if (MT7992_PER_CH_A5_BW20_BW80_BW160_FREQ[i] == CentralFreq) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
						"%d is in DPD-Flatness cal table, index = %d\n",
						CentralFreq, i);
					break;
				}
			}
		} else {/* 6G */
			upper_bound = MT7992_PER_CH_A6_BW20_BW80_BW160_FREQ_SIZE;
			eeprom_ofst = cap->prek_ee_info.dpd_flash_offset_a6_begin;
			for (i = 0; i < upper_bound; i++) {
				if (MT7992_PER_CH_A6_BW20_BW80_BW160_FREQ[i] == CentralFreq) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
						"%d is in DPD-Flatness cal table, index = %d\n",
						CentralFreq, i);
					break;
				}
			}
		}
	}

	/* upper bound check */
	if (i == upper_bound) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Unexpected freq (%d)\n", CentralFreq);
		return;
	}
	if (Band == GBAND)
		sendNum = (cap->prek_ee_info.per_legaycy_ch_cal_size +  cap->prek_ee_info.per_ch_cal_OTFG0_size) / PRE_CAL_SET_MAX_LENGTH;
	else {
		if (SwChCfg.Bw == BW_80 ||  SwChCfg.Bw == BW_160)
			sendNum = (cap->prek_ee_info.per_mem_ch_cal_size + cap->prek_ee_info.per_ch_cal_OTFG0_size) / PRE_CAL_SET_MAX_LENGTH;
		else
			sendNum = (cap->prek_ee_info.per_legaycy_ch_cal_size + cap->prek_ee_info.per_ch_cal_OTFG0_size) / PRE_CAL_SET_MAX_LENGTH;
	}
	while (ofst < sendNum) {
		if (Band == GBAND)
			MtCmdSetDpdFlatnessCal_7992(pAd, i * sendNum + ofst, PRE_CAL_SET_MAX_LENGTH, eeprom_ofst, TX_DPD_FLATNESS_CAL);
		else if (((SwChCfg.Bw == BW_80) || (SwChCfg.Bw == BW_160)) && (pAd->BandSel == BAND_SELECT_5G || pAd->BandSel == BAND_SELECT_5GLOW || pAd->BandSel == BAND_SELECT_5GHIGH)) {
			BW80_160_ofst = (cap->prek_ee_info.dpd_cal_5g_legacy_size / PRE_CAL_SET_MAX_LENGTH) + (i - MT7992_PER_CH_A5_BW20_SIZE) * sendNum;
			MtCmdSetDpdFlatnessCal_7992(pAd, BW80_160_ofst + ofst, PRE_CAL_SET_MAX_LENGTH, eeprom_ofst, TX_DPD_FLATNESS_CAL_A5_MEM);
		} else if (((SwChCfg.Bw == BW_80) || (SwChCfg.Bw == BW_160)) && (pAd->BandSel == BAND_SELECT_6G || pAd->BandSel == BAND_SELECT_6GLOW || pAd->BandSel == BAND_SELECT_6GHIGH)) {
			BW80_160_ofst = (cap->prek_ee_info.dpd_cal_6g_legacy_size / PRE_CAL_SET_MAX_LENGTH) + (i - MT7992_PER_CH_A6_BW20_SIZE) * sendNum;
			MtCmdSetDpdFlatnessCal_7992(pAd, BW80_160_ofst + ofst, PRE_CAL_SET_MAX_LENGTH, eeprom_ofst, TX_DPD_FLATNESS_CAL_A6_MEM);
		} else if ((SwChCfg.Bw < BW_160) && (pAd->BandSel == BAND_SELECT_5G || pAd->BandSel == BAND_SELECT_5GLOW || pAd->BandSel == BAND_SELECT_5GHIGH)) {
			MtCmdSetDpdFlatnessCal_7992(pAd, i * sendNum + ofst, PRE_CAL_SET_MAX_LENGTH, eeprom_ofst, TX_DPD_FLATNESS_CAL_A5);
		} else {
			MtCmdSetDpdFlatnessCal_7992(pAd, i * sendNum + ofst, PRE_CAL_SET_MAX_LENGTH, eeprom_ofst, TX_DPD_FLATNESS_CAL_A6);
		}
		ofst++;
	}
}

#endif /* PRE_CAL_MT7992_SUPPORT */

#ifndef MOBILE_FW_SUPPORT
static void switch_channel(
	RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR cent_ch;

	cap->channelbw = SwChCfg.Bw;

#ifdef DOT11_EHT_BE
	/* For BW320, pass eht_cen_ch to FW*/
	if (SwChCfg.Bw == BW_320)
		cent_ch = SwChCfg.eht_cen_ch;
	else
#endif /* DOT11_EHT_BE */
		cent_ch = SwChCfg.CentralChannel;

#ifdef SINGLE_SKU_V2
	/* update power limit table */
	MtPwrLimitTblChProc(pAd, SwChCfg.BandIdx, SwChCfg.Channel_Band,
		SwChCfg.ControlChannel, SwChCfg.CentralChannel);
#endif
#ifdef ZERO_PKT_LOSS_SUPPORT
	pAd->chan_switch_time[9] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/

#ifdef PRE_CAL_MT7992_SUPPORT
	mt7992_apply_dpd_flatness_data(pAd, SwChCfg);
#endif

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		UniCmdChannelSwitch(pAd, SwChCfg);
#ifdef ZERO_PKT_LOSS_SUPPORT
		pAd->chan_switch_time[10] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
		UniCmdSetTxRxPath(pAd, SwChCfg);
#ifdef ZERO_PKT_LOSS_SUPPORT
		pAd->chan_switch_time[11] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
	} else {
#endif /* WIFI_UNIFIED_COMMAND */
	MtCmdChannelSwitch(pAd, SwChCfg);
	MtCmdSetTxRxPath(pAd, SwChCfg);
#ifdef WIFI_UNIFIED_COMMAND
	}
#endif /* WIFI_UNIFIED_COMMAND */
	if (!SwChCfg.bScan)
		EDCCAInit(pAd, SwChCfg.BandIdx);

	pAd->LatchRfRegs.Channel = SwChCfg.CentralChannel;
}
#endif

#ifdef NEW_SET_RX_STREAM
static INT set_RxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 path = 0;
	UINT i;

	if (StreamNums > 4) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				 "illegal StreamNums(%d)\n", StreamNums);
		StreamNums = 4;
	}

	for (i = 0; i < StreamNums; i++)
		path |= 1 << i;

	return MtCmdSetRxPath(pAd, path, BandIdx);
}
#endif

static inline VOID bufferModeDataFill(RTMP_ADAPTER *pAd, union _EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd, uint16_t addr)
{
	UINT32 i = pCmd->v2.ucCount;
	struct physical_device *ph_dev = pAd->physical_dev;

	pCmd->v2.BinContent[i] = ph_dev->EEPROMImage[addr];
	pCmd->v2.ucCount++;
}


static VOID bufferModeCmdFill(RTMP_ADAPTER *pAd, union _EXT_CMD_EFUSE_BUFFER_MODE_T *cmd, uint16_t ctrl_msg)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	uint16_t addr = 0, page_size = 0x400;
	uint8_t total_page = 0, page_idx = 0, format = 0;

	format = ((ctrl_msg & 0xff00) >> 8);

	cmd->v2.ucSourceMode = (ctrl_msg & 0xff);
	cmd->v2.ucContentFormat = format;
	cmd->v2.ucCount = 0;

	if (ctrl_msg & EEPROM_MODE_BUFFER) {
		total_page = (format & BUFFER_BIN_TOTAL_PAGE_MASK) >> BUFFER_BIN_TOTAL_PAGE_SHIFT;
		page_idx = (format & BUFFER_BIN_PAGE_INDEX_MASK) >> BUFFER_BIN_PAGE_INDEX_SHIFT;

		if (page_idx == total_page) {
			if ((cap->EFUSE_BUFFER_CONTENT_SIZE % 0x400) != 0)
				page_size = (cap->EFUSE_BUFFER_CONTENT_SIZE % 0x400);
		}

		for (addr = 0; addr < page_size; addr++)
			bufferModeDataFill(pAd, cmd, (addr + (page_idx*0x400)));
	}

	cmd->v2.ucCount = cpu2le16(cmd->v2.ucCount);
}

static UINT32 mt7992_get_sku_decision(RTMP_ADAPTER *ad)
{
	UINT32 sku_type = MT7992_SKU_INVALID;
	UINT32 adie_num = MT7992_ADIE_MULTI;
	UINT32 adie_type = 0;
	UINT32 chip_id = 0;
	UINT16 value = 0;

	RTMP_IO_READ32(ad->hdev_ctrl, MT7992_CBTOP_GPIO_ADIE_STRAP_ADDR, &adie_num);

	adie_num = (adie_num & MT7992_CBTOP_GPIO_ADIE_NUM_STRAP_MASK) >>
		MT7992_CBTOP_GPIO_ADIE_NUM_STRAP_SHIF;

	if (adie_num == MT7992_ADIE_SINGLE) {
		UniCmdRFRegAccessRead(ad, ADIE0_RFIDX, CHIP_ID_OFFSET, &chip_id);
		rtmp_ee_efuse_read16(ad, 0x470, &value);
		if ((chip_id == MT7976C_CHIP_ID) || (chip_id == MT7976C_HL_CHIP_ID) ||
			(chip_id == MT7976C_PS_CHIP_ID) || (value == 0xc)) {
			sku_type = MT7992_SKU_BE5040_MT7976C;
			MTWF_PRINT("\033[1;33mSKU: #%d.MT7992-BE5040-7976C\033[0m\n", sku_type);
		} else {
			sku_type = MT7992_SKU_BE5040_MT7976DA;
			MTWF_PRINT("\033[1;33mSKU: #%d.MT7992-BE5040-7976DA\033[0m\n", sku_type);
		}
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, MT7992_CBTOP_GPIO_ADIE_STRAP_ADDR, &adie_type);
		adie_type = (adie_type & MT7992_CBTOP_GPIO_ADIE_COMBATION_STRAP_MASK) >>
			MT7992_CBTOP_GPIO_ADIE_COMBATION_STRAP_SHIF;
		if (adie_type == MT7992_ADIE_78_79) {
			sku_type = MT7992_SKU_BE6500_MT7978_MT7979;
			MTWF_PRINT("\033[1;33mSKU: #%d.MT7992-BE6500-7978-7979\033[0m\n", sku_type);
		} else if (adie_type == MT7992_ADIE_75_7779) {
			UniCmdRFRegAccessRead(ad, ADIE1_RFIDX, CHIP_ID_OFFSET, &chip_id);
			chip_id = (chip_id & ADIE1_CHIP_ID_MASK);
			if (chip_id == MT7979_CHIP_ID) {
				sku_type = MT7992_SKU_BE7200_MT7975_MT7979;
				MTWF_PRINT("\033[1;33mSKU: #%d.MT7992-BE7200-7975-7979\033[0m\n",
					sku_type);
			} else {
				sku_type = MT7992_SKU_BE7200_MT7975_MT7977;
				MTWF_PRINT("\033[1;33mSKU: #%d.MT7992-BE7200-7975-7977\033[0m\n",
					sku_type);
			}
		} else if (adie_type == MT7992_ADIE_76_77) {
			sku_type = MT7992_SKU_BE7200_MT7976G_MT7977;
			MTWF_PRINT("\033[1;33mSKU: #%d.MT7992-BE7200-7976G-7977\033[0m\n", sku_type);
		}
	}
	return sku_type;
}

#ifdef CAL_FREE_IC_SUPPORT

static struct _kfree_def adie_def_7975[] = {
	{12, {0x5cd, 0x5cf, 0x5d1, 0x5d3, 0x6c0, 0x6c1, 0x6c2, 0x6c3, 0x7a1, 0x7a6,
		0x7a8, 0x7aa}
	},
	/* adie0 efuse merge to eeprom offset list */
	{12, {0x451, 0x453, 0x455, 0x457, 0x44c, 0x44d, 0x44e, 0x44f, 0xba1, 0xba6,
		0xba8, 0xbaa}
	}
};

static struct _kfree_def adie_def_7976[] = {
	/* adie efuse offset with FT result.
	   The other indicate that adie0 efuse merge to eeprom offset list */
	{17, {0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x53, 0x55, 0x57, 0x59,
		0x790, 0x791, 0x794, 0x795, 0x7a6, 0x7a8, 0x7aa}
	},
	/* adie0 efuse merge to eeprom offset list */
	{17, {0x44c, 0x44d, 0x44e, 0x44f, 0x450, 0x451, 0x453, 0x455, 0x457, 0x459,
		0xb90, 0xb91, 0xb94, 0xb95, 0xba6, 0xba8, 0xbaa}
	}
};

static struct _kfree_def adie_def_7978[] = {
	/* adie efuse offset with FT result.
	   The other indicate that adie0 efuse merge to eeprom offset list */
	{14, {0x91, 0x95, 0x100, 0x102, 0x104, 0x106, 0x107, 0x108, 0x109,
		0x10a, 0x10b, 0x10c, 0x10e, 0x110}
	},
	/* adie0 efuse merge to eeprom offset list */
	{14, {0xb91, 0xb95, 0x480, 0x482, 0x484, 0x486, 0x487, 0x488, 0x489,
		0x48a, 0x48b, 0x48c, 0x48e, 0x490}
	}
};

static struct _kfree_def adie_def_7977[] = {
	/* adie efuse offset with FT result.
	   The other indicate that adie0 efuse merge to eeprom offset list */
	{17, {0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x53, 0x55, 0x57, 0x59,
		0x69, 0x6a, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e}
	},
	/* adie1 efuse merge to eeprom offset list */
	{17, {0x124c, 0x124d, 0x124e, 0x124f, 0x1250, 0x1251, 0x1253, 0x1255, 0x1257, 0x1259,
		0x1269, 0x126a, 0x127a, 0x127b, 0x127c, 0x127d, 0x127e}
	}
};

static struct _kfree_def adie_def_7979[] = {
	/* adie efuse offset with FT result.
	   The other indicate that adie0 efuse merge to eeprom offset list */
	{17, {0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x53, 0x55, 0x57, 0x59,
		0x69, 0x6a, 0x7a, 0x7b, 0x7c, 0x7e, 0x80}
	},
	/* adie1 efuse merge to eeprom offset list */
	{17, {0x124c, 0x124d, 0x124e, 0x124f, 0x1250, 0x1251, 0x1253, 0x1255, 0x1257, 0x1259,
		0x1269, 0x126a, 0x127a, 0x127b, 0x127c, 0x127e, 0x1280}
	}
};

static inline uint8_t check_valid(RTMP_ADAPTER *pAd, uint16_t Offset)
{
	uint16_t Value = 0;
	uint8_t NotValid, ret = FALSE;

	if ((Offset % 2))
		Offset -= 1;

	NotValid = rtmp_ee_efuse_read16(pAd, Offset, &Value);

	if (NotValid)
		ret = FALSE;
	else
		ret = TRUE;

	return ret;
}

static BOOLEAN is_cal_free_ic_check(RTMP_ADAPTER *pAd,
	struct _kfree_def *adie_kfree_def, UINT32 adie_offset)
{
	UINT8 kfree_cnt = 0, loop = 0;
	//UINT16 Value = 0;
	BOOLEAN ret = FALSE;

	kfree_cnt = adie_kfree_def->count;

	for (loop = 0; loop < kfree_cnt; loop++)
		if (check_valid(pAd, adie_kfree_def->offsets[loop] + adie_offset) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				 "\x1b[41m%s 0x%x invalid, iCal merge dismissed!\x1b[m\n",
				  __func__, adie_kfree_def->offsets[loop]);
			goto err_out;
		}

	ret = TRUE;

err_out:
	return ret;
}

static uint8_t is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	uint8_t ret = FALSE;
	struct _kfree_def *adie_kfree_def = NULL;
	uint32_t sku = 0;

	sku = mt7992_get_sku_decision(pAd);

	if ((sku == MT7992_SKU_BE5040_MT7976C) ||
		(sku == MT7992_SKU_BE5040_MT7976DA)) {
		adie_kfree_def = &adie_def_7976[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE0_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				 "\x1b[41m SKU:BE5040, ADIE:MT7976C, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}
	} else if (sku == MT7992_SKU_BE7200_MT7975_MT7979) {
		adie_kfree_def = &adie_def_7975[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE0_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE7200, ADIE:MT7975, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}

		adie_kfree_def = &adie_def_7979[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE1_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:E7200, ADIE:MT7979, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}
	} else if (sku == MT7992_SKU_BE6500_MT7978_MT7979) {
		adie_kfree_def = &adie_def_7978[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE0_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE6500, ADIE:MT7978, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}

		adie_kfree_def = &adie_def_7979[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE1_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE6500, ADIE:MT7979, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}
	} else if (sku == MT7992_SKU_BE7200_MT7976G_MT7977) {
		adie_kfree_def = &adie_def_7976[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE0_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE7200, ADIE:MT7976G, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}

		adie_kfree_def = &adie_def_7977[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE1_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE7200, ADIE:MT7977, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}
	} else if (sku == MT7992_SKU_BE7200_MT7975_MT7977) {
		adie_kfree_def = &adie_def_7975[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE0_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE7200, ADIE:MT7975, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}

		adie_kfree_def = &adie_def_7977[0];
		ret = is_cal_free_ic_check(pAd, adie_kfree_def, MT7992_ADIE1_OFFSET);
		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
				"\x1b[41m SKU:BE7200, ADIE:MT7977, iCal merge dismissed! \x1b[m\n");
			goto err_out;
		}
	}
	/* else panta band ? */

	ret = TRUE;

err_out:
	return ret;
}

static inline VOID cal_free_data_get_from_addr(RTMP_ADAPTER *ad,
		UINT16 adie_offset, UINT16 ee_offset)
{
	uint16_t value = 0;
	struct physical_device *ph_dev = ad->physical_dev;

	if ((adie_offset % 2) != 0) {
		rtmp_ee_efuse_read16(ad, adie_offset - 1, &value);
		ph_dev->EEPROMImage[ee_offset] = ((value >> 8) & 0xFF);
	} else {
		rtmp_ee_efuse_read16(ad, adie_offset, &value);
		ph_dev->EEPROMImage[ee_offset] = (value & 0xFF);
	}
	MTWF_DBG(ad, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_DEBUG,
		"\t\t adie_offset: 0x%x Replace: 0x%x=0x%02x\n", adie_offset,
		ee_offset, ph_dev->EEPROMImage[ee_offset]);
}


static VOID cal_free_data_get(RTMP_ADAPTER *ad)
{
	uint8_t kfree_cnt = 0, loop = 0;
	struct _kfree_def *adie_kfree_def = NULL, *ee_kfree_def = NULL;
	//struct physical_device *ph_dev = ad->physical_dev;
	uint32_t sku = 0, adie_cnt = 0, adie_idx = 0, adie_offset = 0;

	MTWF_DBG(ad, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_INFO, "%s\n", __func__);

	/* A-Die merge */
	sku = mt7992_get_sku_decision(ad);

	if (sku == MT7992_SKU_BE5040_MT7976C || sku == MT7992_SKU_BE5040_MT7976DA)
		adie_cnt = 1;
	else
		adie_cnt = 2;

	for (adie_idx = 0; adie_idx < adie_cnt; adie_idx++) {
		/* adie0 merge */
		if (adie_idx == 0 && (
			sku == MT7992_SKU_BE5040_MT7976C ||
			sku == MT7992_SKU_BE5040_MT7976DA ||
			sku == MT7992_SKU_BE7200_MT7976G_MT7977)) {
			adie_kfree_def = &adie_def_7976[0];
			ee_kfree_def = &adie_def_7976[1];
			adie_offset = MT7992_ADIE0_OFFSET;
		} else if (adie_idx == 0 && (
			sku == MT7992_SKU_BE7200_MT7975_MT7979 ||
			sku == MT7992_SKU_BE7200_MT7975_MT7977)) {
			adie_kfree_def = &adie_def_7975[0];
			ee_kfree_def = &adie_def_7975[1];
			adie_offset = MT7992_ADIE0_OFFSET;
		} else if (adie_idx == 0 && (
			sku == MT7992_SKU_BE6500_MT7978_MT7979)) {
			adie_kfree_def = &adie_def_7978[0];
			ee_kfree_def = &adie_def_7978[1];
			adie_offset = MT7992_ADIE0_OFFSET;
		}
		/* adie1 merge */
		if (adie_idx == 1 && (
			sku == MT7992_SKU_BE7200_MT7975_MT7979 ||
			sku == MT7992_SKU_BE6500_MT7978_MT7979)) {
			adie_kfree_def = &adie_def_7979[0];
			ee_kfree_def = &adie_def_7979[1];
			adie_offset = MT7992_ADIE1_OFFSET;
		} else if (adie_idx == 1 && (
			sku == MT7992_SKU_BE7200_MT7975_MT7977 ||
			sku == MT7992_SKU_BE7200_MT7976G_MT7977)) {
			adie_kfree_def = &adie_def_7977[0];
			ee_kfree_def = &adie_def_7977[1];
			adie_offset = MT7992_ADIE1_OFFSET;
		}
		/* else panta band ? */

		MTWF_DBG(ad, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_DEBUG,
			"adie_idx:%d, adie_offset:0x%x\n", adie_idx, adie_offset);

		kfree_cnt = adie_kfree_def->count;
		for (loop = 0; loop < kfree_cnt; loop++) {
			cal_free_data_get_from_addr(ad,
				adie_kfree_def->offsets[loop] + adie_offset,
				ee_kfree_def->offsets[loop]);
		}
	}

	MTWF_DBG(ad, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR, "\n");
}

#endif /* CAL_FREE_IC_SUPPORT */

static VOID init_mac_cr(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG, "\n");
}

static VOID BBPInit(RTMP_ADAPTER *pAd)
{
	uint8_t band_vld[2];
	INT idx, cbw[2] = {0};
	INT cent_ch[2] = {0}, prim_ch[2] = {0}, prim_ch_idx[2] = {0};
	INT band[2] = {0};
	INT txStream[2] = {0};
	UCHAR use_bands;

	band_vld[0] = TRUE;
	cbw[0] = RF_BW_20;
	cent_ch[0] = 1;
	prim_ch[0] = 1;
	band[0] = BAND_24G;
	txStream[0] = 2;
#ifdef DOT11_VHT_AC
	prim_ch_idx[0] = vht_prim_ch_idx(cent_ch[0], prim_ch[0], cbw[0]);
#endif /* DOT11_VHT_AC */

	if (pAd->CommonCfg.dbdc_mode) {
		band_vld[1] = TRUE;
		band[1] = BAND_5G;
		cbw[1] = RF_BW_20;
		cent_ch[1] = 36;
		prim_ch[1] = 36;
#ifdef DOT11_VHT_AC
		prim_ch_idx[1] = vht_prim_ch_idx(cent_ch[1], prim_ch[1], cbw[1]);
#endif /* DOT11_VHT_AC */
		txStream[1] = 2;
		use_bands = 2;
	} else {
		band_vld[1] = FALSE;
		use_bands = 1;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			 "BBP Initialization.....\n");

	for (idx = 0; idx < 2; idx++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
				 "\tBand %d: valid=%d, isDBDC=%d, Band=%d, CBW=%d, CentCh/PrimCh=%d/%d, prim_ch_idx=%d, txStream=%d\n",
				  idx, band_vld[idx], pAd->CommonCfg.dbdc_mode, band[idx], cbw[idx], cent_ch[idx], prim_ch[idx],
				  prim_ch_idx[idx], txStream[idx]);
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "todo\n");
}

static void antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	USHORT value = 0;
	struct mcs_nss_caps *mcs_nss = MCS_NSS_CAP(pAd);
	UINT8 band_idx = 0, i, boundary_idx = MAX_MULTI_STA;
	struct wifi_dev *wdev = NULL;

	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7992;
	wdev = pAd->wdev_list[0];

	if (wdev != NULL) {
		band_idx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "band idx=%u\n", band_idx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "wdev is null\n");
		return;
	}

#ifndef TXBF_SUPPORT
	pAntenna->field.TxPath = 4;
	pAntenna->field.RxPath = 4;
#else
	switch (band_idx) {
	case 0:
		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_1, value);
		value &= 0xFF;
		pAntenna->field.TxPath = (value & TX_MASK_B0) >> TX_OFFSET_B0;
		mcs_nss->max_path[MAX_PATH_TX] = pAntenna->field.TxPath;

		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_3, value);
		value &= 0xFF;
		pAntenna->field.RxPath = (value & RX_MASK_B0) >> RX_OFFSET_B0;
		mcs_nss->max_path[MAX_PATH_RX] = pAntenna->field.RxPath;

		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_4, value);
		value &= 0xFF;
		mcs_nss->max_nss = (value & STREAM_MASK_B0) >> STREAM_OFFSET_B0;
		break;

	case 1:
		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_2, value);
		value &= 0xFF;
		pAntenna->field.TxPath = (value & TX_MASK_B1) >> TX_OFFSET_B1;
		mcs_nss->max_path[MAX_PATH_TX] = pAntenna->field.TxPath;

		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_3, value);
		value &= 0xFF;
		pAntenna->field.RxPath = (value & RX_MASK_B1) >> RX_OFFSET_B1;
		mcs_nss->max_path[MAX_PATH_RX] = pAntenna->field.RxPath;

		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_5, value);
		value &= 0xFF;
		mcs_nss->max_nss = (value & STREAM_MASK_B1) >> STREAM_OFFSET_B1;
		break;

	case 2:
		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_2, value);
		value &= 0xFF;
		pAntenna->field.TxPath = (value & TX_MASK_B2) >> TX_OFFSET_B2;
		mcs_nss->max_path[MAX_PATH_TX] = pAntenna->field.TxPath;

		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_4, value);
		value &= 0xFF;
		pAntenna->field.RxPath = (value & RX_MASK_B2) >> RX_OFFSET_B2;
		mcs_nss->max_path[MAX_PATH_RX] = pAntenna->field.RxPath;

		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_5, value);
		value &= 0xFF;
		mcs_nss->max_nss = (value & STREAM_MASK_B2) >> STREAM_OFFSET_B2;

		break;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"value = 0x%x, TxPath = %d, RxPath = %d\n",
		value, pAntenna->field.TxPath, pAntenna->field.RxPath);
#endif /* TXBF_SUPPORT */
#if defined(CONFIG_AP_SUPPORT)
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		boundary_idx = pAd->ApCfg.BssidNum;
#endif

	for (i = 0; i < boundary_idx; i++) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i < MAX_MBSSID_NUM(pAd))
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (i < MAX_MULTI_STA)
				wdev = &pAd->StaCfg[i].wdev;
		}
#endif /*CONFIG_STA_SUPPORT*/
		if (wdev) {
			wlan_config_set_tx_stream(wdev,
				min(wlan_config_get_tx_stream(wdev),
				mcs_nss->max_nss));
			wlan_config_set_rx_stream(wdev,
				min(wlan_config_get_rx_stream(wdev),
				mcs_nss->max_nss));
#ifdef DOT11_HE_AX
			wlan_config_set_he_tx_nss(wdev,
				min(wlan_config_get_he_tx_nss(wdev),
				mcs_nss->max_nss));
			wlan_config_set_he_rx_nss(wdev,
				min(wlan_config_get_he_rx_nss(wdev),
				mcs_nss->max_nss));
#endif
#ifdef DOT11_EHT_BE
			wlan_config_set_eht_tx_nss(wdev,
				min(wlan_config_get_eht_tx_nss(wdev),
				mcs_nss->max_nss));
			wlan_config_set_eht_rx_nss(wdev,
				min(wlan_config_get_eht_rx_nss(wdev),
				mcs_nss->max_nss));
#endif
		}
	}
}

static UINT32 mt7992_get_band_select_from_eeprom(
	struct _RTMP_ADAPTER *pAd)
{
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	USHORT value = 0;

	switch (band_idx) {
	case 0:
		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET, value);
		value &= 0xFF;
		pAd->BandSel = (value & BAND_SELECT_MASK_B0) >> BAND_SELECT_OFFSET_B0;
		break;

	case 1:
		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET, value);
		value &= 0xFF;
		pAd->BandSel = (value & BAND_SELECT_MASK_B1) >> BAND_SELECT_OFFSET_B1;
		break;

	case 2:
		RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET_1, value);
		value &= 0xFF;
		pAd->BandSel = (value & BAND_SELECT_MASK_B2) >> BAND_SELECT_OFFSET_B2;
		break;

	default:
		MTWF_PRINT("%s(): band_idx:%d error\n", __func__, band_idx);
		pAd->BandSel = BAND_SELECT_NA;
		break;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"band_idx:%d, band_sel:%d\n", band_idx, pAd->BandSel);

	return pAd->BandSel;
}

static UINT32 mt7992_get_rx_stat_all(
	struct _RTMP_ADAPTER *pAd,
	uint8_t band_idx,
	uint8_t blk_idx,
	struct _TEST_RX_STAT_ALL_INFO *rx_all_info
)
{
	if (rx_all_info != NULL)
		os_zero_mem(rx_all_info, sizeof(struct _TEST_RX_STAT_ALL_INFO));

	if (rx_all_info == NULL)
		return TRUE;

	mt_cmd_get_rx_stat_all(pAd, band_idx, rx_all_info);

	return FALSE;
}

static UINT32 mt7992_get_rx_stat(
	struct _RTMP_ADAPTER *pAd,
	UCHAR band_idx,
	P_TESTMODE_STATISTIC_INFO ptest_mode_stat_info)
{
	if (mt_cmd_get_rx_stat(pAd, band_idx,
			ptest_mode_stat_info) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	return FALSE;
}

#ifdef CONFIG_WLAN_SERVICE
static INT32 mt7992_get_chip_capability(
	struct _RTMP_ADAPTER *pAd, struct test_capability *chip_cap)
{
	uint32_t sku = 0;
	UINT16 BandSel = 0;
	RTMP_ADAPTER *ad = NULL;
	struct mcs_nss_caps *mcs_nss;

	/*Follow format of struct test_capability*/
	chip_cap->version = GET_CAPABILITY_VER;
	chip_cap->tag_num = GET_CAPABILITY_TAG_NUM;
	chip_cap->ph_cap.tag = GET_CAPABILITY_TAG_PHY;
	chip_cap->ph_cap.tag_len = GET_CAPABILITY_TAG_PHY_LEN;
	chip_cap->ph_cap.ant_num = 5;
	chip_cap->ext_cap.tag = GET_CAPABILITY_TAG_PHY_EXT;
	chip_cap->ext_cap.tag_len = GET_CAPABILITY_TAG_PHY_EXT_LEN;
	chip_cap->ph_cap.phy_adie_index |= BITS(0, 1);

	sku = mt7992_get_sku_decision(pAd);

	/* adie idx */
	if (sku == MT7992_SKU_BE5040_MT7976C || sku == MT7992_SKU_BE5040_MT7976DA)
		chip_cap->ph_cap.phy_adie_index |= BIT8;
	else
		chip_cap->ph_cap.phy_adie_index |= BITS(8, 9);

	/* b0 */
	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, BAND0);
	if (ad) {
		mcs_nss = MCS_NSS_CAP(ad);
		/* BIT[7:0]: Band0 TX path num */
		/* BIT[15:8]: Band0 RX path num */
		chip_cap->ph_cap.band_0_1_wf_path_num |=
				(BITS(0, 7) & mcs_nss->max_path[MAX_PATH_TX]) |
				(BITS(8, 15) & (mcs_nss->max_path[MAX_PATH_RX] << 8));

		RTMP_GET_BAND_SELECT(ad, &BandSel);
		/* BIT[7:0]: Band0 system bandwidth */
		if (BandSel == BAND_SELECT_2G)
			chip_cap->ph_cap.band_bandwidth |= BITS(0, 1);
	}

	/* b1 */
	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, BAND1);
	if (ad) {
		mcs_nss = MCS_NSS_CAP(ad);
	    /* BIT[23:16]: Band1 TX path num */
		/* BIT[31:24]: Band1 RX path num */
		chip_cap->ph_cap.band_0_1_wf_path_num |=
			(BITS(16, 23) & ((UINT32)mcs_nss->max_path[MAX_PATH_TX]  << 16)) |
			(BITS(24, 31) & ((UINT32)mcs_nss->max_path[MAX_PATH_RX]) << 24);

		RTMP_GET_BAND_SELECT(ad, &BandSel);
		/* BIT[15:8]: Band1 system bandwidth */
		if (BandSel == BAND_SELECT_5G || BandSel == BAND_SELECT_5GLOW ||
				BandSel == BAND_SELECT_5GHIGH)
			chip_cap->ph_cap.band_bandwidth |= (BITS(0, 3) << 8);
	}


	/* bandwidth duplicate mode */
	chip_cap->ext_cap.feature1 |= BIT7;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"%s(): ph_cap.phy_adie_index:       %x, bitwise phy: %x, adie: %x\n", __func__,
		chip_cap->ph_cap.phy_adie_index,
		(UINT32)(chip_cap->ph_cap.phy_adie_index & BITS(0, 7)),
		(UINT32)((chip_cap->ph_cap.phy_adie_index & BITS(8, 15)) >> 8));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"%s(): ph_cap.band_0_1_wf_path_num: %x, bn0: %x\n", __func__,
		chip_cap->ph_cap.band_0_1_wf_path_num,
		(UINT32)(chip_cap->ph_cap.band_0_1_wf_path_num & BITS(0, 15)));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"%s(): ph_cap.band_0_1_wf_path_num: %x  bn1: %x\n", __func__,
		chip_cap->ph_cap.band_0_1_wf_path_num,
		(UINT32)((chip_cap->ph_cap.band_0_1_wf_path_num & BITS(16, 31)) >> 16));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"%s(): ph_cap.band_2_3_wf_path_num: %x, bn2: %x\n", __func__,
		chip_cap->ph_cap.band_2_3_wf_path_num,
		(UINT32)(chip_cap->ph_cap.band_2_3_wf_path_num & BITS(0, 15)));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"%s(): ph_cap.band_bandwidth:       %x\n", __func__, chip_cap->ph_cap.band_bandwidth);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"%s(): ext_cap.feature1:            %x\n", __func__, chip_cap->ext_cap.feature1);


	return FALSE;
}
#endif /* CONFIG_WLAN_SERVICE */


static uint8_t mt7992_check_RF_lock_down(RTMP_ADAPTER *pAd)
{
	BOOL RFlockDown = FALSE;
	struct physical_device *ph_dev = pAd->physical_dev;

	if (ph_dev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"ph_dev is NULL and return\n");
		return TRUE;
	}

	if (ph_dev->EEPROMImage == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"ph_dev->EEPROMImage is NULL and return\n");
		return TRUE;
	}

	if (ph_dev->EEPROMImage[RF_LOCKDOWN_EEPROME_OFFSET]
			& RF_LOCKDOWN_EEPROME_MASK)
		RFlockDown = TRUE;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
		"RFlockDown Enable: %d\n", RFlockDown);

	return RFlockDown;
}

static UINT32 mt7992_get_sku_tbl_idx(
	RTMP_ADAPTER *pAd, uint8_t *sku_tbl_idx)
{
	uint8_t tbl_idx = 0;
	uint8_t rf_lock = FALSE;
	struct physical_device *ph_dev = pAd->physical_dev;

	/* read profile for table index */
	tbl_idx = pAd->CommonCfg.SKUTableIdx;

	/* check rf lockdown status*/
	rf_lock = chip_check_rf_lock_down(pAd);

	/* update sku table index */
	if (rf_lock)
		tbl_idx = (ph_dev->EEPROMImage[RF_LOCKDOWN_EEPROME_SKU_TBL_OFFSET]
				& RF_LOCKDOWN_EEPROME_SKU_TBL_MASK)
					>> RF_LOCKDOWN_EEPROME_SKU_TBL_SHIFT;

	/* update sku table index */
	*sku_tbl_idx = tbl_idx;

	return 0;
}

static uint8_t mt7992_Config_Effuse_Country(RTMP_ADAPTER *pAd)
{
	UCHAR   Buffer0, Buffer1;
	UCHAR   CountryCode[2];
	struct physical_device *ph_dev = pAd->physical_dev;

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Region 2G */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Buffer0 = ph_dev->EEPROMImage[COUNTRY_REGION_2G_EEPROME_OFFSET];

	/* Check the RF lock status */
	if (Buffer0 != 0xFF) {
		/* Check Validation bit for content */
		if (((Buffer0) & (COUNTRY_REGION_VALIDATION_MASK))
				>> (COUNTRY_REGION_VALIDATION_OFFSET))
			pAd->CommonCfg.CountryRegion =
				((Buffer0) & (COUNTRY_REGION_CONTENT_MASK));
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Region 5G */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Buffer1 = ph_dev->EEPROMImage[COUNTRY_REGION_5G_EEPROME_OFFSET];

	/* Check the RF lock status */
	if (Buffer1 != 0xFF) {
		/* Check Validation bit for content */
		if (((Buffer1) & (COUNTRY_REGION_VALIDATION_MASK))
				>> (COUNTRY_REGION_VALIDATION_OFFSET))
			pAd->CommonCfg.CountryRegionForABand =
				((Buffer1) & (COUNTRY_REGION_CONTENT_MASK));
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Code */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	CountryCode[0] = ph_dev->EEPROMImage[COUNTRY_CODE_BYTE0_EEPROME_OFFSET];
	CountryCode[1] = ph_dev->EEPROMImage[COUNTRY_CODE_BYTE1_EEPROME_OFFSET];

	/* Check the RF lock status */
	if ((CountryCode[0] != 0xFF) && (CountryCode[1] != 0xFF)) {
		/* Check Validation for content */
		if ((CountryCode[0] != 0x00) && (CountryCode[1] != 0x00)) {
			pAd->CommonCfg.CountryCode[0] = CountryCode[0];
			pAd->CommonCfg.CountryCode[1] = CountryCode[1];
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
				"pAd->CommonCfg.CountryCode[0]: 0x%x, %c ",
				pAd->CommonCfg.CountryCode[0],
				pAd->CommonCfg.CountryCode[0]);
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
				"pAd->CommonCfg.CountryCode[1]: 0x%x, %c ",
				pAd->CommonCfg.CountryCode[1],
				pAd->CommonCfg.CountryCode[1]);
		}
	}

	return TRUE;
}

static UINT32 mt7992_rxv_entry_hdr_parser(
	struct _RTMP_ADAPTER *pAd,
	VOID *rxv_entry,
	RXV_ENTRY_HDR *rxv_entry_hdr)
{
	UINT32 *ptr = NULL;

	if (!rxv_entry_hdr)
		goto error0;

	if (!rxv_entry)
		goto error1;

	ptr = (UINT32 *) rxv_entry;

	/* rxv pkt header parsing */
	rxv_entry_hdr->rx_sta_airtime = (*(ptr) & BITS(0, 15)) >> 0;
	rxv_entry_hdr->rx_sta_cnt = (*(ptr) & BITS(16, 22)) >> 16;
	rxv_entry_hdr->rxv_sn = (*(ptr + 1) & BITS(16, 23)) >> 16;
	rxv_entry_hdr->band_idx = (*(ptr + 1) & BIT(24)) >> 24;
	rxv_entry_hdr->rx_airtime_cal = (*(ptr + 1) & BIT(25)) >> 25;
	rxv_entry_hdr->tr = (*(ptr + 1) & BIT(26)) >> 26;
	rxv_entry_hdr->trig_mpdu = (*(ptr + 1) & BIT(27)) >> 27;

	rxv_entry_hdr->time_stamp = 0;
	rxv_entry_hdr->time_stamp |= ((*(ptr) & BITS(2, 10)) >> 2) << 0;
	rxv_entry_hdr->time_stamp |= ((*(ptr + 1) & BITS(0, 15)) >> 0) << 9;
	rxv_entry_hdr->time_stamp |= ((*(ptr + 1) & BITS(28, 31)) >> 28) << 25;

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for rxv pkt header.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for rxv pkt.\n");
	return 1;
}

static UINT32 mt7992_rxv_pkt_hdr_parser(
	struct _RTMP_ADAPTER *pAd, VOID *rxv_pkt, RXV_PKT_HDR *rxv_pkt_hdr)
{
	UINT32 *ptr = NULL;

	if (!rxv_pkt_hdr)
		goto error0;

	if (!rxv_pkt)
		goto error1;

	ptr = (UINT32 *) rxv_pkt;

	/* rxv pkt header parsing */
	rxv_pkt_hdr->rx_byte_cnt = (*(ptr) & BITS(0, 15)) >> 0;
	rxv_pkt_hdr->rxv_cnt = (*(ptr) & BITS(16, 20)) >> 16;
	rxv_pkt_hdr->pkt_type = (*(ptr) & BITS(29, 31)) >> 29;
#ifdef CFG_BIG_ENDIAN
	if ((rxv_pkt_hdr->rx_byte_cnt - 4) > 0)
		MTMacInfoEndianChange(pAd, (UCHAR *)ptr, TYPE_RMACINFO,
			rxv_pkt_hdr->rx_byte_cnt);
#endif

	rxv_pkt_hdr->pse_fid = (*(ptr + 1) & BITS(16, 27)) >> 16;

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for rxv pkt header.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for rxv pkt.\n");
	return 1;
}

static UINT32 mt7992_rxv_get_byte_cnt(
	struct _RTMP_ADAPTER *pAd,
	uint8_t band_idx,
	UINT32 *byte_cnt)
{
	uint32_t *ptr = NULL;

	/* read rxv pakcet raw data */

	/* sanity check for null pointer */
	if (!ptr)
		goto error;

	/* read byte count */
	/* *byte_cnt = ((*ptr) & BITS(0, 15)) >> 0; */

error:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"(rxv) null pointer for rxv byte cnt\n");
	return TRUE;
}

static UINT32 mt7992_rxv_get_content(
	struct _RTMP_ADAPTER *pAd,
	uint8_t band_idx,
	PVOID *content)
{
	uint32_t *ptr = NULL;

	/* read rxv pakcet raw data */

	/* sanity check for null pointer */
	if (!ptr)
		goto error;

error:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"(rxv) null pointer for rxv data\n");
	return TRUE;
}

static UINT32 mt7992_rxv_raw_data_show(
	struct _RTMP_ADAPTER *pAd, uint8_t band_idx)
{
	uint32_t *ptr = NULL, *ptr2 = NULL;
	uint16_t dw_cnt = 0, dw_idx = 0, rxv_cnt = 0, rxv_byte_cnt = 0;
	uint8_t dw_line_cnt = 4;

	/* read rxv pakcet raw data */
	ptr = (uint32_t *)(pAd->rxv_raw_data.rxv_pkt);

	/* sanity check for null pointer */
	if (!ptr) {
		MTWF_PRINT("%s(): null pointer for rxv raw dara.\n", __func__);
		return FALSE;
	}

	ptr2 = ptr;
	rxv_cnt = ((*ptr2) & BITS(16, 20)) >> 16;
	rxv_byte_cnt = ((*ptr2) & BITS(0, 15)) >> 0;

	MTWF_PRINT("%s: rxv_cnt: %d, rxv_byte_cnt: %d\n", __func__,
		rxv_cnt, rxv_byte_cnt);

	dw_cnt = rxv_byte_cnt >> 2;
	if (rxv_byte_cnt % 4 != 0)
		dw_cnt++;

	for (dw_idx = 0, ptr2 = ptr; dw_idx < dw_cnt; dw_idx++, ptr2++) {
		MTWF_PRINT("[0x%04X]:%08X  ", dw_idx, *ptr2);

		if ((dw_idx % dw_line_cnt) == (dw_line_cnt - 1))
			MTWF_PRINT("\n");
	}
	MTWF_PRINT("\n");

	return TRUE;
}

static UINT32 mt7992_rxv_info_show(
	struct _RTMP_ADAPTER *pAd, uint8_t band_idx)
{
	return FALSE;
}

static UINT32 mt7992_rxv_stat_reset(
	struct _RTMP_ADAPTER *pAd, uint8_t band_idx)
{
	/* free memory for rxv pkt */
	os_zero_mem(pAd->rxv_raw_data.rxv_pkt, pAd->rxv_raw_data.rxv_byte_cnt);
	os_free_mem(pAd->rxv_raw_data.rxv_pkt);
	pAd->rxv_raw_data.rxv_pkt = NULL;
	pAd->rxv_raw_data.rxv_byte_cnt = 0;

	/* reset rxv count */
	pAd->rx_stat_rxv.rxv_cnt = 0;

	/* TODO: reset rssi */

	return 0;
}

static UINT32 mt7992_rxv_cap_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (IS_MT7992_FW_VER_E1(pAd)) {
		cap->rxv_pkt_hdr_dw_num = E1_RXV_PACKET_HEADER_DW_NUM;
		cap->rxv_entry_hdr_dw_num = E1_RXV_ENTRY_HEADER_DW_NUM;
		cap->rxv_cmn1_dw_num = E1_CMN_RXV1_DW_NUM;
		cap->rxv_cmn2_dw_num = E1_CMN_RXV2_DW_NUM;
		cap->rxv_usr1_dw_num = E1_USR_RXV1_DW_NUM;
		cap->rxv_usr2_dw_num = E1_USR_RXV2_DW_NUM;
	} else if (IS_MT7992_FW_VER_E2(pAd)) {
		cap->rxv_pkt_hdr_dw_num = E2_RXV_PACKET_HEADER_DW_NUM;
		cap->rxv_entry_hdr_dw_num = E2_RXV_ENTRY_HEADER_DW_NUM;
		cap->rxv_cmn1_dw_num = E2_CMN_RXV1_DW_NUM;
		cap->rxv_cmn2_dw_num = E2_CMN_RXV2_DW_NUM;
		cap->rxv_usr1_dw_num = E2_USR_RXV1_DW_NUM;
		cap->rxv_usr2_dw_num = E2_USR_RXV2_DW_NUM;
	} else {
		cap->rxv_pkt_hdr_dw_num = E1_RXV_PACKET_HEADER_DW_NUM;
		cap->rxv_entry_hdr_dw_num = E1_RXV_ENTRY_HEADER_DW_NUM;
		cap->rxv_cmn1_dw_num = E1_CMN_RXV1_DW_NUM;
		cap->rxv_cmn2_dw_num = E1_CMN_RXV2_DW_NUM;
		cap->rxv_usr1_dw_num = E1_USR_RXV1_DW_NUM;
		cap->rxv_usr2_dw_num = E1_USR_RXV2_DW_NUM;
	}

	return 0;
}

static UINT32 mt7992_rxv_entry_parse(struct _RTMP_ADAPTER *pAd, VOID *Data)
{
	RX_STATISTIC_RXV *rx_stat;
	INT32 foe = 0;
	UINT32 i = 0;
	uint32_t *pbuf = (uint32_t *)Data, *pbuf2 = NULL;
	uint32_t *pbuf3 = (uint32_t *)Data;
	uint8_t sta_cnt = 0, user_idx, band_idx;
	UINT32 rxv_entry_dw_cnt = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_ADAPTER *mac_ad = NULL;

	/* read sta count */
	sta_cnt = (*pbuf & BITS(16, 22)) >> 16;

	/* decide band index */
	band_idx = (*(pbuf + 1) & BITS(24, 25)) >> 24;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
			"sta_cnt: %d, band_idx: %d\n", sta_cnt, band_idx);

	/* sanity check for sta count */
	if (sta_cnt > MAX_USER_NUM)
		goto error1;

	/* sanity check for band index */
	if (band_idx >= PD_GET_BAND_NUM(pAd->physical_dev))
		goto error2;
	mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
	if (!mac_ad)
		goto error2;
	rx_stat = &mac_ad->rx_stat_rxv;

	/* rxv raw data log (header) */
	for (i = 0, pbuf2 = pbuf3; i < cap->rxv_entry_hdr_dw_num; i++, pbuf2++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_DEBUG,
			"HEADER(%d): 0x%x\n", i, *pbuf2);
	}
	pbuf3 += cap->rxv_entry_hdr_dw_num;

	/* rxv raw data log (CMN1) */
	for (i = 0, pbuf2 = pbuf3; i < cap->rxv_cmn1_dw_num; i++, pbuf2++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_DEBUG,
			"CMN1(%d): 0x%x\n", i, *pbuf2);
	}
	pbuf3 += cap->rxv_cmn1_dw_num;

	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		/* rxv raw data log (USR1) */
		for (i = 0, pbuf2 = pbuf3; i < cap->rxv_usr1_dw_num * sta_cnt; i++, pbuf2++) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_DEBUG,
				"user_idx: %d, USR1(%d): 0x%x\n", user_idx, i, *pbuf2);
		}
		pbuf3 += cap->rxv_usr1_dw_num;
	}

	for (user_idx = 0, pbuf2 = pbuf3; user_idx < sta_cnt; user_idx++) {
		/* rxv raw data log (USR2) */
		for (i = 0, pbuf2 = pbuf3; i < cap->rxv_usr2_dw_num * sta_cnt; i++, pbuf2++) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_DEBUG,
				"user_idx: %d, USR2(%d): 0x%x\n", user_idx, i, *pbuf2);
		}
		pbuf3 += cap->rxv_usr2_dw_num;
	}

	/* rxv raw data log (CMN2) */
	for (i = 0, pbuf2 = pbuf3; i < cap->rxv_cmn2_dw_num; i++, pbuf2++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_DEBUG,
			"CMN2(%d): 0x%x\n", i, *pbuf2);
	}
	pbuf3 += cap->rxv_cmn2_dw_num;

	/* pointer increment (entry header) */
	pbuf += cap->rxv_entry_hdr_dw_num;

	/* rxv raw data parsing (CMN1) */
	/* pfd */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW0_L;
	rx_stat->pfd = (*pbuf2 & BITS(4, 7)) >> 4;

	/* vht_group_id */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW1_L;
	rx_stat->vht_gid = (*pbuf2 & BITS(22, 27)) >> 22;

	/* rcpi */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW3_L;
	rx_stat->RCPI[0] = (*pbuf2 & BITS(0, 7)) >> 0;
	rx_stat->RCPI[1] = (*pbuf2 & BITS(8, 15)) >> 8;
	rx_stat->RCPI[2] = (*pbuf2 & BITS(16, 23)) >> 16;
	rx_stat->RCPI[3] = (*pbuf2 & BITS(24, 31)) >> 24;

	/* rssi */
	for (i = WF0; i < WF_NUM; i++) {
		if (rx_stat->RCPI[i] == 0xFF) {
			rx_stat->RCPI[i] = 0;
			rx_stat->RSSI[i] = -127;
		} else {
			rx_stat->RSSI[i] = (rx_stat->RCPI[i] - 220)/2;
		}
	}

	/* fagc rssi (in-band) */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW3_H;
	rx_stat->FAGC_RSSI_IB[0] = (*pbuf2 & BITS(0, 7)) >> 0;
	rx_stat->FAGC_RSSI_IB[1] = (*pbuf2 & BITS(8, 15)) >> 8;
	rx_stat->FAGC_RSSI_IB[2] = (*pbuf2 & BITS(16, 23)) >> 16;
	rx_stat->FAGC_RSSI_IB[3] = (*pbuf2 & BITS(24, 31)) >> 24;
	for (i = WF0; i < WF_NUM; i++) {
		if (rx_stat->FAGC_RSSI_IB[i] >= 128)
			rx_stat->FAGC_RSSI_IB[i] -= 256;
	}

	/* fagc rssi (wide-band) */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW4_L;
	rx_stat->FAGC_RSSI_WB[0] = (*pbuf2 & BITS(5, 12)) >> 5;
	rx_stat->FAGC_RSSI_WB[1] = (*pbuf2 & BITS(14, 21)) >> 14;
	rx_stat->FAGC_RSSI_WB[2] = (*pbuf2 & BITS(23, 30)) >> 23;
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW4_H;
	rx_stat->FAGC_RSSI_WB[3] = (*pbuf2 & BITS(0, 7)) >> 0;
	for (i = WF0; i < WF_NUM; i++) {
		if (rx_stat->FAGC_RSSI_WB[i] >= 128)
			rx_stat->FAGC_RSSI_WB[i] -= 256;
	}

	/* pointer increment (CMN1) */
	pbuf += cap->rxv_cmn1_dw_num;

	/* rxv raw data parsing (USR1) */
	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {

		/* rx_vld_ind */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_H;
		rx_stat->rx_vld_ind[user_idx] = (*pbuf2 & BIT(31)) >> 31;

		/* pointer increment (USR1) */
		pbuf += cap->rxv_usr1_dw_num;
	}

	/* rxv raw data parsing (USR2) */
	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		/* foe */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_L;
		foe = 0;
		foe |= (*pbuf2 & BITS(19, 31)) >> 19;
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_H;
		foe |= (((*pbuf2 & BITS(0, 6)) >> 0) << (31-19+1));
		rx_stat->FreqOffsetFromRx[user_idx] = foe;

		/* snr */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_L;
		rx_stat->SNR[user_idx] = (*pbuf2 & BITS(13, 18)) >> 13;
		rx_stat->SNR[user_idx] -= 16;

		/* fcs error */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_H;
		rx_stat->fcs_error[user_idx] = (*pbuf2 & BIT(13)) >> 13;
		if (rx_stat->fcs_error[user_idx])
			rx_stat->fcs_error_cnt[user_idx]++;

		/* pointer increment (USR2) */
		pbuf += cap->rxv_usr2_dw_num;
	}

	/* rxv raw data parsing (CMN2) */


	/* u4RxMuPktCount */
	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		if (rx_stat->pfd == TXMODE_VHT && !rx_stat->fcs_error[user_idx] &&
			(rx_stat->vht_gid != 0 && rx_stat->vht_gid != 63)) {
				rx_stat->rx_mu_ok_cnt[user_idx]++;
#ifdef CFG_SUPPORT_MU_MIMO
				if (user_idx == 0)
					pAd->u4RxMuPktCount++;
#endif
		} else if (rx_stat->pfd == TXMODE_HE_MU && !rx_stat->fcs_error[user_idx]) {
				rx_stat->rx_mu_ok_cnt[user_idx]++;
#ifdef CFG_SUPPORT_MU_MIMO
				if (user_idx == 0)
					pAd->u4RxMuPktCount++;
#endif
		}
	}

	/* update rxv count */
	rx_stat->rxv_cnt++;

	/* compute memory buffer size rxv entry raw data */
	rxv_entry_dw_cnt += cap->rxv_entry_hdr_dw_num;
	rxv_entry_dw_cnt += cap->rxv_cmn1_dw_num + cap->rxv_cmn2_dw_num;
	rxv_entry_dw_cnt += (cap->rxv_usr1_dw_num + cap->rxv_usr2_dw_num) * sta_cnt;

	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
			"\t user_idx: %d, FreqOffsetFromRx = %d\n",
			user_idx, rx_stat->FreqOffsetFromRx[user_idx]);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
			"\t user_idx: %d, fcs_error: %d, fcs error cnt: %d\n",
			user_idx, rx_stat->fcs_error[user_idx], rx_stat->fcs_error_cnt[user_idx]);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
			"\t user_idx: %d, rx_vld_ind: %d, fcs_error: %d, rx mu ok cnt: %d\n",
			user_idx, rx_stat->rx_vld_ind[user_idx], rx_stat->fcs_error[user_idx],
			rx_stat->rx_mu_ok_cnt[user_idx]);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
			"\t user_idx: %d,  SNR: %d\n",
			user_idx, rx_stat->SNR[user_idx]);
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"\t pfd(txmode): %d, vht_gid: %d\n",
		rx_stat->pfd, rx_stat->vht_gid);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"\t RCPI: (%d,%d,%d,%d)\n",
		rx_stat->RCPI[0], rx_stat->RCPI[1], rx_stat->RCPI[2], rx_stat->RCPI[3]);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"\t RSSI: (%d,%d,%d,%d)\n",
		rx_stat->RSSI[0], rx_stat->RSSI[1], rx_stat->RSSI[2], rx_stat->RSSI[3]);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"\t FAGC_RSSI_IB: (%d,%d,%d,%d)\n",
		rx_stat->FAGC_RSSI_IB[0], rx_stat->FAGC_RSSI_IB[1],
		rx_stat->FAGC_RSSI_IB[2], rx_stat->FAGC_RSSI_IB[3]);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"\t FAGC_RSSI_WB: (%d,%d,%d,%d)\n",
		rx_stat->FAGC_RSSI_WB[0], rx_stat->FAGC_RSSI_WB[1],
		rx_stat->FAGC_RSSI_WB[2], rx_stat->FAGC_RSSI_WB[3]);

	return 0;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"%s(): sta count is invalid(%d).\n", __func__, sta_cnt);
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"%s(): band index is invalid(%d).\n", __func__, band_idx);
	return 1;
}

static UINT32 mt7992_rxv_content_len(struct _RTMP_ADAPTER *pAd, uint8_t type_mask, uint8_t rxv_sta_cnt, uint16_t *len)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!len)
		return 1;

	/* rxv content length init */
	*len = 0;

	/*  update rxv content length (cmn1) */
	if (type_mask & BIT(RXV_CONTENT_CMN1))
		*len += cap->rxv_cmn1_dw_num;

	/*  update rxv content length (cmn2) */
	if (type_mask & BIT(RXV_CONTENT_CMN2))
		*len += cap->rxv_cmn2_dw_num;

	/*  update rxv content length (usr1) */
	if (type_mask & BIT(RXV_CONTENT_USR1))
		*len += (cap->rxv_usr1_dw_num * rxv_sta_cnt);

	/*  update rxv content length (usr2) */
	if (type_mask & BIT(RXV_CONTENT_USR2))
		*len += (cap->rxv_usr2_dw_num * rxv_sta_cnt);

	return 0;
}

static UINT32 mt7992_rxv_dump_start(struct _RTMP_ADAPTER *pAd)
{
	if (!pAd->rxv_dump_ctrl.alloc)
		goto error0;

	pAd->rxv_dump_ctrl.enable = true;
	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"dump list not alloc for start dump action.\n");
	return 1;
}

static UINT32 mt7992_rxv_dump_stop(struct _RTMP_ADAPTER *pAd)
{
	pAd->rxv_dump_ctrl.enable = false;
	return 0;
}

static UINT32 mt7992_rxv_dump_link_list_instl(
	struct _RTMP_ADAPTER *pAd, DL_LIST *list,
	VOID *ptr, RXV_DUMP_LIST_TYPE type)
{
	UINT32 ret = 0;
	VOID *ptr_tmp = NULL;
	uint8_t len = 0;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL;

	if (!ptr)
		goto error1;

	switch (type) {
	case RXV_DUMP_LIST_TYPE_CONTENT:
		len = sizeof(RXV_DUMP_ENTRY_CONTENT);
		break;
	case RXV_DUMP_LIST_TYPE_BASIC_ENTRY:
		len = sizeof(RXV_DUMP_BASIC_ENTRY);
		break;
	case RXV_DUMP_LIST_TYPE_ENTRY:
		len = sizeof(RXV_DUMP_ENTRY);
		break;
	default:
		goto error2;
	}

	ret = os_alloc_mem(pAd, (uint8_t **)&ptr_tmp, len);
	if (ret)
		goto error0;

	/* clear allocated memory for rxv dump entry content */
	os_zero_mem(ptr_tmp, len);
	/* update rxv dump entry content */
	os_move_mem(ptr_tmp, ptr, len);

	switch (type) {
	case RXV_DUMP_LIST_TYPE_CONTENT:
		rxv_dump_entry_content = (RXV_DUMP_ENTRY_CONTENT *) ptr_tmp;
		/* add rxv dump entry entity  to link list  */
		DlListAddTail(list, &rxv_dump_entry_content->list);
		break;
	case RXV_DUMP_LIST_TYPE_BASIC_ENTRY:
		rxv_dump_basic_entry = (RXV_DUMP_BASIC_ENTRY *) ptr_tmp;
		/* add rxv dump entry entity to link list  */
		DlListAddTail(list, &rxv_dump_basic_entry->list);
		break;
	case RXV_DUMP_LIST_TYPE_ENTRY:
		rxv_dump_entry = (RXV_DUMP_ENTRY *) ptr_tmp;
		/* add rxv dump entry entity to link list */
		DlListAddTail(list, &rxv_dump_entry->list);
		break;
	default:
		goto error2;
	}

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"allocate memory fail.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for data entry.\n");
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"invalid type for rxv dump link list.\n");
	return 1;
}

static UINT32 mt7992_rxv_dump_link_list_remv(struct _RTMP_ADAPTER *pAd, VOID *ptr, RXV_DUMP_LIST_TYPE type)
{
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL;

	if (!ptr)
		goto error1;

	switch (type) {
	case RXV_DUMP_LIST_TYPE_CONTENT:
		rxv_dump_entry_content = (RXV_DUMP_ENTRY_CONTENT *) ptr;
		/* free memory for rxv dump entry content */
		if (rxv_dump_entry_content) {
			/* delete link for this entry */
			DlListDel(&rxv_dump_entry_content->list);
			/* free memory for rxv content */
			if (rxv_dump_entry_content->content) {
				os_free_mem(rxv_dump_entry_content->content);
				rxv_dump_entry_content->content = NULL;
			}
			os_free_mem(rxv_dump_entry_content);
		}
		break;
	case RXV_DUMP_LIST_TYPE_BASIC_ENTRY:
		rxv_dump_basic_entry = (RXV_DUMP_BASIC_ENTRY *) ptr;
		/* free memory for rxv dump basic entry */
		if (rxv_dump_basic_entry) {
			/* delete link for this entry */
			DlListDel(&rxv_dump_basic_entry->list);
			os_free_mem(rxv_dump_basic_entry);
		}
		break;
	case RXV_DUMP_LIST_TYPE_ENTRY:
		rxv_dump_entry = (RXV_DUMP_ENTRY *) ptr;
		/* free memory for rxv dump entry */
		if (rxv_dump_entry) {
			/* delete link for this entry */
			DlListDel(&rxv_dump_entry->list);
			os_free_mem(rxv_dump_entry);
		}
		break;
	default:
		goto error2;
	}

	return 0;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for data entry.\n");
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"invalid type for rxv dump link list.\n");
	return 1;
}

static UINT32 mt7992_rxv_dump_buf_alloc(struct _RTMP_ADAPTER *pAd, uint8_t type_mask)
{
	INT32 ret = 0;
	uint8_t dump_entry_num = 10, entry_idx = 0, type_idx = 0, type_num = 0;
	RXV_DUMP_ENTRY rxv_dump_entry;
	DL_LIST *rxv_dump_entry_list = NULL;

	if (pAd->rxv_dump_ctrl.enable)
		goto error2;

	if (pAd->rxv_dump_ctrl.rxv_dump_entry_list)
		goto error1;

	ret = os_alloc_mem(pAd, (uint8_t **)&pAd->rxv_dump_ctrl.rxv_dump_entry_list, sizeof(DL_LIST));
	if (ret)
		goto error3;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	DlListInit(rxv_dump_entry_list);

	/* decide number of type */
	for (type_idx = 0, type_num = 0; type_idx < RXV_CONTENT_NUM; type_idx++) {
		if (type_mask & BIT(type_idx))
			type_num++;
	}

	/* for loop link list all rxv dump entry */
	for (entry_idx = 0; entry_idx < dump_entry_num; entry_idx++) {
		/* clear temp buffer */
		os_zero_mem(&rxv_dump_entry, sizeof(RXV_DUMP_ENTRY));
		/* config entry index and type num */
		rxv_dump_entry.entry_idx = entry_idx;
		rxv_dump_entry.type_num = type_num;

		ret = os_alloc_mem(pAd, (uint8_t **)&rxv_dump_entry.rxv_dump_basic_entry_list, sizeof(DL_LIST));
		if (ret)
			goto error3;
		DlListInit(rxv_dump_entry.rxv_dump_basic_entry_list);

		/* allocate entry and link to list */
		ret = mt7992_rxv_dump_link_list_instl(pAd,
			rxv_dump_entry_list, &rxv_dump_entry, RXV_DUMP_LIST_TYPE_ENTRY);
		if (ret)
			goto error0;
	}

	/* config rxv dump control */
	pAd->rxv_dump_ctrl.type_mask = type_mask;
	pAd->rxv_dump_ctrl.type_num = type_num;
	pAd->rxv_dump_ctrl.ring_idx = 0;
	pAd->rxv_dump_ctrl.dump_entry_total_num = DlListLen(rxv_dump_entry_list);
	pAd->rxv_dump_ctrl.valid_entry_num = 0;
	pAd->rxv_dump_ctrl.alloc = true;

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"link list install fail.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"no need to alloc buf for nonempty list.\n");
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"cannot alloc buf when enable dump process.\n");
	return 1;

error3:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"allocate memory fail.\n");
	return 1;
}

static UINT32 mt7992_rxv_dump_buf_clear(struct _RTMP_ADAPTER *pAd)
{
	UINT32 ret = 0;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL, *rxv_dump_entry_content_tmp = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL;
	DL_LIST *rxv_dump_basic_entry_list = NULL, *data_list = NULL;
	UINT32 dl_len = 0;

	if (pAd->rxv_dump_ctrl.enable)
		goto error2;

	/* loop for rxv dump entry list */
	if (!pAd->rxv_dump_ctrl.rxv_dump_entry_list)
		goto error3;

	dl_len = DlListLen(pAd->rxv_dump_ctrl.rxv_dump_entry_list);
	if (dl_len == 0)
		goto error1;

	DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp,
		pAd->rxv_dump_ctrl.rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {
		rxv_dump_basic_entry_list = rxv_dump_entry->rxv_dump_basic_entry_list;
		if (!rxv_dump_basic_entry_list)
			continue;

		dl_len = DlListLen(rxv_dump_basic_entry_list);
		if (dl_len == 0)
			continue;

		/* loop for rxv dump basic entry list */
		DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
			rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
			data_list = rxv_dump_basic_entry->data_list;
			if (!data_list)
				continue;

			dl_len = DlListLen(data_list);
			if (dl_len == 0)
				continue;

			/* loop for rxv dump entry content list */
			DlListForEachSafe(rxv_dump_entry_content, rxv_dump_entry_content_tmp,
				data_list, RXV_DUMP_ENTRY_CONTENT, list) {
				ret = mt7992_rxv_dump_link_list_remv(pAd,
					rxv_dump_entry_content, RXV_DUMP_LIST_TYPE_CONTENT);
				if (ret)
					goto error0;
			}

			os_free_mem(data_list);
			/* reset to null pointer */
			data_list = NULL;

			ret = mt7992_rxv_dump_link_list_remv(pAd, rxv_dump_basic_entry, RXV_DUMP_LIST_TYPE_BASIC_ENTRY);
			if (ret)
				goto error0;
		}

		os_free_mem(rxv_dump_basic_entry_list);
		/* reset to null pointer */
		rxv_dump_basic_entry_list = NULL;

		ret = mt7992_rxv_dump_link_list_remv(pAd, rxv_dump_entry, RXV_DUMP_LIST_TYPE_ENTRY);
		if (ret)
			goto error0;
	}

	os_free_mem(pAd->rxv_dump_ctrl.rxv_dump_entry_list);
	/* reset to null pointer */
	pAd->rxv_dump_ctrl.rxv_dump_entry_list = NULL;

	/* config rxv dump control */
	pAd->rxv_dump_ctrl.type_mask = 0;
	pAd->rxv_dump_ctrl.type_num = 0;
	pAd->rxv_dump_ctrl.ring_idx = 0;
	pAd->rxv_dump_ctrl.dump_entry_total_num = 0;
	pAd->rxv_dump_ctrl.valid_entry_num = 0;
	pAd->rxv_dump_ctrl.alloc = false;

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"link list remove fail.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"no need to clear empty list.\n");
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"cannot alloc buf when enable dump process.\n");
	return 1;

error3:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for dump entry list.\n");
	return 1;
}

static UINT32 mt7992_rxv_dump_update(struct _RTMP_ADAPTER *pAd, VOID *rxv_pkt, uint8_t rxv_cnt)
{
	UINT32 ret = 0;
	uint8_t rxv_idx = 0, type_idx = 0, type_mask = 0, type_num = 0, user_num = 0, user_idx = 0;
	uint8_t user_num_tbl[RXV_CONTENT_NUM] = {1, 0, 0, 1};
	UINT32 *ptr = NULL;
	uint16_t len = 0, len_byte = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	RXV_DUMP_ENTRY_CONTENT rxv_dump_entry_content;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content_ptr = NULL, *rxv_dump_entry_content_ptr2 = NULL;
	RXV_DUMP_BASIC_ENTRY rxv_dump_basic_entry;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry_ptr = NULL, *rxv_dump_basic_entry_ptr2 = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry_curr = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL, *rxv_dump_entry_curr = NULL;
	DL_LIST *rxv_dump_entry_list = NULL, *rxv_dump_basic_entry_list = NULL, *data_list = NULL;
	RXV_ENTRY_HDR rxv_entry_hdr;
	uint8_t rx_sta_cnt = 0;
	UINT32 dl_len = 0;

	if (!rxv_pkt)
		goto error0;

	if (!pAd->rxv_dump_ctrl.enable)
		return 0;

	ptr = (UINT32 *) rxv_pkt;

	/* rxv pkt header */
	ptr += cap->rxv_pkt_hdr_dw_num;

	type_mask = pAd->rxv_dump_ctrl.type_mask;
	type_num = pAd->rxv_dump_ctrl.type_num;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	if (!rxv_dump_entry_list)
		goto error5;

	dl_len = DlListLen(rxv_dump_entry_list);
	if (dl_len == 0)
		goto error6;

	os_zero_mem(&rxv_entry_hdr, sizeof(RXV_ENTRY_HDR));
	for (rxv_idx = 0; rxv_idx < rxv_cnt; rxv_idx++) {
		/* read entry pointer from link list */
		DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp, rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {
			if (rxv_dump_entry->entry_idx == pAd->rxv_dump_ctrl.ring_idx) {
				rxv_dump_entry_curr = rxv_dump_entry;
				break;
			}
		}

		/* sanity check for null pointer */
		if (!rxv_dump_entry_curr)
			goto error2;

		/* rxv entry header parsing */
		mt7992_rxv_entry_hdr_parser(pAd, ptr, &rxv_entry_hdr);
		rx_sta_cnt = rxv_entry_hdr.rx_sta_cnt;
		user_num_tbl[RXV_CONTENT_USR1] = user_num_tbl[RXV_CONTENT_USR2] = rx_sta_cnt;
		/* pointer increment for rxv entry header */
		ptr += cap->rxv_entry_hdr_dw_num;

		rxv_dump_basic_entry_list = rxv_dump_entry_curr->rxv_dump_basic_entry_list;
		if (!rxv_dump_basic_entry_list)
			goto error7;
		dl_len = DlListLen(rxv_dump_basic_entry_list);
		if (dl_len != 0) {
			/* loop for rxv dump basic entry list */
			DlListForEachSafe(rxv_dump_basic_entry_ptr, rxv_dump_basic_entry_ptr2,
				rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
				data_list = rxv_dump_basic_entry_ptr->data_list;
				if (!data_list)
					continue;
				dl_len = DlListLen(data_list);
				if (dl_len == 0)
					continue;

				/* loop for rxv dump entry content list */
				DlListForEachSafe(rxv_dump_entry_content_ptr, rxv_dump_entry_content_ptr2,
					data_list, RXV_DUMP_ENTRY_CONTENT, list) {
					ret = mt7992_rxv_dump_link_list_remv(pAd,
						rxv_dump_entry_content_ptr, RXV_DUMP_LIST_TYPE_CONTENT);
					if (ret)
						goto error4;
				}

				os_free_mem(data_list);
				/* reset to null pointer */
				data_list = NULL;

				ret = mt7992_rxv_dump_link_list_remv(pAd,
					rxv_dump_basic_entry_ptr, RXV_DUMP_LIST_TYPE_BASIC_ENTRY);
				if (ret)
					goto error4;
			}

			os_free_mem(rxv_dump_basic_entry_list);
			/* reset to null pointer */
			rxv_dump_basic_entry_list = NULL;
		}

		/* allocate memory for basic dump entry list */
		ret = os_alloc_mem(pAd, (uint8_t **)&rxv_dump_entry_curr->rxv_dump_basic_entry_list, sizeof(DL_LIST));
		if (ret)
			goto error3;
		rxv_dump_basic_entry_list = rxv_dump_entry_curr->rxv_dump_basic_entry_list;
		DlListInit(rxv_dump_basic_entry_list);

		for (type_idx = 0; type_idx < RXV_CONTENT_NUM; type_idx++) {
			mt7992_rxv_content_len(pAd, BIT(type_idx), 1, &len);
			len_byte = len << 2;

			if (type_mask & BIT(type_idx)) {
				os_zero_mem(&rxv_dump_basic_entry, sizeof(RXV_DUMP_BASIC_ENTRY));
				/* config basic entry parameter */
				rxv_dump_basic_entry.type_idx = type_idx;
				rxv_dump_basic_entry.len = len_byte;
				user_num = user_num_tbl[type_idx];
				rxv_dump_basic_entry.usr_num = user_num;
				rxv_dump_basic_entry.data_list = NULL;
				ret = os_alloc_mem(pAd, (uint8_t **)&rxv_dump_basic_entry.data_list, sizeof(DL_LIST));
				if (ret)
					goto error3;
				DlListInit(rxv_dump_basic_entry.data_list);
				/* install basic entry to list */
				ret = mt7992_rxv_dump_link_list_instl(pAd, rxv_dump_basic_entry_list,
						&rxv_dump_basic_entry, RXV_DUMP_LIST_TYPE_BASIC_ENTRY);
				if (ret) {
					os_free_mem(rxv_dump_basic_entry.data_list);
					rxv_dump_basic_entry.data_list = NULL;
					goto error3;
				}
				/* read basic entry pointer from link list */
				DlListForEachSafe(rxv_dump_basic_entry_ptr, rxv_dump_basic_entry_ptr2,
					rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
					if (rxv_dump_basic_entry_ptr->type_idx == type_idx) {
						rxv_dump_basic_entry_curr = rxv_dump_basic_entry_ptr;
						break;
					}
				}

				/* rxv content copy process */
				if (!rxv_dump_basic_entry_curr ||
					!rxv_dump_basic_entry_curr->data_list)
					continue;

				data_list = rxv_dump_basic_entry_curr->data_list;

				for (user_idx = 0; user_idx < user_num; user_idx++) {
					os_zero_mem(&rxv_dump_entry_content, sizeof(RXV_DUMP_ENTRY_CONTENT));
					/* config basic entry parameter */
					rxv_dump_entry_content.user_idx = user_idx;
					rxv_dump_entry_content.len = len_byte;
					rxv_dump_entry_content.content = NULL;
					ret = os_alloc_mem(pAd, (uint8_t **)&rxv_dump_entry_content.content, len_byte);
					if (ret)
						goto error1;

					/* copy rxv content to buffer */
					os_move_mem(rxv_dump_entry_content.content, ptr, len_byte);

					/* install basic entry to list */
					ret = mt7992_rxv_dump_link_list_instl(pAd, data_list,
							&rxv_dump_entry_content, RXV_DUMP_LIST_TYPE_CONTENT);
					if (ret) {
						os_free_mem(rxv_dump_entry_content.content);
						rxv_dump_entry_content.content = NULL;
						goto error3;
					}
					/* pointer increment for rxv entry content */
					ptr += len;
				}
			} else {
				/* pointer increment for rxv entry content */
				ptr += len * user_num_tbl[type_idx];
			}
		}

		/* increment rxv dump control ring index */
		pAd->rxv_dump_ctrl.ring_idx++;
		/* overflow handle for rxv dump control ring index */
		if (pAd->rxv_dump_ctrl.ring_idx >= pAd->rxv_dump_ctrl.dump_entry_total_num)
			pAd->rxv_dump_ctrl.ring_idx -= pAd->rxv_dump_ctrl.dump_entry_total_num;

		if (pAd->rxv_dump_ctrl.valid_entry_num < pAd->rxv_dump_ctrl.dump_entry_total_num)
			pAd->rxv_dump_ctrl.valid_entry_num++;
	}

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for rxv pkt.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"allocate memory fail.\n");
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for current rxv dump entry.\n");
	return 1;

error3:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"invalid type for rxv dump link list.\n");
	return 1;

error4:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"remove dump basic entry list fail.\n");
	return 1;

error5:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for dump entry list.\n");
	return 1;

error6:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"empty list for dump entry.\n");
	return 1;

error7:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"null pointer for basic dump entry list.\n");
	return 1;
}

static UINT32 mt7992_rxv_dump_show_list(struct _RTMP_ADAPTER *pAd)
{
	DL_LIST *rxv_dump_entry_list = NULL, *rxv_dump_basic_entry_list = NULL, *data_list = NULL;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL, *rxv_dump_entry_content_tmp = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL;
	UINT32 *ptr = NULL;
	uint8_t dw_idx = 0, dw_num = 0;
	UINT32 dl_len = 0;

	MTWF_PRINT("%s(): enable: %d, type_mask: 0x%x, type_num: %d\n", __func__,
		pAd->rxv_dump_ctrl.enable,
		pAd->rxv_dump_ctrl.type_mask,
		pAd->rxv_dump_ctrl.type_num);

	MTWF_PRINT("%s(): ring_idx: %d, dump_entry_total_num: %d, valid_entry_num: %d\n",
		__func__, pAd->rxv_dump_ctrl.ring_idx,
		pAd->rxv_dump_ctrl.dump_entry_total_num,
		pAd->rxv_dump_ctrl.valid_entry_num);

	if (pAd->rxv_dump_ctrl.enable)
		goto error2;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	if (!rxv_dump_entry_list)
		goto error1;

	dl_len = DlListLen(rxv_dump_entry_list);
	if (dl_len == 0)
		goto error0;

	/* list loop for dump entry */
	DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp,
		rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {

		MTWF_PRINT("    entry_idx: %d, type_num: %d\n",
			rxv_dump_entry->entry_idx,
			rxv_dump_entry->type_num);

		rxv_dump_basic_entry_list = rxv_dump_entry->rxv_dump_basic_entry_list;
		if (!rxv_dump_basic_entry_list)
			continue;
		dl_len = DlListLen(rxv_dump_basic_entry_list);
		if (dl_len == 0)
			continue;

		/* list loop for dump basic entry */
		DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
			rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {

			MTWF_PRINT("        type_idx: %d, len: %d, usr_num: %d\n",
				rxv_dump_basic_entry->type_idx,
				rxv_dump_basic_entry->len,
				rxv_dump_basic_entry->usr_num);

			data_list = rxv_dump_basic_entry->data_list;
			if (!data_list)
				continue;
			dl_len = DlListLen(data_list);
			if (dl_len == 0)
				continue;

			/* list loop for dump entry content */
			DlListForEachSafe(rxv_dump_entry_content, rxv_dump_entry_content_tmp,
				data_list, RXV_DUMP_ENTRY_CONTENT, list) {

				MTWF_PRINT("            user_idx: %d, len: %d\n",
					rxv_dump_entry_content->user_idx,
					rxv_dump_entry_content->len);

				ptr = (UINT32 *) rxv_dump_entry_content->content;
				dw_num = rxv_dump_entry_content->len >> 2;

				if (!ptr)
					continue;

				MTWF_PRINT("                ");
				for (dw_idx = 0; dw_idx < dw_num; dw_idx++, ptr++) {
					MTWF_PRINT("(DW%02d):%08X  ", dw_idx, *ptr);
					if (dw_idx % 4 == 3) {
						MTWF_PRINT("\n");
						MTWF_PRINT("                ");
					}
				}
				MTWF_PRINT("\n");
			}
		}
	}

	return 0;

error0:
	MTWF_PRINT("%s(): empty dump entry list.\n", __func__);
	return 1;

error1:
	MTWF_PRINT("%s(): null pointer for dump entry list.\n", __func__);
	return 1;

error2:
	MTWF_PRINT("%s(): cannot access list in rxv dumping process.\n", __func__);
	return 1;
}

static UINT32 mt7992_rxv_dump_user_content_compose(struct _RTMP_ADAPTER *pAd, DL_LIST *data_list, uint8_t user_idx,
		VOID *user_content, UINT32 *len)
{
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL, *rxv_dump_entry_content_tmp = NULL;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content_curr = NULL;
	uint8_t *ptr = NULL;
	UINT32 usr_idx = 0, raw_data_len = 0;

	if (!data_list)
		goto error0;

	if (!user_content)
		goto error1;

	if (!len)
		goto error2;

	ptr = (uint8_t *) user_content;

	/* read entry content pointer from link list */
	DlListForEachSafe(rxv_dump_entry_content, rxv_dump_entry_content_tmp, data_list, RXV_DUMP_ENTRY_CONTENT, list) {
		if (rxv_dump_entry_content->user_idx == user_idx) {
			rxv_dump_entry_content_curr = rxv_dump_entry_content;
			break;
		}
	}

	/* update user content param */
	usr_idx = rxv_dump_entry_content->user_idx;
	raw_data_len = rxv_dump_entry_content->len;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &usr_idx, sizeof(usr_idx));
	*len += sizeof(usr_idx);
	ptr += sizeof(usr_idx);
	os_move_mem(ptr, &raw_data_len, sizeof(raw_data_len));
	*len += sizeof(raw_data_len);
	ptr += sizeof(raw_data_len);
	os_move_mem(ptr, rxv_dump_entry_content->content, raw_data_len);
	*len += raw_data_len;

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for data list.\n", __func__);
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for user content.\n", __func__);
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for buffer of length.\n", __func__);
	return 1;
}

static UINT32 mt7992_rxv_dump_type_detail_content_compose(
	struct _RTMP_ADAPTER *pAd,
	uint8_t usr_num,
	DL_LIST *data_list,
	VOID *type_detail_content,
	UINT32 *len)
{
	uint8_t *ptr = NULL, *ptr2 = NULL;
	UINT32 usr_cnt = 0, usr_content_len = 0, usr_content_len_sum = 0;
	uint8_t user_idx = 0;

	if (!type_detail_content)
		goto error0;

	if (!len)
		goto error1;

	ptr = (uint8_t *) type_detail_content;

	/* update type detail content param */
	usr_cnt = (UINT32) usr_num;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &usr_cnt, sizeof(usr_cnt));
	*len += sizeof(usr_cnt);
	ptr += sizeof(usr_cnt);

	/* keep pointer to update user content length */
	ptr2 = ptr;
	*len += sizeof(usr_content_len);
	ptr += sizeof(usr_content_len);

	for (user_idx = 0; user_idx < usr_cnt; user_idx++) {
		mt7992_rxv_dump_user_content_compose(pAd, data_list, user_idx, ptr, &usr_content_len);
		*len += usr_content_len;
		ptr += usr_content_len;
		usr_content_len_sum += usr_content_len;
	}

	os_move_mem(ptr2, &usr_content_len_sum, sizeof(usr_content_len_sum));

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for type detail content.\n", __func__);
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for buffer of length.\n", __func__);
	return 1;
}

static UINT32 mt7992_rxv_dump_type_content_compose(
	struct _RTMP_ADAPTER *pAd,
	DL_LIST *rxv_dump_basic_entry_list,
	uint8_t type_idx,
	VOID *type_content,
	UINT32 *len)
{
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry_curr = NULL;
	DL_LIST *data_list = NULL;
	uint8_t *ptr = NULL, *ptr2 = NULL;
	UINT32 param_type_idx = 0, type_detail_content_len = 0;
	uint8_t usr_num = 0;

	if (!rxv_dump_basic_entry_list)
		goto error0;

	if (!type_content)
		goto error1;

	if (!len)
		goto error2;

	ptr = (uint8_t *) type_content;

	/* read entry content pointer from link list */
	DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
		rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
		if (rxv_dump_basic_entry->type_idx == type_idx) {
			rxv_dump_basic_entry_curr = rxv_dump_basic_entry;
			break;
		}
	}

	/* update type detail content param */
	if (rxv_dump_basic_entry_curr) {
		param_type_idx = rxv_dump_basic_entry_curr->type_idx;
		usr_num = rxv_dump_basic_entry_curr->usr_num;
		data_list = rxv_dump_basic_entry_curr->data_list;
	}
	if (!data_list)
		goto error0;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &param_type_idx, sizeof(param_type_idx));
	*len += sizeof(param_type_idx);
	ptr += sizeof(param_type_idx);

	/* keep pointer to update user content length */
	ptr2 = ptr;
	*len += sizeof(type_detail_content_len);
	ptr += sizeof(type_detail_content_len);

	mt7992_rxv_dump_type_detail_content_compose(pAd,
		usr_num, data_list, ptr, &type_detail_content_len);
	*len += type_detail_content_len;

	os_move_mem(ptr2, &type_detail_content_len, sizeof(type_detail_content_len));

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for badic entry list.\n", __func__);
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for type content.\n", __func__);
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
		"%s(): null pointer for buffer of length.\n", __func__);
	return 1;
}

static UINT32 mt7992_rxv_dump_rxv_content_compose(
	struct _RTMP_ADAPTER *pAd,
	uint8_t entry_idx,
	VOID *rxv_content,
	UINT32 *len)
{
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry_curr = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	DL_LIST *rxv_dump_basic_entry_list = NULL, *rxv_dump_entry_list = NULL;
	uint8_t *ptr = NULL;
	UINT32 type_cnt = 0, type_content_len = 0;
	uint8_t type_idx = 0;

	if (!rxv_content)
		goto error1;

	if (!len)
		goto error2;

	ptr = (uint8_t *) rxv_content;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	if (!rxv_dump_entry_list)
		goto error0;

	/* read entry content pointer from link list */
	DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp,
		rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {
		if (rxv_dump_entry->entry_idx == entry_idx) {
			rxv_dump_entry_curr = rxv_dump_entry;
			break;
		}
	}

	/* update type detail content param */
	if (rxv_dump_entry_curr) {
		type_cnt = rxv_dump_entry_curr->type_num;
		rxv_dump_basic_entry_list = rxv_dump_entry_curr->rxv_dump_basic_entry_list;
	}

	if (!rxv_dump_basic_entry_list)
		goto error0;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &type_cnt, sizeof(type_cnt));
	*len += sizeof(type_cnt);
	ptr += sizeof(type_cnt);

	/* recursive update parameters for different types */
	DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
		rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
			type_idx = rxv_dump_basic_entry->type_idx;
			mt7992_rxv_dump_type_content_compose(pAd, rxv_dump_basic_entry_list,
				type_idx, ptr, &type_content_len);
			*len += type_content_len;
			ptr += type_content_len;
	}

	return 0;

error0:
	MTWF_PRINT("%s(): null pointer for entry list.\n", __func__);
	return 1;

error1:
	MTWF_PRINT("%s(): null pointer for rxv content.\n", __func__);
	return 1;

error2:
	MTWF_PRINT("%s(): null pointer for buffer of length.\n", __func__);
	return 1;
}

static UINT32 mt7992_rxv_dump_show_rpt(struct _RTMP_ADAPTER *pAd)
{
	UINT32 ret = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 *ptr = NULL, *ptr2 = NULL;
	uint16_t buf_len = 0;
	uint8_t cmn1_len = 0, cmn2_len = 0, usr1_len = 0, usr2_len = 0;
	UINT32 len = 0, dw_cnt = 0, dw_idx = 0;
	uint8_t valid_entry_num = 0, ring_idx = 0, dump_entry_total_num = 0;
	CHAR idx = 0;
	uint8_t dw_line_cnt = 4, entry_idx = 0;

	/* compute max buffer size */
	cmn1_len = (1 + 1) + (1 + 1) + (1 + 1) + cap->rxv_cmn1_dw_num;
	cmn2_len = (1 + 1) + (1 + 1) + (1 + 1) + cap->rxv_cmn2_dw_num;
	usr1_len = (1 + 1) + (1 + 1) + MAX_USER_NUM * ((1 + 1) + cap->rxv_usr1_dw_num);
	usr2_len = (1 + 1) + (1 + 1) + MAX_USER_NUM * ((1 + 1) + cap->rxv_usr2_dw_num);

	buf_len = (cmn1_len + cmn2_len + usr1_len + usr2_len) << 2;
	ret = os_alloc_mem(pAd, (uint8_t **)&ptr, buf_len);
	if (ret)
		goto error1;

	valid_entry_num = pAd->rxv_dump_ctrl.valid_entry_num;
	ring_idx = pAd->rxv_dump_ctrl.ring_idx;
	dump_entry_total_num = pAd->rxv_dump_ctrl.dump_entry_total_num;

	ptr2 = ptr;
	for (idx = ring_idx - valid_entry_num; idx < ring_idx; idx++) {
		if (idx < 0)
			entry_idx = idx + dump_entry_total_num;
		else
			entry_idx = idx;

		mt7992_rxv_dump_rxv_content_compose(pAd, entry_idx, ptr, &len);

		dw_cnt = len >> 2;
		if (len % 4 != 0)
			dw_cnt += 1;

		MTWF_PRINT("entry_idx: %d\n", entry_idx);
		for (dw_idx = 0; dw_idx < dw_cnt; dw_idx++, ptr2++) {
			MTWF_PRINT("DW[%04d]: 0x%08X  ", dw_idx, *ptr2);
			if ((dw_idx % dw_line_cnt) == (dw_line_cnt - 1))
				MTWF_PRINT("\n");
		}
		MTWF_PRINT("\n");

		os_zero_mem(ptr, buf_len);
		ptr2 = ptr;
	}

	os_free_mem(ptr);

	return 0;

error1:
	MTWF_PRINT("%s(): allocate memory fail.\n", __func__);
	return 1;
}

static UINT32 mt7992_rxv_data_dump(struct _RTMP_ADAPTER *pAd, VOID *Data, UINT32 rxv_cnt, UINT32 rxv_byte_cnt)
{
	if (!pAd->rxv_raw_data.rxv_pkt) {
		/* allocate memory for rxv pkt */
		os_alloc_mem(pAd, (uint8_t **)&pAd->rxv_raw_data.rxv_pkt, rxv_byte_cnt);
		if (!pAd->rxv_raw_data.rxv_pkt) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
				"allocate memory for pAd->rxv_raw_data.rxv_pkt failed!\n");
			return 0;
		}
		pAd->rxv_raw_data.rxv_byte_cnt = rxv_byte_cnt;
	} else {
		if (rxv_byte_cnt != pAd->rxv_raw_data.rxv_byte_cnt) {
			/* free memory for rxv pkt */
			os_zero_mem(pAd->rxv_raw_data.rxv_pkt, pAd->rxv_raw_data.rxv_byte_cnt);
			os_free_mem(pAd->rxv_raw_data.rxv_pkt);
			pAd->rxv_raw_data.rxv_pkt = NULL;
			pAd->rxv_raw_data.rxv_byte_cnt = 0;

			/* re-allocate memory for rxv pkt */
			os_alloc_mem(pAd, (uint8_t **)&pAd->rxv_raw_data.rxv_pkt, rxv_byte_cnt);
			if (!pAd->rxv_raw_data.rxv_pkt) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
					"allocate memory for pAd->rxv_raw_data.rxv_pkt failed!\n");
				return 0;
			}
			pAd->rxv_raw_data.rxv_byte_cnt = rxv_byte_cnt;
		}
	}

	/* copy rxv packet content to buffer */
	os_move_mem(pAd->rxv_raw_data.rxv_pkt, Data, rxv_byte_cnt);

	/* testmode rxv dump control */
	mt7992_rxv_dump_update(pAd, Data, rxv_cnt);

	return 0;
}

static UINT32 mt7992_rxv_packet_parse(struct _RTMP_ADAPTER *pAd, VOID *Data)
{
	uint32_t *pbuf = (uint32_t *)Data, *pbuf2 = NULL;
	UINT32 byte_cnt_sum = 0;
	uint8_t i, sta_cnt = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	uint16_t rxv_entry_dw_cnt = 0;
	RXV_PKT_HDR rxv_pkt_hdr;

	/* rxv pkt header parsing */
	os_zero_mem(&rxv_pkt_hdr, sizeof(RXV_PKT_HDR));
	mt7992_rxv_pkt_hdr_parser(pAd, pbuf, &rxv_pkt_hdr);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
		"RxV Report: RxvCnt: %d, ByteCnt: %d\n",
		 rxv_pkt_hdr.rxv_cnt, rxv_pkt_hdr.rx_byte_cnt);

	mt7992_rxv_data_dump(pAd, Data, rxv_pkt_hdr.rxv_cnt, rxv_pkt_hdr.rx_byte_cnt);

	/* packet header processing */
	pbuf += cap->rxv_pkt_hdr_dw_num;
	byte_cnt_sum += cap->rxv_pkt_hdr_dw_num << 2;

	for (i = 0, pbuf2 = pbuf; i < rxv_pkt_hdr.rxv_cnt; i++) {
		/* read sta count */
		sta_cnt = ((*pbuf2) & BITS(16, 22)) >> 16;
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
			"sta_cnt: %d\n", sta_cnt);

		/* sanity check for sta count */
		if (sta_cnt > MAX_USER_NUM) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO,
				"sta count is invalid(%d).\n", sta_cnt);
			return 1;
		}

		/* parsing rxv entry */
		chip_parse_rxv_entry(pAd, pbuf2);

		/* update rxv entry dw count */
		rxv_entry_dw_cnt = (cap->rxv_entry_hdr_dw_num + cap->rxv_cmn1_dw_num +
			sta_cnt*(cap->rxv_usr1_dw_num + cap->rxv_usr2_dw_num) + cap->rxv_cmn2_dw_num);

		/* pointer update for next rxv entry */
		pbuf2 += rxv_entry_dw_cnt;
		byte_cnt_sum += (rxv_entry_dw_cnt << 2);
	}

	/* sanity check for byte count */
	if (byte_cnt_sum != rxv_pkt_hdr.rx_byte_cnt) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
				"RxV Report: byte_cnt_sum: %d, rxv_byte_cnt: %d\n",
				byte_cnt_sum, rxv_pkt_hdr.rx_byte_cnt);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_ERROR,
				"Received byte count not equal to rxv_entry byte count required!\n");
		return 1;
	}

	return 0;
}

static VOID mt7992_set_mgmt_pkt_txpwr_prctg(RTMP_ADAPTER *pAd,	struct wifi_dev *wdev, uint8_t prctg)
{
	int8_t  power_dropLevel = 0;


	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"%s: wdev NULL\n", __func__);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"%s: [TxdPwrOffset]: %d\n", __func__, wdev->mgmt_txd_txpwr_offset);


	if ((prctg > 90) && (prctg < 100))
		power_dropLevel = 0;
	else if ((prctg > 60) && (prctg <= 90))  /* reduce Pwr for 1 dB. */
		power_dropLevel = -2;
	else if ((prctg > 30) && (prctg <= 60))  /* reduce Pwr for 3 dB. */
		power_dropLevel = -6;
	else if ((prctg > 15) && (prctg <= 30))  /* reduce Pwr for 6 dB. */
		power_dropLevel = -12;
	else if ((prctg > 9) && (prctg <= 15))   /* reduce Pwr for 9 dB. */
		power_dropLevel = -18;
	else if ((prctg > 0) && (prctg <= 9))   /* reduce Pwr for 12 dB. */
		power_dropLevel = -24;


	wdev->mgmt_txd_txpwr_offset = (uint8_t)power_dropLevel;

#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
#endif /* CONFIG_AP_SUPPORT */

	return;

}
static VOID mt7992_rssi_get(RTMP_ADAPTER *pAd, uint16_t Wcid, CHAR *RssiSet)
{
	RSSI_REPORT rssi_rpt;

	NdisZeroMemory(&rssi_rpt, sizeof(rssi_rpt));
	/* fw command to query rcpi/rssi */
	MtCmdGetRssi(pAd, Wcid, (UINT32 *)&rssi_rpt);

	RssiSet[0] = rssi_rpt.rssi[0];
	RssiSet[1] = rssi_rpt.rssi[1];
	RssiSet[2] = rssi_rpt.rssi[2];
	RssiSet[3] = rssi_rpt.rssi[3];

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
		"%s(): wcid: %d, rssi: %d, %d, %d, %d\n", __func__, Wcid,
		 RssiSet[0], RssiSet[1], RssiSet[2], RssiSet[3]);
}

#ifdef CONFIG_WLAN_SERVICE
#ifdef PRE_CAL_MT7992_SUPPORT
VOID MtATE_Dump_Group_PreCal_7992(RTMP_ADAPTER *pAd, UINT8 op)
{
	UINT32 i = 0;
	UINT32 *ptr = NULL;
	UINT32 totalsize;
	struct _RTMP_CHIP_CAP *cap = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (op == PREK_GROUP_DUMP) {
		ptr = (UINT32 *)(pAd->PreCalImage + cap->prek_ee_info.group_flash_offset_g_begin);
		totalsize = cap->prek_ee_info.cal_result_size;
	} else if (op == PREK_GROUP_DUMP_5G) {
		ptr = (UINT32 *)(pAd->PreCalImage + cap->prek_ee_info.group_flash_offset_a5_begin);
		totalsize = cap->prek_ee_info.cal_result_size_5g;
	} else {
		ptr = (UINT32 *)(pAd->PreCalImage + cap->prek_ee_info.group_flash_offset_a6_begin);
		totalsize = cap->prek_ee_info.cal_result_size_6g +
				cap->prek_ee_info.cal_result_size_adcdcoc_2g +
				cap->prek_ee_info.cal_result_size_adcdcoc_5g +
				cap->prek_ee_info.cal_result_size_adcdcoc_6g;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Group Pre-Cal:\n");

	for (i = 0; i < totalsize / 4; i += 4) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
		}

}

VOID MtATE_Dump_DPD_PreCal_7992(RTMP_ADAPTER *pAd, UINT8 op)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 i = 0;
	UINT32 *ptr = NULL;
	UINT32 totalsize;

	if (op == PREK_DPD_DUMP_6G) {
		ptr = (UINT32 *)(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a6_begin);
		totalsize = cap->prek_ee_info.dpd_cal_6g_total_size;
	} else if (op == PREK_DPD_DUMP_5G) {
		ptr = (UINT32 *)(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a5_begin);
		totalsize = cap->prek_ee_info.dpd_cal_5g_total_size;
	} else {
		ptr = (UINT32 *)(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_g_begin);
		totalsize = cap->prek_ee_info.dpd_cal_2g_total_size;
	}
	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"DPD/Flatness Pre-Cal: %p\n", ptr);


	for (i = 0; i < totalsize / 4; i += 4) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

INT32 mt7992_ate_group_prek(RTMP_ADAPTER *pAd, UINT8 op)
{
	USHORT doCal1 = 0;
	USHORT value = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct physical_device *ph_dev = pAd->physical_dev;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ph_dev->E2pAccessMode != E2P_FLASH_MODE && ph_dev->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	/* This flag is used for receive N9 Firmware re-cal event */
	pAd->bPreCalMode = TRUE;

	if (op == PREK_GROUP_CLEAN) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);

		/* Clear bit0 */
		chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

		doCal1 = doCal1 & ~(BIT(GROUP_PRECAL_INDN_BIT)) & ~(BIT(GROUP5G_PRECAL_INDN_BIT)) & ~(BIT(GROUP6G_PRECAL_INDN_BIT));
		chip_ops->eewrite(pAd, PRECAL_INDICATION_BYTE, doCal1);

	} else if ((op == PREK_GROUP_DUMP) || (op == PREK_GROUP_DUMP_5G) || (op == PREK_GROUP_DUMP_6G)) {
		MtATE_Dump_Group_PreCal_7992(pAd, op);
	} else if (op == PREK_GROUP_PROC) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);

		/* Execute pre-k(no dpd) and apply */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<29), 0);

		chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

		/* raise group pre-cal indication bit */
		chip_ops->eeread(pAd, EEPROM_ANTENNA_CFG_OFFSET, &value);
		value &= 0xFF;
		if (((value & BAND_SELECT_MASK_B0) >> BAND_SELECT_OFFSET_B0) == BAND_SELECT_2G)
			doCal1 = doCal1 | (1 << GROUP_PRECAL_INDN_BIT);
		if (((value & BAND_SELECT_MASK_B1) >> BAND_SELECT_OFFSET_B1) == BAND_SELECT_5G)
			doCal1 = doCal1 | (1 << GROUP5G_PRECAL_INDN_BIT);

		chip_ops->eeread(pAd, EEPROM_ANTENNA_CFG_OFFSET_1, &value);
		value &= 0xFF;
		if (((value & BAND_SELECT_MASK_B2) >> BAND_SELECT_OFFSET_B2) == BAND_SELECT_6G)
			doCal1 = doCal1 | (1 << GROUP6G_PRECAL_INDN_BIT);

		chip_ops->eewrite(pAd, PRECAL_INDICATION_BYTE, doCal1);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"Should not be here !\n");
	}


	return TRUE;

}


INT32 mt7992_ate_dpd_prek(RTMP_ADAPTER *pAd, UINT8 op)
{
	UINT8                 i = 0;
	USHORT                doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct physical_device *ph_dev = pAd->physical_dev;
#ifdef DBDC_MODE
	UINT32 band0_tx_path_backup, band0_rx_path_backup;
	UINT32 band1_tx_path_backup, band1_rx_path_backup;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ph_dev->E2pAccessMode != E2P_FLASH_MODE && ph_dev->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	pAd->bPreCalMode = TRUE;

	if (op == PREK_DPD_CLEAN) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, cap->prek_ee_info.dpd_cal_total_size);
		/* Clear DPD2G, DPD5G and DPD6G indication bit */

		chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

		doCal1 = doCal1 & ~(BIT(DPD2G_PRECAL_INDN_BIT)) & ~(BIT(DPD5G_PRECAL_INDN_BIT)) & ~(BIT(DPD6G_PRECAL_INDN_BIT));
		chip_ops->eewrite(pAd, PRECAL_INDICATION_BYTE, doCal1);

	} else if ((op == PREK_DPD_DUMP_2G) || (op == PREK_DPD_DUMP_5G) || (op == PREK_DPD_DUMP_6G)) {
		MtATE_Dump_DPD_PreCal_7992(pAd, op);
	} else if (op == PREK_DPD_5G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a5_begin, cap->prek_ee_info.dpd_cal_5g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug*/
		/* we can use only ch36 to verify and need pay attention to the index */
		for (i = 0; i < (MT7992_PER_CH_A5_BW20_BW80_BW160_SIZE); i++) {
			/* set channel command */
			/* per group calibration - set to channel 36, 52, BW20 */
			if (i < MT7992_PER_CH_A5_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7992_PER_CH_A5_BW20[i];
				ch_cfg.ControlChannel = MT7992_PER_CH_A5_BW20[i];
			} else if (i < (MT7992_PER_CH_A5_BW20_SIZE + MT7992_PER_CH_A5_BW80_SIZE)) {
				ch_cfg.Bw = BW_80;
				ch_cfg.CentralChannel =
				MT7992_PER_CH_A5_BW80[i - MT7992_PER_CH_A5_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7992_PER_CH_A5_BW80[i - MT7992_PER_CH_A5_BW20_SIZE];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7992_PER_CH_A5_BW160[i - MT7992_PER_CH_A5_BW20_SIZE - MT7992_PER_CH_A5_BW80_SIZE];
				ch_cfg.ControlChannel =
				MT7992_PER_CH_A5_BW160[i - MT7992_PER_CH_A5_BW20_SIZE - MT7992_PER_CH_A5_BW80_SIZE];
				ch_cfg.eht_cen_ch =
				MT7992_PER_CH_A5_BW160[i - MT7992_PER_CH_A5_BW20_SIZE - MT7992_PER_CH_A5_BW80_SIZE];
			}

			ch_cfg.BandIdx = 1;
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 1;
			ch_cfg.Channel_Band = 1;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = pAd->Antenna.field.RxPath;
				ch_cfg.RxStream = pAd->Antenna.field.TxPath;
			} else {
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
#endif
				ch_cfg.TxStream = pAd->Antenna.field.RxPath;
				ch_cfg.RxStream = pAd->Antenna.field.TxPath;
			}
			UniCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			ch_cfg.TxStream = pAd->Antenna.field.RxPath;
			ch_cfg.RxStream = pAd->Antenna.field.TxPath;

			UniCmdSetTxRxPath(pAd, ch_cfg);

			if (ch_cfg.Bw < BW_80)
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, ch_cfg.BandIdx);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5_MEM, ch_cfg.BandIdx);
		}
		chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

		doCal1 |= (1 << DPD5G_PRECAL_INDN_BIT);
		chip_ops->eewrite(pAd, PRECAL_INDICATION_BYTE, doCal1);
	} else if (op == PREK_DPD_6G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a6_begin,
		cap->prek_ee_info.dpd_cal_6g_total_size);

		/* 6G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < (MT7992_PER_CH_A6_BW20_BW80_BW160_SIZE); i++) {
			/* set channel command*/
			/* per group calibration - set to channel 36, 52, BW20 */
			if (i < MT7992_PER_CH_A6_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7992_PER_CH_A6_BW20[i];
				ch_cfg.ControlChannel = MT7992_PER_CH_A6_BW20[i];
			} else if (i < (MT7992_PER_CH_A6_BW20_SIZE + MT7992_PER_CH_A6_BW80_SIZE)) {
				ch_cfg.Bw = BW_80;
				ch_cfg.CentralChannel =
				MT7992_PER_CH_A6_BW80[i - MT7992_PER_CH_A6_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7992_PER_CH_A6_BW80[i - MT7992_PER_CH_A6_BW20_SIZE];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7992_PER_CH_A6_BW160[i - MT7992_PER_CH_A6_BW20_SIZE - MT7992_PER_CH_A6_BW80_SIZE];
				ch_cfg.ControlChannel =
				MT7992_PER_CH_A6_BW160[i - MT7992_PER_CH_A6_BW20_SIZE - MT7992_PER_CH_A6_BW80_SIZE];
				ch_cfg.eht_cen_ch =
				MT7992_PER_CH_A6_BW160[i - MT7992_PER_CH_A6_BW20_SIZE - MT7992_PER_CH_A6_BW80_SIZE];
				}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 1;
			ch_cfg.Channel_Band = 2;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = pAd->Antenna.field.RxPath;
				ch_cfg.RxStream = pAd->Antenna.field.TxPath;
			} else {
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
#endif
				ch_cfg.TxStream = pAd->Antenna.field.RxPath;
				ch_cfg.RxStream = pAd->Antenna.field.TxPath;
			}
			UniCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			ch_cfg.TxStream = pAd->Antenna.field.RxPath;
			ch_cfg.RxStream = pAd->Antenna.field.TxPath;

			UniCmdSetTxRxPath(pAd, ch_cfg);

			if (ch_cfg.Bw < BW_80)
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, ch_cfg.BandIdx);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6_MEM, ch_cfg.BandIdx);
		}

		chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

		doCal1 |= (1 << DPD6G_PRECAL_INDN_BIT);
		chip_ops->eewrite(pAd, PRECAL_INDICATION_BYTE, doCal1);

	} else if (op == PREK_DPD_2G_PROC) {
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_g_begin, cap->prek_ee_info.dpd_cal_2g_total_size);

		/* 2.4G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug */
		/* we can use only ch6 to verify and need pay attention to the index */
		for (i = 0; i < MT7992_PER_CH_G_BW20_SIZE; i++) {
			/* set channel command */
			/* per group calibration - set to channel 1, 6, 11, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7992_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel = MT7992_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bScan = 1;
			ch_cfg.Channel_Band = 0;

			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = pAd->Antenna.field.RxPath;
				ch_cfg.RxStream = pAd->Antenna.field.TxPath;
			} else {
#ifdef DBDC_MODE
				band0_tx_path_backup = pAd->dbdc_band0_tx_path;
				band0_rx_path_backup = pAd->dbdc_band0_rx_path;
#endif
				ch_cfg.TxStream = pAd->Antenna.field.RxPath;
				ch_cfg.RxStream = pAd->Antenna.field.TxPath;
			}

			UniCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band0_tx_path = band0_tx_path_backup;
				pAd->dbdc_band0_rx_path = band0_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			ch_cfg.TxStream = pAd->Antenna.field.RxPath;
			ch_cfg.RxStream = pAd->Antenna.field.TxPath;

			UniCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL, ch_cfg.BandIdx);
		}

		chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

		doCal1 |= (1 << DPD2G_PRECAL_INDN_BIT);
		chip_ops->eewrite(pAd, PRECAL_INDICATION_BYTE, doCal1);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"Should not be here !\n");
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"op=%d, 0x%2x=%x\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}

#endif /* PRE_CAL_MT7992_SUPPORT */
#endif /* CONFIG_WLAN_SERVICE */

static UINT32 mt7992_ra_init(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry)
{
	RAInit(pAd, pEntry);
	return FALSE;
}

static VOID mt7992_cninfo_get(RTMP_ADAPTER *pAd, uint8_t ucBandIdx, uint16_t *pCnInfo)
{
	uint16_t cninfo;

	MtCmdGetCnInfo(pAd, ucBandIdx, &cninfo);

	(*pCnInfo) = cninfo;
}

MT_MIB_COUNTER_STAT               _rPrevMibCnt;
P_MT_MIB_COUNTER_STAT             _prPrevMibCnt = &_rPrevMibCnt;

VOID mt7992_update_mib_bucket(RTMP_ADAPTER *pAd)
{
	UCHAR band_idx = 0;
	UCHAR curr_idx = 0;
	RTMP_MIB_PAIR Reg[UNI_CMD_MIB_MAX_PAIR];
	ULONG update_duration = 0;
#ifdef WIFI_UNIFIED_COMMAND
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(&Reg, sizeof(Reg));
	pAd->MsMibBucket.CurIdx++;
	if (pAd->MsMibBucket.CurIdx >= 2)
		pAd->MsMibBucket.CurIdx = 0;

	curr_idx = pAd->MsMibBucket.CurIdx;
	band_idx = hc_get_hw_band_idx(pAd);

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "Invalid band_idx(%d), maximun band idx is %d!\n",
			 band_idx, CFG_WIFI_RAM_BAND_NUM);
		return;
	}

	if (pAd->MsMibBucket.Enabled == TRUE) {
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			Reg[0].Counter = UNI_CMD_RMAC_CNT_OBSS_AIRTIME;
			Reg[1].Counter = UNI_CMD_MIB_CNT_TX_DUR_CNT;
			Reg[2].Counter = UNI_CMD_MIB_CNT_RX_DUR_CNT;
			Reg[3].Counter = UNI_CMD_RMAC_CNT_NONWIFI_AIRTIME;
			Reg[4].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;
			Reg[5].Counter = UNI_CMD_MIB_CNT_P_CCA_TIME;
			Reg[6].Counter = UNI_CMD_MIB_CNT_RX_CCK_MDRDY_TIME;
			Reg[7].Counter = UNI_CMD_MIB_CNT_RX_OFDM_LG_MIXED_MDRDY_TIME;
			Reg[8].Counter = UNI_CMD_MIB_CNT_RX_OFDM_GREEN_MDRDY_TIME;
			Reg[9].Counter = UNI_CMD_MIB_CNT_MAC2PHY_TX_TIME;
			Reg[10].Counter = UNI_CMD_MIB_CNT_BSS0_CTRL_FRAME_CNT;
			Reg[11].Counter = UNI_CMD_MIB_CNT_AMPDU;
			UniCmdMib(pAd, band_idx, Reg, 12);
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			Reg[0].Counter = RMAC_CNT_OBSS_AIRTIME;/* RMAC.AIRTIME14 OBSS Air time */
			Reg[1].Counter = MIB_CNT_TX_DUR_CNT;/* M0SDR36 TX Air time */
			Reg[2].Counter = MIB_CNT_RX_DUR_CNT;/* M0SDR37 RX Air time */
			Reg[3].Counter = RMAC_CNT_NONWIFI_AIRTIME;/*AIRTIME13 Non Wifi Air time */
			Reg[4].Counter = MIB_CNT_CCA_NAV_TX_TIME;/* M0SDR9 Channel Busy Time */
			Reg[5].Counter = MIB_CNT_P_CCA_TIME;/* M0SDR16 Primary Channel Busy Time */
			MtCmdMultipleMibRegAccessRead(pAd, band_idx, Reg, 6);
		}

		NdisGetSystemUpTime(&pAd->MsMibBucket.UpdateSystemTime[curr_idx]);
		if (_prPrevMibCnt->UpdateSystemTime[band_idx] != 0)
			update_duration = pAd->MsMibBucket.UpdateSystemTime[curr_idx] - _prPrevMibCnt->UpdateSystemTime[band_idx];

		if (update_duration != 0) {
			/*Make sure the measurement period of updated value is 500ms*/
			pAd->MsMibBucket.OBSSAirtime[curr_idx] =
				(UINT32)((Reg[0].Value - _prPrevMibCnt->ObssAirtimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.MyTxAirtime[curr_idx] =
				(UINT32)((Reg[1].Value - _prPrevMibCnt->MyTxAirtimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.MyRxAirtime[curr_idx] =
				(UINT32)((Reg[2].Value - _prPrevMibCnt->MyRxAirtimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.EDCCAtime[curr_idx] =
				(UINT32)((Reg[3].Value - _prPrevMibCnt->EdccaAirtimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[curr_idx] =
				(UINT32)((Reg[4].Value - _prPrevMibCnt->CcaNavTxTimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.ChannelBusyTime[curr_idx] =
				(UINT32)((Reg[5].Value - _prPrevMibCnt->PCcaTimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;

			pAd->MsMibBucket.RxCckMdrdyTime[curr_idx] =
				(UINT32)((Reg[6].Value - _prPrevMibCnt->RxCckMdrdyTimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.RxOfdmLgMixedMdrdyTime[curr_idx] =
				(UINT32)((Reg[7].Value - _prPrevMibCnt->RxOfdmLgMixedMdrdyTimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.RxOfdmGreenMdrdyTime[curr_idx] =
				(UINT32)((Reg[8].Value - _prPrevMibCnt->RxOfdmGreenMdrdyTimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.MyMac2phyTxTime[curr_idx] =
				(UINT32)((Reg[9].Value - _prPrevMibCnt->MyMac2phyTxTimeAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;

			pAd->MsMibBucket.MibBss0CtrlFrameCnt[curr_idx] =
				(UINT32)((Reg[10].Value - _prPrevMibCnt->MibBss0CtrlFrameCntAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
			pAd->MsMibBucket.MibCntAmpdu[curr_idx] =
				(UINT32)((Reg[11].Value - _prPrevMibCnt->MibCntAmpduAcc[band_idx])) * (500 * OS_HZ / 1000) / update_duration;
		} else {
			pAd->MsMibBucket.OBSSAirtime[curr_idx] =
				(UINT32)(Reg[0].Value - _prPrevMibCnt->ObssAirtimeAcc[band_idx]);
			pAd->MsMibBucket.MyTxAirtime[curr_idx] =
				(UINT32)(Reg[1].Value - _prPrevMibCnt->MyTxAirtimeAcc[band_idx]);
			pAd->MsMibBucket.MyRxAirtime[curr_idx] =
				(UINT32)(Reg[2].Value - _prPrevMibCnt->MyRxAirtimeAcc[band_idx]);
			pAd->MsMibBucket.EDCCAtime[curr_idx] =
				(UINT32)(Reg[3].Value - _prPrevMibCnt->EdccaAirtimeAcc[band_idx]);
			pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[curr_idx] =
				(UINT32)(Reg[4].Value - _prPrevMibCnt->CcaNavTxTimeAcc[band_idx]);
			pAd->MsMibBucket.ChannelBusyTime[curr_idx] =
				(UINT32)(Reg[5].Value - _prPrevMibCnt->PCcaTimeAcc[band_idx]);

			pAd->MsMibBucket.RxCckMdrdyTime[curr_idx] =
				(UINT32)(Reg[6].Value - _prPrevMibCnt->RxCckMdrdyTimeAcc[band_idx]);
			pAd->MsMibBucket.RxOfdmLgMixedMdrdyTime[curr_idx] =
				(UINT32)(Reg[7].Value - _prPrevMibCnt->RxOfdmLgMixedMdrdyTimeAcc[band_idx]);
			pAd->MsMibBucket.RxOfdmGreenMdrdyTime[curr_idx] =
				(UINT32)(Reg[8].Value - _prPrevMibCnt->RxOfdmGreenMdrdyTimeAcc[band_idx]);
			pAd->MsMibBucket.MyMac2phyTxTime[curr_idx] =
				(UINT32)(Reg[9].Value - _prPrevMibCnt->MyMac2phyTxTimeAcc[band_idx]);

			pAd->MsMibBucket.MibBss0CtrlFrameCnt[curr_idx] =
				(UINT32)(Reg[10].Value - _prPrevMibCnt->MibBss0CtrlFrameCntAcc[band_idx]);
			pAd->MsMibBucket.MibCntAmpdu[curr_idx] =
				(UINT32)(Reg[11].Value - _prPrevMibCnt->MibCntAmpduAcc[band_idx]);

		}

		_prPrevMibCnt->ObssAirtimeAcc[band_idx] = Reg[0].Value;
		_prPrevMibCnt->MyTxAirtimeAcc[band_idx] = Reg[1].Value;
		_prPrevMibCnt->MyRxAirtimeAcc[band_idx] = Reg[2].Value;
		_prPrevMibCnt->EdccaAirtimeAcc[band_idx] = Reg[3].Value;
		_prPrevMibCnt->CcaNavTxTimeAcc[band_idx] = Reg[4].Value;
		_prPrevMibCnt->PCcaTimeAcc[band_idx] = Reg[5].Value;
		_prPrevMibCnt->RxCckMdrdyTimeAcc[band_idx] = Reg[6].Value;
		_prPrevMibCnt->RxOfdmLgMixedMdrdyTimeAcc[band_idx] = Reg[7].Value;
		_prPrevMibCnt->RxOfdmGreenMdrdyTimeAcc[band_idx] = Reg[8].Value;
		_prPrevMibCnt->MyMac2phyTxTimeAcc[band_idx] = Reg[9].Value;
		_prPrevMibCnt->MibBss0CtrlFrameCntAcc[band_idx] = Reg[10].Value;
		_prPrevMibCnt->MibCntAmpduAcc[band_idx] = Reg[11].Value;
		_prPrevMibCnt->UpdateSystemTime[band_idx] = pAd->MsMibBucket.UpdateSystemTime[curr_idx];
	}
}

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID mt7922_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat)
{
	RTMP_MIB_PAIR Reg[8];
	P_MT_MIB_COUNTER_STAT pChStat = (P_MT_MIB_COUNTER_STAT)ChStat;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	Reg[0].Counter = UNI_CMD_RMAC_CNT_OBSS_AIRTIME;/* RMAC.AIRTIME14 OBSS Air time */
	Reg[1].Counter = UNI_CMD_MIB_CNT_TX_DUR_CNT;/* M0SDR36 TX Air time */
	Reg[2].Counter = UNI_CMD_MIB_CNT_RX_DUR_CNT;/* M0SDR37 RX Air time */
	Reg[3].Counter = UNI_CMD_RMAC_CNT_NONWIFI_AIRTIME;/* RMAC.AIRTIME13 Non Wifi Air time */
	Reg[4].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;/* M0SDR9 Channel Busy Time */
	Reg[5].Counter = UNI_CMD_MIB_CNT_P_CCA_TIME;/* M0SDR16 Primary Channel Busy Time */
	Reg[6].Counter = UNI_CMD_MIB_CNT_MAC2PHY_TX_TIME;/* M0SDR35 MAC2PHY Tx Time */
	Reg[7].Counter = UNI_CMD_MIB_CNT_BA_CNT;/* M0SDR31 BA Count */

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMib(pAd, BandIdx, Reg, 8);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMibRegAccessRead(pAd, BandIdx, Reg, 8);

	pChStat->ObssAirtimeAcc[BandIdx] = (UINT32)(Reg[0].Value);
	pChStat->MyTxAirtimeAcc[BandIdx] = (UINT32)(Reg[1].Value);
	pChStat->MyRxAirtimeAcc[BandIdx] = (UINT32)(Reg[2].Value);
	pChStat->EdccaAirtimeAcc[BandIdx] = (UINT32)(Reg[3].Value);
	pChStat->CcaNavTxTimeAcc[BandIdx] = (UINT32)(Reg[4].Value);
	pChStat->PCcaTimeAcc[BandIdx] = (UINT32)(Reg[5].Value);
	pChStat->MyMac2PhyTxTimeAcc[BandIdx] = (UINT32)(Reg[6].Value);
	pChStat->BACountAcc[BandIdx] = (UINT32)(Reg[7].Value);
}
#endif

VOID mt7992_ctrl_rxv_group(RTMP_ADAPTER *ad, uint8_t band_idx, uint8_t group, uint8_t enable)
{
	UINT32 cr_addr = BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_ADDR+(band_idx*0x10000);
	UINT32 cr_mask = 0xffffffff, cr_value = 0;

	switch (group) {
	case FMAC_RXV_GROUP1:
		cr_mask = (0x1 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	case FMAC_RXV_GROUP2:
		cr_mask = (0x2 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	case FMAC_RXV_GROUP3:
		cr_mask = (0x4 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	case FMAC_RXV_GROUP5:
		cr_mask = (0x8 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	default:
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "Unknown group(%d), ignored!\n", group);
		break;
	}

	if (~cr_mask != 0) {
		MAC_IO_READ32(ad->hdev_ctrl, cr_addr, &cr_value);
		if (enable)
			MAC_IO_WRITE32(ad->hdev_ctrl, cr_addr, (cr_value | cr_mask));
		else
			MAC_IO_WRITE32(ad->hdev_ctrl, cr_addr, (cr_value & ~cr_mask));
	}
}

extern RTMP_STRING *get_dev_eeprom_binary(VOID *pvAd);

const UCHAR *mt7992_get_default_bin_image(
	struct physical_device *ph_dev)
{
#ifdef CONFIG_CPE_SUPPORT
	MTWF_PRINT("Use the default ePAeLNA bin image!\n");
	return MT7992_E2PImage_ePAeLNA;
#else
	/* TODO - To get correct bin image by physical device index. @20220330 */
	{
		MTWF_PRINT("Use the default iPAiLNA bin image!\n");
		return MT7992_E2PImage_iPAiLNA;
	}
#endif
	return NULL;
}


INT32 mt7992_get_default_bin_image_file(
	RTMP_ADAPTER *ad, RTMP_STRING *path, BOOLEAN fgBinMode)
{
	INT ret = 0;
#if !defined(MT7992_SKU_BE3600SDB)
	uint32_t sku = MT7992_SKU_INVALID;
#endif

	if (fgBinMode) {
#ifdef CONFIG_CPE_SUPPORT
		if ((get_dev_eeprom_binary(ad) != NULL) &&
			(strlen(get_dev_eeprom_binary(ad)) > 0))
			ret = snprintf(path, 100, "/mnt/vendor/nvcfg/%s", get_dev_eeprom_binary(ad));
#else
		if ((get_dev_eeprom_binary(ad) != NULL) &&
			(strlen(get_dev_eeprom_binary(ad)) > 0))
			ret = snprintf(path, 100, "/lib/firmware/%s", get_dev_eeprom_binary(ad));
		else
			ret = snprintf(path, 100, "%s", "/lib/firmware/e2p");
#endif
	} else {/* fill default for bin mode */
#if defined(MT7992_SKU_BE3600SDB)
		if ((get_dev_eeprom_default_bin_path(ad) != NULL) &&
			(strlen(get_dev_eeprom_default_bin_path(ad)) > 0)) {
			MTWF_PRINT("get_dev_eeprom_default_bin_path: %s\n", get_dev_eeprom_default_bin_path(ad));
			ret = snprintf(path, 100, "%s", get_dev_eeprom_default_bin_path(ad));
		}
#else
		sku = mt7992_get_sku_decision(ad);
		switch (sku) {
		case MT7992_SKU_BE7200_MT7975_MT7979:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_7200_7975_7979_FILE_PATH);
			break;
		case MT7992_SKU_BE6500_MT7978_MT7979:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_6500_7978_7979_FILE_PATH);
			break;
		case MT7992_SKU_BE7200_MT7976G_MT7977:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_7200_7976G_7977_FILE_PATH);
			break;
		case MT7992_SKU_BE7200_MT7975_MT7977:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_7200_7975_7977_FILE_PATH);
			break;
		case MT7992_SKU_BE5040_MT7976C:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_5040_7976C_FILE_PATH);
			break;
		case MT7992_SKU_BE5040_MT7976DA:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_5040_7976DA_FILE_PATH);
			break;
		default:
			ret = snprintf(path, 100, "%s", MT7992_EEPROM_DEFAULT_FILE_PATH);
			break;
		}
#endif
	}
	if (ret < 0 || ret >= 100)
		MTWF_PRINT("Unexpected error with default BIN.\n");
	else
		MTWF_PRINT("Use default BIN from:%s.\n", path);

	return 0;
}

INT32 mt7992_get_prek_image_file(RTMP_ADAPTER *ad, RTMP_STRING *path)
{
	INT ret;

	if ((get_dev_eeprom_binary(ad) != NULL) &&
		(strlen(get_dev_eeprom_binary(ad)) > 0))
		ret = snprintf(path, 100, "/lib/firmware/%s", get_dev_eeprom_binary(ad));
	else
		ret = snprintf(path, 100, "%s", "/lib/firmware/e2p");

	if (ret < 0 || ret >= 100)
		MTWF_PRINT("Unexpected error with PreCal BIN.\n");
	else
		MTWF_PRINT("Use PreCal BIN from:%s.\n", path);

	return 0;
}

UINT32 mt7992_get_efuse_free_blk_num(RTMP_ADAPTER *ad, uint8_t blk_section)
{
	UINT32 ret = 0;
	struct _EXT_CMD_EFUSE_FREE_BLOCK_T cmd;
	struct _EXT_EVENT_EFUSE_FREE_BLOCK_V1_T rsp;

	memset(&cmd, 0, sizeof(cmd));
	memset(&rsp, 0, sizeof(rsp));
	cmd.ucVersion = 1;
	cmd.ucDieIndex = blk_section;
	if (MtCmdEfuseFreeBlockCount(ad,
				     (PVOID)&cmd,
				     (PVOID)&rsp) == NDIS_STATUS_SUCCESS)
		ret = rsp.ucFreeBlockNum;
	else
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
			 "(%s) cmd failed!\n", __func__);

	return ret;
}

#ifdef LED_CONTROL_SUPPORT
VOID mt7992_wps_led_init(RTMP_ADAPTER *pAd)
{
	/*for others init, TBD*/
}

UCHAR mt7992_wps_led_control(RTMP_ADAPTER *pAd, UCHAR flag)
{
	return 0;
}
#endif /*LED_CONTROL_SUPPORT*/

#ifdef WIFI_UNIFIED_COMMAND
static VOID mt7992_update_ampdu_raw_counters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	/* for PER debug */
	UINT32 rx_fcs_error_cnt = 0;
	UINT32 rx_fifo_full_cnt = 0;
	UINT32 rx_mdrdy_cnt = 0;
	UINT32 old_val;
	COUNTER_802_3 *dot3Counters;
	COUNTER_802_11 *wlanCounter;
	RTMP_MIB_PAIR Reg[10];

	os_zero_mem(Reg, sizeof(Reg));

	dot3Counters = &pAd->Counters8023;
	wlanCounter = &pAd->WlanCounters;

	if (pAd->partial_mib_show_en == 1) {
		Reg[0].Counter = UNI_CMD_MIB_CNT_RX_FCS_ERR;
		Reg[1].Counter = UNI_CMD_MIB_CNT_RX_FIFO_OVERFLOW;
		Reg[2].Counter = UNI_CMD_MIB_CNT_RX_MPDU;
		Reg[3].Counter = UNI_CMD_MIB_CNT_CHANNEL_IDLE;
		Reg[4].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;
		Reg[5].Counter = UNI_CMD_MIB_CNT_MDRDY;
		Reg[6].Counter = UNI_CMD_MIB_CNT_S_CCA_TIME;
		Reg[7].Counter = UNI_CMD_MIB_CNT_P_ED_TIME;
		Reg[8].Counter = UNI_CMD_MIB_CNT_RX_TOTAL_BYTE;

		UniCmdMib(pAd, BandIdx, Reg, 9);

		wlanCounter->RxFcsErrorCount.u.LowPart = Reg[0].Value;
		wlanCounter->RxFifoFullCount.u.LowPart = Reg[1].Value;
		wlanCounter->RxMpduCount.QuadPart = Reg[2].Value;
		wlanCounter->ChannelIdleCount.QuadPart = Reg[3].Value;
		wlanCounter->CcaNavTxTime.QuadPart = Reg[4].Value;
		wlanCounter->RxMdrdyCount.QuadPart = Reg[5].Value;
		wlanCounter->SCcaTime.QuadPart = Reg[6].Value;
		wlanCounter->PEdTime.QuadPart = Reg[7].Value;
		wlanCounter->RxTotByteCount.QuadPart = Reg[8].Value;
	} else {
		Reg[0].Counter = UNI_CMD_MIB_CNT_RX_MPDU;
		Reg[1].Counter = UNI_CMD_MIB_CNT_RX_TOTAL_BYTE;
		Reg[2].Counter = UNI_CMD_MIB_CNT_RX_FCS_ERR;
		Reg[3].Counter = UNI_CMD_MIB_CNT_RX_FIFO_OVERFLOW;
		Reg[4].Counter = UNI_CMD_MIB_CNT_MDRDY;
		Reg[5].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;

		UniCmdMib(pAd, BandIdx, Reg, 6);

		wlanCounter->RxMpduCount.QuadPart = Reg[0].Value;
		wlanCounter->RxTotByteCount.QuadPart = Reg[1].Value;
		rx_fcs_error_cnt = Reg[2].Value;
		wlanCounter->RxFcsErrorCount.u.LowPart = Reg[2].Value;
		rx_fifo_full_cnt = Reg[3].Value;
		wlanCounter->RxFifoFullCount.u.LowPart = Reg[3].Value;
		rx_mdrdy_cnt = Reg[4].Value;
		wlanCounter->RxMdrdyCount.u.LowPart = Reg[4].Value;
		wlanCounter->CcaNavTxTime.QuadPart = Reg[5].Value;
	}

	if (pAd->parse_rxv_stat_enable) {
		/* Used for rx statistic */
		if (BandIdx == 0) {
			pAd->AccuOneSecRxBand0FcsErrCnt = rx_fcs_error_cnt;
			pAd->AccuOneSecRxBand0MdrdyCnt = rx_mdrdy_cnt;
		} else {
			pAd->AccuOneSecRxBand1FcsErrCnt = rx_fcs_error_cnt;
			pAd->AccuOneSecRxBand1MdrdyCnt = rx_mdrdy_cnt;
		}
	}

#ifdef STATS_COUNT_SUPPORT
	old_val = wlanCounter->FCSErrorCount.u.LowPart;
	wlanCounter->FCSErrorCount.u.LowPart = rx_fcs_error_cnt;

	if (wlanCounter->FCSErrorCount.u.LowPart < old_val)
		wlanCounter->FCSErrorCount.u.HighPart++;
#endif

	dot3Counters->RxNoBuffer = rx_fifo_full_cnt;
}
#endif /* #ifdef WIFI_UNIFIED_COMMAND */

#ifdef CONFIG_STA_SUPPORT
static VOID init_dev_nick_name(RTMP_ADAPTER *ad)
{
	int ret;

	ret = snprintf((RTMP_STRING *) ad->nickname, sizeof(ad->nickname), "mt7992_sta");
	if (os_snprintf_error(sizeof(ad->nickname), ret)) {
		MTWF_DBG(ad, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"nickname init error!\n");
		return;
	}
}
#endif /* CONFIG_STA_SUPPORT */


#ifdef TXBF_SUPPORT
static VOID chip_dump_pfmu_tag1(union txbf_pfmu_tag1 *pfmu_tag1)
{
	MTWF_PRINT(
		"============================= TxBf profile Tage1 Info ========================================\n"
		"Row data0 = 0x%08x, Row data1 = 0x%08x, Row data2 = 0x%08x, Row data3 = 0x%08x\n"
		"Row data4 = 0x%08x, Row data5 = 0x%08x, Row data6 = 0x%08x\n"
		"\n"
		"PFMU ID = %d		Invalid status = %d\n"
		"iBf/eBf = %d\n"
		"\n"
		"DBW   = %d\n"
		"SU/MU = %d\n"
		"nrow=%d, ncol=%d, ng=%d, LM=%d, CodeBook=%d MobCalEn=%d\n",
		pfmu_tag1->raw_data[0], pfmu_tag1->raw_data[1], pfmu_tag1->raw_data[2], pfmu_tag1->raw_data[3],
		pfmu_tag1->raw_data[4], pfmu_tag1->raw_data[5], pfmu_tag1->raw_data[6],
		pfmu_tag1->field.profile_id, pfmu_tag1->field.invalid_prof,
		pfmu_tag1->field.txbf,
		pfmu_tag1->field.dbw,
		pfmu_tag1->field.su_mu,
		pfmu_tag1->field.nrow, pfmu_tag1->field.ncol, pfmu_tag1->field.ngroup, pfmu_tag1->field.lm,
		pfmu_tag1->field.codebook, pfmu_tag1->field.mob_cal_en);

	if (pfmu_tag1->field.lm <= 3) {
		MTWF_PRINT("RU start = %d, RU end = %d\n",
			pfmu_tag1->field_var.ru_start_id, pfmu_tag1->field_var.ru_end_id);
	} else {
		MTWF_PRINT("PartialBw = %d\n",
			pfmu_tag1->field.partial_bw_info);
	}

	MTWF_PRINT("\n"
		"Mem Col1 = %d, Mem Row1 = %d, Mem Col2 = %d, Mem Row2 = %d\n"
		"Mem Col3 = %d, Mem Row3 = %d, Mem Col4 = %d, Mem Row4 = %d\n"
		"\n"
		"STS0_SNR =0x%02x, STS1_SNR=0x%02x, STS2_SNR=0x%02x, STS3_SNR=0x%02x\n"
		"STS4_SNR =0x%02x, STS5_SNR=0x%02x, STS6_SNR=0x%02x, STS7_SNR=0x%02x\n"
		"==============================================================================================\n",
		pfmu_tag1->field.mem_addr1_col_id, pfmu_tag1->field.mem_addr1_row_id,
		pfmu_tag1->field.mem_addr2_col_id, pfmu_tag1->field.mem_addr2_row_id,
		pfmu_tag1->field.mem_addr3_col_id, pfmu_tag1->field.mem_addr3_row_id,
		pfmu_tag1->field.mem_addr4_col_id, pfmu_tag1->field.mem_addr4_row_id,
		pfmu_tag1->field.snr_sts0, pfmu_tag1->field.snr_sts1,
		pfmu_tag1->field.snr_sts2, pfmu_tag1->field.snr_sts3,
		pfmu_tag1->field.snr_sts4, pfmu_tag1->field.snr_sts5,
		pfmu_tag1->field.snr_sts6, pfmu_tag1->field.snr_sts7);
}

static VOID chip_dump_pfmu_tag2(union txbf_pfmu_tag2 *pfmu_tag2)
{
	MTWF_PRINT("============================= TxBf profile Tage2 Info ========================================\n"
		"Row data0 = 0x%08x, Row data1 = 0x%08x, Row data2 = 0x%08x, Row data3 = 0x%08x\n"
		"Row data4 = 0x%08x, Row data5 = 0x%08x, Row data6 = 0x%08x\n"
		"\n"
		"Smart antenna ID = 0x%x,  SE index = %d\n"
		"Timeout = 0x%x\n"
		"Desired BW = %d, Desired Ncol = %d, Desired Nrow = %d\n"
#ifdef IBF_BITMAP_SUPPORT
		"Desired BW bitmap = 0x%x\n"
#else
		"Desired RU Allocation = %d\n"
#endif
		"Mobility DeltaT = %d, Mobility LQ = %d\n"
		"==============================================================================================\n",
		pfmu_tag2->raw_data[0], pfmu_tag2->raw_data[1], pfmu_tag2->raw_data[2], pfmu_tag2->raw_data[3],
		pfmu_tag2->raw_data[4], pfmu_tag2->raw_data[5], pfmu_tag2->raw_data[6],
		pfmu_tag2->field.smart_ant, pfmu_tag2->field.se_idx,
		pfmu_tag2->field.ibf_timeout, pfmu_tag2->field.ibf_dbw,
		pfmu_tag2->field.ibf_ncol, pfmu_tag2->field.ibf_nrow,
#ifdef IBF_BITMAP_SUPPORT
		pfmu_tag2->field.ibf_bw_bitmap,
#else
		pfmu_tag2->field.ibf_ru,
#endif
		pfmu_tag2->field.mob_delta_t, pfmu_tag2->field.mob_lq_result);
}

static VOID txbf_dump_tag(struct _RTMP_ADAPTER *pAd, uint8_t fgBFer, uint8_t *pBuf)
{
	struct txbf_pfmu_tags_info *tags_info = &pAd->pfmu_tags_info;
	union txbf_pfmu_tag1 *pfmu_tag1;
	union txbf_pfmu_tag2 *pfmu_tag2;

	pfmu_tag1 = (union txbf_pfmu_tag1 *) pBuf;
	pfmu_tag2 = (union txbf_pfmu_tag2 *) (pBuf + sizeof(*pfmu_tag1));
#ifdef CFG_BIG_ENDIAN
	RTMPEndianChange((char *)pfmu_tag1, sizeof(*pfmu_tag1));
	RTMPEndianChange((char *)pfmu_tag2, sizeof(*pfmu_tag2));
#endif

	/* cache tag1 */
	tags_info->pfmu_idx = pfmu_tag1->field.profile_id;
	tags_info->ebf = pfmu_tag1->field.txbf;
	tags_info->dbw = pfmu_tag1->field.dbw;
	tags_info->lm = pfmu_tag1->field.lm;
	tags_info->su_mu = pfmu_tag1->field.su_mu;
	tags_info->nr = pfmu_tag1->field.nrow;
	tags_info->nc = pfmu_tag1->field.ncol;
	tags_info->codebook = pfmu_tag1->field.codebook;
	tags_info->ng = pfmu_tag1->field.ngroup;
	tags_info->invalid_prof = pfmu_tag1->field.invalid_prof;

	tags_info->mem_col0 = pfmu_tag1->field.mem_addr1_col_id;
	tags_info->mem_row0 = pfmu_tag1->field.mem_addr1_row_id;
	tags_info->mem_col1 = pfmu_tag1->field.mem_addr2_col_id;
	tags_info->mem_row1 = pfmu_tag1->field.mem_addr2_row_id;
	tags_info->mem_col2 = pfmu_tag1->field.mem_addr3_col_id;
	tags_info->mem_row2 = pfmu_tag1->field.mem_addr3_row_id;
	tags_info->mem_col3 = pfmu_tag1->field.mem_addr4_col_id;
	tags_info->mem_row3 = pfmu_tag1->field.mem_addr4_row_id;

	tags_info->ru_start = pfmu_tag1->field_var.ru_start_id;
	tags_info->ru_end = pfmu_tag1->field_var.ru_end_id;
	tags_info->partial_bw_info = pfmu_tag1->field.partial_bw_info;
	tags_info->mobility_cal_en = pfmu_tag1->field.mob_cal_en;
	tags_info->snr_sts0 = pfmu_tag1->field.snr_sts0;
	tags_info->snr_sts1 = pfmu_tag1->field.snr_sts1;
	tags_info->snr_sts2 = pfmu_tag1->field.snr_sts2;
	tags_info->snr_sts3 = pfmu_tag1->field.snr_sts3;
	tags_info->snr_sts4 = pfmu_tag1->field.snr_sts4;
	tags_info->snr_sts5 = pfmu_tag1->field.snr_sts5;
	tags_info->snr_sts6 = pfmu_tag1->field.snr_sts6;
	tags_info->snr_sts7 = pfmu_tag1->field.snr_sts7;

	/* cache tag2 */
	tags_info->smart_ant = pfmu_tag2->field.smart_ant;
	tags_info->se_idx = pfmu_tag2->field.se_idx;
	tags_info->ibf_timeout = pfmu_tag2->field.ibf_timeout;
	tags_info->ibf_desired_dbw = pfmu_tag2->field.ibf_dbw;
	tags_info->ibf_desired_ncol = pfmu_tag2->field.ibf_ncol;
	tags_info->ibf_desired_nrow = pfmu_tag2->field.ibf_nrow;
#ifdef IBF_BITMAP_SUPPORT
	tags_info->ibf_desired_bw_bitmap = pfmu_tag2->field.ibf_bw_bitmap;
#else
	tags_info->ibf_desired_ru_alloc = pfmu_tag2->field.ibf_ru;
#endif

	chip_dump_pfmu_tag1(pfmu_tag1);

	if (fgBFer == TRUE)
		chip_dump_pfmu_tag2(pfmu_tag2);
}

static void setETxBFCap(
	IN  RTMP_ADAPTER	  *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	HT_BF_CAP *pTxBFCap = pTxBfInfo->pHtTxBFCap;

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->RxNDPCapable		 = TRUE;
			pTxBFCap->TxNDPCapable		 = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF		   = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF			 =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping		  = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup	 = 3;
			pTxBFCap->TxSoundCapable	   = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation	   = (pTxBfInfo->ucRxPathNum <= 4) ? (pTxBfInfo->ucRxPathNum - 1) : 3;
			break;

		case SUBF_BFER:
			pTxBFCap->RxNDPCapable		 = FALSE;
			pTxBFCap->TxNDPCapable		 = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF		   = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF			 =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping		  = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup	 = 3;
			pTxBFCap->TxSoundCapable	   = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation	   = (pTxBfInfo->ucRxPathNum <= 4) ? (pTxBfInfo->ucRxPathNum - 1) : 3;
			break;

		case SUBF_BFEE:
			pTxBFCap->RxNDPCapable		 = TRUE;
			pTxBFCap->TxNDPCapable		 = FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF		   = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF			 =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping		  = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup	 = 3;
			pTxBFCap->TxSoundCapable	   = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation	   = (pTxBfInfo->ucRxPathNum <= 4) ? (pTxBfInfo->ucRxPathNum - 1) : 3;
			break;
		}
	} else
		memset(pTxBFCap, 0, sizeof(*pTxBFCap));
}

static INT chip_set_txbf_pfmu_tag(struct hdev_ctrl *ctrl, enum txbf_pfmu_tag idx, UINT32 val)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	struct txbf_pfmu_tags_info *tags_info = &pAd->pfmu_tags_info;

	switch (idx) {
	case TAG1_PFMU_ID:
		tags_info->pfmu_idx = val;
		break;
	case TAG1_IEBF:
		tags_info->ebf = val;
		break;
	case TAG1_DBW:
		tags_info->dbw = val;
		break;
	case TAG1_SU_MU:
		tags_info->su_mu = val;
		break;
	case TAG1_INVALID:
		tags_info->invalid_prof = val;
		break;
	case TAG1_MEM_ROW0:
		tags_info->mem_row0 = val;
		break;
	case TAG1_MEM_ROW1:
		tags_info->mem_row1 = val;
		break;
	case TAG1_MEM_ROW2:
		tags_info->mem_row2 = val;
		break;
	case TAG1_MEM_ROW3:
		tags_info->mem_row3 = val;
		break;
	case TAG1_MEM_COL0:
		tags_info->mem_col0 = val;
		break;
	case TAG1_MEM_COL1:
		tags_info->mem_col1 = val;
		break;
	case TAG1_MEM_COL2:
		tags_info->mem_col2 = val;
		break;
	case TAG1_MEM_COL3:
		tags_info->mem_col3 = val;
		break;
	case TAG1_RMSD:
		tags_info->rmsd = val;
		break;
	case TAG1_NR:
		tags_info->nr = val;
		break;
	case TAG1_NC:
		tags_info->nc = val;
		break;
	case TAG1_NG:
		tags_info->ng = val;
		break;
	case TAG1_LM:
		tags_info->lm = val;
		break;
	case TAG1_CODEBOOK:
		tags_info->codebook = val;
		break;
	case TAG1_HTC:
		tags_info->htc = val;
		break;
	case TAG1_RU_START:
		tags_info->ru_start = val;
		break;
	case TAG1_RU_END:
		tags_info->ru_end = val;
		break;
	case TAG1_MOB_CAL_EN:
		tags_info->mobility_cal_en = val;
		break;
	case TAG1_PARTIAL_BW:
		tags_info->partial_bw_info = val;
		break;
	case TAG1_SNR_STS0:
		tags_info->snr_sts0 = val;
		break;
	case TAG1_SNR_STS1:
		tags_info->snr_sts1 = val;
		break;
	case TAG1_SNR_STS2:
		tags_info->snr_sts2 = val;
		break;
	case TAG1_SNR_STS3:
		tags_info->snr_sts3 = val;
		break;
	case TAG1_SNR_STS4:
		tags_info->snr_sts4 = val;
		break;
	case TAG1_SNR_STS5:
		tags_info->snr_sts5 = val;
		break;
	case TAG1_SNR_STS6:
		tags_info->snr_sts6 = val;
		break;
	case TAG1_SNR_STS7:
		tags_info->snr_sts7 =  val;
		break;
	case TAG2_SE_ID:
		tags_info->se_idx = val;
		break;
	case TAG2_SMART_ANT:
		tags_info->smart_ant = val;
		break;
	case TAG2_RMSD_THRESHOLD:
		tags_info->rmsd_threshold = val;
		break;
	case TAG2_IBF_TIMEOUT:
		tags_info->ibf_timeout = val;
		break;
	case TAG2_IBF_DBW:
		tags_info->ibf_desired_dbw = val;
		break;
	case TAG2_IBF_NROW:
		tags_info->ibf_desired_nrow = val;
		break;
	case TAG2_IBF_NCOL:
		tags_info->ibf_desired_ncol = val;
		break;
	case TAG2_IBF_RU_ALLOC:
#ifdef IBF_BITMAP_SUPPORT
		tags_info->ibf_desired_bw_bitmap = val;
#else
		tags_info->ibf_desired_ru_alloc = val;
#endif
		break;
	default:
		break;
	}

	return TRUE;
}

static INT chip_write_txbf_pfmu_tag(struct hdev_ctrl *ctrl, uint8_t pf_idx)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	struct txbf_pfmu_tags_info *tags_info = &pAd->pfmu_tags_info;
	union txbf_pfmu_tag1 pfmu_tag1;
	union txbf_pfmu_tag2 pfmu_tag2;

	os_zero_mem(&pfmu_tag1, sizeof(pfmu_tag1));
	os_zero_mem(&pfmu_tag2, sizeof(pfmu_tag2));

	/* prepare tag1 */
	pfmu_tag1.field.profile_id = tags_info->pfmu_idx;
	pfmu_tag1.field.txbf = tags_info->ebf;
	pfmu_tag1.field.dbw = tags_info->dbw;
	pfmu_tag1.field.lm = tags_info->lm;
	pfmu_tag1.field.su_mu = tags_info->su_mu;
	pfmu_tag1.field.nrow = tags_info->nr;
	pfmu_tag1.field.ncol = tags_info->nc;
	pfmu_tag1.field.codebook = tags_info->codebook;
	pfmu_tag1.field.ngroup = tags_info->ng;
	pfmu_tag1.field.invalid_prof = tags_info->invalid_prof;

	pfmu_tag1.field.mem_addr1_col_id = tags_info->mem_col0;
	pfmu_tag1.field.mem_addr1_row_id = tags_info->mem_row0;
	pfmu_tag1.field.mem_addr2_col_id = tags_info->mem_col1;
	pfmu_tag1.field.mem_addr2_row_id = tags_info->mem_row1;
	pfmu_tag1.field.mem_addr3_col_id = tags_info->mem_col2;
	pfmu_tag1.field.mem_addr3_row_id = tags_info->mem_row2;
	pfmu_tag1.field.mem_addr4_col_id = tags_info->mem_col3;
	pfmu_tag1.field.mem_addr4_row_id = tags_info->mem_row3;

	if (pfmu_tag1.field.lm <= 3) {
		pfmu_tag1.field_var.ru_start_id = tags_info->ru_start;
		pfmu_tag1.field_var.ru_end_id = tags_info->ru_end;
	} else {
		pfmu_tag1.field.partial_bw_info = tags_info->partial_bw_info;
	}
	pfmu_tag1.field.mob_cal_en = tags_info->mobility_cal_en;
	pfmu_tag1.field.snr_sts0 = tags_info->snr_sts0;
	pfmu_tag1.field.snr_sts1 = tags_info->snr_sts1;
	pfmu_tag1.field.snr_sts2 = tags_info->snr_sts2;
	pfmu_tag1.field.snr_sts3 = tags_info->snr_sts3;
	pfmu_tag1.field.snr_sts4 = tags_info->snr_sts4;
	pfmu_tag1.field.snr_sts5 = tags_info->snr_sts5;
	pfmu_tag1.field.snr_sts6 = tags_info->snr_sts6;
	pfmu_tag1.field.snr_sts7 = tags_info->snr_sts7;

	/* prepare tag2 */
	pfmu_tag2.field.smart_ant = tags_info->smart_ant;
	pfmu_tag2.field.se_idx = tags_info->se_idx;
	pfmu_tag2.field.ibf_timeout = tags_info->ibf_timeout;
	pfmu_tag2.field.ibf_dbw = tags_info->ibf_desired_dbw;
	pfmu_tag2.field.ibf_ncol = tags_info->ibf_desired_ncol;
	pfmu_tag2.field.ibf_nrow = tags_info->ibf_desired_nrow;
#ifdef IBF_BITMAP_SUPPORT
	pfmu_tag2.field.ibf_bw_bitmap = tags_info->ibf_desired_bw_bitmap;
#else
	pfmu_tag2.field.ibf_ru = tags_info->ibf_desired_ru_alloc;
#endif

	if (CmdETxBfPfmuProfileTagWrite(pAd,
		(uint8_t *)(&pfmu_tag1), (uint8_t *)(&pfmu_tag2),
		sizeof(pfmu_tag1), sizeof(pfmu_tag2), pf_idx) == STATUS_TRUE) {
		/* dump tag1 */
		chip_dump_pfmu_tag1(&pfmu_tag1);
		/* dump tag2 */
		chip_dump_pfmu_tag2(&pfmu_tag2);
		return 1;
	}

	return 0;
}

VOID txbf_show_pfmu_data_nss4above_seg0(
	IN PRTMP_ADAPTER pAd,
	union txbf_bfer_pfmu_data *pfmu_data,
	IN USHORT subCarrIdx,
	IN UINT8 tx_path)
{
		MTWF_PRINT(
			"================ TxBf profile Data - Subcarrier Idx = %d(0x%03x) =================\n"
			"=============================== Seg0 Angles ==================================\n"
			"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x, Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
			"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x, Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
			subCarrIdx, subCarrIdx,
			pfmu_data->field.rSeg0Ang.field.psi41, pfmu_data->field.rSeg0Ang.field.phi31,
			pfmu_data->field.rSeg0Ang.field.psi31, pfmu_data->field.rSeg0Ang.field.phi21,
			pfmu_data->field.rSeg0Ang.field.psi21, pfmu_data->field.rSeg0Ang.field.phi11,
			pfmu_data->field.rSeg0Ang.field.psi43, pfmu_data->field.rSeg0Ang.field.phi33,
			pfmu_data->field.rSeg0Ang.field.psi42, pfmu_data->field.rSeg0Ang.field.phi32,
			pfmu_data->field.rSeg0Ang.field.psi32, pfmu_data->field.rSeg0Ang.field.phi22);

		if (tx_path > 4) {
			MTWF_PRINT(
				"Psi51 = 0x%02x, Phi41 = 0x%03x, Psi52 = 0x%02x, Phi42 = 0x%03x, Psi53 = 0x%02x, Phi43 = 0x%03x,\n"
				"Psi54 = 0x%02x, Phi44 = 0x%03x\n",
				pfmu_data->field.rSeg0Ang.field.psi51, pfmu_data->field.rSeg0Ang.field.phi41,
				pfmu_data->field.rSeg0Ang.field.psi52, pfmu_data->field.rSeg0Ang.field.phi42,
				pfmu_data->field.rSeg0Ang.field.psi53, pfmu_data->field.rSeg0Ang.field.phi43,
				pfmu_data->field.rSeg0Ang.field.psi54, pfmu_data->field.rSeg0Ang.field.phi43);
		}
		if (tx_path > 5) {
			MTWF_PRINT(
				"Psi61 = 0x%02x, Phi51 = 0x%03x, Psi62 = 0x%02x, Phi52 = 0x%03x, Psi63 = 0x%02x, Phi53 = 0x%03x,\n"
				"Psi64 = 0x%02x, Phi54 = 0x%03x, Psi65 = 0x%02x, Phi55 = 0x%03x\n",
				pfmu_data->field.rSeg0Ang.field.psi61, pfmu_data->field.rSeg0Ang.field.phi51,
				pfmu_data->field.rSeg0Ang.field.psi62, pfmu_data->field.rSeg0Ang.field.phi52,
				pfmu_data->field.rSeg0Ang.field.psi63, pfmu_data->field.rSeg0Ang.field.phi53,
				pfmu_data->field.rSeg0Ang.field.psi64, pfmu_data->field.rSeg0Ang.field.phi53,
				pfmu_data->field.rSeg0Ang.field.psi65, pfmu_data->field.rSeg0Ang.field.phi55);
		}
		if (tx_path > 6) {
			MTWF_PRINT(
				"Psi71 = 0x%02x, Phi61 = 0x%03x, Psi72 = 0x%02x, Phi62 = 0x%03x, Psi73 = 0x%02x, Phi63 = 0x%03x,\n"
				"Psi74 = 0x%02x, Phi64 = 0x%03x, Psi75 = 0x%02x, Phi65 = 0x%03x, Psi76 = 0x%02x, Phi66 = 0x%03x\n",
				pfmu_data->field.rSeg0Ang.field.psi71, pfmu_data->field.rSeg0Ang.field.phi61,
				pfmu_data->field.rSeg0Ang.field.psi72, pfmu_data->field.rSeg0Ang.field.phi62,
				pfmu_data->field.rSeg0Ang.field.psi73, pfmu_data->field.rSeg0Ang.field.phi63,
				pfmu_data->field.rSeg0Ang.field.psi74, pfmu_data->field.rSeg0Ang.field.phi63,
				pfmu_data->field.rSeg0Ang.field.psi75, pfmu_data->field.rSeg0Ang.field.phi65,
				pfmu_data->field.rSeg0Ang.field.psi76, pfmu_data->field.rSeg0Ang.field.phi66);
		}
		if (tx_path > 7) {
			MTWF_PRINT(
				"Psi81 = 0x%02x, Phi71 = 0x%03x, Psi82 = 0x%02x, Phi72 = 0x%03x, Psi83 = 0x%02x, Phi73 = 0x%03x,\n"
				"Psi84 = 0x%02x, Phi74 = 0x%03x, Psi85 = 0x%02x, Phi75 = 0x%03x, Psi86 = 0x%02x, Phi76 = 0x%03x,\n"
				"Psi87 = 0x%02x, Phi77 = 0x%03x\n",
				pfmu_data->field.rSeg0Ang.field.psi81, pfmu_data->field.rSeg0Ang.field.phi71,
				pfmu_data->field.rSeg0Ang.field.psi82, pfmu_data->field.rSeg0Ang.field.phi72,
				pfmu_data->field.rSeg0Ang.field.psi83, pfmu_data->field.rSeg0Ang.field.phi73,
				pfmu_data->field.rSeg0Ang.field.psi84, pfmu_data->field.rSeg0Ang.field.phi73,
				pfmu_data->field.rSeg0Ang.field.psi85, pfmu_data->field.rSeg0Ang.field.phi75,
				pfmu_data->field.rSeg0Ang.field.psi86, pfmu_data->field.rSeg0Ang.field.phi76,
				pfmu_data->field.rSeg0Ang.field.psi87, pfmu_data->field.rSeg0Ang.field.phi77);
		}
}

GNU_DISABLE_STACK_CHECK_START
VOID txbf_show_pfmu_data(
	IN PRTMP_ADAPTER pAd,
	IN USHORT subCarrIdx,
	IN UINT8 *pBuf)
{
	union txbf_bfer_pfmu_data pfmu_data, *pfmu_data_start;
	struct wifi_dev *wdev = NULL;
	struct mcs_nss_caps *mcs_nss = NULL;
	UINT8 he_bw, tx_path;
#ifdef DOT11_EHT_BE
	UINT8 eht_bw;
#endif

	wdev = txbf_get_last_wdev();
	he_bw = wlan_config_get_he_bw(wdev);
#ifdef DOT11_EHT_BE
	eht_bw = wlan_config_get_eht_bw(wdev);
#endif
	mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
	tx_path = mcs_nss->max_path[MAX_PATH_TX];

	pfmu_data_start = (union txbf_bfer_pfmu_data *) pBuf;
#ifdef CFG_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)pfmu_data_start, sizeof(pfmu_data));
#endif
	NdisCopyMemory(&pfmu_data, pfmu_data_start, sizeof(pfmu_data));

	if (tx_path > 4) {
		txbf_show_pfmu_data_nss4above_seg0(pAd, &pfmu_data, subCarrIdx, tx_path);

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_PRINT(
				"=============================== Seg1 Angles =================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x, Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x, Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rSeg1Ang.field.psi41, pfmu_data.field.rSeg1Ang.field.phi31,
				pfmu_data.field.rSeg1Ang.field.psi31, pfmu_data.field.rSeg1Ang.field.phi21,
				pfmu_data.field.rSeg1Ang.field.psi21, pfmu_data.field.rSeg1Ang.field.phi11,
				pfmu_data.field.rSeg1Ang.field.psi43, pfmu_data.field.rSeg1Ang.field.phi33,
				pfmu_data.field.rSeg1Ang.field.psi42, pfmu_data.field.rSeg1Ang.field.phi32,
				pfmu_data.field.rSeg1Ang.field.psi32, pfmu_data.field.rSeg1Ang.field.phi22);

			if (tx_path > 4) {
				MTWF_PRINT(
					"Psi51 = 0x%02x, Phi41 = 0x%03x, Psi52 = 0x%02x, Phi42 = 0x%03x, Psi53 = 0x%02x, Phi43 = 0x%03x,\n"
					"Psi54 = 0x%02x, Phi44 = 0x%03x\n",
					pfmu_data.field.rSeg1Ang.field.psi51, pfmu_data.field.rSeg1Ang.field.phi41,
					pfmu_data.field.rSeg1Ang.field.psi52, pfmu_data.field.rSeg1Ang.field.phi42,
					pfmu_data.field.rSeg1Ang.field.psi53, pfmu_data.field.rSeg1Ang.field.phi43,
					pfmu_data.field.rSeg1Ang.field.psi54, pfmu_data.field.rSeg1Ang.field.phi43);
			}
			if (tx_path > 5) {
				MTWF_PRINT(
					"Psi61 = 0x%02x, Phi51 = 0x%03x, Psi62 = 0x%02x, Phi52 = 0x%03x, Psi63 = 0x%02x, Phi53 = 0x%03x,\n"
					"Psi64 = 0x%02x, Phi54 = 0x%03x, Psi65 = 0x%02x, Phi55 = 0x%03x\n",
					pfmu_data.field.rSeg1Ang.field.psi61, pfmu_data.field.rSeg1Ang.field.phi51,
					pfmu_data.field.rSeg1Ang.field.psi62, pfmu_data.field.rSeg1Ang.field.phi52,
					pfmu_data.field.rSeg1Ang.field.psi63, pfmu_data.field.rSeg1Ang.field.phi53,
					pfmu_data.field.rSeg1Ang.field.psi64, pfmu_data.field.rSeg1Ang.field.phi53,
					pfmu_data.field.rSeg1Ang.field.psi65, pfmu_data.field.rSeg1Ang.field.phi55);
			}
			if (tx_path > 6) {
				MTWF_PRINT(
					"Psi71 = 0x%02x, Phi61 = 0x%03x, Psi72 = 0x%02x, Phi62 = 0x%03x, Psi73 = 0x%02x, Phi63 = 0x%03x,\n"
					"Psi74 = 0x%02x, Phi64 = 0x%03x, Psi75 = 0x%02x, Phi65 = 0x%03x, Psi76 = 0x%02x, Phi66 = 0x%03x\n",
					pfmu_data.field.rSeg1Ang.field.psi71, pfmu_data.field.rSeg1Ang.field.phi61,
					pfmu_data.field.rSeg1Ang.field.psi72, pfmu_data.field.rSeg1Ang.field.phi62,
					pfmu_data.field.rSeg1Ang.field.psi73, pfmu_data.field.rSeg1Ang.field.phi63,
					pfmu_data.field.rSeg1Ang.field.psi74, pfmu_data.field.rSeg1Ang.field.phi63,
					pfmu_data.field.rSeg1Ang.field.psi75, pfmu_data.field.rSeg1Ang.field.phi65,
					pfmu_data.field.rSeg1Ang.field.psi76, pfmu_data.field.rSeg1Ang.field.phi66);
			}
			if (tx_path > 7) {
				MTWF_PRINT(
					"Psi81 = 0x%02x, Phi71 = 0x%03x, Psi82 = 0x%02x, Phi72 = 0x%03x, Psi83 = 0x%02x, Phi73 = 0x%03x,\n"
					"Psi84 = 0x%02x, Phi74 = 0x%03x, Psi85 = 0x%02x, Phi75 = 0x%03x, Psi86 = 0x%02x, Phi76 = 0x%03x,\n"
					"Psi87 = 0x%02x, Phi77 = 0x%03x\n",
					pfmu_data.field.rSeg1Ang.field.psi81, pfmu_data.field.rSeg1Ang.field.phi71,
					pfmu_data.field.rSeg1Ang.field.psi82, pfmu_data.field.rSeg1Ang.field.phi72,
					pfmu_data.field.rSeg1Ang.field.psi83, pfmu_data.field.rSeg1Ang.field.phi73,
					pfmu_data.field.rSeg1Ang.field.psi84, pfmu_data.field.rSeg1Ang.field.phi73,
					pfmu_data.field.rSeg1Ang.field.psi85, pfmu_data.field.rSeg1Ang.field.phi75,
					pfmu_data.field.rSeg1Ang.field.psi86, pfmu_data.field.rSeg1Ang.field.phi76,
					pfmu_data.field.rSeg1Ang.field.psi87, pfmu_data.field.rSeg1Ang.field.phi77);
			}
		}

#ifdef DOT11_EHT_BE
		if (eht_bw == EHT_BW_320) {
			MTWF_PRINT(
				"=============================== Seg2 Angles =================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x, Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x, Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rSeg2Ang.field.psi41, pfmu_data.field.rSeg2Ang.field.phi31,
				pfmu_data.field.rSeg2Ang.field.psi31, pfmu_data.field.rSeg2Ang.field.phi21,
				pfmu_data.field.rSeg2Ang.field.psi21, pfmu_data.field.rSeg2Ang.field.phi11,
				pfmu_data.field.rSeg2Ang.field.psi43, pfmu_data.field.rSeg2Ang.field.phi33,
				pfmu_data.field.rSeg2Ang.field.psi42, pfmu_data.field.rSeg2Ang.field.phi32,
				pfmu_data.field.rSeg2Ang.field.psi32, pfmu_data.field.rSeg2Ang.field.phi22);

			if (tx_path > 4) {
				MTWF_PRINT(
					"Psi51 = 0x%02x, Phi41 = 0x%03x, Psi52 = 0x%02x, Phi42 = 0x%03x, Psi53 = 0x%02x, Phi43 = 0x%03x,\n"
					"Psi54 = 0x%02x, Phi44 = 0x%03x\n",
					pfmu_data.field.rSeg2Ang.field.psi51, pfmu_data.field.rSeg2Ang.field.phi41,
					pfmu_data.field.rSeg2Ang.field.psi52, pfmu_data.field.rSeg2Ang.field.phi42,
					pfmu_data.field.rSeg2Ang.field.psi53, pfmu_data.field.rSeg2Ang.field.phi43,
					pfmu_data.field.rSeg2Ang.field.psi54, pfmu_data.field.rSeg2Ang.field.phi43);
			}
			if (tx_path > 5) {
				MTWF_PRINT(
					"Psi61 = 0x%02x, Phi51 = 0x%03x, Psi62 = 0x%02x, Phi52 = 0x%03x, Psi63 = 0x%02x, Phi53 = 0x%03x,\n"
					"Psi64 = 0x%02x, Phi54 = 0x%03x, Psi65 = 0x%02x, Phi55 = 0x%03x\n",
					pfmu_data.field.rSeg2Ang.field.psi61, pfmu_data.field.rSeg2Ang.field.phi51,
					pfmu_data.field.rSeg2Ang.field.psi62, pfmu_data.field.rSeg2Ang.field.phi52,
					pfmu_data.field.rSeg2Ang.field.psi63, pfmu_data.field.rSeg2Ang.field.phi53,
					pfmu_data.field.rSeg2Ang.field.psi64, pfmu_data.field.rSeg2Ang.field.phi53,
					pfmu_data.field.rSeg2Ang.field.psi65, pfmu_data.field.rSeg2Ang.field.phi55);
			}
			if (tx_path > 6) {
				MTWF_PRINT(
					"Psi71 = 0x%02x, Phi61 = 0x%03x, Psi72 = 0x%02x, Phi62 = 0x%03x, Psi73 = 0x%02x, Phi63 = 0x%03x,\n"
					"Psi74 = 0x%02x, Phi64 = 0x%03x, Psi75 = 0x%02x, Phi65 = 0x%03x, Psi76 = 0x%02x, Phi66 = 0x%03x\n",
					pfmu_data.field.rSeg2Ang.field.psi71, pfmu_data.field.rSeg2Ang.field.phi61,
					pfmu_data.field.rSeg2Ang.field.psi72, pfmu_data.field.rSeg2Ang.field.phi62,
					pfmu_data.field.rSeg2Ang.field.psi73, pfmu_data.field.rSeg2Ang.field.phi63,
					pfmu_data.field.rSeg2Ang.field.psi74, pfmu_data.field.rSeg2Ang.field.phi63,
					pfmu_data.field.rSeg2Ang.field.psi75, pfmu_data.field.rSeg2Ang.field.phi65,
					pfmu_data.field.rSeg2Ang.field.psi76, pfmu_data.field.rSeg2Ang.field.phi66);
			}
			if (tx_path > 7) {
				MTWF_PRINT(
					"Psi81 = 0x%02x, Phi71 = 0x%03x, Psi82 = 0x%02x, Phi72 = 0x%03x, Psi83 = 0x%02x, Phi73 = 0x%03x,\n"
					"Psi84 = 0x%02x, Phi74 = 0x%03x, Psi85 = 0x%02x, Phi75 = 0x%03x, Psi86 = 0x%02x, Phi76 = 0x%03x,\n"
					"Psi87 = 0x%02x, Phi77 = 0x%03x\n",
					pfmu_data.field.rSeg2Ang.field.psi81, pfmu_data.field.rSeg2Ang.field.phi71,
					pfmu_data.field.rSeg2Ang.field.psi82, pfmu_data.field.rSeg2Ang.field.phi72,
					pfmu_data.field.rSeg2Ang.field.psi83, pfmu_data.field.rSeg2Ang.field.phi73,
					pfmu_data.field.rSeg2Ang.field.psi84, pfmu_data.field.rSeg2Ang.field.phi73,
					pfmu_data.field.rSeg2Ang.field.psi85, pfmu_data.field.rSeg2Ang.field.phi75,
					pfmu_data.field.rSeg2Ang.field.psi86, pfmu_data.field.rSeg2Ang.field.phi76,
					pfmu_data.field.rSeg2Ang.field.psi87, pfmu_data.field.rSeg2Ang.field.phi77);
			}

			MTWF_PRINT(
				"=============================== Seg3 Angles =================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x, Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x, Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rSeg3Ang.field.psi41, pfmu_data.field.rSeg3Ang.field.phi31,
				pfmu_data.field.rSeg3Ang.field.psi31, pfmu_data.field.rSeg3Ang.field.phi21,
				pfmu_data.field.rSeg3Ang.field.psi21, pfmu_data.field.rSeg3Ang.field.phi11,
				pfmu_data.field.rSeg3Ang.field.psi43, pfmu_data.field.rSeg3Ang.field.phi33,
				pfmu_data.field.rSeg3Ang.field.psi42, pfmu_data.field.rSeg3Ang.field.phi32,
				pfmu_data.field.rSeg3Ang.field.psi32, pfmu_data.field.rSeg3Ang.field.phi22);

			if (tx_path > 4) {
				MTWF_PRINT(
					"Psi51 = 0x%02x, Phi41 = 0x%03x, Psi52 = 0x%02x, Phi42 = 0x%03x, Psi53 = 0x%02x, Phi43 = 0x%03x,\n"
					"Psi54 = 0x%02x, Phi44 = 0x%03x\n",
					pfmu_data.field.rSeg3Ang.field.psi51, pfmu_data.field.rSeg3Ang.field.phi41,
					pfmu_data.field.rSeg3Ang.field.psi52, pfmu_data.field.rSeg3Ang.field.phi42,
					pfmu_data.field.rSeg3Ang.field.psi53, pfmu_data.field.rSeg3Ang.field.phi43,
					pfmu_data.field.rSeg3Ang.field.psi54, pfmu_data.field.rSeg3Ang.field.phi43);
			}
			if (tx_path > 5) {
				MTWF_PRINT(
					"Psi61 = 0x%02x, Phi51 = 0x%03x, Psi62 = 0x%02x, Phi52 = 0x%03x, Psi63 = 0x%02x, Phi53 = 0x%03x,\n"
					"Psi64 = 0x%02x, Phi54 = 0x%03x, Psi65 = 0x%02x, Phi55 = 0x%03x\n",
					pfmu_data.field.rSeg3Ang.field.psi61, pfmu_data.field.rSeg3Ang.field.phi51,
					pfmu_data.field.rSeg3Ang.field.psi62, pfmu_data.field.rSeg3Ang.field.phi52,
					pfmu_data.field.rSeg3Ang.field.psi63, pfmu_data.field.rSeg3Ang.field.phi53,
					pfmu_data.field.rSeg3Ang.field.psi64, pfmu_data.field.rSeg3Ang.field.phi53,
					pfmu_data.field.rSeg3Ang.field.psi65, pfmu_data.field.rSeg3Ang.field.phi55);
			}
			if (tx_path > 6) {
				MTWF_PRINT(
					"Psi71 = 0x%02x, Phi61 = 0x%03x, Psi72 = 0x%02x, Phi62 = 0x%03x, Psi73 = 0x%02x, Phi63 = 0x%03x,\n"
					"Psi74 = 0x%02x, Phi64 = 0x%03x, Psi75 = 0x%02x, Phi65 = 0x%03x, Psi76 = 0x%02x, Phi66 = 0x%03x\n",
					pfmu_data.field.rSeg3Ang.field.psi71, pfmu_data.field.rSeg3Ang.field.phi61,
					pfmu_data.field.rSeg3Ang.field.psi72, pfmu_data.field.rSeg3Ang.field.phi62,
					pfmu_data.field.rSeg3Ang.field.psi73, pfmu_data.field.rSeg3Ang.field.phi63,
					pfmu_data.field.rSeg3Ang.field.psi74, pfmu_data.field.rSeg3Ang.field.phi63,
					pfmu_data.field.rSeg3Ang.field.psi75, pfmu_data.field.rSeg3Ang.field.phi65,
					pfmu_data.field.rSeg3Ang.field.psi76, pfmu_data.field.rSeg3Ang.field.phi66);
			}
			if (tx_path > 7) {
				MTWF_PRINT(
					"Psi81 = 0x%02x, Phi71 = 0x%03x, Psi82 = 0x%02x, Phi72 = 0x%03x, Psi83 = 0x%02x, Phi73 = 0x%03x,\n"
					"Psi84 = 0x%02x, Phi74 = 0x%03x, Psi85 = 0x%02x, Phi75 = 0x%03x, Psi86 = 0x%02x, Phi76 = 0x%03x,\n"
					"Psi87 = 0x%02x, Phi77 = 0x%03x\n",
					pfmu_data.field.rSeg3Ang.field.psi81, pfmu_data.field.rSeg3Ang.field.phi71,
					pfmu_data.field.rSeg3Ang.field.psi82, pfmu_data.field.rSeg3Ang.field.phi72,
					pfmu_data.field.rSeg3Ang.field.psi83, pfmu_data.field.rSeg3Ang.field.phi73,
					pfmu_data.field.rSeg3Ang.field.psi84, pfmu_data.field.rSeg3Ang.field.phi73,
					pfmu_data.field.rSeg3Ang.field.psi85, pfmu_data.field.rSeg3Ang.field.phi75,
					pfmu_data.field.rSeg3Ang.field.psi86, pfmu_data.field.rSeg3Ang.field.phi76,
					pfmu_data.field.rSeg3Ang.field.psi87, pfmu_data.field.rSeg3Ang.field.phi77);
			}
		}
#endif

		MTWF_PRINT(
			"================================= Seg0 SNRs ===================================\n"
			"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n"
			"SNR04 = 0x%01x, SNR05 = 0x%01x, SNR06 = 0x%01x, SNR07 = 0x%01x\n",
			pfmu_data.field.rSeg0Snr.field.dsnr00, pfmu_data.field.rSeg0Snr.field.dsnr01,
			pfmu_data.field.rSeg0Snr.field.dsnr02, pfmu_data.field.rSeg0Snr.field.dsnr03,
			pfmu_data.field.rSeg0Snr.field.dsnr04, pfmu_data.field.rSeg0Snr.field.dsnr05,
			pfmu_data.field.rSeg0Snr.field.dsnr06, pfmu_data.field.rSeg0Snr.field.dsnr07);

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_PRINT(
				"================================ Seg1 SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n"
				"SNR04 = 0x%01x, SNR05 = 0x%01x, SNR06 = 0x%01x, SNR07 = 0x%01x\n",
				pfmu_data.field.rSeg1Snr.field.dsnr00, pfmu_data.field.rSeg1Snr.field.dsnr01,
				pfmu_data.field.rSeg1Snr.field.dsnr02, pfmu_data.field.rSeg1Snr.field.dsnr03,
				pfmu_data.field.rSeg1Snr.field.dsnr04, pfmu_data.field.rSeg1Snr.field.dsnr05,
				pfmu_data.field.rSeg1Snr.field.dsnr06, pfmu_data.field.rSeg1Snr.field.dsnr07);
		}

#ifdef DOT11_EHT_BE
		if (eht_bw == EHT_BW_320) {
			MTWF_PRINT(
				"================================ Seg2 SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n"
				"SNR04 = 0x%01x, SNR05 = 0x%01x, SNR06 = 0x%01x, SNR07 = 0x%01x\n",
				pfmu_data.field.rSeg2Snr.field.dsnr00, pfmu_data.field.rSeg2Snr.field.dsnr01,
				pfmu_data.field.rSeg2Snr.field.dsnr02, pfmu_data.field.rSeg2Snr.field.dsnr03,
				pfmu_data.field.rSeg2Snr.field.dsnr04, pfmu_data.field.rSeg2Snr.field.dsnr05,
				pfmu_data.field.rSeg2Snr.field.dsnr06, pfmu_data.field.rSeg2Snr.field.dsnr07);

			MTWF_PRINT(
				"================================ Seg3 SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n"
				"SNR04 = 0x%01x, SNR05 = 0x%01x, SNR06 = 0x%01x, SNR07 = 0x%01x\n",
				pfmu_data.field.rSeg3Snr.field.dsnr00, pfmu_data.field.rSeg3Snr.field.dsnr01,
				pfmu_data.field.rSeg3Snr.field.dsnr02, pfmu_data.field.rSeg3Snr.field.dsnr03,
				pfmu_data.field.rSeg3Snr.field.dsnr04, pfmu_data.field.rSeg3Snr.field.dsnr05,
				pfmu_data.field.rSeg3Snr.field.dsnr06, pfmu_data.field.rSeg3Snr.field.dsnr07);
		}
#endif

		MTWF_PRINT(
			"==================================================================================\n");
	} else {
		MTWF_PRINT(
			"================= TxBf profile Data - Subcarrier Idx = %d(0x%03x) ===================\n"
			"================================= Seg0 Angles ====================================\n"
			"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
			"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
			subCarrIdx, subCarrIdx,
			pfmu_data.field.rSeg0Ang.field.psi41, pfmu_data.field.rSeg0Ang.field.phi31,
			pfmu_data.field.rSeg0Ang.field.psi31, pfmu_data.field.rSeg0Ang.field.phi21,
			pfmu_data.field.rSeg0Ang.field.psi21, pfmu_data.field.rSeg0Ang.field.phi11,
			pfmu_data.field.rSeg0Ang.field.psi43, pfmu_data.field.rSeg0Ang.field.phi33,
			pfmu_data.field.rSeg0Ang.field.psi42, pfmu_data.field.rSeg0Ang.field.phi32,
			pfmu_data.field.rSeg0Ang.field.psi32, pfmu_data.field.rSeg0Ang.field.phi22);

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_PRINT(
				"================================== Seg1 Angles ====================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rSeg1Ang.field.psi41, pfmu_data.field.rSeg1Ang.field.phi31,
				pfmu_data.field.rSeg1Ang.field.psi31, pfmu_data.field.rSeg1Ang.field.phi21,
				pfmu_data.field.rSeg1Ang.field.psi21, pfmu_data.field.rSeg1Ang.field.phi11,
				pfmu_data.field.rSeg1Ang.field.psi43, pfmu_data.field.rSeg1Ang.field.phi33,
				pfmu_data.field.rSeg1Ang.field.psi42, pfmu_data.field.rSeg1Ang.field.phi32,
				pfmu_data.field.rSeg1Ang.field.psi32, pfmu_data.field.rSeg1Ang.field.phi22);
		}

#ifdef DOT11_EHT_BE
		if (eht_bw == EHT_BW_320) {
			MTWF_PRINT(
				"================================== Seg2 Angles ====================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rSeg2Ang.field.psi41, pfmu_data.field.rSeg2Ang.field.phi31,
				pfmu_data.field.rSeg2Ang.field.psi31, pfmu_data.field.rSeg2Ang.field.phi21,
				pfmu_data.field.rSeg2Ang.field.psi21, pfmu_data.field.rSeg2Ang.field.phi11,
				pfmu_data.field.rSeg2Ang.field.psi43, pfmu_data.field.rSeg2Ang.field.phi33,
				pfmu_data.field.rSeg2Ang.field.psi42, pfmu_data.field.rSeg2Ang.field.phi32,
				pfmu_data.field.rSeg2Ang.field.psi32, pfmu_data.field.rSeg2Ang.field.phi22);
			MTWF_PRINT(
				"================================== Seg3 Angles ====================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rSeg3Ang.field.psi41, pfmu_data.field.rSeg3Ang.field.phi31,
				pfmu_data.field.rSeg3Ang.field.psi31, pfmu_data.field.rSeg3Ang.field.phi21,
				pfmu_data.field.rSeg3Ang.field.psi21, pfmu_data.field.rSeg3Ang.field.phi11,
				pfmu_data.field.rSeg3Ang.field.psi43, pfmu_data.field.rSeg3Ang.field.phi33,
				pfmu_data.field.rSeg3Ang.field.psi42, pfmu_data.field.rSeg3Ang.field.phi32,
				pfmu_data.field.rSeg3Ang.field.psi32, pfmu_data.field.rSeg3Ang.field.phi22);
		}
#endif
		MTWF_PRINT(
			"================================= Seg0 SNRs ===================================\n"
			"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n",
			pfmu_data.field.rSeg0Snr.field.dsnr00, pfmu_data.field.rSeg0Snr.field.dsnr01,
			pfmu_data.field.rSeg0Snr.field.dsnr02, pfmu_data.field.rSeg0Snr.field.dsnr03);

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_PRINT(
				"================================ Seg1 SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n",
				pfmu_data.field.rSeg1Snr.field.dsnr00, pfmu_data.field.rSeg1Snr.field.dsnr01,
				pfmu_data.field.rSeg1Snr.field.dsnr02, pfmu_data.field.rSeg1Snr.field.dsnr03);
		}

#ifdef DOT11_EHT_BE
		if (eht_bw == EHT_BW_320) {
			MTWF_PRINT(
				"================================ Seg2 SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n",
				pfmu_data.field.rSeg2Snr.field.dsnr00, pfmu_data.field.rSeg2Snr.field.dsnr01,
				pfmu_data.field.rSeg2Snr.field.dsnr02, pfmu_data.field.rSeg2Snr.field.dsnr03);
			MTWF_PRINT(
				"================================ Seg3 SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n",
				pfmu_data.field.rSeg3Snr.field.dsnr00, pfmu_data.field.rSeg3Snr.field.dsnr01,
				pfmu_data.field.rSeg3Snr.field.dsnr02, pfmu_data.field.rSeg3Snr.field.dsnr03);
		}
#endif
		MTWF_PRINT(
			"==================================================================================\n");
	}
}
GNU_DISABLE_STACK_CHECK_END

static INT chip_set_txbf_angle(struct hdev_ctrl *ctrl, UINT32 bfer, UINT32 nc, UINT32 *angle)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	union txbf_bfer_pfmu_data *pfmu_data = (union txbf_bfer_pfmu_data *)pAd->pfmu_data_raw;
	union txbf_seg0_angle *LowSegAng = &pfmu_data->field.rSeg0Ang;

	switch (nc) {
	case 0: /* n x 1 */
		LowSegAng->field.phi11 = angle[0];
		LowSegAng->field.psi21 = angle[1];
		LowSegAng->field.phi21 = angle[2];
		LowSegAng->field.psi31 = angle[3];
		LowSegAng->field.phi31 = angle[4];
		LowSegAng->field.psi41 = angle[5];
		LowSegAng->field.phi41 = angle[6];
		LowSegAng->field.psi51 = angle[7];
		LowSegAng->field.phi51 = angle[8];
		LowSegAng->field.psi61 = angle[9];
		LowSegAng->field.phi61 = angle[10];
		LowSegAng->field.psi71 = angle[11];
		LowSegAng->field.phi71 = angle[12];
		LowSegAng->field.psi81 = angle[13];
		break;
	case 1: /* n x 2 */
		LowSegAng->field.phi22 = angle[0];
		LowSegAng->field.psi32 = angle[1];
		LowSegAng->field.phi32 = angle[2];
		LowSegAng->field.psi42 = angle[3];
		LowSegAng->field.phi42 = angle[4];
		LowSegAng->field.psi52 = angle[5];
		LowSegAng->field.phi52 = angle[6];
		LowSegAng->field.psi62 = angle[7];
		LowSegAng->field.phi62 = angle[8];
		LowSegAng->field.psi72 = angle[9];
		LowSegAng->field.phi72 = angle[10];
		LowSegAng->field.psi82 = angle[11];
		break;
	case 2: /* n x 3 */
		LowSegAng->field.phi33 = angle[0];
		LowSegAng->field.psi43 = angle[1];
		LowSegAng->field.phi43 = angle[2];
		LowSegAng->field.psi53 = angle[3];
		LowSegAng->field.phi53 = angle[4];
		LowSegAng->field.psi63 = angle[5];
		LowSegAng->field.phi63 = angle[6];
		LowSegAng->field.psi73 = angle[7];
		LowSegAng->field.phi73 = angle[8];
		LowSegAng->field.psi83 = angle[9];
		break;
	case 3: /* n x 4 */
		LowSegAng->field.phi44 = angle[0];
		LowSegAng->field.psi54 = angle[1];
		LowSegAng->field.phi54 = angle[2];
		LowSegAng->field.psi64 = angle[3];
		LowSegAng->field.phi64 = angle[4];
		LowSegAng->field.psi74 = angle[5];
		LowSegAng->field.phi74 = angle[6];
		LowSegAng->field.psi84 = angle[7];
		break;
	case 4: /* n x 5 */
		LowSegAng->field.phi55 = angle[0];
		LowSegAng->field.psi65 = angle[1];
		LowSegAng->field.phi65 = angle[2];
		LowSegAng->field.psi75 = angle[3];
		LowSegAng->field.phi75 = angle[4];
		LowSegAng->field.psi85 = angle[5];
		break;
	case 5: /* n x 6 */
		LowSegAng->field.phi66 = angle[0];
		LowSegAng->field.psi76 = angle[1];
		LowSegAng->field.phi76 = angle[2];
		LowSegAng->field.psi86 = angle[3];
		break;
	case 6: /* n x 7 */
	case 7: /* n x 8 */
		LowSegAng->field.phi77 = angle[0];
		LowSegAng->field.psi87 = angle[1];
		break;
	default:
		break;
	}

	return TRUE;
}

static INT chip_set_txbf_dsnr(struct hdev_ctrl *ctrl, UINT32 bfer, UINT32 *dsnr)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	union txbf_bfer_pfmu_data *pfmu_data = (union txbf_bfer_pfmu_data *)pAd->pfmu_data_raw;
	union txbf_bfer_seg0_dsnr *LowSegSnr = &pfmu_data->field.rSeg0Snr;

	LowSegSnr->field.dsnr00 = dsnr[0];
	LowSegSnr->field.dsnr01 = dsnr[1];
	LowSegSnr->field.dsnr02 = dsnr[2];
	LowSegSnr->field.dsnr03 = dsnr[3];
	LowSegSnr->field.dsnr04 = dsnr[4];
	LowSegSnr->field.dsnr05 = dsnr[5];
	LowSegSnr->field.dsnr06 = dsnr[6];
	LowSegSnr->field.dsnr07 = dsnr[7];

	return TRUE;
}

static INT chip_write_txbf_pfmu_data(
	struct hdev_ctrl *ctrl, UINT8 pf_idx, UINT16 subc_idx, UINT8 bfer)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	union txbf_bfer_pfmu_data *pfmu_data = (union txbf_bfer_pfmu_data *)pAd->pfmu_data_raw;
	UINT8 fgStatus = FALSE;

	if (CmdETxBfPfmuFullDimDataWrite(pAd, pf_idx, subc_idx, bfer,
		(PUCHAR)pfmu_data, sizeof(*pfmu_data)) == STATUS_TRUE)
		fgStatus = TRUE;

	os_zero_mem((pAd->pfmu_data_raw), sizeof(pAd->pfmu_data_raw));

	return fgStatus;
}

static INT chip_write_txbf_profile_data(struct _RTMP_ADAPTER *pAd, PUSHORT Input)
{
	UINT8 fgStatus = FALSE;
	UCHAR profileIdx;
	USHORT subcarrierIdx;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 angle[14];
	UINT32 bfer = 1;
	UINT32 dsnr[8];
	UINT8 nc = 0;

	profileIdx = Input[0];
	subcarrierIdx = Input[1];

	os_zero_mem((pAd->pfmu_data_raw), sizeof(pAd->pfmu_data_raw));
	os_zero_mem(angle, sizeof(angle));
	os_zero_mem(dsnr, sizeof(dsnr));

	angle[0] = Input[2];
	angle[1] = Input[3];
	angle[2] = Input[4];
	angle[3] = Input[5];
	angle[4] = Input[6];
	angle[5] = Input[7];

	if (ops->set_txbf_angle)
		fgStatus = ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"set_txbf_angle failed for nc:%d\n", nc);
		return fgStatus;
	}

	nc = 1;
	os_zero_mem(angle, sizeof(angle));
	angle[0] = Input[8];
	angle[1] = Input[9];
	angle[2] = Input[10];
	angle[3] = Input[11];

	if (ops->set_txbf_angle)
		fgStatus = ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"set_txbf_angle failed for nc:%d\n", nc);
		return fgStatus;
	}

	nc = 2;
	os_zero_mem(angle, sizeof(angle));
	angle[0] = Input[12];
	angle[1] = Input[13];

	if (ops->set_txbf_angle)
		fgStatus = ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"set_txbf_angle failed for nc:%d\n", nc);
		return fgStatus;
	}

	dsnr[0] = Input[14];
	dsnr[1] = Input[15];
	dsnr[2] = Input[16];
	dsnr[3] = Input[17];

	if (ops->set_txbf_dsnr)
		fgStatus = ops->set_txbf_dsnr(pAd->hdev_ctrl, bfer, dsnr);

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"set_txbf_dsnr failed\n");
		return fgStatus;
	}

	if (ops->write_txbf_pfmu_data)
		fgStatus = ops->write_txbf_pfmu_data(pAd->hdev_ctrl, profileIdx, subcarrierIdx, bfer);

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"write_txbf_pfmu_data failed for pfmu_id:%u and subcarrier:%u\n",
			profileIdx, subcarrierIdx);
		return fgStatus;
	}

	return fgStatus;
}

#ifdef VHT_TXBF_SUPPORT
static void setVHTETxBFCap(
	IN  RTMP_ADAPTER *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	VHT_CAP_INFO *pTxBFCap = pTxBfInfo->pVhtTxBFCap;
	UINT8 tx_path = 0;
	UINT8 max_bfee_ss = MT7992_MAX_BFEE_SS;

	if (max_bfee_ss > pAd->CommonCfg.MaxBfeeSS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
				"Set max bfee ss from profile: %u\n", pAd->CommonCfg.MaxBfeeSS);
		max_bfee_ss = pAd->CommonCfg.MaxBfeeSS;
	}

	if (pTxBfInfo->ucTxPathNum > 0)
		tx_path = pTxBfInfo->ucTxPathNum - 1;

	/*
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
		"cmmCfgETxBfEnCond = %d\n", (UCHAR)pTxBfInfo->cmmCfgETxBfEnCond);
	*/

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->bfee_cap_su	   = 1;
			pTxBFCap->bfer_cap_su	   = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
					"set wrong parameters\n");
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			if (pTxBFCap->bfee_sts_cap)
				pTxBFCap->bfee_sts_cap	= (pTxBFCap->bfee_sts_cap < max_bfee_ss) ? pTxBFCap->bfee_sts_cap : max_bfee_ss;
			else
				pTxBFCap->bfee_sts_cap	  = max_bfee_ss;
			pTxBFCap->num_snd_dimension = tx_path;
			break;

		case SUBF_BFER:
			pTxBFCap->bfee_cap_su	   = 0;
			pTxBFCap->bfer_cap_su	   = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
					"set wrong parameters\n");
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			pTxBFCap->bfee_sts_cap	  = 0;
			pTxBFCap->num_snd_dimension = tx_path;
			break;

		case SUBF_BFEE:
			pTxBFCap->bfee_cap_su	   = 1;
			pTxBFCap->bfer_cap_su	   = 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
					"set wrong parameters\n");
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			if (pTxBFCap->bfee_sts_cap)
				pTxBFCap->bfee_sts_cap	= (pTxBFCap->bfee_sts_cap < 3) ? pTxBFCap->bfee_sts_cap : max_bfee_ss;
			else
				pTxBFCap->bfee_sts_cap	  = max_bfee_ss;
			pTxBFCap->num_snd_dimension = tx_path;
			break;
		}
	} else {
		pTxBFCap->num_snd_dimension = 0;
		pTxBFCap->bfee_cap_mu	   = 0;
		pTxBFCap->bfee_cap_su	   = 0;
		pTxBFCap->bfer_cap_mu	   = 0;
		pTxBFCap->bfer_cap_su	   = 0;
		pTxBFCap->bfee_sts_cap	  = 0;
	}
}

#endif /* VHT_TXBF_SUPPORT */

#ifdef HE_TXBF_SUPPORT
void get_he_etxbf_cap(
	struct wifi_dev *wdev,
	TXBF_STATUS_INFO * txbf_status)
{
	struct he_bf_info *he_bf_struct = txbf_status->he_bf_info;
	UINT8 tx_path = 0;
	UINT8 max_bfee_ss = MT7992_MAX_BFEE_SS;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (max_bfee_ss > pAd->CommonCfg.MaxBfeeSS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
				"Set max bfee ss from profile: %u\n", pAd->CommonCfg.MaxBfeeSS);
		max_bfee_ss = pAd->CommonCfg.MaxBfeeSS;
	}

	if (txbf_status->ucTxPathNum > 0)
		tx_path = txbf_status->ucTxPathNum - 1;

	if (txbf_status->cmmCfgETxBfEnCond > 0) {
		switch (txbf_status->cmmCfgETxBfEnCond) {
		case SUBF_ALL:

			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_STA) {
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_SU_BFER : 0;
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_MU_BFER : 0;
			}

			he_bf_struct->bf_cap |= HE_SU_BFEE;

			if (he_bf_struct->bf_cap & HE_SU_BFEE) {
				/* Harrier E1/E2 don't support Ng16 due to Spec D4.0 change subcarrier index. */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_SU_FEEDBACK; */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_MU_FEEDBACK; */
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_SU_FEEDBACK;
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_MU_FEEDBACK;
				he_bf_struct->bfee_sts_gt_bw80 = max_bfee_ss;
				he_bf_struct->bfee_sts_le_eq_bw80 = max_bfee_ss;

				/*
				* Our max cap is 4x3, (Nr, Nc) = (3, 2).
				* We should boundle the value to our antenna quantity.
				* Antenna 4	3   2   1
				* Nc	  3	3   2   1
				*/
				he_bf_struct->bfee_max_nc = min(txbf_status->ucTxPathNum - 1, 2);
			}

			if (he_bf_struct->bf_cap & HE_SU_BFER) {
				/*
				* HE dintinguished Nr to '<=BW80' and '>BW80' in its protocol.
				* This fit our HW architecture.
				* Our current HW's Nsts of '<=BW80' is 4 and Nsts of '>BW80' is 2.
				*/
				he_bf_struct->snd_dim_gt_bw80 = tx_path;
				he_bf_struct->snd_dim_le_eq_bw80 = tx_path;

				he_bf_struct->bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
				he_bf_struct->bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
			}

			break;

		case SUBF_BFER:

			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_STA) {
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_SU_BFER : 0;
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_MU_BFER : 0;
			}

			if (he_bf_struct->bf_cap & HE_SU_BFER) {
				/*
				* HE dintinguished Nr to '<=BW80' and '>BW80' in its protocol.
				* This fit our HW architecture.
				* Our current HW's Nsts of '<=BW80' is 4 and Nsts of '>BW80' is 2.
				*/
				he_bf_struct->snd_dim_gt_bw80 = tx_path;
				he_bf_struct->snd_dim_le_eq_bw80 = tx_path;

				he_bf_struct->bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
				he_bf_struct->bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
			}

			break;

		case SUBF_BFEE:
			he_bf_struct->bf_cap |= HE_SU_BFEE;

			if (he_bf_struct->bf_cap & HE_SU_BFEE) {
				/* Harrier E1/E2 don't support Ng16 due to Spec D4.0 change subcarrier index. */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_SU_FEEDBACK; */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_MU_FEEDBACK; */
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_SU_FEEDBACK;
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_MU_FEEDBACK;
				he_bf_struct->bfee_sts_gt_bw80 = max_bfee_ss;
				he_bf_struct->bfee_sts_le_eq_bw80 = max_bfee_ss;
				he_bf_struct->bfee_max_nc = min(txbf_status->ucTxPathNum - 1, 2);
				he_bf_struct->bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
				he_bf_struct->bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
			}
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"set wrong parameters\n");
			break;
		}
	}
}
#endif /* HE_TXBF_SUPPORT */

#ifdef DOT11_EHT_BE
void get_eht_etxbf_cap(
	struct wifi_dev *wdev,
	TXBF_STATUS_INFO *txbf_status)
{
	struct eht_bf_info *eht_bf_struct = txbf_status->eht_bf_info;
	UINT8 tx_path = 0;
	UINT8 max_bfee_ss = MT7992_MAX_BFEE_SS;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (max_bfee_ss > pAd->CommonCfg.MaxBfeeSS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
				"Set max bfee ss from profile: %u\n", pAd->CommonCfg.MaxBfeeSS);
		max_bfee_ss = pAd->CommonCfg.MaxBfeeSS;
	}

	if (txbf_status->ucTxPathNum > 0)
		tx_path = txbf_status->ucTxPathNum - 1;

	if (txbf_status->cmmCfgETxBfEnCond > 0) {
		switch (txbf_status->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_STA) {
				eht_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? EHT_SU_BFER : 0;
				eht_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? EHT_MU_BFER : 0;
			}

			eht_bf_struct->bf_cap |= EHT_SU_BFEE;
			eht_bf_struct->bf_cap |= EHT_BFEE_NG16_SU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_BFEE_NG16_MU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_BFEE_CODEBOOK_4_2_SU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_BFEE_CODEBOOK_7_5_SU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_TRIGED_SU_BF_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_TRIGED_MU_BF_PATIAL_BW_FEEDBACK;
			eht_bf_struct->bfee_ss_le_eq_bw80 = max_bfee_ss;
			eht_bf_struct->bfee_ss_bw160 = max_bfee_ss;
			if (WMODE_CAP_BE_6G(wdev->PhyMode))
				eht_bf_struct->bfee_ss_bw320 = max_bfee_ss;

			eht_bf_struct->bfee_max_nc = min(txbf_status->ucTxPathNum - 1, 2);

			if (eht_bf_struct->bf_cap & EHT_SU_BFER) {
				eht_bf_struct->snd_dim_bw160 = tx_path;
				eht_bf_struct->snd_dim_le_eq_bw80 = tx_path;
				if (WMODE_CAP_BE_6G(wdev->PhyMode))
					eht_bf_struct->snd_dim_bw320 = tx_path;
			}

			break;

		case SUBF_BFER:
			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_STA) {
				eht_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? EHT_SU_BFER : 0;
				eht_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? EHT_MU_BFER : 0;
			}

			eht_bf_struct->bf_cap |= EHT_TRIGED_SU_BF_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_TRIGED_MU_BF_PATIAL_BW_FEEDBACK;

			if (eht_bf_struct->bf_cap & EHT_SU_BFER) {
				eht_bf_struct->snd_dim_bw160 = tx_path;
				eht_bf_struct->snd_dim_le_eq_bw80 = tx_path;
				if (WMODE_CAP_BE_6G(wdev->PhyMode))
					eht_bf_struct->snd_dim_bw320 = tx_path;
			}

			break;

		case SUBF_BFEE:
			eht_bf_struct->bf_cap |= EHT_SU_BFEE;
			eht_bf_struct->bf_cap |= EHT_BFEE_NG16_SU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_BFEE_NG16_MU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_BFEE_CODEBOOK_4_2_SU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_BFEE_CODEBOOK_7_5_SU_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_TRIGED_SU_BF_FEEDBACK;
			eht_bf_struct->bf_cap |= EHT_TRIGED_MU_BF_PATIAL_BW_FEEDBACK;

			eht_bf_struct->bfee_ss_le_eq_bw80 = max_bfee_ss;
			eht_bf_struct->bfee_ss_bw160 = max_bfee_ss;
			if (WMODE_CAP_BE_6G(wdev->PhyMode))
				eht_bf_struct->bfee_ss_bw320 = max_bfee_ss;
			eht_bf_struct->bfee_max_nc = min(txbf_status->ucTxPathNum - 1, 2);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"set wrong parameters\n");
			break;
		}

		if ((eht_bf_struct->bf_cap & EHT_SU_BFER) || (eht_bf_struct->bf_cap & EHT_MU_BFER)) {
			eht_bf_struct->max_eht_ltf_num = EXTRA_EHT_LTF;
			if (eht_bf_struct->bf_cap & EHT_SU_BFER) {
				if (eht_bf_struct->snd_dim_le_eq_bw80 >= 7)
					eht_bf_struct->max_eht_ltf_num |= MAX_EHT_LTF_NUM_8 << 1;
				else if (eht_bf_struct->snd_dim_le_eq_bw80 >= 3)
					eht_bf_struct->max_eht_ltf_num |= MAX_EHT_LTF_NUM_4 << 1;
					/*B1~B2 max eht ltf num for su*/
			}
			if (eht_bf_struct->bf_cap & EHT_MU_BFER) {
				if (eht_bf_struct->snd_dim_le_eq_bw80 >= 7)
					eht_bf_struct->max_eht_ltf_num |= MAX_EHT_LTF_NUM_8 << 3;
				else if (eht_bf_struct->snd_dim_le_eq_bw80 >= 3)
					eht_bf_struct->max_eht_ltf_num |= MAX_EHT_LTF_NUM_4 << 3;
					/*B3~B4 max eht ltf num for mu*/
			}
		}
	}
}
#endif /* DOT11_EHT_BE */
VOID mt7992_ibf_phase_cal_report(IN PRTMP_ADAPTER pAd,
											   IN UCHAR   ucGroupL_M_H,
											   IN UCHAR   ucGroup,
											   IN UCHAR   u1DbdcBandIdx,
											   IN UCHAR   ucStatus,
											   IN UCHAR   ucPhaseCalType,
											   IN PUCHAR  pBuf)
{
	UCHAR  ucGroupIdx;
	PUCHAR pucIBfPhaseG;
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;
	UINT_8 u1IbfPhaseOutStructLen   = 0;
	struct MT7992_IBF_PHASE_OUT  iBfPhaseOut;
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = NULL;

	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}

	if (ucGroup == 0)
		pAd = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	else
		pAd = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);

	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}

	serv_test = (struct service_test *)pAd->serv.serv_handle;
#endif

	u1IbfCalPhaseStructLenG0 = sizeof(struct MT7992_IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct MT7992_IBF_PHASE_Gx_T);
	u1IbfPhaseOutStructLen	 = sizeof(struct MT7992_IBF_PHASE_OUT);

	MTWF_PRINT(" Calibrated iBF phases\n");
	pucIBfPhaseG = pBuf + u1IbfPhaseOutStructLen;
	NdisZeroMemory(&iBfPhaseOut, sizeof(struct MT7992_IBF_PHASE_OUT));
	NdisCopyMemory(&iBfPhaseOut, pBuf, u1IbfPhaseOutStructLen);

	switch (ucPhaseCalType) {
	case IBF_PHASE_CAL_NOTHING: /* Do nothing */
		break;

	case IBF_PHASE_CAL_NORMAL_INSTRUMENT:
	case IBF_PHASE_CAL_NORMAL: /* Store calibrated phases with buffer mode */
		/* IF phase calibration is for BW20/40/80 */
		if (ucGroup == GROUP_0) {
			struct MT7992_IBF_PHASE_G0_T *piBfPhaseG0;
			struct MT7992_IBF_PHASE_Gx_T *piBfPhaseGx;

			piBfPhaseG0 = (struct MT7992_IBF_PHASE_G0_T *)pAd->piBfPhaseG0;
			piBfPhaseGx = (struct MT7992_IBF_PHASE_Gx_T *)pAd->piBfPhaseGx;
			NdisCopyMemory(piBfPhaseGx, pucIBfPhaseG, u1IbfCalPhaseStructLenGx);

			piBfPhaseG0->ucG0_M_T0_H = piBfPhaseGx->ucGx_M_T0_H;
			piBfPhaseG0->ucG0_M_T1_H = piBfPhaseGx->ucGx_M_T1_H;
			piBfPhaseG0->ucG0_M_T2_H = piBfPhaseGx->ucGx_M_T2_H;
			piBfPhaseG0->ucG0_R0_UH  = piBfPhaseGx->ucGx_R0_UH;
			piBfPhaseG0->ucG0_R0_H   = piBfPhaseGx->ucGx_R0_H;
			piBfPhaseG0->ucG0_R0_MH   = piBfPhaseGx->ucGx_R0_MH;
			piBfPhaseG0->ucG0_R0_M   = piBfPhaseGx->ucGx_R0_M;
			piBfPhaseG0->ucG0_R0_L   = piBfPhaseGx->ucGx_R0_L;
			piBfPhaseG0->ucG0_R1_UH  = piBfPhaseGx->ucGx_R1_UH;
			piBfPhaseG0->ucG0_R1_H   = piBfPhaseGx->ucGx_R1_H;
			piBfPhaseG0->ucG0_R1_MH   = piBfPhaseGx->ucGx_R1_MH;
			piBfPhaseG0->ucG0_R1_M   = piBfPhaseGx->ucGx_R1_M;
			piBfPhaseG0->ucG0_R1_L   = piBfPhaseGx->ucGx_R1_L;
			piBfPhaseG0->ucG0_R2_UH  = piBfPhaseGx->ucGx_R2_UH;
			piBfPhaseG0->ucG0_R2_H   = piBfPhaseGx->ucGx_R2_H;
			piBfPhaseG0->ucG0_R2_MH   = piBfPhaseGx->ucGx_R2_MH;
			piBfPhaseG0->ucG0_R2_M   = piBfPhaseGx->ucGx_R2_M;
			piBfPhaseG0->ucG0_R2_L   = piBfPhaseGx->ucGx_R2_L;
			piBfPhaseG0->ucG0_R3_UH  = piBfPhaseGx->ucGx_R3_UH;
			piBfPhaseG0->ucG0_R3_H   = piBfPhaseGx->ucGx_R3_H;
			piBfPhaseG0->ucG0_R3_MH   = piBfPhaseGx->ucGx_R3_MH;
			piBfPhaseG0->ucG0_R3_M   = piBfPhaseGx->ucGx_R3_M;
			piBfPhaseG0->ucG0_R3_L   = piBfPhaseGx->ucGx_R3_L;

			switch (ucGroupL_M_H) {
			case GROUP_L:
				break;

			case GROUP_M:
				MTWF_PRINT("G0 and Group_M\n G0_M_T0_H = %d\n G0_M_T1_H = %d\n",
						 piBfPhaseG0->ucG0_M_T0_H,
						 piBfPhaseG0->ucG0_M_T1_H);

				MTWF_PRINT("G0_M_T2_H = %d\n",
						 piBfPhaseG0->ucG0_M_T2_H);

				MTWF_PRINT("G0_R0_UH = %d\n G0_R0_H = %d\n G0_R0_MH = %d\n"
				" G0_R0_M = %d\n G0_R0_L = %d\n G0_R1_UH = %d\n G0_R1_H = %d\n"
				" G0_R1_MH = %d\n G0_R1_M = %d\n G0_R1_L = %d\n",
						 piBfPhaseG0->ucG0_R0_UH,
						 piBfPhaseG0->ucG0_R0_H,
						 piBfPhaseG0->ucG0_R0_MH,
						 piBfPhaseG0->ucG0_R0_M,
						 piBfPhaseG0->ucG0_R0_L,
						 piBfPhaseG0->ucG0_R1_UH,
						 piBfPhaseG0->ucG0_R1_H,
						 piBfPhaseG0->ucG0_R1_MH,
						 piBfPhaseG0->ucG0_R1_M,
						 piBfPhaseG0->ucG0_R1_L);

				MTWF_PRINT(
				"G0_R2_UH = %d\n G0_R2_H = %d\n G0_R2_MH = %d\n G0_R2_M = %d\n"
				" G0_R2_L = %d\n",
						 piBfPhaseG0->ucG0_R2_UH,
						 piBfPhaseG0->ucG0_R2_H,
						 piBfPhaseG0->ucG0_R2_MH,
						 piBfPhaseG0->ucG0_R2_M,
						 piBfPhaseG0->ucG0_R2_L);

				MTWF_PRINT("G0_R3_UH = %d\n G0_R3_H = %d\n G0_R3_MH = %d\n"
				" G0_R3_M = %d\n G0_R3_L = %d\n G0_R3_UL = %d\n",
						piBfPhaseG0->ucG0_R3_UH,
						piBfPhaseG0->ucG0_R3_H,
						piBfPhaseG0->ucG0_R3_MH,
						piBfPhaseG0->ucG0_R3_M,
						piBfPhaseG0->ucG0_R3_L,
						piBfPhaseG0->ucG0_R3_UL);

				MTWF_PRINT("Group : %d\n", ucGroup);
				MTWF_PRINT("Calibration == 1? or Verification == 2? : %d\n",
					ucPhaseCalType);
				MTWF_PRINT("Calibrated result = %d\n", ucStatus);
				MTWF_PRINT("0 : Means failed\n 1: means pass\n2: means ongoing\n");
				MTWF_PRINT("C0_UH:%d, C1_UH:%d, C2_UH:%d, C3_UH:%d\n"
					"C0_H:%d,  C1_H:%d,  C2_H:%d,  C3_H:%d\n"
					"C0_M:%d,  C1_M:%d,  C2_M:%d,  C3_M:%d\n"
				 "C0_L:%d,  C1_L:%d,  C2_L:%d,  C3_L:%d\n",
				iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH,
					iBfPhaseOut.ucC0_H,  iBfPhaseOut.ucC1_H,  iBfPhaseOut.ucC2_H,  iBfPhaseOut.ucC3_H,
					iBfPhaseOut.ucC0_M,  iBfPhaseOut.ucC1_M,  iBfPhaseOut.ucC2_M,  iBfPhaseOut.ucC3_M,
					iBfPhaseOut.ucC0_L,  iBfPhaseOut.ucC1_L,  iBfPhaseOut.ucC2_L,  iBfPhaseOut.ucC3_L);

				break;

			case GROUP_H:
				break;
			}
		} else {
			struct MT7992_IBF_PHASE_Gx_T *piBfPhaseGx;

			ucGroupIdx = ucGroup - 1;
			piBfPhaseGx = (struct MT7992_IBF_PHASE_Gx_T *)&pAd->piBfPhaseGx[ucGroupIdx * u1IbfCalPhaseStructLenGx];
			NdisCopyMemory(piBfPhaseGx, pucIBfPhaseG, u1IbfCalPhaseStructLenGx);

			switch (ucGroupL_M_H) {
			case GROUP_L:
				break;

			case GROUP_M:
				MTWF_PRINT("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n",
						 ucGroup,
						 ucGroup, piBfPhaseGx->ucGx_M_T0_H,
						 ucGroup, piBfPhaseGx->ucGx_M_T1_H);

				MTWF_PRINT("G%d_M_T2_H = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_M_T2_H);

#ifdef CONFIG_WLAN_SERVICE
				if (serv_test->test_config.tx_ant > TX_PATH_4) {
#else
				if (pAd->Antenna.field.TxPath > TX_PATH_4_BIN) {
#endif // CONFIG_WLAN_SERVICE

					MTWF_PRINT("G%d_M_T3_H = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_M_T3_H);
				}

				MTWF_PRINT("G%d_R0_UH = %d\n G%d_R0_H = %d\n G%d_R0_MH =%d\n",
						 ucGroup, piBfPhaseGx->ucGx_R0_UH,
						 ucGroup, piBfPhaseGx->ucGx_R0_H,
						 ucGroup, piBfPhaseGx->ucGx_R0_MH);

				MTWF_PRINT("G%d_R0_M = %d\n G%d_R0_L = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R0_M,
						 ucGroup, piBfPhaseGx->ucGx_R0_L);

				MTWF_PRINT("G%d_R1_UH = %d\n G%d_R1_H = %d\n G%d_R1_MH = %d\nn",
						 ucGroup, piBfPhaseGx->ucGx_R1_UH,
						 ucGroup, piBfPhaseGx->ucGx_R1_H,
						 ucGroup, piBfPhaseGx->ucGx_R1_MH);

				MTWF_PRINT("G%d_R1_M = %d\n G%d_R1_L = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R1_M,
						 ucGroup, piBfPhaseGx->ucGx_R1_L);

				MTWF_PRINT(
						"G%d_R2_UH = %d\n G%d_R2_H = %d\n G%d_R2_MH = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R2_UH,
						 ucGroup, piBfPhaseGx->ucGx_R2_H,
						 ucGroup, piBfPhaseGx->ucGx_R2_MH);

				MTWF_PRINT(
						"G%d_R2_M = %d\n G%d_R2_L = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R2_M,
						 ucGroup, piBfPhaseGx->ucGx_R2_L);

				MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n G%d_R3_MH = %d\n",
						ucGroup, piBfPhaseGx->ucGx_R3_UH,
						ucGroup, piBfPhaseGx->ucGx_R3_H,
						ucGroup, piBfPhaseGx->ucGx_R3_MH);

				MTWF_PRINT("G%d_R3_M = %d\n G%d_R3_L = %d\n",
						ucGroup, piBfPhaseGx->ucGx_R3_M,
						ucGroup, piBfPhaseGx->ucGx_R3_L);

#ifdef CONFIG_WLAN_SERVICE
				if (serv_test->test_config.tx_ant > TX_PATH_4) {
#else
				if (pAd->Antenna.field.TxPath > TX_PATH_4_BIN) {
#endif // CONFIG_WLAN_SERVICE
					MTWF_PRINT("G%d_R4_UH = %d\n G%d_R4_H = %d\n"
					"G%d_R4_M = %d\n"
					"G%d_R4_L = %d\n G%d_R4_UL = %d\n",
						ucGroup, piBfPhaseGx->ucGx_R4_UH,
						ucGroup, piBfPhaseGx->ucGx_R4_H,
						ucGroup, piBfPhaseGx->ucGx_R4_M,
						ucGroup, piBfPhaseGx->ucGx_R4_L,
						ucGroup, piBfPhaseGx->ucGx_R4_UL);
				}

				MTWF_PRINT("G%d_R3_UL = %d\n", ucGroup, piBfPhaseGx->ucGx_R3_UL);

				MTWF_PRINT("Group : %d\n", ucGroup);
				MTWF_PRINT("Calibration == 1? or Verification == 2? : %d\n",
					ucPhaseCalType);
				MTWF_PRINT("Calibrated result = %d\n", ucStatus);
				MTWF_PRINT("0 : Means failed\n1: means pass\n 2: means ongoing\n");

#ifdef CONFIG_WLAN_SERVICE
				if (serv_test->test_config.tx_ant <= TX_PATH_4) {
#else
				if (pAd->Antenna.field.TxPath > TX_PATH_4_BIN) {
#endif
					MTWF_PRINT("C0_UH : %d, C1_UH : %d, C2_UH : %d,\n"
					"C3_UH : %d\n"
					"C0_H : %d, C1_H : %d, C2_H : %d, C3_H : %d\n"
					"C0_MH : %d, C1_MH : %d, C2_MH : %d, C3_MH : %d\n"
					"C0_M : %d, C1_M : %d, C2_M : %d, C3_M : %d\n"
					"C0_L : %d, C1_L : %d, C2_L : %d,  C3_L : %d\n",
						iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH,
						iBfPhaseOut.ucC0_H,  iBfPhaseOut.ucC1_H,  iBfPhaseOut.ucC2_H,  iBfPhaseOut.ucC3_H,
						iBfPhaseOut.ucC0_MH, iBfPhaseOut.ucC1_MH, iBfPhaseOut.ucC2_MH, iBfPhaseOut.ucC3_MH,
						iBfPhaseOut.ucC0_M,  iBfPhaseOut.ucC1_M,  iBfPhaseOut.ucC2_M,  iBfPhaseOut.ucC3_M,
						iBfPhaseOut.ucC0_L,  iBfPhaseOut.ucC1_L,  iBfPhaseOut.ucC2_L,  iBfPhaseOut.ucC3_L);
				} else {
					MTWF_PRINT("C0_UH : %d, C1_UH : %d, C2_UH : %d,\n"
					"C3_UH : %d, C4_UH : %d\n"
					"C0_H : %d, C1_H : %d, C2_H : %d, C3_H : %d, C4_H : %d\n"
					"C0_MH : %d, C1_MH : %d, C2_MH : %d, C3_MH : %d, C4_MH : %d\n"
					"C0_M : %d, C1_M : %d, C2_M : %d, C3_M : %d, C4_M : %d\n"
					"C0_L : %d, C1_L : %d, C2_L : %d,  C3_L : %d, C4_L : %d\n",
						iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH, iBfPhaseOut.ucC4_UH,
						iBfPhaseOut.ucC0_H,  iBfPhaseOut.ucC1_H,  iBfPhaseOut.ucC2_H,  iBfPhaseOut.ucC3_H,  iBfPhaseOut.ucC4_H,
						iBfPhaseOut.ucC0_MH, iBfPhaseOut.ucC1_MH, iBfPhaseOut.ucC2_MH, iBfPhaseOut.ucC3_MH, iBfPhaseOut.ucC4_MH,
						iBfPhaseOut.ucC0_M,  iBfPhaseOut.ucC1_M,  iBfPhaseOut.ucC2_M,  iBfPhaseOut.ucC3_M,  iBfPhaseOut.ucC4_M,
						iBfPhaseOut.ucC0_L,  iBfPhaseOut.ucC1_L,  iBfPhaseOut.ucC2_L,  iBfPhaseOut.ucC3_L,  iBfPhaseOut.ucC4_L);
				}
				break;

			case GROUP_H:
				break;
			}
		}
	break;

	case IBF_PHASE_CAL_VERIFY: /* Show calibrated result only */
	case IBF_PHASE_CAL_VERIFY_INSTRUMENT:
		NdisCopyMemory(&iBfPhaseOut, pBuf, u1IbfPhaseOutStructLen);

		/* Update calibrated status */
		pAd->fgCalibrationFail |= ((ucStatus == 1) ? FALSE : TRUE);
		pAd->fgGroupIdPassFailStatus[ucGroup] = ((ucStatus == 1) ? FALSE : TRUE);
		MTWF_PRINT("Group : %d\n", ucGroup);
		MTWF_PRINT("Calibration == 1? or Verification == 2? : %d\n",
				ucPhaseCalType);
		MTWF_PRINT("Calibrated result = %d\n", ucStatus);
		MTWF_PRINT("0 : Means failed\n1: means pass\n 2: means ongoing\n");

#ifdef CONFIG_WLAN_SERVICE
		if (serv_test->test_config.tx_ant <= TX_PATH_4) {
#else
		if (pAd->Antenna.field.TxPath > TX_PATH_4_BIN) {
#endif
			MTWF_PRINT("C0_UH : %d, C1_UH : %d, C2_UH : %d, C3_UH : %d\n"
				"C0_H : %d, C1_H : %d, C2_H : %d, C3_H : %d\n"
				"C0_MH : %d, C1_MH : %d, C2_MH : %d, C3_MH : %d\n"
				"C0_M : %d, C1_M : %d, C2_M : %d, C3_M : %d\n"
				"C0_L : %d, C1_L : %d,C2_L : %d, C3_L : %d\n",
				iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH,
				iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, iBfPhaseOut.ucC3_H,
				iBfPhaseOut.ucC0_MH, iBfPhaseOut.ucC1_MH, iBfPhaseOut.ucC2_MH, iBfPhaseOut.ucC3_MH,
				iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M, iBfPhaseOut.ucC3_M,
				iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, iBfPhaseOut.ucC3_L);
		} else {
			MTWF_PRINT("C0_UH : %d, C1_UH : %d, C2_UH : %d, C3_UH : %d, C4_UH : %d\n"
				"C0_H : %d, C1_H : %d, C2_H : %d, C3_H : %d, C4_H : %d\n"
				"C0_MH : %d, C1_MH : %d, C2_MH : %d, C3_MH : %d, C4_MH : %d\n"
				"C0_M : %d, C1_M : %d, C2_M : %d, C3_M : %d, C4_M : %d\n"
				"C0_L : %d, C1_L : %d,C2_L : %d, C3_L : %d, C4_L : %d\n",
				iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH, iBfPhaseOut.ucC4_UH,
				iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, iBfPhaseOut.ucC3_H, iBfPhaseOut.ucC4_H,
				iBfPhaseOut.ucC0_MH, iBfPhaseOut.ucC1_MH, iBfPhaseOut.ucC2_MH, iBfPhaseOut.ucC3_MH, iBfPhaseOut.ucC4_MH,
				iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M, iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC4_M,
				iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, iBfPhaseOut.ucC3_L, iBfPhaseOut.ucC4_L);
		}
		break;

	default:
		break;
	}
}


VOID mt7992_ibf_phase_comp(IN PRTMP_ADAPTER pAd,
														IN UCHAR ucGroup,
														IN PCHAR pCmdBuf)
{
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;
	struct _RTMP_ADAPTER *ad = NULL;


	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}
#ifdef CONFIG_WLAN_SERVICE
	if (ucGroup == 0)
		ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	else
		ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);
	//ad = pAd;
	if (ad == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}
#else
	ad = pAd;
#endif

	u1IbfCalPhaseStructLenG0 = sizeof(struct MT7992_IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct MT7992_IBF_PHASE_Gx_T);

	if (ucGroup == 0)
		os_move_mem(pCmdBuf, ad->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	else
		os_move_mem(pCmdBuf, &ad->piBfPhaseGx[(ucGroup - 1) * u1IbfCalPhaseStructLenGx], u1IbfCalPhaseStructLenGx);
}


VOID mt7992_ibf_phase_cal_init(IN PRTMP_ADAPTER pAd)
{
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;
	struct _RTMP_ADAPTER *ad = NULL, *ad1 = NULL;


	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}
#ifdef CONFIG_WLAN_SERVICE

	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	ad1 = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);

	if ((ad == NULL) || (ad1 == NULL)) {
		MTWF_PRINT("%s ad NULL\n", __func__);
		return;
	}
#else
	ad = pAd;
	ad1 = pAd;
#endif

	u1IbfCalPhaseStructLenG0 = sizeof(struct MT7992_IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct MT7992_IBF_PHASE_Gx_T);

	/* Free memory allocated by iBF phase calibration */
	iBFPhaseFreeMem(pAd);

	os_alloc_mem(ad, (PUCHAR *)&ad->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	os_alloc_mem(ad, (PUCHAR *)&ad->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 8);
	os_alloc_mem(ad1, (PUCHAR *)&ad1->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	os_alloc_mem(ad1, (PUCHAR *)&ad1->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 8);

	if ((ad->piBfPhaseG0 == NULL) || (ad->piBfPhaseGx == NULL)
		|| (ad1->piBfPhaseG0 == NULL) || (ad1->piBfPhaseGx == NULL)) {
		MTWF_DBG(ad, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"Fail to allocate memory!\n");
		return;
	}

	NdisZeroMemory(ad->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	NdisZeroMemory(ad->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 12);
	NdisZeroMemory(ad1->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	NdisZeroMemory(ad1->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 12);

	ad->fgCalibrationFail = FALSE;
	ad1->fgCalibrationFail = FALSE;
	NdisZeroMemory(&ad->fgGroupIdPassFailStatus[0], 9);
	NdisZeroMemory(&ad->fgGroupIdPassFailStatus[0], 9);
	NdisZeroMemory(&ad1->fgGroupIdPassFailStatus[0], 9);
	NdisZeroMemory(&ad1->fgGroupIdPassFailStatus[0], 9);
}


#define MT7992_IBF_PHASE_EEPROM_START       0xC00
#define MT7992_IBF_LNA_PHASE_G0_ADDR        MT7992_IBF_PHASE_EEPROM_START
#define MT7992_IBF_LNA_PHASE_G1_ADDR        (MT7992_IBF_LNA_PHASE_G0_ADDR   + 29)
#define MT7992_IBF_LNA_PHASE_G2_ADDR        (MT7992_IBF_LNA_PHASE_G1_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G3_ADDR        (MT7992_IBF_LNA_PHASE_G2_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G4_ADDR        (MT7992_IBF_LNA_PHASE_G3_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G5_ADDR        (MT7992_IBF_LNA_PHASE_G4_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G6_ADDR        (MT7992_IBF_LNA_PHASE_G5_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G7_ADDR        (MT7992_IBF_LNA_PHASE_G6_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G8_ADDR        (MT7992_IBF_LNA_PHASE_G7_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G9_ADDR        (MT7992_IBF_LNA_PHASE_G8_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G10_ADDR       (MT7992_IBF_LNA_PHASE_G9_ADDR   + 34)
#define MT7992_IBF_LNA_PHASE_G11_ADDR       (MT7992_IBF_LNA_PHASE_G10_ADDR  + 34)
#define MT7992_IBF_LNA_PHASE_G12_ADDR       (MT7992_IBF_LNA_PHASE_G11_ADDR  + 34)

UINT_16 au2Mt7992IBfCalEEPROMOffset[13] = {
	MT7992_IBF_LNA_PHASE_G0_ADDR,
	MT7992_IBF_LNA_PHASE_G1_ADDR,
	MT7992_IBF_LNA_PHASE_G2_ADDR,
	MT7992_IBF_LNA_PHASE_G3_ADDR,
	MT7992_IBF_LNA_PHASE_G4_ADDR,
	MT7992_IBF_LNA_PHASE_G5_ADDR,
	MT7992_IBF_LNA_PHASE_G6_ADDR,
	MT7992_IBF_LNA_PHASE_G7_ADDR,
	MT7992_IBF_LNA_PHASE_G8_ADDR,
	MT7992_IBF_LNA_PHASE_G9_ADDR,
	MT7992_IBF_LNA_PHASE_G10_ADDR,
	MT7992_IBF_LNA_PHASE_G11_ADDR,
	MT7992_IBF_LNA_PHASE_G12_ADDR
};


VOID mt7992_ibf_phase_cal_e2p_update(IN PRTMP_ADAPTER pAd,
													  IN UCHAR   ucGroup,
													  IN UINT8   fgSX2,
													  IN UCHAR   ucUpdateAllTye)
{
	UCHAR  ucGroupIdx, ucEndLoop;
	UCHAR  ucCounter;
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;
	struct MT7992_IBF_PHASE_Gx_T iBfPhaseGx;
	struct MT7992_IBF_PHASE_G0_T iBfPhaseG0;
	struct _RTMP_ADAPTER *ad = NULL, *ad1 = NULL;
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = NULL;
#endif


	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}
#ifdef CONFIG_WLAN_SERVICE
	serv_test = (struct service_test *)pAd->serv.serv_handle;
	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	ad1 = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);

	if ((ad == NULL) || (ad1 == NULL)) {
		MTWF_PRINT("%s ad NULL\n", __func__);
		return;
	}
#else
	ad = pAd;
	ad1 = pAd;
#endif

	u1IbfCalPhaseStructLenG0 = sizeof(struct MT7992_IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct MT7992_IBF_PHASE_Gx_T);
	NdisZeroMemory(&iBfPhaseG0, u1IbfCalPhaseStructLenG0);
	NdisZeroMemory(&iBfPhaseGx, u1IbfCalPhaseStructLenGx);

	/* uint16_t u2Value; */
	/* IF phase calibration is for BW20/40/80/160 */
	ucGroupIdx = 0;

	switch (ucUpdateAllTye) {
	case IBF_PHASE_ONE_GROUP_UPDATE:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		if (ucGroup == GROUP_0) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(ad,
				au2Mt7992IBfCalEEPROMOffset[ucGroup],
				u1IbfCalPhaseStructLenG0,
				ad->piBfPhaseG0);
			NdisCopyMemory(&iBfPhaseG0,
				ad->piBfPhaseG0,
				u1IbfCalPhaseStructLenG0);
		} else {
			RT28xx_EEPROM_WRITE_WITH_RANGE(ad1,
				au2Mt7992IBfCalEEPROMOffset[ucGroup],
				u1IbfCalPhaseStructLenGx,
				&ad1->piBfPhaseGx[
					ucGroupIdx * u1IbfCalPhaseStructLenGx]);
			NdisCopyMemory(&iBfPhaseGx,
				&ad1->piBfPhaseGx[
					ucGroupIdx * u1IbfCalPhaseStructLenGx],
					u1IbfCalPhaseStructLenGx);
		}

		if (ucGroup == GROUP_0) {
			MTWF_PRINT("G%d and Group_M\n"
				"G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_M_T0_H,
				ucGroup, iBfPhaseG0.ucG0_M_T1_H);

			MTWF_PRINT("G%d_M_T2_H = %d\n",
				ucGroup, iBfPhaseG0.ucG0_M_T2_H);

			MTWF_PRINT("G%d_R0_UH = %d\n G%d_R0_H = %d\n"
				"G%d_R0_M = %d\n G%d_R0_L = %d\n"
				"G%d_R1_UH = %d\n G%d_R1_H = %d\n"
				"G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R0_UH,
				ucGroup, iBfPhaseG0.ucG0_R0_H,
				ucGroup, iBfPhaseG0.ucG0_R0_M,
				ucGroup, iBfPhaseG0.ucG0_R0_L,
				ucGroup, iBfPhaseG0.ucG0_R1_UH,
				ucGroup, iBfPhaseG0.ucG0_R1_H,
				ucGroup, iBfPhaseG0.ucG0_R1_M,
				ucGroup, iBfPhaseG0.ucG0_R1_L);

			MTWF_PRINT("G%d_R2_UH = %d\n G%d_R2_H = %d\n"
				"G%d_R2_M = %d\n G%d_R2_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R2_UH,
				ucGroup, iBfPhaseG0.ucG0_R2_H,
				ucGroup, iBfPhaseG0.ucG0_R2_M,
				ucGroup, iBfPhaseG0.ucG0_R2_L);

			MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n"
				"G%d_R3_M = %d\n"
				"G%d_R3_L = %d\n G%d_R3_UL = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R3_UH,
				ucGroup, iBfPhaseG0.ucG0_R3_H,
				ucGroup, iBfPhaseG0.ucG0_R3_M,
				ucGroup, iBfPhaseG0.ucG0_R3_L,
				ucGroup, iBfPhaseG0.ucG0_R3_UL);

		} else {
			MTWF_PRINT("G%d and Group_M\n"
				"G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseGx.ucGx_M_T0_H,
				ucGroup, iBfPhaseGx.ucGx_M_T1_H);

			MTWF_PRINT("G%d_M_T2_H = %d\n",
				ucGroup, iBfPhaseGx.ucGx_M_T2_H);

#ifdef CONFIG_WLAN_SERVICE
			if (serv_test->test_config.tx_ant > TX_PATH_4) {
#else
			if (pAd->Antenna.field.TxPath > TX_PATH_4_BIN) {
#endif // CONFIG_WLAN_SERVICE
				MTWF_PRINT("G%d_M_T3_H = %d\n",
				ucGroup, iBfPhaseGx.ucGx_M_T3_H);
			}

			MTWF_PRINT("G%d_R0_UH = %d\n G%d_R0_H = %d\n"
				"G%d_R0_M = %d\n G%d_R0_L = %d\n"
				"G%d_R1_UH = %d\n G%d_R1_H = %d\n"
				"G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseGx.ucGx_R0_UH,
				ucGroup, iBfPhaseGx.ucGx_R0_H,
				ucGroup, iBfPhaseGx.ucGx_R0_M,
				ucGroup, iBfPhaseGx.ucGx_R0_L,
				ucGroup, iBfPhaseGx.ucGx_R1_UH,
				ucGroup, iBfPhaseGx.ucGx_R1_H,
				ucGroup, iBfPhaseGx.ucGx_R1_M,
				ucGroup, iBfPhaseGx.ucGx_R1_L);
		}

		MTWF_PRINT("G%d_R2_UH = %d\n G%d_R2_H = %d\n"
			"G%d_R2_M = %d\n G%d_R2_L = %d\n",
			ucGroup, iBfPhaseGx.ucGx_R2_UH,
			ucGroup, iBfPhaseGx.ucGx_R2_H,
			ucGroup, iBfPhaseGx.ucGx_R2_M,
			ucGroup, iBfPhaseGx.ucGx_R2_L);

		MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n"
			"G%d_R3_M = %d\n G%d_R3_L = %d\n G%d_R3_UL = %d\n",
			ucGroup, iBfPhaseGx.ucGx_R3_UH,
			ucGroup, iBfPhaseGx.ucGx_R3_H,
			ucGroup, iBfPhaseGx.ucGx_R3_M,
			ucGroup, iBfPhaseGx.ucGx_R3_L,
			ucGroup, iBfPhaseGx.ucGx_R3_UL);

		MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n"
			"G%d_R3_M = %d\n G%d_R3_L = %d\n G%d_R3_UL = %d\n",
			ucGroup, iBfPhaseGx.ucGx_R3_UH,
			ucGroup, iBfPhaseGx.ucGx_R3_H,
			ucGroup, iBfPhaseGx.ucGx_R3_M,
			ucGroup, iBfPhaseGx.ucGx_R3_L,
			ucGroup, iBfPhaseGx.ucGx_R3_UL);

#ifdef CONFIG_WLAN_SERVICE
		if (serv_test->test_config.tx_ant > TX_PATH_4) {
#else
		if (pAd->Antenna.field.TxPath > TX_PATH_4_BIN) {
#endif // CONFIG_WLAN_SERVICE
			MTWF_PRINT("G%d_R4_UH = %d\n G%d_R4_H = %d\n"
			"G%d_R4_M = %d\n"
			"G%d_R4_L = %d\n G%d_R4_UL = %d\n",
			ucGroup, iBfPhaseGx.ucGx_R4_UH,
			ucGroup, iBfPhaseGx.ucGx_R4_H,
			ucGroup, iBfPhaseGx.ucGx_R4_M,
			ucGroup, iBfPhaseGx.ucGx_R4_L,
			ucGroup, iBfPhaseGx.ucGx_R4_UL);
		}
		break;

	case IBF_PHASE_ALL_GROUP_UPDATE:
		if (ad->fgCalibrationFail == FALSE) {
			if ((ad->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
				(ad->u1IbfCalPhase2G5GE2pClean == CLEAN_2G)) {
				MTWF_PRINT("2G group can pass criterion and ");
				MTWF_PRINT("calibrated phases can be written ");
				MTWF_PRINT("into EEPROM\n");

				/* Write Group 0 into EEPROM */
				RT28xx_EEPROM_WRITE_WITH_RANGE(ad,
					au2Mt7992IBfCalEEPROMOffset[GROUP_0],
					u1IbfCalPhaseStructLenG0,
					ad->piBfPhaseG0);
			}
		}

		if (ad1->fgCalibrationFail == FALSE) {
			if ((ad1->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
				(ad1->u1IbfCalPhase2G5GE2pClean == CLEAN_5G)) {
				MTWF_PRINT("All of 5G groups can pass criterion and ");
				MTWF_PRINT("calibrated phases can be written ");
				MTWF_PRINT("into EEPROM\n");

				/* Write Group 1 ~ 8 into EEPROM */
				RT28xx_EEPROM_WRITE_WITH_RANGE(ad1,
					au2Mt7992IBfCalEEPROMOffset[GROUP_1],
					(u1IbfCalPhaseStructLenGx * 12),
					ad1->piBfPhaseGx);
			}
		} else {
			MTWF_PRINT("Calibrated phases can't be written into ");
			MTWF_PRINT("EEPROM because some groups can't pass ");
			MTWF_PRINT("criterion!!!\n");

			for (ucCounter = GROUP_0; ucCounter <= GROUP_8;
								ucCounter++) {
				if (ucCounter == GROUP_0)
					MTWF_PRINT("Group%d = %s\n",
					ucCounter,
					(ad->fgGroupIdPassFailStatus[ucCounter]
							== TRUE) ? "FAIL" : "PASS");
				else
					MTWF_PRINT("Group%d = %s\n",
					ucCounter,
					(ad1->fgGroupIdPassFailStatus[ucCounter]
							== TRUE) ? "FAIL" : "PASS");
			}
		}

		break;

	case IBF_PHASE_ALL_GROUP_ERASE:
		NdisZeroMemory(ad->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
		NdisZeroMemory(ad1->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 12);

		if ((ad->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
			(ad->u1IbfCalPhase2G5GE2pClean == CLEAN_2G)) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(ad,
				au2Mt7992IBfCalEEPROMOffset[GROUP_0],
				u1IbfCalPhaseStructLenG0,
				ad->piBfPhaseG0);
		}

		if ((ad1->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
			(ad1->u1IbfCalPhase2G5GE2pClean == CLEAN_5G)) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(ad1,
				au2Mt7992IBfCalEEPROMOffset[GROUP_1],
				(u1IbfCalPhaseStructLenGx * 12),
				ad1->piBfPhaseGx);
		}
		break;

	case IBF_PHASE_ALL_GROUP_READ_FROM_E2P:
		iBFPhaseCalE2PInit(pAd);
		break;

	default:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		NdisCopyMemory(&iBfPhaseGx,
		&ad1->piBfPhaseGx[ucGroupIdx * u1IbfCalPhaseStructLenGx],
		u1IbfCalPhaseStructLenGx);
		NdisCopyMemory(&iBfPhaseG0,
		ad->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
		break;
	}
}


VOID mt7992_ibf_phase_free_mem(IN PRTMP_ADAPTER pAd)
{
	struct _RTMP_ADAPTER *ad = NULL, *ad1 = NULL;


	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}
#ifdef CONFIG_WLAN_SERVICE
	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	ad1 = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);

	if ((ad == NULL) || (ad1 == NULL)) {
		MTWF_PRINT("%s ad NULL\n", __func__);
		return;
	}
#else
	ad = pAd;
	ad1 = pAd;
#endif

    /* Group 0 */
	if (ad->piBfPhaseG0 != NULL) {
		os_free_mem(ad->piBfPhaseG0);
		os_free_mem(ad->piBfPhaseGx);
	}

	/* Group 1 ~ 8 */
	if (ad1->piBfPhaseGx != NULL) {
		os_free_mem(ad1->piBfPhaseG0);
		os_free_mem(ad1->piBfPhaseGx);
	}
}

#endif /* TXBF_SUPPORT */

/*
*
*/
#define DEFAULT_RID 0
inline UINT32 get_rid_value(struct pci_hif_chip *pci_hif)
{
	return DEFAULT_RID;
}

inline VOID set_rid_value(struct pci_hif_chip *pci_hif, UINT32 rid)
{
}

static VOID pci_io_remap_l1_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	UINT32 backup_val = 0, tmp_val = 0;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L1_REMAP_MASK);
	tmp_val |= GET_L1_REMAP_BASE(reg) << HIF_ADDR_L1_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &tmp_val);
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_BASE_ADDR + GET_L1_REMAP_OFFSET(reg), val);
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, backup_val);
}

static VOID pci_io_remap_l1_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	UINT32 backup_val = 0, tmp_val = 0;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L1_REMAP_MASK);
	tmp_val |= GET_L1_REMAP_BASE(reg) << HIF_ADDR_L1_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &tmp_val);
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_BASE_ADDR + GET_L1_REMAP_OFFSET(reg), val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_BASE_ADDR + GET_L1_REMAP_OFFSET(reg), &val);
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, backup_val);
}

static VOID pci_io_remap_l2_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	UINT32 backup_val = 0, tmp_val = 0;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L2_REMAP_MASK);
	tmp_val |= GET_L2_REMAP_BASE(reg) << HIF_ADDR_L2_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_BASE_ADDR + GET_L2_REMAP_OFFSET(reg), val);
}

static VOID pci_io_remap_l2_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	UINT32 backup_val = 0, tmp_val = 0;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L2_REMAP_MASK);
	tmp_val |= GET_L2_REMAP_BASE(reg) << HIF_ADDR_L2_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L2_REMAP_BASE_ADDR + GET_L2_REMAP_OFFSET(reg), val);
}

static uint8_t pci_io_remap_is_l1_remap(UINT32 *reg)
{
	/* physical addr shall use layer 1 remap */
	if (IS_PHY_ADDR(*reg))
		return true;

	/* mcu view addr are handled depends on domain */
	/* CBTOP: MCU view addr equals to physical addr, already handled */

	/* CONN_INFRA: covert to phyiscal addr and use layer 1 remap */
	if (IS_CONN_INFRA_MCU_ADDR(*reg)) {
		(*reg) -= CONN_INFRA_MCU_TO_PHY_ADDR_OFFSET;
		return true;
	}

	/* WFSYS: shall use layer 2 remap */
	return false;
}
static VOID pci_io_remap_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	if (pci_io_remap_is_l1_remap(&reg))
		pci_io_remap_l1_read32(hdev_ctrl, reg, val);
	else
		pci_io_remap_l2_read32(hdev_ctrl, reg, val);
}

static VOID pci_io_remap_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	if (pci_io_remap_is_l1_remap(&reg))
		pci_io_remap_l1_write32(hdev_ctrl, reg, val);
	else
		pci_io_remap_l2_write32(hdev_ctrl, reg, val);
}

static VOID mt7992_mcs_nss_cap_init(struct mcs_nss_caps *mcs_nss)
{
	UINT8 txrx = 0;

#ifdef G_BAND_256QAM
	mcs_nss->g_band_256_qam = TRUE;
#endif
	mcs_nss->max_nss = 4;
	for (txrx = 0; txrx < 2; txrx++)
		mcs_nss->max_path[txrx] = 4;
	mcs_nss->bw160_max_nss = 4;
	mcs_nss->max_vht_mcs = VHT_MCS_CAP_9;
#ifdef DOT11_HE_AX
	mcs_nss->max_24g_ru_num = 2;
	mcs_nss->max_5g_ru_num = 3;
	if (mcs_nss->bw160_max_nss)
		mcs_nss->max_5g_ru_num = RU_IDX_ALLOC_NUM;
#endif
}

#ifdef ERR_RECOVERY
#define WF_SW_DEF_HWITS_WAR_EN_MASK		WF_SW_DEF_CR_RSVD_DBG_0

static VOID mt7992_dump_ser_stat(RTMP_ADAPTER *pAd, uint8_t fgDumpAll)
{
	/* Add the EDCCA Time for Debug */
	Show_MibBucket_Proc(pAd, "");
}

#ifdef CONFIG_TX_DELAY
static VOID mt7992_tx_deley_parm_init(UCHAR tx_delay_mode, struct tx_delay_control *tx_delay_ctl)
{
	if (tx_delay_mode == TX_DELAY_SW_MODE) {
		tx_delay_ctl->min_tx_delay_en_tp = MIN_AGG_EN_TP;
		tx_delay_ctl->max_tx_delay_en_tp = MAX_AGG_EN_TP;
		tx_delay_ctl->que_agg_timeout_value = QUE_AGG_TIMEOUT;
		tx_delay_ctl->min_pkt_len = MIN_AGG_PKT_LEN;
		tx_delay_ctl->max_pkt_len = MAX_AGG_PKT_LEN;
		tx_delay_ctl->tx_process_batch_cnt = TX_BATCH_CNT;
	} else if (tx_delay_mode == TX_DELAY_HW_MODE) {
		tx_delay_ctl->que_agg_timeout_value = HW_QUE_AGG_TIMEOUT;
		tx_delay_ctl->tx_process_batch_cnt = HW_TX_BATCH_CNT;
	}
}
#endif

#endif
#define LMAC_TXD_MAX_SIZE (sizeof(struct txd_l))
#define CT_PARSE_PAYLOAD_LEN 72

#ifdef CFG_SUPPORT_FALCON_MURU
static UINT_8 muru_io_r_u8(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 addr_4b = (addr) & 0xFFFFFFFC;
	UINT32 pos = (addr) & 0x3;
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr_4b, &value);

	return (UINT_8)(value >> (pos << 3));
}

static UINT_16 muru_io_r_u16(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 addr_4b = (addr) & 0xFFFFFFFC;
	UINT32 pos = (addr) & 0x3;
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr_4b, &value);

	return (UINT_16)(value >> (pos << 3));
}

static UINT_32 muru_io_r_u32(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr, &value);

	return value;
}


static INT_16 muru_io_r_i16(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 addr_4b = (addr) & 0xFFFFFFFC;
	UINT32 pos = (addr) & 0x3;
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr_4b, &value);

	return (INT_16)(value >> (pos << 3));
}


INT SyncMuruSram(struct _RTMP_ADAPTER *pAd)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_DO_SRAM_SYNC;
	struct _CMD_ATTRIBUTE attr = {0};

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef CFG_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	return Ret;
}

VOID SyncMuruSramCheckAddr(struct _RTMP_ADAPTER *pAd, UINT32 Addr)
{
	if (Addr >= 0xE0000000) {
		SyncMuruSram(pAd);
		RtmpusecDelay(500);
	}
}

VOID set_muru_data(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_GLO_ADDR;
	struct _CMD_ATTRIBUTE attr = {0};

	RTMP_STRING *macptr = NULL;
	UINT_32 Addr = 0;
	UINT_32 Value = 0;
	UINT_8 numofparam = 0, i = 0;
	CMD_MURU_SET_GLOBAL_ADDR rSetParam;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam > 2) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-");
		macptr && i < 2; macptr = rstrtok(NULL, "-"), i++) {

		if (i == 0)
			Addr = (UINT_32)os_str_tol(macptr, 0, 16);

		if (i == 1)
			Value = (UINT_32)os_str_tol(macptr, 0, 16);
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"Address = 0x%08X Value = %d\n", Addr, Value);

	os_zero_mem(&rSetParam, sizeof(CMD_MURU_SET_GLOBAL_ADDR));
	rSetParam.u4Addr  = Addr;
	rSetParam.u4Value = Value;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(
		pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_GLOBAL_ADDR));

	if (!msg)
		goto error;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef CFG_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	rSetParam.u4Addr  = cpu2le32(rSetParam.u4Addr);
	rSetParam.u4Value = cpu2le32(rSetParam.u4Value);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&rSetParam,
			sizeof(CMD_MURU_SET_GLOBAL_ADDR));
	AndesSendCmdMsg(pAd, msg);
error:
	return;
}

static VOID show_muru_local_muru_para(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rMuruPara) */
	UINT_32 offset;
	UINT_32 addr;
	UINT_32 addr_u1TxCmdQLen, addr_ai1ManTargetRssi;
	uint8_t err = pAd->CommonCfg.rGloInfo.rLocalDataMuruPara.fgError;
	UINT_8  i = 0;

	if (!err) {
		/* rMuruPara */
		MTWF_PRINT("|-rMuruPara (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_PARA_T, fgPingPongAlgo);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgPingPongAlgo = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgSu);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSu = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fg256BitMap);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fg256BitMap = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgUlBsrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgUlBsrp = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgTxcmdsnd);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTxcmdsnd = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgTpc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTpc = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgTpcManualMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTpcManualMode = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u2fixedTPNum);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2fixedTPNum = %d\n",
			addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u1UlMpduCntPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1UlMpduCntPolicy = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u1DelayPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1DelayPolicy = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, rTpcManPara);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) rTpcManPara\n", addr);
		MTWF_PRINT("| |-|-(0x%08X) ai1ManTargetRssi\n", addr);

		addr_ai1ManTargetRssi = addr;
		for (i = 0; i < MAX_USER_IN_PPDU; i++) {
			addr = addr_ai1ManTargetRssi + sizeof(INT_8) * i;
			MTWF_PRINT("| |-|-|-(0x%08X) ai1ManTargetRssi[%d] = %d\n",
				addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(MURU_PARA_T, fgTpcOptMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTpcOptMode = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u1TxCmdQLen);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxCmdQLen\n", addr);

		addr_u1TxCmdQLen = addr;
		for (i = 0; i < MAX_DATA_AC_NUM; i++) {
			addr = addr_u1TxCmdQLen + sizeof(UINT_8) * i;
			MTWF_PRINT("| |-|-(0x%08X) u1TxCmdQLen[%d] = %d\n",
				addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(MURU_PARA_T, fgTBSuAdaptiveLSIGLen);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTBSuAdaptiveLSIGLen = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgSRState);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSRState = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u1TypeCDelayReq);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TypeCDelayReq = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u4BsrTruncateThr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4BsrTruncateThr = %d\n",
			addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u2MaxStaCntLimit);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MaxStaCntLimit = %d\n",
			addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgPreGrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgPreGrp = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgTxopBurst);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTxopBurst = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, i2PsdDiffThr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2PsdDiffThr = %d\n",
			addr, muru_io_r_i16(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u1SplPriority);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SplPriority = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, u1DlSolictAckPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1DlSolictAckPolicy = %d\n",
			addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_muru_local_qlen_info(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rQlenInfo) */
	UINT_32 offset;
	UINT_32 addr;
	UINT_32 addr_au4DLQlen, addr_au4ULQlen;
	uint8_t err = pAd->CommonCfg.rGloInfo.rLocalDataQlenInfo.fgError;
	UINT_8   i = 0;

	if (!err) {
		MTWF_PRINT("|-rQlenInfo (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_QLEN_INFO_T, au4DLQlen);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4DLQlen\n", addr);

		addr_au4DLQlen = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_au4DLQlen + sizeof(UINT_32) * i;
			MTWF_PRINT("| |-|-(0x%08X) au4DLQlen[%d] = %d\n",
				addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(MURU_QLEN_INFO_T, au4ULQlen);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4ULQlen\n", addr);

		addr_au4ULQlen = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_au4ULQlen + sizeof(UINT_32) * i;
			MTWF_PRINT("| |-|-(0x%08X) au4ULQlen[%d] = %d\n",
				addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(MURU_QLEN_INFO_T, u4TotDLQlenAllAc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4TotDLQlenAllAc = %d\n",
			addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_QLEN_INFO_T, u4TotULQlenAllAc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4TotULQlenAllAc = %d\n",
			addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_QLEN_INFO_T, u4BsrTruncateThr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4BsrTruncateThr = %d\n",
			addr, muru_io_r_u32(pAd, addr));
	}
}

static VOID show_muru_local_bsrp_ctrl(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rExt_Cmd_Bsrp_Ctrl)*/
	UINT_32 offset;
	UINT_32 addr;
	uint8_t err = pAd->CommonCfg.rGloInfo.rLocalDataBsrpCtrl.fgError;

	if (!err) {
		MTWF_PRINT("|-rExt_Cmd_Bsrp_Ctrl (0x%08X)\n", subbase);

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u1TriggerFlow);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TriggerFlow = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u2BsrpInterval);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2BsrpInterval = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u2BsrpRuAlloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2BsrpRuAlloc = %d\n",
			addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u4TriggerType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4TriggerType = %d\n",
			addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, fgExtCmdBsrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgExtCmdBsrp = %d\n",
			addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_muru_local_txcmd_ctrl(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base;
	UINT_32 offset;
	UINT_32 addr;
	uint8_t err = pAd->CommonCfg.rGloInfo.rLocalDataTxCmdCtrl.fgError;

	if (!err) {
		MTWF_PRINT("|-rMuru_TxCmd_Ctrl (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_TXCMD_CTRL_T, fgGlobalPreLoad);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgGlobalPreLoad = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TXCMD_CTRL_T, i2PuPreGrpMaxPsd_dBm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2PuPreGrpMaxPsd_dBm = %d\n",
			addr, muru_io_r_i16(pAd, addr));
	}
}

static VOID show_muru_local_data(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32	base = pAd->CommonCfg.rGloInfo.rLocalData.u4Addr;
	UINT_32 offset;
	UINT_32 addr;
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[3][25] = { {0}, {0} };
	UINT_8  numofparam = 0;
	uint8_t err = FALSE;
	uint8_t muruparam = FALSE, qleninfo = FALSE;
	uint8_t bsrpctrl = FALSE, txcmdctrl = FALSE;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 4) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-");
			macptr && i < 3; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= 25)
			NdisMoveMemory(InputStr[i], macptr, strlen(macptr));
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Length of macprt is too long!\n");
			return;
		}

		if (!(NdisCmpMemory(InputStr[i], "all", strlen("all"))))
			muruparam = qleninfo = bsrpctrl = txcmdctrl = TRUE;

		if (!(NdisCmpMemory(InputStr[i],
			"muruparam", strlen("muruparam"))))
			muruparam = TRUE;

		if (!(NdisCmpMemory(InputStr[i],
			"qleninfo", strlen("qleninfo"))))
			qleninfo = TRUE;

		if (!(NdisCmpMemory(InputStr[i],
			"bsrpctrl", strlen("bsrpctrl"))))
			bsrpctrl = TRUE;

		if (!(NdisCmpMemory(InputStr[i],
			"txcmdctrl", strlen("txcmdctrl"))))
			txcmdctrl = TRUE;
	}

	SyncMuruSramCheckAddr(pAd, base);

	MTWF_PRINT("_rMuru_Local_Data (0x%08X)\n", base);

	err = pAd->CommonCfg.rGloInfo.rLocalDataMuruPara.fgError;

	if (muruparam == TRUE)
		show_muru_local_muru_para(pAd,
			pAd->CommonCfg.rGloInfo.rLocalDataMuruPara.u4Addr);

	if (!err) {
		offset = OFFSET_OF(MURU_LOCAL_DATA_T, eDuSchWeight);
		addr = base + offset;
		MTWF_PRINT("|-(0x%08X) eDuSchWeight = %d\n",
			addr, muru_io_r_u32(pAd, addr));
	}

	if (qleninfo == TRUE)
		show_muru_local_qlen_info(pAd,
			pAd->CommonCfg.rGloInfo.rLocalDataQlenInfo.u4Addr);

	if (!err) {
		offset = OFFSET_OF(MURU_LOCAL_DATA_T, u2MuruSplHeadWlanId);
		addr = base + offset;
		MTWF_PRINT("|-(0x%08X) u2MuruSplHeadWlanId = %d\n",
			addr, muru_io_r_u16(pAd, addr));
	}

	if (bsrpctrl == TRUE)
		show_muru_local_bsrp_ctrl(pAd,
			pAd->CommonCfg.rGloInfo.rLocalDataBsrpCtrl.u4Addr);

	if (txcmdctrl == TRUE)
		show_muru_local_txcmd_ctrl(pAd,
			pAd->CommonCfg.rGloInfo.rLocalDataTxCmdCtrl.u4Addr);

	if (!err) {
		offset = OFFSET_OF(MURU_LOCAL_DATA_T, fgMumUl);
		addr = base + offset;
		MTWF_PRINT("|-(0x%08X) fgMumUl = %d\n", addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_muru_txinfo_global_data(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rGlobalData) */
	UINT_32 offset;
	UINT_32 addr;
	uint8_t err = pAd->CommonCfg.rGloInfo.rMuruTxInfoGlobalData.fgError;

	if (!err) {
		/* rGlobalData */
		MTWF_PRINT("|-rGlobalData (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1TxdNum);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxdNum = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1Qid);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Qid = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1TxcmdType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxcmdType = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgSpl);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSpl = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1PresentSpTblIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PresentSpTblIdx = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgTv);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTv = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgDbdcIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgDbdcIdx = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgPreload);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgPreload = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgTxop);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTxop = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1OwnMac);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1OwnMac = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgIgnoreBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgIgnoreBw = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgSmartAnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSmartAnt = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PARA_T, fgSRState);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSRState = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1AggPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AggPolicy = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1Bandwidth);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Bandwidth = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u4AntId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4AntId = %d\n",
			addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1SerialId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SerialId = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1SpeIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SpeIdx = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgOptionalBackoff);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgOptionalBackoff = %d\n",
			addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_muru_txinfo_protect_data(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rProtectData) */
	UINT_32 offset;
	UINT_32 addr, addr_rProtRuInfo;
	UINT_32 subbase_rProtRuInfo;
	uint8_t err = pAd->CommonCfg.rGloInfo.rMuruTxInfoProtectData.fgError;
	UINT_8  i = 0;
	UINT_8  StaCnt = 0;

	if (!err) {
		/* rProtectData */
		MTWF_PRINT("|-rProtectData (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1Protect);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Protect = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1StaCnt);
		addr = subbase + offset;
		StaCnt = muru_io_r_u8(pAd, addr);
		MTWF_PRINT("| |-(0x%08X) u1StaCnt = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgCascadeIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgCascadeIdx = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgCsRequired);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgCsRequired = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1TfPad);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TfPad = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1Rate);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Rate = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1TxMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxMode = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1Nsts);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Nsts = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgCoding);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgCoding = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgDoppler);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgDoppler = %d\n",
			addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, rProtRuInfo);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) rProtRuInfo\n", addr);

		addr_rProtRuInfo = addr;
		for (i = 0; i < StaCnt; i++) {
			subbase_rProtRuInfo =
				addr_rProtRuInfo + sizeof(PROT_RU_INFO_T) * i;
			MTWF_PRINT("| |-|-(0x%08X) rProtRuInfo[%d]\n",
				subbase_rProtRuInfo, i);

			offset = OFFSET_OF(PROT_RU_INFO_T, u2Aid);
			addr = subbase_rProtRuInfo + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2Aid = %d\n",
				addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(PROT_RU_INFO_T, u1RuAlloc);
			addr = subbase_rProtRuInfo + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1RuAlloc = %d\n",
				addr, muru_io_r_u8(pAd, addr));
		}
	}
}

static VOID show_muru_txinfo_tx_data(
	struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rSxnTxData) */
	UINT_32 offset;
	UINT_32 addr, addr_aucRuAlloc;
	UINT_32 subbase_arTxcmdUser, addr_arTxcmdUser;
	uint8_t err = pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTxData.fgError;
	UINT_8  i = 0;
	UINT_8  StaCnt = 0;

	if (!err) {
		/* rSxnTxData */
		MTWF_PRINT("|-rSxnTxData (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Rxv);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Rxv = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRsp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRsp = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgPsIgnore);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgPsIgnore = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBCh1StaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBCh1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBCh2StaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBCh2StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1StaCnt);
		addr = subbase + offset;
		StaCnt = muru_io_r_u8(pAd, addr);
		MTWF_PRINT("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBSym);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBSym = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRa);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRa = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgSigBDcm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSigBDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgSigBCompress);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSigBCompress = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1LtfSym);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Gi);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgStbc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgStbc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgCmdPower);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgCmdPower = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u2MuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MuPpduDur = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1TxPower);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxPower = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, aucRuAlloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) aucRuAlloc\n", addr);

		addr_aucRuAlloc = addr;
		for (i = 0; i < 8; i++) {
			addr = addr_aucRuAlloc + sizeof(UINT_8) * i;
			MTWF_PRINT("| |-|-(0x%08X) aucRuAlloc[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(MURU_TX_DATA_T, fgDoppler);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1PrimaryUserIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PrimaryUserIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Ltf);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1TfPad);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu0UserPosition);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Mu0UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu1UserPosition);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Mu1UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu2UserPosition);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Mu2UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu3UserPosition);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Mu3UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1MuGroupId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MuGroupId = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRu26dSigBCh1);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRu26dSigBCh1 = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRu26uSigBCh2);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRu26uSigBCh2 = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1TxMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxMode = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgDynamicBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgDynamicBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1PreamblePuncture);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PreamblePuncture = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1MuUser);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MuUser = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u2ProtectionDuration);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2ProtectionDuration = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, u2ResponseDuration);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2ResponseDuration = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_TX_DATA_T, arTxcmdUser);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) arTxcmdUser\n", addr);

		addr_arTxcmdUser = addr;
		for (i = 0; i < StaCnt; i++) {
			subbase_arTxcmdUser = addr_arTxcmdUser + sizeof(MURU_USER_INFO_T) * i;
			MTWF_PRINT("| |-|-(0x%08X) arTxcmdUser[%d]\n", subbase_arTxcmdUser, i);

			offset = OFFSET_OF(MURU_USER_INFO_T, u2TxPowerAlpha);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2TxPowerAlpha = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCoding);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgCoding = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2WlanId);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1MuMimoGroup);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1MuMimoGroup = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1MuMimoSpatial);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1MuMimoSpatial = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1StartStream);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1StartStream = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgMultiTid);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgMultiTid = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgRuAllocBn);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1RuAlloc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckGroup);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1AckGroup = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgSuBar);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgSuBar = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgMuBar);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgMuBar = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCbSta);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgCbSta = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgAggOld);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgAggOld = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgPreload);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgPreload = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Rate);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Rate = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Nsts);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Nsts = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1LpCtrl);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1LpCtrl = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgContentCh);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgContentCh = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckPol);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1AckPol = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2SrRate);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2SrRate = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2RuRatio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2RuRatio = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgSplPrimaryUser);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgSplPrimaryUser = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AcSeq);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1AcSeq = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AcNum);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1AcNum = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2BarRuRatio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2BarRuRatio = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2LSigLen);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2LSigLen = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Bw);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac0Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ac0Ratio = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac1Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ac1Ratio = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac2Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ac2Ratio = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac3Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ac3Ratio = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BarRate);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1BarRate = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BarMode);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1BarMode = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BarNsts);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1BarNsts = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BaType);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCsRequired);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgCsRequired = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1LtfType);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1LtfType = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1LtfSym);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgStbc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgStbc = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgLdpcExtraSym);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgLdpcExtraSym = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1PktExt);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1PktExt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCoding2);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgCoding2 = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgDcm);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgBarAckPol);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgBarAckPol = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgAckRuAllocBn);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgAckRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckRuAlloc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1AckRuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckMcs);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1AckMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1SsAlloc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SsAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1TargetRssi);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TargetRssi = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgDoppler);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgBf);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgBf = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1TidInfo);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TidInfo = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2SpatialReuse);
			addr = subbase_arTxcmdUser + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2SpatialReuse = %d\n", addr, muru_io_r_u16(pAd, addr));
		}
	}
}

static VOID show_muru_txinfo_tx_trig_data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rSxnTrigData) */
	UINT_32 offset;
	UINT_32 addr, addr_au1RuAlloc;
	UINT_32 subbase_rTxcmdUserAck, addr_rTxcmdUserAck;
	uint8_t err = pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTrigData.fgError;
	UINT_8  i = 0;
	UINT_8  StaCnt = 0;

	if (!err) {
		/* rSxnTrigData */
		MTWF_PRINT("|-rSxnTrigData (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1Rxv);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Rxv = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1StaCnt);
		addr = subbase + offset;
		StaCnt = muru_io_r_u8(pAd, addr);
		MTWF_PRINT("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1BaPol);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BaPol = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgPriOrder);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgPriOrder = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SplAc);
		addr = subbase + offset;
		MTWF_PRINT(
			"| |-(0x%08X) u1SplAc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1PreambPunc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PreambPunc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1AckTxMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckTxMode = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1TrigType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u4RxHetbCfg1);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4RxHetbCfg1 = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u4RxHetbCfg2);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4RxHetbCfg2 = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1TfPad);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u2LSigLen);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2LSigLen = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBCh1StaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBCh1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBSym);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBSym = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgSigBDcm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSigBDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgSigBCompress);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSigBCompress = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1LtfSym);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1Gi);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgStbc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgStbc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgDoppler);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgCmdPower);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgCmdPower = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBCh2StaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigBCh2StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u2MuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MuPpduDur = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1Ltf);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgRu26dSigBCh1);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRu26dSigBCh1 = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgRu26uSigBCh2);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRu26uSigBCh2 = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, au1RuAlloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au1RuAlloc\n", addr);

		addr_au1RuAlloc = addr;
		for (i = 0; i < 8; i++) {
			addr = addr_au1RuAlloc + sizeof(UINT_8) * i;
			MTWF_PRINT("| |-|-(0x%08X) au1RuAlloc[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1AckTxPower);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckTxPower = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SsnUser);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SsnUser = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1MuUser);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MuUser = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u2MsduId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MsduId = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, rTxcmdUserAck);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) rTxcmdUserAck\n", addr);

		addr_rTxcmdUserAck = addr;
		for (i = 0; i < StaCnt; i++) {
			subbase_rTxcmdUserAck = addr_rTxcmdUserAck + sizeof(MURU_USER_ACK_INFO_T) * i;
			MTWF_PRINT("| |-|-(0x%08X) rTxcmdUserAck[%d]\n", subbase_rTxcmdUserAck, i);

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2StaId);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2StaId = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2AckTxPowerAlpha);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2AckTxPowerAlpha = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgCoding);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgCoding = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgContentCh);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgContentCh = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2WlanId);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgRuAllocBn);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1RuAlloc);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1Rate);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Rate = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1Nsts);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Nsts = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1RuAllNss);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1RuAllNss = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2RuRatio);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2RuRatio = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgSfEnable);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgSfEnable = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1Ac);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ac = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgSplPrimaryUser);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) fgSplPrimaryUser = %d\n", addr, muru_io_r_u8(pAd, addr));
		}
	}
}

static VOID show_muru_tx_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32	base = pAd->CommonCfg.rGloInfo.rMuruTxInfo.u4Addr;
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[3][25] = { {0}, {0} };
	UINT_8  numofparam = 0;
	uint8_t Globaldata = FALSE, ProtectData = FALSE, SxnTxData = FALSE, SxnTrigData = FALSE;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 4) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr && i < 3; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= 25)
			NdisMoveMemory(InputStr[i], macptr, strlen(macptr));
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Length of macprt is too long!\n");
			return;
		}

		if (!(NdisCmpMemory(InputStr[i], "all", strlen("all"))))
			Globaldata = ProtectData = SxnTxData = SxnTrigData = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "Globaldata", strlen("Globaldata"))))
			Globaldata = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "ProtectData", strlen("ProtectData"))))
			ProtectData = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "SxnTxData", strlen("SxnTxData"))))
			SxnTxData = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "SxnTrigData", strlen("SxnTrigData"))))
			SxnTrigData = TRUE;
	}

	SyncMuruSramCheckAddr(pAd, base);

	MTWF_PRINT("_rMuru_Tx_Info; (0x%08X)\n", base);

	if (Globaldata == TRUE)
		show_muru_txinfo_global_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoGlobalData.u4Addr);

	if (ProtectData == TRUE)
		show_muru_txinfo_protect_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoProtectData.u4Addr);

	if (SxnTxData == TRUE)
		show_muru_txinfo_tx_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTxData.u4Addr);

	if (SxnTrigData == TRUE)
		show_muru_txinfo_tx_trig_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTrigData.u4Addr);
}

static VOID show_ShareData_Share_Data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, _rMuru_Share_Data) */
	UINT_32 offset;
	UINT_32 addr, addr_r;
	UINT_8 i;
	uint8_t err = pAd->CommonCfg.rGloInfo.rShareData.fgError;

	if (!err) {
		/* _rMuru_Share_Data */
		MTWF_PRINT("|-_rMuru_Share_Data (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PrimaryAc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PrimaryAc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PrimaryStaIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PrimaryStaIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1Qid);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Qid = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2MuRuMaxSplCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MuRuMaxSplCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1MaxStaCntInPpdu);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MaxStaCntInPpdu = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2TypeAStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TypeAStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2TypeBStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TypeBStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2TypeCStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TypeCStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eBandIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eBandIdx = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1GlobalBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1GlobalBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgBsrpBandRequest);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgBsrpBandRequest\n", addr);

		addr_r = addr;
		for (i = 0; i < RAM_BAND_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) fgBsrpBandRequest[%d] = %d\n",
				addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eLastBsrpBandTx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eLastBsrpBandTx = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PuBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PuBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eTxCmdTye);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eTxCmdTye = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, ePuRuBuftype);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) ePuRuBuftype = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgUplink);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgUplink = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgUlSnd);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgUlSnd = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eSchtype);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eSchtype = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1LastBSRPStaIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LastBSRPStaIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4MaxRuAlgoTimeOut);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MaxRuAlgoTimeOut = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PpduDurBias);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PpduDurBias = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PreGrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PreGrp = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgTxopBurst);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgTxopBurst = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgOptionalBackoff);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgOptionalBackoff = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, i2PsdDiffThr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2PsdDiffThr = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgExp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgExp = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1Pdc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Pdc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgProt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgProt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4ProtFrameThr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4ProtFrameThr = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1ProtRuAlloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1ProtRuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgFixedRate);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgFixedRate = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TxDataSec_Bw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxDataSec_Bw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4TxDataSec_MuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4TxDataSec_MuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TrigSec_BA_Policy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigSec_BA_Policy = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TrigSec_Global_BA_BW);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigSec_Global_BA_BW = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4TrigSec_Global_BA_Dur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4TrigSec_Global_BA_Dur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eTonePlanPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eTonePlanPolicy = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1FixedMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedNss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1FixedNss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedBaMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1FixedBaMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedBaNss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1FixedBaNss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4PpduDuration);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4PpduDuration = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgUlMuBa);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgUlMuBa = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2UlAvgMpduCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2UlAvgMpduCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4UlAvgMpduSize);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4UlAvgMpduSize = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1MaxMuNum);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MaxMuNum = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TypeA_SwPdaPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TypeA_SwPdaPolicy = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TypeB_SwPdaPolicy);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TypeB_SwPdaPolicy = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2MpduByte);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MpduByte = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2QidNeedsDlSplTrigger);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2QidNeedsDlSplTrigger = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2NonBsrpCount);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2NonBsrpCount = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1SplBackupSeq);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SplBackupSeq = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4AcBitmapPreviousBsrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4AcBitmapPreviousBsrp = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TriggerTypeOfBsrpTimer);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TriggerTypeOfBsrpTimer = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1DisableBsrpTimer);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1DisableBsrpTimer = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1DisableULData);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1DisableULData = %d\n", addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_ShareData_RuAlloc_Data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, rRuAllocData) */
	UINT_32 offset;
	UINT_32 addr;
	uint8_t err = pAd->CommonCfg.rGloInfo.rShareDataRuAllocData.fgError;

	if (!err) {
		/* rRuAllocData */
		MTWF_PRINT("|-rRuAllocData (0x%08X)\n", subbase);

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, eBand);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eBand = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AggPol);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AggPol = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1Ac);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Ac = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, txCmdType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) txCmdType = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SerialId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SerialId = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SpeIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SpeIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SigbSym);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigbSym = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1LtfSym);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SigbMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SigbMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1GiType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1GiType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1LtfType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LtfType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1StaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, eTxMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eTxMode = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AckGiType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckGiType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AckLtfType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckLtfType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AckMaxNss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckMaxNss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TxPwr_dBm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxPwr_dBm = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1Bw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1PrimaryUserIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1PrimaryUserIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MuPpduDuration);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MuPpduDuration = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxBaMuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MaxBaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxBaDurForLSig);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MaxBaDurForLSig = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1GrpId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1GrpId = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigBaPL);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigBaPL = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TfType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TfType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigSplAc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigSplAc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigAckBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigAckBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigAckTxPwr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigAckTxPwr = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigAckTxMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TrigAckTxMode = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4LSigLength);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4LSigLength = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, ucTfPe);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) ucTfPe = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TotMumGrpCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TotMumGrpCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, eSchType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eSchType = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1OperateBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1OperateBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1HavmDLULIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1HavmDLULIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SplStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SplStaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u2TonePlanIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TonePlanIdx = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u2TypeAStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TypeAStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u2TypeBStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TypeBStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxHeadTime);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MaxHeadTime = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxScore);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MaxScore = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4SuScore);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4SuScore = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MuScore);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MuScore = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4TotBitsOfThisTP);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4TotBitsOfThisTP = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4PpduTxDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4PpduTxDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MuPpduUtilization);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4MuPpduUtilization = %d\n", addr, muru_io_r_u32(pAd, addr));
	}
}

static VOID show_ShareData_User_Info(struct _RTMP_ADAPTER *pAd, UINT32 base, uint16_t ArrayIdx)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, userInfo) */
	UINT_32 offset;
	UINT_32 addr, addr_u1AcRatio;
	uint8_t err = pAd->CommonCfg.rGloInfo.rShareDataUserInfo.fgError;
	UINT_8  i = 0;

	subbase = subbase + sizeof(PER_USER_INFO) * ArrayIdx;

	if (!err) {
		/* userInfo */
		MTWF_PRINT("|-userInfo (0x%08X)\n", subbase);

		offset = OFFSET_OF(PER_USER_INFO, u2WlanId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgUserPreLoad);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgUserPreLoad = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1MuMimoGrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MuMimoGrp = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1RuAlloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1RuTreeMapArrayIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1RuTreeMapArrayIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1RuMapArrayIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1RuMapArrayIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgRuAllocBn);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1MuMimoSpatial);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MuMimoSpatial = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1StartStream);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1StartStream = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1RateMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1RateMode = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1Nss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Nss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1StartSpatialStream);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1StartSpatialStream = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1Mcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Mcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1Gi);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgLdpc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgLdpc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u2WeightFactor);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2WeightFactor = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1SrMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SrMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1UpperMCS);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1UpperMCS = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgDcm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u2RuRatio);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2RuRatio = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1RuAllNss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1RuAllNss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgAggOld);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgAggOld = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgCB);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgCB = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AckBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AcSeq);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AcSeq = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AcNum);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AcNum = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u2BarRuRatio);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2BarRuRatio = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AcRatio);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AcRatio\n", addr);

		addr_u1AcRatio = addr;
		for (i = 0; i < 4; i++) {
			addr = addr_u1AcRatio + sizeof(UINT_8) * i;
			MTWF_PRINT("| |-|-(0x%08X) u1AcRatio[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(PER_USER_INFO, u2MumGrpIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MumGrpIdx = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u2MumGrpStaCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2MumGrpStaCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1LtfType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LtfType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgSplPrimaryUser);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSplPrimaryUser = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1BfType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BfType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AckPol);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckPol = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AckGrp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckGrp = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgSuBar);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgSuBar = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgMuBar);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgMuBar = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1BarRate);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BarRate = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1BarMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BarMode = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1BarNsts);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BarNsts = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1BaType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u4BaMuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4BaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u4BaLSigDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4BaLSigDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgBaDcm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgBaDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgBaStbc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgBaStbc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AckRuAlloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckRuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgAckRuAllocBn);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgAckRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AckMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1AckNss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1AckNss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgAckLdpc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgAckLdpc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1BarAckPol);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BarAckPol = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1SsAaloc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1SsAaloc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1TargetRssi);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TargetRssi = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u1TidInfo);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TidInfo = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u2EffSnr);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2EffSnr = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u2TxPwrAlpha_dB);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2TxPwrAlpha_dB = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u4RuScore);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4RuScore = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, u4StaMuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4StaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(PER_USER_INFO, fgLargeRu);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgLargeRu = %d\n", addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_ShareData_StaRu_Record(struct _RTMP_ADAPTER *pAd, UINT32 base, uint16_t ArrayIdx)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, arStaRuRecord) */
	UINT_32 offset;
	UINT_32 addr;
	UINT_32 addr_r;
	UINT_8 i;
	UINT_32 addr_subbase;
	uint8_t err = pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.fgError;

	subbase = subbase + sizeof(STA_MURU_RECORD_T) * ArrayIdx;

	if (!err) {
		/* arStaRuRecord */
		MTWF_PRINT("|-arStaRuRecord (0x%08X)\n", subbase);

		offset = OFFSET_OF(STA_MURU_RECORD_T, eStaRecCapMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eStaRecCapMode = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Bw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1TxBw);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1TxBw = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2WlanId);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2StaIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2StaIdx = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2NextStaRecIdxbySPL);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2NextStaRecIdxbySPL = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BandIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BandIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1MumCapBitmap);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MumCapBitmap = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BfType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BfType = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Mcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Mcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Nss);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Nss = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Gi);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Ecc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Ecc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeLtf);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1HeLtf = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Stbc);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Stbc = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Priority);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1Priority = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, afgNonEmptyState);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) afgNonEmptyState\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) afgNonEmptyState[%d] = %d\n",
				addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BsrpPeriod);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BsrpPeriod = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BsrpMaxPeriod);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BsrpMaxPeriod = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BsrpMissCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BsrpMissCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BsrpHitCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1BsrpHitCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au1DlQuantum);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au1DlQuantum\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) au1DlQuantum[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au1UlQuantum);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au1UlQuantum\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) au1UlQuantum[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1DelayWeight);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1DelayWeight = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeSndPeriod);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1HeSndPeriod = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeSndMaxPeriod);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1HeSndMaxPeriod = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, fgRtsForMuPpduRetry);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) fgRtsForMuPpduRetry\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) fgRtsForMuPpduRetry[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1LastTxMcs);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LastTxMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i1UlPwrHeadroom_dB);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i1UlPwrHeadroom_dB = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1MinTxPwrNonHitCnt);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1MinTxPwrNonHitCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, afgCanNotAgg);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) afgCanNotAgg\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) afgCanNotAgg[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2MinRssi_dBm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2MinRssi_dBm = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1LastRuBitmapIdx);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u1LastRuBitmapIdx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2LastPerUserRssi_dBm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2LastPerUserRssi_dBm = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2PreGrpMaxPsd_dBm);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2PreGrpMaxPsd_dBm = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2RssiOffset_dB);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) i2RssiOffset_dB = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2DelayReq);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2DelayReq = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2MpduCntInPpdu);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2MpduCntInPpdu\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2MpduCntInPpdu[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2RxAvgMpduSize);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2RxAvgMpduSize\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2RxAvgMpduSize[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2CurrMsn);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2CurrMsn\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2CurrMsn[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2BaWin);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2BaWin\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2BaWin[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2NewMpduCntInPpdu);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2NewMpduCntInPpdu\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2NewMpduCntInPpdu[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2RxPer);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2RxPer\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2RxPer[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2HeadPktLen);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2HeadPktLen\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2HeadPktLen[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2UlHeadPktDelay);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2UlHeadPktDelay\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2UlHeadPktDelay[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2RxAvgLongTermMpduSize);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au2RxAvgLongTermMpduSize\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) au2RxAvgLongTermMpduSize[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2WeightFactor);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u2WeightFactor = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, eDataTxMode);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eDataTxMode = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, ePpduType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) ePpduType = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, eDL_LTPpduType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eDL_LTPpduType\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) eDL_LTPpduType[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, eUL_LTPpduType);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eUL_LTPpduType\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) eUL_LTPpduType[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, eHeStaStae);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eHeStaStae = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, eBand);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) eBand = %d\n", addr, muru_io_r_u32(pAd, addr));

		/*Start of Bitwise fields*/
		addr = addr + 4;
		MTWF_PRINT("| |-(0x%08X) fgIsAddBaForAnyTIDsOfAC0 = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000001) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgIsAddBaForAnyTIDsOfAC1 = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000002) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgIsAddBaForAnyTIDsOfAC2 = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000004) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgIsAddBaForAnyTIDsOfAC3 = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000008) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgNonAggressiveRA = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000010) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgBsrpCandidate = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000020) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgBsrpTriggerCurPPDU = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000040) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgBsrpHasSentInBasicTF = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000080) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fg20MOnlyCap = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000100) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgHeSndCandidate = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000200) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgHeSndTriggerCurPPDU = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000400) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgPsMode = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00000800) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgUlSuSnd = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00001000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgSrAbortBit = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00002000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgDepCmd = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00004000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgIsTpcInfoValid = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00008000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgIsTriggerred = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00010000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgTcp = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00020000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgMinTxPwrFlag = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00040000) != 0) ? "TRUE" : "FALSE");

		MTWF_PRINT("| |-(0x%08X) fgHaveHitMinTxPwrFg = %s\n", addr,
				((muru_io_r_u32(pAd, addr) & 0x00080000) != 0) ? "TRUE" : "FALSE");

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4DlTotQlenBytes);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4DlTotQlenBytes\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) au4DlTotQlenBytes[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4UlTotQlenBytes);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4UlTotQlenBytes\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) au4UlTotQlenBytes[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u4UlTotAllQlenBytes);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4UlTotAllQlenBytes = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4HeadPktTime);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4HeadPktTime\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) au4HeadPktTime[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4ByesInPpdu);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4ByesInPpdu\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) au4ByesInPpdu[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4UlSchTimeStamp);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4UlSchTimeStamp\n", addr);

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) au4UlSchTimeStamp[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u4RuScore);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4RuScore = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u4StaMuPpduDur);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) u4StaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4TidQueueSizeBytes);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4TidQueueSizeBytes\n", addr);

		addr_r = addr;
		for (i = 0; i < MAX_TID_NUM; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_PRINT("| | |-(0x%08X) au4TidQueueSizeBytes[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4ServiceBytesRxPerSecond);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4ServiceBytesRxPerSecond\n", addr);

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_PRINT("| |-|-(0x%08X) au4ServiceBytesRxPerSecond[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4TotBytesRxInService);
		addr = subbase + offset;
		MTWF_PRINT("| |-(0x%08X) au4TotBytesRxInService\n", addr);

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_PRINT("| |-|-(0x%08X) au4TotBytesRxInService[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr));
		}
	}
}

static VOID show_muru_shared_data(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32	base = pAd->CommonCfg.rGloInfo.rShareData.u4Addr;
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	uint8_t ShareData = FALSE, RuAllocData = FALSE, UserInfo = FALSE, StaRuRecord = FALSE;
	UINT_16 ArrayIdx = 0;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 3) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr && i < 3; macptr = rstrtok(NULL, "-"), i++) {

		if (i == 0) {
			if (strlen(macptr) <= sizeof(InputStr))
				NdisMoveMemory(InputStr, macptr, strlen(macptr));
			else {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Length of macprt is too long!\n");
				return;
			}

			if (!(NdisCmpMemory(InputStr, "ShareData", strlen("ShareData"))))
				ShareData = TRUE;

			if (!(NdisCmpMemory(InputStr, "RuAllocData", strlen("RuAllocData"))))
				RuAllocData = TRUE;

			if (!(NdisCmpMemory(InputStr, "UserInfo", strlen("UserInfo"))))
				UserInfo = TRUE;

			if (!(NdisCmpMemory(InputStr, "StaRuRecord", strlen("StaRuRecord"))))
				StaRuRecord = TRUE;
		}

		if ((i == 1) && ((UserInfo == TRUE) || (StaRuRecord == TRUE)))
			ArrayIdx = (uint16_t)os_str_tol(macptr, 0, 10);
	}

	SyncMuruSramCheckAddr(pAd, base);

	MTWF_PRINT("_rMuru_Shared_Data; (0x%08X)\n", base);

	if (ShareData == TRUE)
		show_ShareData_Share_Data(pAd, pAd->CommonCfg.rGloInfo.rShareData.u4Addr);

	if (RuAllocData == TRUE)
		show_ShareData_RuAlloc_Data(pAd, pAd->CommonCfg.rGloInfo.rShareDataRuAllocData.u4Addr);

	if (UserInfo == TRUE)
		show_ShareData_User_Info(pAd, pAd->CommonCfg.rGloInfo.rShareDataUserInfo.u4Addr, ArrayIdx);

	if (StaRuRecord == TRUE)
		show_ShareData_StaRu_Record(pAd, pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.u4Addr, ArrayIdx);
}

static VOID show_muru_mancfg_ctrl(struct _RTMP_ADAPTER *pAd)
{
	UINT_32 subbase, subbase_r;
	UINT_32 offset;
	UINT_32 addr;
	uint8_t err;
	UINT_8  i = 0;

	subbase = pAd->CommonCfg.rGloInfo.rMuruCmdManCfgInf.u4Addr;
	err = pAd->CommonCfg.rGloInfo.rMuruCmdManCfgInf.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_PRINT("|-rMuruExtCmdManCfgInf (0x%08X)\n", subbase);
		for (i = 0; i < 2; i++) {
#ifdef WIFI_UNIFIED_COMMAND
			subbase += sizeof(struct UNI_MURU_MANUAL_CONFIG_T) * i;

			MTWF_PRINT("| |-rMuruExtCmdManCfgInf[%d] (0x%08X)\n", i, subbase);

			offset = OFFSET_OF(struct UNI_MURU_MANUAL_CONFIG_T, u4ManCfgBmpCmm);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) u4ManCfgBmpCmm = %d\n", addr, muru_io_r_u32(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_MANUAL_CONFIG_T, u4ManCfgBmpDl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) u4ManCfgBmpDl = %d\n", addr, muru_io_r_u32(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_MANUAL_CONFIG_T, u4ManCfgBmpUl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) u4ManCfgBmpUl = %d\n", addr, muru_io_r_u32(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_MANUAL_CONFIG_T, rCfgCmm);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) rCfgCmm\n", addr);

			subbase_r = addr;

			offset = OFFSET_OF(struct UNI_MURU_CMM_MANUAL_CONFIG, u1PpduFmt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1PpduFmt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_CMM_MANUAL_CONFIG, u1SchType);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SchType = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_CMM_MANUAL_CONFIG, u1Band);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Band = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_CMM_MANUAL_CONFIG, u1Ac);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1WmmSet = %d\n", addr, muru_io_r_u8(pAd, addr));

			/* MURU_DL_MANUAL_CONFIG */
			offset = OFFSET_OF(struct UNI_MURU_MANUAL_CONFIG_T, rCfgDl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) rCfgDl\n", addr);

			subbase_r = addr;

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1UserCnt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UserCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1TxMode);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TxMode = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1Bw);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1GI);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1GI = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1Ltf);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1SigMcs);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SigMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1SigDcm);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SigDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_DL_MANUAL_CONFIG, u1SigCmprs);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SigCmprs = %d\n", addr, muru_io_r_u8(pAd, addr));

			/* MURU_DL_MANUAL_CONFIG */
			offset = OFFSET_OF(struct UNI_MURU_MANUAL_CONFIG_T, rCfgUl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) rCfgUl\n", addr);

			subbase_r = addr;

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u1UserCnt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UserCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u1TrigType);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TrigType = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u2TrigCnt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2TrigCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u2TrigIntv);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2TrigIntv = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u1UlBw);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UlBw = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u1UlGiLtf);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UlGiLtf = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u2UlLength);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2UlLength = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u1TfPad);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(struct UNI_MURU_UL_MANUAL_CONFIG, u1BaType);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr));
#else
			subbase += sizeof(CMD_MURU_MANCFG_INTERFACER) * i;

			MTWF_PRINT("| |-rMuruExtCmdManCfgInf[%d] (0x%08X)\n", i, subbase);

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, u4ManCfgBmpCmm);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) u4ManCfgBmpCmm = %d\n", addr, muru_io_r_u32(pAd, addr));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, u4ManCfgBmpDl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) u4ManCfgBmpDl = %d\n", addr, muru_io_r_u32(pAd, addr));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, u4ManCfgBmpUl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) u4ManCfgBmpUl = %d\n", addr, muru_io_r_u32(pAd, addr));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, rCfgCmm);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) rCfgCmm\n", addr);

			subbase_r = addr;

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1PpduFmt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1PpduFmt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1SchType);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SchType = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1Band);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Band = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1WmmSet);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1WmmSet = %d\n", addr, muru_io_r_u8(pAd, addr));

			/* MURU_DL_MANUAL_CONFIG */
			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, rCfgDl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) rCfgDl\n", addr);

			subbase_r = addr;

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1UserCnt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UserCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1TxMode);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TxMode = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1Bw);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1GI);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1GI = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1Ltf);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1SigBMcs);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SigBMcs = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1SigBDcm);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SigBDcm = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1SigBCmprs);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1SigBCmprs = %d\n", addr, muru_io_r_u8(pAd, addr));

			/* MURU_DL_MANUAL_CONFIG */
			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, rCfgUl);
			addr = subbase + offset;
			MTWF_PRINT("| |-|-(0x%08X) rCfgUl\n", addr);

			subbase_r = addr;

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1UserCnt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UserCnt = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1TrigType);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TrigType = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u2TrigCnt);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2TrigCnt = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u2TrigIntv);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2TrigIntv = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1UlBw);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UlBw = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1UlGiLtf);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1UlGiLtf = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u2UlLength);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u2UlLength = %d\n", addr, muru_io_r_u16(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1TfPad);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1BaType);
			addr = subbase_r + offset;
			MTWF_PRINT("| |-|-|-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr));
#endif /* WIFI_UNIFIED_COMMAND */
		}
	}

	subbase = pAd->CommonCfg.rGloInfo.rMuTxPktCnt.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuTxPktCnt.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_PRINT("|-rMuTxPktCnt (0x%08X)\n", subbase);
		MTWF_PRINT("| |-(0x%08X) u4MuTxPktCnt[0] = %d\n", subbase, muru_io_r_u32(pAd, subbase));

		subbase = subbase + 4;
		MTWF_PRINT("| |-(0x%08X) u4MuTxPktCnt[1] = %d\n", subbase, muru_io_r_u32(pAd, subbase));
	}

	subbase = pAd->CommonCfg.rGloInfo.rMuTxPktCntDwn.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuTxPktCntDwn.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_PRINT("|-rMuTxPktCntDwn (0x%08X)\n", subbase);
		MTWF_PRINT("| |-(0x%08X) u4MuTxPktCntDwn[0] = %d\n", subbase, muru_io_r_u32(pAd, subbase));

		subbase = subbase + 4;
		MTWF_PRINT("| |-(0x%08X) u4MuTxPktCntDwn[1] = %d\n", subbase, muru_io_r_u32(pAd, subbase));
	}

	subbase = pAd->CommonCfg.rGloInfo.rAggPolicy.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rAggPolicy.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_PRINT("|-rAggPolicy (0x%08X)\n", subbase);
		MTWF_PRINT("| |-(0x%08X) u1AggPolicy = %d\n", subbase, muru_io_r_u8(pAd, subbase));
	}

	subbase = pAd->CommonCfg.rGloInfo.rDurationComp.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rDurationComp.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_PRINT("|-rDurationComp (0x%08X)\n", subbase);
		MTWF_PRINT("| |-(0x%08X) u1DurationComp = %d\n", subbase, muru_io_r_u8(pAd, subbase));
	}
}

static VOID show_muru_stacapinfo_ctrl(struct _RTMP_ADAPTER *pAd, UINT_16 WlanIdx)
{
	UINT_32 subbase, subbase_r;
	UINT_32 offset;
	UINT_32 addr;
	uint8_t err;

	subbase = pAd->CommonCfg.rGloInfo.rMuruStaCapInfo.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuruStaCapInfo.fgError;
	subbase = subbase + sizeof(MURU_PURE_STACAP_INFO) * WlanIdx;
	SyncMuruSramCheckAddr(pAd, subbase);

	if (!err) {
		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rDlOfdma);
		addr = subbase + offset;
		MTWF_PRINT("|-(0x%08X) rDlOfdma\n", addr);

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1PhyPunRx);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1PhyPunRx = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u120MIn40M2G);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u120MIn40M2G = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u120MIn160M);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u120MIn160M = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u180MIn160M);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u180MIn160M = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1Lt16SigB);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1Lt16SigB = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1RxSUCompSigB);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1RxSUCompSigB = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1RxSUNonCompSigB);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1RxSUNonCompSigB = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rUlOfdma);
		addr = subbase + offset;
		MTWF_PRINT("|-(0x%08X) rUlOfdma\n", addr);

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1TrigFrmPad);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1TrigFrmPad = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1MuCascading);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1MuCascading = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1UoRa);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1UoRa = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u12x996Tone);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u12x996Tone = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1RxTrgFrmBy11ac);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) u1RxTrgFrmBy11ac = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rDlMimo);
		addr = subbase + offset;
		MTWF_PRINT("|-(0x%08X) rDlMimo\n", addr);

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_DL_MIMO, fgVhtMuBfee);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) fgVhtMuBfee = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_DL_MIMO, fgParBWDlMimo);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) fgParBWDlMimo = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rUlMimo);
		addr = subbase + offset;
		MTWF_PRINT("|-(0x%08X) rUlMimo\n", addr);

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_UL_MIMO, fgFullUlMimo);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) fgFullUlMimo = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(MURU_STA_UL_MIMO, fgParUlMimo);
		addr = subbase_r + offset;
		MTWF_PRINT("|-|-(0x%08X) fgParUlMimo = %d\n", addr, muru_io_r_u8(pAd, addr));
	}
}

static VOID show_muru_mancfg_data(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	uint8_t ManCfg = FALSE;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 2) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr && i < 2; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= sizeof(InputStr))
			NdisMoveMemory(InputStr, macptr, strlen(macptr));
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Length of macprt is too long!\n");
			return;
		}

		if (!(NdisCmpMemory(InputStr, "all", strlen("all"))))
			ManCfg = TRUE;
	}

	MTWF_PRINT("_rMuru_ManCfg_Data\n");
	if (ManCfg == TRUE)
		show_muru_mancfg_ctrl(pAd);
}

static VOID show_muru_stacap_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	UINT_16 WlanIdx = 0;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 2) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr && i < 2; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= 25)
			NdisMoveMemory(InputStr, macptr, strlen(macptr));
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Length of macprt is too long!\n");
			return;
		}

		WlanIdx = (uint16_t)os_str_tol(macptr, 0, 10);
	}

	MTWF_PRINT("_Muru_StaCap_Info\n");
	show_muru_stacapinfo_ctrl(pAd, WlanIdx);
}

static VOID show_muru_mum_group_tbl_entry(struct _RTMP_ADAPTER *pAd, UINT_16 GroupIdx)
{
	UINT_32 subbase;
	UINT_32 addr;
	uint8_t err;
	UINT_32 DW_Value = 0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW0 pDW0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW1 pDW1;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW2 pDW2;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW3 pDW3;

	subbase = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.fgError;

	subbase = subbase + sizeof(MURU_MUM_GROUP_TBL_ENTRY_T) * GroupIdx;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_PRINT("MURU MUM GROUP TABLE ENTRY: GROUP IDX = %d\n", GroupIdx);
		/* DW0 */
		addr = subbase;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_PRINT("|-DW0 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value);
		pDW0 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW0)&DW_Value;

		MTWF_PRINT("| |-u1NumUser    = %u\n", pDW0->rField.u1NumUser);
		MTWF_PRINT("| |-u1DlGi       = %u\n", pDW0->rField.u1DlGi);
		MTWF_PRINT("| |-u1UlGi       = %u\n", pDW0->rField.u1UlGi);
		MTWF_PRINT("| |-u1Ax         = %u\n", pDW0->rField.u1Ax);
		MTWF_PRINT("| |-u1PFIDUser0  = %u\n", pDW0->rField.u1PFIDUser0);
		MTWF_PRINT("| |-u1PFIDUser1  = %u\n", pDW0->rField.u1PFIDUser1);
		MTWF_PRINT("| |-u1PFIDUser2  = %u\n", pDW0->rField.u1PFIDUser2);
		MTWF_PRINT("| |-u1PFIDUser3  = %u\n", pDW0->rField.u1PFIDUser3);
		MTWF_PRINT("| |-u1DlVld      = %u\n", pDW0->rField.u1DlVld);
		MTWF_PRINT("| |-u1UlVld      = %u\n", pDW0->rField.u1UlVld);

		/* DW1 */
		addr = subbase + 4;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_PRINT("|-DW1 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value);
		pDW1 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW1)&DW_Value;

		MTWF_PRINT("| |-u1RuAlloc    = %u\n", pDW1->rField.u1RuAlloc);
		MTWF_PRINT("| |-u1NssUser0   = %u\n", pDW1->rField.u1NssUser0);
		MTWF_PRINT("| |-u1NssUser1   = %u\n", pDW1->rField.u1NssUser1);
		MTWF_PRINT("| |-u1NssUser2   = %u\n", pDW1->rField.u1NssUser2);
		MTWF_PRINT("| |-u1NssUser3   = %u\n", pDW1->rField.u1NssUser3);

		/* DW2 */
		addr = subbase + 8;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_PRINT("|-DW2 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value);
		pDW2 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW2)&DW_Value;

		MTWF_PRINT("| |-u1DlMcsUser0 = %u\n", pDW2->rField.u1DlMcsUser0);
		MTWF_PRINT("| |-u1DlMcsUser1 = %u\n", pDW2->rField.u1DlMcsUser1);
		MTWF_PRINT("| |-u1DlMcsUser2 = %u\n", pDW2->rField.u1DlMcsUser2);
		MTWF_PRINT("| |-u1DlMcsUser3 = %u\n", pDW2->rField.u1DlMcsUser3);
		MTWF_PRINT("| |-u1DlWfUser0  = %u\n", pDW2->rField.u1DlWfUser0);
		MTWF_PRINT("| |-u1DlWfUser1  = %u\n", pDW2->rField.u1DlWfUser1);
		MTWF_PRINT("| |-u1DlWfUser2  = %u\n", pDW2->rField.u1DlWfUser2);
		MTWF_PRINT("| |-u1DlWfUser3  = %u\n", pDW2->rField.u1DlWfUser3);

		/* DW3 */
		addr = subbase + 12;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_PRINT("|-DW3 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value);
		pDW3 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW3)&DW_Value;

		MTWF_PRINT("| |-u1UlMcsUser0 = %u\n", pDW3->rField.u1UlMcsUser0);
		MTWF_PRINT("| |-u1UlMcsUser1 = %u\n", pDW3->rField.u1UlMcsUser1);
		MTWF_PRINT("| |-u1UlMcsUser2 = %u\n", pDW3->rField.u1UlMcsUser2);
		MTWF_PRINT("| |-u1UlMcsUser3 = %u\n", pDW3->rField.u1UlMcsUser3);
		MTWF_PRINT("| |-u1UlWfUser0  = %u\n", pDW3->rField.u1UlWfUser0);
		MTWF_PRINT("| |-u1UlWfUser1  = %u\n", pDW3->rField.u1UlWfUser1);
		MTWF_PRINT("| |-u1UlWfUser2  = %u\n", pDW3->rField.u1UlWfUser2);
		MTWF_PRINT("| |-u1UlWfUser3  = %u\n", pDW3->rField.u1UlWfUser3);
	}
}

static VOID show_mumimo_group_entry_tbl(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	UINT_16 GroupIdx = 0;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 2) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Num of Input Parameters Wrong\n");
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr && i < 2; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= sizeof(InputStr))
			NdisMoveMemory(InputStr, macptr, strlen(macptr));
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Length of macprt is too long!\n");
			return;
		}

		GroupIdx = (uint16_t)os_str_tol(macptr, 0, 10);
	}

	if (GroupIdx >= 512) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "Group Entry Idx is Wrong\n");
		return;
	}
	show_muru_mum_group_tbl_entry(pAd, GroupIdx);
}

static VOID show_candidate_list(struct _RTMP_ADAPTER *pAd, UINT_8 counter, UINT32 addr)
{
	UINT_8 i, j, value, bit;
	UINT_32 addr_3, addr_4;
	RTMP_STRING * group[3] = {" 8: ", "36: ", "71: "};
	UINT_8 u1CnBits = 0, u1MaxBits;

	for (i = 0; i < counter; i++) {
		addr_3 = addr + sizeof(UINT_8) * i * MAX_CAP_MUM_GRP_BLOCK;

		if (i == MUM_GRP_CN_2)
			u1CnBits = MAX_CAP_MUM_GRP_BLOCK - 1;
		else if (i == MUM_GRP_CN_3)
			u1CnBits = MAX_CAP_MUM_GRP_BLOCK;
		else if (i == MUM_GRP_CN_4)
			u1CnBits = 1;

		u1MaxBits = 8;
		MTWF_PRINT("%s", group[i]);

		for (j = 0; j < u1CnBits; j++) {
			addr_4 = addr_3 + sizeof(UINT_8) * j;
			value = muru_io_r_u8(pAd, addr_4);

			if ((i == MUM_GRP_CN_2) && (j == u1CnBits - 1))
				u1MaxBits = 4;
			else if ((i == MUM_GRP_CN_3) && (j == u1CnBits - 1))
				u1MaxBits = 3;
			else if ((i == MUM_GRP_CN_4) && (j == u1CnBits - 1))
				u1MaxBits = 5;

			for (bit = 0; bit < u1MaxBits; bit++) {
				if ((value & BIT(bit)) != 0) {
					if (i == 0)
						MTWF_PRINT(" %d, ", 1);
					else if (i == 1)
						MTWF_PRINT("\x1b[31m %d\x1b[0m, ", 1);
					else if (i == 2)
						MTWF_PRINT("\x1b[32m %d\x1b[0m, ", 1);
				} else {
					if (i == 0)
						MTWF_PRINT(" %d, ", 0);
					else if (i == 1)
						MTWF_PRINT("\x1b[31m %d\x1b[0m, ", 0);
					else if (i == 2)
						MTWF_PRINT("\x1b[32m %d\x1b[0m, ", 0);
				}
			}
		}
		MTWF_PRINT("\n");
	}
}

static VOID show_mumimo_algorithm_monitor1(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32 base, base_starec, offset, offset_starec, subbase_starec;
	UINT_8 i, j, k, l = 0;
	uint8_t err, err_starec, err_mu_tx;
	UINT_32 addr, addr_, addr1, addr_2, addr_starec;
	UINT_32 u4SuccessCount[MURU_MAX_GROUP_CN][MURU_MUM_MAX_PFID_NUM];
	UINT_32 u4TotalCount[MURU_MAX_GROUP_CN][MURU_MUM_MAX_PFID_NUM];
	UINT_32 base_mu_tx, subbase;
	UINT_8  PfiIdx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	UINT_16 Muru_mum_usr_mgmt[RAM_BAND_NUM][MURU_MUM_MAX_PFID_NUM];
	RTMP_STRING *group_cap[5] = {"VHT_CAP", "HE_DLFBMUM_CAP", "HE_DLPBMUM_CAP", "HE_ULFBMUM_CAP", "HE_ULPBMUM_CAP"};
	UINT_16 WlanIdx[8];
	UINT_8  MumCapBitmap[MURU_MUM_MAX_PFID_NUM] = {0};
	UINT_8 counter;

	base = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.u4Addr;
	err  = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.fgError;

	base_starec = pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.u4Addr;
	err_starec = pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.fgError;

	base_mu_tx = pAd->CommonCfg.rGloInfo.rMuruTxStatInfo.u4Addr;
	err_mu_tx = pAd->CommonCfg.rGloInfo.rMuruTxStatInfo.fgError;

	SyncMuruSramCheckAddr(pAd, base);
	SyncMuruSramCheckAddr(pAd, base_starec);
	SyncMuruSramCheckAddr(pAd, base_mu_tx);

	counter = MUM_GRP_USR_NUM_4;
	memset(Muru_mum_usr_mgmt, 0, sizeof(UINT_16) * RAM_BAND_NUM * MURU_MUM_MAX_PFID_NUM);
	memset(u4SuccessCount, 0, sizeof(UINT_32) * MURU_MAX_GROUP_CN * MURU_MUM_MAX_PFID_NUM);
	memset(u4TotalCount, 0, sizeof(UINT_32) * MURU_MAX_GROUP_CN * MURU_MUM_MAX_PFID_NUM);

	if (err)
		return;

	MTWF_PRINT("|-(0x%08X)MURU MU ALGORITHM MONITOR:\n", base);

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, au2MuProfileIdxToWlanIdx);
	addr_ = base + offset;

	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		addr = addr_ + sizeof(UINT_16) * i;
		WlanIdx[i] = muru_io_r_u16(pAd, addr);
		if (WlanIdx[i] > STA_REC_NUM)
			WlanIdx[i] = 0;
	}

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, arMuUserMgmt);
	addr_ = base + offset;

	for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
		addr_2 = addr_ + sizeof(UINT_16) * j * RAM_BAND_NUM;
		if (WlanIdx[j] != 0) {
			for (i = 0; i < RAM_BAND_NUM; i++) {
				addr = addr_2 + sizeof(UINT_16) * i;
				Muru_mum_usr_mgmt[i][j] = muru_io_r_u16(pAd, addr);
			}
		}
	}

	MTWF_PRINT("\tMURU MUM PFID IDX\n\t");
	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		if (i == MURU_MUM_MAX_PFID_NUM - 1)
			MTWF_PRINT("\t%d", PfiIdx[i]);
		else
			MTWF_PRINT("\t%d,", PfiIdx[i]);
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tWLAN IDX\n");

	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		if (i == MURU_MUM_MAX_PFID_NUM - 1)
			MTWF_PRINT("\t%d", WlanIdx[i]);
		else
			MTWF_PRINT("\t%d,", WlanIdx[i]);
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\n MURU MUM USER GROUP COUNT\n");

	for (i = 0; i < RAM_BAND_NUM; i++) {
		MTWF_PRINT("\tBAND%d USER GROUP COUNT\n\t", i);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
			if (j == MURU_MUM_MAX_PFID_NUM - 1)
				MTWF_PRINT("\t%d", Muru_mum_usr_mgmt[i][j]);
			else
				MTWF_PRINT("\t%d,", Muru_mum_usr_mgmt[i][j]);
		}
			MTWF_PRINT("\n\n");
	}

	if (!err_starec) {
		MTWF_PRINT(
			"PFID  VHT_CAP  HE_DLFBMUM_CAP  HE_DLPBMUM_CAP  HE_ULFBMUM_CAP  HE_ULPBMUM_CAP\n");
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			if (WlanIdx[i] != 0) {
				subbase_starec = base_starec + sizeof(STA_MURU_RECORD_T) * WlanIdx[i];

				offset_starec = OFFSET_OF(STA_MURU_RECORD_T, u1MumCapBitmap);
				addr_starec = subbase_starec + offset_starec;
				MumCapBitmap[i] = muru_io_r_u8(pAd, addr_starec);

				MTWF_PRINT("%d", i);

				for (j = 0; j < MU_MAX_GRP_USR_CAP; j++) {
					if ((MumCapBitmap[i] & BIT(j)) != 0)
						MTWF_PRINT("\t%d", 1);
					else
						MTWF_PRINT("\t%d", 0);
					MTWF_PRINT(" \t");
				}
				MTWF_PRINT("\n");
			}
		}
	}

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, arB0PfidGrpBitmap);
	addr_ = base + offset;

	MTWF_PRINT("\n");

	if (pAd->CommonCfg.dbdc_mode != 0)
		counter = MUM_GRP_USR_NUM_2;

	for (k = 0; k < MURU_MUM_MAX_PFID_NUM; k++) {
		addr_2 = addr_ + sizeof(UINT_8) * k * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
		if (WlanIdx[k] == 0)
			continue;

		MTWF_PRINT("\n");
		MTWF_PRINT("\tMURU MUM PFID IDX : %x\n", k);

		for (l = 0; l < MU_MAX_GRP_USR_CAP; l++) {
			if ((MumCapBitmap[k] & BIT(l)) != 0) {
				addr = addr_2 + sizeof(UINT_8) * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
				MTWF_PRINT("\t%s:\n", group_cap[l]);

				show_candidate_list(pAd, counter, addr);

				MTWF_PRINT("\n");
			}
		}
		MTWF_PRINT("\n");
	}

	if (pAd->CommonCfg.dbdc_mode == 1) {
		offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, arB1PfidGrpBitmap);
		addr_ = base + offset;

		for (k = 0; k < MURU_MUM_MAX_PFID_NUM; k++) {
			addr_2 = addr_ + sizeof(UINT_8) * k * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
			if (WlanIdx[k] == 0)
				continue;

			MTWF_PRINT("\n");
			MTWF_PRINT("\tMURU MUM PFID IDX : %x\n", k);

			for (l = 0; l < MU_MAX_GRP_USR_CAP; l++) {
				if ((MumCapBitmap[k] & BIT(l)) != 0) {
					addr = addr_2 + sizeof(UINT_8) * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
					MTWF_PRINT("\t%s:\n", group_cap[l]);

					show_candidate_list(pAd, counter, addr);

					MTWF_PRINT("\n");
				}
			}
			MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n\n");
	}

	if (err_mu_tx)
		return;

	subbase = base_mu_tx;

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4SuccessCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++) {
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			addr = addr1 + i * sizeof(UINT_32);
			u4SuccessCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4TotalCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++) {
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			addr = addr1 + i * sizeof(UINT_32);
			u4TotalCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	for (i = 0; i < MURU_MAX_GROUP_CN; i++)	{
		MTWF_PRINT("\n\n");
		MTWF_PRINT("CN%u DL TX SUCCESS CNT :\n", i+2);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			MTWF_PRINT("\t %u", u4SuccessCount[i][j]);
		}

		MTWF_PRINT("\n");

		MTWF_PRINT("\nCN%u DL TX TOTAL CNT :\n", i+2);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			MTWF_PRINT("\t %u", u4TotalCount[i][j]);
		}

		MTWF_PRINT("\n");

		MTWF_PRINT("\nCN%u DL TX PER :\n", i+2);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			if (u4TotalCount[i][j] == 0)
				MTWF_PRINT("\t %u", u4TotalCount[i][j]);
			else
				MTWF_PRINT("\t %u",
					((u4TotalCount[i][j] - u4SuccessCount[i][j]) * 100) / u4TotalCount[i][j]);
		}
	}

	subbase = base_mu_tx + sizeof(MU_TX_STAT_INFO_T);

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4SuccessCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++) {
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			addr = addr1 + i * sizeof(UINT_32);
			u4SuccessCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4TotalCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++)	{
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++)	{
			addr = addr1 + i * sizeof(UINT_32);
			u4TotalCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	MTWF_PRINT("\n");

	for (i = 0; i < MURU_MAX_GROUP_CN; i++)	{
		MTWF_PRINT("\n\n");

		MTWF_PRINT("CN%u UL TX SUCCESS CNT :\n", i+2);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			MTWF_PRINT("\t %u", u4SuccessCount[i][j]);
		}

		MTWF_PRINT("\n");

		MTWF_PRINT("\nCN%u UL TX TOTAL CNT :\n", i+2);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
			MTWF_PRINT("\t %u", u4TotalCount[i][j]);
		}

		MTWF_PRINT("\n");

		MTWF_PRINT("\nCN%u UL TX PER :\n", i+2);

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
			if (u4TotalCount[i][j] == 0)
				MTWF_PRINT("\t %u", u4TotalCount[i][j]);
			else
				MTWF_PRINT("\t %u",
					((u4TotalCount[i][j] - u4SuccessCount[i][j]) * 100) / u4TotalCount[i][j]);
		}
	}

	MTWF_PRINT("\n\n");
}

static VOID show_mumimo_groupidcli(struct _RTMP_ADAPTER *pAd)
{
	UINT_32 base, base_grptbl, subbase_grptbl, offset, base1;
	UINT_8 u1NumUsr, u1Gid = 0, i;
	uint8_t err, err_grp_tbl, err1;
	UINT_32 addr, addr1, addr2;
	UINT_16 WlanIdx[8], u2HwGrpIdx;
	UINT_32 DW0_Value = 0, DW1_Value = 0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW0 pDW0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW1 pDW1;

	base = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.u4Addr;
	err  = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.fgError;
	base_grptbl = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.u4Addr;
	err_grp_tbl = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.fgError;
	base1 = pAd->CommonCfg.rGloInfo.rCn4GidLookupTable.u4Addr;
	err1  = pAd->CommonCfg.rGloInfo.rCn4GidLookupTable.fgError;

	SyncMuruSramCheckAddr(pAd, base);
	SyncMuruSramCheckAddr(pAd, base_grptbl);
	SyncMuruSramCheckAddr(pAd, base1);

	if (err || err_grp_tbl || err1)
		return;
	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, au2MuProfileIdxToWlanIdx);
	addr = base + offset;

	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		addr = addr + sizeof(UINT_16) * i;
		WlanIdx[i] = muru_io_r_u16(pAd, addr);
		if (WlanIdx[i] > STA_REC_NUM)
			WlanIdx[i] = 0;
	}

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, u2LatestMuTxGrpIdx);
	addr = base + offset;
	u2HwGrpIdx = muru_io_r_u16(pAd, addr);

	subbase_grptbl = base_grptbl + sizeof(MURU_MUM_GROUP_TBL_ENTRY_T) * u2HwGrpIdx;
	SyncMuruSramCheckAddr(pAd, subbase_grptbl);

	MTWF_PRINT("|-(0x%08X)MURU MU ALGORITHM MONITOR:\n", base);

	addr = subbase_grptbl;
	DW0_Value = muru_io_r_u32(pAd, addr);
	addr1 = subbase_grptbl + 4;
	DW1_Value = muru_io_r_u32(pAd, addr1);

	pDW0 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW0)&DW0_Value;
	pDW1 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW1)&DW1_Value;

	u1NumUsr = pDW0->rField.u1NumUser;

	if (u1NumUsr != 3)
		u1Gid = pDW0->rField.u1PFIDUser1;
	else {
		for (i = 0; i < MUM_VHT_4MU_GRP_NUM; i++) {
			addr2  = base1 + sizeof(UINT_8) * i * 6;

			if (muru_io_r_u8(pAd, addr2) == pDW0->rField.u1PFIDUser0 &&
				muru_io_r_u8(pAd, (addr2 + 1)) == pDW0->rField.u1PFIDUser1 &&
				muru_io_r_u8(pAd, (addr2 + 2)) == pDW0->rField.u1PFIDUser2 &&
				muru_io_r_u8(pAd, (addr2 + 3)) == pDW0->rField.u1PFIDUser3) {
					u1Gid = muru_io_r_u8(pAd, (addr2 + 4));
			    }
		}
	}

	MTWF_PRINT("\n GroupID = %d", u1Gid);

	MTWF_PRINT("| |-u1PFIDUser0  = %u WlanIdx = %u Mustream = %u\n",
		pDW0->rField.u1PFIDUser0, WlanIdx[pDW0->rField.u1PFIDUser0],
		pDW1->rField.u1NssUser0 + 1);
	if (u1NumUsr >= 1)
		MTWF_PRINT("| |-u1PFIDUser1  = %u WlanIdx = %u Mustream = %u\n",
			pDW0->rField.u1PFIDUser1, WlanIdx[pDW0->rField.u1PFIDUser1],
			pDW1->rField.u1NssUser1 + 1);
	if (u1NumUsr >= 2)
		MTWF_PRINT("| |-u1PFIDUser2  = %u WlanIdx = %u Mustream = %u\n",
			pDW0->rField.u1PFIDUser2, WlanIdx[pDW0->rField.u1PFIDUser2],
			pDW1->rField.u1NssUser2 + 1);
	if (u1NumUsr == 3)
		MTWF_PRINT("| |-u1PFIDUser3  = %u WlanIdx = %u Mustream = %u\n",
			pDW0->rField.u1PFIDUser3, WlanIdx[pDW0->rField.u1PFIDUser3],
			pDW1->rField.u1NssUser3 + 1);
}

static VOID show_mumimo_algorithm_monitor(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;

	pch = strsep(&arg, "");

	if (pch == NULL)
		return;

	switch (*pch) {
	case '1':
			show_mumimo_algorithm_monitor1(pAd, arg);
			break;

	case '4':
			show_mumimo_groupidcli(pAd);
			break;

	default:
			break;
	}
}

static VOID show_muru_txc_tx_stats(struct _RTMP_ADAPTER *pAd, VOID *pData)
{
	P_EVENT_MURU_TXCMD_TX_STATS pTxCTxStats = (P_EVENT_MURU_TXCMD_TX_STATS)pData;
	P_MURU_TXCMD_DL_TX_STATS pDlTxStats;
	P_MURU_TXCMD_UL_TX_TRIG_STATS pTxTrigUlStats;
	UINT32 u4TotalVhtCount, u4TotalVhtMuCount;
	UINT32 u4TotalHeMuCount, u4TotalHeMuTrigCount;
	UINT32 u4HeDlOfdm2to4RuCount, u4HeDlOfdmGtr5RuCount;
	UINT32 u4TotalHeDlOfdmCount, u4TotalDlHeMuCount;
	UINT32 u4HeUlOfdm2to4RuCount, u4HeUlOfdmGtr5RuCount;
	UINT32 u4TotalHeTrigCount, u4TotalHeUlOfdmCount, u4TotalPpduCount, u4TotalTrigPpduCount;
	uint16_t u2TxModeCck, u2TxModeOfdm, u2TxModeHtMix, u2TxModeHeGf;
	uint16_t u2TxModeVht, u2TxModeHeSu, u2TxModeHeExt, u2TxModeHeMu;
	uint16_t u2StaVht2Mu, u2StaVht3Mu, u2StaVht4Mu, u2StaHe2Mu, u2StaHe3Mu, u2StaHe4Mu;
	uint16_t u2SubModeVhtSuCnt, u2SubModeVhtMuMimoCnt, u2SubModeHeOfdmCnt, u2SubModeHeMumimoCnt;
	uint16_t u2StaOfdm2Ru, u2StaOfdm3Ru, u2StaOfdm4Ru, u2StaOfdm5to8Ru, u2StaOfdm9to16Ru, u2StaOfdmGtr16Ru;
	uint16_t u2TrigOfdmSu, u2TrigOfdm2Ru, u2TrigOfdm3Ru, u2TrigOfdm4Ru, u2TrigOfdm5to8Ru, u2TrigOfdm9to16Ru, u2TrigOfdmGtr16Ru;
	uint16_t u2Trig2MuCnt, u2Trig3MuCnt, u2Trig4MuCnt, u2HeTrigCount, u2HeTrigMuCnt, u2HeTrigOfdmCnt;

	pDlTxStats = (P_MURU_TXCMD_DL_TX_STATS)(&(pTxCTxStats->EventTxDlStats));
	pTxTrigUlStats = (P_MURU_TXCMD_UL_TX_TRIG_STATS)(&(pTxCTxStats->EventTxTrigUlStats));

	u4TotalVhtMuCount = pDlTxStats->u4TxCmdTxModeVht2MuCnt + pDlTxStats->u4TxCmdTxModeVht3MuCnt + pDlTxStats->u4TxCmdTxModeVht4MuCnt;
	u4TotalVhtCount = u4TotalVhtMuCount + pDlTxStats->u4TxCmdTxModeVhtSuCnt;
	u4TotalHeMuCount = pDlTxStats->u4TxCmdTxModeHeMu2MuCnt + pDlTxStats->u4TxCmdTxModeHeMu3MuCnt + pDlTxStats->u4TxCmdTxModeHeMu4MuCnt;
	u4TotalHeMuTrigCount = pTxTrigUlStats->u4TxCmdTxModeHeTrig2MuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig3MuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig4MuCnt;

	u4HeDlOfdm2to4RuCount = pDlTxStats->u4TxCmdTxModeHeMu2RuCnt + pDlTxStats->u4TxCmdTxModeHeMu3RuCnt + pDlTxStats->u4TxCmdTxModeHeMu4RuCnt;
	u4HeDlOfdmGtr5RuCount = pDlTxStats->u4TxCmdTxModeHeMu5to8RuCnt + pDlTxStats->u4TxCmdTxModeHeMu9to16RuCnt + pDlTxStats->u4TxCmdTxModeHeMuGtr16RuCnt;
	u4TotalHeDlOfdmCount = u4HeDlOfdm2to4RuCount + u4HeDlOfdmGtr5RuCount;
	u4TotalDlHeMuCount = u4TotalHeDlOfdmCount + u4TotalHeMuCount;

	u4HeUlOfdm2to4RuCount = pTxTrigUlStats->u4TxCmdTxModeHeTrig2RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig3RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig4RuCnt;
	u4HeUlOfdmGtr5RuCount = pTxTrigUlStats->u4TxCmdTxModeHeTrig5to8RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig9to16RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrigGtr16RuCnt;
	u4TotalHeUlOfdmCount = u4HeUlOfdm2to4RuCount + u4HeUlOfdmGtr5RuCount + pTxTrigUlStats->u4TxCmdTxModeHeTrigSuCnt;
	u4TotalHeTrigCount = u4TotalHeUlOfdmCount + u4TotalHeMuTrigCount;


	u4TotalPpduCount = u4TotalVhtCount + pDlTxStats->u4TxCmdTxModeCckCnt + pDlTxStats->u4TxCmdTxModeOfdmCnt + pDlTxStats->u4TxCmdTxModeHtMmCnt;
	u4TotalPpduCount = u4TotalPpduCount + pDlTxStats->u4TxCmdTxModeHtGfCnt + pDlTxStats->u4TxCmdTxModeHeSuCnt + pDlTxStats->u4TxCmdTxModeHeExtSuCnt;
	u4TotalPpduCount = u4TotalPpduCount + u4TotalDlHeMuCount;

	if (u4TotalPpduCount != 0) {

		u2TxModeCck = ((pDlTxStats->u4TxCmdTxModeCckCnt * 100) / u4TotalPpduCount);
		u2TxModeOfdm = ((pDlTxStats->u4TxCmdTxModeOfdmCnt * 100) / u4TotalPpduCount);
		u2TxModeHtMix = ((pDlTxStats->u4TxCmdTxModeHtMmCnt * 100) / u4TotalPpduCount);
		u2TxModeHeGf = ((pDlTxStats->u4TxCmdTxModeHtGfCnt * 100) / u4TotalPpduCount);
		u2TxModeVht = ((u4TotalVhtCount * 100) / u4TotalPpduCount);
		u2TxModeHeSu = ((pDlTxStats->u4TxCmdTxModeHeSuCnt * 100) / u4TotalPpduCount);
		u2TxModeHeExt = ((pDlTxStats->u4TxCmdTxModeHeExtSuCnt * 100) / u4TotalPpduCount);
		u2TxModeHeMu = ((u4TotalDlHeMuCount * 100) / u4TotalPpduCount);

		u2SubModeVhtSuCnt = ((pDlTxStats->u4TxCmdTxModeVhtSuCnt * 100) / u4TotalPpduCount);
		u2SubModeVhtMuMimoCnt = ((u4TotalVhtMuCount * 100) / u4TotalPpduCount);
		u2SubModeHeOfdmCnt = ((u4TotalHeDlOfdmCount * 100) / u4TotalPpduCount);
		u2SubModeHeMumimoCnt = ((u4TotalHeMuCount * 100) / u4TotalPpduCount);

		u2StaVht2Mu = ((pDlTxStats->u4TxCmdTxModeVht2MuCnt * 100) / u4TotalPpduCount);
		u2StaVht3Mu = ((pDlTxStats->u4TxCmdTxModeVht3MuCnt * 100) / u4TotalPpduCount);
		u2StaVht4Mu = ((pDlTxStats->u4TxCmdTxModeVht4MuCnt * 100) / u4TotalPpduCount);

		u2StaHe2Mu = ((pDlTxStats->u4TxCmdTxModeHeMu2MuCnt * 100) / u4TotalPpduCount);
		u2StaHe3Mu = ((pDlTxStats->u4TxCmdTxModeHeMu3MuCnt * 100) / u4TotalPpduCount);
		u2StaHe4Mu = ((pDlTxStats->u4TxCmdTxModeHeMu4MuCnt * 100) / u4TotalPpduCount);

		u2StaOfdm2Ru = ((pDlTxStats->u4TxCmdTxModeHeMu2RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm3Ru = ((pDlTxStats->u4TxCmdTxModeHeMu3RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm4Ru = ((pDlTxStats->u4TxCmdTxModeHeMu4RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm5to8Ru = ((pDlTxStats->u4TxCmdTxModeHeMu5to8RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm9to16Ru = ((pDlTxStats->u4TxCmdTxModeHeMu9to16RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdmGtr16Ru = ((pDlTxStats->u4TxCmdTxModeHeMuGtr16RuCnt * 100) / u4TotalPpduCount);

	}

	else {
		u2TxModeCck = 0;
		u2TxModeOfdm = 0;
		u2TxModeHtMix = 0;
		u2TxModeHeGf = 0;
		u2TxModeVht = 0;
		u2TxModeHeSu = 0;
		u2TxModeHeExt = 0;
		u2TxModeHeMu = 0;

		u2SubModeVhtSuCnt = 0;
		u2SubModeVhtMuMimoCnt = 0;
		u2SubModeHeOfdmCnt = 0;
		u2SubModeHeMumimoCnt = 0;

		u2StaVht2Mu = 0;
		u2StaVht3Mu = 0;
		u2StaVht4Mu = 0;

		u2StaHe2Mu = 0;
		u2StaHe3Mu = 0;
		u2StaHe4Mu = 0;

		u2StaOfdm2Ru = 0;
		u2StaOfdm3Ru = 0;
		u2StaOfdm4Ru = 0;
		u2StaOfdm5to8Ru = 0;
		u2StaOfdm9to16Ru = 0;
		u2StaOfdmGtr16Ru = 0;
	}

	u4TotalTrigPpduCount = u4TotalHeTrigCount;

	if (u4TotalTrigPpduCount != 0) {
		u2TrigOfdmSu = ((pTxTrigUlStats->u4TxCmdTxModeHeTrigSuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm2Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig2RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm3Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig3RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm4Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig4RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm5to8Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig5to8RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm9to16Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig9to16RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdmGtr16Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrigGtr16RuCnt * 100) / u4TotalTrigPpduCount);

		u2Trig2MuCnt = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig2MuCnt * 100) / u4TotalTrigPpduCount);
		u2Trig3MuCnt = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig3MuCnt * 100) / u4TotalTrigPpduCount);
		u2Trig4MuCnt = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig4MuCnt * 100) / u4TotalTrigPpduCount);

		u2HeTrigCount = ((u4TotalHeTrigCount * 100) / u4TotalTrigPpduCount);
		u2HeTrigOfdmCnt = ((u4TotalHeUlOfdmCount * 100) / u4TotalTrigPpduCount);
		u2HeTrigMuCnt = ((u4TotalHeMuTrigCount * 100) / u4TotalTrigPpduCount);
	}

	else {
		u2TrigOfdmSu = 0;
		u2TrigOfdm2Ru = 0;
		u2TrigOfdm3Ru = 0;
		u2TrigOfdm4Ru = 0;
		u2TrigOfdm5to8Ru = 0;
		u2TrigOfdm9to16Ru = 0;
		u2TrigOfdmGtr16Ru = 0;

		u2Trig2MuCnt = 0;
		u2Trig3MuCnt = 0;
		u2Trig4MuCnt = 0;

		u2HeTrigCount = 0;
		u2HeTrigOfdmCnt = 0;
		u2HeTrigMuCnt = 0;
	}

	MTWF_PRINT("Downlink:\ttotal\t\ttx_mode\t\tsub_mode\tstacnt\n");
	MTWF_PRINT("         \tcount\t\tratio  \t\tratio   \tratio\n");
	MTWF_PRINT("=====================================================================\n");

	MTWF_PRINT("CCK:        \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeCckCnt, u2TxModeCck);
	MTWF_PRINT("OFDM:       \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeOfdmCnt, u2TxModeOfdm);
	MTWF_PRINT("HT_MIX:     \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHtMmCnt, u2TxModeHtMix);
	MTWF_PRINT("HT_GF:      \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHtGfCnt, u2TxModeHeGf);
	MTWF_PRINT("VHT:        \t%u   \t\t%u%%\n", u4TotalVhtCount, u2TxModeVht);
	MTWF_PRINT("  SU:       \t%u   \t         \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeVhtSuCnt, u2SubModeVhtSuCnt);
	MTWF_PRINT("  MUMIMO:   \t%u   \t         \t\t%u%%\n", u4TotalVhtMuCount, u2SubModeVhtMuMimoCnt);
	MTWF_PRINT("	   2MU:   \t%u \t         \t\t        \t%u%%\n", pDlTxStats->u4TxCmdTxModeVht2MuCnt, u2StaVht2Mu);
	MTWF_PRINT("	   3MU:      \t%u\t        \t\t        \t%u%%\n", pDlTxStats->u4TxCmdTxModeVht3MuCnt, u2StaVht3Mu);
	MTWF_PRINT("	   4MU:        \t%u        \t\t        \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeVht4MuCnt, u2StaVht4Mu);
	MTWF_PRINT("HE_SU:      \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeSuCnt, u2TxModeHeSu);
	MTWF_PRINT("HE_EXT:     \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeExtSuCnt, u2TxModeHeExt);
	MTWF_PRINT("HE_MU:      \t%u   \t\t%u%%\n", u4TotalDlHeMuCount, u2TxModeHeMu);
	MTWF_PRINT("  OFDMA:    \t%u   \t         \t\t%u%%\n", u4TotalHeDlOfdmCount, u2SubModeHeOfdmCnt);
	MTWF_PRINT("      2RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu2RuCnt, u2StaOfdm2Ru);
	MTWF_PRINT("      3RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu3RuCnt, u2StaOfdm3Ru);
	MTWF_PRINT("      4RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu4RuCnt, u2StaOfdm4Ru);
	MTWF_PRINT("    5-8RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu5to8RuCnt, u2StaOfdm5to8Ru);
	MTWF_PRINT("   9-16RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu9to16RuCnt, u2StaOfdm9to16Ru);
	MTWF_PRINT("    >16RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMuGtr16RuCnt, u2StaOfdmGtr16Ru);
	MTWF_PRINT(" MUMIMO:    \t%u   \t         \t\t%u%%\n", u4TotalHeMuCount, u2SubModeHeMumimoCnt);
	MTWF_PRINT("	   2MU:   \t%u \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu2MuCnt, u2StaHe2Mu);
	MTWF_PRINT("	   3MU:   \t%u \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu3MuCnt, u2StaHe3Mu);
	MTWF_PRINT("	   4MU:        \t%u \t     \t\t       \t\t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu4MuCnt, u2StaHe4Mu);

	MTWF_PRINT("\n");


	MTWF_PRINT("Uplink:  \ttotal\t\ttx_mode\t\tsub_mode\tstacnt\n");
	MTWF_PRINT("         \tcount\t\tratio  \t\tratio   \tratio\n");
	MTWF_PRINT("=====================================================================\n");
	MTWF_PRINT("HE_TRIG     \t%u   \t\t%u%%\n", u4TotalHeTrigCount, u2HeTrigCount);
	MTWF_PRINT("  OFDMA:    \t%u   \t         \t\t%u%%\n", u4TotalHeUlOfdmCount, u2HeTrigOfdmCnt);
	MTWF_PRINT("       SU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrigSuCnt, u2TrigOfdmSu);
	MTWF_PRINT("      2RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig2RuCnt, u2TrigOfdm2Ru);
	MTWF_PRINT("      3RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig3RuCnt, u2TrigOfdm3Ru);
	MTWF_PRINT("      4RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig4RuCnt, u2TrigOfdm4Ru);
	MTWF_PRINT("    5-8RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig5to8RuCnt, u2TrigOfdm5to8Ru);
	MTWF_PRINT("   9-16RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig9to16RuCnt, u2TrigOfdm9to16Ru);
	MTWF_PRINT("    >16RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrigGtr16RuCnt, u2TrigOfdmGtr16Ru);
	MTWF_PRINT(" MUMIMO:    \t%u   \t         \t\t%u%%\n", u4TotalHeMuTrigCount, u2HeTrigMuCnt);
	MTWF_PRINT("	   2MU:   \t%u \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig2MuCnt, u2Trig2MuCnt);
	MTWF_PRINT("	   3MU:      \t%u \t       \t\t        \t\t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig3MuCnt, u2Trig3MuCnt);
	MTWF_PRINT("	   4MU:        \t%u \t     \t\t        \t\t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig4MuCnt, u2Trig4MuCnt);
}

static VOID check_muru_glo(struct _RTMP_ADAPTER *pAd, VOID *pData)
{
	P_DRV_MURU_GLO pDrvGlo = &pAd->CommonCfg.rGloInfo;
	P_EVENT_MURU_GLO pFwGlo = (P_EVENT_MURU_GLO)pData;
	UINT_32 DriverSize = 0;

	pDrvGlo->rLocalData.u4Addr = pFwGlo->rLocalData.u4Addr;
	pDrvGlo->rLocalData.fgError = (pFwGlo->rLocalData.u4Size != sizeof(MURU_LOCAL_DATA_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,  "MURU_LOCAL_DATA_T");
	if (pDrvGlo->rLocalData.fgError) {
		DriverSize = sizeof(MURU_LOCAL_DATA_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rLocalDataMuruPara.u4Addr = pFwGlo->rLocalDataMuruPara.u4Addr;
	pDrvGlo->rLocalDataMuruPara.fgError = (pFwGlo->rLocalDataMuruPara.u4Size != sizeof(MURU_PARA_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,  "|-MURU_PARA_T");
	if (pDrvGlo->rLocalDataMuruPara.fgError) {
		DriverSize = sizeof(MURU_PARA_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataMuruPara.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rLocalDataQlenInfo.u4Addr = pFwGlo->rLocalDataQlenInfo.u4Addr;
	pDrvGlo->rLocalDataQlenInfo.fgError = (pFwGlo->rLocalDataQlenInfo.u4Size != sizeof(MURU_QLEN_INFO_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_QLEN_INFO_T");
	if (pDrvGlo->rLocalDataQlenInfo.fgError) {
		DriverSize = sizeof(MURU_QLEN_INFO_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataQlenInfo.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,  "\n");

	pDrvGlo->rLocalDataBsrpCtrl.u4Addr = pFwGlo->rLocalDataBsrpCtrl.u4Addr;
	pDrvGlo->rLocalDataBsrpCtrl.fgError = (pFwGlo->rLocalDataBsrpCtrl.u4Size != sizeof(CMD_MURU_BSRP_CTRL))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-CMD_MURU_BSRP_CTRL");
	if (pDrvGlo->rLocalDataBsrpCtrl.fgError) {
		DriverSize = sizeof(CMD_MURU_BSRP_CTRL);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataBsrpCtrl.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rLocalDataTxCmdCtrl.u4Addr = pFwGlo->rLocalDataTxCmdCtrl.u4Addr;
	pDrvGlo->rLocalDataTxCmdCtrl.fgError = (pFwGlo->rLocalDataTxCmdCtrl.u4Size != sizeof(MURU_TXCMD_CTRL_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_TXCMD_CTRL_T");
	if (pDrvGlo->rLocalDataTxCmdCtrl.fgError) {
		DriverSize = sizeof(MURU_TXCMD_CTRL_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m\n");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataTxCmdCtrl.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruTxInfo.u4Addr = pFwGlo->rMuruTxInfo.u4Addr;
	pDrvGlo->rMuruTxInfo.fgError = (pFwGlo->rMuruTxInfo.u4Size != sizeof(MURU_TX_INFO_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "MURU_TX_INFO_T");
	if (pDrvGlo->rMuruTxInfo.fgError) {
		DriverSize = sizeof(MURU_TX_INFO_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfo.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruTxInfoGlobalData.u4Addr = pFwGlo->rMuruTxInfoGlobalData.u4Addr;
	pDrvGlo->rMuruTxInfoGlobalData.fgError = (pFwGlo->rMuruTxInfoGlobalData.u4Size != sizeof(MURU_GLOBAL_INFO_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_GLOBAL_INFO_T");
	if (pDrvGlo->rMuruTxInfoGlobalData.fgError) {
		DriverSize = sizeof(MURU_GLOBAL_INFO_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoGlobalData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruTxInfoProtectData.u4Addr = pFwGlo->rMuruTxInfoProtectData.u4Addr;
	pDrvGlo->rMuruTxInfoProtectData.fgError = (pFwGlo->rMuruTxInfoProtectData.u4Size != sizeof(MURU_PROTECT_INFO_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_PROTECT_INFO_T");
	if (pDrvGlo->rMuruTxInfoProtectData.fgError) {
		DriverSize = sizeof(MURU_PROTECT_INFO_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoProtectData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruTxInfoSxnTxData.u4Addr = pFwGlo->rMuruTxInfoSxnTxData.u4Addr;
	pDrvGlo->rMuruTxInfoSxnTxData.fgError = (pFwGlo->rMuruTxInfoSxnTxData.u4Size != sizeof(MURU_TX_DATA_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_TX_DATA_T");
	if (pDrvGlo->rMuruTxInfoSxnTxData.fgError) {
		DriverSize = sizeof(MURU_TX_DATA_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoSxnTxData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruTxInfoSxnTrigData.u4Addr = pFwGlo->rMuruTxInfoSxnTrigData.u4Addr;
	pDrvGlo->rMuruTxInfoSxnTrigData.fgError = (pFwGlo->rMuruTxInfoSxnTrigData.u4Size != sizeof(MURU_TX_TRIG_DATA_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_TX_TRIG_DATA_T");
	if (pDrvGlo->rMuruTxInfoSxnTrigData.fgError) {
		DriverSize = sizeof(MURU_TX_TRIG_DATA_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoSxnTrigData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rShareData.u4Addr = pFwGlo->rShareData.u4Addr;
	pDrvGlo->rShareData.fgError = (pFwGlo->rShareData.u4Size != sizeof(MURU_SHARE_DATA_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "MURU_SHARE_DATA_T");
	if (pDrvGlo->rShareData.fgError) {
		DriverSize = sizeof(MURU_SHARE_DATA_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rShareDataRuAllocData.u4Addr = pFwGlo->rShareDataRuAllocData.u4Addr;
	pDrvGlo->rShareDataRuAllocData.fgError = (pFwGlo->rShareDataRuAllocData.u4Size != sizeof(MURU_ALLOC_DATA_INFO_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_ALLOC_DATA_INFO_T");
	if (pDrvGlo->rShareDataRuAllocData.fgError) {
		DriverSize = sizeof(MURU_ALLOC_DATA_INFO_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareDataRuAllocData.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rShareDataUserInfo.u4Addr = pFwGlo->rShareDataUserInfo.u4Addr;
	pDrvGlo->rShareDataUserInfo.fgError = (pFwGlo->rShareDataUserInfo.u4Size != sizeof(PER_USER_INFO))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-PER_USER_INFO");
	if (pDrvGlo->rShareDataUserInfo.fgError) {
		DriverSize = sizeof(PER_USER_INFO);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareDataUserInfo.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rShareDataStaRuRecord.u4Addr = pFwGlo->rShareDataStaRuRecord.u4Addr;
	pDrvGlo->rShareDataStaRuRecord.fgError = (pFwGlo->rShareDataStaRuRecord.u4Size != sizeof(STA_MURU_RECORD_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-STA_MURU_RECORD_T");
	if (pDrvGlo->rShareDataStaRuRecord.fgError) {
		DriverSize = sizeof(STA_MURU_RECORD_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareDataStaRuRecord.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "MURU_MAN_CFG_DATA\n");
	pDrvGlo->rMuruCmdManCfgInf.u4Addr = pFwGlo->rMuruCmdManCfgInf.u4Addr;
#ifdef WIFI_UNIFIED_COMMAND
	pDrvGlo->rMuruCmdManCfgInf.fgError = (pFwGlo->rMuruCmdManCfgInf.u4Size != sizeof(struct UNI_MURU_MANUAL_CONFIG_T))?1:0;
#else
	pDrvGlo->rMuruCmdManCfgInf.fgError = (pFwGlo->rMuruCmdManCfgInf.u4Size != sizeof(CMD_MURU_MANCFG_INTERFACER))?1:0;
#endif /* WIFI_UNIFIED_COMMAND */
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-CMD_MURU_MANCFG_INTERFACER");
	if (pDrvGlo->rMuruCmdManCfgInf.fgError) {
#ifdef WIFI_UNIFIED_COMMAND
		DriverSize = sizeof(struct UNI_MURU_MANUAL_CONFIG_T);
#else
		DriverSize = sizeof(CMD_MURU_MANCFG_INTERFACER);
#endif /* WIFI_UNIFIED_COMMAND */
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruCmdManCfgInf.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuTxPktCnt.u4Addr = pFwGlo->rMuTxPktCnt.u4Addr;
	pDrvGlo->rMuTxPktCnt.fgError = (pFwGlo->rMuTxPktCnt.u4Size != sizeof(UINT_32))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-rMuTxPktCnt");
	if (pDrvGlo->rMuTxPktCnt.fgError) {
		DriverSize = sizeof(UINT_32);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuTxPktCnt.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuTxPktCntDwn.u4Addr = pFwGlo->rMuTxPktCntDwn.u4Addr;
	pDrvGlo->rMuTxPktCntDwn.fgError = (pFwGlo->rMuTxPktCntDwn.u4Size != sizeof(UINT_32))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-rMuTxPktCntDwn");
	if (pDrvGlo->rMuTxPktCntDwn.fgError) {
		DriverSize = sizeof(UINT_32);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuTxPktCntDwn.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rAggPolicy.u4Addr = pFwGlo->rAggPolicy.u4Addr;
	pDrvGlo->rAggPolicy.fgError = (pFwGlo->rAggPolicy.u4Size != sizeof(UINT_8))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-rAggPolicy");
	if (pDrvGlo->rAggPolicy.fgError) {
		DriverSize = sizeof(UINT_8);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rAggPolicy.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rDurationComp.u4Addr = pFwGlo->rDurationComp.u4Addr;
	pDrvGlo->rDurationComp.fgError = (pFwGlo->rDurationComp.u4Size != sizeof(UINT_8))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-rDurationComp");
	if (pDrvGlo->rDurationComp.fgError) {
		DriverSize = sizeof(UINT_8);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rDurationComp.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "MURU_MUMIMO_DATA\n");
	pDrvGlo->rMuruMumGrpTable.u4Addr = pFwGlo->rMuruMumGrpTable.u4Addr;
	pDrvGlo->rMuruMumGrpTable.fgError = (pFwGlo->rMuruMumGrpTable.u4Size != sizeof(MURU_MUM_GROUP_TBL_ENTRY_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_MUM_GROUP_TBL_ENTRY");
	if (pDrvGlo->rMuruMumGrpTable.fgError) {
		DriverSize = sizeof(MURU_MUM_GROUP_TBL_ENTRY_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruMumGrpTable.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruMumCtrl.u4Addr = pFwGlo->rMuruMumCtrl.u4Addr;
	pDrvGlo->rMuruMumCtrl.fgError = (pFwGlo->rMuruMumCtrl.u4Size != sizeof(MURU_MUM_CTRL_PARA_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_MU_ALGORITHM_MONITOR");
	if (pDrvGlo->rMuruMumCtrl.fgError) {
		DriverSize = sizeof(MURU_MUM_CTRL_PARA_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruMumCtrl.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruStaCapInfo.u4Addr = pFwGlo->rMuruStaCapInfo.u4Addr;
	pDrvGlo->rMuruStaCapInfo.fgError = (pFwGlo->rMuruStaCapInfo.u4Size != sizeof(MURU_PURE_STACAP_INFO))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_STACAP_INFO");
	if (pDrvGlo->rMuruStaCapInfo.fgError) {
		DriverSize = sizeof(MURU_PURE_STACAP_INFO);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruStaCapInfo.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rMuruTxStatInfo.u4Addr = pFwGlo->rMuruTxStatInfo.u4Addr;
	pDrvGlo->rMuruTxStatInfo.fgError = (pFwGlo->rMuruTxStatInfo.u4Size != sizeof(MU_TX_STAT_INFO_LINK_T))?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_TXSTAT_INFO");
	if (pDrvGlo->rMuruTxStatInfo.fgError) {
		DriverSize = sizeof(MU_TX_STAT_INFO_LINK_T);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxStatInfo.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");

	pDrvGlo->rCn4GidLookupTable.u4Addr = pFwGlo->rCn4GidLookupTable.u4Addr;
	DriverSize = (MUM_VHT_4MU_GRP_NUM * 6);
	pDrvGlo->rCn4GidLookupTable.fgError = (pFwGlo->rCn4GidLookupTable.u4Size != DriverSize)?1:0;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "|-MURU_TXSTAT_INFO");
	if (pDrvGlo->rCn4GidLookupTable.fgError) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"\x1b[31m is not synced.\x1b[0m");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rCn4GidLookupTable.u4Size);
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "\n");
}

static INT32 set_mumimo_fixed_rate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdMuruParameterSet(pAd, arg, UNI_CMD_MURU_FIXED_RATE_CTRL);
#else
	Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_MU_MIMO_CTRL;
	uint16_t subcmd = MU_MIMO_SET_FIXED_RATE;
	uint16_t value = 0;

	if (arg != NULL)
		value = os_str_tol(arg, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(subcmd) + sizeof(value));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef CFG_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	subcmd = cpu2le16(subcmd);
	value = cpu2le16(value);

#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&subcmd, sizeof(subcmd));
	AndesAppendCmdMsg(msg, (char *)&value, sizeof(value));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "(Ret = %d_\n", Ret);
#endif /* WIFI_UNIFIED_COMMAND */

	return Ret;
}

static INT32 set_mumimo_fixed_group_rate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdMuruParameterSet(pAd, arg, UNI_CMD_MURU_FIXED_GROUP_RATE_CTRL);
#else
	Ret = TRUE;
	PCHAR pch = NULL;
	UINT_8 ucNNS_MCS = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_MU_MIMO_CTRL;
	UINT32 subcmd = MU_MIMO_SET_FIXED_GROUP_RATE;
	CMD_MURU_MUM_SET_GROUP_TBL_ENTRY param = {0};

	pch = strsep(&arg, "-");
	/*Get NumUsr*/
	if (pch != NULL)
		param.u1NumUser = os_str_tol(pch, 0, 10) - 1;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get Rualloc*/
	if (pch != NULL)
		param.u1RuAlloc = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get GuardInterval*/
	if (pch != NULL)
		param.u1GI = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get Capability*/
	if (pch != NULL)
		param.u1Capability = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
	/*Get DL /UL*/
	pch = strsep(&arg, "-");

	if (pch != NULL)
		param.u1Dl_Ul = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	if (param.u1NumUser == 0) {
		pch = strsep(&arg, "-");

		if (pch != NULL)
			param.u2WlidUser0 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if (param.u1Dl_Ul == 2) {
			UINT_8 ucNNS_MCS = 0;

			pch = strsep(&arg, "");
			if (pch == NULL) {
				Ret = 0;
				goto error;
			}
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		}
	}

	if (param.u1NumUser >= 1) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			param.u2WlidUser0 = os_str_tol(pch, 0, 10);
#ifdef CFG_BIG_ENDIAN
			param.u2WlidUser0 = cpu2le16(param.u2WlidUser0);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			pch = strsep(&arg, "-");
			} else if (pch == NULL) {
				Ret = 0;
				goto error;
			}

		if (pch != NULL) {
			param.u2WlidUser1 = os_str_tol(pch, 0, 10);
#ifdef CFG_BIG_ENDIAN
			param.u2WlidUser1 = cpu2le16(param.u2WlidUser1);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 1) && (param.u1Dl_Ul != 2))
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns1 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser1 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser1 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 1) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "");
		else if ((param.u1NumUser != 1) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "-");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns1 = (ucNNS_MCS > 11);
			param.u1UlMcsUser1 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			} else if (pch == NULL) {
				Ret = 0;
				goto error;
			}
	}

	if (param.u1NumUser >= 2) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			param.u2WlidUser2 = os_str_tol(pch, 0, 10);
#ifdef CFG_BIG_ENDIAN
			param.u2WlidUser2 = cpu2le16(param.u2WlidUser2);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 2) && (param.u1Dl_Ul != 2))
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns2 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser2 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser2 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 2) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "");
		else if ((param.u1NumUser != 2) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "-");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns2 = (ucNNS_MCS > 11);
			param.u1UlMcsUser2 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else if (pch == NULL) {
				Ret = 0;
				goto error;
		}
	}

	if (param.u1NumUser >= 3) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			param.u2WlidUser3 = os_str_tol(pch, 0, 10);
#ifdef CFG_BIG_ENDIAN
			param.u2WlidUser3 = cpu2le16(param.u2WlidUser3);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		if (param.u1Dl_Ul != 2)
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns3 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser3 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser3 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if (param.u1Dl_Ul == 2)
			pch = strsep(&arg, "");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns3 = (ucNNS_MCS > 11);
			param.u1UlMcsUser3 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else if (pch == NULL) {
				Ret = 0;
				goto error;
		}
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(subcmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef CFG_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	subcmd = cpu2le32(subcmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&subcmd, sizeof(subcmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "%s:(Ret = %d_\n", __func__, Ret);
#endif /* WIFI_UNIFIED_COMMAND */
	return Ret;
}


static INT32 set_mumimo_force_mu_enable(RTMP_ADAPTER *pAd, uint8_t fgForceMu)
{
	INT32 Ret = TRUE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support)
		Ret = UniCmdMuruParameterSet(pAd, (RTMP_STRING *)&fgForceMu, UNI_CMD_MURU_SET_FORCE_MU);
#else
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_MU_MIMO_CTRL;
	uint16_t subcmd = MU_MIMO_SET_FORCE_MU;

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(subcmd) + sizeof(fgForceMu));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef CFG_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&subcmd, sizeof(subcmd));
	AndesAppendCmdMsg(msg, (char *)&fgForceMu, sizeof(fgForceMu));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);
#endif /* WIFI_UNIFIED_COMMAND */

	return Ret;
}
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
static VOID show_scs_info(struct _RTMP_ADAPTER *pAd)
{
	UINT_32 base;
	UINT_32 offset;
	UINT_32 addr, addr_r;
	UINT_8 i, j;
	BOOLEAN err;

	for (j = 0; j < 2; j++) {
		base = pAd->CommonCfg.rScsGloInfo.rscsband[j].u4Addr;
		err = pAd->CommonCfg.rScsGloInfo.rscsband[j].fgError;

		SyncMuruSramCheckAddr(pAd, base);
		if (err)
			continue;
		/* SCS_Show_Info */
		MTWF_PRINT("********** Band %d  Information *********\n", j);
		MTWF_PRINT("|-rscsband%d (0x%08X)\n", j, base);

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1SCSMinRssi);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) i1SCSMinRssi = %d\n",
                addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4OneSecTxByteCount);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u4OneSecTxByteCount = %d\n",
                addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4OneSecRxByteCount);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u4OneSecRxByteCount = %d\n",
                addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CckPdBlkTh);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2CckPdBlkTh = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2OfdmPdBlkTh);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2OfdmPdBlkTh = %d\n", addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2SCSMinRssiTolerance);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2SCSMinRssiTolerance = %d\n",
                addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CckPdThrMax);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2CckPdThrMax = %d\n",
                addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2OfdmPdThrMax);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2OfdmPdThrMax = %d\n",
                addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CckPdThrMin);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2CckPdThrMin = %d\n",
                addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2OfdmPdThrMin);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2OfdmPdThrMin = %d\n",
                addr, muru_io_r_u16(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2IniAvgTput);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2IniAvgTput\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) u2IniAvgTput[%d] = %d\n",
                    addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastTputDiff);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2LastTputDiff\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) u2LastTputDiff[%d] = %d\n", addr,
                    i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastAvgTput);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2LastAvgTput\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) u2LastAvgTput[%d] = %d\n",
                    addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastMaxTput);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2LastMaxTput\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) u2LastMaxTput[%d] = %d\n",
                    addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastMinTput);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2LastMinTput\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) u2LastMinTput[%d] = %d\n",
                    addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastTputIdx);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2LastTputIdx\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
            MTWF_PRINT("| | |-(0x%08X) u2LastTputIdx[%d] = %d\n",
                    addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, fgLastTputDone);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) fgLastTputDone\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) fgLastTputDone[%d] = %d\n",
                    addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CurAvgTput);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2CurAvgTput\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_PRINT("| | |-(0x%08X) u2CurAvgTput[%d] = %d\n",
                    addr, i, muru_io_r_u16(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CurTputIdx);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u2CurTputIdx\n", addr);

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) u2CurTputIdx[%d] = %d\n",
                    addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1TputPeriodScaleBit);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u1TputPeriodScaleBit\n", addr);
			addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_PRINT("| | |-(0x%08X) u1TputPeriodScaleBit[%d] = %d\n",
                    addr, i, muru_io_r_u8(pAd, addr));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1ChannelBusyTh);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u1ChannelBusyTh = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, fgChBusy);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) fgChBusy = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1MyTxRxTh);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u1MyTxRxTh = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, fgPDreset);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) fgPDreset = %d\n", addr, muru_io_r_u8(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4ChannelBusyTime);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u4ChannelBusyTime = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4MyTxAirtime);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u4MyTxAirtime = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4MyRxAirtime);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u4MyRxAirtime = %d\n", addr, muru_io_r_u32(pAd, addr));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4OBSSAirtime);
		addr = base + offset;
		MTWF_PRINT("| |-(0x%08X) u4OBSSAirtime = %d\n", addr, muru_io_r_u32(pAd, addr));
	}
}

#endif /*SMART_CARRIER_SENSE_SUPPORT*/

#ifdef WF_RESET_SUPPORT
static VOID mt7992_wf_subsys_reset(struct _RTMP_ADAPTER *pAd)
{
	asic_ser_handler(pAd, SER_ACTION_HW_RESET, 0);
}

static VOID mt7992_heart_beat_check(struct _RTMP_ADAPTER *pAd)
{
#define HEART_BEAT_CHECK_PERIOD 20
#define WM_TIMEOUT_COUNT_CHECK 1
#define WM_HANG_COUNT_CHECK 1
	UINT32 cnt = 0, cidx = 0, didx = 0, queue_cnt = 0;
	UINT8 i = 0;

	if (PD_GET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev) > 0) {
		P_FWCMD_TIMEOUT_RECORD pToRec = NULL;

		RTMP_IO_READ32(pAd->hdev_ctrl,
			WF_WFDMA_MEM_DMA_WPDMA_RX_RING1_CTRL0_ADDR + 4, &cnt);
		RTMP_IO_READ32(pAd->hdev_ctrl,
			WF_WFDMA_MEM_DMA_WPDMA_RX_RING1_CTRL0_ADDR + 8, &cidx);
		RTMP_IO_READ32(pAd->hdev_ctrl,
			WF_WFDMA_MEM_DMA_WPDMA_RX_RING1_CTRL0_ADDR + 12, &didx);
		queue_cnt = (didx > cidx) ? (didx - cidx - 1) : (didx - cidx + cnt - 1);

		if ((cidx == 0) && (didx == 0) && (cnt == 0))
			return;

		pToRec = &pAd->physical_dev->FwCmdTimeoutRecord[PD_GET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev)];
		pToRec->cidx = cidx;
		pToRec->didx = didx;

		if (((cnt - 1) == queue_cnt) && (PD_GET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev) > WM_HANG_COUNT_CHECK)) {
			for (i = 1; i < PD_GET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev); i++) {
				pToRec = &pAd->physical_dev->FwCmdTimeoutRecord[PD_GET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev) - i];

				if ((pToRec->cidx != cidx) || (pToRec->didx != didx)) {
					PD_SET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev, 0);
					return;
				}
			}

			if (PD_GET_WF_RESET_IN_PROGRESS(pAd->physical_dev) == FALSE) {
				PD_SET_WF_RESET_WM_COUNT(pAd->physical_dev, PD_GET_WF_RESET_WM_COUNT(pAd->physical_dev) + 1);
				if (PD_GET_WF_RESET_IN_PROGRESS(pAd->physical_dev) == TRUE)
					return;

				PD_SET_WF_RESET_IN_PROGRESS(pAd->physical_dev, TRUE);
				PD_SET_WF_CMD_TIMEOUT_COUNT(pAd->physical_dev, 0);
				RTCMDUp(&pAd->physical_dev->wf_reset_thread);
			}
		}
	}
}

#endif

#ifdef MGMT_TXPWR_CTRL
static VOID update_mgmt_tx_pwr(struct wifi_dev *wdev, MAC_TX_INFO *info, UINT32 *txd)
{
	UCHAR fixed_rate_idx;

	if (info->Type != FC_TYPE_MGMT)
		return;
	fixed_rate_idx = HAL_TX_DESC_GET_FIXED_RATE_IDX(txd);
	switch (fixed_rate_idx) {
	case FR_CCK_1M:
	case FR_CCK_SPE0x18_1M:
		info->txpwr_offset = wdev->mgmt_txd_txpwr_offset_cck[0];
		break;
	case FR_CCK_11M:
		info->txpwr_offset = wdev->mgmt_txd_txpwr_offset_cck[3];
		break;
	case FR_CCK_SPE0x18_2M:
	case FR_CCK_SPE0x18_5_5M:
	case FR_CCK_SPE0x18_11M:
		if (fixed_rate_idx - FR_CCK_SPE0x18_2M + 1 < MODULATION_SYSTEM_CCK_NUM)
			info->txpwr_offset =
				wdev->mgmt_txd_txpwr_offset_cck[fixed_rate_idx - FR_CCK_SPE0x18_2M + 1];
		break;
	case FR_CCKS_SPE0x18_2M:
	case FR_CCKS_SPE0x18_5_5M:
	case FR_CCKS_SPE0x18_11M:
		if (fixed_rate_idx - FR_CCKS_SPE0x18_2M + 1 < MODULATION_SYSTEM_CCK_NUM)
			info->txpwr_offset =
				wdev->mgmt_txd_txpwr_offset_cck[fixed_rate_idx - FR_CCKS_SPE0x18_2M + 1];
		break;
	case FR_OFDM_6M:
	case FR_OFDM_SPE0x18_6M:
	case FR_OFDM_SPE0x18_6M_BCN0:
	case FR_OFDM_SPE0x18_6M_BCN1:
	case FR_OFDM_SPE0x18_6M_BCN2:
		info->txpwr_offset = wdev->mgmt_txd_txpwr_offset_ofdm[0];
		break;
	case FR_OFDM_24M:
		info->txpwr_offset = wdev->mgmt_txd_txpwr_offset_ofdm[4];
		break;
	case FR_OFDM_SPE0x18_9M:
	case FR_OFDM_SPE0x18_12M:
	case FR_OFDM_SPE0x18_18M:
	case FR_OFDM_SPE0x18_24M:
	case FR_OFDM_SPE0x18_36M:
	case FR_OFDM_SPE0x18_48_5M:
	case FR_OFDM_SPE0x18_54M:
		if (fixed_rate_idx - FR_OFDM_SPE0x18_9M + 1 < MODULATION_SYSTEM_OFDM_NUM)
			info->txpwr_offset =
				wdev->mgmt_txd_txpwr_offset_ofdm[fixed_rate_idx - FR_OFDM_SPE0x18_9M + 1];
		break;
	default:
		break;
	}
}
#endif

static VOID mt7992_write_tmac_info_fixed_rate(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *tmac_info,
	MAC_TX_INFO *info,
	union _HTTRANSMIT_SETTING *transmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UCHAR q_idx = info->q_idx;
	STA_TR_ENTRY *tr_entry = NULL;
	UINT32 *txd = (UINT32 *)tmac_info;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _BSS_INFO_ARGUMENT_T *bssinfo;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	NdisZeroMemory(txd, sizeof(struct bmac_txd));

	if (VALID_UCAST_ENTRY_WCID(pAd, info->SW_WCID))
		mac_entry = entry_get(pAd, info->SW_WCID);

	/*  DW0 */

	/* TX Byte Count */
	HAL_TX_DESC_SET_TX_BYTE_COUNT(txd,
		(sizeof(struct bmac_txd) + info->Length));

	/* PKT_FT */
	if (info->IsOffloadPkt == TRUE)
		HAL_TX_DESC_SET_PKT_FT(txd, BMAC_FT_MCU_FW);
	else
		HAL_TX_DESC_SET_PKT_FT(txd, BMAC_FT_HIF_CTD);

	/* Q_IDX */
	if (q_idx < 4)
		HAL_TX_DESC_SET_Q_IDX(txd, (info->wmm_set*4+q_idx));
	else {
		HAL_TX_DESC_SET_Q_IDX(txd, q_idx);
		HAL_TX_DESC_SET_TGID(txd, band_idx);
	}

	/* DW1 */

	/* WLAN Index [9:0] */
	HAL_TX_DESC_SET_MLD_ID(txd, info->WCID);

	/* HEADER_LENGTH */
	HAL_TX_DESC_SET_HEADER_LENGTH(txd, info->hdr_len >> 1);

	/* HF */
	HAL_TX_DESC_SET_HF(txd, BMAC_HF_802_11_FRAME);

	/* Header Padding */
	if (info->hdr_pad)
		HAL_TX_DESC_SET_HEADER_PADDING(txd,
				BMAC_TXD_HDR_PAD_MODE_TAIL);

	/*
	 * TID_MGMT
	 * TID or management frame type
	 * TID whenTXD.type = DATA
	 * TM/ADD_BA/FD merge to
	 * management frame type when TXD.type = MGNT
	 */
	if (info->Type == FC_TYPE_MGMT) {
		if (cap->TmrEnable && info->IsTmr)
			HAL_TX_DESC_SET_TID_MGMT_TYPE(
					txd, BMAC_MGMT_TM);
		else if (info->addba)
			HAL_TX_DESC_SET_TID_MGMT_TYPE(
					txd, BMAC_MGMT_ADD_BA);
	} else if (info->Type == FC_TYPE_DATA)
		HAL_TX_DESC_SET_TID_MGMT_TYPE(txd, info->TID);

	/* OM */
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		tr_entry = tr_entry_get(pAd, info->SW_WCID);
		HAL_TX_DESC_SET_OM(txd, tr_entry->OmacIdx);
	} else if (mac_entry && !IS_ENTRY_NONE(mac_entry)
			&& !IS_ENTRY_MCAST(mac_entry))
		HAL_TX_DESC_SET_OM(txd, mac_entry->wdev->OmacIdx);
	else
		HAL_TX_DESC_SET_OM(txd, info->OmacIdx);

	/* Subtype */
	/* Type */
	if (info->IsOffloadPkt == TRUE) {
		HAL_TX_DESC_SET_SUBTYPE(txd, info->SubType);
		HAL_TX_DESC_SET_FTYPE(txd, info->Type);
	}

	/* BM */
	if (info->BM)
		HAL_TX_DESC_SET_BM(txd, 1);

	/* B */
	if (info->prot == 2) {
#ifdef CONFIG_AP_SUPPORT
		if (mac_entry) {
			struct wifi_dev *temp_wdev = mac_entry->wdev;

			GET_GroupKey_WCID(temp_wdev, info->WCID);
		}
		HAL_TX_DESC_SET_MLD_ID(txd, info->WCID);
#endif
		HAL_TX_DESC_SET_TID_MGMT_TYPE(
					txd, BMAC_MGMT_BIP);
	}

	if (info->sw_duration)
		HAL_TX_DESC_SET_DU(txd, 1);

	/* H/W won't add HTC for mgmt/ctrl frame */
	if (((info->Type == FC_TYPE_MGMT) || ((info->Type == FC_TYPE_CNTL))) &&
		cap->mgmt_ctrl_frm_hw_htc_disable)
		info->htc = TRUE;

	if (info->htc)
		HAL_TX_DESC_SET_HE(txd, 1);

	/* life time */
	/* convert timing unit from 32TU to 64TU */
	HAL_TX_DESC_SET_REMAINING_TX_TIME(txd, info->tx_lifetime);

	/* FR */
	if (!info->IsAutoRate) {
		bssinfo = &wdev->bss_info_argument;

		HAL_TX_DESC_SET_FR(txd, 1);

		if ((pAd->CommonCfg.bSeOff != TRUE)
			&& (band_idx < cap->hw_band_num)) {
			UINT8 rate_idx = FR_TABLE_START_IDX+(band_idx*2);

			HAL_TX_DESC_SET_FIXED_RATE_IDX(txd, rate_idx);
		} else if (info->Type == FC_TYPE_MGMT && info->SubType == SUBTYPE_BEACON
				&& transmit->field.MODE == MODE_HE && transmit->field.MCS == MCS_0) {
			HAL_TX_DESC_SET_FIXED_RATE_IDX(txd, FR_HE_MCS0);
		} else
			HAL_TX_DESC_SET_FIXED_RATE_IDX(txd, bssinfo->FixedRateIdx);

#ifdef RT_CFG80211_SUPPORT
		if (wdev->cfg80211_bcn_rate &&
			info->Type == FC_TYPE_MGMT && info->SubType == SUBTYPE_BEACON) {
			HAL_TX_DESC_CLR_FIXED_RATE_IDX(txd);
			HAL_TX_DESC_SET_FIXED_RATE_IDX(txd, transmit_to_fr_idx(transmit->word));
		}
#endif
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (wdev->eap.eap_mgmrate_en & (1 << BCN_TYPE)) {
			if (info->Type == FC_TYPE_MGMT && info->SubType == SUBTYPE_BEACON) {
				HAL_TX_DESC_CLR_FIXED_RATE_IDX(txd);
				HAL_TX_DESC_SET_FIXED_RATE_IDX(txd, transmit_to_fr_idx(transmit->word));
			}
		}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#if defined(CONFIG_6G_SUPPORT) && defined(MT7992_SKU_BE3600SDB)
		if (wdev && WMODE_CAP_6G(wdev->PhyMode)
#ifdef CONFIG_6G_AFC_SUPPORT
			&& (!is_afc_in_run_state(pAd) || pAd->CommonCfg.AfcSpBwDup)
#endif /*CONFIG_6G_AFC_SUPPORT*/
			)
			HAL_TX_DESC_SET_BW(txd, 0); /*auto bw*/
		else
#endif /* CONFIG_6G_SUPPORT && MT7992_SKU_BE3600SDB */
			HAL_TX_DESC_SET_BW(txd, (bssinfo->FixedRateBW | 0x8));
	}

#ifdef MGMT_TXPWR_CTRL
	update_mgmt_tx_pwr(wdev, info, txd);
#endif

	HAL_TX_DESC_SET_POWER_OFFSET(txd, ((info->txpwr_offset) & 0x3f));

	/* NA */
	/* Remaining TX Count */
	if (info->Ack)
		HAL_TX_DESC_SET_REMAINING_TX_COUNT(txd, MT_TX_LONG_RETRY);
	else {
		HAL_TX_DESC_SET_NA(txd, 1);
		HAL_TX_DESC_SET_REMAINING_TX_COUNT(
			txd, MT_TX_RETRY_UNLIMIT);
	}

	/* PF */
	if (info->prot == 1)
		HAL_TX_DESC_SET_PF(txd, 1);

	/* Power Management */
	if (info->PsmBySw)
		HAL_TX_DESC_SET_PM(txd, 1);

	if (info->NSeq) {
		HAL_TX_DESC_SET_SN_VLD(txd, 1);
		HAL_TX_DESC_SET_SN(txd, (info->assigned_seq & 0xfff));
	}

	if (info->txs2m)
		HAL_TX_DESC_SET_TXS2M(txd, 1);

	if (info->txs2h)
		HAL_TX_DESC_SET_TXS2H(txd, 1);
	HAL_TX_DESC_SET_PID(txd, info->PID);

#ifdef EAP_STATS_SUPPORT
	HAL_TX_DESC_SET_VTA(txd, 1);
#endif

	/* MSDU Count */
	HAL_TX_DESC_SET_MSDU_COUNT(txd, 1);
}

struct bus_info mt7992_bus_info = {
#ifdef RTMP_MAC_PCI
	.bus2chip = mt7992_mac_cr_range,
	.max_static_map_addr = 0x001c0000,
	.l1_remap_addr = HIF_ADDR_L1_REMAP_ADDR,
	.l1_remap_mask = HIF_ADDR_L1_REMAP_MASK,
	.l1_remap_shift = HIF_ADDR_L1_REMAP_SHFT,
	.l1_remap_base_mask = L1_REMAP_BASE_MASK,
	.l1_remap_base_addr = HIF_ADDR_L1_REMAP_BASE_ADDR,
	.l1_remap_offset = L1_REMAP_OFFSET_MASK,
	.l2_remap_addr = HIF_ADDR_L2_REMAP_ADDR,
	.l2_remap_mask = HIF_ADDR_L2_REMAP_MASK,
	.l2_remap_shift = HIF_ADDR_L2_REMAP_SHFT,
	.l2_remap_base_mask = L2_REMAP_BASE_MASK,
	.l2_remap_base_addr = HIF_ADDR_L2_REMAP_BASE_ADDR,
	.l2_remap_offset = L2_REMAP_OFFSET_MASK,
	.io_remap_is_l1_remap = pci_io_remap_is_l1_remap,
#endif /* RTMP_MAC_PCI */
};

static VOID mt7992_chipCap_init(
	struct physical_device *ph_dev, RTMP_CHIP_CAP *chip_cap)
{
	chip_cap->TXWISize = LMAC_TXD_MAX_SIZE;
	chip_cap->RXWISize = 28;
	chip_cap->tx_hw_hdr_len = chip_cap->TXWISize;
	chip_cap->rx_hw_hdr_len = 0x60;
	chip_cap->tx_ring_size = 1024;
	chip_cap->tkn_info.feature = TOKEN_TX;
	chip_cap->tkn_info.token_tx_cnt = 8192;
	chip_cap->tkn_info.band0_token_cnt = 2048;
	chip_cap->tkn_info.low_water_mark = 5;
	chip_cap->tkn_info.hw_tx_token_cnt = 7168;
	chip_cap->tkn_info.token_rx_cnt = 7168;
	chip_cap->multi_token_ques_per_band = FALSE;

#ifdef HW_TX_BYPASS_WA
	chip_cap->asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT
					| fASIC_CAP_CT | fASIC_CAP_HW_DAMSDU
					| fASIC_CAP_WMM_PKTDETECT_OFFLOAD);
#else /* HW_TX_BYPASS_WA */
	chip_cap->asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT
					| fASIC_CAP_CT | fASIC_CAP_HW_DAMSDU
					| fASIC_CAP_MCU_OFFLOAD
					| fASIC_CAP_WMM_PKTDETECT_OFFLOAD);
#endif /* !HW_TX_BYPASS_WA */

#ifdef HDR_TRANS_TX_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_TX_HDR_TRANS;
#endif
#ifdef HDR_TRANS_RX_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_RX_HDR_TRANS;
#endif
#ifdef CONFIG_CSO_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_CSO;
#endif
#ifdef RX_SCATTER
	chip_cap->asic_caps |= fASIC_CAP_RX_DMA_SCATTER;
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_TWT;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef HW_TX_AMSDU_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_HW_TX_AMSDU;
#endif /* HW_TX_AMSDU_SUPPORT */
	chip_cap->asic_caps |= fASIC_CAP_DBDC;
	chip_cap->asic_caps |= fASIC_CAP_SEPARATE_DBDC;
#ifdef WHNAT_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_WHNAT;
#endif

	chip_cap->asic_caps |= fASIC_CAP_DLY_INT_PER_RING;
	chip_cap->asic_caps |= fASIC_CAP_TWO_PCIE;
	chip_cap->asic_caps |= fASIC_CAP_TXCMD;
	chip_cap->asic_caps |= fASIC_CAP_FW_RESTART_POLLING_MODE;
	chip_cap->asic_caps |= fASIC_CAP_ADV_SECURITY;
	chip_cap->asic_caps |= fASIC_CAP_HW_ADMISSION_CTRL;

#ifdef DOT11_HE_AX
	chip_cap->mac_caps |= fMAC_CAP_BA_256;
#endif /* DOT11_HE_AX */

#ifdef DOT11_EHT_BE
	chip_cap->mac_caps |= fMAC_CAP_BA_1024;
#endif /* DOT11_EHT_BE */

	chip_cap->phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | fPHY_CAP_6G |
			fPHY_CAP_HT | fPHY_CAP_VHT | fPHY_CAP_HE | \
			fPHY_CAP_TXBF | fPHY_CAP_LDPC | \
			fPHY_CAP_BW40 | fPHY_CAP_BW80 | fPHY_CAP_BW160C | fPHY_CAP_BW160NC | \
			fPHY_CAP_BW20_242TONE | fPHY_CAP_HE_SR | \
			fPHY_CAP_HE_PPE_EXIST | fPHY_CAP_DUALPHY | fPHY_CAP_BW160C_STD);

	chip_cap->phy_caps |= (fPHY_CAP_DL_MUMIMO | fPHY_CAP_UL_MUMIMO |
		fPHY_CAP_HE_DL_MUOFDMA | fPHY_CAP_HE_UL_MUOFDMA);

	chip_cap->phy_caps |= fPHY_CAP_HE_ER_SU;

#ifdef DOT11_EHT_BE
	chip_cap->phy_caps |=
		(fPHY_CAP_EHT | fPHY_CAP_BW320);
#endif /* DOT11_EHT_BE */

	chip_cap->hw_ops_ver = HWCTRL_OP_TYPE_V2;
	chip_cap->hw_protect_update_ver = HWCTRL_PROT_UPDATE_METHOD_V2;
	chip_cap->hif_type = HIF_MT;
	chip_cap->mac_type = MAC_MT;
	chip_cap->MCUType = ANDES | CR4;
	chip_cap->rf_type = RF_MT;
	chip_cap->MaxNumOfRfId = 127;
	chip_cap->MaxNumOfBbpId = 200;
	chip_cap->ProbeRspTimes = 2;
	chip_cap->FlgIsHwWapiSup = TRUE;
	chip_cap->FlgIsHwAntennaDiversitySup = FALSE;
#ifdef TXBF_SUPPORT
	chip_cap->FlgHwTxBfCap = TXBF_HW_CAP | TXBF_HW_2BF;
	chip_cap->pfmu_num_max = 84;
#endif
	chip_cap->SnrFormula = SNR_FORMULA4;
#ifdef RTMP_EFUSE_SUPPORT
	chip_cap->EFUSE_USAGE_MAP_START = 0x1e0;
	chip_cap->EFUSE_USAGE_MAP_END = 0x1ff;
	chip_cap->EFUSE_USAGE_MAP_SIZE = 30;
	chip_cap->EFUSE_RESERVED_SIZE = 29;
#endif
	chip_cap->efuse_content_start = 0x0;
	chip_cap->efuse_content_end = 0xdff;
	chip_cap->EEPROM_DEFAULT_BIN = mt7992_get_default_bin_image(ph_dev);
	chip_cap->EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7992_E2PImage_iPAiLNA);
	chip_cap->EFUSE_BUFFER_CONTENT_SIZE = sizeof(MT7992_E2PImage_iPAiLNA);
	chip_cap->efuse_free_block_ver = EFUSE_FREE_BLOCK_FORMAT_V2;
	chip_cap->efuse_total_bank_num = 17;
	chip_cap->ddie_lvts_num = 4;
	chip_cap->MBSSIDMode = MBSSID_MODE0;
#ifdef DOT11W_PMF_SUPPORT
	chip_cap->FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */
#ifdef NEED_ROM_PATCH
	chip_cap->need_load_patch = BIT(WM_CPU);
#else
	chip_cap->need_load_patch = 0;
#endif
#ifdef HW_TX_BYPASS_WA
	chip_cap->need_load_fw = BIT(WM_CPU);
#else
	chip_cap->need_load_fw = BIT(WM_CPU) | BIT(WA_CPU);
#endif
	chip_cap->load_patch_flow = PATCH_FLOW_V1;
	chip_cap->load_fw_flow = FW_FLOW_V1;
	chip_cap->patch_format = PATCH_FORMAT_V2;
	chip_cap->fw_format = FW_FORMAT_V3;
	chip_cap->load_patch_method = BIT(BIN_METHOD);
	chip_cap->load_fw_method = BIT(BIN_METHOD);
	chip_cap->rom_patch_offset = MT7992_ROM_PATCH_START_ADDRESS;
	chip_cap->cmd_header_len = sizeof(FW_TXD) + sizeof(struct bmac_txd);
	chip_cap->cmd_padding_len = 0;
#ifdef WIFI_UNIFIED_COMMAND
	chip_cap->uni_cmd_header_len = sizeof(UNI_CMD_HEADER) + sizeof(TMAC_TXD_L);
	chip_cap->u4MaxInBandCmdLen = (2048 - chip_cap->uni_cmd_header_len - chip_cap->cmd_padding_len);
	chip_cap->uni_cmd_support = TRUE;
#endif /* WIFI_UNIFIED_COMMAND */

	/* ppdu_caps */
	chip_cap->ppdu.TxAggLimit = 64;
	chip_cap->ppdu.max_amsdu_len = MPDU_7991_OCTETS;
	chip_cap->ppdu.ht_max_ampdu_len_exp = 3;
#ifdef DOT11_VHT_AC
	chip_cap->ppdu.max_mpdu_len = MPDU_11454_OCTETS;
	chip_cap->ppdu.vht_max_ampdu_len_exp = 7;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	chip_cap->ppdu.he_max_ampdu_len_exp = 3;
	chip_cap->ppdu.max_agg_tid_num = 1;
	chip_cap->ppdu.default_pe_duration = 4;/*unit:4us*/
	chip_cap->ppdu.er_su_dis = 1;/*disable ER_SU*/
	chip_cap->ppdu.trig_mac_pad_dur = PADDING_16US;/*unit:8us*/
#endif
#if defined(CONFIG_6G_SUPPORT) && defined(MT7992_SKU_BE3600SDB)
	chip_cap->ppdu.he6g_start_spacing = 0x2; /*0.5 us*/
	chip_cap->ppdu.he6g_max_ampdu_len_exp = 0x7; /*20*/
	chip_cap->ppdu.he6g_max_mpdu_len = 0x2; /*11454*/
	chip_cap->ppdu.he6g_smps = 3; /*disable*/
#endif
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	chip_cap->fgRateAdaptFWOffload = TRUE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	chip_cap->qos.WmmHwNum = 4;
	chip_cap->PDA_PORT = 0xf800;
	chip_cap->ppdu.tx_amsdu_support = TRUE;
	chip_cap->ppdu.rx_amsdu_in_ampdu_support = TRUE;
	chip_cap->APPSMode = APPS_MODE2;
	chip_cap->CtParseLen = CT_PARSE_PAYLOAD_LEN ;
	chip_cap->qm = FAST_PATH_QM;
#ifdef CONFIG_SOC_MT7621
	chip_cap->rx_qm_en = FALSE;
#endif
	chip_cap->rx_qm = GENERIC_QM;
	chip_cap->qm_tm = TASKLET_METHOD;
	chip_cap->hif_tm = TASKLET_METHOD;

	if (chip_cap->rx_qm == GENERIC_QM &&
		chip_cap->qm_tm == TASKLET_METHOD) {
		chip_cap->RxSwRpsEnable = TRUE;
		chip_cap->RxSwRpsTpThreshold = 600;
		chip_cap->sw_rps_tp_thd_dl = 1500;
		chip_cap->RxSwRpsCpu = 2;
	}

	chip_cap->qos.wmm_detect_method = WMM_DETECT_METHOD1;
	chip_cap->hw_max_amsdu_nums = 31;
	chip_cap->amsdu_txdcmp = 0xFF7FFFFF;
	chip_cap->band_cnt = 3;
	chip_cap->txd_type = TXD_V2;
	chip_cap->OmacNums = 5;
	chip_cap->BssNums = 4;
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	chip_cap->BcnMaxNum = 32; /* for 1.5k bcn, at most 32 limited by fw */
	chip_cap->BcnMaxLength = 1500; /* limited by harrier fw */
#else
	chip_cap->BcnMaxNum = 32;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
	chip_cap->perBandBcnMaxNum = MAX_BEACON_NUM;

	chip_cap->ExtMbssOmacStartIdx = 0x10;
	chip_cap->RepeaterStartIdx = 0x20;
#ifdef AIR_MONITOR
	chip_cap->MaxRepeaterNum = 16 * chip_cap->band_cnt;
#else
	chip_cap->MaxRepeaterNum = 16 * chip_cap->band_cnt;
#endif /* AIR_MONITOR */
	chip_cap->fgIsNeedPretbttIntEvent = FALSE;
	chip_cap->tx_delay_support = TRUE;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	chip_cap->twt_sp_duration_min_num = TWT_MIN_SP_DURATION;
#endif /* WIFI_TWT_SUPPORT */
	chip_cap->mu_edca_timer = 255;
#endif /* DOT11_HE_AX */

	chip_cap->wtbl_max_entries = MT7992_MT_WTBL_SIZE;

	chip_cap->drr_max_dw = 17;
	/* WF_DRR_TOP_AC0_STATION_PAUSE00_ADDR ~  WF_DRR_TOP_AC0_STATION_PAUSE15_ADDR,
	WF_DRR_TOP_AC0_STATION_PAUSE_EXT_00_ADDR */
	chip_cap->drr_max_dw_twt = 17;
	/* WF_DRR_TOP_TWT_STA_MAP00_ADDR ~  WF_DRR_TOP_TWT_STA_MAP15_ADDR,
	WF_DRR_TOP_TWT_STA_MAP_EXT_00_ADDR */
	chip_cap->drr_cmd_query = TRUE;

#ifdef SW_CONNECT_SUPPORT
	/* 1025 - 509 = 516 */
	/* HW Layout : 0, 1, 2, ~ 509 */
	/* SW DUMMY Layout : (DUMMY) 510, 511 */
	/* SW Layout : 512 ~ 1025, 1 extra for 1 other band use */
	chip_cap->sw_ucast_max_entries = 516;
	/* chip_cap->ucast_sw.start = 512; */
	/* chip_cap->ucast_sw.end = 1025; */
	/* use same boundary, and let mt7992_mactbl_uwtbl_adjust() to adjust again */
	chip_cap->hw_sw_max_entries = chip_cap->wtbl_max_entries;
#endif /* SW_CONNECT_SUPPORT */

	chip_cap->wtbl_no_matched = 0xfff; /* MT7992 wcid use 12 bits */
	chip_cap->single_sku_type_parse_num = SINGLE_SKU_TYPE_PARSE_NUM_V1;
	chip_cap->single_sku_para_parse_num = SINGLE_SKU_PARAM_PARSE_NUM_V1;
	chip_cap->single_sku_type_num = SINGLE_SKU_TYPE_NUM_V1;
	chip_cap->single_sku_para_num = SINGLE_SKU_PARAM_NUM_V1;
	chip_cap->backoff_type_parse_num = BACKOFF_TYPE_PARSE_NUM_V1;
	chip_cap->backoff_para_parse_num = BACKOFF_PARAM_PARSE_NUM_V1;
	chip_cap->backoff_type_num = BACKOFF_TYPE_NUM_V1;
	chip_cap->backoff_para_num = BACKOFF_PARAM_NUM_V1;
	chip_cap->single_sku_fill_tbl_cck = SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_ofdm = SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_ht20 = SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_ht40 = SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht20 = SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht40 = SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht80 = SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht160 = SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1;
	chip_cap->single_sku_parse_tbl_htvht40 = SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH_V1;
	chip_cap->backoff_tbl_bfon_ht40 = BACKOFF_TABLE_BF_ON_HT40_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_length = MT7992_SINGLE_SKU_FILL_TABLE_LENGTH;
	chip_cap->txpower_type = TX_POWER_TYPE_V1;
	chip_cap->single_sku_tbl_type_ht40 = SINGLE_SKU_TABLE_HT40;
	chip_cap->backoff_tbl_bf_on_type_ht40 = BACKOFF_TABLE_BF_ON_HT40;
	chip_cap->spe_map_list.spe_map = mt7992_spe_map;
	chip_cap->spe_map_list.size = ARRAY_SIZE(mt7992_spe_map);
	chip_cap->channelbw = BW_20;
	chip_cap->mgmt_ctrl_frm_hw_htc_disable = TRUE;
	chip_cap->peak_txop = TXOP_BB;
#ifdef BCN_PROTECTION_SUPPORT
	chip_cap->bcn_prot_sup = BCN_PROT_EN_HW_MODE;
#endif
#ifdef VLAN_SUPPORT
	chip_cap->vlan_rx_tag_mode = VLAN_RX_TAG_HW_MODE;
#endif
#ifdef PRE_CAL_MT7992_SUPPORT
	/* DW0 : Used for save total pre-cal size
	 * DW1 : reserved
	 * DW2 : reserved
	 * DW3 : reserved
	  */
	chip_cap->prek_ee_info.info_size = 0x10;
	/* Group Calibration item */
	chip_cap->prek_ee_info.cal_result_size = 4 * 1024;
	chip_cap->prek_ee_info.cal_result_size_5g = 45 * 1024;
	chip_cap->prek_ee_info.cal_result_size_6g = 110 * 1024;
	chip_cap->prek_ee_info.cal_result_size_adcdcoc_2g = 4 * 4;
	chip_cap->prek_ee_info.cal_result_size_adcdcoc_5g = 4 * 5;
	chip_cap->prek_ee_info.cal_result_size_adcdcoc_6g = 4 * 5;
	chip_cap->prek_ee_info.pre_cal_total_size =
	chip_cap->prek_ee_info.cal_result_size + chip_cap->prek_ee_info.cal_result_size_adcdcoc_2g +
	chip_cap->prek_ee_info.cal_result_size_5g + chip_cap->prek_ee_info.cal_result_size_6g +
	chip_cap->prek_ee_info.cal_result_size_adcdcoc_5g + chip_cap->prek_ee_info.cal_result_size_adcdcoc_6g;

	/* DPD & Flatness item */

	chip_cap->prek_ee_info.per_legaycy_ch_cal_size = 5 * 1024;
	chip_cap->prek_ee_info.per_mem_ch_cal_size = 16 * 1024;
	chip_cap->prek_ee_info.per_ch_cal_OTFG0_size = 2 * 1024;

	chip_cap->prek_ee_info.per_ch_6g_num = MT7992_PER_CH_A6_BW20_BW80_BW160_SIZE;
	chip_cap->prek_ee_info.per_ch_5g_num = MT7992_PER_CH_A5_BW20_BW80_BW160_SIZE;
	chip_cap->prek_ee_info.per_ch_2g_num = MT7992_PER_CH_G_BW20_SIZE;

	chip_cap->prek_ee_info.total_chan_for_per_ch =
	chip_cap->prek_ee_info.per_ch_6g_num + chip_cap->prek_ee_info.per_ch_5g_num + chip_cap->prek_ee_info.per_ch_2g_num;

	chip_cap->prek_ee_info.dpd_cal_6g_legacy_size =
	(chip_cap->prek_ee_info.per_legaycy_ch_cal_size + chip_cap->prek_ee_info.per_ch_cal_OTFG0_size)  * MT7992_PER_CH_A6_BW20_SIZE;

	chip_cap->prek_ee_info.dpd_cal_6g_mem_size =
	(chip_cap->prek_ee_info.per_mem_ch_cal_size + chip_cap->prek_ee_info.per_ch_cal_OTFG0_size) * (MT7992_PER_CH_A6_BW80_SIZE + MT7992_PER_CH_A6_BW160_SIZE);

	chip_cap->prek_ee_info.dpd_cal_6g_total_size =
	chip_cap->prek_ee_info.dpd_cal_6g_legacy_size + chip_cap->prek_ee_info.dpd_cal_6g_mem_size;

	chip_cap->prek_ee_info.dpd_cal_5g_legacy_size =
	(chip_cap->prek_ee_info.per_legaycy_ch_cal_size + chip_cap->prek_ee_info.per_ch_cal_OTFG0_size) * MT7992_PER_CH_A5_BW20_SIZE;

	chip_cap->prek_ee_info.dpd_cal_5g_mem_size =
	(chip_cap->prek_ee_info.per_mem_ch_cal_size + chip_cap->prek_ee_info.per_ch_cal_OTFG0_size) * (MT7992_PER_CH_A5_BW80_SIZE + MT7992_PER_CH_A5_BW160_SIZE);

	chip_cap->prek_ee_info.dpd_cal_5g_total_size =
	chip_cap->prek_ee_info.dpd_cal_5g_legacy_size + chip_cap->prek_ee_info.dpd_cal_5g_mem_size;


	chip_cap->prek_ee_info.dpd_cal_2g_total_size =
	(chip_cap->prek_ee_info.per_legaycy_ch_cal_size + chip_cap->prek_ee_info.per_ch_cal_OTFG0_size) * chip_cap->prek_ee_info.per_ch_2g_num;

	chip_cap->prek_ee_info.dpd_cal_total_size =
	chip_cap->prek_ee_info.dpd_cal_6g_total_size + chip_cap->prek_ee_info.dpd_cal_5g_total_size + chip_cap->prek_ee_info.dpd_cal_2g_total_size;

	/* Flash offset */
	chip_cap->prek_ee_info.pre_cal_flash_offset = CAL_FLASH_OFFSET;
	chip_cap->prek_ee_info.group_flash_offset_g_begin = 0;
	chip_cap->prek_ee_info.group_flash_offset_a5_begin =
	chip_cap->prek_ee_info.group_flash_offset_g_begin + chip_cap->prek_ee_info.cal_result_size;
	chip_cap->prek_ee_info.group_flash_offset_a6_begin =
	chip_cap->prek_ee_info.group_flash_offset_a5_begin + chip_cap->prek_ee_info.cal_result_size_5g;

	chip_cap->prek_ee_info.dpd_flash_offset = CAL_FLASH_OFFSET + chip_cap->prek_ee_info.pre_cal_total_size;

	chip_cap->prek_ee_info.dpd_flash_offset_g_begin = 0;
	chip_cap->prek_ee_info.dpd_flash_offset_a5_begin =
	chip_cap->prek_ee_info.dpd_flash_offset_g_begin + chip_cap->prek_ee_info.dpd_cal_2g_total_size;
	chip_cap->prek_ee_info.dpd_flash_offset_a6_begin =
	chip_cap->prek_ee_info.dpd_flash_offset_a5_begin + chip_cap->prek_ee_info.dpd_cal_5g_total_size;
#endif
#ifdef OCE_SUPPORT
       chip_cap->FdFrameFwOffloadEnabled = TRUE;
#endif /* OCE_SUPPORT */
	chip_cap->bus_info = &mt7992_bus_info;
	chip_cap->sizeof_rxd_grp0 = sizeof(struct bmac_rxd_grp_0);
	chip_cap->fid_shift = 15;
	chip_cap->fid_invalid_id = UMAC_FID_1FFF_FAULT;
	chip_cap->hw_band_num = CFG_WIFI_MT7992_BAND_NUM;
	chip_cap->rxgaincal_ofst = RX_GAIN_CAL_EEPROM_OFST;
#ifdef TX_POWER_CONTROL_SUPPORT_V2
	chip_cap->tx_power_boost_ver = 2;
#endif
	chip_cap->step_one_timeout = RRO_STEP_ONE_TIMEOUT_V2;
	chip_cap->flush_all_timeout = RRO_FLUSH_ALL_TIMEOUT_V2;
#ifdef DOT11V_MBSSID_SUPPORT
	chip_cap->transmitted_bss_num = MT7992_MAX_TRANSMITTED_NUM;
	chip_cap->transmitted_bss_bitmap = MT7992_TRANSMITTED_BSS_BITMAP;
#endif /* DOT11V_MBSSID_SUPPORT */
	chip_cap->rss_map.map0 = RRO_RSS_MAP_4RING;
	chip_cap->rss_map.map1 = RRO_RSS_MAP_4RING;
	/* use 256 for per second probe rsp drop */
	chip_cap->ProbeRspMaxNum = 256;
	chip_cap->WtblDupNum = 80;
}

static VOID mt7992_chipCap_post_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_CAP *chip_cap)
{
	chip_cap->asic_caps |= fASIC_CAP_ADDBA_HW_SSN;
}

/* Add txd_refine function due to some txd bit only works on specific chips.
 * We add the function here to meet the need of one driver for all chips.
 */
static VOID mt7992_txd_post_process(
	struct _RTMP_ADAPTER *pAd,
	UCHAR *tmac_info, MAC_TX_INFO *info,
	union _HTTRANSMIT_SETTING *pTransmit)
{
	struct txd_l *txd = (struct txd_l *)tmac_info;
	/* Raise FBCZ bit to force bss color zero for BCN frame with HE rate*/
	if ((info->Type == FC_TYPE_MGMT)
		&& (info->SubType == SUBTYPE_BEACON)
		&& (pTransmit->field.MODE > MODE_VHT))
		txd->txd_5 |= WF_TX_DESCRIPTOR_FBCZ_MASK;
}

static UINT32 mt7992_get_wf_path_comb(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 dbdc_mode_en,
	UINT8 *path,
	UINT8 *path_len)
{
	UINT8 i = 0;
	UINT8 wf_path_begin[3] = {0, 0, 0};

	/* sanity check for null pointer */
	if (!path)
		return FALSE;

	/* sanity check for null pointer */
	if (!path_len)
		return FALSE;

	for (i = 0; i < *path_len; i++)
		*(path + i) = wf_path_begin[band_idx] + i;

	if (*path_len > MAX_ANTENNA_NUM)
		return FALSE;

	return FALSE;
}

static VOID mt7992_reset_rro_entry(
	struct _RTMP_ADAPTER *ad,
	VOID *pEntry,
	VOID *pRecBAEntry, VOID *MLD_Entry, UINT16 mld_sta_idx, UINT8 bPNChk)
{
	struct _MAC_TABLE_ENTRY *entry = (struct _MAC_TABLE_ENTRY *)pEntry;
	struct BA_REC_ENTRY *pBAEntry = (struct BA_REC_ENTRY *)pRecBAEntry;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = (struct mld_entry_t *)MLD_Entry;
#endif


#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
	if ((!bPNChk && (entry->DISABLE_PN_CHK & (BIT(pBAEntry->TID)))) ||
		(bPNChk && !(entry->DISABLE_PN_CHK & (BIT(pBAEntry->TID))))) {
		return;
	}
#endif

	if (bPNChk)
		entry->DISABLE_PN_CHK &= ~(BIT(pBAEntry->TID));
	else
		entry->DISABLE_PN_CHK |= BIT(pBAEntry->TID);

	pBAEntry->RRO_RESET_CNT++;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();
	if (mld_entry) {
		if (entry->wdev) {
			if (entry->wdev->wdev_type == WDEV_TYPE_AP)
				bss_mngr_mld_ba_del_from_asic(entry->wdev,
					mld_sta_idx, pBAEntry->TID, BA_SESSION_RECP, 0);
#ifdef CONFIG_STA_SUPPORT
			else if (entry->wdev->wdev_type == WDEV_TYPE_STA)
				sta_mld_ba_del_from_asic(entry->wdev,
					entry, pBAEntry->TID, BA_SESSION_RECP, 0);
#endif/*CONFIG_STA_SUPPORT*/
		}
	} else
#endif
	{
		RTMP_DEL_BA_SESSION_FROM_ASIC(ad, entry->wcid, pBAEntry->TID,
			BA_SESSION_RECP, 0);
	}

#ifdef DOT11_EHT_BE
	if (mld_entry) {
		if (entry->wdev) {
			UINT8 idx;

			for (idx = 0; idx < MLD_LINK_MAX; idx++) {
				if (mld_entry->link_entry[idx]) {
					struct _MAC_TABLE_ENTRY *link_entry =
						(struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[idx];

					if (bPNChk)
						link_entry->DISABLE_PN_CHK &= ~(BIT(pBAEntry->TID));
					else
						link_entry->DISABLE_PN_CHK |= BIT(pBAEntry->TID);
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
					if (glb_npu_en_node)
						WIFI_MAIL_API_SET_WAIT_ADD_DEL_STA(bPNChk,
							((link_entry->wcid & 0x7ff) | ((pBAEntry->TID & 0xf) << 11)));
#endif
				}
			}
			if (entry->wdev->wdev_type == WDEV_TYPE_AP)
				bss_mngr_mld_ba_add_to_asic(entry->wdev,
					mld_sta_idx, pBAEntry->TID, pBAEntry->LastIndSeq,
					pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
#ifdef CONFIG_STA_SUPPORT
			else if (entry->wdev->wdev_type == WDEV_TYPE_STA)
				sta_mld_ba_add_to_asic(entry->wdev,
					entry, pBAEntry->TID, pBAEntry->LastIndSeq,
					pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
#endif/*CONFIG_STA_SUPPORT*/
		}
	} else
#endif
	{
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
		if (glb_npu_en_node)
			WIFI_MAIL_API_SET_WAIT_ADD_DEL_STA(bPNChk,
				(entry->wcid & 0x7ff) | ((pBAEntry->TID & 0xf) << 11));
#endif
		RTMP_ADD_BA_SESSION_TO_ASIC(ad, entry->wcid, pBAEntry->TID,
			pBAEntry->LastIndSeq, pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
	}

#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"RESET; Wcid=%d TID=%d bPNChk=%d DisPnChk=%d RRO_RESET_CNT=%ld flag_err_cnt=%ld\n",
		entry->wcid, pBAEntry->TID, bPNChk, entry->DISABLE_PN_CHK, pBAEntry->RRO_RESET_CNT, pBAEntry->flag_err_cnt);
}

static VOID mt7992_extra_action(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR *peer_mac,
	enum CHIP_EXTRA_ACTION action,
	UCHAR *data,
	ULONG *data_len)
{
	struct _MAC_TABLE_ENTRY *entry = NULL;

	MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"start to do extra action(=0x%x).\n", action);

	if (action & CHIP_EXTRA_ACTION_ICV_ERROR) {
		struct _RX_BLK *pRxBlk = (struct _RX_BLK *)data;
		struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ad->physical_dev);
		struct BA_REC_ENTRY *pBAEntry;
		struct BA_INFO *ba_info;
		struct _MAC_TABLE_ENTRY *link_entry = NULL;
#ifdef DOT11_EHT_BE
		struct mld_entry_t *mld_entry = NULL;
		uint16_t mld_sta_idx = 0;
#endif

		if (pRxBlk->pRxInfo->Mcast || pRxBlk->pRxInfo->Bcast || (pRxBlk->TID > 7))
			return;

		if (VALID_UCAST_ENTRY_WCID(ad, pRxBlk->wcid))
			entry = &ad->MacTab->Content[pRxBlk->wcid];

		if (!entry || IS_ENTRY_NONE(entry) || IS_CIPHER_NONE_OR_WEP_Entry(entry))
			return;

		ba_info = &entry->ba_info;

#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();
		if (IS_ENTRY_MLO(entry)) {
			mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
			if (!mld_entry) {
				MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
					"Get mld_entry fail for tid:%d\n", pRxBlk->TID);
				mt_rcu_read_unlock();
				return;
			}
			mld_sta_idx = mld_entry->mld_sta_idx;
			ba_info = &mld_entry->ba_info;
			link_entry = &ad->MacTab->Content[mld_entry->setup_link_wcid];
		} else
#endif /* DOT11_EHT_BE */
		{
			link_entry = entry;
		}

#ifdef DOT11_EHT_BE
		mt_rcu_read_unlock();
#endif

		pBAEntry = &ba_ctl->BARecEntry[ba_info->RecWcidArray[pRxBlk->TID]];

		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_ICV_ERR)) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"ICV ERR! wcid=%d TID=%u SN=%u PN=%llx ErrFlags=0x%x Flags=0x%x\n",
				pRxBlk->wcid, pRxBlk->TID, pRxBlk->SN, pRxBlk->CCMP_PN, pRxBlk->ErrFlags, pRxBlk->Flags);
			pBAEntry->HAS_ICV_ERROR = TRUE;
			pBAEntry->icv_err_cnt++;
			if (!RX_BLK_TEST_ERR_FLAG(pRxBlk, fRX_ERR_OLD_PKT))
				pBAEntry->CCMP_ICV_ERROR = TRUE;
		} else if (pBAEntry->CCMP_ICV_ERROR) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"Reset! wcid=%d TID=%u SN=%u PN=%llx ErrFlags=0x%x Flags=0x%x\n",
				pRxBlk->wcid, pRxBlk->TID, pRxBlk->SN, pRxBlk->CCMP_PN, pRxBlk->ErrFlags, pRxBlk->Flags);
			pBAEntry->CCMP_ICV_ERROR = FALSE;
			mt7992_reset_rro_entry(ad, (VOID *)link_entry, (VOID *)pBAEntry, (VOID *)mld_entry, mld_sta_idx, 0);
		}

		if (pRxBlk->ErrFlags) {
			pBAEntry->flag_err_cnt++;
			if (link_entry->DISABLE_PN_CHK & BIT(pRxBlk->TID)) {
				RX_BLK_CLEAR_ERR_FLAG(pRxBlk, fRX_ERR_PN_CHK_FAIL);
				RX_BLK_CLEAR_ERR_FLAG(pRxBlk, fRX_ERR_OLD_PKT);
			}
		}


		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		return;
	}

	if (action & CHIP_EXTRA_ACTION_PN_CHK) {
		struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ad->physical_dev);
		struct BA_REC_ENTRY *pBAEntry;
		struct BA_INFO *ba_info;
#ifdef DOT11_EHT_BE
		struct mld_entry_t *mld_entry = NULL;
		uint16_t mld_sta_idx = 0;
#endif
		UINT32 idx, tid;


		for (idx = 1; VALID_UCAST_ENTRY_WCID(ad, idx); idx++) {
			entry = &ad->MacTab->Content[idx];

			if (IS_ENTRY_NONE(entry) || IS_CIPHER_NONE_OR_WEP_Entry(entry))
				continue;

			ba_info = &entry->ba_info;

#ifdef DOT11_EHT_BE
			mt_rcu_read_lock();
			if (IS_ENTRY_MLO(entry)) {
				mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
				if (!mld_entry) {
					MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
						"Get mld_entry fail\n");
					mt_rcu_read_unlock();
					return;
				}
				if (mld_entry->setup_link_wcid != idx) {
					mt_rcu_read_unlock();
					continue;
				}
				ba_info = &mld_entry->ba_info;
				mld_sta_idx = mld_entry->mld_sta_idx;
			}
			mt_rcu_read_unlock();
#endif /* DOT11_EHT_BE */
			for (tid = 0; tid < NUM_OF_TID; tid++) {
				pBAEntry = &ba_ctl->BARecEntry[ba_info->RecWcidArray[tid]];

				NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
				if (pBAEntry->REC_BA_Status != Recipient_Offload) {
					NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
					continue;
				}
				if ((entry->DISABLE_PN_CHK & BIT(tid)) &&
					((ad->Mlme.PeriodicRound % 80) == 0)) {//8s
					if (pBAEntry->HAS_ICV_ERROR)
						pBAEntry->HAS_ICV_ERROR = 0;
					else
						mt7992_reset_rro_entry(ad, (VOID *)entry,
							(VOID *)pBAEntry, (VOID *)mld_entry,
							mld_sta_idx, 1);
				}
				NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			}
		}
	}

	if (action & CHIP_EXTRA_ACTION_ICV_ERR_INIT) {
		struct BA_REC_ENTRY *pBAEntry = (struct BA_REC_ENTRY *)data;

		entry = pBAEntry->pEntry;
		pBAEntry->RRO_RESET_CNT = 0;
		pBAEntry->CCMP_ICV_ERROR = 0;
		pBAEntry->HAS_ICV_ERROR = 0;
		pBAEntry->icv_err_cnt = 0;
		pBAEntry->flag_err_cnt = 0;

		if (!entry)
			return;

#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();
		if (IS_ENTRY_MLO(entry)) {
			UINT8 idx;
			struct mld_entry_t *mld_entry;

			mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
			if (!mld_entry) {
				MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
						"Get mld_entry fail\n");
				mt_rcu_read_unlock();
				return;
			}
			for (idx = 0; idx < MLD_LINK_MAX; idx++) {
				if (mld_entry->link_entry[idx]) {
					struct _MAC_TABLE_ENTRY *link_entry =
						(struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[idx];

					link_entry->DISABLE_PN_CHK = 0;
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
					if (glb_npu_en_node)
						WIFI_MAIL_API_SET_WAIT_ADD_DEL_STA(1,
							(link_entry->wcid & 0x7ff) | ((pBAEntry->TID & 0xf) << 11));
#endif
				}
			}
		} else
#endif /* DOT11_EHT_BE */
		{
			entry->DISABLE_PN_CHK = 0;
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
			if (glb_npu_en_node)
				WIFI_MAIL_API_SET_WAIT_ADD_DEL_STA(1, (pBAEntry->pEntry->wcid & 0x7ff) | ((pBAEntry->TID & 0xf) << 11));
#endif
		}
#ifdef DOT11_EHT_BE
		mt_rcu_read_unlock();
#endif
	}
}

static VOID mt7992_set_rro_timeout(
	struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	int ret = NDIS_STATUS_SUCCESS;

	if (!(chip_cap->asic_caps & fASIC_CAP_HW_RRO))
		return;

	if (chip_cap->step_one_timeout && chip_cap->flush_all_timeout) {
		ret = uni_cmd_rro_flush_timeout(
			ad, chip_cap->step_one_timeout, chip_cap->flush_all_timeout);
		MTWF_PRINT("%s(%d):ret=0x%x,step_one_timeout = %d,flush_all_timeout = %d\n",
			__func__, __LINE__, ret,
			chip_cap->step_one_timeout, chip_cap->flush_all_timeout);
	}
}

static VOID mt7992_set_rro_rss_map(
	struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	struct rro_rss_map *rss_map = &chip_cap->rss_map;
	int ret = NDIS_STATUS_SUCCESS;

	if (rss_map->map0 || rss_map->map1) {
		ret = uni_cmd_rro_rss_map(ad, rss_map->map0, rss_map->map1);
		MTWF_PRINT("%s(%d):ret=0x%x,rro_rss_map0 = 0x%x,rro_rss_map1 = 0x%x\n",
			__func__, __LINE__, ret, rss_map->map0, rss_map->map1);
	}
}

static BOOLEAN mt7992_set_spe_fr_table(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 rate_idx,
	VOID *handle)
{
	struct UNI_CMD_FR_TABLE_SET_T *fr_tab_set = (struct UNI_CMD_FR_TABLE_SET_T *)handle;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_FR_TABLE, DBG_LVL_DEBUG,
		"band_idx = %d, rate_idx = %d\n",
		band_idx, rate_idx);
	fr_tab_set->u2Tag = UNI_CMD_FR_TABLE_SET;
	fr_tab_set->u2Length = sizeof(struct UNI_CMD_FR_TABLE_SET_T);
	fr_tab_set->u2Tag = cpu2le16(fr_tab_set->u2Tag);
	fr_tab_set->u2Length = cpu2le16(fr_tab_set->u2Length);
	fr_tab_set->ucTabIdx = rate_idx;
	fr_tab_set->ucFrAntIdx = 0;
	fr_tab_set->ucFrSpeIdxSel = 0;
	switch (band_idx) {
	case BAND1: /* 5G */
		fr_tab_set->ucFrSpeIdx = BAND1_SPE_IDX;
		fr_tab_set->u2FixedRate = 0x004B;
		break;
	case BAND0: /* 2G */
	default:
		fr_tab_set->ucFrSpeIdx = BAND0_SPE_IDX;
		fr_tab_set->u2FixedRate = 0x004B; /* for better performance */
		break;
	}
	if (ad->CommonCfg.BcnDataRate != 0xFFFF)
		fr_tab_set->u2FixedRate = ad->CommonCfg.BcnDataRate;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_FR_TABLE, DBG_LVL_DEBUG,
		"fr_tab_set->u2FixedRate = 0x%x\n",
		fr_tab_set->u2FixedRate);
	fr_tab_set->ucFrGI = 1;
	fr_tab_set->ucFrHeLtf = 1;
	fr_tab_set->fgFrLDPC = 0;
	fr_tab_set->fgFrTBF = 0;
	fr_tab_set->fgFrDynBw = 0;
	return TRUE;
}

/* Only need to do once */
INT32 mt7992_mactbl_uwtbl_adjust(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(ad);
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	struct physical_device *ph_dev = (struct physical_device *)ad->physical_dev;
	int ret = 0;
	u16 max_wtbl = 0, ucast_wtbl = 0;
	u16 hw_group_low = 0, hw_group_high = 0;
	u16 hw_uwtbl_low = 0, hw_uwtbl_high = 0;
	/* to judje need re-alloc or not */
	u16 orig_max_wtbl = 0;

	if (mac_dev == NULL) {
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_ERROR,
			"mac_dev is NULL\n");
		return -1;
	}

#ifndef SW_CONNECT_SUPPORT
/* SW_CONNECT_SUPPORT cost more host memory
 *  suppose MEMORY_SLIM is not compiled
 */
#ifdef MEMORY_SLIM
	/* MT7992 MEM Shrink */
	ret = mac_dev->ops->set_max_uwtbl_sta_num(&mac_dev->hw, MAX_STA_NUM, (2 * MAX_BEACON_NUM));

	if (ret) {
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_ERROR, "ret=%d\n", ret);
		return ret;
	}
#endif /* MEMORY_SLIM */
#endif /* !SW_CONNECT_SUPPORT */

	/* get the original hw bcast range */
	mac_dev->ops->get_wtbl_idrm_range_num(&mac_dev->hw, &hw_group_low,
						&hw_group_high, MAC_STA_TYPE_GROUP);

	orig_max_wtbl = WTBL_MAX_NUM(ad);

#ifdef CONFIG_CPE_SUPPORT
	{
		/* To support 15 AP MBSS + 1 APCLI per band, need 34 bcast grps */
		u16 sku_max_sta = 509, sku_max_grp = 34;

		ret = mac_dev->ops->set_max_uwtbl_sta_num(&mac_dev->hw, sku_max_sta, sku_max_grp);
		if (ret) {
			MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_ERROR,
					"set max uwtbl sta num fail, ret=%d\n", ret);
			return ret;
		}
	}
#endif

	/* sync hwifi's max cap to physical dev */
	max_wtbl = mac_dev->ops->get_max_uwtbl_num(&mac_dev->hw, TRUE);
	ucast_wtbl = mac_dev->ops->get_max_uwtbl_num(&mac_dev->hw, FALSE);
	MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE,
		"orig_max_wtbl=%u, new max_wtbl=%u, new ucast_wtbl=%u\n", orig_max_wtbl, max_wtbl, ucast_wtbl);
	if (max_wtbl && ucast_wtbl)
		hc_set_chip_wtbl_num(ad->hdev_ctrl, max_wtbl, (ucast_wtbl+1)); /* ucast_wtbl need + wcid 0 */

	mac_dev->ops->get_wtbl_idrm_range_num(&mac_dev->hw, &hw_uwtbl_low,
				&hw_uwtbl_high, MAC_STA_TYPE_NORMAL);

#ifdef SW_CONNECT_SUPPORT
	if (hc_is_sw_sta_enable(ad)) {
		/* Force off dup WTBL */
		chip_cap->WtblDupNum = 0;
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE,
					"[BN %d] SW STA On , Off WTBL DUP !!\n", hc_get_hw_band_idx(ad));
	}
#endif /* SW_CONNECT_SUPPORT */

	if (chip_cap->WtblDupNum && ucast_wtbl > chip_cap->WtblDupNum)
		uni_cmd_chip_config_dup_wtbl_range(ad, hw_uwtbl_high-chip_cap->WtblDupNum+1, chip_cap->WtblDupNum);

#ifdef SW_CONNECT_SUPPORT
	if (hc_is_sw_sta_enable(ad)) {
		u16 sw_uwtbl_low = 0, sw_uwtbl_high = 0;
		u16 sw_group_low = 0, sw_group_high = 0;
		u16 sw_ucast_max = hc_get_chip_sw_ucast_max_num(ad->hdev_ctrl);

		ASSERT(sw_ucast_max); /* expect atleast one */

		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE,
				"sw_ucast_max=%d\n", sw_ucast_max);

		mac_dev->ops->get_wtbl_idrm_range_num(&mac_dev->hw, &hw_group_low,
							&hw_group_high, MAC_STA_TYPE_GROUP);

		/* S/W U/C Layout : N to 1 mappinng to H/W U/C, with S/W config Size */
		/* as 511 as hw_uwtbl_high example : Dummy 510/511 is also as S/W */
		sw_uwtbl_low = (hw_uwtbl_high - PD_GET_BAND_NUM(ad->physical_dev) + 1);
		sw_uwtbl_high = (sw_uwtbl_low + sw_ucast_max - 1);
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE,
				"hw_uwtbl_low=%d hw_uwtbl_high=%d, sw_uwtbl_low=%d sw_uwtbl_high=%d\n",
				hw_uwtbl_low, hw_uwtbl_high, sw_uwtbl_low, sw_uwtbl_high);
		hc_set_chip_sw_ucast_range_num(ad->hdev_ctrl, sw_uwtbl_low, sw_uwtbl_high);
		os_idrm_init(&(ph_dev->sw_uwtbl_mgmt), sw_uwtbl_low, sw_uwtbl_high);

		/* S/W B/C Layout :  1 to 1 mapping to H/W B/C , with Fixed H/W Group Num */
		sw_group_low = (sw_uwtbl_high + 1);
		sw_group_high = (sw_group_low + (hw_group_high - hw_group_low));
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE,
				"hw_group_low=%d hw_group_high=%d, sw_group_low=%d sw_group_high=%d\n",
				hw_group_low, hw_group_high, sw_group_low, sw_group_high);
		hc_set_chip_sw_bcast_range_num(ad->hdev_ctrl, sw_group_low, sw_group_high);
		os_idrm_init(&(ph_dev->sw_group_mgmt), sw_group_low, sw_group_high);

		/* Update the H/W + S/W upper bound for generic MACRO use
		 * ex: WTBL_MAX_NUM() & GET_MAX_UCAST_NUM()
		 */
		/* ex : max_wtbl(544, include wcid 0) + (516 - 2 dummy wcid) = 1058 = WTBL_MAX_NUM() */
		hc_set_chip_hw_sw_wtbl_num(ad->hdev_ctrl, max_wtbl, (sw_ucast_max - PD_GET_BAND_NUM(ad->physical_dev)));
		/* ex: (1 (wcid 0)) + 511 + (515 -2) = 1027  = GET_MAX_UCAST_NUM() */
		hc_set_chip_hw_sw_ucast_num(ad->hdev_ctrl, (ucast_wtbl+1), (sw_ucast_max - PD_GET_BAND_NUM(ad->physical_dev)));
	}
#endif /* SW_CONNECT_SUPPORT */

	if (orig_max_wtbl != WTBL_MAX_NUM(ad)) {
		/* re-size from ex:
		*	1). FULL SW Case : 544 to 1057 for H/W + S/W
		*/
		entry_table_deinit(ph_dev);
		entry_table_init(ph_dev);
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE,
				"mactbl re-size from orig_max_wtbl=%u to WTBL_MAX_NUM(ad)=%u\n", orig_max_wtbl, WTBL_MAX_NUM(ad));
	}

	return ret;
}

#ifdef VERIFICATION_MODE
static VOID mt7992_dump_rmac_info_normal(
	struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	UINT32 *rxd = (UINT32 *)rmac_info;

	hex_dump32("rxd raw data: ", rxd,
		sizeof(struct bmac_rxd_grp_0));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG, "RxData_BASE:\n");

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tPktType = %u(%s), ",
		(UINT32)(HAL_RX_STATUS_GET_PACKET_TYPE(rxd)),
		rxd_pkt_type_str(HAL_RX_STATUS_GET_PACKET_TYPE(rxd)));

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"RxByteCnt = %u\n",
		(UINT32)(HAL_RX_STATUS_GET_RX_BYTE_COUNT(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tEtherTypeOffset = %u(WORD), ",
		(UINT32)(HAL_RX_STATUS_GET_ETHER_TYPE_OFFSET(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"IP/UT = %d/%d, ",
		HAL_RX_STATUS_GET_IP(rxd),
		HAL_RX_STATUS_GET_UT(rxd));

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tWlanIndex = %u, ",
		(UINT32)(HAL_RX_STATUS_GET_MLD_ID(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"GroupValid = 0x%x, ",
		(UINT32)(HAL_RX_STATUS_GET_GROUP_VLD(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"BN = %u, ",
		(UINT32)(HAL_RX_STATUS_GET_BN(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tSEC Mode = %u, ",
		(UINT32)(HAL_RX_STATUS_GET_SEC_MODE(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"KeyID = %u, ",
		(UINT32)(HAL_RX_STATUS_GET_KID(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"CM = %d, ",
		HAL_RX_STATUS_GET_CM(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"CLM = %d,",
		HAL_RX_STATUS_GET_CLM(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tICV Err(I) = %d, ",
		HAL_RX_STATUS_GET_I(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"TKIP MIC Err(T) = %d, ",
		HAL_RX_STATUS_GET_T(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"FCE Error(FC) = %d\n",
		HAL_RX_STATUS_GET_FC(rxd));

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tBSSID = 0x%x, ",
		HAL_RX_STATUS_GET_BSSID(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"BF_CQI = %d, ",
		HAL_RX_STATUS_GET_BF_CQI(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"HdearLength(MAC) = %d\n",
		(UINT32)(HAL_RX_STATUS_GET_HEADER_LENGTH(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tHeaerTrans(H) = %d, ",
		HAL_RX_STATUS_GET_H(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"HeaerOffset(HO) = %d, ",
		(UINT32)(HAL_RX_STATUS_GET_HO(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"TID = %d\n",
		(UINT32)(HAL_RX_STATUS_GET_TID(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tMU_BAR = %d, ",
		HAL_RX_STATUS_GET_MUBAR(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"SWBIT = %d, ",
		HAL_RX_STATUS_GET_SWBIT(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"DeAMSDU Fail(DAF) = %d\n",
		HAL_RX_STATUS_GET_DAF(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tExceedMax Rx Length(EL) = %d, ",
		HAL_RX_STATUS_GET_EL(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"HdrTransFail(HTF) = %d\n",
		HAL_RX_STATUS_GET_HTF(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tInterested Frame(INTF) = %d, ",
		HAL_RX_STATUS_GET_INTF(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"Fragment Frame(FRAG) = %d\n",
		HAL_RX_STATUS_GET_FRAG(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tNull Frame(NULL) = %d, ",
		HAL_RX_STATUS_GET_NUL(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"Non Data Frame(NDATA) = %d\n",
		HAL_RX_STATUS_GET_NDATA(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tNon-AMPDU Subframe(NASF) = %d, ",
		HAL_RX_STATUS_GET_NAMP(rxd));

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"BF_RPT = %d\n",
		HAL_RX_STATUS_GET_BF_RPT(rxd));

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tRX Vector Sequence No = %d, ",
		(UINT32)(HAL_RX_STATUS_GET_RXV_SN(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"Channel Frequency = %d\n",
		(UINT32)(HAL_RX_STATUS_GET_CH_FREQUENCY(rxd)));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tHTC/UC2ME/MC/BC = %d/%d/%d/%d\n",
		HAL_RX_STATUS_GET_HTC(rxd),
		(HAL_RX_STATUS_GET_A1_TYPE(rxd) == 0x1) ? 1 : 0,
		(HAL_RX_STATUS_GET_A1_TYPE(rxd) == 0x2) ? 1 : 0,
		(HAL_RX_STATUS_GET_A1_TYPE(rxd) == 0x3) ? 1 : 0);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tTCL = %d, ",
		HAL_RX_STATUS_GET_TCL(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"BBM = %d, ",
		HAL_RX_STATUS_GET_BBM(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"BU = %d, ",
		HAL_RX_STATUS_GET_BU(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tPF = %d, ",
		HAL_RX_STATUS_GET_PF(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"DP = %d, ",
		HAL_RX_STATUS_GET_DP(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"CLS = %d, ",
		HAL_RX_STATUS_GET_CLS(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"MGC = %d\n",
		HAL_RX_STATUS_GET_MGC(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"\tWOL = %d, ",
		HAL_RX_STATUS_GET_WOL(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"CLS_BITMAP_31_0 = 0x%x, ",
		HAL_RX_STATUS_GET_CLS_BITMAP_31_0_(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"CLS_BITMAP_33_32 = 0x%x, ",
		HAL_RX_STATUS_GET_CLS_BITMAP_33_32_(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"PF_MODE = %d, ",
		HAL_RX_STATUS_GET_PF_MODE(rxd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"PF_STS = %d\n",
		HAL_RX_STATUS_GET_PF_STS(rxd));
}
#endif /* VERIFICATION_MODE */

/*
* 1'b0: the related GROUP is not present
* 1'b1: the related GROUP is present
* bit[0]: indicates GROUP1 (DW10~DW13)
* bit[1]: indicates GROUP2 (DW14~DW15)
* bit[2]: indicates GROUP3 (DW16~DW17)
* bit[3]: indicates GROUP4 (DW6~DW9)
* bit[4]: indicates GROUP5 (DW18~DW35)
*/
static INT32 mt7992_trans_rxd_into_rxblk(
	struct _RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, PNDIS_PACKET rx_pkt)
{

	UCHAR *rmac_info, *pos, *fc = NULL;
	UINT32 *rxd_grp0;
	struct bmac_rxd_grp_1 *rxd_grp1 = NULL;
	struct bmac_rxd_grp_2 *rxd_grp2 = NULL;
	struct bmac_rxd_grp_3 *rxd_grp3 = NULL;
	struct bmac_rxd_grp_4 *rxd_grp4 = NULL;
	struct bmac_rxd_grp_5 *rxd_grp5 = NULL;
	UINT8 i;
	INT32 rmac_info_len = BMAC_RMAC_INFO_BASE_SIZE;
	UINT8 ho_len;
	UINT16 temp_fc, fn_sn = 0;

	rmac_info = (UCHAR *)(GET_OS_PKT_DATAPTR(rx_pkt));
#ifdef VERIFICATION_MODE
	if (pAd->veri_ctrl.dump_rx_debug)
		mt7992_dump_rmac_info_normal(pAd, rmac_info);
#endif

	rx_blk->pRxInfo = (struct _RXINFO_STRUC *)(&rx_blk->hw_rx_info[RXINFO_OFFSET]);
	pos = rmac_info;
	rx_blk->rmac_info = rmac_info;
	rxd_grp0 = (UINT32 *)rmac_info;
	pos += BMAC_RMAC_INFO_BASE_SIZE;
#ifdef CFG_BIG_ENDIAN
	MTMacInfoEndianChange(
		pAd, rmac_info, TYPE_RMACINFO, BMAC_RMAC_INFO_BASE_SIZE);
#endif

	if (HAL_RX_STATUS_GET_GROUP_VLD(rxd_grp0) & BMAC_GROUP_VLD_4) {
		rxd_grp4  = (struct bmac_rxd_grp_4 *)pos;
		rmac_info_len += BMAC_RMAC_INFO_GRP_4_SIZE;
#ifdef CFG_BIG_ENDIAN
		RTMPDescriptorEndianChange((UCHAR *)rxd_grp4, 0);
#endif
		fc = pos;
		pos += BMAC_RMAC_INFO_GRP_4_SIZE;
	}

	if (HAL_RX_STATUS_GET_GROUP_VLD(rxd_grp0) & BMAC_GROUP_VLD_1) {
		rxd_grp1 = (struct bmac_rxd_grp_1 *)pos;
		rmac_info_len += BMAC_RMAC_INFO_GRP_1_SIZE;
		pos += BMAC_RMAC_INFO_GRP_1_SIZE;
	}

	if (HAL_RX_STATUS_GET_GROUP_VLD(rxd_grp0) & BMAC_GROUP_VLD_2) {
		rxd_grp2 = (struct bmac_rxd_grp_2 *)pos;
		rmac_info_len += BMAC_RMAC_INFO_GRP_2_SIZE;
		pos += BMAC_RMAC_INFO_GRP_2_SIZE;
	}

	if (HAL_RX_STATUS_GET_GROUP_VLD(rxd_grp0) & BMAC_GROUP_VLD_3) {
		rxd_grp3 = (struct bmac_rxd_grp_3 *)pos;
		rmac_info_len += BMAC_RMAC_INFO_GRP_3_SIZE;
		pos += BMAC_RMAC_INFO_GRP_3_SIZE;
	}

	if (HAL_RX_STATUS_GET_GROUP_VLD(rxd_grp0) & BMAC_GROUP_VLD_5) {
		rxd_grp5 = (struct bmac_rxd_grp_5 *)pos;
		rmac_info_len += BMAC_RMAC_INFO_GRP_5_SIZE;
		pos += BMAC_RMAC_INFO_GRP_5_SIZE;
	}

#ifdef CFG_BIG_ENDIAN
	if (rxd_grp4)
		RTMPEndianChange(
			(UCHAR *)rxd_grp4, BMAC_RMAC_INFO_GRP_4_SIZE);
	if (rxd_grp1)
		RTMPEndianChange(
			(UCHAR *)rxd_grp1, BMAC_RMAC_INFO_GRP_1_SIZE);
	if (rxd_grp2)
		RTMPEndianChange(
			(UCHAR *)rxd_grp2, BMAC_RMAC_INFO_GRP_2_SIZE);
	if (rxd_grp3)
		RTMPEndianChange(
			(UCHAR *)rxd_grp3, BMAC_RMAC_INFO_GRP_3_SIZE);
	if (rxd_grp5)
		RTMPEndianChange(
			(UCHAR *)rxd_grp5, BMAC_RMAC_INFO_GRP_5_SIZE);
#endif /* CFG_BIG_ENDIAN */

	rx_blk->MPDUtotalByteCnt =
		HAL_RX_STATUS_GET_RX_BYTE_COUNT(rxd_grp0) - rmac_info_len;

	ho_len = HAL_RX_STATUS_GET_HO(rxd_grp0);

	if (ho_len) {
		rx_blk->MPDUtotalByteCnt -= (ho_len << 1);
		rmac_info_len += (ho_len << 1);
	}

	rx_blk->DataSize = rx_blk->MPDUtotalByteCnt;
	rx_blk->band = HAL_RX_STATUS_GET_BN(rxd_grp0);
	rx_blk->wcid = HAL_RX_STATUS_GET_MLD_ID(rxd_grp0);
	rx_blk->bss_idx = HAL_RX_STATUS_GET_BSSID(rxd_grp0);
	rx_blk->sec_mode = HAL_RX_STATUS_GET_SEC_MODE(rxd_grp0);
	rx_blk->key_idx = HAL_RX_STATUS_GET_KID(rxd_grp0);
	if (HAL_RX_STATUS_GET_HTC(rxd_grp0))
		RX_BLK_SET_FLAG(rx_blk, fRX_HTC);

	if (HAL_RX_STATUS_GET_CM(rxd_grp0))
		RX_BLK_SET_FLAG(rx_blk, fRX_CM);

	if (HAL_RX_STATUS_GET_CLM(rxd_grp0))
		RX_BLK_SET_FLAG(rx_blk, fRX_CLM);

	if (HAL_RX_STATUS_GET_I(rxd_grp0))
		RX_BLK_SET_FLAG(rx_blk, fRX_ICV_ERR);

	if (HAL_RX_STATUS_GET_T(rxd_grp0))
		RX_BLK_SET_FLAG(rx_blk, fRX_TKIP_MIC_ERR);

	rx_blk->channel_freq = HAL_RX_STATUS_GET_CH_FREQUENCY(rxd_grp0);

	rx_blk->TID = HAL_RX_STATUS_GET_TID(rxd_grp0);

	if (HAL_RX_STATUS_GET_H(rxd_grp0))
		RX_BLK_SET_FLAG(rx_blk, fRX_HDR_TRANS);

	switch (HAL_RX_STATUS_GET_A1_TYPE(rxd_grp0)) {
	case 0x1:
		rx_blk->pRxInfo->U2M = 1;
		break;
	case 0x2:
		rx_blk->pRxInfo->Mcast = 1;
		break;
	case 0x3:
		rx_blk->pRxInfo->Bcast = 1;
		break;
	}

	rx_blk->AmsduState = HAL_RX_STATUS_GET_PF(rxd_grp0);

	if (HAL_RX_STATUS_GET_DAF(rxd_grp0))
		rx_blk->DeAmsduFail = 1;

	if (HAL_RX_STATUS_GET_FRAG(rxd_grp0))
		rx_blk->pRxInfo->FRAG = 1;

	if (HAL_RX_STATUS_GET_NUL(rxd_grp0))
		rx_blk->pRxInfo->NULLDATA = 1;

	if (!HAL_RX_STATUS_GET_NDATA(rxd_grp0))
		rx_blk->pRxInfo->DATA = 1;

	if (HAL_RX_STATUS_GET_HTC(rxd_grp0))
		rx_blk->pRxInfo->HTC = 1;

	if (!(HAL_RX_STATUS_GET_NAMP(rxd_grp0)))
		rx_blk->pRxInfo->AMPDU = 1;

	rx_blk->pRxInfo->L2PAD = 0;
	rx_blk->pRxInfo->AMSDU = 0;

	/* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	rx_blk->pRxInfo->CipherErr =
		((HAL_RX_STATUS_GET_I(rxd_grp0) ? 1 : 0) |
		((HAL_RX_STATUS_GET_T(rxd_grp0) ? 1 : 0) << 1));

	if (HAL_RX_STATUS_GET_FC(rxd_grp0))
		rx_blk->pRxInfo->Crc = 1;

	rx_blk->pRxInfo->MyBss = ((rx_blk->bss_idx == 0xf) ? 0 : 1);
	rx_blk->pRxInfo->Decrypted = 0;

	SET_OS_PKT_DATAPTR(rx_pkt, GET_OS_PKT_DATAPTR(rx_pkt) + rmac_info_len);
	SET_OS_PKT_LEN(rx_pkt, rx_blk->MPDUtotalByteCnt);

	rx_blk->pRxPacket = rx_pkt;
	rx_blk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(rx_pkt);

#ifdef VERIFICATION_MODE
	if (pAd->veri_ctrl.dump_rx_debug) {
		if (rxd_grp2)
			hex_dump("rxd_group2",
				(UCHAR *)rxd_grp2, BMAC_RMAC_INFO_GRP_2_SIZE);

		hex_dump("rx_data", rx_blk->pData, rx_blk->MPDUtotalByteCnt);
	}
#endif

	if (RX_BLK_TEST_FLAG(rx_blk, fRX_HDR_TRANS)) {
		struct wifi_dev *wdev = NULL;

		if (!fc)
			return 0;

		rx_blk->FC = fc;

		temp_fc = *((uint16_t *)fc);
		fn_sn = *((uint16_t *)(fc + 8));
#ifdef CFG_BIG_ENDIAN
		temp_fc = le2cpu16(temp_fc);
		fn_sn = le2cpu16(fn_sn);
#endif
		rx_blk->FN = fn_sn & 0x000f;
		rx_blk->SN = (fn_sn & 0xfff0) >> 4;
		rx_blk->UserPriority =
			HAL_RX_STATUS_GET_QOS_CONTROL_FIELD(rxd_grp0);
		wdev = wdev_search_by_wcid(pAd, rx_blk->wcid);

		if (!wdev)
			wdev = wdev_search_by_band_omac_idx(
				pAd, rx_blk->band, rx_blk->bss_idx);

		if (!wdev)
			return 0;

		if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 0)
			&& (((FRAME_CONTROL *)&temp_fc)->FrDs == 0)) {
			rx_blk->Addr1 = rx_blk->pData;
			rx_blk->Addr2 = rx_blk->pData + 6;
			rx_blk->Addr3 = wdev->bssid;
		} else if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 0)
			&& (((FRAME_CONTROL *)&temp_fc)->FrDs == 1)) {
			rx_blk->Addr1 = rx_blk->pData;
			rx_blk->Addr2 = wdev->bssid;
			rx_blk->Addr3 = rx_blk->pData + 6;
		} else if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 1)
			&& (((FRAME_CONTROL *)&temp_fc)->FrDs == 0)) {
			rx_blk->Addr1 = wdev->bssid;
			rx_blk->Addr2 = rx_blk->pData + 6;
			rx_blk->Addr3 = rx_blk->pData;
		} else {
			rx_blk->Addr1 = wdev->if_addr;
			rx_blk->Addr2 = fc + 2;
			rx_blk->Addr3 = rx_blk->pData;
			rx_blk->Addr4 = rx_blk->pData + 6;
		}
	} else {
		rx_blk->FC = rx_blk->pData;
		if (!rx_blk->FC)
			return 0;

		temp_fc = *((uint16_t *)(rx_blk->FC));
		fn_sn = *((uint16_t *)(rx_blk->FC + 22));
#ifdef CFG_BIG_ENDIAN
		temp_fc = le2cpu16(temp_fc);
		fn_sn = le2cpu16(fn_sn);
#endif
		rx_blk->Duration = *((uint16_t *)(rx_blk->pData + 2));
#ifdef CFG_BIG_ENDIAN
		rx_blk->Duration = le2cpu16(rx_blk->Duration);
#endif

		if ((((FRAME_CONTROL *)&temp_fc)->Type == FC_TYPE_MGMT) ||
			(((FRAME_CONTROL *)&temp_fc)->Type == FC_TYPE_DATA)) {
			rx_blk->FN = fn_sn & 0x000f;
			rx_blk->SN = (fn_sn & 0xfff0) >> 4;
		}

		rx_blk->Addr1 = rx_blk->pData + 4;
		rx_blk->Addr2 = rx_blk->pData + 10;
		rx_blk->Addr3 = rx_blk->pData + 16;


		if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 1)
			&& (((FRAME_CONTROL *)&temp_fc)->FrDs == 1))
			rx_blk->Addr4 = rx_blk->pData + 24;
	}

	if (((FRAME_CONTROL *)&temp_fc)->SubType == SUBTYPE_AUTH) {
		if (HAL_RX_STATUS_GET_SEC_MODE(rxd_grp0) &&
			!HAL_RX_STATUS_GET_CM(rxd_grp0) &&
			!HAL_RX_STATUS_GET_CM(rxd_grp0)) {
			rx_blk->pRxInfo->Decrypted = 1;
		}
	}

	if (rxd_grp1) {
		UINT64 pn1 = HAL_RX_STATUS_GET_PN_63_32_(rxd_grp0);
		UINT64 pn_total = 0;

		if (pn1 != 0)
			pn_total = HAL_RX_STATUS_GET_PN_31_0_(rxd_grp0)
						+ (pn1 << 32);
		else
			pn_total = HAL_RX_STATUS_GET_PN_31_0_(rxd_grp0);

		rx_blk->CCMP_PN = pn_total;
	}

	rx_blk->rcpi[0] = 0;
	rx_blk->rcpi[1] = 0;
	rx_blk->rcpi[2] = 0;
	rx_blk->rcpi[3] = 0;

	for (i = 0; i < MAX_ANTENNA_NUM; i++)
		rx_blk->rx_signal.raw_rssi[i] = RCPI_TO_RSSI(rx_blk->rcpi[i]);

	if (((HAL_RX_STATUS_GET_GROUP_VLD(rxd_grp0)
			& (BMAC_GROUP_VLD_3 | BMAC_GROUP_VLD_5))
		== (BMAC_GROUP_VLD_3 | BMAC_GROUP_VLD_5))
		&& pAd->tr_ctl.en_rx_profiling) {
		UINT8 rx_rate = 0, rx_nsts = 0;
		UINT8 rx_mode = MODE_HE_SU, rx_gi = 0, dbw = 0;
		struct _rx_profiling *rx_rate_rc = NULL;
		struct _rx_mod_cnt *mpdu_cnt = NULL;
		struct _rx_mod_cnt *retry_cnt = NULL;
		UINT8 is_retry = FALSE;

		rx_rate = (rxd_grp3->rxd_20 & BITS(0, 6)) >> 0;
		rx_nsts = (rxd_grp3->rxd_20 & BITS(7, 9)) >> 7;

		dbw = (rxd_grp5->rxd_24 & BITS(8, 10)) >> 8;
		rx_mode = (rxd_grp5->rxd_24 & BITS(4, 7)) >> 4;
		rx_gi = (rxd_grp5->rxd_24 & BITS(13, 14)) >> 13;

		if ((rx_mode < MODE_HE_SU) && (rx_gi > 1)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"[mode(0x%x)]Invalid gi(%d)!\n", rx_mode, rx_gi);
			goto err_out;
		} else if ((rx_mode <= MODE_HE_MU) && (rx_gi > 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"[mode(0x%x)]Invalid gi(%d)!\n", rx_mode, rx_gi);
			goto err_out;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"[mode(0x%x)]vlid band(%d)!\n", rx_mode, rx_gi);
		}

		if (rx_blk->band >= MT7992_WIFI_RAM_BAND_NUM) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"Invalid band(%d)!\n", rx_blk->band);
			goto err_out;
		}

		rx_rate_rc = &pAd->tr_ctl.tr_cnt.rx_rate_rc;
		if (dbw >= ARRAY_SIZE(rx_rate_rc->mpdu_cnt)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
				"Invalid BW(%d)!\n", dbw);
			goto err_out;
		}

		rx_rate_rc->total_mpdu_cnt++;
		mpdu_cnt = &rx_rate_rc->mpdu_cnt[dbw];
		retry_cnt = &rx_rate_rc->retry_cnt[dbw];

		temp_fc = *((uint16_t *)(rx_blk->FC));
#ifdef CFG_BIG_ENDIAN
			temp_fc = le2cpu16(temp_fc);
#endif
		if (((FRAME_CONTROL *)&temp_fc)->Retry) {
			rx_rate_rc->total_retry_cnt++;
			is_retry = TRUE;
		}

		switch (rx_mode) {
		case MODE_CCK:
			rx_rate = rx_rate & BITS(0, 2);

			if (rx_rate < ARRAY_SIZE(mpdu_cnt->cck)) {
				if (rx_rate & BIT2) {
					mpdu_cnt->cck[4+
					((rx_rate & BITS(0, 1))-1)]++;

					if (is_retry)
						retry_cnt->cck[4+
						((rx_rate & BITS(0, 1))-1)]++;
				} else {
					mpdu_cnt->cck[
					(rx_rate & BITS(0, 1))]++;

					if (is_retry)
						retry_cnt->cck[4+
						((rx_rate & BITS(0, 1))-1)]++;
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
					DBG_LVL_INFO,
					"[CCK]Invalid rate idx(%d)!\n",
					rx_rate);
			break;

		case MODE_OFDM:
			switch (rx_rate & BITS(0, 3)) {
			case 0xb:
				rx_rate = 0;
				break;
			case 0xf:
				rx_rate = 1;
				break;
			case 0xa:
				rx_rate = 2;
				break;
			case 0xe:
				rx_rate = 3;
				break;
			case 0x9:
				rx_rate = 4;
				break;
			case 0xd:
				rx_rate = 5;
				break;
			case 0x8:
				rx_rate = 6;
				break;
			case 0xc:
				rx_rate = 7;
				break;
			default:
				rx_rate = 0;
			}

			if (rx_rate < ARRAY_SIZE(mpdu_cnt->ofdm)) {
				mpdu_cnt->ofdm[rx_rate]++;

				if (is_retry)
					retry_cnt->ofdm[rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
					DBG_LVL_INFO,
					"[OFDM]Invalid rate idx(%d)!\n",
					rx_rate);
			break;

		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			rx_rate = (rx_rate & BITS(0, 5));

			if (rx_rate < ARRAY_SIZE(mpdu_cnt->ht)) {
				mpdu_cnt->ht[rx_gi][rx_rate]++;

				if (is_retry)
					retry_cnt->ht[rx_gi][rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
					DBG_LVL_INFO,
					"[HT]Invalid rate idx(%d)!\n",
					rx_rate);
			break;
		case MODE_VHT:
			rx_rate = (rx_rate & BITS(0, 3));

			if ((rx_rate < ARRAY_SIZE(mpdu_cnt->vht[rx_gi][0]))
						&& rx_nsts < 4) {
				mpdu_cnt->vht[rx_gi][rx_nsts][rx_rate]++;

				if (is_retry)
					retry_cnt->vht[rx_gi]
						[rx_nsts][rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
					DBG_LVL_INFO,
					"[VHT]Invalid rate idx(%d)!\n",
					rx_rate);
			break;
		case MODE_HE_SU:
		case MODE_HE_EXT_SU:
		case MODE_HE_TRIG:
		case MODE_HE_MU:
			rx_rate = (rx_rate & BITS(0, 3));

			if ((rx_rate < ARRAY_SIZE(mpdu_cnt->he[rx_gi][0]))
					&& rx_nsts < 4) {
				mpdu_cnt->he[rx_gi][rx_nsts][rx_rate]++;

				if (is_retry)
					retry_cnt->he[rx_gi]
						[rx_nsts][rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
					DBG_LVL_INFO,
					"[HE]Invalid rate idx(%d)!\n",
					rx_rate);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				 "Unknown Mode(%x)!\n", rx_mode);
			break;
		}
	}

err_out:
	return rmac_info_len;
}

#ifdef CONFIG_WLAN_SERVICE
VOID mt7992_testmode_get_chip_info(RTMP_ADAPTER *pAd, struct _chip_testmode_chip_info *ptestmode_chip_info)
{
	static UINT32 sku_type = MT7992_SKU_INVALID;

	if (sku_type == MT7992_SKU_INVALID)
		sku_type = mt7992_get_sku_decision(pAd);

	ptestmode_chip_info->band_used_num = 2;
	if (sku_type == MT7992_SKU_BE5040_MT7976C || sku_type == MT7992_SKU_BE5040_MT7976DA)
		ptestmode_chip_info->u4ChipId = 0x7991;

	ptestmode_chip_info->u1testmode_sup_mlo = FALSE;
}
#endif /* CONFIG_WLAN_SERVICE */

static struct _RTMP_CHIP_OP mt7992_chip_op = { 0 };

static VOID mt7992_chipOp_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_OP *chip_op)
{
	chip_op->AsicBbpInit = BBPInit;
	chip_op->AsicMacInit = init_mac_cr;
	chip_op->AsicReverseRfFromSleepMode = NULL;
	chip_op->AsicHaltAction = NULL;
#ifndef MOBILE_FW_SUPPORT
	chip_op->ChipSwitchChannel = switch_channel;
#endif
#ifdef NEW_SET_RX_STREAM
	chip_op->ChipSetRxStream = set_RxStream;
#endif
	chip_op->AsicAntennaDefaultReset = antenna_default_reset;
	chip_op->GetBandSelectFromEEPROM = mt7992_get_band_select_from_eeprom;

#ifdef CONFIG_STA_SUPPORT
	chip_op->NetDevNickNameInit = init_dev_nick_name;
#endif
#ifdef CAL_FREE_IC_SUPPORT
	chip_op->is_cal_free_ic = is_cal_free_ic;
	chip_op->cal_free_data_get = cal_free_data_get;
#endif /* CAL_FREE_IC_SUPPORT */
	chip_op->DisableTxRx = NULL; /* 302 */
	chip_op->get_sku_tbl_idx = mt7992_get_sku_tbl_idx;
	chip_op->check_RF_lock_down = mt7992_check_RF_lock_down;
	chip_op->Config_Effuse_Country = mt7992_Config_Effuse_Country;
#ifdef MT_WOW_SUPPORT
	chip_op->AsicWOWEnable = MT76xxAndesWOWEnable;
	chip_op->AsicWOWDisable = MT76xxAndesWOWDisable;
	/* chip_op->.AsicWOWInit = MT76xxAndesWOWInit, */
#endif /* MT_WOW_SUPPORT */
	chip_op->show_pwr_info = NULL;
	chip_op->bufferModeCmdFill = bufferModeCmdFill;
	chip_op->MtCmdTx = MtCmdSendMsg;
	chip_op->HeraStbcPriorityCtrl   = CmdHeraStbcPriorityCtrl;
#ifdef TXBF_SUPPORT
	chip_op->iBFPhaseComp			= mt7992_ibf_phase_comp;
	chip_op->iBFPhaseCalInit		= mt7992_ibf_phase_cal_init;
	chip_op->iBFPhaseFreeMem		= mt7992_ibf_phase_free_mem;
	chip_op->iBFPhaseCalE2PUpdate	= mt7992_ibf_phase_cal_e2p_update;
	chip_op->iBFPhaseCalReport		= mt7992_ibf_phase_cal_report;
	chip_op->iBfCaleBfPfmuMemAlloc	= eBFPfmuMemAlloc;
	chip_op->iBfCaliBfPfmuMemAlloc	= iBFPfmuMemAlloc;
	chip_op->TxBFInit				= mt_WrapTxBFInit;
	chip_op->ClientSupportsETxBF	= mt_WrapClientSupportsETxBF;
	chip_op->setETxBFCap			= setETxBFCap;
	chip_op->dump_pfmu_tag			= txbf_dump_tag;
	chip_op->set_txbf_pfmu_tag		= chip_set_txbf_pfmu_tag;
	chip_op->write_txbf_pfmu_tag	= chip_write_txbf_pfmu_tag;
	chip_op->dump_pfmu_data			= txbf_show_pfmu_data;
	chip_op->write_txbf_profile_data  = chip_write_txbf_profile_data;
	chip_op->set_txbf_angle			= chip_set_txbf_angle;
	chip_op->set_txbf_dsnr			= chip_set_txbf_dsnr;
	chip_op->write_txbf_pfmu_data	= chip_write_txbf_pfmu_data;
#ifdef VHT_TXBF_SUPPORT
	chip_op->ClientSupportsVhtETxBF	 = mt_WrapClientSupportsVhtETxBF;
	chip_op->setVHTETxBFCap			 = setVHTETxBFCap;
#endif /* VHT_TXBF_SUPPORT */
#ifdef HE_TXBF_SUPPORT
	chip_op->get_he_etxbf_cap = get_he_etxbf_cap;
#endif /* HE_TXBF_SUPPORT */
#ifdef DOT11_EHT_BE
	chip_op->get_eht_etxbf_cap = get_eht_etxbf_cap;
#endif /* DOT11_EHT_BE */
#endif /* TXBF_SUPPORT */
#ifdef GREENAP_SUPPORT
	chip_op->EnableAPMIMOPS = enable_greenap;
	chip_op->DisableAPMIMOPS = disable_greenap;
#endif /* GREENAP_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	chip_op->ICapStart = MtCmdRfTestICapStart;
	chip_op->ICapStatus = MtCmdRfTestSolicitICapStatus;
	chip_op->ICapCmdUnSolicitRawDataProc = NULL;
	chip_op->ICapCmdSolicitRawDataProc = MtCmdRfTestSolicitICapRawDataProc;
	chip_op->ICapGetIQData = NULL;
	chip_op->ICapEventRawDataHandler = NULL;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	chip_op->SpectrumStart = MtCmdWifiSpectrumStart;
	chip_op->SpectrumStatus = MtCmdWifiSpectrumSolicitCapStatus;
	chip_op->SpectrumCmdRawDataProc = MtCmdWifiSpectrumUnSolicitRawDataProc;
	chip_op->SpectrumEventRawDataHandler = ExtEventWifiSpectrumUnSolicitIQDataHandler;
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef PHY_ICS_SUPPORT
	chip_op->PhyIcsStart = MtCmdPhyIcsStart;
	chip_op->PhyIcsEventEnable = UniCmdPhyIcsEventEnable;
	chip_op->PhyIcsEventRawDataHandler = ExtEventPhyIcsUnSolicitDataHandler;
#endif /* PHY_ICS_SUPPORT */
	chip_op->get_wf_path_comb = mt7992_get_wf_path_comb;
	chip_op->read_chl_pwr = NULL;
	chip_op->get_rx_stat_all = mt7992_get_rx_stat_all;
	chip_op->get_rx_stat = mt7992_get_rx_stat;
#ifdef CONFIG_WLAN_SERVICE
	chip_op->get_chip_capability = mt7992_get_chip_capability;
#endif
	chip_op->rxv_get_byte_cnt = mt7992_rxv_get_byte_cnt;
	chip_op->rxv_get_content = mt7992_rxv_get_content;
	chip_op->rxv_packet_parse = mt7992_rxv_packet_parse;
	chip_op->rxv_entry_parse = mt7992_rxv_entry_parse;
	chip_op->rxv_info_show = mt7992_rxv_info_show;
	chip_op->rxv_raw_data_show = mt7992_rxv_raw_data_show;
	chip_op->rxv_stat_reset = mt7992_rxv_stat_reset;
	chip_op->rxv_cap_init = mt7992_rxv_cap_init;
	chip_op->rxv_dump_start = mt7992_rxv_dump_start;
	chip_op->rxv_dump_stop = mt7992_rxv_dump_stop;
	chip_op->rxv_dump_buf_alloc = mt7992_rxv_dump_buf_alloc;
	chip_op->rxv_dump_buf_clear = mt7992_rxv_dump_buf_clear;
	chip_op->rxv_dump_show_list = mt7992_rxv_dump_show_list;
	chip_op->rxv_dump_show_rpt = mt7992_rxv_dump_show_rpt;
	chip_op->rxv_dump_rxv_content_compose = mt7992_rxv_dump_rxv_content_compose;
	chip_op->rxv_content_len = mt7992_rxv_content_len;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	chip_op->twt_agrt_update = rtmp_twt_agrt_update;
	chip_op->twt_agrt_mgmt = rtmp_twt_agrt_mgmt;
	chip_op->mgmt_frame_offload = rtmp_mgmt_frame_offload;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	chip_op->get_tid_sn = NULL;
	/* per chip sku adjust */
	chip_op->mactbl_wtbl_adjust = mt7992_mactbl_uwtbl_adjust;
	chip_op->hif_io_remap_read32 = pci_io_remap_read32;
	chip_op->hif_io_remap_write32 = pci_io_remap_write32;
	chip_op->fill_key_install_cmd = fill_key_install_cmd_v2;
#ifdef WIFI_UNIFIED_COMMAND
	chip_op->fill_key_install_uni_cmd = fill_key_install_uni_cmd_v2;
	chip_op->fill_key_install_uni_cmd_dynsize_check = fill_key_install_uni_cmd_dynsize_check_v2;
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef CONFIG_WLAN_SERVICE
#ifdef PRE_CAL_MT7992_SUPPORT
	chip_op->ate_group_prek = mt7992_ate_group_prek;
	chip_op->ate_dpd_prek = mt7992_ate_dpd_prek;
	chip_op->ChipCheckPrek = mt7992_apply_dpd_flatness_data;
#endif
#endif


#ifdef CONFIG_TX_DELAY
	chip_op->tx_deley_parm_init = mt7992_tx_deley_parm_init;
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	chip_op->set_off_ch_scan = mt_off_ch_scan_dedicated;
	chip_op->bgnd_scan_cr_init = bgnd_scan_ipi_cr_init;
#endif
#ifdef ERR_RECOVERY
	chip_op->dump_ser_stat = mt7992_dump_ser_stat;
#endif
#ifdef CFG_SUPPORT_FALCON_MURU
	chip_op->check_muru_glo = check_muru_glo;
	chip_op->show_muru_local_data = show_muru_local_data;
	chip_op->show_muru_tx_info    = show_muru_tx_info;
	chip_op->show_muru_shared_data = show_muru_shared_data;
	chip_op->show_muru_mancfg_data = show_muru_mancfg_data;
	chip_op->show_muru_stacap_info = show_muru_stacap_info;
	chip_op->show_mumimo_algorithm_monitor = show_mumimo_algorithm_monitor;
	chip_op->set_mumimo_fixed_rate = set_mumimo_fixed_rate;
	chip_op->set_mumimo_fixed_group_rate  = set_mumimo_fixed_group_rate;
	chip_op->set_mumimo_force_mu_enable = set_mumimo_force_mu_enable;
	chip_op->show_mumimo_group_entry_tbl = show_mumimo_group_entry_tbl;
	chip_op->set_muru_data = set_muru_data;
	chip_op->show_muru_txc_tx_stats = show_muru_txc_tx_stats;
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
	chip_op->show_scs_info = show_scs_info;
#endif /*SMART_CARRIER_SENSE_SUPPORT*/

#if defined(CONFIG_ATE)
	/* offload to fw control
	chip_op->backup_reg_before_ate = mtf_ate_mac_cr_backup_and_set;
	chip_op->restore_reg_after_ate = mtf_ate_mac_cr_restore;
	chip_op->restore_reg_during_ate = mtf_ate_ipg_cr_restore;
	chip_op->set_ifs = mtf_ate_set_ifs_cr;
	chip_op->set_ba_limit = mtf_ate_ampdu_ba_limit;
	chip_op->pause_ac_queue = mtf_ate_set_sta_pause_cr; */
	chip_op->get_testmode_chip_info = mt7992_testmode_get_chip_info;
#endif
	chip_op->sta_per_get = MtCmdGetPER;
	chip_op->rssi_get = mt7992_rssi_get;
	chip_op->ra_init = mt7992_ra_init;
	chip_op->cninfo_get = mt7992_cninfo_get;
	chip_op->txpower_show_info = mtf_txpower_show_info;
	chip_op->update_mib_bucket = mt7992_update_mib_bucket;
#ifdef ZERO_PKT_LOSS_SUPPORT
	chip_op->read_channel_stat_registers = mt7922_read_channel_stat_registers;
#endif
	chip_op->ctrl_rxv_group = mt7992_ctrl_rxv_group;
	chip_op->get_bin_image_file = mt7992_get_default_bin_image_file;
#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT)
	chip_op->get_prek_image_file = mt7992_get_prek_image_file;
#endif
	chip_op->set_mgmt_pkt_txpwr_prctg = mt7992_set_mgmt_pkt_txpwr_prctg;
#ifdef RTMP_EFUSE_SUPPORT
	chip_op->get_efuse_free_blk_bnum = mt7992_get_efuse_free_blk_num;
#endif
#ifdef LED_CONTROL_SUPPORT
	chip_op->wps_led_init = mt7992_wps_led_init;
	chip_op->wps_led_control = mt7992_wps_led_control;
#endif /*LED_CONTROL_SUPPORT*/
#ifdef WIFI_UNIFIED_COMMAND
	chip_op->update_ampdu_raw_counters = mt7992_update_ampdu_raw_counters;
#endif
	chip_op->init_hw_ring_setting = hwifi_init_hw_ring_setting;
	chip_op->set_rro_timeout = mt7992_set_rro_timeout;
	chip_op->set_rro_rss_map = mt7992_set_rro_rss_map;
	chip_op->set_spe_fr_table = mt7992_set_spe_fr_table;
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	chip_op->set_ack_timeout_mode_byband = asic_set_ack_timeout_mode_byband_by_fw;
	chip_op->get_ack_timeout_mode_byband = asic_get_ack_timeout_mode_byband_by_fw;
#endif /* ACK_CTS_TIMEOUT_SUPPORT */
	chip_op->chip_extra_action = mt7992_extra_action;

}

static VOID mt7992_archOp_init(RTMP_ADAPTER *ad, RTMP_ARCH_OP *arch_ops)
{
	arch_ops->archGetChBusyCnt = MtAsicGetChBusyCntByFw;
	arch_ops->archGetCCACnt = MtAsicGetCCACnt;
	arch_ops->archRcpiReset = NULL;
	arch_ops->show_mac_info = mtf_show_mac_info;
	arch_ops->get_wtbl_entry234 = mtf_get_wtbl_entry234;
	arch_ops->init_wtbl = mtf_init_wtbl;
	arch_ops->archAutoFallbackInit = NULL;
	arch_ops->archUpdateProtect = asic_wrap_protinfo_in_bssinfo;
	arch_ops->archUpdateRtsThld = MtAsicUpdateRtsThldByFw;
	arch_ops->archSwitchChannel = MtfAsicSwitchChannel;
	arch_ops->archSetRDG = NULL;
	arch_ops->archSetDevMac = MtAsicSetDevMacByFw;
	arch_ops->archSetBssid = MtAsicSetBssidByFw;
	arch_ops->archSetStaRec = MtAsicSetStaRecByFw;
	arch_ops->asic_rts_on_off = mtf_asic_rts_on_off;
	arch_ops->asic_set_agglimit = mtf_asic_set_agglimit;
#ifdef CONFIG_AP_SUPPORT
	arch_ops->archSetWdevIfAddr = MtAsicSetWdevIfAddr;
	arch_ops->archSetMbssHwCRSetting = MtfDmacSetMbssHwCRSetting;
	arch_ops->archSetExtTTTTHwCRSetting = MtfDmacSetExtTTTTHwCRSetting;
	arch_ops->archSetExtMbssEnableCR = MtfDmacSetExtMbssEnableCR;
#endif /* CONFIG_AP_SUPPORT */
	arch_ops->archDelWcidTab = MtAsicDelWcidTabByFw;
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	arch_ops->archSetWcid4Addr_HdrTrans = MtAsicSetWcid4Addr_HdrTransByFw;
#endif
#ifdef HTC_DECRYPT_IOT
	arch_ops->archSetWcidAAD_OM = MtAsicUpdateStaRecAadOmByFw;
#endif
	arch_ops->archSetWcidPsm = MtAsicUpdateStaRecPsmByFw;
	arch_ops->archAddRemoveKeyTab = MtAsicAddRemoveKeyTabByFw;
#ifdef DOT11V_MBSSID_SUPPORT
	arch_ops->archUpdateBeacon = MtUpdateBcnToMcuV2;
#else
	arch_ops->archUpdateBeacon = MtUpdateBcnToMcu;
#endif
	arch_ops->archGetTsfTime = MtAsicGetTsfTimeByFirmware;
	arch_ops->archGetTsfDiffTime = MtAsicGetTsfDiffTime;
	arch_ops->archSetPreTbtt = NULL;
	arch_ops->archSetGPTimer = MtfAsicSetGPTimer;
	arch_ops->archSetWmmParam = MtAsicSetWmmParam;
#ifdef WIFI_UNIFIED_COMMAND
	arch_ops->archUniCmdSetWmmParam = MtAsicUniCmdSetWmmParam;
#endif /* WIFI_UNIFIED_COMMAND */
	arch_ops->archSetEdcaParm = MtAsicSetEdcaParm;
#ifdef WIFI_UNIFIED_COMMAND
	arch_ops->archUniCmdSetEdcaParm = MtAsicUniCmdSetEdcaParm;
#endif /* WIFI_UNIFIED_COMMAND */
	arch_ops->archSetSlotTime = MtAsicSetSlotTime;
#ifdef WIFI_UNIFIED_COMMAND
	arch_ops->archUniCmdSetSlotTime = MtAsicUniCmdSetSlotTime;
#endif /* WIFI_UNIFIED_COMMAND */
#ifdef ZERO_PKT_LOSS_SUPPORT
	arch_ops->read_channel_stat_registers = mtf_read_channel_stat_registers;
	arch_ops->read_skip_tx = mtf_read_skip_tx;
	arch_ops->update_skip_tx = mtf_update_skip_tx;
#endif /* ZERO_PKT_LOSS_SUPPORT */
	arch_ops->archGetTxTsc = MtAsicGetTxTscByFw;
	arch_ops->archGetRxStat = MtfAsicGetRxStat;
	arch_ops->archSetTmrCal = MtfSetTmrCal;
	arch_ops->archGetHwQFromAc = mtf_get_hwq_from_ac;
	arch_ops->get_nsts_by_mcs = mtf_get_nsts_by_mcs;
	arch_ops->tx_rate_to_tmi_rate = mtf_tx_rate_to_tmi_rate;
	arch_ops->update_raw_counters = mtf_update_raw_counters;
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRxByFw;
	arch_ops->archSetMacMaxLen = NULL;
	arch_ops->archSetTxStream = NULL;
	arch_ops->archSetRxvFilter = MtAsicSetRxvFilter;
#ifdef DOT11_VHT_AC
	arch_ops->archSetRtsSignalTA = mtf_asic_set_rts_signal_ta;
#endif /*  DOT11_VHT_AC */
	arch_ops->archTOPInit = MtfAsicTOPInit;
	arch_ops->archSetTmrCR = MtSetTmrCRByFw;
	arch_ops->archUpdateRxWCIDTable = MtAsicUpdateRxWCIDTableByFw;
	arch_ops->archUpdateBASession = MtAsicUpdateBASessionOffloadByFw;
	arch_ops->archUpdateStaRecBa = MtAsicUpdateStaRecBaByFw;
	arch_ops->archSetSMPS = MtAsicSetSMPSByFw;

	arch_ops->archRxHeaderTransCtl = MtAsicRxHeaderTransCtl;
	arch_ops->archRxHeaderTaranBLCtl = MtAsicRxHeaderTaranBLCtl;
#ifdef VLAN_SUPPORT
	arch_ops->update_vlan_id = mt_asic_update_vlan_id_by_fw;
	arch_ops->update_vlan_priority = mt_asic_update_vlan_priority_by_fw;
#endif
	arch_ops->rx_event_handler = mtf_rx_event_handler;
	arch_ops->trans_rxd_into_rxblk = mt7992_trans_rxd_into_rxblk;
#ifdef IGMP_SNOOP_SUPPORT
#ifdef IGMP_SNOOPING_NON_OFFLOAD
	arch_ops->archMcastEntryInsert = MulticastFilterTableInsertEntry;
	arch_ops->archMcastEntryDelete = MulticastFilterTableDeleteEntry;
#else
	arch_ops->archMcastEntryInsert = HW_WA_MCAST_ENTRY_ADD;
	arch_ops->archMcastEntryDelete = HW_WA_MCAST_ENTRY_DEL;
#endif
#ifdef IGMP_SNOOPING_DENY_LIST
	arch_ops->archMcastEntryDenyList = HW_WA_MCAST_Deny_LIST;
#endif
#endif

#ifdef WFDMA_WED_COMPATIBLE
	arch_ops->asic_wa_update = mtf_wa_cpu_update;
#endif

	arch_ops->write_tmac_info_fixed_rate = mt7992_write_tmac_info_fixed_rate;
#ifdef WF_RESET_SUPPORT
	arch_ops->wf_subsys_reset = mt7992_wf_subsys_reset;
	arch_ops->heart_beat_check = mt7992_heart_beat_check;
#endif
	arch_ops->dump_rmac_info = mtb_dump_rmac_info;
	arch_ops->dump_rx_info = mtb_dump_rxinfo;
	arch_ops->dump_rmac_info_for_icverr = mtb_dump_rmac_info_for_ICVERR;
	arch_ops->dump_txs = mtf_dump_txs;
	arch_ops->rxv_handler = mtf_rxv_handler;
	if (IS_HWIFI_INF(ad)) {
		arch_ops->hw_tx = hwifi_tx_data;
		arch_ops->mlme_hw_tx = hwifi_tx_mgmt;
#ifdef ERR_RECOVERY
		arch_ops->ser_handler = hwifi_ser_handler;
#endif /* ERR_RECOVERY */
	} else
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Not support infType(=%d)\n", ad->infType);
	arch_ops->dump_wtbl_info = mtf_dump_wtbl_info;
	arch_ops->dump_wtbl_base_info = mtf_dump_wtbl_base_info;
	/*hif related*/
	arch_ops->arch_calculate_ecc = mtf_calculate_ecc;
#ifdef TX_POWER_CONTROL_SUPPORT_V2
	arch_ops->arch_txpower_boost = mtb_txpower_boost_v2;
	arch_ops->arch_txpower_boost_ctrl = mtb_txpower_boost_ctrl_v2;
	arch_ops->arch_txpower_boost_info_V2 = mtb_txpower_boost_info_v2;
	arch_ops->arch_txpower_boost_profile = mtb_txpower_boost_profile_v2;
#endif
	arch_ops->arch_txpower_all_rate_info = mtf_txpower_all_rate_info;
#ifdef SINGLE_SKU_V2
	arch_ops->arch_txpower_sku_cfg_para = mtf_txpower_sku_cfg_para;
#endif
	arch_ops->arch_get_bcn_tx_cnt = mt7992_get_mib_bcn_tx_cnt;
#ifdef AIR_MONITOR
	arch_ops->arch_set_air_mon_enable = mtf_set_air_monitor_enable;
	arch_ops->arch_set_air_mon_rule = mtf_set_air_monitor_rule;
	arch_ops->arch_set_air_mon_idx = mtf_set_air_monitor_idx;
#endif
	arch_ops->txd_post_process = mt7992_txd_post_process;
#ifdef DOT11_EHT_BE
	arch_ops->archUpdateDscbInfo = MtAsicUpdateDscbInfo;
	arch_ops->archUpdateEMLOp = mtf_asic_sta_eml_op_update;
	arch_ops->archReqAt2lmRes = mtf_asic_at2lm_res_req;
	arch_ops->archReqNt2lm = mtf_asic_nt2lm_req;
	arch_ops->archReqReconfigRmLink = mtf_asic_reconfig_rm_link_req;
	arch_ops->archSetReconfigTmr = mtf_asic_set_reconfig_tmr;
	arch_ops->archUpdateEmlsrMgmt = ap_mgmt_emlsr_update;
#endif /* DOT11_EHT_BE */
#ifdef TXRX_STAT_SUPPORT
	arch_ops->arch_get_mib_txrx_cnts = mt_asic_get_mib_txrx_cnts;
	arch_ops->arch_get_all_rate_cnts = mt_asic_get_all_rate_cnts;
	arch_ops->arch_get_stbc_cnts = mt_asic_get_stbc_cnts;
	arch_ops->arch_get_gi_cnts = mt_asic_get_gi_cnts;
#endif /* TXRX_STAT_SUPPORT */

}

static VOID mt7992_chipOp_post_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_OP *chip_op)
{
}

static VOID mt7992_hif_ctrl_init(struct _RTMP_ADAPTER *pAd)
{
}

VOID mt7992_init(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ctrl);
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ctrl);
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ctrl);
	uint32_t sku = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "-->\n");
	mt7992_mcs_nss_cap_init(MCS_NSS_CAP(pAd));
	mt7992_chipOp_init(pAd, &mt7992_chip_op);
	hc_register_chip_ops(ctrl, &mt7992_chip_op);
	mt7992_archOp_init(pAd, arch_ops);
	mt7992_chip_dbg_init(chip_dbg);
	mt7992_hif_ctrl_init(pAd);
	mt_phy_probe(pAd);
	mt7992_chipCap_post_init(pAd, pChipCap);
	mt7992_chipOp_post_init(pAd, chip_ops);
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_AGBS);

	sku = mt7992_get_sku_decision(pAd);
	if ((sku == MT7992_SKU_BE7200_MT7975_MT7979) ||
		(sku == MT7992_SKU_BE7200_MT7975_MT7977))
		pAd->CommonCfg.DfsParameter.BW80DedicatedZWSupport = TRUE;
	else
		pAd->CommonCfg.DfsParameter.BW80DedicatedZWSupport = FALSE;

	/* For calibration log buffer size limitation issue */
	pAd->fgQAtoolBatchDumpSupport = TRUE;

#ifdef CONFIG_AP_SUPPORT
	/*VOW CR Address offset - Gen_BESRA*/
	pAd->vow_gen.VOW_GEN = VOW_GEN_BESRA;
#endif /* #ifdef CONFIG_AP_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "<--\n");
}

void mt7992_init_default(void *ph_dev_obj)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;

	MTWF_PRINT("%s()-->\n", __func__);
	mt7992_chipCap_init(ph_dev, &ph_dev->chip_cap);
	ph_dev->edcca_region = EDCCA_REGION_NONE;
	PD_SET_BAND_EN(ph_dev, (BAND0_ENABLED | BAND1_ENABLED | BAND2_ENABLED));
	MTWF_PRINT("<--%s()\n", __func__);
}

#endif /* MT7992 */
