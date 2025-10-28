#ifndef __MT7992_CR_H__
#define __MT7992_CR_H__

/*
	Please don't include this header outside of per chip scope
*/
#include "mt7992_coda/conn_host_csr_top.h"
#include "mt7992_coda/conn_mcu_bus_cr.h"
#include "mt7992_coda/conn_bus_cr_von.h"
#include "mt7992_coda/wf_wfdma_host_dma0.h"
#include "mt7992_coda/wf_wfdma_host_dma0_pcie1.h"
#include "mt7992_coda/wf_wfdma_mcu_dma0.h"
#include "mt7992_coda/wf_wfdma_mem_dma.h"
#include "mt7992_coda/wf_rro_top.h"
#include "mt7992_coda/wf_pse_top.h"
#include "mt7992_coda/wf_ple_top.h"
#include "mt7992_coda/wf_drr_top.h"
#include "mt7992_coda/bn0_wf_agg_top.h"
#include "mt7992_coda/bn1_wf_agg_top.h"
#include "mt7992_coda/bn0_wf_mib_top.h"
#include "mt7992_coda/bn1_wf_mib_top.h"
#include "mt7992_coda/wf_umib_top.h"
#include "mt7992_coda/bn0_wf_arb_top.h"
#include "mt7992_coda/bn0_wf_lpon_top.h"
#include "mt7992_coda/wf_uwtbl_top.h"
#include "mt7992_coda/wf_wtblon_top.h"
#include "mt7992_coda/WF_UWTBL_BESRA_c_header.h"
#include "mt7992_coda/WF_LWTBL_BESRA_c_header.h"
#include "mt7992_coda/bn0_wf_tmac_top.h"
#include "mt7992_coda/bn1_wf_tmac_top.h"
#include "mt7992_coda/bn0_wf_rmac_top.h"
#include "mt7992_coda/bn0_wf_dma_top.h"
#include "mt7992_coda/wf_hif_dmashdl_top.h"
#include "mt7992_coda/WF_TX_DESCRIPTOR_BESRA_c_header.h"
#include "mt7992_coda/conn_dbg_ctl.h"

#define MT_HOST_SET_OWN	(1<<0)
#define MT_HOST_CLR_OWN	(1<<1)

/*
	remap CR
*/
#define HIF_ADDR_L1_REMAP_ADDR	\
	CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_54_cr_pcie2ap_public_remapping_wf_04_ADDR
#define HIF_ADDR_L1_REMAP_MASK	\
	CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_54_cr_pcie2ap_public_remapping_wf_04_MASK
#define HIF_ADDR_L1_REMAP_SHFT	\
	CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_54_cr_pcie2ap_public_remapping_wf_04_SHFT
#define HIF_ADDR_L1_REMAP_BASE_ADDR	0x40000
#define L1_REMAP_OFFSET_MASK (0xffff)
#define GET_L1_REMAP_OFFSET(p) (((p) & L1_REMAP_OFFSET_MASK))
#define L1_REMAP_BASE_MASK (0xffff << 16)
#define GET_L1_REMAP_BASE(p) (((p) & L1_REMAP_BASE_MASK) >> 16)

#define HIF_ADDR_L2_REMAP_ADDR	\
	CONN_MCU_BUS_CR_LEGACY_REMAP_CTRL_1_R_CONNAC_LEGACY_AP2WF_ADDR_MAP1_ADDR
#define HIF_ADDR_L2_REMAP_MASK	\
	CONN_MCU_BUS_CR_LEGACY_REMAP_CTRL_1_R_CONNAC_LEGACY_AP2WF_ADDR_MAP1_MASK
#define HIF_ADDR_L2_REMAP_SHFT	\
	CONN_MCU_BUS_CR_LEGACY_REMAP_CTRL_1_R_CONNAC_LEGACY_AP2WF_ADDR_MAP1_SHFT
#define HIF_ADDR_L2_REMAP_BASE_ADDR	0x01000
#define L2_REMAP_OFFSET_MASK (0xfff)
#define GET_L2_REMAP_OFFSET(p) (((p) & L2_REMAP_OFFSET_MASK))
#define L2_REMAP_BASE_MASK (0xfffff << 12)
#define GET_L2_REMAP_BASE(p) (((p) & L2_REMAP_BASE_MASK) >> 12)

#define CONN_INFRA_PHY_ADDR_START	0x18000000
#define CONN_INFRA_PHY_ADDR_END		0x183fffff
#define WFSYS_PHY_ADDR_START		0x18400000
#define WFSYS_PHY_ADDR_END		0x187fffff
#define BGFSYS_PHY_ADDR_START		0x18800000
#define BGFSYS_PHY_ADDR_END		0x18bfffff
#define CBTOP1_PHY_ADDR_START		0x70000000
#define CBTOP1_PHY_ADDR_END		0x77ffffff
#define CBTOP2_PHY_ADDR_START		0xf0000000
#define CBTOP2_PHY_ADDR_END		0xffffffff

#define CONN_INFRA_MCU_ADDR_START	0x7c000000
#define CONN_INFRA_MCU_ADDR_END		0x7c3fffff
#define CONN_INFRA_MCU_TO_PHY_ADDR_OFFSET \
	(CONN_INFRA_MCU_ADDR_START - CONN_INFRA_PHY_ADDR_START)

#define IS_CONN_INFRA_PHY_ADDR(_reg) \
	((_reg) >= CONN_INFRA_PHY_ADDR_START && (_reg) \
		<= CONN_INFRA_PHY_ADDR_END)
#define IS_WFSYS_PHY_ADDR(_reg) \
	((_reg) >= WFSYS_PHY_ADDR_START && (_reg) <= WFSYS_PHY_ADDR_END)
#define IS_BGFSYS_PHY_ADDR(_reg) \
	((_reg) >= BGFSYS_PHY_ADDR_START && (_reg) <= BGFSYS_PHY_ADDR_END)
#define IS_CBTOP_PHY_ADDR(_reg) \
	(((_reg) >= CBTOP1_PHY_ADDR_START && (_reg) <= CBTOP1_PHY_ADDR_END) || \
	((_reg) >= CBTOP2_PHY_ADDR_START && (_reg) <= CBTOP2_PHY_ADDR_END))
#define IS_PHY_ADDR(_reg) \
	(IS_CONN_INFRA_PHY_ADDR(_reg) || IS_WFSYS_PHY_ADDR(_reg) \
		|| IS_BGFSYS_PHY_ADDR(_reg) || IS_CBTOP_PHY_ADDR(_reg))

#define IS_CONN_INFRA_MCU_ADDR(_reg) \
	((_reg) >= CONN_INFRA_MCU_ADDR_START && (_reg) \
			<= CONN_INFRA_MCU_ADDR_END)

/*
WTBL definition
Copy form BORA hal_wtbl_rom.c
*/

typedef enum _ENUM_WTBL_TYPE_T {
	WTBL_TYPE_LMAC = 0,     /** WTBL in LMAC */
	WTBL_TYPE_UMAC = 1,     /** WTBL in UMAC */
	WTBL_TYPE_KEY = 2,      /** Key Table */
	MAX_NUM_WTBL_TYPE
} ENUM_WTBL_TYPE_T;

typedef enum _ENUM_OWNERSHIP_CR_TYPE_T {
	OWNERSHIP_CR_TYPE_OWN = 0,
	OWNERSHIP_CR_TYPE_OWN_INT_STS = 1,
	OWNERSHIP_CR_TYPE_NUM
} ENUM_OWNERSHIP_CR_TYPE_T;

/* to be removed when E1 phase out */
#define LWTBL_LEN_IN_DW_E1 31

#define LWTBL_LEN_IN_DW 36
#define UWTBL_LEN_IN_DW 10

#define IO_R_32(_addr) io_r_32(pAd, _addr)
#define IO_W_32(_addr, _val) RTMP_IO_WRITE32(pAd->hdev_ctrl, _addr, _val)

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define WIFI_LWTBL_BASE                 WF_WTBLON_TOP_BASE
#define WIFI_UWTBL_BASE                 WF_UWTBL_TOP_BASE


#define CONFIG_WIFI_RAM_HW_WFDMA        1
#define CONFIG_WIFI_RAM_RU_NUM_PARA     32
#define CFG_SUPPORT_MURU                1
#define MAX_RUSCORE_RECORD_NUM          7
#define MAX_TID_NUM                     8
#define CFG_RU_STA_SORTING              1
#define RAM_BAND_NUM                    2
#define CFG_MURU_MIPS_ESTIMATION        0
#define MAX_MU_NUM_PER_PPDU             16
#define MU_ENTRY_MAX                    175
#define MAX_BA_GRP                      2

/***** WTBL(UMAC) *****/

/* UWTBL DW 6 */
#define WTBL_AMSDU_LEN_MASK             BITS(0, 5)
#define WTBL_AMSDU_LEN_OFFSET           0
#define WTBL_AMSDU_NUM_MASK             BITS(6, 10)
#define WTBL_AMSDU_NUM_OFFSET           6
#define WTBL_AMSDU_EN_MASK              BIT(11)
#define WTBL_AMSDU_EN_OFFSET            11

/* UWTBL DW 8 */
#define WTBL_SEC_ADDR_MODE_MASK			BITS(20, 21)
#define WTBL_SEC_ADDR_MODE_OFFSET		20

/* LWTBL Rate field */
#define WTBL_RATE_TX_RATE_MASK          BITS(0, 5)
#define WTBL_RATE_TX_RATE_OFFSET        0
#define WTBL_RATE_TX_MODE_MASK          BITS(6, 9)
#define WTBL_RATE_TX_MODE_OFFSET        6
#define WTBL_RATE_NSTS_MASK             BITS(10, 13)
#define WTBL_RATE_NSTS_OFFSET           10
#define WTBL_RATE_STBC_MASK             BIT(14)
#define WTBL_RATE_STBC_OFFSET           14

/***** WTBL(LMAC) DW Offset *****/
/* LMAC WTBL Group - Peer Unique Information */
#define WTBL_GROUP_PEER_INFO_DW_0               0
#define WTBL_GROUP_PEER_INFO_DW_1               1

/* WTBL Group - TxRx Capability/Information */
#define WTBL_GROUP_TRX_CAP_DW_2                 2
#define WTBL_GROUP_TRX_CAP_DW_3                 3
#define WTBL_GROUP_TRX_CAP_DW_4                 4
#define WTBL_GROUP_TRX_CAP_DW_5                 5
#define WTBL_GROUP_TRX_CAP_DW_6                 6
#define WTBL_GROUP_TRX_CAP_DW_7                 7
#define WTBL_GROUP_TRX_CAP_DW_8                 8
#define WTBL_GROUP_TRX_CAP_DW_9                 9

/* WTBL Group - Auto Rate Table*/
#define WTBL_GROUP_AUTO_RATE_1_2                10
#define WTBL_GROUP_AUTO_RATE_3_4                11
#define WTBL_GROUP_AUTO_RATE_5_6                12
#define WTBL_GROUP_AUTO_RATE_7_8                13

/* WTBL Group - Tx Counter */
#define WTBL_GROUP_TX_CNT_LINE_1                14
#define WTBL_GROUP_TX_CNT_LINE_2                15
#define WTBL_GROUP_TX_CNT_LINE_3                16
#define WTBL_GROUP_TX_CNT_LINE_4                17
#define WTBL_GROUP_TX_CNT_LINE_5                18
#define WTBL_GROUP_TX_CNT_LINE_6                19

/* WTBL Group - Admission Control Counter */
#define WTBL_GROUP_ADM_CNT_LINE_1               20
#define WTBL_GROUP_ADM_CNT_LINE_2               21
#define WTBL_GROUP_ADM_CNT_LINE_3               22
#define WTBL_GROUP_ADM_CNT_LINE_4               23
#define WTBL_GROUP_ADM_CNT_LINE_5               24
#define WTBL_GROUP_ADM_CNT_LINE_6               25
#define WTBL_GROUP_ADM_CNT_LINE_7               26
#define WTBL_GROUP_ADM_CNT_LINE_8               27

/* WTBL Group -MLO Info */
#define WTBL_GROUP_MLO_INFO_LINE_1              28
#define WTBL_GROUP_MLO_INFO_LINE_2              29
#define WTBL_GROUP_MLO_INFO_LINE_3              30

/* WTBL Group -RESP Info */
#define WTBL_GROUP_RESP_INFO_DW_31              31

/* WTBL Group -RX DUP Info */
#define WTBL_GROUP_RX_DUP_INFO_DW_32            32

/* WTBL Group - Rx Statistics Counter */
#define WTBL_GROUP_RX_STAT_CNT_LINE_1           33
#define WTBL_GROUP_RX_STAT_CNT_LINE_2           34
#define WTBL_GROUP_RX_STAT_CNT_LINE_3           35


/* UWTBL Group - HW AMSDU */
#define UWTBL_HW_AMSDU_DW                       WF_UWTBL_AMSDU_CFG_DW

/* LWTBL DW 4 */
#define WTBL_DIS_RHTR                           WF_LWTBL_DIS_RHTR_MASK

/* UWTBL DW 5 */
#define WTBL_KEY_LINK_DW_KEY_LOC0_MASK          BITS(0, 10)
#define WTBL_PSM				WF_LWTBL_PSM_MASK

/* Need to sync with FW define */
#define INVALID_KEY_ENTRY                       WTBL_KEY_LINK_DW_KEY_LOC0_MASK

#define ONE_KEY_ENTRY_LEN_IN_DW                8

/* Key Table operation command value
* (WF_UWTBL_TOP_KTCR_OPERATION_ADDR)
*/
#define UWTBL_TOP_KTCR_OPERATION_DELETE     0
#define UWTBL_TOP_KTCR_OPERATION_ALLOCATE   1
#define UWTBL_TOP_KTCR_OPERATION_SEARCH     2

/***** AMSDU HW Setting *****/
#define MAX_AMSDU_LEN       32
#define MAX_AMSDU_STA_NUM   8

enum {
	FMAC_RXV_GROUP1 = 0,
	FMAC_RXV_GROUP2,
	FMAC_RXV_GROUP3,
	FMAC_RXV_GROUP5,
	FMAC_RXV_GROUP_MAX
};

#define MURU_MAX_PFID_NUM 8
#define MURU_MAX_GROUP_CN 3
#define EXE_IN_INET 0
/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
#ifdef TXBF_SUPPORT
#ifdef CFG_BIG_ENDIAN

union txbf_pfmu_tag1 {
	struct {
		UINT32 reserved0          : 3; /* [31:29]   : Reserved */
		UINT32 invalid_prof       : 1; /* [28]      : 0:default, 1: This profile number is invalid by SW */
		UINT32 ngroup             : 2; /* [27:26]   : Ngroup */
		UINT32 codebook           : 2; /* [25:24]   : Code book */
		UINT32 ncol               : 3; /* [23:21]   : Ncol 3bits for 8x8 */
		UINT32 nrow               : 3; /* [20:18]   : Nrow 3bits for 8x8 */
		UINT32 su_mu              : 1; /* [17]      : 0:SU, 1: MU */
		UINT32 lm                 : 3; /* [16:14]   : 0/1/2/3/4: Legacy/HT/VHT/HE/EHT */
		UINT32 dbw                : 3; /* [13:11]   : 0/1/2/3/4: DW20/40/80/160NC/320 */
		UINT32 txbf               : 1; /* [10]      : 0: iBF, 1: eBF */
		UINT32 profile_id         : 10;/* [9:0]     : 0 ~ 1023 */

		UINT32 mem_addr2_row_id   : 9; /* [63:55]   : row index : 0 ~ 63 */
		UINT32 mem_addr2_col_id   : 7; /* [54:48]   : column index : 0 ~ 5 */
		UINT32 mem_addr1_row_id   : 9; /* [47:39]   : row index : 0 ~ 63 */
		UINT32 mem_addr1_col_id   : 7; /* [38:32]   : column index : 0 ~ 5 */

		UINT32 mem_addr4_row_id   : 9; /* [95:87]   : row index : 0 ~ 63 */
		UINT32 mem_addr4_col_id   : 7; /* [86:80]   : column index : 0 ~ 5 */
		UINT32 mem_addr3_row_id   : 9; /* [79:71]   : row index : 0 ~ 63 */
		UINT32 mem_addr3_col_id   : 7; /* [70:64]   : column index : 0 ~ 5 */

		UINT32 reserved3		  : 3; /* [127:125] : Reserved */
		UINT32 mob_ru_alloc       : 9; /* [124:116] : EHT profile use full 9-bit */
		UINT32 reserved2          : 15;/* [115:113] : Reserved */
		UINT32 mob_cal_en         : 1; /* [112]  : Mobility detection calculation en */
		UINT32 reserved1		  : 7; /* [111:105] : Reserved */
		UINT32 partial_bw_info	  : 9; /* [104:96] : Bitmap, Follow NDP partial BW Info*/

		UINT32 snr_sts3           : 8; /* [159:152] : SNR_STS3 */
		UINT32 snr_sts2           : 8; /* [151:144] : SNR_STS2 */
		UINT32 snr_sts1           : 8; /* [143:136] : SNR_STS1 */
		UINT32 snr_sts0           : 8; /* [135:128] : SNR_STS0 */

		UINT32 snr_sts7           : 8; /* [191:184] : SNR_STS7 */
		UINT32 snr_sts6           : 8; /* [183:176] : SNR_STS6 */
		UINT32 snr_sts5           : 8; /* [175:168] : SNR_STS5 */
		UINT32 snr_sts4           : 8; /* [167:160] : SNR_STS4 */
	} field;

	struct {
		UINT32 reserved0          : 3; /* [31:29]   : Reserved */
		UINT32 invalid_prof       : 1; /* [28]   : 0:default, 1: profile num is invalid */
		UINT32 ngroup             : 2; /* [27:26]   : Ngroup */
		UINT32 codebook           : 2; /* [25:24]   : Code book */
		UINT32 ncol               : 3; /* [23:21]   : Ncol 3bits for 8x8 */
		UINT32 nrow               : 3; /* [20:18]   : Nrow 3bits for 8x8 */
		UINT32 su_mu              : 1; /* [17]      : 0:SU, 1: MU */
		UINT32 lm                 : 3; /* [16:14]   : 0/1/2/3/4: Legacy/HT/VHT/HE/EHT */
		UINT32 dbw                : 3; /* [13:11]   : 0/1/2/3/4: DW20/40/80/160NC/320 */
		UINT32 txbf               : 1; /* [10]      : 0: iBF, 1: eBF */
		UINT32 profile_id         : 10;/* [9:0]     : 0 ~ 1023 */

		UINT32 mem_addr2_row_id   : 9; /* [63:55]   : row index : 0 ~ 63 */
		UINT32 mem_addr2_col_id   : 7; /* [54:48]   : column index : 0 ~ 5 */
		UINT32 mem_addr1_row_id   : 9; /* [47:39]   : row index : 0 ~ 63 */
		UINT32 mem_addr1_col_id   : 7; /* [38:32]   : column index : 0 ~ 5 */

		UINT32 mem_addr4_row_id   : 9; /* [95:87]   : row index : 0 ~ 63 */
		UINT32 mem_addr4_col_id   : 7; /* [86:80]   : column index : 0 ~ 5 */
		UINT32 mem_addr3_row_id   : 9; /* [79:71]   : row index : 0 ~ 63 */
		UINT32 mem_addr3_col_id   : 7; /* [70:64]   : column index : 0 ~ 5 */

		UINT32 reserved4          : 3; /* [127:125] : Reserved */
		UINT32 mob_ru_alloc       : 9; /* [124:116] : EHT profile use full 9-bit */
		UINT32 reserved3          : 3; /* [115:113] : Reserved */
		UINT32 mob_cal_en         : 1; /* [112] : Mobility detection calculation en */
		UINT32 reserved2          : 1; /* [111] : Reserved */
		UINT32 ru_end_id          : 7; /* [110:104] : 0~73, only for HE profile(V matrix) */
		UINT32 reserved1          : 1; /* [103] : Reserved */
		UINT32 ru_start_id        : 7; /* [102:96] : 0~73, only for HE profile (V matrix) */

		UINT32 snr_sts3           : 8; /* [159:152] : SNR_STS3 */
		UINT32 snr_sts2           : 8; /* [151:144] : SNR_STS2 */
		UINT32 snr_sts1           : 8; /* [143:136] : SNR_STS1 */
		UINT32 snr_sts0           : 8; /* [135:128] : SNR_STS0 */

		UINT32 snr_sts7           : 8; /* [191:184] : SNR_STS7 */
		UINT32 snr_sts6           : 8; /* [183:176] : SNR_STS6 */
		UINT32 snr_sts5           : 8; /* [175:168] : SNR_STS5 */
		UINT32 snr_sts4           : 8; /* [167:160] : SNR_STS4 */
	} field_var;
	UINT32 raw_data[7];
};

union txbf_pfmu_tag2 {
	struct {
		UINT32 reserved0      : 3; /* [31:29]  : Reserved */
		UINT32 se_idx         : 5; /* [28:24]  : SE index */
		UINT32 smart_ant      : 24;/* [23:0]   : Smart Ant config */

		UINT32 reserved2      : 8; /* [63:56]  : Reserved */
		UINT32 ibf_timeout    : 8; /* [55:48]  : iBF timeout limit */
		UINT32 reserved1      : 16; /* [47:32]  : Reserved */

		UINT32 reserved3      : 14;/* [95:82]  : Reserved */
		UINT32 ibf_ru         : 9; /* [81:73]  : Desired RX packet RU index(OFDMA data) */
		UINT32 ibf_nrow       : 3; /* [72:70]  : iBF desired Nrow = 1 ~ 8 */
		UINT32 ibf_ncol       : 3; /* [69:67]  : iBF desired Ncol = 1 ~ 8 */
		UINT32 ibf_dbw        : 3; /* [66:64]  : iBF DBW 0/1/2/3/4: BW20/40/80/160NC/320 */

		UINT32 reserved6      : 16;/* [127:112] : Reserved */
		UINT32 reserved5      : 1; /* [111]     : Reserved */
		UINT32 mob_lq_result  : 7; /* [110:104] : Mobility detection calculation result. U1.6 */
		UINT32 mob_delta_t    : 8; /* [103:96]  : Mobility detection delta T value. Resolution: 1ms. Max = 255ms */
	} field;
	UINT32 raw_data[7];
};

union txbf_seg0_angle {
	struct {
		/* DATA 0 */
		UINT32 psi31 : 7;
		UINT32 phi21 : 9;
		UINT32 psi21 : 7;
		UINT32 phi11 : 9;

		/* DATA 1*/
		UINT32 psi51 : 7;
		UINT32 phi41 : 9;
		UINT32 psi41 : 7;
		UINT32 phi31 : 9;

		/* DATA 2*/
		UINT32 psi71 : 7;
		UINT32 phi61 : 9;
		UINT32 psi61 : 7;
		UINT32 phi51 : 9;

		/* DATA 3*/
		UINT32 psi32 : 7;
		UINT32 phi22 : 9;
		UINT32 psi81 : 7;
		UINT32 phi71 : 9;

		/* DATA 4*/
		UINT32 psi52 : 7;
		UINT32 phi42 : 9;
		UINT32 psi42 : 7;
		UINT32 phi32 : 9;

		/* DATA 5*/
		UINT32 psi72 : 7;
		UINT32 phi62 : 9;
		UINT32 psi62 : 7;
		UINT32 phi52 : 9;

		/* DATA 6*/
		UINT32 psi43 : 7;
		UINT32 phi33 : 9;
		UINT32 psi82 : 7;
		UINT32 phi72 : 9;

		/* DATA 7*/
		UINT32 psi63 : 7;
		UINT32 phi53 : 9;
		UINT32 psi53 : 7;
		UINT32 phi43 : 9;

		/* DATA 8*/
		UINT32 psi83 : 7;
		UINT32 phi73 : 9;
		UINT32 psi73 : 7;
		UINT32 phi63 : 9;

		/* DATA 9*/
		UINT32 psi64 : 7;
		UINT32 phi54 : 9;
		UINT32 psi54 : 7;
		UINT32 phi44 : 9;

		/* DATA 10*/
		UINT32 psi84 : 7;
		UINT32 phi74 : 9;
		UINT32 psi74 : 7;
		UINT32 phi64 : 9;

		/* DATA 11*/
		UINT32 psi75 : 7;
		UINT32 phi65 : 9;
		UINT32 psi65 : 7;
		UINT32 phi55 : 9;

		/* DATA 12 */
		UINT32 psi76 : 7;
		UINT32 phi66 : 9;
		UINT32 psi85 : 7;
		UINT32 phi75 : 9;

		/* DATA 13 */
		UINT32 psi87 : 7;
		UINT32 phi77 : 9;
		UINT32 psi86 : 7;
		UINT32 phi76 : 9;
	} field;
	UINT32 raw_data[14];
};

union txbf_seg1_angle {
	struct {
		/* DATA 14 */
		UINT32 psi31 : 7;
		UINT32 phi21 : 9;
		UINT32 psi21 : 7;
		UINT32 phi11 : 9;

		/* DATA 15 */
		UINT32 psi51 : 7;
		UINT32 phi41 : 9;
		UINT32 psi41 : 7;
		UINT32 phi31 : 9;

		/* DATA 16 */
		UINT32 psi71 : 7;
		UINT32 phi61 : 9;
		UINT32 psi61 : 7;
		UINT32 phi51 : 9;

		/* DATA 17 */
		UINT32 psi32 : 7;
		UINT32 phi22 : 9;
		UINT32 psi81 : 7;
		UINT32 phi71 : 9;

		/* DATA 18*/
		UINT32 psi52 : 7;
		UINT32 phi42 : 9;
		UINT32 psi42 : 7;
		UINT32 phi32 : 9;

		/* DATA 19 */
		UINT32 psi72 : 7;
		UINT32 phi62 : 9;
		UINT32 psi62 : 7;
		UINT32 phi52 : 9;

		/* DATA 20 */
		UINT32 psi43 : 7;
		UINT32 phi33 : 9;
		UINT32 psi82 : 7;
		UINT32 phi72 : 9;

		/* DATA 21 */
		UINT32 psi63 : 7;
		UINT32 phi53 : 9;
		UINT32 psi53 : 7;
		UINT32 phi43 : 9;

		/* DATA 22 */
		UINT32 psi83 : 7;
		UINT32 phi73 : 9;
		UINT32 psi73 : 7;
		UINT32 phi63 : 9;

		/* DATA 23 */
		UINT32 psi64 : 7;
		UINT32 phi54 : 9;
		UINT32 psi54 : 7;
		UINT32 phi44 : 9;

		/* DATA 24 */
		UINT32 psi84 : 7;
		UINT32 phi74 : 9;
		UINT32 psi74 : 7;
		UINT32 phi64 : 9;

		/* DATA 25 */
		UINT32 psi75 : 7;
		UINT32 phi65 : 9;
		UINT32 psi65 : 7;
		UINT32 phi55 : 9;

		/* DATA 26 */
		UINT32 psi76 : 7;
		UINT32 phi66 : 9;
		UINT32 psi85 : 7;
		UINT32 phi75 : 9;

		/* DATA 27 */
		UINT32 psi87 : 7;
		UINT32 phi77 : 9;
		UINT32 psi86 : 7;
		UINT32 phi76 : 9;
	} field;
	UINT32 raw_data[14];
};

union txbf_seg2_angle {
	struct {
		/* DATA 28 */
		UINT32 psi31 : 7;
		UINT32 phi21 : 9;
		UINT32 psi21 : 7;
		UINT32 phi11 : 9;

		/* DATA 29*/
		UINT32 psi51 : 7;
		UINT32 phi41 : 9;
		UINT32 psi41 : 7;
		UINT32 phi31 : 9;

		/* DATA 30*/
		UINT32 psi71 : 7;
		UINT32 phi61 : 9;
		UINT32 psi61 : 7;
		UINT32 phi51 : 9;

		/* DATA 31*/
		UINT32 psi32 : 7;
		UINT32 phi22 : 9;
		UINT32 psi81 : 7;
		UINT32 phi71 : 9;

		/* DATA 32*/
		UINT32 psi52 : 7;
		UINT32 phi42 : 9;
		UINT32 psi42 : 7;
		UINT32 phi32 : 9;

		/* DATA 33*/
		UINT32 psi72 : 7;
		UINT32 phi62 : 9;
		UINT32 psi62 : 7;
		UINT32 phi52 : 9;

		/* DATA 34*/
		UINT32 psi43 : 7;
		UINT32 phi33 : 9;
		UINT32 psi82 : 7;
		UINT32 phi72 : 9;

		/* DATA 35*/
		UINT32 psi63 : 7;
		UINT32 phi53 : 9;
		UINT32 psi53 : 7;
		UINT32 phi43 : 9;

		/* DATA 36*/
		UINT32 psi83 : 7;
		UINT32 phi73 : 9;
		UINT32 psi73 : 7;
		UINT32 phi63 : 9;

		/* DATA 37*/
		UINT32 psi64 : 7;
		UINT32 phi54 : 9;
		UINT32 psi54 : 7;
		UINT32 phi44 : 9;

		/* DATA 38*/
		UINT32 psi84 : 7;
		UINT32 phi74 : 9;
		UINT32 psi74 : 7;
		UINT32 phi64 : 9;

		/* DATA 39*/
		UINT32 psi75 : 7;
		UINT32 phi65 : 9;
		UINT32 psi65 : 7;
		UINT32 phi55 : 9;

		/* DATA 40*/
		UINT32 psi76 : 7;
		UINT32 phi66 : 9;
		UINT32 psi85 : 7;
		UINT32 phi75 : 9;

		/* DATA 41*/
		UINT32 psi87 : 7;
		UINT32 phi77 : 9;
		UINT32 psi86 : 7;
		UINT32 phi76 : 9;
	} field;
	UINT32 raw_data[14];
};

union txbf_seg3_angle {
	struct {
		/* DATA 42 */
		UINT32 psi31 : 7;
		UINT32 phi21 : 9;
		UINT32 psi21 : 7;
		UINT32 phi11 : 9;

		/* DATA 43*/
		UINT32 psi51 : 7;
		UINT32 phi41 : 9;
		UINT32 psi41 : 7;
		UINT32 phi31 : 9;

		/* DATA 44*/
		UINT32 psi71 : 7;
		UINT32 phi61 : 9;
		UINT32 psi61 : 7;
		UINT32 phi51 : 9;

		/* DATA 45*/
		UINT32 psi32 : 7;
		UINT32 phi22 : 9;
		UINT32 psi81 : 7;
		UINT32 phi71 : 9;

		/* DATA 46*/
		UINT32 psi52 : 7;
		UINT32 phi42 : 9;
		UINT32 psi42 : 7;
		UINT32 phi32 : 9;

		/* DATA 47*/
		UINT32 psi72 : 7;
		UINT32 phi62 : 9;
		UINT32 psi62 : 7;
		UINT32 phi52 : 9;

		/* DATA 48*/
		UINT32 psi43 : 7;
		UINT32 phi33 : 9;
		UINT32 psi82 : 7;
		UINT32 phi72 : 9;

		/* DATA 49*/
		UINT32 psi63 : 7;
		UINT32 phi53 : 9;
		UINT32 psi53 : 7;
		UINT32 phi43 : 9;

		/* DATA 50*/
		UINT32 psi83 : 7;
		UINT32 phi73 : 9;
		UINT32 psi73 : 7;
		UINT32 phi63 : 9;

		/* DATA 51*/
		UINT32 psi64 : 7;
		UINT32 phi54 : 9;
		UINT32 psi54 : 7;
		UINT32 phi44 : 9;

		/* DATA 52*/
		UINT32 psi84 : 7;
		UINT32 phi74 : 9;
		UINT32 psi74 : 7;
		UINT32 phi64 : 9;

		/* DATA 53*/
		UINT32 psi75 : 7;
		UINT32 phi65 : 9;
		UINT32 psi65 : 7;
		UINT32 phi55 : 9;

		/* DATA 54*/
		UINT32 psi76 : 7;
		UINT32 phi66 : 9;
		UINT32 psi85 : 7;
		UINT32 phi75 : 9;

		/* DATA 55*/
		UINT32 psi87 : 7;
		UINT32 phi77 : 9;
		UINT32 psi86 : 7;
		UINT32 phi76 : 9;
	} field;
	UINT32 raw_data[14];
};

union txbf_bfer_seg0_dsnr {
	struct {
		/* DATA 56*/
		UINT32 dsnr07 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr00 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_seg1_dsnr {
	struct {
		/* DATA 57 */
		UINT32 dsnr07 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr00 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_seg2_dsnr {
	struct {
		/* DATA 58 */
		UINT32 dsnr07 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr00 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_seg3_dsnr {
	struct {
		/* DATA 59 */
		UINT32 dsnr07 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr00 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfee_seg01_dsnr {
	struct {
		/* DATA 56 */
		UINT32 segldsnr03        : 2;
		UINT32 segldsnr02        : 10;
		UINT32 segldsnr01        : 10;
		UINT32 segldsnr00        : 10;

		/* DATA 57 */
		UINT32 segldsnr06        : 4;
		UINT32 segldsnr05        : 10;
		UINT32 segldsnr04        : 10;
		UINT32 segldsnr03_MSB	 : 8;

		/* DATA 58 */
		UINT32 seghdsnr01        : 6;
		UINT32 seghdsnr00        : 10;
		UINT32 segldsnr07        : 10;
		UINT32 segldsnr06_MSB    : 6;

		/* DATA 59 */
		UINT32 seghdsnr04        : 8;
		UINT32 seghdsnr03        : 10;
		UINT32 seghdsnr02        : 10;
		UINT32 seghdsnr01_MSB    : 4;

		/* DATA 60 */
		UINT32 seghdsnr07        : 10;
		UINT32 seghdsnr06        : 10;
		UINT32 seghdsnr05        : 10;
		UINT32 seghdsnr04_MSB    : 2;
	} field;
	UINT32 raw_data[5];
};

union txbf_bfee_seg23_dsnr {
	struct {
		/* DATA 61 */
		UINT32 segldsnr03        : 2;
		UINT32 segldsnr02        : 10;
		UINT32 segldsnr01        : 10;
		UINT32 segldsnr00        : 10;

		/* DATA 62 */
		UINT32 segldsnr06        : 4;
		UINT32 segldsnr05        : 10;
		UINT32 segldsnr04        : 10;
		UINT32 segldsnr03_MSB	 : 8;

		/* DATA 63 */
		UINT32 seghdsnr01        : 6;
		UINT32 seghdsnr00        : 10;
		UINT32 segldsnr07        : 10;
		UINT32 segldsnr06_MSB    : 6;

		/* DATA 64 */
		UINT32 seghdsnr04        : 8;
		UINT32 seghdsnr03        : 10;
		UINT32 seghdsnr02        : 10;
		UINT32 seghdsnr01_MSB    : 4;


		/* DATA 65 */
		UINT32 seghdsnr07        : 10;
		UINT32 seghdsnr06        : 10;
		UINT32 seghdsnr05        : 10;
		UINT32 seghdsnr04_MSB    : 2;
	} field;
	UINT32 raw_data[5];
};

#else
union txbf_pfmu_tag1 {
	struct {
		UINT32 profile_id         : 10;/* [9:0]     : 0 ~ 1023 */
		UINT32 txbf               : 1; /* [10]      : 0: iBF, 1: eBF */
		UINT32 dbw                : 3; /* [13:11]   : 0/1/2/3: DW20/40/80/160NC/320 */
		UINT32 lm                 : 3; /* [16:14]   : 0/1/2/3: Legacy/HT/VHT/HE/EHT */
		UINT32 su_mu              : 1; /* [17]      : 0:SU, 1: MU */
		UINT32 nrow               : 3; /* [20:18]   : Nrow 3bits for 8x8 */
		UINT32 ncol               : 3; /* [23:21]   : Ncol 3bits for 8x8 */
		UINT32 codebook           : 2; /* [25:24]   : Code book */
		UINT32 ngroup             : 2; /* [27:26]   : Ngroup */
		UINT32 invalid_prof       : 1; /* [28]      : 0:default, 1: This profile number is invalid by SW */
		UINT32 reserved0          : 3; /* [31:29]   : Reserved */

		UINT32 mem_addr1_col_id   : 7; /* [38:32]   : column index : 0 ~ 5 */
		UINT32 mem_addr1_row_id   : 9; /* [47:39]   : row index : 0 ~ 63 */
		UINT32 mem_addr2_col_id   : 7; /* [54:48]   : column index : 0 ~ 5 */
		UINT32 mem_addr2_row_id   : 9; /* [63:55]   : row index : 0 ~ 63 */

		UINT32 mem_addr3_col_id   : 7; /* [70:64]   : column index : 0 ~ 5 */
		UINT32 mem_addr3_row_id   : 9; /* [79:71]   : row index : 0 ~ 63 */
		UINT32 mem_addr4_col_id   : 7; /* [86:80]   : column index : 0 ~ 5 */
		UINT32 mem_addr4_row_id   : 9; /* [95:87]   : row index : 0 ~ 63 */
		UINT32 partial_bw_info	  : 9; /*[104:96] : Bitmap, Follow NDP partial BW Info*/

		UINT32 reserved1          : 7; /* [111:105] : Reserved */
		UINT32 mob_cal_en         : 1; /* [112]  : Mobility detection calculation enable */
		UINT32 reserved2          : 3; /* [115:113] : Reserved */
		UINT32 mob_ru_alloc       : 9; /* [124:116] : EHT profile use full 9-bit */
		UINT32 reserved3          : 3; /* [127:125] : Reserved */

		UINT32 snr_sts0           : 8; /* [135:128] : SNR_STS0 */
		UINT32 snr_sts1           : 8; /* [143:136] : SNR_STS1 */
		UINT32 snr_sts2           : 8; /* [151:144] : SNR_STS2 */
		UINT32 snr_sts3           : 8; /* [159:152] : SNR_STS3 */

		UINT32 snr_sts4           : 8; /* [167:160] : SNR_STS4 */
		UINT32 snr_sts5           : 8; /* [175:168] : SNR_STS5 */
		UINT32 snr_sts6           : 8; /* [183:176] : SNR_STS6 */
		UINT32 snr_sts7           : 8; /* [191:184] : SNR_STS7 */
	} field;

	struct {
		UINT32 profile_id         : 10;/* [9:0]     : 0 ~ 1023 */
		UINT32 txbf               : 1; /* [10]      : 0: iBF, 1: eBF */
		UINT32 dbw                : 3; /* [13:11]   : 0/1/2/3: DW20/40/80/160NC/320 */
		UINT32 lm                 : 3; /* [16:14]   : 0/1/2/3: Legacy/HT/VHT/HE/EHT */
		UINT32 su_mu              : 1; /* [17]      : 0:SU, 1: MU */
		UINT32 nrow               : 3; /* [20:18]   : Nrow 3bits for 8x8 */
		UINT32 ncol               : 3; /* [23:21]   : Ncol 3bits for 8x8 */
		UINT32 codebook           : 2; /* [25:24]   : Code book */
		UINT32 ngroup             : 2; /* [27:26]   : Ngroup */
		UINT32 invalid_prof       : 1; /* [28]  : 1: This profile number is invalid by SW */
		UINT32 reserved0          : 3; /* [31:29]   : Reserved */

		UINT32 mem_addr1_col_id   : 7; /* [38:32]   : column index : 0 ~ 5 */
		UINT32 mem_addr1_row_id   : 9; /* [47:39]   : row index : 0 ~ 63 */
		UINT32 mem_addr2_col_id   : 7; /* [54:48]   : column index : 0 ~ 5 */
		UINT32 mem_addr2_row_id   : 9; /* [63:55]   : row index : 0 ~ 63 */

		UINT32 mem_addr3_col_id   : 7; /* [70:64]   : column index : 0 ~ 5 */
		UINT32 mem_addr3_row_id   : 9; /* [79:71]   : row index : 0 ~ 63 */
		UINT32 mem_addr4_col_id   : 7; /* [86:80]   : column index : 0 ~ 5 */
		UINT32 mem_addr4_row_id   : 9; /* [95:87]   : row index : 0 ~ 63 */

		UINT32 ru_start_id        : 7; /* [102:96] : 0~73, only for HE profile (V matrix) */
		UINT32 reserved1          : 1; /* [103] : Reserved */
		UINT32 ru_end_id          : 7; /*  [110:104] : 0~73,only for HE profile(V matrix) */
		UINT32 reserved2          : 1; /* [111]   : Reserved */
		UINT32 mob_cal_en         : 1; /* [112]   : Mobility detection calculation enable */
		UINT32 reserved3          : 3; /* [115:113] : Reserved */
		UINT32 mob_ru_alloc       : 9; /* [124:116] : EHT profile use full 9-bit */
		UINT32 reserved4          : 3; /* [127:125] : Reserved */

		UINT32 snr_sts0           : 8; /* [135:128] : SNR_STS0 */
		UINT32 snr_sts1           : 8; /* [143:136] : SNR_STS1 */
		UINT32 snr_sts2           : 8; /* [151:144] : SNR_STS2 */
		UINT32 snr_sts3           : 8; /* [159:152] : SNR_STS3 */

		UINT32 snr_sts4           : 8; /* [167:160] : SNR_STS4 */
		UINT32 snr_sts5           : 8; /* [175:168] : SNR_STS5 */
		UINT32 snr_sts6           : 8; /* [183:176] : SNR_STS6 */
		UINT32 snr_sts7           : 8; /* [191:184] : SNR_STS7 */
	} field_var;
	UINT32 raw_data[7];
};

union txbf_pfmu_tag2 {
	struct {
		UINT32 smart_ant      : 24;/* [23:0]   : Smart Ant config */
		UINT32 se_idx         : 5; /* [28:24]  : SE index */
		UINT32 reserved0      : 3; /* [31:29]  : Reserved */

		UINT32 reserved1      : 16;/* [47:32]  : Reserved */
		UINT32 ibf_timeout    : 8; /* [55:48]  : iBF timeout limit */
		UINT32 reserved2      : 8; /* [63:56]  : Reserved */

		UINT32 ibf_dbw        : 3; /* [66:64]  : iBF DBW 0/1/2/3/4: BW20/40/80/160NC/320 */
		UINT32 ibf_ncol       : 3; /* [69:67]  : iBF desired Ncol = 1 ~ 8 */
		UINT32 ibf_nrow       : 3; /* [72:70]  : iBF desired Nrow = 1 ~ 8 */

#ifdef IBF_BITMAP_SUPPORT
		UINT32 ibf_bw_bitmap  : 16;/* [88:73]  : Desired BW bitmap (MRU data), each bit
represents BW20 */
		UINT32 reserved3      : 7; /* [95:89]  : Reserved */
#else
		UINT32 ibf_ru         : 9; /* [81:73]  : Desired RX packet RU index(OFDMA data) */
		UINT32 reserved3      : 14;/* [95:82]  : Reserved */
#endif

		UINT32 mob_delta_t    : 8; /* [103:96]	: Mobility detection delta T value. Resolution: 1ms. Max = 255ms */
		UINT32 mob_lq_result  : 7; /* [110:104] : Mobility detection calculation result. U1.6 */
		UINT32 reserved4      : 1; /* [111]	: Reserved */
		UINT32 reserved5      : 16;/* [127:112] : Reserved */
	} field;
	UINT32 raw_data[7];
};

union txbf_seg0_angle {
	struct {
		/* DATA 0 */
		UINT32 phi11 : 9;
		UINT32 psi21 : 7;
		UINT32 phi21 : 9;
		UINT32 psi31 : 7;

		/* DATA 1*/
		UINT32 phi31 : 9;
		UINT32 psi41 : 7;
		UINT32 phi41 : 9;
		UINT32 psi51 : 7;

		/* DATA 2*/
		UINT32 phi51 : 9;
		UINT32 psi61 : 7;
		UINT32 phi61 : 9;
		UINT32 psi71 : 7;

		/* DATA 3*/
		UINT32 phi71 : 9;
		UINT32 psi81 : 7;
		UINT32 phi22 : 9;
		UINT32 psi32 : 7;

		/* DATA 4*/
		UINT32 phi32 : 9;
		UINT32 psi42 : 7;
		UINT32 phi42 : 9;
		UINT32 psi52 : 7;

		/* DATA 5*/
		UINT32 phi52 : 9;
		UINT32 psi62 : 7;
		UINT32 phi62 : 9;
		UINT32 psi72 : 7;

		/* DATA 6*/
		UINT32 phi72 : 9;
		UINT32 psi82 : 7;
		UINT32 phi33 : 9;
		UINT32 psi43 : 7;

		/* DATA 7*/
		UINT32 phi43 : 9;
		UINT32 psi53 : 7;
		UINT32 phi53 : 9;
		UINT32 psi63 : 7;

		/* DATA 8*/
		UINT32 phi63 : 9;
		UINT32 psi73 : 7;
		UINT32 phi73 : 9;
		UINT32 psi83 : 7;

		/* DATA 9*/
		UINT32 phi44 : 9;
		UINT32 psi54 : 7;
		UINT32 phi54 : 9;
		UINT32 psi64 : 7;

		/* DATA 10*/
		UINT32 phi64 : 9;
		UINT32 psi74 : 7;
		UINT32 phi74 : 9;
		UINT32 psi84 : 7;

		/* DATA 11*/
		UINT32 phi55 : 9;
		UINT32 psi65 : 7;
		UINT32 phi65 : 9;
		UINT32 psi75 : 7;

		/* DATA 12*/
		UINT32 phi75 : 9;
		UINT32 psi85 : 7;
		UINT32 phi66 : 9;
		UINT32 psi76 : 7;

		/* DATA 13*/
		UINT32 phi76 : 9;
		UINT32 psi86 : 7;
		UINT32 phi77 : 9;
		UINT32 psi87 : 7;
	} field;
	UINT32 raw_data[14];
};

union txbf_seg1_angle {
	struct {
		/* DATA 14 */
		UINT32 phi11 : 9;
		UINT32 psi21 : 7;
		UINT32 phi21 : 9;
		UINT32 psi31 : 7;

		/* DATA 15*/
		UINT32 phi31 : 9;
		UINT32 psi41 : 7;
		UINT32 phi41 : 9;
		UINT32 psi51 : 7;

		/* DATA 16*/
		UINT32 phi51 : 9;
		UINT32 psi61 : 7;
		UINT32 phi61 : 9;
		UINT32 psi71 : 7;

		/* DATA 17*/
		UINT32 phi71 : 9;
		UINT32 psi81 : 7;
		UINT32 phi22 : 9;
		UINT32 psi32 : 7;

		/* DATA 18*/
		UINT32 phi32 : 9;
		UINT32 psi42 : 7;
		UINT32 phi42 : 9;
		UINT32 psi52 : 7;

		/* DATA 19*/
		UINT32 phi52 : 9;
		UINT32 psi62 : 7;
		UINT32 phi62 : 9;
		UINT32 psi72 : 7;

		/* DATA 20*/
		UINT32 phi72 : 9;
		UINT32 psi82 : 7;
		UINT32 phi33 : 9;
		UINT32 psi43 : 7;

		/* DATA 21*/
		UINT32 phi43 : 9;
		UINT32 psi53 : 7;
		UINT32 phi53 : 9;
		UINT32 psi63 : 7;

		/* DATA 22*/
		UINT32 phi63 : 9;
		UINT32 psi73 : 7;
		UINT32 phi73 : 9;
		UINT32 psi83 : 7;

		/* DATA 23*/
		UINT32 phi44 : 9;
		UINT32 psi54 : 7;
		UINT32 phi54 : 9;
		UINT32 psi64 : 7;

		/* DATA 24*/
		UINT32 phi64 : 9;
		UINT32 psi74 : 7;
		UINT32 phi74 : 9;
		UINT32 psi84 : 7;

		/* DATA 25*/
		UINT32 phi55 : 9;
		UINT32 psi65 : 7;
		UINT32 phi65 : 9;
		UINT32 psi75 : 7;

		/* DATA 26*/
		UINT32 phi75 : 9;
		UINT32 psi85 : 7;
		UINT32 phi66 : 9;
		UINT32 psi76 : 7;

		/* DATA 27*/
		UINT32 phi76 : 9;
		UINT32 psi86 : 7;
		UINT32 phi77 : 9;
		UINT32 psi87 : 7;
	} field;
	UINT32 raw_data[14];
};

union txbf_seg2_angle {
	struct {
		/* DATA 28 */
		UINT32 phi11 : 9;
		UINT32 psi21 : 7;
		UINT32 phi21 : 9;
		UINT32 psi31 : 7;

		/* DATA 29*/
		UINT32 phi31 : 9;
		UINT32 psi41 : 7;
		UINT32 phi41 : 9;
		UINT32 psi51 : 7;

		/* DATA 30*/
		UINT32 phi51 : 9;
		UINT32 psi61 : 7;
		UINT32 phi61 : 9;
		UINT32 psi71 : 7;

		/* DATA 31*/
		UINT32 phi71 : 9;
		UINT32 psi81 : 7;
		UINT32 phi22 : 9;
		UINT32 psi32 : 7;

		/* DATA 32*/
		UINT32 phi32 : 9;
		UINT32 psi42 : 7;
		UINT32 phi42 : 9;
		UINT32 psi52 : 7;

		/* DATA 33*/
		UINT32 phi52 : 9;
		UINT32 psi62 : 7;
		UINT32 phi62 : 9;
		UINT32 psi72 : 7;

		/* DATA 34*/
		UINT32 phi72 : 9;
		UINT32 psi82 : 7;
		UINT32 phi33 : 9;
		UINT32 psi43 : 7;

		/* DATA 35*/
		UINT32 phi43 : 9;
		UINT32 psi53 : 7;
		UINT32 phi53 : 9;
		UINT32 psi63 : 7;

		/* DATA 36*/
		UINT32 phi63 : 9;
		UINT32 psi73 : 7;
		UINT32 phi73 : 9;
		UINT32 psi83 : 7;

		/* DATA 37*/
		UINT32 phi44 : 9;
		UINT32 psi54 : 7;
		UINT32 phi54 : 9;
		UINT32 psi64 : 7;

		/* DATA 38*/
		UINT32 phi64 : 9;
		UINT32 psi74 : 7;
		UINT32 phi74 : 9;
		UINT32 psi84 : 7;

		/* DATA 39*/
		UINT32 phi55 : 9;
		UINT32 psi65 : 7;
		UINT32 phi65 : 9;
		UINT32 psi75 : 7;

		/* DATA 40*/
		UINT32 phi75 : 9;
		UINT32 psi85 : 7;
		UINT32 phi66 : 9;
		UINT32 psi76 : 7;

		/* DATA 41*/
		UINT32 phi76 : 9;
		UINT32 psi86 : 7;
		UINT32 phi77 : 9;
		UINT32 psi87 : 7;
	} field;
	UINT32 raw_data[14];
};

union txbf_seg3_angle {
	struct {
		/* DATA 42 */
		UINT32 phi11 : 9;
		UINT32 psi21 : 7;
		UINT32 phi21 : 9;
		UINT32 psi31 : 7;

		/* DATA 43*/
		UINT32 phi31 : 9;
		UINT32 psi41 : 7;
		UINT32 phi41 : 9;
		UINT32 psi51 : 7;

		/* DATA 44*/
		UINT32 phi51 : 9;
		UINT32 psi61 : 7;
		UINT32 phi61 : 9;
		UINT32 psi71 : 7;

		/* DATA 45*/
		UINT32 phi71 : 9;
		UINT32 psi81 : 7;
		UINT32 phi22 : 9;
		UINT32 psi32 : 7;

		/* DATA 46*/
		UINT32 phi32 : 9;
		UINT32 psi42 : 7;
		UINT32 phi42 : 9;
		UINT32 psi52 : 7;

		/* DATA 47*/
		UINT32 phi52 : 9;
		UINT32 psi62 : 7;
		UINT32 phi62 : 9;
		UINT32 psi72 : 7;

		/* DATA 48*/
		UINT32 phi72 : 9;
		UINT32 psi82 : 7;
		UINT32 phi33 : 9;
		UINT32 psi43 : 7;

		/* DATA 49*/
		UINT32 phi43 : 9;
		UINT32 psi53 : 7;
		UINT32 phi53 : 9;
		UINT32 psi63 : 7;

		/* DATA 50*/
		UINT32 phi63 : 9;
		UINT32 psi73 : 7;
		UINT32 phi73 : 9;
		UINT32 psi83 : 7;

		/* DATA 51*/
		UINT32 phi44 : 9;
		UINT32 psi54 : 7;
		UINT32 phi54 : 9;
		UINT32 psi64 : 7;

		/* DATA 52*/
		UINT32 phi64 : 9;
		UINT32 psi74 : 7;
		UINT32 phi74 : 9;
		UINT32 psi84 : 7;

		/* DATA 53*/
		UINT32 phi55 : 9;
		UINT32 psi65 : 7;
		UINT32 phi65 : 9;
		UINT32 psi75 : 7;

		/* DATA 54*/
		UINT32 phi75 : 9;
		UINT32 psi85 : 7;
		UINT32 phi66 : 9;
		UINT32 psi76 : 7;

		/* DATA 55*/
		UINT32 phi76 : 9;
		UINT32 psi86 : 7;
		UINT32 phi77 : 9;
		UINT32 psi87 : 7;
	} field;
	UINT32 raw_data[14];
};

union txbf_bfer_seg0_dsnr {
	struct {
		/* DATA 56*/
		UINT32 dsnr00 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr07 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_seg1_dsnr {
	struct {
		/* DATA 57 */
		UINT32 dsnr00 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr07 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_seg2_dsnr {
	struct {
		/* DATA 58 */
		UINT32 dsnr00 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr07 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_seg3_dsnr {
	struct {
		/* DATA 59 */
		UINT32 dsnr00 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr07 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfee_seg01_dsnr {
	struct {
		/* DATA 56 */
		UINT32 segldsnr00        : 10;
		UINT32 segldsnr01        : 10;
		UINT32 segldsnr02        : 10;
		UINT32 segldsnr03        : 2;

		/* DATA 57 */
		UINT32 segldsnr03_MSB    : 8;
		UINT32 segldsnr04        : 10;
		UINT32 segldsnr05        : 10;
		UINT32 segldsnr06        : 4;

		/* DATA 58 */
		UINT32 segldsnr06_MSB    : 6;
		UINT32 segldsnr07        : 10;
		UINT32 seghdsnr00        : 10;
		UINT32 seghdsnr01        : 6;

		/* DATA 59 */
		UINT32 seghdsnr01_MSB    : 4;
		UINT32 seghdsnr02        : 10;
		UINT32 seghdsnr03        : 10;
		UINT32 seghdsnr04        : 8;

		/* DATA 60 */
		UINT32 seghdsnr04_MSB    : 2;
		UINT32 seghdsnr05        : 10;
		UINT32 seghdsnr06        : 10;
		UINT32 seghdsnr07        : 10;
	} field;
	UINT32 raw_data[5];
};

union txbf_bfee_seg23_dsnr {
	struct {
		/* DATA 61 */
		UINT32 segldsnr00        : 10;
		UINT32 segldsnr01        : 10;
		UINT32 segldsnr02        : 10;
		UINT32 segldsnr03        : 2;

		/* DATA 62 */
		UINT32 segldsnr03_MSB    : 8;
		UINT32 segldsnr04        : 10;
		UINT32 segldsnr05        : 10;
		UINT32 segldsnr06        : 4;

		/* DATA 63 */
		UINT32 segldsnr06_MSB    : 6;
		UINT32 segldsnr07        : 10;
		UINT32 seghdsnr00        : 10;
		UINT32 seghdsnr01        : 6;

		/* DATA 64 */
		UINT32 seghdsnr01_MSB    : 4;
		UINT32 seghdsnr02        : 10;
		UINT32 seghdsnr03        : 10;
		UINT32 seghdsnr04        : 8;

		/* DATA 65 */
		UINT32 seghdsnr04_MSB    : 2;
		UINT32 seghdsnr05        : 10;
		UINT32 seghdsnr06        : 10;
		UINT32 seghdsnr07        : 10;
	} field;
	UINT32 raw_data[5];
};
#endif

union txbf_bfer_pfmu_data {
	struct {
		union txbf_seg0_angle rSeg0Ang;
		union txbf_seg1_angle rSeg1Ang;
		union txbf_seg2_angle rSeg2Ang;
		union txbf_seg3_angle rSeg3Ang;
		union txbf_bfer_seg0_dsnr rSeg0Snr;
		union txbf_bfer_seg1_dsnr rSeg1Snr;
		union txbf_bfer_seg2_dsnr rSeg2Snr;
		union txbf_bfer_seg3_dsnr rSeg3Snr;
	} field;
	UINT32 raw_data[66];
};

union txbf_bfee_pfmu_data {
	struct {
		union txbf_seg0_angle rSeg0Ang;
		union txbf_seg1_angle rSeg1Ang;
		union txbf_seg2_angle rSeg2Ang;
		union txbf_seg3_angle rSeg3Ang;
		union txbf_bfee_seg01_dsnr rSeg01Snr;
		union txbf_bfee_seg23_dsnr rSeg23Snr;
	} field;
	UINT32 raw_data[66];
};
#endif /* TXBF_SUPPORT */

union hetb_rx_cmm {
	struct {
		ULONGLONG tigger_type:4;
		ULONGLONG ul_length:12;
		ULONGLONG cascade_ind:1;
		ULONGLONG cs_required:1;
		ULONGLONG ul_bw:2;
		ULONGLONG gi_ltf:2;
		ULONGLONG mimo_ltf:1;
		ULONGLONG ltf_sym_midiam:3;
		ULONGLONG stbc:1;
		ULONGLONG ldpc_extra_sym:1;
		ULONGLONG ap_tx_pwr:6;
		ULONGLONG t_pe:3;
		ULONGLONG spt_reuse:16;
		ULONGLONG doppler:1;
		ULONGLONG sig_a_reserved:9;
		ULONGLONG reserved:1;
	} field;
	ULONGLONG cmm_info;
};

union hetb_rx_usr {
	struct {
		UINT32 uid:8;
		UINT32 non_sf_enb:1;
		UINT32 nss:3;
		UINT32 allocation:8;
		UINT32 coding:1;
		UINT32 mcs:4;
		UINT32 dcm:1;
		UINT32 ss_allocation:6;
	} field;
	UINT32 usr_info;
};

union hetb_tx_usr {
	struct {
		UINT32 aid:12;
		UINT32 allocation:8;
		UINT32 coding:1;
		UINT32 mcs:4;
		UINT32 dcm:1;
		UINT32 ss_allocation:6;
	} field;
	UINT32 usr_info;
};

/* MURU local data - _rMuru_Local_Data */
#ifndef OFFSET_OF
#define OFFSET_OF(_type, _field)    ((size_t)(&((_type *)0)->_field))
#endif

#define CONFIG_WIFI_RAM_MURU_MAX_USER_PPDU	16
#define MAX_USER_IN_PPDU        (CONFIG_WIFI_RAM_MURU_MAX_USER_PPDU)

#define MAX_DATA_AC_NUM                 4 /*DL + UL Data*/

typedef enum _ENUM_MURU_AC_NUM_T {
	MURU_AC_NUM_0 = 0,
	MURU_AC_NUM_1 = 1,
	MURU_AC_NUM_2 = 2,
	MURU_AC_NUM_3 = 3,
	MURU_AC_NUM_MAX = 4
} ENUM_MURU_AC_NUM_T, *P_ENUM_MURU_AC_NUM_T;

typedef enum _MURU_DUSCH_WEIGHT {
	MURU_DUSCH_WEIGHT_0_1 = 0x0,
	MURU_DUSCH_WEIGHT_1_4 = 0x1,
	MURU_DUSCH_WEIGHT_2_1 = 0x2,
	MURU_DUSCH_WEIGHT_2_2 = 0x3,
	MURU_DUSCH_WEIGHT_1_2 = 0x4,
	MURU_DUSCH_WEIGHT_4_1 = 0x5,
	MURU_DUSCH_WEIGHT_1_0 = 0x6,
	MURU_DUSCH_WEIGHT_1_1 = 0x7,
	MURU_DUSCH_WEIGHT_16_1 = 0x8,
	MURU_DUSCH_WEIGHT_3_1,
	MURU_DUSCH_WEIGHT_5_1,
	MURU_DUSCH_WEIGHT_6_1,
	MURU_DUSCH_WEIGHT_7_1,
	MURU_DUSCH_WEIGHT_NULL
} MURU_DUSCH_WEIGHT_T;

typedef enum _ENUM_MUM_GRP_USR_CAP_T {
	MUM_GRP_USR_VHT_CAP = 0x0,
	MUM_GRP_USR_HE_DLFUMUM_CAP = 0x1,
	MUM_GRP_USR_HE_DLPBMUM_CAP = 0x2,
	MUM_GRP_USR_HE_ULFBMUM_CAP = 0x3,
	MUM_GRP_USR_HE_ULPBMUM_CAP = 0x4,
	MUM_MAX_GRP_USR_CAP        = 0x5
} ENUM_MUM_GRP_USR_CAP_T, *P_ENUM_MUM_GRP_USR_CAP_T;

typedef struct _LINK_T {
	UINT_32 prNext;			/* Set Host 8B pointer to 4B */
	UINT_32 prPrev;			/* Set Host 8B pointer to 4B */
	UINT_32 u4NumElem;
} LINK_T, *P_LINK_T;

typedef struct _MURU_TPC_MAN_PARA_T {
	INT_8 ai1ManTargetRssi[MAX_USER_IN_PPDU];
} MURU_TPC_MAN_PARA_T, P_MURU_TPC_MAN_PARA_T;

typedef struct _MURU_PARA_T {
	uint8_t fgPingPongAlgo;
	uint8_t fgSu;
	uint8_t fg256BitMap;
	uint8_t fgUlBsrp;
	uint8_t fgTxcmdsnd;
	uint8_t fgTpc;
	uint8_t fgSrTxcmd;
	uint8_t fgTpcManualMode;
	UINT_16 u2fixedTPNum;
	UINT_8  u1UlMpduCntPolicy;
	UINT_8  u1DelayPolicy;
	MURU_TPC_MAN_PARA_T rTpcManPara;
	uint8_t fgTpcOptMode;
	UINT_8  u1TxCmdQLen[MAX_DATA_AC_NUM];
	uint8_t fgTBSuAdaptiveLSIGLen;
	uint8_t fgSRState;
	UINT_8 u1TypeCDelayReq;
	UINT_32 u4BsrTruncateThr;
	UINT_16 u2MaxStaCntLimit;
	uint8_t fgPreGrp;
	uint8_t fgTxopBurst;
	INT_16 i2PsdDiffThr;
	UINT_8 u1SplPriority;
	UINT_8 u1DlSolictAckPolicy;
	UINT_8 u1MaxRuCandidateNum;
	UINT_8 u1Resv[3];
} MURU_PARA_T, *P_MURU_PARA_T;

typedef struct _MURU_QLEN_INFO_T {
	UINT_32 au4DLQlen[MURU_AC_NUM_MAX];
	UINT_32 au4ULQlen[MURU_AC_NUM_MAX];
	UINT_32 u4TotDLQlenAllAc;
	UINT_32 u4TotULQlenAllAc;
	UINT_32 u4BsrTruncateThr;
} MURU_QLEN_INFO_T, *P_MURU_QLEN_INFO_T;

typedef struct _MURU_TXCMD_CTRL_T {
	uint8_t fgGlobalPreLoad;
	INT_16  i2PuPreGrpMaxPsd_dBm;
} MURU_TXCMD_CTRL_T, *P_MURU_TXCMD_CTRL_T;

typedef struct _MURU_RU_TONE_PLAN_DBG_T {
	UINT_8 u1ToneStr[12];
	UINT_32 prToneGroup;	/* Set Host 8B pointer to 4B */
} MURU_RU_TONE_PLAN_DBG_T, *P_MURU_RU_TONE_PLAN_DBG_T;

typedef struct _MURU_TP_DBG_CTRL_T {
	LINK_T rMuruTpDbgFreeList;
	LINK_T rMuruTpDbgUsedList;
	MURU_RU_TONE_PLAN_DBG_T rTonePlanDbg[8];
	uint8_t fgTpDbgEn;
	UINT_16 u2TpDbgShowPeriod;
	UINT_8 u1TonePlanDbgIdx;
	UINT_8 u1Resv[1];
} MURU_TP_DBG_CTRL_T, *P_MURU_TP_DBG_CTRL_T;

typedef struct _MURU_LOCAL_DATA_T {
	/*MURU local Control Parameters*/
	MURU_PARA_T         rMuruPara;

	/*DL and UL Scheduler*/
	MURU_DUSCH_WEIGHT_T eDuSchWeight;

	/*Qlen Info maintained*/
	MURU_QLEN_INFO_T    rQlenInfo;
	UINT_16 u2MuruSplHeadWlanId;

	CMD_MURU_BSRP_CTRL  rExt_Cmd_Bsrp_Ctrl;
	CMD_MURU_HESND_CTRL rExt_Cmd_HeSnd_Ctrl;
	CMD_MURU_CERT_SEND_FRAME_CTRL rExt_Cmd_Cert_Send_frame_Ctrl;
	MURU_TXCMD_CTRL_T   rMuru_TxCmd_Ctrl;
	uint8_t fgMumUl;
	uint8_t fgTwtNonCriticalTxReq;

	MURU_TP_DBG_CTRL_T  rMuru_TpDbg_Ctrl;

	/*Retry STA List*/
	LINK_T  _rRetryForPktDropStaList[MURU_AC_NUM_MAX];
	LINK_T  _rRetryForPktFreeList;
} MURU_LOCAL_DATA_T, *P_MURU_LOCAL_DATA_T;

/*MURU part*/
typedef struct _MURU_GLOBAL_INFO_T {
	UINT_8  u1TxdNum;
	UINT_8  u1Qid;
	UINT_8  u1TxcmdType; /*not used*/
	uint8_t fgSpl;
	UINT_8  u1PresentSpTblIdx;
	uint8_t fgTv;
	uint8_t fgDbdcIdx;
	uint8_t fgPreload;
	uint8_t fgTxop;
	UINT_8  u1OwnMac;
	uint8_t fgIgnoreBw;
	uint8_t fgSmartAnt;
	UINT_8  u1AggPolicy;
	UINT_8  u1Bandwidth;
	UINT_32 u4AntId;
	UINT_8  u1SerialId;
	UINT_8  u1SpeIdx;
	uint8_t fgOptionalBackoff;
} MURU_GLOBAL_INFO_T, *P_MURU_GLOBAL_INFO_T;

typedef  struct _PROT_RU_INFO_T {
	UINT_16  u2Aid;
	UINT_8   u1RuAlloc;
} PROT_RU_INFO_T, *P_PROT_RU_INFO_T;

typedef struct _MURU_PROTECT_INFO_T {
	UINT_8   u1Protect;
	UINT_8   u1StaCnt;
	uint8_t  fgCascadeIdx;
	uint8_t  fgCsRequired;
	UINT_8   u1TfPad;
	UINT_8   u1Rate;
	UINT_8   u1TxMode;
	UINT_8   u1Nsts;
	uint8_t  fgCoding;
	uint8_t  fgDoppler;
	PROT_RU_INFO_T rProtRuInfo[CONFIG_WIFI_RAM_RU_NUM_PARA];
} MURU_PROTECT_INFO_T, *P_MURU_PROTECT_INFO_T;

typedef struct _MURU_USER_INFO_T {
	UINT_16  u2TxPowerAlpha;
	uint8_t  fgCoding;
	UINT_16  u2WlanId;
	UINT_8   u1MuMimoGroup;
	UINT_8   u1MuMimoSpatial;
	UINT_8   u1StartStream;
	uint8_t  fgMultiTid;
	uint8_t  fgRuAllocBn;
	UINT_8   u1RuAlloc;
	UINT_8   u1AckGroup;
	uint8_t  fgSuBar;
	uint8_t  fgMuBar;
	uint8_t  fgCbSta;
	uint8_t  fgAggOld;
	uint8_t  fgPreload;
	UINT_8   u1Rate;
	UINT_8   u1Nsts;
	UINT_8   u1LpCtrl;
	uint8_t  fgContentCh;
	UINT_8   u1AckPol;
	UINT_16  u2SrRate;
	UINT_16  u2RuRatio;
	uint8_t  fgSplPrimaryUser;
	UINT_8   u1AcSeq;
	UINT_8   u1AcNum;
	UINT_16  u2BarRuRatio;
	UINT_16  u2LSigLen;
	UINT_8   u1Bw;
	UINT_8   u1Ac0Ratio;
	UINT_8   u1Ac1Ratio;
	UINT_8   u1Ac2Ratio;
	UINT_8   u1Ac3Ratio;
	UINT_8   u1BarRate;
	UINT_8   u1BarMode;
	UINT_8   u1BarNsts;
	UINT_8   u1BaType;
	uint8_t  fgCsRequired;
	UINT_8   u1LtfType;
	UINT_8   u1LtfSym;
	uint8_t  fgStbc;
	uint8_t  fgLdpcExtraSym;
	UINT_8   u1PktExt;
	uint8_t  fgCoding2;
	uint8_t  fgDcm;
	uint8_t  fgBarAckPol;
	uint8_t  fgAckRuAllocBn;
	UINT_8   u1AckRuAlloc;
	UINT_8   u1AckMcs;
	UINT_8   u1SsAlloc;
	UINT_8   u1TargetRssi;
	uint8_t  fgDoppler;
	uint8_t  fgBf;
	UINT_8   u1TidInfo;
	UINT_16  u2SpatialReuse;
} MURU_USER_INFO_T, *P_MURU_USER_INFO_T;

typedef  struct _MURU_TX_DATA_T {
	UINT_8  u1Rxv;
	uint8_t fgRsp;
	uint8_t fgPsIgnore;
	UINT_8  u1SigBCh1StaCnt;
	UINT_8  u1SigBCh2StaCnt;
	UINT_8  u1StaCnt;
	UINT_8  u1SigBSym;
	UINT_8  u1SigBMcs;
	uint8_t fgRa;
	uint8_t fgSigBDcm;
	uint8_t fgSigBCompress;
	UINT_8  u1LtfSym;
	UINT_8  u1Gi;
	uint8_t fgStbc;
	uint8_t fgCmdPower;
	UINT_16 u2MuPpduDur;
	UINT_8  u1TxPower;
	UINT_8  aucRuAlloc[8];
	uint8_t fgDoppler;
	UINT_8  u1PrimaryUserIdx;
	UINT_8  u1Ltf;
	UINT_8  u1TfPad;
	UINT_8  u1Mu0UserPosition;
	UINT_8  u1Mu1UserPosition;
	UINT_8  u1Mu2UserPosition;
	UINT_8  u1Mu3UserPosition;
	UINT_8  u1MuGroupId;
	uint8_t fgRu26dSigBCh1;
	uint8_t fgRu26uSigBCh2;
	UINT_8  u1TxMode;
	uint8_t fgDynamicBw;
	UINT_8  u1PreamblePuncture;
	UINT_8  u1MuUser;
	UINT_16 u2ProtectionDuration;
	UINT_16 u2ResponseDuration;
	MURU_USER_INFO_T arTxcmdUser[CONFIG_WIFI_RAM_RU_NUM_PARA];
} MURU_TX_DATA_T, *P_MURU_TX_DATA_T;

typedef struct _MURU_USER_ACK_INFO_T {
	UINT_16 u2StaId;
	UINT_16 u2AckTxPowerAlpha;
	uint8_t fgCoding;
	uint8_t fgContentCh;
	UINT_16 u2WlanId;
	uint8_t fgRuAllocBn;
	UINT_8  u1RuAlloc;
	UINT_8  u1Rate;
	UINT_8  u1Nsts;
	UINT_8  u1RuAllNss;
	UINT_16 u2RuRatio;
	uint8_t fgSfEnable;
	UINT_8  u1Ac;
	uint8_t fgSplPrimaryUser;
} MURU_USER_ACK_INFO_T, *P_MURU_USER_ACK_INFO_T;

typedef enum _ENUM_MEM_TYPE_T {
	MEM_TYPE_NONE = 0,
	MEM_TYPE_PLE,
	MEM_TYPE_PSE,
#if (CONFIG_WIFI_RAM_HW_WFDMA == 1)
	MEM_TYPE_PKT_DLM,
#endif
} ENUM_MEM_TYPE_T;

typedef struct _FRAME_BUF_INFO_T {
	ENUM_MEM_TYPE_T eMemType;
	UINT_32         pucBuffer;
	union PKT_ADDR	{
#if (CONFIG_WIFI_RAM_HW_WFDMA == 1)
		UINT_32 u4WfPktAddr;
#endif
		UINT_16 u2Fid;
	} addr;
} FRAME_BUF_INFO_T, *P_FRAME_BUF_INFO_T;

typedef struct _MURU_TX_TRIG_DATA_T {
	UINT_8  u1Rxv;
	UINT_8  u1StaCnt;
	UINT_8  u1BaPol;
	uint8_t fgPriOrder;
	UINT_8  u1SplAc;
	UINT_8  u1PreambPunc;
	UINT_8  u1AckTxMode;
	UINT_8  u1TrigType;
	UINT_32 u4RxHetbCfg1;
	UINT_32 u4RxHetbCfg2;
	UINT_8  u1TfPad;
	UINT_16 u2LSigLen;
	UINT_8  u1SigBCh1StaCnt;
	UINT_8  u1SigBSym;
	UINT_8  u1SigBMcs;
	uint8_t fgSigBDcm;
	uint8_t fgSigBCompress;
	UINT_8  u1LtfSym;
	UINT_8  u1Gi;
	uint8_t fgStbc;
	uint8_t fgDoppler;
	uint8_t fgCmdPower;
	UINT_8  u1SigBCh2StaCnt;
	UINT_16 u2MuPpduDur;
	UINT_8  u1Ltf;
	uint8_t fgRu26dSigBCh1;
	uint8_t fgRu26uSigBCh2;
	UINT_8  au1RuAlloc[8];
	UINT_8  u1AckTxPower;
	UINT_8  u1SsnUser;
	UINT_8  u1MuUser;

	FRAME_BUF_INFO_T    rTxDBufInfo;    /* including the Pointer to the associated buffer */
	FRAME_BUF_INFO_T    rFixFidBufInfo; /* including the Pointer to the associated buffer */
	UINT_16 u2MsduId;
	MURU_USER_ACK_INFO_T rTxcmdUserAck[CONFIG_WIFI_RAM_RU_NUM_PARA];
} MURU_TX_TRIG_DATA_T, *P_MURU_TX_TRIG_DATA_T;

/* A collection of fields related to SR in RXRPT */
typedef struct _SR_RXRPT_T {
	uint8_t fgIsFromCmdrptTx;
	UINT_8 u1SrBand;
	UINT_8 u1SrEntry;
	UINT_32 u4User0Addr2;
	UINT_32 u4TimeStamp;
	INT_32 i4SrPeriodRemain;
} SR_RXRPT_T, *P_SR_RXRPT_T;

typedef struct _TXCMD_DEPOT_TXUSER_T {
	UINT_8 u1Ac;
	UINT_8 u1Rate;
	UINT_8 u1Nsts;
	UINT_8 u1Stbc;
	UINT_8 u1Gi;
	UINT_8 u1RuAlloc;
	uint8_t fgRuAllocBn;
	UINT_8 u1MuMimoGrp;
	UINT_16 u1RuTreeMapArrayIdx;
	UINT_8  u1RuMapArrayIdx;
	UINT_16 u2WlanId;
	UINT_16 u2MumGrpIdx;
	UINT_8 u1TargetRssi;
	uint8_t  fgCoding;
} TXCMD_DEPOT_TXUSER_T, *P_TXCMD_DEPOT_TXUSER_T;

typedef struct _TXCMD_DEPOT_T {
	uint8_t fgIsOccupied;
	UINT_8 u1SerialId;
	UINT_8 u1TxMode;
	UINT_8 u1TxModePrev;
	UINT_8 u1StaCnt;
	UINT_8 u1TxcmdType;
	UINT_8 u1Ac;
	UINT_8 u1Qid;
	UINT_8 u1SchType;
	UINT_8 u1Bandwidth;
	uint8_t fgRa;
	uint8_t fgIsTwt;
	UINT_8 u1AckPol;
	UINT_32 u4MuPpduDuration;
	ENUM_DBDC_BN_T eBandIdx;
	uint8_t fgIsMuRts;
	SR_RXRPT_T rSrRxrpt;
	TXCMD_DEPOT_TXUSER_T arTxUser[MAX_NUM_TXCMD_TX_USER];
} TXCMD_DEPOT_T, *P_TXCMD_DEPOT_T;

typedef struct _MURU_TX_INFO_T {
	/* MURU Global Info */
	MURU_GLOBAL_INFO_T rGlobalData;

	/* MURU Protect Info */
	MURU_PROTECT_INFO_T rProtectData;

	/* MURU TX data Info */
	MURU_TX_DATA_T	rSxnTxData;

	/* MURU TRIG data Info */
	MURU_TX_TRIG_DATA_T rSxnTrigData;

	/* TXCMD Depot */
	/*P_TXCMD_DEPOT_T prTxcmdDepot;*/
	UINT32             prTxcmdDepot;
} MURU_TX_INFO_T, *P_MURU_TX_INFO_T;

typedef struct _PER_USER_DATA_INFO {
	UINT_16 u2WlanId;
	uint8_t fgUserPreLoad;
	UINT_8  u1MuMimoGrp;
	UINT_8  u1RuAlloc;
	UINT_8 u1RuTreeMapArrayIdx;
	UINT_8 u1RuMapArrayIdx;
	uint8_t fgRuAllocBn;
	UINT_8  u1MuMimoSpatial;
	UINT_8  u1StartStream;
	UINT_8  u1RateMode; /* CCk, HT, VHT */
	UINT_8  u1Nss;
	UINT_8 u1StartSpatialStream;
	UINT_8  u1Mcs;
	UINT_8  u1Gi;
	uint8_t fgLdpc;
	UINT_16 u2WeightFactor;
	UINT_8  u1SrMcs;
	UINT_8  u1UpperMCS;
	uint8_t fgDcm;
	UINT_16 u2RuRatio;
	UINT_8 u1RuAllNss;
	uint8_t fgAggOld;
	uint8_t fgCB;
	UINT_8  u1AckBw;
	UINT_8  u1AcSeq;
	UINT_8  u1AcNum;
	UINT_16 u2BarRuRatio;
	UINT_8  u1AcRatio[4];
	UINT_16 u2MumGrpIdx;
	UINT_8  u2MumGrpStaCnt;
	UINT_8  u1LtfType;
	uint8_t fgSplPrimaryUser;
	UINT_8  u1BfType;

	/* BA use */
	UINT_8  u1AckPol;
	UINT_8  u1AckGrp;
	uint8_t fgSuBar;
	uint8_t fgMuBar;
	UINT_8  u1BarRate;
	UINT_8  u1BarMode;
	UINT_8  u1BarNsts;
	UINT_8  u1BaType;
	UINT_32 u4BaMuPpduDur;
	UINT_32 u4BaLSigDur;
	uint8_t fgBaDcm;
	uint8_t fgBaStbc;
	UINT_8  u1AckRuAlloc;
	uint8_t fgAckRuAllocBn;
	UINT_8  u1AckMcs;
	UINT_8  u1AckNss;
	uint8_t fgAckLdpc;
	UINT_8  u1BarAckPol;
	UINT_8  u1SsAaloc;
	/*TPC info*/
	UINT_8  u1TargetRssi;
	UINT_8  u1TidInfo;
	UINT_16 u2EffSnr;

	/*TPA info*/
	UINT_16 u2TxPwrAlpha_dB;

	/* algo. use */
	UINT_32 u4RuScore;
	UINT_32 u4StaMuPpduDur;
	uint8_t fgLargeRu;
} PER_USER_INFO, *P_PER_USER_INFO;

typedef enum _ENUM_TXCMD_TYPE_T {
	TXCMD_HE_TRIG_DATA        = 0x0,
	TXCMD_HE_TX_DATA          = 0x1,
	TXCMD_HE_PROT_TRIG_DATA   = 0x2,
	TXCMD_HE_PROT_TX_DATA     = 0x3,
	TXCMD_HE_SOUNDING         = 0x4,
	TXCMD_SW_PACKET           = 0x5,
	TXCMD_NON_HE_TX_DATA      = 0x6,
	TXCMD_NON_HE_PROT_TX_DATA = 0x7,
	TXCMD_HE_PROT_SOUNDING    = 0x8
} ENUM_TXCMD_TYPE_T, *P_ENUM_TXCMD_TYPE_T;

typedef enum _MURU_TF_PAD_T {
	MURU_TF_PAD_0_US = 0,
	MURU_TF_PAD_8_US = 1,
	MURU_TF_PAD_16_US = 2,
} MURU_TF_PAD_T, *P_MURU_TF_PAD_T;

typedef struct _PROT_RU_INFO {
	UINT_16 aid             : 12,
			ruAlloc         : 4;
} PROT_RU_INFO, *P_PROT_RU_INFO;

typedef struct _MURU_ALGO_PROTSEC_INFO_T {
	UINT_8 u1ProtType;
	UINT_8 u1ProtStaCnt;
	UINT_8 u1ProtBw;
	UINT_8 u1ProtTxMode;
	UINT_8 u1ProtRate;
	UINT_8 u1ProtNsts;
	UINT_8 u1ProtCoding;
	PROT_RU_INFO arProtRuInfo[MAX_USER_IN_PPDU];

	uint8_t fgOptionalBackoff;
	MURU_TF_PAD_T eProtTfPadType;
	uint8_t fgDoppler; /* share with all Sections */
} MURU_ALGO_PROTSEC_INFO_T, *P_MURU_ALGO_PROTSEC_INFO_T;

typedef enum _ENUM_TX_MODE_T {
	TXCMD_TX_MODE_LEGACY_CCK = 0x0,
	TXCMD_TX_MODE_LEGACY_OFDMA = 0x1,
	TXCMD_TX_MODE_HT_MIXED = 0x2,
	TXCMD_TX_MODE_HT_GREEN_FIELD = 0x3,
	TXCMD_TX_MODE_VHT = 0x4,
	TXCMD_TX_MODE_HE_SU = 0x8,
	TXCMD_TX_MODE_HE_EXT_SU = 0x9,
	TXCMD_TX_MODE_HE_TRIG = 0xA,
	TXCMD_TX_MODE_HE_MU = 0xB
} ENUM_TX_MODE_T;

typedef enum _ENUM_RU_SCH_T {
	SCH_T_NONE = 0x0,
	DL_DATA = 0x1,
	UL_DATA = 0x2,
	UL_BSRP = 0x3,
	UL_HE_SND_TF = 0x4,
	DL_SW_PKT = 0x5,
} ENUM_RU_SCH_T, *P_ENUM_RU_SCH_T;

typedef struct _MURU_RU_TP_DBG_T {
	LINK_T  rMuruTpDbgEntry;
	UINT_32 u4SelCnt;
	UINT_32 u4Score;
	UINT_32 u4TotBitsOfThisTP;
	UINT_32 u4PpduTxDur;
	UINT_32 u4StaCnt;
	UINT_16 u2TonePlanIdx;
	UINT_8  u1Resv[2];
	struct MURU_RU_TONE_PLAN *prRuTonePlan;
} MURU_RU_TP_DBG_T, *P_MURU_RU_TP_DBG_T;

struct MURU_RU_TONE_PLAN {
	UINT_16 ruCnt;          /* Total RU cnt */
	UINT_16 largeRuSize;        /* The RU cnt which is equal or more than 106 tones */
	UINT_16 smallRuSize;        /* The RU cnt which is smaller than 106 tones*/
	UINT_8  ruAlloc[8];     /* The tone plan transformation of per 20Mhz RU allocation signaling common part */
	UINT_8 *tpMap;          /* The array which contains the RU set of this tone plan */
	MURU_RU_TP_DBG_T *prTpDbg;
};

typedef struct _SPL_USER_INFO {
	UINT_32 headPktLen  : 10,   /* DW0 */
			rsv1        : 3,
			frag        : 1,
			hdPktNotAgg : 1,
			hdPktRetry  : 1,
			priority    : 4,
			powerSave   : 1,
			updated     : 1,
			wlanId      : 10;
	UINT_32 hdPktDelay  : 10,   /* DW1 */
			rsv2        : 2,
			ppduLen     : 20;
	UINT_32 rxTotQLen   : 16,   /* DW2 */
			txTotQLen   : 16;
	UINT_32 totPktCnt   : 16,   /* DW3 */
			ac4Twt      : 4,
			quota       : 8,
			rsv3        : 1,
			rxEarlyEnd    : 1,
			txEarlyEnd    : 1,
			txed        : 1;
} SPL_USER_INFO, *P_SPL_USER_INFO;

typedef struct _SPL_T {
	UINT_32 rxByteCnt   : 16,   /* DW0 */
			staCount    : 8,
			rsv1        : 3,
			pktType     : 5;
	UINT_32 ac          : 4,    /* DW1 */
			SpTblIdx    : 4,
			tv          : 1,
			roundEnd    : 1,
			SubRoundEnd : 1,
			SplGenMode  : 4,
			notEmpty    : 1,
			rsv2        : 16;
	UINT_32 rsv3;               /* DW2 */
	UINT_32 timeStamp   : 16,    /* DW3 */
			rsv4        : 16;
	SPL_USER_INFO splUser[20]; /* DW4~ */
} SPL_T, *P_SPL_T;

typedef enum _ENUM_BAND_T {
	BAND_NULL = 0000000,
	BAND_2G4 = 2407000,
	BAND_5G0 = 5000000,
	BAND_4G9375 = 4937500,
	BAND_4G89 = 4890000,
	BAND_4G85 = 4850000,
	BAND_4G = 4000000,
	BAND_5G0025 = 5002500,
	BAND_4G0025 = 4002500
} ENUM_BAND_T, *P_ENUM_BAND_T;

typedef struct _MURU_ALLOC_DATA_INFO_T {
	UINT_32 fgBcRu: 1;
	UINT_32 fgSpl: 1;
	UINT_32 fgExp: 1;
	UINT_32 fgTxopBurst: 1;
	UINT_32 fgTxopFailRu: 1;
	UINT_32 fgGlobalPreLoad: 1;
	UINT_32 fgIB: 1;
	UINT_32 fgBs: 1;
	UINT_32 fgSigbDcm: 1; /*share with Section TRIG-Data (uplink MU-OFDMA-BA)*/
	UINT_32 fgRa: 1;
	UINT_32 fgAckLdpcExtra: 1;
	UINT_32 fg26D: 1;
	UINT_32 fg26U: 1;
	UINT_32 fgTrigPO: 1;
	UINT_32 fgMaxScore: 1;
	UINT_32 fgSplPrimaryUser: 1;

	ENUM_BAND_T eBand;
	/* global section */
	UINT_8  u1AggPol;
	UINT_8  u1Ac;
	ENUM_TXCMD_TYPE_T txCmdType;
	UINT_8  u1SerialId;
	UINT_8  u1SpeIdx;
	/* protect section */
	MURU_ALGO_PROTSEC_INFO_T rMuruAlgoProtSec;

	/* Section TX-Data */
	UINT_8  u1SigbSym;
	UINT_8  u1LtfSym;
	UINT_8  u1SigbMcs;  /* share with Section TRIG-Data (uplink MU-OFDMA-BA)  */
	UINT_8  u1GiType;
	UINT_8  u1LtfType;
	UINT_8  u1StaCnt;
	ENUM_TX_MODE_T eTxMode;
	UINT_8 u1AckGiType;     /* share with Section TRIG-Data (uplink MU-OFDMA-BA)  */
	UINT_8 u1AckLtfType;    /* share with Section TRIG-Data (uplink MU-OFDMA-BA)  */
	UINT_8 u1AckMaxNss;
	MURU_TF_PAD_T eTxSecTfPadType;
	/*DL TX Power Allocatioin Info*/
	UINT_8  u1TxPwr_dBm;
	UINT_8  u1Bw;
	UINT_8  u1PrimaryUserIdx;
	UINT_16 u2LongPpduWlanId;
	UINT_8  u1LongPpduUsrIdx;/*userinfo idx in prRuRsp, not wlanid*/
	UINT_32 u4MuPpduDuration;/*for TRIG Section, it would be L-SIG Len*/
	UINT_32 u4MaxBaMuPpduDur;
	UINT_32 u4MaxBaDurForLSig;
	UINT_8  au1MuUp[4];
	UINT_8  u1GrpId;

	/* Section TX-Data Per User Info */
	PER_USER_INFO userInfo[MAX_USER_IN_PPDU];

	/* Section TRIG-Data */
	UINT_8  u1TrigBaPL;
	UINT_8  u1TfType;
	UINT_8  u1TrigSplAc;
	UINT_8  u1TrigAckBw;
	UINT_8  u1TrigAckTxPwr;
	UINT_8  u1TrigAckTxMode;
	UINT_32 u4LSigLength;
	MURU_TF_PAD_T eTrigSecTfPadType;
	UINT_8  ucTfPe;
	/*Per User Ack Info uses PER_USER_INFO in Tx-Data Sec*/

	/*RU Algo Use Begin*/
	UINT_8  u1TotMumGrpCnt;
	UINT_8  au1RuMumGrpStaNum[7]; /* Only RU61~67 support MUMIMO */
	ENUM_RU_SCH_T eSchType;
	UINT_8  u1OperateBw;
	UINT_8  u1HavmDLULIdx;  /* HAVM DL/UL scheduling */
	UINT_8  u1SplStaCnt;
	UINT_16 u2TonePlanIdx;
	UINT_16 u2TypeAStaCnt;
	UINT_16 u2TypeBStaCnt;
	UINT_32 u4MaxHeadTime;
	UINT_32 u4MaxScore;
	UINT_32 u4SuScore;
	UINT_32 u4MuScore;
	UINT_32 u4TotBitsOfThisTP;
	UINT_32 u4PpduTxDur;
	UINT_32 u4MuPpduUtilization;
	/*struct MURU_RU_TONE_PLAN *prRuTonePlan;*/
	UINT_32 prRuTonePlan;
	/*P_SPL_T prSplPkt;*/
	UINT_32 prSplPkt;
	UINT_8  u1AckPol;
	/*RU Algo Use End*/

	/*SR Algo Use Begin*/
	uint8_t fgSr;
	/*P_SR_RXRPT_T prSrRxrpt;*/
	UINT_32 prSrRxrpt;
	/*SR Algo Use End*/

} MURU_ALLOC_DATA_INFO_T, *P_MURU_ALLOC_DATA_INFO_T;

typedef enum _ENUM_MURU_HE_STA_STATE {
	MURU_HE_STA_STATE_CONTENTION     = 0x0,
	MURU_HE_STA_STATE_TRIG           = 0x1,
	MURU_HE_STA_STATE_MAX            = 0x2,
} ENUM_MURU_HE_STA_STATE, *P_ENUM_MURU_HE_STA_STATE;

/*speed up guscore calculation*/
typedef struct _MURU_RUSCORE_RECORD_T {
	UINT_32 u4StaMuPpduDur;
	UINT_32 u4RuScore;
	UINT_8  u1Mcs;
	UINT_8  u1Nss;
	UINT_32 u4BitRate;
} MURU_RUSCORE_RECORD_T, *P_MURU_RUSCORE_RECORD_T;

typedef enum _ENUM_MURU_MAX_AGG_SIZE_T {
	MURU_MAX_AGG_SIZE_64 = 0,
	MURU_MAX_AGG_SIZE_256,
} ENUM_MURU_MAX_AGG_SIZE_T, *P_ENUM_MURU_MAX_AGG_SIZE_T;

typedef enum _MURU_BUF_TYPE {
	MURU_BUF_TYPE_C = 0,
	MURU_BUF_TYPE_B = 1,
	MURU_BUF_TYPE_A = 2,
	MURU_BUF_TYPE_NUM = 3
} MURU_BUF_TYPE, *P_MURU_BUF_TYPE;

typedef struct _LINK_ENTRY_S {
	/* struct _LINK_ENTRY_S *prNext, *prPrev; */
	UINT32 prNext, prPrev;
} LINK_ENTRY_S, *P_LINK_ENTRY_S;

typedef struct _MURU_RETRY_FOR_PKT_T {
	LINK_ENTRY_S rRetryForPktDropEntry;
	UINT_32 prRuSta;
	BOOL fgDropPkt[MURU_AC_NUM_MAX];
	UINT_32 u4AggStatus;
	UINT_8  u1RefCnt;
	UINT_8  u1Resv[3];
} MURU_RETRY_FOR_PKT_T, *P_MURU_RETRY_FOR_PKT_T;

typedef struct _STA_MURU_RECORD_T {
	/*---------------------------------------------------------*/
	/* MU RU record*/
	/*---------------------------------------------------------*/
	ENUM_TX_MODE_T eStaRecCapMode;

#if (CFG_SUPPORT_MURU == 1)

	UINT_8  u1Bw;
	UINT_8  u1TxBw;
	UINT_16 u2WlanId;
	UINT_16 u2StaIdx;
	UINT_16 u2NextStaRecIdxbySPL;

	/*STA Capability*/
	UINT_8 u1BandIdx;
	UINT_8 u1MumCapBitmap;

	/*************input to RU*********/
	UINT_8  u1BfType;
	UINT_8  u1Mcs;
	UINT_8  u1Nss;
	UINT_8  u1Gi;
	UINT_8  u1Ecc;
	UINT_8  u1HeLtf;
	UINT_8  u1Stbc;
	UINT_8  u1Priority;
	UINT_8  afgNonEmptyState[MURU_AC_NUM_MAX];
	UINT_8  u1DlDepCmd[MURU_AC_NUM_MAX]; /*Has be scheduled in DL AC TxCmd Queue for independent txcmd*/
	UINT_8  u1UlDepCmd[MURU_AC_NUM_MAX]; /*Has be scheduled in UL AC TxCmd Queue for independent txcmd*/

	/*BSRP Begin*/
	UINT_8  u1BsrpPeriod;
	UINT_8  u1BsrpMaxPeriod;
	UINT_8  u1BsrpMissCnt;
	UINT_8  u1BsrpHitCnt;
	/*BSRP End*/

	/*DL UL WRR Scheduler Begin*/
	UINT_8  au1DlQuantum[MURU_AC_NUM_MAX];
	UINT_8  au1UlQuantum[MURU_AC_NUM_MAX];
	/*DL UL WRR Scheduler End*/
	UINT_8  u1DelayWeight;
	UINT_8  u1HeSndPeriod;
	UINT_8  u1HeSndMaxPeriod;
	/* MUPPDU RTS retry limit */
	BOOL fgRtsForMuPpduRetry[MURU_AC_NUM_MAX];
	/*MU TPC Begin*/
	UINT_8  u1LastTxMcs;
	INT_8   i1UlPwrHeadroom_dB;
	UINT_8  u1MinTxPwrNonHitCnt;
	uint8_t afgCanNotAgg[MURU_AC_NUM_MAX];

	/*MU TPC Begin*/
	INT_16  i2MinRssi_dBm;
	UINT_8 u1LastRuBitmapIdx;
	INT_16  i2LastPerUserRssi_dBm;
	INT_16  i2PreGrpMaxPsd_dBm;
	INT_16  i2RssiOffset_dB;
	UINT_8  u1RxRptLastBw;
	UINT_8  u1RxRptLastTxMcs;
	INT_16  i2RxRptLastPerUserRssi_dBm;

	UINT_16 u2DelayReq;

	UINT_16 au2MpduCntInPpdu[MURU_AC_NUM_MAX]; /*ppdu cnt*/
	UINT_16 au2RxAvgMpduSize[MURU_AC_NUM_MAX];
	UINT_16 au2CurrMsn[MURU_AC_NUM_MAX];
	UINT_16 au2BaWin[MURU_AC_NUM_MAX];
	UINT_16 au2NewMpduCntInPpdu[MURU_AC_NUM_MAX];
	UINT_16 au2RxPer[MURU_AC_NUM_MAX];
	UINT_16 au2HeadPktLen[MURU_AC_NUM_MAX]; /* unit byte */
	UINT_16 au2UlHeadPktDelay[MURU_AC_NUM_MAX];
	UINT_16 au2RxAvgLongTermMpduSize[MURU_AC_NUM_MAX];
	UINT_16 u2WeightFactor;

	UINT_8 u1DisableBsrpByAc : 7;
	UINT_8 fgLastRxFrmTrig : 1;
	UINT_8 u1UlTypeACnt : 4;
	UINT_8 u1UlNonTypeACnt : 4;
	UINT_8 u1HasRxData : 1;
	UINT_8 u1NoRxDataStopBsrp : 1;
	UINT_8 u1Reserved : 6;
	ENUM_TX_MODE_T eDataTxMode;
	MURU_BUF_TYPE ePpduType; /*type A, type B, or type C traffic*/
	MURU_BUF_TYPE eDL_LTPpduType[MURU_AC_NUM_MAX];
	MURU_BUF_TYPE eUL_LTPpduType[MURU_AC_NUM_MAX];
	MURU_BUF_TYPE eUL_LTTrafficType[MURU_AC_NUM_MAX];

	/*STA State*/
	ENUM_MURU_HE_STA_STATE eHeStaStae;
	ENUM_BAND_T eBand;
	/* DropPkt by retry limit */
	/* P_MURU_RETRY_FOR_PKT_T prRetryForPktInfo; */
	UINT_32 prRetryForPktInfo;

	UINT_32 fgIsAddBaForAnyTIDsOfAC0 : 1;
	UINT_32 fgIsAddBaForAnyTIDsOfAC1 : 1;
	UINT_32 fgIsAddBaForAnyTIDsOfAC2 : 1;
	UINT_32 fgIsAddBaForAnyTIDsOfAC3 : 1;
	UINT_32 fgNonAggressiveRA : 1;
	UINT_32 fgBsrpCandidate : 1;
	UINT_32 fgBsrpTriggerCurPPDU : 1;
	UINT_32 fgBsrpHasSentInBasicTF : 1;
	UINT_32 fg20MOnlyCap : 1;
	UINT_32 fgHeSndCandidate : 1;
	UINT_32 fgHeSndTriggerCurPPDU : 1;
	UINT_32 fgPsMode : 1;
	UINT_32 fgUlSuSnd : 1;
	UINT_32 fgSrAbortBit : 1;
	UINT_32 fgDepCmd : 1; /*for use of algo. MIPs improvement*/
	UINT_32 fgIsTpcInfoValid : 1;
	UINT_32 fgRxRptIsTpcInfoValid: 1;
	UINT_32 fgIsTriggerred : 1;
	UINT_32 fgTcp : 1;
	UINT_32 fgMinTxPwrFlag : 1;
	UINT_32 fgHaveHitMinTxPwrFg : 1;
	UINT_32 fgAssocOk: 1;
	UINT_32 fgNeedBroadcastRU: 1; /*this STA has to TX by Broadcast RU*/
	UINT_32 fgHasRuAssign: 1; /*Has assigned RU to this STA*/
#if (EXE_IN_INET == 1)
	UINT_32 fgHeDlFBMUMCap: 1;
	UINT_32 fgHeDlPBMUMCap: 1;
	UINT_32 fgHeUlPBMUMCap: 1;
	UINT_32 fgHeUlFBMUMCap: 1;
	UINT_32 fgVhtDlMumCap: 1;
	UINT_32 fgUlMuCap: 1;
	UINT_32 fgReserved: 2;
#else
	UINT_32 fgReserved: 8;
#endif

	/* Last RX Rate for STA */
	UINT_8  u1RxRate;
	UINT_8  u1RxMode;
	UINT_8  u1RxNsts;
	UINT_8  u1RxGi;
	UINT_8  u1RxStbc;
	UINT_8  u1RxCoding;
	UINT_8  u1RxBW;
	/*SPL Qlen Info*/
	UINT_32 au4DlTotQlenBytes[MURU_AC_NUM_MAX];
	UINT_32 au4UlTotQlenBytes[MURU_AC_NUM_MAX];   /* unit bytes */
	UINT_32 u4UlTotAllQlenBytes;
	UINT_32 au4HeadPktTime[MURU_AC_NUM_MAX];        /* headPktTime unit is us */
	UINT_32 au4ByesInPpdu[MURU_AC_NUM_MAX]; /*ppdu length*/
	UINT_32 au4UlSchTimeStamp[MURU_AC_NUM_MAX];
	/************RU operation*********/
	UINT_32 u4RuScore;
	UINT_32 u4StaMuPpduDur;      /* staMuPpduTime unit is us */

	/* MUMIMO */
	LINK_ENTRY_S rBsrpLinkEntry;

	/*OFDMA*/
	UINT_32 prRuCandidate;
	UINT_32 u4MaxPpduLenInBits;

	UINT_32  au4TidQueueSizeBytes[MAX_TID_NUM];

	/*throughput monitor*/
	UINT_32 au4ServiceBytesRxPerSecond[MURU_AC_NUM_MAX];
	UINT_32 au4TotBytesRxInService[MURU_AC_NUM_MAX];

	UINT_32 au4AvgTxPpduDur[MURU_AC_NUM_MAX];
	UINT_32 au4AvgRxPpduDur[MURU_AC_NUM_MAX];

	UINT_8 u1MuRtsFailStatusBuf;
	UINT_8 u1MuRtsFailScore;
	UINT_16 u2MuRtsInactiveTimer;

	UINT_8 u1SuAfterMuFailScore;

	MURU_RUSCORE_RECORD_T arRuScoreRrd[MAX_RUSCORE_RECORD_NUM];

	UINT_8 u1McsSmallRU;
#endif
} STA_MURU_RECORD_T, *P_STA_MURU_RECORD_T;

typedef struct _MURU_PURE_STACAP_INFO {
	MURU_STA_DL_OFDMA rDlOfdma;
	MURU_STA_UL_OFDMA rUlOfdma;
	MURU_STA_DL_MIMO rDlMimo;
	MURU_STA_UL_MIMO rUlMimo;
} MURU_PURE_STACAP_INFO, *P_MURU_PURE_STACAP_INFO;

typedef struct _MU_TX_STAT_INFO_T {
	UINT_32 u4SuccessCnt[MURU_MAX_GROUP_CN][MURU_MAX_PFID_NUM];
	UINT_32 u4TotalCnt[MURU_MAX_GROUP_CN][MURU_MAX_PFID_NUM];
} MU_TX_STAT_INFO_T, *P_MU_TX_STAT_INFO_T;

typedef struct _MU_TX_STAT_INFO_LINK_T {
	MU_TX_STAT_INFO_T DownLink;
	MU_TX_STAT_INFO_T UpLink;
} MU_TX_STAT_INFO_LINK_T, *P_MU_TX_STAT_INFO_LINK_T;

#define MAX_SPL_BACKUP_LEN   sizeof(SPL_T)
#define MAX_TXCMD_BACKUP_LEN 200

typedef enum _ENUM_WH_SPL_GEN_MODE_T {
	WH_SPL_TXCMD_DONE_TX_MODE = 0x0,
	WH_SPL_TXCMD_DONE_RX_MODE = 0x1,
	WH_SPL_TXCMD_DONE_TWT_DL_MODE = 0x2,
	WH_SPL_TXCMD_DONE_TWT_UL_MODE = 0x3,
	WH_SPL_IO_TX_MODE = 0x4,
	WH_SPL_IO_RX_MODE = 0x5,
	WH_SPL_TWT_DL_MODE = 0x6,
	WH_SPL_TWT_UL_MODE = 0x7,
	WH_SPL_CHNL_NONEMPTY_TX_MODE = 0x8,
	WH_SPL_CHNL_NONEMPTY_RX_MODE = 0x9,
	WH_SPL_BWREFILL_TX_MODE = 0xa,
	WH_SPL_BWREFILL_RX_MODE = 0xb,
	WH_SPL_ARB_B0_MODE = 0xc,
	WH_SPL_ARB_B1_MODE = 0xd,
	WH_SPL_PRELPAD_MODE = 0xe,
	WH_SPL_PM_CHG_DET_MODE = 0xf,
	WH_SPL_GEN_MODE_INVAILD,
	WH_SPL_GEN_MODE_NUM
} ENUM_WH_SPL_GEN_MODE_T;

typedef enum _ENUM_WH_TXCMD_QUE_T {
	WH_TXC_AC00 = 0,
	WH_TXC_AC01,
	WH_TXC_AC02,
	WH_TXC_AC03,
	WH_TXC_AC10,
	WH_TXC_AC11,
	WH_TXC_AC12,
	WH_TXC_AC13,
	WH_TXC_AC20,
	WH_TXC_AC21,
	WH_TXC_AC22,
	WH_TXC_AC23,
	WH_TXC_AC30,
	WH_TXC_AC31,
	WH_TXC_AC32,
	WH_TXC_AC33,
	WH_TXC_ALTX0,
	WH_TXC_TF0,
	WH_TXC_TWT_TSF_TF0,
	WH_TXC_TWT_DL0,
	WH_TXC_TWT_UL0,
	WH_TXC_QUE_NUM
} ENUM_WH_TXCMD_QUE_T;

typedef struct _MURU_SPL_BACKUP {
	UINT_8      u1SplBuf[MAX_SPL_BACKUP_LEN];
	UINT_8      u1TxCmdBuf[MAX_TXCMD_BACKUP_LEN];
} MURU_SPL_BACKUP, *P_MURU_SPL_BACKUP;

typedef enum _ENUM_MURU_TONE_PLAN_POLICY_T {
	MURU_NORMAL_TP_POLICY = 0,
	MURU_FIXED_2RU_TP_POLICY,
	MURU_FIXED_4RU_TP_POLICY,
	MURU_FIXED_8RU_TP_POLICY,
	MURU_FIXED_16RU_TP_POLICY,
} ENUM_MURU_TONE_PLAN_POLICY_T, *P_ENUM_MURU_TONE_PLAN_POLICY_T;

enum {
	MURU_CMDRPT_TX_SUCCESS = 0,
	MURU_CMDRPT_TX_RTS_FAIL,
	MURU_CMDRPT_TX_PIFS_TIMEOUT,
	MURU_CMDRPT_TX_RESV1,
	MURU_CMDRPT_TX_TWT_TIMEOUT,
	MURU_CMDRPT_TX_MDRDY_TIMEOUT,
	MURU_CMDRPT_TX_STATUS_CNT
};

typedef enum _ENUM_MURU_EST_T	{
	MURU_EST_SPL_TXC = 0,
	MURU_EST_TIMER_CNT,
	MURU_EST_TXC_EMPTY = MURU_EST_TIMER_CNT,
	MURU_EST_SPL,
	MURU_EST_CMDRPT_TX,
	MURU_EST_CNT
} ENUM_MURU_EST_T;

typedef struct _USERINFO_TRIGCMDRPT {
	UINT_16 u2WlanID;
	UINT_8 u1FailReson;
	UINT_16 u2RxAvgMpduSize;
	UINT_16 u2Padding; /*Bytes, u2NonTailDelimter + u2TailDelimter*/
	UINT_16 u2RxTotBytes;
	UINT_16 u2RxMaxMPDUBytes;
	UINT_16 u2RxAcSuccessByte[MURU_AC_NUM_MAX];
	UINT_16 u2RxTotMpduCnt;
	UINT_16 u2RxSuccMpduCnt;
	UINT_16 u2ACQlen[MURU_AC_NUM_MAX];
} USERINFO_TRIGCMDRPT, *P_USERINFO_TRIGCMDRPT;

typedef struct _USERINFO_TXGCMDRPT {
	UINT_16 u2WlanID;
	UINT_16 u2Airtime;
	UINT_8 u1FailReson;
	BOOL fgTCP;
	UINT_16 u2Padding; /*Bytes, u2NonTailDelimter + u2TailDelimter*/
	UINT_16 u2TxTotBytes;
	UINT_16 u2TxMaxMPDUBytes;
	UINT_16 u2TxTotMpduCnt;
	UINT_16 u2TxSuccMpduCnt;
} USERINFO_TXGCMDRPT, *P_USERINFO_TXGCMDRPT;

typedef struct _TX_CMDRPT {
	UINT_8 u1SeriID;
	UINT_8 u1StaCnt;
	UINT_8 u1TxBw;
	UINT_8 u1Status;
	UINT_32 u4TimeStamp;
	USERINFO_TXGCMDRPT userInfo[MAX_MU_NUM_PER_PPDU];
} TX_CMDRPT, *P_TX_CMDRPT;

typedef struct _TRIG_CMDRPT {
	UINT_8 u1SeriID;
	UINT_8 u1StaCnt;
	UINT_8 u1TxBw;
	UINT_8 u1Status;
	UINT_32 u4TimeStamp;
	USERINFO_TRIGCMDRPT userInfo[MAX_MU_NUM_PER_PPDU];
} TRIG_CMDRPT, *P_TRIG_CMDRPT;

typedef struct _MURU_STACNT_STAT_T {
	UINT_16 u2NonHtStaCnt;
	UINT_16 u2HtStaCnt;
	UINT_16 u2VhtStaCnt;
	UINT_16 u2VhtMumStaCnt; /*HE STA with MU-MIMO capability which repeat with HeStaCnt*/
	UINT_16 u2HeStaCnt;
	UINT_16 u2HeMumStaCnt; /*HE STA with MU-MIMO capability which repeat with HeStaCnt*/
	UINT_16 u2TotStaCnt;
} MURU_STACNT_STAT_T, *P_MURU_STACNT_STAT_T;

typedef struct _MURU_BAND_STATISTICS_T {
	UINT_16 au2DL_LT_TypeA_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2DL_LT_TypeB_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2DL_LT_TypeC_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2UL_LT_TypeA_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2UL_LT_TypeB_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2UL_LT_TypeC_StaCnt[MURU_AC_NUM_MAX];
	BOOL    fgRxCntDoTrig;
	MURU_STACNT_STAT_T rStaCnt;
} MURU_BAND_STATISTICS_T, *P_MURU_BAND_STATISTICS_T;

typedef struct _MURU_MUGRP_ENTRY_T {
	UINT_8  u1UserCnt;
	UINT_16 au2WlanId[4];
	INT_32  i4InitOffset[4];
	UINT_8  au1Mcs[4];
	UINT_8  au1BbpMcs[4];
	BOOL    fgHasSounding;
	double  initDataRate[4];     /* dataRate by sounding result or initial setting */
	double  currDataRate[4];     /* Tx data rate between sounding periods */
	double score;
	UINT_32 au4MuQLen[4];
	UINT_8 au1Ac[4];
	UINT_16 u2next;
} MURU_MUGRP_ENTRY_T, *P_MURU_MUGRP_ENTRY_T;

typedef struct _MURU_TAILCMD_T {
	UINT_16 u2SerialId;
} MURU_TAILCMD_T, *P_MURU_TAILCMD_T;

typedef struct _MURU_EST_CMDRPT_TX_T {
	UINT_32 u4StationCnt[MAX_MU_NUM_PER_PPDU];
	UINT_32 u4Status[MURU_CMDRPT_TX_STATUS_CNT];
	UINT_32 u4TotPpduDur;
	UINT_32 u4CmdRptCnt;
} MURU_EST_CMDRPT_TX_T, *P_MURU_EST_CMDRPT_TX_T;

typedef enum _ENUM_DBDC_BN_S {
	S_ENUM_BAND_0,
	S_ENUM_BAND_1,
	S_ENUM_BAND_NUM,
	S_ENUM_BAND_ALL
} ENUM_DBDC_BN_S, *P_ENUM_DBDC_BN_S;

typedef struct _MURU_ALGO_EST_T {
	UINT_8  u1AlgoEstEn;
	UINT_8  u1Resv;
	UINT_16 u2AlgoEstPeriod;
	UINT_32 u4EstStartTime[MURU_EST_TIMER_CNT];
	UINT_32 u4EstEndTime[MURU_EST_TIMER_CNT];
	UINT_32 u4EstTotalTime[MURU_EST_TIMER_CNT];
	UINT_32 u4EstCnt[MURU_EST_TIMER_CNT];
	UINT_32 u4TxCmdEmptyEvntCnt;
	UINT_32 u4SplCnt[WH_SPL_GEN_MODE_NUM];
	ENUM_DBDC_BN_S eEstBand;
	/* MURU_EST_CMDRPT_TX_T *prEstCmdRptTx; */
	UINT32  prEstCmdRptTx;
} MURU_ALGO_EST_T, *P_MURU_ALGO_EST_T;

typedef struct _MURU_SHARE_DATA_T {
	/*for double pointer */
	MURU_ALLOC_DATA_INFO_T rRuAllocData;
	MURU_ALLOC_DATA_INFO_T rTempRuAllocData;
#if (CFG_MURU_MIPS_ESTIMATION == 1)
	MURU_ALLOC_DATA_INFO_T rTestData;
#endif
	/* P_MURU_ALLOC_DATA_INFO_T prRuallocRst; */
	UINT32      prRuallocRst;
	UINT_8      u1PrimaryAc;
	UINT_8      u1PrimaryStaIdx;	/* #256STA */
	UINT_8      u1Qid;
	TX_CMDRPT   rTxCmdRpt;
	TRIG_CMDRPT rTRIG_CMDRPT;
	UINT_8      u2MuRuMaxSplCnt;
	UINT_8      u1MaxStaCntInPpdu;
	UINT_16     u2TypeAStaCnt;
	UINT_16     u2TypeBStaCnt;
	UINT_16     u2TypeCStaCnt;

	BOOL fgPassDueTo20MhzOnlySta;
	/*BW capability*/
	BOOL fg20MhzOperatingDynamicRuAlgo;
	UINT_8 u1Head20MhzOnlySplUsrIdx;
	BOOL fg20MhzOnlyDoPrimaryUserAlways;
	UINT_16 u2PuOriWlanIdx;
	UINT_16 u2Pu20MHzWlanIdx;
	/* 2.4GHz */
	UINT_16 u2HE40Mhz2Dot4GHz_StaCnt;
	UINT_16 u2In40MMuPpduIn2Dot4GHz_HE20MOnlyStaCnt;
	/* 5GHz */
	UINT_16 u2HE4080M_5GHz_StaCnt; /*this would be independent with 160Mhz*/
	UINT_16 u2HE160M_5GHz_StaCnt;
	UINT_16 u2HE1608080M_5GHz_StaCnt;
	UINT_16 u2HE4080M_IN_1608080HEPPDU_5GHz_StaCnt;
	UINT_8 u1TxModePrev;

	/*DBDC Support*/
	MURU_BAND_STATISTICS_T eBandStatistics[RAM_BAND_NUM];
	ENUM_DBDC_BN_S eBandIdx;
	UINT_8         u1GlobalBw;
	uint8_t        fgBsrpBandRequest[RAM_BAND_NUM];
	ENUM_DBDC_BN_S eLastBsrpBandTx;
	MURU_TAILCMD_T rTailCmdInfo;
	/*Currently, Assume WlanIdx = StaRuIdx*/
	STA_MURU_RECORD_T arStaRuRecord[CFG_STA_REC_NUM];
	UINT_8 u1BaGrp[MAX_BA_GRP];
	UINT_8 u1PuBw;
	ENUM_TXCMD_TYPE_T eTxCmdTye;
	MURU_BUF_TYPE ePuRuBuftype;
	BOOL fgUplink;
	BOOL fgUlSnd;
	ENUM_RU_SCH_T eSchtype;
	UINT_8 u1LastBSRPStaIdx;	/* #256STA */

	/* Profiling parameters */
	/* MURU_ALGO_EST_T *prAlgoEst; */
	UINT32   prAlgoEst;
	/*Default Value and Support Ext Cmd*/
	/*Algo Ctrl Section*/
	UINT_32 u4MaxRuAlgoTimeOut;
	UINT_8  u1PpduDurBias;
	UINT_8  u1PreGrp;
	UINT_8  u1MuRtsRule;
	UINT_8  u1MuRtsFailScoreThr;
	UINT_16 u2MuRtsInactiveTimerThr;
	BOOL    fgTxopBurst;
	BOOL    fgOptionalBackoff;
	INT_16  i2PsdDiffThr;
	/*Global and Protection Section*/
	BOOL    fgExp;
	UINT_8  u1Pdc;            /*PPDU Duration Control*/
	BOOL    fgProt;
	UINT_32 u4ProtFrameThr;   /*@us SU PPDU Duration > the thr => Enable RTS/CTS*/
	BOOL    fgForceMuRTS;
	UINT_8  u1ProtRuAlloc;    /*61~68*/
	BOOL    fgFixedRate;      /*0:use RA module rate, 1: use RU fixed rate*/
	/*Tx Data Section*/
	UINT_8  u1TxDataSec_Bw;   /*To Do, 0~7, 0: full 20Mhz, 1: full 40Mhz, 2: full 80Mhz, 3: 160Mhz*/
	UINT_32 u4TxDataSec_MuPpduDur; /*0: follow algo, otherwise, */

	/*TRIG Data Section*/
	UINT_8  u1TrigSec_BA_Policy;
	UINT_8  u1TrigSec_Global_BA_BW;
	UINT_32 u4TrigSec_Global_BA_Dur;

	/*Ext CmdEnd*/
	ENUM_MURU_TONE_PLAN_POLICY_T eTonePlanPolicy;
	UINT_8  u1FixedMcs;
	UINT_8  u1FixedNss;
	UINT_8  u1FixedBaMcs;
	UINT_8  u1FixedBaNss;
	UINT_32 u4PpduDuration; /*0 : No limit, 1~5484 us*/
	BOOL    fgUlMuBa;

	UINT_16 u2UlAvgMpduCnt; /*0: depends on algo. estimation, otherwise, fixed Mpdu Cnt*/
	UINT_32 u4UlAvgMpduSize;/*0: depends on algo. estimation, otherwise, fixed Mpdu Size*/
	UINT_8  u1MaxMuNum;
	UINT_8  u1TypeA_SwPdaPolicy; /*0: max t-put, 1: max Ppdu duration, 2: follow PLE Primary User*/
	UINT_8  u1TypeB_SwPdaPolicy; /*0: max t-put, 1: max Ppdu duration, 2: follow PLE Primary User*/
	UINT_16 u2MpduByte;          /*value = MPDU bytes used for ideal RA's PER calculation*/
	UINT_16 u2QidNeedsDlSplTrigger; /* bitwise record Qid which needs IO trigger DL SPL, SPL lost war */
	UINT_16 u2NonBsrpCount;   /* counter for bsrp debug record */
	MURU_SPL_BACKUP rSplDbgBackup[WH_TXC_ALTX0];
	UINT_8  u1SplBackupSeq;
	UINT_32 u4AcBitmapPreviousBsrp; /* per AC record if previous TXCMD is BSRP, for E2 cmdRpt lost war  */
	UINT_32 u4AcBitmapPreviousDlData; /* per AC record if previous TXCMD is DL, for E2 cmdRpt lost war  */
	UINT_8  u1TriggerTypeOfBsrpTimer;  /* assign TriggerType to UL_BSRP, default type is BSRP */
	UINT_8  u1DisableBsrpTimer;  /* 1: disable UL_BSRP even when fixTpNum > 0 */
	UINT_8  u1DisableULData;  /* 1: disable UL_DATA even when fixTpNum > 0 */
	uint8_t fgSr;  /*TRUE: SR SU TX in SR period, FALSE: after muruSuDataAlloc */
	UINT_32  u4SplWcidMissCnt;
	UINT_32  u4SplWcidMissCnt1;
	uint8_t fgBroadcastRu[RAM_BAND_NUM];  /* TRUE: 20TU probe rsp by broadcast RU, FALSE: by ALTX (algo don't care) */
	UINT_16 u2BroadcastRuWcid[RAM_BAND_NUM]; /* per band unicast wcid faked by broadcast RU */

	uint8_t fgDlOfdmaEn[RAM_BAND_NUM]; /* Switch control of DLOFDMA */
	uint8_t fgUlOfdmaEn[RAM_BAND_NUM]; /* Switch control of ULOFDMA */
	uint8_t fgDlMimoEn[RAM_BAND_NUM]; /* Switch control of DLMIMO */
	uint8_t fgUlMimoEn[RAM_BAND_NUM]; /* Switch control of ULMIMO */
	UINT_8  u1TpStaCntThPri80;
	UINT_8  u1TpStaCntThSec80;
	UINT_8  u1TpStaCntThFB160;
	uint8_t fgIsApTriggerMode[RAM_BAND_NUM]; /* perBandAP trigger/contetion mode record */
	UINT_8  fgTestCase562;
	UINT_16 u2MuEdcaTimerInTriggerMode;  /* MUEDCA value in Trigger mode AP BCN, default max 255 = 2sec */
	UINT_8  u1MuEdcaMaxDelayCnt;
	uint8_t fgCertMuEdcaOverride;
	UINT_8  u1PlatformType;
} MURU_SHARE_DATA_T, *P_MURU_SHARE_DATA_T;

typedef enum _ENUM_MUAR_INDEX_T {
	MUAR_INDEX_OWN_MAC_ADDR_0 = 0,
	MUAR_INDEX_OWN_MAC_ADDR_1,
	MUAR_INDEX_OWN_MAC_ADDR_2,
	MUAR_INDEX_OWN_MAC_ADDR_3,
	MUAR_INDEX_OWN_MAC_ADDR_4,
	MUAR_INDEX_OWN_MAC_ADDR_BC_MC = 0xE,
	MUAR_INDEX_UNMATCHED = 0xF,
	MUAR_INDEX_OWN_MAC_ADDR_11 = 0x11,
	MUAR_INDEX_OWN_MAC_ADDR_12,
	MUAR_INDEX_OWN_MAC_ADDR_13,
	MUAR_INDEX_OWN_MAC_ADDR_14,
	MUAR_INDEX_OWN_MAC_ADDR_15,
	MUAR_INDEX_OWN_MAC_ADDR_16,
	MUAR_INDEX_OWN_MAC_ADDR_17,
	MUAR_INDEX_OWN_MAC_ADDR_18,
	MUAR_INDEX_OWN_MAC_ADDR_19,
	MUAR_INDEX_OWN_MAC_ADDR_1A,
	MUAR_INDEX_OWN_MAC_ADDR_1B,
	MUAR_INDEX_OWN_MAC_ADDR_1C,
	MUAR_INDEX_OWN_MAC_ADDR_1D,
	MUAR_INDEX_OWN_MAC_ADDR_1E,
	MUAR_INDEX_OWN_MAC_ADDR_1F,
	MUAR_INDEX_OWN_MAC_ADDR_20,
	MUAR_INDEX_OWN_MAC_ADDR_21,
	MUAR_INDEX_OWN_MAC_ADDR_22,
	MUAR_INDEX_OWN_MAC_ADDR_23,
	MUAR_INDEX_OWN_MAC_ADDR_24,
	MUAR_INDEX_OWN_MAC_ADDR_25,
	MUAR_INDEX_OWN_MAC_ADDR_26,
	MUAR_INDEX_OWN_MAC_ADDR_27,
	MUAR_INDEX_OWN_MAC_ADDR_28,
	MUAR_INDEX_OWN_MAC_ADDR_29,
	MUAR_INDEX_OWN_MAC_ADDR_2A,
	MUAR_INDEX_OWN_MAC_ADDR_2B,
	MUAR_INDEX_OWN_MAC_ADDR_2C,
	MUAR_INDEX_OWN_MAC_ADDR_2D,
	MUAR_INDEX_OWN_MAC_ADDR_2E,
	MUAR_INDEX_OWN_MAC_ADDR_2F
} ENUM_MUAR_INDEX_T;

enum _ENUM_IGTK_CIPHER_SUIT_T {
	IGTK_CIPHER_SUIT_NONE = 0,
	IGTK_CIPHER_SUIT_BIP,
	IGTK_CIPHER_SUIT_BIP_256
};

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/* This address is not generated by CODA and might be different by project */
#define WIFI_WTBL_BASE                  0x820D8000

#define LWTBL_CONFIG(_wlanIdx) \
	IO_W_32(WF_WTBLON_TOP_WDUCR_ADDR, \
		((_wlanIdx >> 7) & WF_WTBLON_TOP_WDUCR_GROUP_MASK) << WF_WTBLON_TOP_WDUCR_GROUP_SHFT)

#define LWTBL_IDX2BASE(_wlanIdx, _DW) \
	(WIFI_WTBL_BASE | ((_wlanIdx & 0x7F) << 8) | (_DW & 0x3F) << 2)

#define UWTBL_CONFIG(_wlanIdx) \
	IO_W_32(WF_UWTBL_TOP_WDUCR_ADDR, \
		((_wlanIdx >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK) << WF_UWTBL_TOP_WDUCR_GROUP_SHFT)

#define UWTBL_IDX2BASE(_wlanIdx, _DW) \
	(WIFI_UWTBL_BASE | 0x2000 | ((_wlanIdx & 0x7F) << 6) | (_DW & 0xF) << 2)

#define KEYTBL_CONFIG(_key_loc) \
	IO_W_32(WF_UWTBL_TOP_WDUCR_ADDR, \
		(WF_UWTBL_TOP_WDUCR_TARGET_MASK | (((_key_loc >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK) << WF_UWTBL_TOP_WDUCR_GROUP_SHFT)))

#define KEYTBL_IDX2BASE(_key_loc, _DW) \
	(WIFI_UWTBL_BASE | 0x2000 | ((_key_loc & 0x7F) << 6) | (_DW & 0xF) << 2)

enum HETB_TX_CTRL {
	HETB_TX_CFG = 0,
	HETB_TX_START = 1,
	HETB_TX_STOP = 2
};

/*
end of WTBL definition
Copy form BORA hal_wtbl_rom.c
*/

struct _RTMP_ADAPTER;

UINT32
halWtblReadRaw(
	struct _RTMP_ADAPTER *pAd,
	UINT_16  u2EntryIdx,
	ENUM_WTBL_TYPE_T  eType,
	UINT_16  u2StartDW,
	UINT_16  u2LenInDW,
	PVOID    pBuffer
);


#define EEPROM_DBDC_ANTENNA_CFG_OFFSET      0x193
#define DBDC_BAND0_RX_MASK                  BITS(2, 3)
#define DBDC_BAND0_RX_OFFSET                2
#define DBDC_BAND0_TX_MASK                  BITS(0, 1)
#define DBDC_BAND0_TX_OFFSET                0
#define DBDC_BAND1_RX_MASK                  BITS(6, 7)
#define DBDC_BAND1_RX_OFFSET                6
#define DBDC_BAND1_TX_MASK                  BITS(4, 5)
#define DBDC_BAND1_TX_OFFSET                4

#endif /* __MT7992_CR_H__ */
