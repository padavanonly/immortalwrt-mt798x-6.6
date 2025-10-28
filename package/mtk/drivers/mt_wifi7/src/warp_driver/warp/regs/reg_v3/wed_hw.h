#ifndef __wed0_REGS_H__
#define __wed0_REGS_H__


#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned int UINT32;

#ifndef REG_BASE_C_MODULE
// ----------------- wed0 Bit Field Definitions -------------------
#define REG_FLD(width, shift) (shift)

#define PACKING
typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MODE                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_ID, *PREG_WED_ID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ECO                       : 8;
        FIELD BRANCH                    : 8;
        FIELD MINOR                     : 12;
        FIELD MAJOR                     : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_REV_ID, *PREG_WED_REV_ID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_BM                     : 1;
        FIELD RX_BM                     : 1;
        FIELD RX_PG_BM                  : 1;
        FIELD RRO_RX_TO_PG              : 1;
        FIELD TX_FREE_AGT               : 1;
        FIELD TX_TKID_ALI               : 1;
        FIELD rsv_6                     : 2;
        FIELD WPDMA_TX_DRV              : 1;
        FIELD WPDMA_RX_DRV              : 1;
        FIELD WPDMA_RX_D_DRV            : 1;
        FIELD WPDMA_INT_AGT             : 1;
        FIELD WED_TX_DMA                : 1;
        FIELD WED_RX_DMA                : 1;
        FIELD rsv_14                    : 2;
        FIELD WDMA_TX_DRV               : 1;
        FIELD WDMA_RX_DRV               : 1;
        FIELD rsv_18                    : 1;
        FIELD WDMA_INT_AGT              : 1;
        FIELD RX_RRO_QM                 : 1;
        FIELD RX_ROUTE_QM               : 1;
        FIELD TX_PAO                    : 1;
        FIELD rsv_23                    : 8;
        FIELD WED                       : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_MOD_RST, *PREG_WED_MOD_RST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WPDMA_INT_AGT_EN          : 1;
        FIELD WPDMA_INT_AGT_BUSY        : 1;
        FIELD WDMA_INT_AGT_EN           : 1;
        FIELD WDMA_INT_AGT_BUSY         : 1;
        FIELD rsv_4                     : 1;
        FIELD WED_RX_IND_CMD_EN         : 1;
        FIELD WED_RX_PG_BM_EN           : 1;
        FIELD WED_RX_PG_BM_BUSY         : 1;
        FIELD WED_TX_BM_EN              : 1;
        FIELD WED_TX_BM_BUSY            : 1;
        FIELD WED_TX_FREE_AGT_EN        : 1;
        FIELD WED_TX_FREE_AGT_BUSY      : 1;
        FIELD WED_RX_BM_EN              : 1;
        FIELD WED_RX_BM_BUSY            : 1;
        FIELD WED_RX_RRO_QM_EN          : 1;
        FIELD WED_RX_RRO_QM_BUSY        : 1;
        FIELD WED_RX_ROUTE_QM_EN        : 1;
        FIELD WED_RX_ROUTE_QM_BUSY      : 1;
        FIELD WED_TX_TKID_BUSY          : 1;
        FIELD rsv_19                    : 1;
        FIELD WED_TX_TKID_ALI_EN        : 1;
        FIELD WED_TX_TKID_ALI_BUSY      : 1;
        FIELD WED_TX_PAO_EN             : 1;
        FIELD WED_TX_PAO_BUSY           : 1;
        FIELD FINAL_DIDX_RD             : 1;
        FIELD ETH_DMAD_FMT              : 1;
        FIELD rsv_26                    : 2;
        FIELD MIB_RD_CLR                : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_WED_CTRL, *PREG_WED_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSR_CG_DISABLE            : 1;
        FIELD W_BUSY                    : 1;
        FIELD R_BUSY                    : 1;
        FIELD CSR_ERRMID_SET_RIRQ       : 1;
        FIELD CSR_ERRMID_SET_BIRQ       : 1;
        FIELD rsv_5                     : 3;
        FIELD SL0_AWTHRES               : 2;
        FIELD SL0_ARTHRES               : 2;
        FIELD rsv_12                    : 4;
        FIELD MI0_QOS_ON                : 1;
        FIELD MI0_OUTSTD_EXTEND_EN      : 1;
        FIELD MI0_LOCK_ERR_FLAG         : 1;
        FIELD MI0_CTRL_UPD              : 1;
        FIELD MI1_QOS_ON                : 1;
        FIELD MI1_OUTSTD_EXTEND_EN      : 1;
        FIELD MI1_LOCK_ERR_FLAG         : 1;
        FIELD MI1_CTRL_UPD              : 1;
        FIELD MI2_QOS_ON                : 1;
        FIELD MI2_OUTSTD_EXTEND_EN      : 1;
        FIELD MI2_LOCK_ERR_FLAG         : 1;
        FIELD MI2_CTRL_UPD              : 1;
        FIELD MI3_QOS_ON                : 1;
        FIELD MI3_OUTSTD_EXTEND_EN      : 1;
        FIELD MI3_LOCK_ERR_FLAG         : 1;
        FIELD MI3_CTRL_UPD              : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_AXI_CTRL, *PREG_WED_AXI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD COHERENT_WAIT_MAXIMUM     : 8;
        FIELD rsv_8                     : 16;
        FIELD GLO_AWULTRA               : 2;
        FIELD rsv_26                    : 1;
        FIELD GLO_AWULTRA_FORCE         : 1;
        FIELD GLO_ARULTRA               : 2;
        FIELD rsv_30                    : 1;
        FIELD GLO_ARULTRA_FORCE         : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_CTRL2, *PREG_WED_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 2;
        FIELD TX_DIDX_FIN0              : 1;
        FIELD TX_DIDX_FIN1              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_BM_LTH                 : 1;
        FIELD TX_BM_HTH                 : 1;
        FIELD TX_TKID_LTH               : 1;
        FIELD TX_TKID_HTH               : 1;
        FIELD RX_PG_BM_L_BUF            : 1;
        FIELD RX_PG_BM_H_BUF            : 1;
        FIELD RX_BM_L_BUF               : 1;
        FIELD RX_BM_H_BUF               : 1;
        FIELD RX_DIDX_FIN0              : 1;
        FIELD RX_DIDX_FIN1              : 1;
        FIELD RX_DRV_COHERENT           : 1;
        FIELD rsv_19                    : 5;
        FIELD RX_DRV_DMAD_RECYCLE       : 1;
        FIELD rsv_25                    : 2;
        FIELD MID_RDY                   : 1;
        FIELD BUS_MON                   : 1;
        FIELD rsv_29                    : 1;
        FIELD ERR_MON                   : 1;
        FIELD FDBK_LEAVE_FULL           : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_STA, *PREG_WED_EX_INT_STA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_MSK, *PREG_WED_EX_INT_MSK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_MSK1, *PREG_WED_EX_INT_MSK1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_MSK2, *PREG_WED_EX_INT_MSK2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_MSK3, *PREG_WED_EX_INT_MSK3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_STA_POST_MSK0, *PREG_WED_EX_INT_STA_POST_MSK0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_STA_POST_MSK1, *PREG_WED_EX_INT_STA_POST_MSK1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_STA_POST_MSK2, *PREG_WED_EX_INT_STA_POST_MSK2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_EX_INT_STA_POST_MSK3, *PREG_WED_EX_INT_STA_POST_MSK3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PCIE_IRQ                  : 3;
        FIELD rsv_3                     : 1;
        FIELD PCIE_MSI_IRQ              : 3;
        FIELD rsv_7                     : 1;
        FIELD WDMA_IRQ                  : 3;
        FIELD rsv_11                    : 5;
        FIELD CONNSYS_IRQ               : 3;
        FIELD rsv_19                    : 9;
        FIELD WED_IRQ                   : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_IRQ_MON, *PREG_WED_IRQ_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TF_LEN_ERR                : 1;
        FIELD TF_TKID_WO_PYLD           : 1;
        FIELD TX_TKID_FREE_ALREADY_FREE : 1;
        FIELD rsv_3                     : 1;
        FIELD TF_TKID_FIFO_INVLD        : 1;
        FIELD rsv_5                     : 2;
        FIELD TX_PAO_ERR                : 1;
        FIELD rsv_8                     : 2;
        FIELD TX_BM_FREE_AT_EMPTY       : 1;
        FIELD TX_BM_DMAD_RD_ERR         : 1;
        FIELD RX_BM_FREE_AT_EMPTY       : 1;
        FIELD RX_BM_DMAD_RD_ERR         : 1;
        FIELD rsv_14                    : 2;
        FIELD RX_DRV_R_RESP_ERR         : 1;
        FIELD RX_DRV_W_RESP_ERR         : 1;
        FIELD rsv_18                    : 1;
        FIELD RX_DRV_INTI_WDMA_ENABLED  : 1;
        FIELD RX_DRV_BM_DMAD_COHERENT   : 1;
        FIELD RX_DRV_BM_DMAD_ERROR      : 1;
        FIELD TX_DMA_R_RESP_ERR         : 1;
        FIELD TX_DMA_W_RESP_ERR         : 1;
        FIELD rsv_24                    : 1;
        FIELD RX_DRV_GET_BM_DMAD_SKIP   : 1;
        FIELD WPDMA_RX_D_DRV_ERR        : 1;
        FIELD rsv_27                    : 2;
        FIELD RX_ROUTE_QM_ERR           : 1;
        FIELD MID_TAKE_IDX_ERR          : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_ERR_MON, *PREG_WED_ERR_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_ST                     : 8;
        FIELD TX_ST                     : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_ST, *PREG_WED_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRV_ST                 : 8;
        FIELD TX_DRV_ST                 : 8;
        FIELD INT_AGT_ST                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_ST, *PREG_WED_WPDMA_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRV_ST                 : 8;
        FIELD TX_DRV_ST                 : 8;
        FIELD INT_AGT_ST                : 8;
        FIELD AXI_W_AFTER_AW_ST         : 2;
        FIELD RX_PREF_DMAD_ST           : 3;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_ST, *PREG_WED_WDMA_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_BM_ST                  : 8;
        FIELD TX_BM_ST                  : 8;
        FIELD rsv_16                    : 8;
        FIELD FREE_AGT_ST               : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_BM_ST, *PREG_WED_BM_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RROQM_ST                  : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_QM_ST, *PREG_WED_QM_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRV_ST                 : 8;
        FIELD AXI_W_AFTER_AW_ST         : 2;
        FIELD rsv_10                    : 2;
        FIELD PREF_DMAD_ST              : 3;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_D_ST, *PREG_WED_WPDMA_D_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VLD_GRP_NUM               : 9;
        FIELD rsv_9                     : 7;
        FIELD RSV_GRP_NUM               : 9;
        FIELD FREE_AXI_RESP_SEL         : 1;
        FIELD LEGACY_EN                 : 1;
        FIELD rsv_27                    : 1;
        FIELD PAUSE                     : 1;
        FIELD SCAN                      : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_CTRL, *PREG_WED_TX_BM_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_BASE, *PREG_WED_TX_BM_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_TAIL_IDX               : 16;
        FIELD INIT_SW_TAIL_IDX          : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_INIT_PTR, *PREG_WED_TX_BM_INIT_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BYTE_LEN                  : 14;
        FIELD rsv_14                    : 2;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_BLEN, *PREG_WED_TX_BM_BLEN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREE_NUM                  : 16;
        FIELD DMAD_NUM                  : 5;
        FIELD rsv_21                    : 3;
        FIELD DMAD_VLD                  : 1;
        FIELD DMAD_RD                   : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_STS, *PREG_WED_TX_BM_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TAIL_IDX                  : 16;
        FIELD HEAD_IDX                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_PTR, *PREG_WED_TX_BM_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREE_SKBID                : 16;
        FIELD rsv_16                    : 12;
        FIELD FREE_SKBID_VLD            : 1;
        FIELD FREE_SKBID_RD             : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_INTF, *PREG_WED_TX_BM_INTF;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LOW_GRP_NUM               : 9;
        FIELD rsv_9                     : 7;
        FIELD HI_GRP_NUM                : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_DYN_TH, *PREG_WED_TX_BM_DYN_TH;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD_AXI_RD_LESS          : 1;
        FIELD DMAD_AXI_RD_MORE          : 1;
        FIELD DMAD_FULL_AXI_RD          : 1;
        FIELD DMAD_AXI_RD_OVER_TAIL     : 1;
        FIELD rsv_4                     : 3;
        FIELD FREE_AT_EMPTY             : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_ERR_STS, *PREG_WED_TX_BM_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_GRP_IDX             : 9;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_RECYC, *PREG_WED_TX_BM_RECYC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VB_PGSEL                  : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_MON_CTRL, *PREG_WED_TX_BM_MON_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_VB_FREE_0_31, *PREG_WED_TX_BM_VB_FREE_0_31;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_VB_FREE_32_63, *PREG_WED_TX_BM_VB_FREE_32_63;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_VB_USED_0_31, *PREG_WED_TX_BM_VB_USED_0_31;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_VB_USED_32_63, *PREG_WED_TX_BM_VB_USED_32_63;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VLD_GRP_NUM               : 8;
        FIELD rsv_8                     : 4;
        FIELD VB_PGSEL                  : 1;
        FIELD rsv_13                    : 3;
        FIELD RSV_GRP_NUM               : 8;
        FIELD rsv_24                    : 1;
        FIELD FORCE_DRAM                : 1;
        FIELD FREE_WAIT                 : 1;
        FIELD FREE_FORMAT               : 1;
        FIELD PAUSE                     : 1;
        FIELD SCAN                      : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_CTRL, *PREG_WED_TX_TKID_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RSV_REG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_SPR, *PREG_WED_TX_TKID_SPR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_ID                  : 16;
        FIELD END_ID                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_TKID, *PREG_WED_TX_TKID_TKID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREE_TKID                 : 16;
        FIELD FREE_TKFIFO_FDEP          : 10;
        FIELD rsv_26                    : 2;
        FIELD FREE_TKID_VLD             : 1;
        FIELD FREE_TKID_RD              : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_INTF, *PREG_WED_TX_TKID_INTF;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LOW_GRP_NUM               : 8;
        FIELD rsv_8                     : 8;
        FIELD HI_GRP_NUM                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_DYN_TH, *PREG_WED_TX_TKID_DYN_TH;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREE_NUM                  : 16;
        FIELD FSM                       : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_STS, *PREG_WED_TX_TKID_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_GRP_IDX             : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_RECYC, *PREG_WED_TX_TKID_RECYC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 1;
        FIELD FIFO_RST                  : 1;
        FIELD rsv_2                     : 2;
        FIELD FIFO_EMPTY                : 4;
        FIELD FIFO_FULL                 : 4;
        FIELD rsv_12                    : 4;
        FIELD FIFO_CNT                  : 12;
        FIELD FIFO_WR_ERR               : 1;
        FIELD FIFO_RD_ERR               : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_ALI, *PREG_WED_TX_TKID_ALI;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_VB_FREE_0_31, *PREG_WED_TX_TKID_VB_FREE_0_31;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_VB_FREE_32_63, *PREG_WED_TX_TKID_VB_FREE_32_63;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_VB_USED_0_31, *PREG_WED_TX_TKID_VB_USED_0_31;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GRP                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_VB_USED_32_63, *PREG_WED_TX_TKID_VB_USED_32_63;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW0, *PREG_WED_TXD_DW0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW1, *PREG_WED_TXD_DW1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW2, *PREG_WED_TXD_DW2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW3, *PREG_WED_TXD_DW3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW4, *PREG_WED_TXD_DW4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW5, *PREG_WED_TXD_DW5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW6, *PREG_WED_TXD_DW6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TXD_DW7, *PREG_WED_TXD_DW7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TXP_DW0, *PREG_WED_TXP_DW0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD VALUE                     : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TXP_DW1, *PREG_WED_TXP_DW1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DW_OVERWR                 : 10;
        FIELD WCID_BIT                  : 1;
        FIELD TXD_ADDR_SEL              : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_TXDP_CTRL, *PREG_WED_TXDP_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PRB_SEL                   : 6;
        FIELD rsv_6                     : 2;
        FIELD PRB_DW_SEL_TO_CHIP        : 2;
        FIELD rsv_10                    : 14;
        FIELD BUS_MON_AW_EN             : 1;
        FIELD BUS_MON_AR_EN             : 1;
        FIELD TX_BM_MEM_PGSEL           : 2;
        FIELD TX_TKID_BM_MEM_DBG        : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_CTRL, *PREG_WED_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_BUS_ADDR_L, *PREG_WED_DBG_BUS_ADDR_L;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_BUS_ADDR_H, *PREG_WED_DBG_BUS_ADDR_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD STATUS                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_BUS_MON, *PREG_WED_DBG_BUS_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SIGNAL                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_PRB0, *PREG_WED_DBG_PRB0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SIGNAL                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_PRB1, *PREG_WED_DBG_PRB1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SIGNAL                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_PRB2, *PREG_WED_DBG_PRB2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SIGNAL                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_DBG_PRB3, *PREG_WED_DBG_PRB3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_FREE_TKID_IN, *PREG_WED_TX_TKID_FREE_TKID_IN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_FREE_TKID_LDDRAM, *PREG_WED_TX_TKID_FREE_TKID_LDDRAM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_FREE_TKID_LDSRAM, *PREG_WED_TX_TKID_FREE_TKID_LDSRAM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_FREE_TKID_RECYCLE, *PREG_WED_TX_TKID_FREE_TKID_RECYCLE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_FREE_TO_TX_TKID_TKID_MIB, *PREG_WED_TX_FREE_TO_TX_TKID_TKID_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB, *PREG_WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB, *PREG_WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB, *PREG_WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB, *PREG_WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB, *PREG_WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB, *PREG_WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD MAX_DIV                   : 4;
        FIELD rsv_20                    : 4;
        FIELD ACC_DIV                   : 4;
        FIELD rsv_28                    : 3;
        FIELD EN                        : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_CTRL, *PREG_WED_PMTR_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MST0                      : 5;
        FIELD rsv_5                     : 3;
        FIELD MST1                      : 5;
        FIELD rsv_13                    : 3;
        FIELD MST2                      : 5;
        FIELD rsv_21                    : 3;
        FIELD MST3                      : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_TGT, *PREG_WED_PMTR_TGT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MST0                      : 8;
        FIELD MST1                      : 8;
        FIELD MST2                      : 8;
        FIELD MST3                      : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_TGT_ST, *PREG_WED_PMTR_TGT_ST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MST0_CNT                  : 16;
        FIELD MST1_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_LTCY_MAX0_1, *PREG_WED_PMTR_LTCY_MAX0_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MST2_CNT                  : 16;
        FIELD MST3_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_LTCY_MAX2_3, *PREG_WED_PMTR_LTCY_MAX2_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_LTCY_ACC0, *PREG_WED_PMTR_LTCY_ACC0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_LTCY_ACC1, *PREG_WED_PMTR_LTCY_ACC1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_LTCY_ACC2, *PREG_WED_PMTR_LTCY_ACC2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PMTR_LTCY_ACC3, *PREG_WED_PMTR_LTCY_ACC3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT_0                   : 1;
        FIELD EVENT_1                   : 1;
        FIELD EVENT_2                   : 1;
        FIELD EVENT_3                   : 1;
        FIELD EVENT_4                   : 1;
        FIELD EVENT_5                   : 1;
        FIELD EVENT_6                   : 1;
        FIELD EVENT_7                   : 1;
        FIELD EVENT_8                   : 1;
        FIELD EVENT_9                   : 1;
        FIELD EVENT_10                  : 1;
        FIELD EVENT_11                  : 1;
        FIELD EVENT_12                  : 1;
        FIELD EVENT_13                  : 1;
        FIELD EVENT_14                  : 1;
        FIELD EVENT_15                  : 1;
        FIELD EVENT_16                  : 1;
        FIELD EVENT_17                  : 1;
        FIELD EVENT_18                  : 1;
        FIELD EVENT_19                  : 1;
        FIELD EVENT_20                  : 1;
        FIELD EVENT_21                  : 1;
        FIELD EVENT_22                  : 1;
        FIELD EVENT_23                  : 1;
        FIELD EVENT_24                  : 1;
        FIELD EVENT_25                  : 1;
        FIELD EVENT_26                  : 1;
        FIELD EVENT_27                  : 1;
        FIELD EVENT_28                  : 1;
        FIELD EVENT_29                  : 1;
        FIELD EVENT_30                  : 1;
        FIELD EVENT_31                  : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_STA, *PREG_WED_INT_STA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_MSK, *PREG_WED_INT_MSK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DMA_EN                 : 1;
        FIELD TX_DMA_BUSY               : 1;
        FIELD RX_DMA_EN                 : 1;
        FIELD RX_DMA_BUSY               : 1;
        FIELD rsv_4                     : 2;
        FIELD TX_WB_DDONE               : 1;
        FIELD BIG_ENDIAN                : 1;
        FIELD DESC_32B_E                : 1;
        FIELD WCOMPLETE_SEL             : 1;
        FIELD rsv_10                    : 1;
        FIELD RX_DMA_BURST_CFG          : 2;
        FIELD rsv_13                    : 16;
        FIELD BYTE_SWAP                 : 1;
        FIELD CSR_CLKGATE_BYP           : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_GLO_CFG, *PREG_WED_GLO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DTX_IDX0                  : 1;
        FIELD DTX_IDX1                  : 1;
        FIELD rsv_2                     : 4;
        FIELD DRX_IDX0                  : 1;
        FIELD DRX_IDX1                  : 1;
        FIELD rsv_8                     : 22;
        FIELD WPDMA_DRX_IDX0            : 1;
        FIELD WPDMA_DRX_IDX1            : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RST_IDX, *PREG_WED_RST_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_MAX_PTIME              : 8;
        FIELD RX_MAX_PINT               : 7;
        FIELD RX_DLY_INT_EN             : 1;
        FIELD TX_MAX_PTIME              : 8;
        FIELD TX_MAX_PINT               : 7;
        FIELD TX_DLY_INT_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_DLY_INT_CFG, *PREG_WED_DLY_INT_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_FREE_MAX_PTIME         : 8;
        FIELD TX_FREE_MAX_PINT          : 7;
        FIELD TX_FREE_DLY_INT_EN        : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_DLY_INT_CFG1, *PREG_WED_DLY_INT_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RSV_REG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SPR, *PREG_WED_SPR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_MSK1, *PREG_WED_INT_MSK1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_MSK2, *PREG_WED_INT_MSK2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EN                        : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_MSK3, *PREG_WED_INT_MSK3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_STA_POST_MSK0, *PREG_WED_INT_STA_POST_MSK0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_STA_POST_MSK1, *PREG_WED_INT_STA_POST_MSK1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_STA_POST_MSK2, *PREG_WED_INT_STA_POST_MSK2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EVENT                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_INT_STA_POST_MSK3, *PREG_WED_INT_STA_POST_MSK3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RSV_REG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SPR_0X24C, *PREG_WED_SPR_0X24C;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RSV_REG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SPR_0X25C, *PREG_WED_SPR_0X25C;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX0_MIB, *PREG_WED_TX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX1_MIB, *PREG_WED_TX1_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_COHERENT_MIB, *PREG_WED_TX_COHERENT_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX0_MIB, *PREG_WED_RX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX1_MIB, *PREG_WED_RX1_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WR_CNT                    : 16;
        FIELD RD_CNT                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_DMA_MIB, *PREG_WED_RX_DMA_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX0_CTRL0, *PREG_WED_TX0_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_TX0_CTRL1, *PREG_WED_TX0_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_TX0_CTRL2, *PREG_WED_TX0_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_TX0_CTRL3, *PREG_WED_TX0_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX1_CTRL0, *PREG_WED_TX1_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_TX1_CTRL1, *PREG_WED_TX1_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_TX1_CTRL2, *PREG_WED_TX1_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_TX1_CTRL3, *PREG_WED_TX1_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR0, *PREG_WED_SCR0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR1, *PREG_WED_SCR1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR2, *PREG_WED_SCR2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR3, *PREG_WED_SCR3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR4, *PREG_WED_SCR4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR5, *PREG_WED_SCR5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR6, *PREG_WED_SCR6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_SCR7, *PREG_WED_SCR7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX0_CTRL0, *PREG_WED_RX0_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX0_CTRL1, *PREG_WED_RX0_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX0_CTRL2, *PREG_WED_RX0_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX0_CTRL3, *PREG_WED_RX0_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX1_CTRL0, *PREG_WED_RX1_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX1_CTRL1, *PREG_WED_RX1_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX1_CTRL2, *PREG_WED_RX1_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX1_CTRL3, *PREG_WED_RX1_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BASE_PTR_0, *PREG_WED_RX_BASE_PTR_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MAX_CNT_0, *PREG_WED_RX_MAX_CNT_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_CRX_IDX_0, *PREG_WED_RX_CRX_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_DRX_IDX_0, *PREG_WED_RX_DRX_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BASE_PTR_1, *PREG_WED_RX_BASE_PTR_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MAX_CNT_1, *PREG_WED_RX_MAX_CNT_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_CRX_IDX_1, *PREG_WED_RX_CRX_IDX_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_DRX_IDX_1, *PREG_WED_RX_DRX_IDX_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INT_STA                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_STA_REC, *PREG_WED_WPDMA_INT_STA_REC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INT_TRIG                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_TRIG, *PREG_WED_WPDMA_INT_TRIG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DRV_EN                 : 1;
        FIELD TX_DRV_BUSY               : 1;
        FIELD RX_DRV_EN                 : 1;
        FIELD RX_DRV_BUSY               : 1;
        FIELD RX_DRV_RING0_PKT_PROC     : 1;
        FIELD RX_DRV_RING1_PKT_PROC     : 1;
        FIELD RX_DRV_RING0_CRX_SYNC     : 1;
        FIELD RX_DRV_RING1_CRX_SYNC     : 1;
        FIELD RSV0                      : 4;
        FIELD RX_DRV_EVENT_PKT_FMT_VER  : 4;
        FIELD RSV2                      : 2;
        FIELD RX_DRV_UNS_VER_FORCE_4    : 1;
        FIELD RX_DRV_UNSUPPORT_FMT_BYP  : 1;
        FIELD RX_DRV_EVENT_PKT_FMT_CHK  : 1;
        FIELD RX_DDONE2_WR              : 1;
        FIELD RSV1                      : 2;
        FIELD TX_TKID_KEEP              : 1;
        FIELD TX_DDONE_CHK_LAST         : 1;
        FIELD TXD_VER                   : 1;
        FIELD rsv_27                    : 1;
        FIELD TX_DMAD_DW3_ZERO          : 1;
        FIELD BYTE_SWAP                 : 1;
        FIELD TX_DDONE_CHK              : 1;
        FIELD RX_2B_OFFSET              : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_GLO_CFG, *PREG_WED_WPDMA_GLO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CTX_IDX0                  : 1;
        FIELD CTX_IDX1                  : 1;
        FIELD rsv_2                     : 14;
        FIELD CRX_IDX0                  : 1;
        FIELD CRX_IDX1                  : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RST_IDX, *PREG_WED_WPDMA_RST_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_WR_BND_4KB_BST         : 1;
        FIELD TX_RD_BND_4KB_BST         : 1;
        FIELD rsv_2                     : 14;
        FIELD RX_WR_BND_4KB_BST         : 1;
        FIELD RX_RD_BND_4KB_BST         : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_BUS_CFG, *PREG_WED_WPDMA_BUS_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD RING_LOWTH                : 8;
        FIELD MAX_PTIME                 : 8;
        FIELD MAX_PIDX                  : 7;
        FIELD EN                        : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX_DLY_CIDX, *PREG_WED_WPDMA_TX_DLY_CIDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SDL1_MAX                  : 14;
        FIELD rsv_14                    : 17;
        FIELD SDL1_FIXED                : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CTRL, *PREG_WED_WPDMA_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE0                  : 1;
        FIELD TX_DONE1                  : 1;
        FIELD rsv_2                     : 6;
        FIELD RX_DONE0                  : 1;
        FIELD RX_DONE1                  : 1;
        FIELD rsv_10                    : 2;
        FIELD RRO_RX_INT0               : 1;
        FIELD RRO_RX_INT1               : 1;
        FIELD rsv_14                    : 2;
        FIELD TX_FREE_DONE0             : 1;
        FIELD TX_FREE_DONE1             : 1;
        FIELD rsv_18                    : 6;
        FIELD MSDU_PG_INT0              : 1;
        FIELD MSDU_PG_INT1              : 1;
        FIELD MSDU_PG_INT2              : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_FORCE_PROC, *PREG_WED_WPDMA_FORCE_PROC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSK_ON_DLY_PRD            : 8;
        FIELD POLL_PRD                  : 8;
        FIELD SRC_SEL                   : 2;
        FIELD rsv_18                    : 2;
        FIELD MSK_EN_POLA               : 1;
        FIELD SUBRT_ADV                 : 1;
        FIELD SIG_SRC                   : 1;
        FIELD MSK_ON_DLY                : 1;
        FIELD POLL_MODE                 : 2;
        FIELD rsv_26                    : 2;
        FIELD MSK_EN                    : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CTRL, *PREG_WED_WPDMA_INT_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VALUE                     : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_MSK, *PREG_WED_WPDMA_INT_MSK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INT_CLR                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CLR, *PREG_WED_WPDMA_INT_CLR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD rsv_2                     : 6;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD RRO_RX_INT0               : 1;
        FIELD RRO_RX_INT1               : 1;
        FIELD MSDU_PG_INT0              : 1;
        FIELD MSDU_PG_INT1              : 1;
        FIELD MSDU_PG_INT2              : 1;
        FIELD rsv_15                    : 1;
        FIELD TX_FREE_DONE_INT0         : 1;
        FIELD TX_FREE_DONE_INT1         : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_MON, *PREG_WED_WPDMA_INT_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_EN0               : 1;
        FIELD TX_DONE_CLR0              : 1;
        FIELD TX_DONE_TRIG0             : 5;
        FIELD rsv_7                     : 1;
        FIELD TX_DONE_EN1               : 1;
        FIELD TX_DONE_CLR1              : 1;
        FIELD TX_DONE_TRIG1             : 5;
        FIELD rsv_15                    : 9;
        FIELD DLY_MERGE_EN              : 1;
        FIELD rsv_25                    : 1;
        FIELD DLY_MERGE_BSEL            : 5;
        FIELD DLY_MERGE_UNP_IMM         : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CTRL_TX, *PREG_WED_WPDMA_INT_CTRL_TX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DONE_EN0               : 1;
        FIELD RX_DONE_CLR0              : 1;
        FIELD RX_DONE_TRIG0             : 5;
        FIELD rsv_7                     : 1;
        FIELD RX_DONE_EN1               : 1;
        FIELD RX_DONE_CLR1              : 1;
        FIELD RX_DONE_TRIG1             : 5;
        FIELD rsv_15                    : 9;
        FIELD DLY_MERGE_EN              : 1;
        FIELD rsv_25                    : 1;
        FIELD DLY_MERGE_BSEL            : 5;
        FIELD DLY_MERGE_UNP_IMM         : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CTRL_RX, *PREG_WED_WPDMA_INT_CTRL_RX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_FREE_DONE_EN0          : 1;
        FIELD TX_FREE_DONE_CLR0         : 1;
        FIELD TX_FREE_DONE_TRIG0        : 5;
        FIELD rsv_7                     : 1;
        FIELD TX_FREE_DONE_EN1          : 1;
        FIELD TX_FREE_DONE_CLR1         : 1;
        FIELD TX_FREE_DONE_TRIG1        : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CTRL_TX_FREE, *PREG_WED_WPDMA_INT_CTRL_TX_FREE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RSV_REG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_SPR, *PREG_WED_WPDMA_SPR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CFG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_MSIS_TRIG, *PREG_WED_PCIE_MSIS_TRIG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD REG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_MSIS_REC, *PREG_WED_PCIE_MSIS_REC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CFG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_MSIS_CLR, *PREG_WED_PCIE_MSIS_CLR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CFG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_INTS_CLR, *PREG_WED_PCIE_INTS_CLR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CFG                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_EP_INTS_CLR, *PREG_WED_PCIE_EP_INTS_CLR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RC_PTR                    : 8;
        FIELD rsv_8                     : 8;
        FIELD EP_PTR                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_CFG_ADDR_H, *PREG_WED_PCIE_CFG_ADDR_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_CFG_ADDR_INTS, *PREG_WED_PCIE_CFG_ADDR_INTS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_CFG_ADDR_INTM, *PREG_WED_PCIE_CFG_ADDR_INTM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_CFG_ADDR_MSIS, *PREG_WED_PCIE_CFG_ADDR_MSIS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_CFG_ADDR_INTS_EP, *PREG_WED_PCIE_CFG_ADDR_INTS_EP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INTS_TRIG                 : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_INTS_TRIG, *PREG_WED_PCIE_INTS_TRIG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INTS                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_INTS_REC, *PREG_WED_PCIE_INTS_REC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INTM                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_INTM_REC, *PREG_WED_PCIE_INTM_REC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD POLL_PRD                  : 8;
        FIELD rsv_8                     : 4;
        FIELD POLL_MODE                 : 2;
        FIELD rsv_14                    : 2;
        FIELD SRC_SEL                   : 2;
        FIELD EXT_INCPT_DIS             : 1;
        FIELD rsv_19                    : 1;
        FIELD MSK_EN_POLA               : 1;
        FIELD IRQ_FILTER                : 1;
        FIELD CLR_PCIE_BF_WPDMA         : 1;
        FIELD rsv_23                    : 1;
        FIELD MSI_EN                    : 1;
        FIELD IRQ_MSI_SEL               : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_PCIE_INT_CTRL, *PREG_WED_PCIE_INT_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_INTS, *PREG_WED_WPDMA_CFG_ADDR_INTS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_INTM, *PREG_WED_WPDMA_CFG_ADDR_INTM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_TX, *PREG_WED_WPDMA_CFG_ADDR_TX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_TX_FREE, *PREG_WED_WPDMA_CFG_ADDR_TX_FREE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_INT_H, *PREG_WED_WPDMA_CFG_ADDR_INT_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_TX_H, *PREG_WED_WPDMA_CFG_ADDR_TX_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_CFG_ADDR_TX_FREE_H, *PREG_WED_WPDMA_CFG_ADDR_TX_FREE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX0_MIB, *PREG_WED_WPDMA_TX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX1_MIB, *PREG_WED_WPDMA_TX1_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX_COHERENT_MIB, *PREG_WED_WPDMA_TX_COHERENT_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX0_MIB, *PREG_WED_WPDMA_RX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX1_MIB, *PREG_WED_WPDMA_RX1_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_COHERENT_MIB, *PREG_WED_WPDMA_RX_COHERENT_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_EXTC_FREE_TKID_MIB, *PREG_WED_WPDMA_RX_EXTC_FREE_TKID_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX0_CTRL0, *PREG_WED_WPDMA_TX0_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX0_CTRL1, *PREG_WED_WPDMA_TX0_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD CPU_IDX_INT               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX0_CTRL2, *PREG_WED_WPDMA_TX0_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_12                    : 4;
        FIELD RLS_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX0_CTRL3, *PREG_WED_WPDMA_TX0_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX1_CTRL0, *PREG_WED_WPDMA_TX1_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX1_CTRL1, *PREG_WED_WPDMA_TX1_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD CPU_IDX_INT               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX1_CTRL2, *PREG_WED_WPDMA_TX1_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_12                    : 4;
        FIELD RLS_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_TX1_CTRL3, *PREG_WED_WPDMA_TX1_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_MON_R0_WRAP, *PREG_RRO_RX_D_DRV_MON_R0_WRAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_MON_R1_WRAP, *PREG_RRO_RX_D_DRV_MON_R1_WRAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_MON_R2_WRAP, *PREG_RRO_RX_D_DRV_MON_R2_WRAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_MON0, *PREG_RRO_RX_D_DRV_MON0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_MON1, *PREG_RRO_RX_D_DRV_MON1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_MON2, *PREG_RRO_RX_D_DRV_MON2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_ERR_COMP0, *PREG_RRO_RX_D_DRV_ERR_COMP0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_ERR_COMP1, *PREG_RRO_RX_D_DRV_ERR_COMP1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_ERR_COMP2, *PREG_RRO_RX_D_DRV_ERR_COMP2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_ERR_COMP3, *PREG_RRO_RX_D_DRV_ERR_COMP3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WADDR                     : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_LEAGLE_START, *PREG_RRO_RX_D_DRV_LEAGLE_START;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WADDR                     : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_LEAGLE_END, *PREG_RRO_RX_D_DRV_LEAGLE_END;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_MON_R0_WRAP, *PREG_RRO_RX_PG_DRV_MON_R0_WRAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_MON_R1_WRAP, *PREG_RRO_RX_PG_DRV_MON_R1_WRAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_MON_R2_WRAP, *PREG_RRO_RX_PG_DRV_MON_R2_WRAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_MON0, *PREG_RRO_RX_PG_DRV_MON0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_MON1, *PREG_RRO_RX_PG_DRV_MON1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_MON2, *PREG_RRO_RX_PG_DRV_MON2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_ERR_COMP0, *PREG_RRO_RX_PG_DRV_ERR_COMP0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_ERR_COMP1, *PREG_RRO_RX_PG_DRV_ERR_COMP1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_ERR_COMP2, *PREG_RRO_RX_PG_DRV_ERR_COMP2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_ERR_COMP3, *PREG_RRO_RX_PG_DRV_ERR_COMP3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WADDR                     : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_LEAGLE_START, *PREG_RRO_RX_PG_DRV_LEAGLE_START;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WADDR                     : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_LEAGLE_END, *PREG_RRO_RX_PG_DRV_LEAGLE_END;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_ERR_SWITCH_ADDR, *PREG_RRO_RX_D_DRV_ERR_SWITCH_ADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE                      : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_ERR_SWITCH_ADDR, *PREG_RRO_RX_PG_DRV_ERR_SWITCH_ADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_IN_MIB, *PREG_RRO_RX_D_DRV_IN_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_OU_MIB, *PREG_RRO_RX_D_DRV_OU_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_DRV_DESC_MIB, *PREG_RRO_RX_D_DRV_DESC_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_IN_MIB, *PREG_RRO_RX_PG_DRV_IN_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_OU_MIB, *PREG_RRO_RX_PG_DRV_OU_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_PG_DRV_DESC_MIB, *PREG_RRO_RX_PG_DRV_DESC_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX0_CTRL0, *PREG_WED_WPDMA_RX0_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX0_CTRL1, *PREG_WED_WPDMA_RX0_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DRV_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX0_CTRL2, *PREG_WED_WPDMA_RX0_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX0_CTRL3, *PREG_WED_WPDMA_RX0_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX1_CTRL0, *PREG_WED_WPDMA_RX1_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX1_CTRL1, *PREG_WED_WPDMA_RX1_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DRV_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX1_CTRL2, *PREG_WED_WPDMA_RX1_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX1_CTRL3, *PREG_WED_WPDMA_RX1_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_BASE, *PREG_WED_WPDMA_RX_D_RX0_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_CNT, *PREG_WED_WPDMA_RX_D_RX0_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DRV_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_CRX_IDX, *PREG_WED_WPDMA_RX_D_RX0_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX_MIRO              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_DRX_IDX, *PREG_WED_WPDMA_RX_D_RX0_DRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_BASE, *PREG_WED_WPDMA_RX_D_RX1_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_CNT, *PREG_WED_WPDMA_RX_D_RX1_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DRV_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_CRX_IDX, *PREG_WED_WPDMA_RX_D_RX1_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX_MIRO              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_DRX_IDX, *PREG_WED_WPDMA_RX_D_RX1_DRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WAIT_BM_CNT_MAX           : 16;
        FIELD DRX_CRX_DISTANCE_THRES    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_THRES_CFG, *PREG_WED_WPDMA_RX_D_RX0_THRES_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WAIT_BM_CNT_MAX           : 16;
        FIELD DRX_CRX_DISTANCE_THRES    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_THRES_CFG, *PREG_WED_WPDMA_RX_D_RX1_THRES_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD RX_RING_NUM               : 8;
        FIELD BASE_PTR_WIDTH            : 8;
        FIELD INDEX_WIDTH               : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_INFO, *PREG_WED_WPDMA_RX_D_INFO;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRV_EN                 : 1;
        FIELD RX_DRV_BUSY               : 1;
        FIELD RXDRV_CLKGATE_BYP         : 1;
        FIELD FSM_RETURN_IDLE           : 1;
        FIELD RST_INIT_COMPLETE_FLAG    : 1;
        FIELD INIT_PHASE_BYPASS         : 1;
        FIELD INIT_PHASE_ENABLING_RXDMA_BYPASS : 1;
        FIELD INIT_PHASE_RXEN_SEL       : 5;
        FIELD RXDRV_DISABLED_FSM_AUTO_IDLE : 1;
        FIELD DISABLE_IRQ_HANDLE_PROC   : 1;
        FIELD DISABLE_DMAD_HANDLE_PROC  : 1;
        FIELD WCOMPLETE_SEL             : 1;
        FIELD WAIT_COHERENT_EN          : 1;
        FIELD AXI_W_AFTER_AW_EN         : 1;
        FIELD IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE : 1;
        FIELD IDLE_STATE_DMAD_SUPPLY_EN : 1;
        FIELD DYNAMIC_SKIP_DMAD_PREPARE : 1;
        FIELD DYNAMIC_DMAD_RECYCLE      : 1;
        FIELD DYNAMIC_SKIP_DMAD_CNT_SRC : 1;
        FIELD DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD : 1;
        FIELD RXD_READ_LEN              : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_GLO_CFG, *PREG_WED_WPDMA_RX_D_GLO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD CRX_IDX0                  : 1;
        FIELD CRX_IDX1                  : 1;
        FIELD rsv_18                    : 2;
        FIELD DRX_IDX_ALL               : 1;
        FIELD rsv_21                    : 3;
        FIELD DRV_IDX0                  : 1;
        FIELD DRV_IDX1                  : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RST_IDX, *PREG_WED_WPDMA_RX_D_RST_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRV_IDX0                  : 12;
        FIELD rsv_12                    : 3;
        FIELD LOAD_DRV_IDX0             : 1;
        FIELD DRV_IDX1                  : 12;
        FIELD rsv_28                    : 3;
        FIELD LOAD_DRV_IDX1             : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_LOAD_DRV_IDX, *PREG_WED_WPDMA_RX_D_LOAD_DRV_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX0                  : 12;
        FIELD rsv_12                    : 3;
        FIELD LOAD_CRX_IDX0             : 1;
        FIELD CRX_IDX1                  : 12;
        FIELD rsv_28                    : 3;
        FIELD LOAD_CRX_IDX1             : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_LOAD_CRX_IDX, *PREG_WED_WPDMA_RX_D_LOAD_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_GLO_CFG_ADDR, *PREG_WED_WPDMA_RX_D_GLO_CFG_ADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_GLO_CFG_ADDR_H, *PREG_WED_WPDMA_RX_D_GLO_CFG_ADDR_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_MIB, *PREG_WED_WPDMA_RX_D_RX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_MIB, *PREG_WED_WPDMA_RX_D_RX1_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_RECYCLE_MIB, *PREG_WED_WPDMA_RX_D_RX0_RECYCLE_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_RECYCLE_MIB, *PREG_WED_WPDMA_RX_D_RX1_RECYCLE_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX0_PROCESSED_MIB, *PREG_WED_WPDMA_RX_D_RX0_PROCESSED_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX1_PROCESSED_MIB, *PREG_WED_WPDMA_RX_D_RX1_PROCESSED_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RX_COHERENT_MIB, *PREG_WED_WPDMA_RX_D_RX_COHERENT_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRV_R_RESP_ERR         : 1;
        FIELD RX_DRV_W_RESP_ERR         : 1;
        FIELD RX_DRV_COHERENT           : 1;
        FIELD RX_DRV_INTI_WDMA_ENABLED  : 1;
        FIELD RX_DRV_BM_DMAD_COHERENT   : 1;
        FIELD RX_DRV_BM_DMAD_ERROR      : 1;
        FIELD RX_DRV_DMAD_RECYCLE       : 1;
        FIELD RX_DRV_GET_BM_DMAD_SKIP   : 1;
        FIELD RX_DRV_PREF_R_RESP_ERR    : 1;
        FIELD RX_DRV_PREF_DMAD_ERR      : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_ERR_STS, *PREG_WED_WPDMA_RX_D_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TKID_RING_EN              : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_TKID_CFG, *PREG_WED_WPDMA_RX_D_TKID_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_TKID_RX0_BASE, *PREG_WED_WPDMA_RX_D_TKID_RX0_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_TKID_RX1_BASE, *PREG_WED_WPDMA_RX_D_TKID_RX1_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX0_PTR_H                 : 8;
        FIELD rsv_8                     : 8;
        FIELD RX1_PTR_H                 : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_TKID_RX_BASE_H, *PREG_WED_WPDMA_RX_D_TKID_RX_BASE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD rsv_2                     : 6;
        FIELD AWULTRA                   : 2;
        FIELD rsv_10                    : 2;
        FIELD ARULTRA                   : 2;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_BUS_CFG, *PREG_WED_WPDMA_RX_D_BUS_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ENABLE                    : 1;
        FIELD BUSY                      : 1;
        FIELD DMAD_SIZE                 : 1;
        FIELD DDONE_CHK                 : 1;
        FIELD DDONE_POLARITY            : 1;
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD rsv_7                     : 1;
        FIELD BURST_SIZE                : 5;
        FIELD rsv_13                    : 3;
        FIELD LOW_THRES                 : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_PREF_CFG, *PREG_WED_WPDMA_RX_D_PREF_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX                 : 12;
        FIELD rsv_12                    : 3;
        FIELD IDX_CLR                   : 1;
        FIELD IDX_OW_VAL                : 12;
        FIELD rsv_28                    : 3;
        FIELD IDX_OW                    : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_PREF_RX0_SIDX, *PREG_WED_WPDMA_RX_D_PREF_RX0_SIDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX                 : 12;
        FIELD rsv_12                    : 3;
        FIELD IDX_CLR                   : 1;
        FIELD IDX_OW_VAL                : 12;
        FIELD rsv_28                    : 3;
        FIELD IDX_OW                    : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_PREF_RX1_SIDX, *PREG_WED_WPDMA_RX_D_PREF_RX1_SIDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_CLEAR               : 1;
        FIELD RING0_FULL                : 1;
        FIELD RING0_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING0_USED_CNT            : 6;
        FIELD RING0_FREE_CNT            : 6;
        FIELD RING1_CLEAR               : 1;
        FIELD RING1_FULL                : 1;
        FIELD RING1_EMPTY               : 1;
        FIELD rsv_19                    : 1;
        FIELD RING1_USED_CNT            : 6;
        FIELD RING1_FREE_CNT            : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_PREF_FIFO_CFG, *PREG_WED_WPDMA_RX_D_PREF_FIFO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CIDX_DLY_THRES_EN         : 1;
        FIELD rsv_1                     : 1;
        FIELD CIDX_DLY_CNT_CLR          : 2;
        FIELD rsv_4                     : 4;
        FIELD CIDX_DLY_CNT_MAX          : 8;
        FIELD CIDX_DLY_THRES            : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_CIDX_WR_CFG, *PREG_WED_WPDMA_RX_D_CIDX_WR_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RING0_CFG_ADDR, *PREG_WED_WPDMA_RX_D_RING0_CFG_ADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RING0_CFG_ADDR_H, *PREG_WED_WPDMA_RX_D_RING0_CFG_ADDR_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RING1_CFG_ADDR, *PREG_WED_WPDMA_RX_D_RING1_CFG_ADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDRESS                   : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_RING1_CFG_ADDR_H, *PREG_WED_WPDMA_RX_D_RING1_CFG_ADDR_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ENABLE                    : 1;
        FIELD ERR                       : 1;
        FIELD ERR_CLR                   : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_AW_RAN_DBG0, *PREG_WED_WPDMA_RX_D_AW_RAN_DBG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDR_MAX_31_0             : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_AW_RAN_DBG1, *PREG_WED_WPDMA_RX_D_AW_RAN_DBG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDR_MIN_31_0             : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_AW_RAN_DBG2, *PREG_WED_WPDMA_RX_D_AW_RAN_DBG2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDR_MAX_47_32            : 16;
        FIELD ADDR_MIN_47_32            : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_AW_RAN_DBG3, *PREG_WED_WPDMA_RX_D_AW_RAN_DBG3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ERR_ADDR_31_0             : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_AW_RAN_DBG4, *PREG_WED_WPDMA_RX_D_AW_RAN_DBG4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ERR_ADDR_47_16            : 16;
        FIELD ERR_LEN                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_RX_D_AW_RAN_DBG5, *PREG_WED_WPDMA_RX_D_AW_RAN_DBG5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_TX0_BASE, *PREG_WED_WDMA_TX0_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_TX0_CNT, *PREG_WED_WDMA_TX0_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CTX_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_TX0_CTX_IDX, *PREG_WED_WDMA_TX0_CTX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DTX_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_TX0_DTX_IDX, *PREG_WED_WDMA_TX0_DTX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WR_CNT                    : 16;
        FIELD RD_CNT                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_TX0_MIB, *PREG_WED_WDMA_TX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_BASE, *PREG_WED_WDMA_RX0_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 16;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_CNT, *PREG_WED_WDMA_RX0_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 16;
        FIELD DRV_IDX                   : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_CRX_IDX, *PREG_WED_WDMA_RX0_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX_MIRO              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_DRX_IDX, *PREG_WED_WDMA_RX0_DRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_BASE, *PREG_WED_WDMA_RX1_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 16;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_CNT, *PREG_WED_WDMA_RX1_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 16;
        FIELD DRV_IDX                   : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_CRX_IDX, *PREG_WED_WDMA_RX1_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX_MIRO              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_DRX_IDX, *PREG_WED_WDMA_RX1_DRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WAIT_BM_CNT_MAX           : 16;
        FIELD DRX_CRX_DISTANCE_THRES    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_THRES_CFG, *PREG_WED_WDMA_RX0_THRES_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WAIT_BM_CNT_MAX           : 16;
        FIELD DRX_CRX_DISTANCE_THRES    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_THRES_CFG, *PREG_WED_WDMA_RX1_THRES_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ENABLE                    : 1;
        FIELD BUSY                      : 1;
        FIELD DMAD_SIZE                 : 1;
        FIELD DDONE_CHK                 : 1;
        FIELD DDONE_POLARITY            : 1;
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD rsv_7                     : 1;
        FIELD BURST_SIZE                : 5;
        FIELD R_RESP_ERR                : 1;
        FIELD DMAD_ERR                  : 1;
        FIELD rsv_15                    : 1;
        FIELD LOW_THRES                 : 6;
        FIELD RX0_SIDX_OW               : 1;
        FIELD RX1_SIDX_OW               : 1;
        FIELD RX0_SIDX_CLR              : 1;
        FIELD RX1_SIDX_CLR              : 1;
        FIELD DDONE2_EN                 : 1;
        FIELD DDONE2_BUSY               : 1;
        FIELD DDONE2_CLR_MET            : 1;
        FIELD DDONE2_CLR                : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX_PREF_CFG, *PREG_WED_WDMA_RX_PREF_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX                 : 16;
        FIELD IDX_OW_VAL                : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX_PREF_RX0_SIDX, *PREG_WED_WDMA_RX_PREF_RX0_SIDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX                 : 16;
        FIELD IDX_OW_VAL                : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX_PREF_RX1_SIDX, *PREG_WED_WDMA_RX_PREF_RX1_SIDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_CLEAR               : 1;
        FIELD RING0_FULL                : 1;
        FIELD RING0_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING0_USED_CNT            : 6;
        FIELD RING0_FREE_CNT            : 6;
        FIELD RING1_CLEAR               : 1;
        FIELD RING1_FULL                : 1;
        FIELD RING1_EMPTY               : 1;
        FIELD rsv_19                    : 1;
        FIELD RING1_USED_CNT            : 6;
        FIELD RING1_FREE_CNT            : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX_PREF_FIFO_CFG, *PREG_WED_WDMA_RX_PREF_FIFO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CIDX_DLY_THRES_EN         : 1;
        FIELD rsv_1                     : 1;
        FIELD CIDX_DLY_CNT_CLR          : 2;
        FIELD rsv_4                     : 4;
        FIELD CIDX_DLY_CNT_MAX          : 8;
        FIELD CIDX_DLY_THRES            : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX_CIDX_WR_CFG, *PREG_WED_WDMA_RX_CIDX_WR_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_RING_NUM               : 8;
        FIELD RX_RING_NUM               : 8;
        FIELD BASE_PTR_WIDTH            : 8;
        FIELD INDEX_WIDTH               : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_INFO, *PREG_WED_WDMA_INFO;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DRV_EN                 : 1;
        FIELD TX_DDONE_CHK              : 1;
        FIELD RX_DRV_EN                 : 1;
        FIELD RX_DRV_BUSY               : 1;
        FIELD WDMA_BT_SIZE              : 2;
        FIELD TX_WB_DDONE               : 1;
        FIELD BIG_ENDIAN                : 1;
        FIELD TX_DDONE_CHK_LAST         : 1;
        FIELD LS_CHK_BYP                : 1;
        FIELD RXDRV_DDONE2_CHK          : 1;
        FIELD TX_DRV_BURST_CFG          : 2;
        FIELD RXDRV_DISABLED_FSM_AUTO_IDLE : 1;
        FIELD DISABLE_IRQ_HANDLE_PROC   : 1;
        FIELD DISABLE_DMAD_HANDLE_PROC  : 1;
        FIELD WCOMPLETE_SEL             : 1;
        FIELD INIT_PHASE_ENABLING_RXDMA_BYPASS : 1;
        FIELD INIT_PHASE_BYPASS         : 1;
        FIELD FSM_RETURN_IDLE           : 1;
        FIELD WAIT_COHERENT_EN          : 1;
        FIELD AXI_W_AFTER_AW_EN         : 1;
        FIELD IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE : 1;
        FIELD IDLE_STATE_DMAD_SUPPLY_EN : 1;
        FIELD DYNAMIC_SKIP_DMAD_PREPARE : 1;
        FIELD DYNAMIC_DMAD_RECYCLE      : 1;
        FIELD RST_INIT_COMPLETE_FLAG    : 1;
        FIELD DYNAMIC_SKIP_DMAD_CNT_SRC : 1;
        FIELD DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD : 1;
        FIELD TX_DRV_DTX_DLY_DIS        : 1;
        FIELD RXDRV_CLKGATE_BYP         : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_GLO_CFG, *PREG_WED_WDMA_GLO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD CRX_IDX0                  : 1;
        FIELD CRX_IDX1                  : 1;
        FIELD CTX_IDX0                  : 1;
        FIELD CTX_IDX1                  : 1;
        FIELD DRX_IDX_ALL               : 1;
        FIELD rsv_21                    : 3;
        FIELD DRV_IDX0                  : 1;
        FIELD DRV_IDX1                  : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RST_IDX, *PREG_WED_WDMA_RST_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRV_IDX0                  : 1;
        FIELD DRV_IDX1                  : 1;
        FIELD rsv_2                     : 6;
        FIELD CRX_IDX0                  : 1;
        FIELD CRX_IDX1                  : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_LOAD_IDX, *PREG_WED_WDMA_LOAD_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRV_IDX0                  : 16;
        FIELD DRV_IDX1                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_LOAD_DRV_IDX, *PREG_WED_WDMA_LOAD_DRV_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX0                  : 16;
        FIELD CRX_IDX1                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_LOAD_CRX_IDX, *PREG_WED_WDMA_LOAD_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RSV_REG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_SPR, *PREG_WED_WDMA_SPR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE0                  : 1;
        FIELD TX_DONE1                  : 1;
        FIELD TX_DONE2                  : 1;
        FIELD TX_DONE3                  : 1;
        FIELD rsv_4                     : 12;
        FIELD RX_DONE0                  : 1;
        FIELD RX_DONE1                  : 1;
        FIELD rsv_18                    : 10;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_INT_STA_REC, *PREG_WED_WDMA_INT_STA_REC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INT_CLR                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_INT_CLR, *PREG_WED_WDMA_INT_CLR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INT_TRIG                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_INT_TRIG, *PREG_WED_WDMA_INT_TRIG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD POLL_PRD                  : 8;
        FIELD rsv_8                     : 4;
        FIELD POLL_MODE                 : 2;
        FIELD rsv_14                    : 2;
        FIELD SRC_SEL                   : 2;
        FIELD EXT_INCPT_DIS             : 1;
        FIELD rsv_19                    : 2;
        FIELD SUBRT_ADV                 : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_INT_CTRL, *PREG_WED_WDMA_INT_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_INT_MON, *PREG_WED_WDMA_INT_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_CFG_BASE, *PREG_WED_WDMA_CFG_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INTS                      : 16;
        FIELD GLO_CFG                   : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_OFST0, *PREG_WED_WDMA_OFST0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX0_CTRL                  : 16;
        FIELD RX0_CTRL                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_OFST1, *PREG_WED_WDMA_OFST1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_CFG_BASE_H, *PREG_WED_WDMA_CFG_BASE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_MIB, *PREG_WED_WDMA_RX0_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_MIB, *PREG_WED_WDMA_RX1_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_RECYCLE_MIB, *PREG_WED_WDMA_RX0_RECYCLE_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_RECYCLE_MIB, *PREG_WED_WDMA_RX1_RECYCLE_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX0_PROCESSED_MIB, *PREG_WED_WDMA_RX0_PROCESSED_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PKT_CNT                   : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX1_PROCESSED_MIB, *PREG_WED_WDMA_RX1_PROCESSED_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_WDMA_RX_COHERENT_MIB, *PREG_WED_WDMA_RX_COHERENT_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ENABLE                    : 9;
        FIELD rsv_9                     : 7;
        FIELD STATUS                    : 13;
        FIELD rsv_29                    : 2;
        FIELD CG_DIS                    : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_GLO_CFG, *PREG_WED_RTQM_GLO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RST                       : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_RST, *PREG_WED_RTQM_RST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD IGRS0_CNT                 : 5;
        FIELD IGRS0_CLEAR               : 1;
        FIELD IGRS0_FULL                : 1;
        FIELD IGRS0_EMPTY               : 1;
        FIELD IGRS1_CNT                 : 5;
        FIELD IGRS1_CLEAR               : 1;
        FIELD IGRS1_FULL                : 1;
        FIELD IGRS1_EMPTY               : 1;
        FIELD IGRS2_CNT                 : 5;
        FIELD IGRS2_CLEAR               : 1;
        FIELD IGRS2_FULL                : 1;
        FIELD IGRS2_EMPTY               : 1;
        FIELD IGRS3_CNT                 : 5;
        FIELD IGRS3_CLEAR               : 1;
        FIELD IGRS3_FULL                : 1;
        FIELD IGRS3_EMPTY               : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS_FIFO_CFG, *PREG_WED_RTQM_IGRS_FIFO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD P2Q_CNT                   : 11;
        FIELD rsv_11                    : 1;
        FIELD P2Q_GFFDIS                : 1;
        FIELD P2Q_CLEAR                 : 1;
        FIELD P2Q_FULL                  : 1;
        FIELD P2Q_EMPTY                 : 1;
        FIELD A2Q_CNT                   : 5;
        FIELD rsv_21                    : 3;
        FIELD A2Q_CLEAR                 : 1;
        FIELD A2Q_FULL                  : 1;
        FIELD A2Q_EMPTY                 : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_PFDBK_FIFO_CFG, *PREG_WED_RTQM_PFDBK_FIFO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 1;
        FIELD PREF_0_CLEAR              : 1;
        FIELD PREF_0_FULL               : 1;
        FIELD PREF_0_EMPTY              : 1;
        FIELD rsv_4                     : 1;
        FIELD PREF_1_CLEAR              : 1;
        FIELD PREF_1_FULL               : 1;
        FIELD PREF_1_EMPTY              : 1;
        FIELD rsv_8                     : 1;
        FIELD PREF_2_CLEAR              : 1;
        FIELD PREF_2_FULL               : 1;
        FIELD PREF_2_EMPTY              : 1;
        FIELD rsv_12                    : 1;
        FIELD PREF_3_CLEAR              : 1;
        FIELD PREF_3_FULL               : 1;
        FIELD PREF_3_EMPTY              : 1;
        FIELD rsv_16                    : 1;
        FIELD PREF_4_CLEAR              : 1;
        FIELD PREF_4_FULL               : 1;
        FIELD PREF_4_EMPTY              : 1;
        FIELD rsv_20                    : 1;
        FIELD PREF_5_CLEAR              : 1;
        FIELD PREF_5_FULL               : 1;
        FIELD PREF_5_EMPTY              : 1;
        FIELD rsv_24                    : 1;
        FIELD PREF_6_CLEAR              : 1;
        FIELD PREF_6_FULL               : 1;
        FIELD PREF_6_EMPTY              : 1;
        FIELD rsv_28                    : 1;
        FIELD PREF_7_CLEAR              : 1;
        FIELD PREF_7_FULL               : 1;
        FIELD PREF_7_EMPTY              : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_PREF_FIFO_CFG, *PREG_WED_RTQM_PREF_FIFO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEST_FORCE                : 3;
        FIELD FORCE_DROP                : 1;
        FIELD DEST_CHK_V0               : 3;
        FIELD rsv_7                     : 1;
        FIELD DEST_CHK_V1               : 8;
        FIELD SCH_REQ_ANY_TIME          : 1;
        FIELD rsv_17                    : 1;
        FIELD DMAD_CLR_PPE_FLD          : 2;
        FIELD DMAD_MOD_EN               : 7;
        FIELD DMAD_MOD_METHOD           : 1;
        FIELD HOST_RING_SRC             : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_CFG0, *PREG_WED_RTQM_IGRS0_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 6;
        FIELD DMAD_MOD_TO_HOST          : 1;
        FIELD DMAD_MOD_RING_INFO        : 2;
        FIELD DMAD_MOD_LS               : 1;
        FIELD DMAD_MOD_DDONE            : 1;
        FIELD DMAD_MOD_CRSN             : 5;
        FIELD DMAD_MOD_PPE_ENTRY        : 15;
        FIELD DMAD_MOD_PPE_VLD          : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_CFG1, *PREG_WED_RTQM_IGRS0_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_I2HW_DMAD_CNT, *PREG_WED_RTQM_IGRS0_I2HW_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_I2H0_DMAD_CNT, *PREG_WED_RTQM_IGRS0_I2H0_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_I2H1_DMAD_CNT, *PREG_WED_RTQM_IGRS0_I2H1_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_I2HW_PKT_CNT, *PREG_WED_RTQM_IGRS0_I2HW_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_I2H0_PKT_CNT, *PREG_WED_RTQM_IGRS0_I2H0_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_I2H1_PKT_CNT, *PREG_WED_RTQM_IGRS0_I2H1_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_FDROP_CNT, *PREG_WED_RTQM_IGRS0_FDROP_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CURR_DMAD_EOP_MON         : 1;
        FIELD CURR_DMAD_SOP_MON         : 1;
        FIELD NEXT_DMAD_SOP_MON         : 1;
        FIELD XOP_CLR                   : 1;
        FIELD SCH_RLS_SET               : 1;
        FIELD SCH_RLS_MANUAL            : 1;
        FIELD SCH_REQ_MON               : 3;
        FIELD SCH_RESULT_MON            : 3;
        FIELD SCH_RESULT_RLS_MON        : 3;
        FIELD SEQ_ID_ENABLE             : 1;
        FIELD SEQ_ID_MON                : 15;
        FIELD SEQ_ID_CLR                : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS0_DBG, *PREG_WED_RTQM_IGRS0_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEST_FORCE                : 3;
        FIELD FORCE_DROP                : 1;
        FIELD DEST_CHK_V0               : 3;
        FIELD rsv_7                     : 1;
        FIELD DEST_CHK_V1               : 8;
        FIELD SCH_REQ_ANY_TIME          : 1;
        FIELD rsv_17                    : 1;
        FIELD DMAD_CLR_PPE_FLD          : 2;
        FIELD DMAD_MOD_EN               : 7;
        FIELD DMAD_MOD_METHOD           : 1;
        FIELD HOST_RING_SRC             : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_CFG0, *PREG_WED_RTQM_IGRS1_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 6;
        FIELD DMAD_MOD_TO_HOST          : 1;
        FIELD DMAD_MOD_RING_INFO        : 2;
        FIELD DMAD_MOD_LS               : 1;
        FIELD DMAD_MOD_DDONE            : 1;
        FIELD DMAD_MOD_CRSN             : 5;
        FIELD DMAD_MOD_PPE_ENTRY        : 15;
        FIELD DMAD_MOD_PPE_VLD          : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_CFG1, *PREG_WED_RTQM_IGRS1_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_I2HW_DMAD_CNT, *PREG_WED_RTQM_IGRS1_I2HW_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_I2H0_DMAD_CNT, *PREG_WED_RTQM_IGRS1_I2H0_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_I2H1_DMAD_CNT, *PREG_WED_RTQM_IGRS1_I2H1_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_I2HW_PKT_CNT, *PREG_WED_RTQM_IGRS1_I2HW_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_I2H0_PKT_CNT, *PREG_WED_RTQM_IGRS1_I2H0_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_I2H1_PKT_CNT, *PREG_WED_RTQM_IGRS1_I2H1_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_FDROP_CNT, *PREG_WED_RTQM_IGRS1_FDROP_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CURR_DMAD_EOP_MON         : 1;
        FIELD CURR_DMAD_SOP_MON         : 1;
        FIELD NEXT_DMAD_SOP_MON         : 1;
        FIELD XOP_CLR                   : 1;
        FIELD SCH_RLS_SET               : 1;
        FIELD SCH_RLS_MANUAL            : 1;
        FIELD SCH_REQ_MON               : 3;
        FIELD SCH_RESULT_MON            : 3;
        FIELD SCH_RESULT_RLS_MON        : 3;
        FIELD SEQ_ID_ENABLE             : 1;
        FIELD SEQ_ID_MON                : 15;
        FIELD SEQ_ID_CLR                : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS1_DBG, *PREG_WED_RTQM_IGRS1_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEST_FORCE                : 3;
        FIELD FORCE_DROP                : 1;
        FIELD DEST_CHK_V0               : 3;
        FIELD rsv_7                     : 1;
        FIELD DEST_CHK_V1               : 8;
        FIELD SCH_REQ_ANY_TIME          : 1;
        FIELD rsv_17                    : 1;
        FIELD DMAD_CLR_PPE_FLD          : 2;
        FIELD DMAD_MOD_EN               : 7;
        FIELD DMAD_MOD_METHOD           : 1;
        FIELD HOST_RING_SRC             : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_CFG0, *PREG_WED_RTQM_IGRS2_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 6;
        FIELD DMAD_MOD_TO_HOST          : 1;
        FIELD DMAD_MOD_RING_INFO        : 2;
        FIELD DMAD_MOD_LS               : 1;
        FIELD DMAD_MOD_DDONE            : 1;
        FIELD DMAD_MOD_CRSN             : 5;
        FIELD DMAD_MOD_PPE_ENTRY        : 15;
        FIELD DMAD_MOD_PPE_VLD          : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_CFG1, *PREG_WED_RTQM_IGRS2_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_I2HW_DMAD_CNT, *PREG_WED_RTQM_IGRS2_I2HW_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_I2H0_DMAD_CNT, *PREG_WED_RTQM_IGRS2_I2H0_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_I2H1_DMAD_CNT, *PREG_WED_RTQM_IGRS2_I2H1_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_I2HW_PKT_CNT, *PREG_WED_RTQM_IGRS2_I2HW_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_I2H0_PKT_CNT, *PREG_WED_RTQM_IGRS2_I2H0_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_I2H1_PKT_CNT, *PREG_WED_RTQM_IGRS2_I2H1_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_FDROP_CNT, *PREG_WED_RTQM_IGRS2_FDROP_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CURR_DMAD_EOP_MON         : 1;
        FIELD CURR_DMAD_SOP_MON         : 1;
        FIELD NEXT_DMAD_SOP_MON         : 1;
        FIELD XOP_CLR                   : 1;
        FIELD SCH_RLS_SET               : 1;
        FIELD SCH_RLS_MANUAL            : 1;
        FIELD SCH_REQ_MON               : 3;
        FIELD SCH_RESULT_MON            : 3;
        FIELD SCH_RESULT_RLS_MON        : 3;
        FIELD SEQ_ID_ENABLE             : 1;
        FIELD SEQ_ID_MON                : 15;
        FIELD SEQ_ID_CLR                : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS2_DBG, *PREG_WED_RTQM_IGRS2_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEST_FORCE                : 3;
        FIELD FORCE_DROP                : 1;
        FIELD DEST_CHK_V0               : 3;
        FIELD rsv_7                     : 1;
        FIELD DEST_CHK_V1               : 8;
        FIELD SCH_REQ_ANY_TIME          : 1;
        FIELD rsv_17                    : 1;
        FIELD DMAD_CLR_PPE_FLD          : 2;
        FIELD DMAD_MOD_EN               : 7;
        FIELD DMAD_MOD_METHOD           : 1;
        FIELD HOST_RING_SRC             : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_CFG0, *PREG_WED_RTQM_IGRS3_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 6;
        FIELD DMAD_MOD_TO_HOST          : 1;
        FIELD DMAD_MOD_RING_INFO        : 2;
        FIELD DMAD_MOD_LS               : 1;
        FIELD DMAD_MOD_DDONE            : 1;
        FIELD DMAD_MOD_CRSN             : 5;
        FIELD DMAD_MOD_PPE_ENTRY        : 15;
        FIELD DMAD_MOD_PPE_VLD          : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_CFG1, *PREG_WED_RTQM_IGRS3_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_I2HW_DMAD_CNT, *PREG_WED_RTQM_IGRS3_I2HW_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_I2H0_DMAD_CNT, *PREG_WED_RTQM_IGRS3_I2H0_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_I2H1_DMAD_CNT, *PREG_WED_RTQM_IGRS3_I2H1_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_I2HW_PKT_CNT, *PREG_WED_RTQM_IGRS3_I2HW_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_I2H0_PKT_CNT, *PREG_WED_RTQM_IGRS3_I2H0_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_I2H1_PKT_CNT, *PREG_WED_RTQM_IGRS3_I2H1_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_FDROP_CNT, *PREG_WED_RTQM_IGRS3_FDROP_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CURR_DMAD_EOP_MON         : 1;
        FIELD CURR_DMAD_SOP_MON         : 1;
        FIELD NEXT_DMAD_SOP_MON         : 1;
        FIELD XOP_CLR                   : 1;
        FIELD SCH_RLS_SET               : 1;
        FIELD SCH_RLS_MANUAL            : 1;
        FIELD SCH_REQ_MON               : 3;
        FIELD SCH_RESULT_MON            : 3;
        FIELD SCH_RESULT_RLS_MON        : 3;
        FIELD SEQ_ID_ENABLE             : 1;
        FIELD SEQ_ID_MON                : 15;
        FIELD SEQ_ID_CLR                : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_IGRS3_DBG, *PREG_WED_RTQM_IGRS3_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD EGRS0_ITF0_WEIGHT         : 2;
        FIELD EGRS0_ITF1_WEIGHT         : 2;
        FIELD EGRS0_ITF2_WEIGHT         : 2;
        FIELD EGRS0_ITF3_WEIGHT         : 2;
        FIELD EGRS1_ITF0_WEIGHT         : 2;
        FIELD EGRS1_ITF1_WEIGHT         : 2;
        FIELD EGRS1_ITF2_WEIGHT         : 2;
        FIELD EGRS1_ITF3_WEIGHT         : 2;
        FIELD EGRS2_ITF0_WEIGHT         : 2;
        FIELD EGRS2_ITF1_WEIGHT         : 2;
        FIELD EGRS2_ITF2_WEIGHT         : 2;
        FIELD EGRS2_ITF3_WEIGHT         : 2;
        FIELD EGRS3_ITF0_WEIGHT         : 2;
        FIELD EGRS3_ITF1_WEIGHT         : 2;
        FIELD EGRS3_ITF2_WEIGHT         : 2;
        FIELD EGRS3_ITF3_WEIGHT         : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_EGRS_WEIGHT, *PREG_WED_RTQM_EGRS_WEIGHT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FORCE_DROP                : 1;
        FIELD ENQ_ERR_NO_ENQ            : 1;
        FIELD ENQ_ERR_AUTO_FREE         : 1;
        FIELD DMAD_TKID_INS             : 1;
        FIELD SEQ_ID_ENABLE             : 1;
        FIELD SEQ_ID_CLR                : 1;
        FIELD RXDMAD_OFST_UNIT          : 2;
        FIELD TXDMAD_OFLD               : 4;
        FIELD TXDMAD_FPORT              : 4;
        FIELD SEQ_ID_MON                : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_CFG0, *PREG_WED_RTQM_ENQ_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_I2Q_DMAD_CNT, *PREG_WED_RTQM_ENQ_I2Q_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_I2N_DMAD_CNT, *PREG_WED_RTQM_ENQ_I2N_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_I2Q_PKT_CNT, *PREG_WED_RTQM_ENQ_I2Q_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_I2N_PKT_CNT, *PREG_WED_RTQM_ENQ_I2N_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_USED_ENTRY_CNT, *PREG_WED_RTQM_ENQ_USED_ENTRY_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ENQ_ERR_CNT, *PREG_WED_RTQM_ENQ_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FORCE_DROP                : 1;
        FIELD DMAD_PVLD_UPD             : 1;
        FIELD SG_DMAD_PFDBK_MERGE       : 1;
        FIELD SG_DMAD_PFDBK_DEFAULT     : 1;
        FIELD DMAD_PFDBK_MERGE_DIS      : 1;
        FIELD DEQ_ERR_INS_ADDR          : 1;
        FIELD DEQ_ERR_DEST              : 2;
        FIELD DEQ_FSM                   : 1;
        FIELD DEQ_FSM_FORCE_IDLE        : 1;
        FIELD DMAD_MOD_EN               : 6;
        FIELD DMAD_MOD_RING_INFO        : 2;
        FIELD DMAD_MOD_LS               : 1;
        FIELD DMAD_MOD_DDONE            : 1;
        FIELD DMAD_MOD_CRSN             : 5;
        FIELD DMAD_MOD_PPE_VLD          : 1;
        FIELD DMAD_MOD_TO_HOST          : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_CFG0, *PREG_WED_RTQM_DEQ_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_DQ_DMAD_CNT, *PREG_WED_RTQM_DEQ_DQ_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_Q2I_DMAD_CNT, *PREG_WED_RTQM_DEQ_Q2I_DMAD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_DQ_PKT_CNT, *PREG_WED_RTQM_DEQ_DQ_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_Q2I_PKT_CNT, *PREG_WED_RTQM_DEQ_Q2I_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_USED_PFDBK_CNT, *PREG_WED_RTQM_DEQ_USED_PFDBK_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_ERR_CNT, *PREG_WED_RTQM_DEQ_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CURR_DMAD_EOP_MON         : 1;
        FIELD CURR_DMAD_SOP_MON         : 1;
        FIELD NEXT_DMAD_SOP_MON         : 1;
        FIELD XOP_CLR                   : 1;
        FIELD rsv_4                     : 2;
        FIELD DEQ_ERR_MOD_EN            : 4;
        FIELD DEQ_ERR_MOD_LS            : 1;
        FIELD DEQ_ERR_MOD_CRSN          : 5;
        FIELD DEQ_ERR_MOD_PPE_ENTRY     : 15;
        FIELD DEQ_ERR_MOD_PPE_VLD       : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DEQ_DBG, *PREG_WED_RTQM_DEQ_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD IGRS0_FIFO_W_ERR          : 1;
        FIELD IGRS0_FIFO_R_ERR          : 1;
        FIELD IGRS1_FIFO_W_ERR          : 1;
        FIELD IGRS1_FIFO_R_ERR          : 1;
        FIELD IGRS2_FIFO_W_ERR          : 1;
        FIELD IGRS2_FIFO_R_ERR          : 1;
        FIELD IGRS3_FIFO_W_ERR          : 1;
        FIELD IGRS3_FIFO_R_ERR          : 1;
        FIELD ENQ_ERR                   : 1;
        FIELD DEQ_ERR                   : 1;
        FIELD PFDBK_FIFO_W_ERR          : 1;
        FIELD PFDBK_FIFO_R_ERR          : 1;
        FIELD PFDBK_AGE_FIFO_W_ERR      : 1;
        FIELD PFDBK_AGE_FIFO_R_ERR      : 1;
        FIELD QFREE_SRCH_FIFO_W_ERR     : 1;
        FIELD QFREE_SRCH_FIFO_R_ERR     : 1;
        FIELD AGE_SRCH_FIFO_W_ERR       : 1;
        FIELD AGE_SRCH_FIFO_R_ERR       : 1;
        FIELD RRO_ITF_FIFO_W_ERR        : 1;
        FIELD PN_ITF_FIFO_W_ERR         : 1;
        FIELD CR_ITF_FIFO_W_ERR         : 1;
        FIELD Q2BM_ITF_FIFO_R_ERR       : 1;
        FIELD Q2H0_ITF_FIFO_W_ERR       : 1;
        FIELD Q2H1_ITF_FIFO_W_ERR       : 1;
        FIELD PFDBK_ITF_FIFO_R_ERR      : 1;
        FIELD Q2N_ITF_FIFO_W_ERR        : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_ERR_STS, *PREG_WED_RTQM_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ENQ_ERR_CHK               : 4;
        FIELD DEQ_ERR_CHK               : 4;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_CFG0, *PREG_WED_RTQM_QUEUE_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WR                        : 1;
        FIELD RD                        : 1;
        FIELD rsv_2                     : 2;
        FIELD FLAG_ENQUEUE_SET_ALL      : 1;
        FIELD FLAG_FETCHABLE_SET_ALL    : 1;
        FIELD FLAG_AGE_SET_ALL          : 1;
        FIELD FLAG_LINK_HEAD_SET_ALL    : 1;
        FIELD FLAG_ENQUEUE_CLR_ALL      : 1;
        FIELD FLAG_FETCHABLE_CLR_ALL    : 1;
        FIELD FLAG_AGE_CLR_ALL          : 1;
        FIELD FLAG_LINK_HEAD_CLR_ALL    : 1;
        FIELD WR_CMD_ERR                : 10;
        FIELD ADDRESS                   : 10;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_CFG, *PREG_WED_RTQM_QUEUE_ACC_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 1;
        FIELD RROQM_BUSY                : 1;
        FIELD rsv_2                     : 6;
        FIELD AWULTRA                   : 2;
        FIELD rsv_10                    : 2;
        FIELD ARULTRA                   : 2;
        FIELD rsv_14                    : 2;
        FIELD WAIT_MOD_COH_EN           : 1;
        FIELD WAIT_FDBK_COH_EN          : 1;
        FIELD rsv_18                    : 6;
        FIELD RROQM_BYPASS              : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_GLO_CFG, *PREG_WED_RROQM_GLO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MIOD                      : 1;
        FIELD rsv_1                     : 3;
        FIELD FDBK                      : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_RST_IDX, *PREG_WED_RROQM_RST_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MID_RDY_MAX_PTIME         : 8;
        FIELD MID_RDY_MAX_PINT          : 7;
        FIELD MID_RDY_EN                : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_DLY_INT, *PREG_WED_RROQM_DLY_INT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_WR0, *PREG_WED_RTQM_QUEUE_ACC_WR0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_WR1, *PREG_WED_RTQM_QUEUE_ACC_WR1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_WR2, *PREG_WED_RTQM_QUEUE_ACC_WR2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_WR3, *PREG_WED_RTQM_QUEUE_ACC_WR3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LINK                      : 10;
        FIELD DATA_RSV                  : 6;
        FIELD WR_CMD                    : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_WR4, *PREG_WED_RTQM_QUEUE_ACC_WR4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_RD0, *PREG_WED_RTQM_QUEUE_ACC_RD0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_RD1, *PREG_WED_RTQM_QUEUE_ACC_RD1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_RD2, *PREG_WED_RTQM_QUEUE_ACC_RD2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_RD3, *PREG_WED_RTQM_QUEUE_ACC_RD3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LINK                      : 10;
        FIELD DATA_RSV                  : 6;
        FIELD FLAGS                     : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_ACC_RD4, *PREG_WED_RTQM_QUEUE_ACC_RD4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_QUEUE_CNT, *PREG_WED_RTQM_QUEUE_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DBG_MON_SEL               : 8;
        FIELD DBG_ENQ_WR                : 1;
        FIELD DBG_ENQ_VLD               : 1;
        FIELD PPE_FDBK_DROP             : 1;
        FIELD rsv_11                    : 1;
        FIELD ENQ_DDONE_CTRL            : 2;
        FIELD ENQ_DDONE_DW1_CTRL        : 2;
        FIELD ENQ_DDONE_DW2_CTRL        : 2;
        FIELD ENQ_DDONE2_CTRL           : 2;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_CFG, *PREG_WED_RTQM_DBG_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MIOD_CTRL0, *PREG_WED_RROQM_MIOD_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_EXT              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MIOD_CTRL1, *PREG_WED_RROQM_MIOD_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TAKE_IDX                  : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MIOD_CTRL2, *PREG_WED_RROQM_MIOD_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PUT_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MIOD_CTRL3, *PREG_WED_RROQM_MIOD_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_CTRL0, *PREG_WED_RROQM_FDBK_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_EXT              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_CTRL1, *PREG_WED_RROQM_FDBK_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_CTRL2, *PREG_WED_RROQM_FDBK_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD PREFETCH_IDX              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_CTRL3, *PREG_WED_RROQM_FDBK_CTRL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_MON0, *PREG_WED_RTQM_DBG_MON0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_MON1, *PREG_WED_RTQM_DBG_MON1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_MON2, *PREG_WED_RTQM_DBG_MON2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_MON3, *PREG_WED_RTQM_DBG_MON3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_ENQ_DATA0, *PREG_WED_RTQM_DBG_ENQ_DATA0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_ENQ_DATA1, *PREG_WED_RTQM_DBG_ENQ_DATA1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_ENQ_DATA2, *PREG_WED_RTQM_DBG_ENQ_DATA2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD                      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_DBG_ENQ_DATA3, *PREG_WED_RTQM_DBG_ENQ_DATA3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQ_BASE_L, *PREG_WED_RROQ_BASE_L;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQ_BASE_H, *PREG_WED_RROQ_BASE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FSM                       : 2;
        FIELD FSM_FORCE_IDLE            : 1;
        FIELD FLUSH_EN                  : 1;
        FIELD rsv_4                     : 4;
        FIELD DFDBK_CSRN                : 5;
        FIELD rsv_13                    : 3;
        FIELD DFDBK_PPE_ENTRY           : 15;
        FIELD DFDBK_TO_HOST             : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_AGE_CFG0, *PREG_WED_RTQM_AGE_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MID_DW                    : 6;
        FIELD rsv_6                     : 2;
        FIELD MOD_DW                    : 6;
        FIELD rsv_14                    : 2;
        FIELD MIOD_ENTRY_DW             : 7;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MIOD_CFG, *PREG_WED_RROQM_MIOD_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD AGE_TIME                  : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_AGE_CFG1, *PREG_WED_RTQM_AGE_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 1;
        FIELD PREF_0_CLEAR              : 1;
        FIELD PREF_0_FULL               : 1;
        FIELD PREF_0_EMPTY              : 1;
        FIELD rsv_4                     : 1;
        FIELD PREF_1_CLEAR              : 1;
        FIELD PREF_1_FULL               : 1;
        FIELD PREF_1_EMPTY              : 1;
        FIELD rsv_8                     : 1;
        FIELD PREF_2_CLEAR              : 1;
        FIELD PREF_2_FULL               : 1;
        FIELD PREF_2_EMPTY              : 1;
        FIELD rsv_12                    : 1;
        FIELD PREF_3_CLEAR              : 1;
        FIELD PREF_3_FULL               : 1;
        FIELD PREF_3_EMPTY              : 1;
        FIELD rsv_16                    : 1;
        FIELD PREF_4_CLEAR              : 1;
        FIELD PREF_4_FULL               : 1;
        FIELD PREF_4_EMPTY              : 1;
        FIELD rsv_20                    : 1;
        FIELD PREF_5_CLEAR              : 1;
        FIELD PREF_5_FULL               : 1;
        FIELD PREF_5_EMPTY              : 1;
        FIELD rsv_24                    : 1;
        FIELD PREF_6_CLEAR              : 1;
        FIELD PREF_6_FULL               : 1;
        FIELD PREF_6_EMPTY              : 1;
        FIELD rsv_28                    : 1;
        FIELD PREF_7_CLEAR              : 1;
        FIELD PREF_7_FULL               : 1;
        FIELD PREF_7_EMPTY              : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_AGE_FIFO_CFG, *PREG_WED_RTQM_AGE_FIFO_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_AGE_PKT_CNT, *PREG_WED_RTQM_AGE_PKT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RTQM_PFDBK_CNT, *PREG_WED_RTQM_PFDBK_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CMD_EN                    : 4;
        FIELD rsv_4                     : 4;
        FIELD PAGE                      : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_REC_CTRL, *PREG_WED_RROQM_FDBK_REC_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CMD                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_REC0, *PREG_WED_RROQM_FDBK_REC0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CMD                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_REC1, *PREG_WED_RROQM_FDBK_REC1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CMD                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_REC2, *PREG_WED_RROQM_FDBK_REC2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CMD                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_REC3, *PREG_WED_RROQM_FDBK_REC3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MID_MIB, *PREG_WED_RROQM_MID_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MOD_MIB, *PREG_WED_RROQM_MOD_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_MOD_COH_MIB, *PREG_WED_RROQM_MOD_COH_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_MIB, *PREG_WED_RROQM_FDBK_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_COH_MIB, *PREG_WED_RROQM_FDBK_COH_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_IND_MIB, *PREG_WED_RROQM_FDBK_IND_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_ENQ_MIB, *PREG_WED_RROQM_FDBK_ENQ_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_ANC_MIB, *PREG_WED_RROQM_FDBK_ANC_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CNT                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RROQM_FDBK_ANC2H_MIB, *PREG_WED_RROQM_FDBK_ANC2H_MIB;

typedef PACKING union
{
    PACKING struct
    {
        FIELD L_BUF_TH                  : 16;
        FIELD H_BUF_TH                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_DYN_ALLOC_TH, *PREG_WED_TX_BM_DYN_ALLOC_TH;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_ADDSUB_IDX_CNT         : 16;
        FIELD SW_SUB_BUF_REQ            : 1;
        FIELD SW_ADD_BUF_REQ            : 1;
        FIELD SW_SUB_RDY                : 1;
        FIELD rsv_19                    : 9;
        FIELD SW_SUB_UNDERFLOW_ERR      : 1;
        FIELD SW_ADD_OVERFLOW_ERR       : 1;
        FIELD HW_ADDSUB_ACK             : 1;
        FIELD HW_SUB_pause              : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_DYN_ALLOC_CFG, *PREG_WED_TX_BM_DYN_ALLOC_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_ADD_BASE, *PREG_WED_TX_BM_ADD_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_ADD_IDX_REMAIN         : 16;
        FIELD HEAD2_IDX                 : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_ADD_PTR, *PREG_WED_TX_BM_ADD_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_TOTAL_RX_BM_CNT        : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_TOTAL_DMAD_IDX, *PREG_WED_TX_BM_TOTAL_DMAD_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_TX_BM_ADD_BASE_H, *PREG_WED_TX_BM_ADD_BASE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SDL0                      : 14;
        FIELD rsv_14                    : 2;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_RX_DMAD, *PREG_WED_RX_BM_RX_DMAD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_BASE, *PREG_WED_RX_BM_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_TAIL_IDX               : 16;
        FIELD INIT_SW_TAIL_IDX          : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_INIT_PTR, *PREG_WED_RX_BM_INIT_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TAIL_IDX                  : 16;
        FIELD HEAD_IDX                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_PTR, *PREG_WED_RX_BM_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BYTE_LEN                  : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_BLEN, *PREG_WED_RX_BM_BLEN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD DMAD_NUM                  : 5;
        FIELD rsv_13                    : 3;
        FIELD DMAD0_NUM                 : 2;
        FIELD rsv_18                    : 2;
        FIELD DMAD0_VLD                 : 1;
        FIELD DMAD0_RD                  : 1;
        FIELD rsv_22                    : 2;
        FIELD DMAD1_NUM                 : 2;
        FIELD rsv_26                    : 2;
        FIELD DMAD1_VLD                 : 1;
        FIELD DMAD1_RD                  : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_STS, *PREG_WED_RX_BM_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD_EXCHG                : 16;
        FIELD rsv_16                    : 12;
        FIELD DMAD_EXCHG_VLD            : 1;
        FIELD DMAD_EXCHG_RD             : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_INTF2, *PREG_WED_RX_BM_INTF2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD_FREE                 : 16;
        FIELD rsv_16                    : 12;
        FIELD DMAD_FREE_VLD             : 1;
        FIELD DMAD_FREE_RD              : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_INTF, *PREG_WED_RX_BM_INTF;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD_AXI_RD_LESS          : 1;
        FIELD DMAD_AXI_RD_MORE          : 1;
        FIELD DMAD_FULL_AXI_RD          : 1;
        FIELD DMAD_AXI_RD_OVER_TAIL     : 1;
        FIELD rsv_4                     : 3;
        FIELD FREE_AT_EMPTY             : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_ERR_STS, *PREG_WED_RX_BM_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREE_AXI_RESP_SEL         : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_OPT_CTRL, *PREG_WED_RX_BM_OPT_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD L_BUF_TH                  : 16;
        FIELD H_BUF_TH                  : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_DYN_ALLOC_TH, *PREG_WED_RX_BM_DYN_ALLOC_TH;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_ADDSUB_IDX_CNT         : 16;
        FIELD SW_SUB_BUF_REQ            : 1;
        FIELD SW_ADD_BUF_REQ            : 1;
        FIELD SW_SUB_RDY                : 1;
        FIELD rsv_19                    : 9;
        FIELD SW_SUB_UNDERFLOW_ERR      : 1;
        FIELD SW_ADD_OVERFLOW_ERR       : 1;
        FIELD HW_ADDSUB_ACK             : 1;
        FIELD HW_SUB_pause              : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_DYN_ALLOC_CFG, *PREG_WED_RX_BM_DYN_ALLOC_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_ADD_BASE, *PREG_WED_RX_BM_ADD_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_ADD_IDX_REMAIN         : 16;
        FIELD HEAD2_IDX                 : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_ADD_PTR, *PREG_WED_RX_BM_ADD_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_TOTAL_RX_BM_CNT        : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_TOTAL_DMAD_IDX, *PREG_WED_RX_BM_TOTAL_DMAD_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_BM_ADD_BASE_H, *PREG_WED_RX_BM_ADD_BASE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD SW_PROC_IDX               : 12;
        FIELD MAGIC_CNT                 : 3;
        FIELD VLD                       : 1;
    } Bits;
    UINT32 Raw;
} REG_RRO_IND_CMD_0_SIGNATURE, *PREG_RRO_IND_CMD_0_SIGNATURE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PROC_IDX                  : 12;
        FIELD rsv_12                    : 4;
        FIELD PREFETCH_IND_CMD_FREE_CNT : 4;
        FIELD rsv_20                    : 8;
        FIELD MAGIC_CNT                 : 3;
        FIELD INIT_MAGIC_CNT_PROC_IDX   : 1;
    } Bits;
    UINT32 Raw;
} REG_IND_CMD_0_CTRL_0, *PREG_IND_CMD_0_CTRL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 4;
        FIELD RRO_IND_CMD_BASE_L        : 28;
    } Bits;
    UINT32 Raw;
} REG_IND_CMD_0_CTRL_1, *PREG_IND_CMD_0_CTRL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 4;
        FIELD MAX_CNT_11_4              : 8;
        FIELD rsv_12                    : 4;
        FIELD RRO_IND_CMD_BASE_M        : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_IND_CMD_0_CTRL_2, *PREG_IND_CMD_0_CTRL_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ACK_SN_BASE_0_L           : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_CONF_0, *PREG_RRO_CONF_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PARTICULAR_SE_ID          : 12;
        FIELD rsv_12                    : 4;
        FIELD ACK_SN_BASE_0_M           : 4;
        FIELD rsv_20                    : 9;
        FIELD MAX_WIN_SZ                : 3;
    } Bits;
    UINT32 Raw;
} REG_RRO_CONF_1, *PREG_RRO_CONF_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PARTICULAR_SE_ID_ADDR_BASE_L : 32;
    } Bits;
    UINT32 Raw;
} REG_ADDR_ELEM_CONF_0, *PREG_ADDR_ELEM_CONF_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PARTICULAR_SE_ID_ADDR_BASE_M : 4;
        FIELD rsv_4                     : 4;
        FIELD CLR_ADDR_ELEM_WSTRB       : 8;
        FIELD PREFETCH_ADDR_ELEM_FREE_CNT : 4;
        FIELD rsv_20                    : 10;
        FIELD CLR_ADDR_ELEM_AXI_OP      : 2;
    } Bits;
    UINT32 Raw;
} REG_ADDR_ELEM_CONF_1, *PREG_ADDR_ELEM_CONF_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_TBL_OFST             : 7;
        FIELD rsv_7                     : 21;
        FIELD RD_RDY                    : 1;
        FIELD WR_RDY                    : 1;
        FIELD RD                        : 1;
        FIELD WR                        : 1;
    } Bits;
    UINT32 Raw;
} REG_ADDR_ELEM_BASE_TBL_CONF, *PREG_ADDR_ELEM_BASE_TBL_CONF;

typedef PACKING union
{
    PACKING struct
    {
        FIELD W_DATA                    : 32;
    } Bits;
    UINT32 Raw;
} REG_ADDR_ELEM_BASE_TBL_WDATA, *PREG_ADDR_ELEM_BASE_TBL_WDATA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD R_DATA                    : 32;
    } Bits;
    UINT32 Raw;
} REG_ADDR_ELEM_BASE_TBL_RDATA, *PREG_ADDR_ELEM_BASE_TBL_RDATA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SE_ID                     : 12;
        FIELD rsv_12                    : 16;
        FIELD PN_RD_RDY                 : 1;
        FIELD PN_WR_RDY                 : 1;
        FIELD PN_RD                     : 1;
        FIELD PN_WR                     : 1;
    } Bits;
    UINT32 Raw;
} REG_PN_CONF_0, *PREG_PN_CONF_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PN_31_0                   : 32;
    } Bits;
    UINT32 Raw;
} REG_PN_CONF_WDATA_L, *PREG_PN_CONF_WDATA_L;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PN_47_32                  : 16;
        FIELD NEED_CHECK_PN             : 1;
        FIELD IS_FIRST                  : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_PN_CONF_WDATA_M, *PREG_PN_CONF_WDATA_M;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PN_31_0                   : 32;
    } Bits;
    UINT32 Raw;
} REG_PN_CONF_RDATA_L, *PREG_PN_CONF_RDATA_L;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PN_47_32                  : 16;
        FIELD NEED_CHECK_PN             : 1;
        FIELD IS_FIRST                  : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_PN_CONF_RDATA_M, *PREG_PN_CONF_RDATA_M;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSDU_PG_ADDR_R0_L         : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_RING0_CFG0, *PREG_RRO_MSDU_PG_RING0_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSDU_PG_ADDR_R0_M         : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_RING0_CFG1, *PREG_RRO_MSDU_PG_RING0_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSDU_PG_ADDR_R1_L         : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_RING1_CFG0, *PREG_RRO_MSDU_PG_RING1_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSDU_PG_ADDR_R1_M         : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_RING1_CFG1, *PREG_RRO_MSDU_PG_RING1_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSDU_PG_ADDR_R2_L         : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_RING2_CFG0, *PREG_RRO_MSDU_PG_RING2_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MSDU_PG_ADDR_R2_M         : 4;
        FIELD rsv_4                     : 4;
        FIELD POLL_CYCLE                : 8;
        FIELD R0_INT_FLAG               : 1;
        FIELD R1_INT_FLAG               : 1;
        FIELD R2_INT_FLAG               : 1;
        FIELD FORCE_IDLE_WHEN_DRV_ERROR : 1;
        FIELD INIT_COMPLETE_FLAG        : 1;
        FIELD POLL_FLAG                 : 1;
        FIELD BUSY_FLAG                 : 1;
        FIELD ERR_LAT_OPT               : 1;
        FIELD DMAD_DBG_EN               : 1;
        FIELD DRV_DBG_CLR               : 1;
        FIELD DRV_CLR                   : 1;
        FIELD INIT_FLAG_CLR             : 1;
        FIELD FORCE_RETURN_IDLE         : 1;
        FIELD FORCE_IDLE_WHEN_DRV_DISABLE : 1;
        FIELD INT_PROC_DISABLE          : 1;
        FIELD DRV_EN                    : 1;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_RING2_CFG1, *PREG_RRO_MSDU_PG_RING2_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR_L                : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_0_CTRL0, *PREG_RRO_MSDU_PG_0_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_M                : 8;
        FIELD rsv_24                    : 4;
        FIELD MAGIC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_0_CTRL1, *PREG_RRO_MSDU_PG_0_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_0_CTRL2, *PREG_RRO_MSDU_PG_0_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR_L                : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_1_CTRL0, *PREG_RRO_MSDU_PG_1_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_M                : 8;
        FIELD rsv_24                    : 4;
        FIELD MAGIC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_1_CTRL1, *PREG_RRO_MSDU_PG_1_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_1_CTRL2, *PREG_RRO_MSDU_PG_1_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BASE_PTR_L                : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_2_CTRL0, *PREG_RRO_MSDU_PG_2_CTRL0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_CNT                   : 12;
        FIELD rsv_12                    : 4;
        FIELD BASE_PTR_M                : 8;
        FIELD rsv_24                    : 4;
        FIELD MAGIC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_2_CTRL1, *PREG_RRO_MSDU_PG_2_CTRL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CPU_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DMA_IDX_MIRO              : 12;
        FIELD rsv_28                    : 3;
        FIELD BURST_MODE_DISABLE        : 1;
    } Bits;
    UINT32 Raw;
} REG_RRO_MSDU_PG_2_CTRL2, *PREG_RRO_MSDU_PG_2_CTRL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX0_BASE, *PREG_RRO_RX_D_RX0_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 12;
        FIELD rsv_12                    : 4;
        FIELD PTR_H                     : 8;
        FIELD rsv_24                    : 4;
        FIELD MAGIC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX0_CNT, *PREG_RRO_RX_D_RX0_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DRV_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX0_CRX_IDX, *PREG_RRO_RX_D_RX0_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX_MIRO              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX0_DRX_IDX, *PREG_RRO_RX_D_RX0_DRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX1_BASE, *PREG_RRO_RX_D_RX1_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX                       : 12;
        FIELD rsv_12                    : 4;
        FIELD PTR_H                     : 8;
        FIELD rsv_24                    : 4;
        FIELD MAGIC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX1_CNT, *PREG_RRO_RX_D_RX1_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRX_IDX                   : 12;
        FIELD rsv_12                    : 4;
        FIELD DRV_IDX                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX1_CRX_IDX, *PREG_RRO_RX_D_RX1_CRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DRX_IDX_MIRO              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RX1_DRX_IDX, *PREG_RRO_RX_D_RX1_DRX_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDR_RRO_R0_L             : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RING_CFG_ADDR_0, *PREG_RRO_RX_D_RING_CFG_ADDR_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDR_RRO_R1_L             : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RING_CFG_ADDR_1, *PREG_RRO_RX_D_RING_CFG_ADDR_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ADDR_RRO_R0_M             : 4;
        FIELD ADDR_RRO_R1_M             : 4;
        FIELD POLL_CYCLE                : 8;
        FIELD R0_INT_FLAG               : 1;
        FIELD R1_INT_FLAG               : 1;
        FIELD BURST_MODE_DISABLE        : 1;
        FIELD FORCE_IDLE_WHEN_DRV_ERROR : 1;
        FIELD INIT_COMPLETE_FLAG        : 1;
        FIELD POLL_FLAG                 : 1;
        FIELD BUSY_FLAG                 : 1;
        FIELD ERR_LAT_OPT               : 1;
        FIELD DMAD_DBG_EN               : 1;
        FIELD DRV_DBG_CLR               : 1;
        FIELD DRV_CLR                   : 1;
        FIELD INIT_FLAG_CLR             : 1;
        FIELD FORCE_RETURN_IDLE         : 1;
        FIELD FORCE_IDLE_WHEN_DRV_DISABLE : 1;
        FIELD INT_PROC_DISABLE          : 1;
        FIELD DRV_EN                    : 1;
    } Bits;
    UINT32 Raw;
} REG_RRO_RX_D_RING_CFG_ADDR_2, *PREG_RRO_RX_D_RING_CFG_ADDR_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SDL0                      : 14;
        FIELD rsv_14                    : 2;
        FIELD BASE_PTR_H                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_RX_DMAD, *PREG_RRO_PG_BM_RX_DMAD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_BASE, *PREG_RRO_PG_BM_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_TAIL_IDX               : 16;
        FIELD INIT_SW_TAIL_IDX          : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_INIT_PTR, *PREG_RRO_PG_BM_INIT_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TAIL_IDX                  : 16;
        FIELD HEAD_IDX                  : 16;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_PTR, *PREG_RRO_PG_BM_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_ADD_BASE_H, *PREG_RRO_PG_BM_ADD_BASE_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD DMAD_NUM                  : 5;
        FIELD rsv_13                    : 3;
        FIELD DMAD0_NUM                 : 2;
        FIELD rsv_18                    : 2;
        FIELD DMAD0_VLD                 : 1;
        FIELD DMAD0_RD                  : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_STS, *PREG_RRO_PG_BM_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD_FREE                 : 16;
        FIELD rsv_16                    : 12;
        FIELD DMAD_FREE_VLD             : 1;
        FIELD DMAD_FREE_RD              : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_INTF, *PREG_RRO_PG_BM_INTF;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMAD_AXI_RD_LESS          : 1;
        FIELD DMAD_AXI_RD_MORE          : 1;
        FIELD DMAD_FULL_AXI_RD          : 1;
        FIELD DMAD_AXI_RD_OVER_TAIL     : 1;
        FIELD rsv_4                     : 3;
        FIELD FREE_AT_EMPTY             : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_ERR_STS, *PREG_RRO_PG_BM_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREE_AXI_RESP_SEL         : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_OPT_CTRL, *PREG_RRO_PG_BM_OPT_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD L_BUF_TH                  : 16;
        FIELD H_BUF_TH                  : 16;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_DYN_ALLOC_TH, *PREG_RRO_PG_BM_DYN_ALLOC_TH;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_ADDSUB_IDX_CNT         : 16;
        FIELD SW_SUB_BUF_REQ            : 1;
        FIELD SW_ADD_BUF_REQ            : 1;
        FIELD SW_SUB_RDY                : 1;
        FIELD rsv_19                    : 9;
        FIELD SW_SUB_UNDERFLOW_ERR      : 1;
        FIELD SW_ADD_OVERFLOW_ERR       : 1;
        FIELD HW_ADDSUB_ACK             : 1;
        FIELD HW_SUB_pause              : 1;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_DYN_ALLOC_CFG, *PREG_RRO_PG_BM_DYN_ALLOC_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PTR                       : 32;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_ADD_BASE, *PREG_RRO_PG_BM_ADD_BASE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_ADD_IDX_REMAIN         : 16;
        FIELD HEAD2_IDX                 : 16;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_ADD_PTR, *PREG_RRO_PG_BM_ADD_PTR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SW_TOTAL_RX_BM_CNT        : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_RRO_PG_BM_TOTAL_DMAD_IDX, *PREG_RRO_PG_BM_TOTAL_DMAD_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RRO_RX_DONE_EN0           : 1;
        FIELD RRO_RX_DONE_CLR0          : 1;
        FIELD RRO_RX_DONE_TRIG0         : 5;
        FIELD rsv_7                     : 1;
        FIELD RRO_RX_DONE_EN1           : 1;
        FIELD RRO_RX_DONE_CLR1          : 1;
        FIELD RRO_RX_DONE_TRIG1         : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CTRL_RRO_RX, *PREG_WED_WPDMA_INT_CTRL_RRO_RX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RRO_PG_DONE_EN0           : 1;
        FIELD RRO_PG_DONE_CLR0          : 1;
        FIELD RRO_PG_DONE_TRIG0         : 5;
        FIELD rsv_7                     : 1;
        FIELD RRO_PG_DONE_EN1           : 1;
        FIELD RRO_PG_DONE_CLR1          : 1;
        FIELD RRO_PG_DONE_TRIG1         : 5;
        FIELD rsv_15                    : 1;
        FIELD RRO_PG_DONE_EN2           : 1;
        FIELD RRO_PG_DONE_CLR2          : 1;
        FIELD RRO_PG_DONE_TRIG2         : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_WED_WPDMA_INT_CTRL_RRO_MSDU_PG, *PREG_WED_WPDMA_INT_CTRL_RRO_MSDU_PG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_IND_CMD_BUSY           : 1;
        FIELD RX_ADDR_ELEM_BUSY         : 1;
        FIELD RX_MSDU_INFO_PG_BUSY      : 1;
        FIELD RX_PN_CHK_BUSY            : 1;
        FIELD rsv_4                     : 4;
        FIELD FETCH_IND_CMD_CS          : 2;
        FIELD IND2ADDR_ELEM_CS          : 3;
        FIELD CLR_ADDR_ACKSN_CS         : 3;
        FIELD MSDU_INFO_PG_CS           : 2;
        FIELD PN_CHK_CS                 : 3;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} REG_WED_RRO_RX_HW_STS, *PREG_WED_RRO_RX_HW_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD IND_CMD_WA_B64            : 4;
        FIELD IND_CMD_RA                : 4;
        FIELD ADDR_ELEM_WA_B64          : 4;
        FIELD ADDR_ELEM_RA              : 4;
        FIELD RX_PG_MEM_WA              : 4;
        FIELD RX_PG_MEM_RA              : 4;
        FIELD rx_ind_cmd                : 6;
        FIELD rx_ind_cmd_rd             : 1;
        FIELD rx_ind_cmd_vld            : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RRO_RX_DBG0, *PREG_WED_RRO_RX_DBG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD parti_addr_elem           : 14;
        FIELD parti_addr_elem_rd        : 1;
        FIELD parti_addr_elem_vld       : 1;
        FIELD acksn_last_cnt            : 12;
        FIELD addr_elem_wr_align_err    : 1;
        FIELD addr_elem_aw_err          : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_WED_RRO_RX_DBG1, *PREG_WED_RRO_RX_DBG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD hif_rxd_ext               : 14;
        FIELD hif_rxd_ext_rd            : 1;
        FIELD hif_rxd_ext_vld           : 1;
        FIELD rx_pg_dmad_free           : 14;
        FIELD rx_pg_dmad_free_rd        : 1;
        FIELD rx_pg_dmad_free_vld       : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RRO_RX_DBG2, *PREG_WED_RRO_RX_DBG2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD rx_msdu2rtqm_wdata        : 14;
        FIELD rx_msdu2rtqm_rdy          : 1;
        FIELD rx_msdu2rtqm_wr           : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RRO_RX_DBG3, *PREG_WED_RRO_RX_DBG3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD clr_addr_acksn_wdata      : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RRO_RX_DBG4, *PREG_WED_RRO_RX_DBG4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_ind_cmd_cnt         : 28;
        FIELD rsv_28                    : 1;
        FIELD dbg_cnt_rdclr_en          : 1;
        FIELD all_dbg_cnt_rst           : 1;
        FIELD dbg_cnt_en                : 1;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT0, *PREG_WED_RX_IND_CMD_CNT0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch1_cnt        : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT1, *PREG_WED_RX_IND_CMD_CNT1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch2_cnt        : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT2, *PREG_WED_RX_IND_CMD_CNT2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch3_cnt        : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT3, *PREG_WED_RX_IND_CMD_CNT3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch4_cnt        : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT4, *PREG_WED_RX_IND_CMD_CNT4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch5_cnt        : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT5, *PREG_WED_RX_IND_CMD_CNT5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch6_cnt        : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT6, *PREG_WED_RX_IND_CMD_CNT6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch7_cnt        : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT7, *PREG_WED_RX_IND_CMD_CNT7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ind_cmd_fetch8_cnt        : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT8, *PREG_WED_RX_IND_CMD_CNT8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD magic_cnt_fail_cnt        : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_CNT9, *PREG_WED_RX_IND_CMD_CNT9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_addr_elem_cnt       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_CNT0, *PREG_WED_RX_ADDR_ELEM_CNT0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_sig_fail_cnt        : 16;
        FIELD total_1st_sig_fail_cnt    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_CNT1, *PREG_WED_RX_ADDR_ELEM_CNT1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD clr_addr_cnt              : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_CNT2, *PREG_WED_RX_ADDR_ELEM_CNT2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD acksn_cnt                 : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_CNT3, *PREG_WED_RX_ADDR_ELEM_CNT3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD acksn_last_cnt            : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_CNT4, *PREG_WED_RX_ADDR_ELEM_CNT4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pg_cnt1_cnt               : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MSDU_PG_CNT1, *PREG_WED_RX_MSDU_PG_CNT1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pg_cnt2_cnt               : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MSDU_PG_CNT2, *PREG_WED_RX_MSDU_PG_CNT2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pg_cnt3_cnt               : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MSDU_PG_CNT3, *PREG_WED_RX_MSDU_PG_CNT3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pg_cnt4_cnt               : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MSDU_PG_CNT4, *PREG_WED_RX_MSDU_PG_CNT4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pg_cnt5_cnt               : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MSDU_PG_CNT5, *PREG_WED_RX_MSDU_PG_CNT5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_chk_fail_cnt           : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_PN_CHK_CNT0, *PREG_WED_RX_PN_CHK_CNT0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD araddr_reg                : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_IND_CMD_AXI0, *PREG_WED_RX_IND_CMD_AXI0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD araddr_reg                : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_AXI0, *PREG_WED_RX_ADDR_ELEM_AXI0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD awaddr_reg                : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_AXI1, *PREG_WED_RX_ADDR_ELEM_AXI1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD awaddr_clr_addr_lat       : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_AXI2, *PREG_WED_RX_ADDR_ELEM_AXI2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD wdata0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_AXI3, *PREG_WED_RX_ADDR_ELEM_AXI3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD wdata1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_ADDR_ELEM_AXI4, *PREG_WED_RX_ADDR_ELEM_AXI4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD araddr_reg                : 32;
    } Bits;
    UINT32 Raw;
} REG_WED_RX_MSDU_PG_AXI0, *PREG_WED_RX_MSDU_PG_AXI0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD value                     : 32;
    } Bits;
    UINT32 Raw;
} REG_DONT_USE_BEHIND_THIS_ADDR, *PREG_DONT_USE_BEHIND_THIS_ADDR;

// ----------------- wed0  Grouping Definitions -------------------
// ----------------- wed0 Register Definition -------------------
typedef volatile PACKING struct
{
    REG_WED_ID                      WED_ID;           // 0000
    REG_WED_REV_ID                  WED_REV_ID;       // 0004
    REG_WED_MOD_RST                 WED_MOD_RST;      // 0008
    REG_WED_CTRL                    WED_CTRL;         // 000C
    REG_WED_AXI_CTRL                WED_AXI_CTRL;     // 0010
    UINT32                          rsv_0014[2];      // 0014..0018
    REG_WED_CTRL2                   WED_CTRL2;        // 001C
    REG_WED_EX_INT_STA              WED_EX_INT_STA;   // 0020
    UINT32                          rsv_0024;         // 0024
    REG_WED_EX_INT_MSK              WED_EX_INT_MSK;   // 0028
    REG_WED_EX_INT_MSK1             WED_EX_INT_MSK1;  // 002C
    REG_WED_EX_INT_MSK2             WED_EX_INT_MSK2;  // 0030
    REG_WED_EX_INT_MSK3             WED_EX_INT_MSK3;  // 0034
    UINT32                          rsv_0038[2];      // 0038..003C
    REG_WED_EX_INT_STA_POST_MSK0    WED_EX_INT_STA_POST_MSK0; // 0040
    REG_WED_EX_INT_STA_POST_MSK1    WED_EX_INT_STA_POST_MSK1; // 0044
    REG_WED_EX_INT_STA_POST_MSK2    WED_EX_INT_STA_POST_MSK2; // 0048
    REG_WED_EX_INT_STA_POST_MSK3    WED_EX_INT_STA_POST_MSK3; // 004C
    REG_WED_IRQ_MON                 WED_IRQ_MON;      // 0050
    REG_WED_ERR_MON                 WED_ERR_MON;      // 0054
    UINT32                          rsv_0058[2];      // 0058..005C
    REG_WED_ST                      WED_ST;           // 0060
    REG_WED_WPDMA_ST                WED_WPDMA_ST;     // 0064
    REG_WED_WDMA_ST                 WED_WDMA_ST;      // 0068
    REG_WED_BM_ST                   WED_BM_ST;        // 006C
    REG_WED_QM_ST                   WED_QM_ST;        // 0070
    REG_WED_WPDMA_D_ST              WED_WPDMA_D_ST;   // 0074
    UINT32                          rsv_0078[2];      // 0078..007C
    REG_WED_TX_BM_CTRL              WED_TX_BM_CTRL;   // 0080
    REG_WED_TX_BM_BASE              WED_TX_BM_BASE;   // 0084
    REG_WED_TX_BM_INIT_PTR          WED_TX_BM_INIT_PTR; // 0088
    REG_WED_TX_BM_BLEN              WED_TX_BM_BLEN;   // 008C
    REG_WED_TX_BM_STS               WED_TX_BM_STS;    // 0090
    REG_WED_TX_BM_PTR               WED_TX_BM_PTR;    // 0094
    UINT32                          rsv_0098;         // 0098
    REG_WED_TX_BM_INTF              WED_TX_BM_INTF;   // 009C
    REG_WED_TX_BM_DYN_TH            WED_TX_BM_DYN_TH; // 00A0
    REG_WED_TX_BM_ERR_STS           WED_TX_BM_ERR_STS; // 00A4
    REG_WED_TX_BM_RECYC             WED_TX_BM_RECYC;  // 00A8
    REG_WED_TX_BM_MON_CTRL          WED_TX_BM_MON_CTRL; // 00AC
    REG_WED_TX_BM_VB_FREE_0_31      WED_TX_BM_VB_FREE_0_31; // 00B0
    REG_WED_TX_BM_VB_FREE_32_63     WED_TX_BM_VB_FREE_32_63; // 00B4
    REG_WED_TX_BM_VB_USED_0_31      WED_TX_BM_VB_USED_0_31; // 00B8
    REG_WED_TX_BM_VB_USED_32_63     WED_TX_BM_VB_USED_32_63; // 00BC
    REG_WED_TX_TKID_CTRL            WED_TX_TKID_CTRL; // 00C0
    REG_WED_TX_TKID_SPR             WED_TX_TKID_SPR;  // 00C4
    REG_WED_TX_TKID_TKID            WED_TX_TKID_TKID; // 00C8
    UINT32                          rsv_00CC[4];      // 00CC..00D8
    REG_WED_TX_TKID_INTF            WED_TX_TKID_INTF; // 00DC
    REG_WED_TX_TKID_DYN_TH          WED_TX_TKID_DYN_TH; // 00E0
    REG_WED_TX_TKID_STS             WED_TX_TKID_STS;  // 00E4
    REG_WED_TX_TKID_RECYC           WED_TX_TKID_RECYC; // 00E8
    REG_WED_TX_TKID_ALI             WED_TX_TKID_ALI;  // 00EC
    REG_WED_TX_TKID_VB_FREE_0_31    WED_TX_TKID_VB_FREE_0_31; // 00F0
    REG_WED_TX_TKID_VB_FREE_32_63   WED_TX_TKID_VB_FREE_32_63; // 00F4
    REG_WED_TX_TKID_VB_USED_0_31    WED_TX_TKID_VB_USED_0_31; // 00F8
    REG_WED_TX_TKID_VB_USED_32_63   WED_TX_TKID_VB_USED_32_63; // 00FC
    REG_WED_TXD_DW0                 WED_TXD_DW0;      // 0100
    REG_WED_TXD_DW1                 WED_TXD_DW1;      // 0104
    REG_WED_TXD_DW2                 WED_TXD_DW2;      // 0108
    REG_WED_TXD_DW3                 WED_TXD_DW3;      // 010C
    REG_WED_TXD_DW4                 WED_TXD_DW4;      // 0110
    REG_WED_TXD_DW5                 WED_TXD_DW5;      // 0114
    REG_WED_TXD_DW6                 WED_TXD_DW6;      // 0118
    REG_WED_TXD_DW7                 WED_TXD_DW7;      // 011C
    REG_WED_TXP_DW0                 WED_TXP_DW0;      // 0120
    REG_WED_TXP_DW1                 WED_TXP_DW1;      // 0124
    UINT32                          rsv_0128[2];      // 0128..012C
    REG_WED_TXDP_CTRL               WED_TXDP_CTRL;    // 0130
    UINT32                          rsv_0134[19];     // 0134..017C
    REG_WED_DBG_CTRL                WED_DBG_CTRL;     // 0180
    REG_WED_DBG_BUS_ADDR_L          WED_DBG_BUS_ADDR_L; // 0184
    REG_WED_DBG_BUS_ADDR_H          WED_DBG_BUS_ADDR_H; // 0188
    REG_WED_DBG_BUS_MON             WED_DBG_BUS_MON;  // 018C
    REG_WED_DBG_PRB0                WED_DBG_PRB0;     // 0190
    REG_WED_DBG_PRB1                WED_DBG_PRB1;     // 0194
    REG_WED_DBG_PRB2                WED_DBG_PRB2;     // 0198
    REG_WED_DBG_PRB3                WED_DBG_PRB3;     // 019C
    UINT32                          rsv_01A0[4];      // 01A0..01AC
    REG_WED_TX_TKID_FREE_TKID_IN    WED_TX_TKID_FREE_TKID_IN; // 01B0
    REG_WED_TX_TKID_FREE_TKID_LDDRAM WED_TX_TKID_FREE_TKID_LDDRAM; // 01B4
    REG_WED_TX_TKID_FREE_TKID_LDSRAM WED_TX_TKID_FREE_TKID_LDSRAM; // 01B8
    REG_WED_TX_TKID_FREE_TKID_RECYCLE WED_TX_TKID_FREE_TKID_RECYCLE; // 01BC
    REG_WED_TX_FREE_TO_TX_TKID_TKID_MIB WED_TX_FREE_TO_TX_TKID_TKID_MIB; // 01C0
    REG_WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB; // 01C4
    REG_WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB; // 01C8
    REG_WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB; // 01CC
    REG_WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB; // 01D0
    REG_WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB; // 01D4
    REG_WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB; // 01D8
    REG_WED_PMTR_CTRL               WED_PMTR_CTRL;    // 01DC
    REG_WED_PMTR_TGT                WED_PMTR_TGT;     // 01E0
    REG_WED_PMTR_TGT_ST             WED_PMTR_TGT_ST;  // 01E4
    REG_WED_PMTR_LTCY_MAX0_1        WED_PMTR_LTCY_MAX0_1; // 01E8
    REG_WED_PMTR_LTCY_MAX2_3        WED_PMTR_LTCY_MAX2_3; // 01EC
    REG_WED_PMTR_LTCY_ACC0          WED_PMTR_LTCY_ACC0; // 01F0
    REG_WED_PMTR_LTCY_ACC1          WED_PMTR_LTCY_ACC1; // 01F4
    REG_WED_PMTR_LTCY_ACC2          WED_PMTR_LTCY_ACC2; // 01F8
    REG_WED_PMTR_LTCY_ACC3          WED_PMTR_LTCY_ACC3; // 01FC
    REG_WED_INT_STA                 WED_INT_STA;      // 0200
    REG_WED_INT_MSK                 WED_INT_MSK;      // 0204
    REG_WED_GLO_CFG                 WED_GLO_CFG;      // 0208
    REG_WED_RST_IDX                 WED_RST_IDX;      // 020C
    REG_WED_DLY_INT_CFG             WED_DLY_INT_CFG;  // 0210
    REG_WED_DLY_INT_CFG1            WED_DLY_INT_CFG1; // 0214
    UINT32                          rsv_0218;         // 0218
    REG_WED_SPR                     WED_SPR;          // 021C
    UINT32                          rsv_0220;         // 0220
    REG_WED_INT_MSK1                WED_INT_MSK1;     // 0224
    REG_WED_INT_MSK2                WED_INT_MSK2;     // 0228
    REG_WED_INT_MSK3                WED_INT_MSK3;     // 022C
    REG_WED_INT_STA_POST_MSK0       WED_INT_STA_POST_MSK0; // 0230
    REG_WED_INT_STA_POST_MSK1       WED_INT_STA_POST_MSK1; // 0234
    REG_WED_INT_STA_POST_MSK2       WED_INT_STA_POST_MSK2; // 0238
    REG_WED_INT_STA_POST_MSK3       WED_INT_STA_POST_MSK3; // 023C
    UINT32                          rsv_0240[3];      // 0240..0248
    REG_WED_SPR_0X24C               WED_SPR_0X24C;    // 024C
    UINT32                          rsv_0250[3];      // 0250..0258
    REG_WED_SPR_0X25C               WED_SPR_0X25C;    // 025C
    UINT32                          rsv_0260[16];     // 0260..029C
    REG_WED_TX0_MIB                 WED_TX0_MIB;      // 02A0
    REG_WED_TX1_MIB                 WED_TX1_MIB;      // 02A4
    UINT32                          rsv_02A8[10];     // 02A8..02CC
    REG_WED_TX_COHERENT_MIB         WED_TX_COHERENT_MIB; // 02D0
    UINT32                          rsv_02D4[3];      // 02D4..02DC
    REG_WED_RX0_MIB                 WED_RX0_MIB;      // 02E0
    REG_WED_RX1_MIB                 WED_RX1_MIB;      // 02E4
    UINT32                          rsv_02E8[2];      // 02E8..02EC
    REG_WED_RX_DMA_MIB              WED_RX_DMA_MIB;   // 02F0
    UINT32                          rsv_02F4[3];      // 02F4..02FC
    REG_WED_TX0_CTRL0               WED_TX0_CTRL0;    // 0300
    REG_WED_TX0_CTRL1               WED_TX0_CTRL1;    // 0304
    REG_WED_TX0_CTRL2               WED_TX0_CTRL2;    // 0308
    REG_WED_TX0_CTRL3               WED_TX0_CTRL3;    // 030C
    REG_WED_TX1_CTRL0               WED_TX1_CTRL0;    // 0310
    REG_WED_TX1_CTRL1               WED_TX1_CTRL1;    // 0314
    REG_WED_TX1_CTRL2               WED_TX1_CTRL2;    // 0318
    REG_WED_TX1_CTRL3               WED_TX1_CTRL3;    // 031C
    UINT32                          rsv_0320[40];     // 0320..03BC
    REG_WED_SCR0                    WED_SCR0;         // 03C0
    REG_WED_SCR1                    WED_SCR1;         // 03C4
    REG_WED_SCR2                    WED_SCR2;         // 03C8
    REG_WED_SCR3                    WED_SCR3;         // 03CC
    REG_WED_SCR4                    WED_SCR4;         // 03D0
    REG_WED_SCR5                    WED_SCR5;         // 03D4
    REG_WED_SCR6                    WED_SCR6;         // 03D8
    REG_WED_SCR7                    WED_SCR7;         // 03DC
    UINT32                          rsv_03E0[8];      // 03E0..03FC
    REG_WED_RX0_CTRL0               WED_RX0_CTRL0;    // 0400
    REG_WED_RX0_CTRL1               WED_RX0_CTRL1;    // 0404
    REG_WED_RX0_CTRL2               WED_RX0_CTRL2;    // 0408
    REG_WED_RX0_CTRL3               WED_RX0_CTRL3;    // 040C
    REG_WED_RX1_CTRL0               WED_RX1_CTRL0;    // 0410
    REG_WED_RX1_CTRL1               WED_RX1_CTRL1;    // 0414
    REG_WED_RX1_CTRL2               WED_RX1_CTRL2;    // 0418
    REG_WED_RX1_CTRL3               WED_RX1_CTRL3;    // 041C
    REG_WED_RX_BASE_PTR_0           WED_RX_BASE_PTR_0; // 0420
    REG_WED_RX_MAX_CNT_0            WED_RX_MAX_CNT_0; // 0424
    REG_WED_RX_CRX_IDX_0            WED_RX_CRX_IDX_0; // 0428
    REG_WED_RX_DRX_IDX_0            WED_RX_DRX_IDX_0; // 042C
    REG_WED_RX_BASE_PTR_1           WED_RX_BASE_PTR_1; // 0430
    REG_WED_RX_MAX_CNT_1            WED_RX_MAX_CNT_1; // 0434
    REG_WED_RX_CRX_IDX_1            WED_RX_CRX_IDX_1; // 0438
    REG_WED_RX_DRX_IDX_1            WED_RX_DRX_IDX_1; // 043C
    UINT32                          rsv_0440[48];     // 0440..04FC
    REG_WED_WPDMA_INT_STA_REC       WED_WPDMA_INT_STA_REC; // 0500
    REG_WED_WPDMA_INT_TRIG          WED_WPDMA_INT_TRIG; // 0504
    REG_WED_WPDMA_GLO_CFG           WED_WPDMA_GLO_CFG; // 0508
    REG_WED_WPDMA_RST_IDX           WED_WPDMA_RST_IDX; // 050C
    REG_WED_WPDMA_BUS_CFG           WED_WPDMA_BUS_CFG; // 0510
    REG_WED_WPDMA_TX_DLY_CIDX       WED_WPDMA_TX_DLY_CIDX; // 0514
    REG_WED_WPDMA_CTRL              WED_WPDMA_CTRL;   // 0518
    REG_WED_WPDMA_FORCE_PROC        WED_WPDMA_FORCE_PROC; // 051C
    REG_WED_WPDMA_INT_CTRL          WED_WPDMA_INT_CTRL; // 0520
    REG_WED_WPDMA_INT_MSK           WED_WPDMA_INT_MSK; // 0524
    REG_WED_WPDMA_INT_CLR           WED_WPDMA_INT_CLR; // 0528
    REG_WED_WPDMA_INT_MON           WED_WPDMA_INT_MON; // 052C
    REG_WED_WPDMA_INT_CTRL_TX       WED_WPDMA_INT_CTRL_TX; // 0530
    REG_WED_WPDMA_INT_CTRL_RX       WED_WPDMA_INT_CTRL_RX; // 0534
    REG_WED_WPDMA_INT_CTRL_TX_FREE  WED_WPDMA_INT_CTRL_TX_FREE; // 0538
    REG_WED_WPDMA_SPR               WED_WPDMA_SPR;    // 053C
    REG_WED_PCIE_MSIS_TRIG          WED_PCIE_MSIS_TRIG; // 0540
    REG_WED_PCIE_MSIS_REC           WED_PCIE_MSIS_REC; // 0544
    REG_WED_PCIE_MSIS_CLR           WED_PCIE_MSIS_CLR; // 0548
    UINT32                          rsv_054C;         // 054C
    REG_WED_PCIE_INTS_CLR           WED_PCIE_INTS_CLR; // 0550
    REG_WED_PCIE_EP_INTS_CLR        WED_PCIE_EP_INTS_CLR; // 0554
    UINT32                          rsv_0558;         // 0558
    REG_WED_PCIE_CFG_ADDR_H         WED_PCIE_CFG_ADDR_H; // 055C
    REG_WED_PCIE_CFG_ADDR_INTS      WED_PCIE_CFG_ADDR_INTS; // 0560
    REG_WED_PCIE_CFG_ADDR_INTM      WED_PCIE_CFG_ADDR_INTM; // 0564
    REG_WED_PCIE_CFG_ADDR_MSIS      WED_PCIE_CFG_ADDR_MSIS; // 0568
    REG_WED_PCIE_CFG_ADDR_INTS_EP   WED_PCIE_CFG_ADDR_INTS_EP; // 056C
    REG_WED_PCIE_INTS_TRIG          WED_PCIE_INTS_TRIG; // 0570
    REG_WED_PCIE_INTS_REC           WED_PCIE_INTS_REC; // 0574
    REG_WED_PCIE_INTM_REC           WED_PCIE_INTM_REC; // 0578
    REG_WED_PCIE_INT_CTRL           WED_PCIE_INT_CTRL; // 057C
    REG_WED_WPDMA_CFG_ADDR_INTS     WED_WPDMA_CFG_ADDR_INTS; // 0580
    REG_WED_WPDMA_CFG_ADDR_INTM     WED_WPDMA_CFG_ADDR_INTM; // 0584
    REG_WED_WPDMA_CFG_ADDR_TX       WED_WPDMA_CFG_ADDR_TX; // 0588
    REG_WED_WPDMA_CFG_ADDR_TX_FREE  WED_WPDMA_CFG_ADDR_TX_FREE; // 058C
    REG_WED_WPDMA_CFG_ADDR_INT_H    WED_WPDMA_CFG_ADDR_INT_H; // 0590
    UINT32                          rsv_0594;         // 0594
    REG_WED_WPDMA_CFG_ADDR_TX_H     WED_WPDMA_CFG_ADDR_TX_H; // 0598
    REG_WED_WPDMA_CFG_ADDR_TX_FREE_H WED_WPDMA_CFG_ADDR_TX_FREE_H; // 059C
    REG_WED_WPDMA_TX0_MIB           WED_WPDMA_TX0_MIB; // 05A0
    REG_WED_WPDMA_TX1_MIB           WED_WPDMA_TX1_MIB; // 05A4
    UINT32                          rsv_05A8[10];     // 05A8..05CC
    REG_WED_WPDMA_TX_COHERENT_MIB   WED_WPDMA_TX_COHERENT_MIB; // 05D0
    UINT32                          rsv_05D4[3];      // 05D4..05DC
    REG_WED_WPDMA_RX0_MIB           WED_WPDMA_RX0_MIB; // 05E0
    REG_WED_WPDMA_RX1_MIB           WED_WPDMA_RX1_MIB; // 05E4
    UINT32                          rsv_05E8[2];      // 05E8..05EC
    REG_WED_WPDMA_RX_COHERENT_MIB   WED_WPDMA_RX_COHERENT_MIB; // 05F0
    UINT32                          rsv_05F4;         // 05F4
    REG_WED_WPDMA_RX_EXTC_FREE_TKID_MIB WED_WPDMA_RX_EXTC_FREE_TKID_MIB; // 05F8
    UINT32                          rsv_05FC;         // 05FC
    REG_WED_WPDMA_TX0_CTRL0         WED_WPDMA_TX0_CTRL0; // 0600
    REG_WED_WPDMA_TX0_CTRL1         WED_WPDMA_TX0_CTRL1; // 0604
    REG_WED_WPDMA_TX0_CTRL2         WED_WPDMA_TX0_CTRL2; // 0608
    REG_WED_WPDMA_TX0_CTRL3         WED_WPDMA_TX0_CTRL3; // 060C
    REG_WED_WPDMA_TX1_CTRL0         WED_WPDMA_TX1_CTRL0; // 0610
    REG_WED_WPDMA_TX1_CTRL1         WED_WPDMA_TX1_CTRL1; // 0614
    REG_WED_WPDMA_TX1_CTRL2         WED_WPDMA_TX1_CTRL2; // 0618
    REG_WED_WPDMA_TX1_CTRL3         WED_WPDMA_TX1_CTRL3; // 061C
    UINT32                          rsv_0620[8];      // 0620..063C
    REG_RRO_RX_D_DRV_MON_R0_WRAP    RRO_RX_D_DRV_MON_R0_WRAP; // 0640
    REG_RRO_RX_D_DRV_MON_R1_WRAP    RRO_RX_D_DRV_MON_R1_WRAP; // 0644
    REG_RRO_RX_D_DRV_MON_R2_WRAP    RRO_RX_D_DRV_MON_R2_WRAP; // 0648
    REG_RRO_RX_D_DRV_MON0           RRO_RX_D_DRV_MON0; // 064C
    REG_RRO_RX_D_DRV_MON1           RRO_RX_D_DRV_MON1; // 0650
    REG_RRO_RX_D_DRV_MON2           RRO_RX_D_DRV_MON2; // 0654
    REG_RRO_RX_D_DRV_ERR_COMP0      RRO_RX_D_DRV_ERR_COMP0; // 0658
    REG_RRO_RX_D_DRV_ERR_COMP1      RRO_RX_D_DRV_ERR_COMP1; // 065C
    REG_RRO_RX_D_DRV_ERR_COMP2      RRO_RX_D_DRV_ERR_COMP2; // 0660
    REG_RRO_RX_D_DRV_ERR_COMP3      RRO_RX_D_DRV_ERR_COMP3; // 0664
    REG_RRO_RX_D_DRV_LEAGLE_START   RRO_RX_D_DRV_LEAGLE_START; // 0668
    REG_RRO_RX_D_DRV_LEAGLE_END     RRO_RX_D_DRV_LEAGLE_END; // 066C
    REG_RRO_RX_PG_DRV_MON_R0_WRAP   RRO_RX_PG_DRV_MON_R0_WRAP; // 0670
    REG_RRO_RX_PG_DRV_MON_R1_WRAP   RRO_RX_PG_DRV_MON_R1_WRAP; // 0674
    REG_RRO_RX_PG_DRV_MON_R2_WRAP   RRO_RX_PG_DRV_MON_R2_WRAP; // 0678
    REG_RRO_RX_PG_DRV_MON0          RRO_RX_PG_DRV_MON0; // 067C
    REG_RRO_RX_PG_DRV_MON1          RRO_RX_PG_DRV_MON1; // 0680
    REG_RRO_RX_PG_DRV_MON2          RRO_RX_PG_DRV_MON2; // 0684
    REG_RRO_RX_PG_DRV_ERR_COMP0     RRO_RX_PG_DRV_ERR_COMP0; // 0688
    REG_RRO_RX_PG_DRV_ERR_COMP1     RRO_RX_PG_DRV_ERR_COMP1; // 068C
    REG_RRO_RX_PG_DRV_ERR_COMP2     RRO_RX_PG_DRV_ERR_COMP2; // 0690
    REG_RRO_RX_PG_DRV_ERR_COMP3     RRO_RX_PG_DRV_ERR_COMP3; // 0694
    REG_RRO_RX_PG_DRV_LEAGLE_START  RRO_RX_PG_DRV_LEAGLE_START; // 0698
    REG_RRO_RX_PG_DRV_LEAGLE_END    RRO_RX_PG_DRV_LEAGLE_END; // 069C
    REG_RRO_RX_D_DRV_ERR_SWITCH_ADDR RRO_RX_D_DRV_ERR_SWITCH_ADDR; // 06A0
    REG_RRO_RX_PG_DRV_ERR_SWITCH_ADDR RRO_RX_PG_DRV_ERR_SWITCH_ADDR; // 06A4
    REG_RRO_RX_D_DRV_IN_MIB         RRO_RX_D_DRV_IN_MIB; // 06A8
    REG_RRO_RX_D_DRV_OU_MIB         RRO_RX_D_DRV_OU_MIB; // 06AC
    REG_RRO_RX_D_DRV_DESC_MIB       RRO_RX_D_DRV_DESC_MIB; // 06B0
    REG_RRO_RX_PG_DRV_IN_MIB        RRO_RX_PG_DRV_IN_MIB; // 06B4
    REG_RRO_RX_PG_DRV_OU_MIB        RRO_RX_PG_DRV_OU_MIB; // 06B8
    REG_RRO_RX_PG_DRV_DESC_MIB      RRO_RX_PG_DRV_DESC_MIB; // 06BC
    UINT32                          rsv_06C0[16];     // 06C0..06FC
    REG_WED_WPDMA_RX0_CTRL0         WED_WPDMA_RX0_CTRL0; // 0700
    REG_WED_WPDMA_RX0_CTRL1         WED_WPDMA_RX0_CTRL1; // 0704
    REG_WED_WPDMA_RX0_CTRL2         WED_WPDMA_RX0_CTRL2; // 0708
    REG_WED_WPDMA_RX0_CTRL3         WED_WPDMA_RX0_CTRL3; // 070C
    REG_WED_WPDMA_RX1_CTRL0         WED_WPDMA_RX1_CTRL0; // 0710
    REG_WED_WPDMA_RX1_CTRL1         WED_WPDMA_RX1_CTRL1; // 0714
    REG_WED_WPDMA_RX1_CTRL2         WED_WPDMA_RX1_CTRL2; // 0718
    REG_WED_WPDMA_RX1_CTRL3         WED_WPDMA_RX1_CTRL3; // 071C
    UINT32                          rsv_0720[4];      // 0720..072C
    REG_WED_WPDMA_RX_D_RX0_BASE     WED_WPDMA_RX_D_RX0_BASE; // 0730
    REG_WED_WPDMA_RX_D_RX0_CNT      WED_WPDMA_RX_D_RX0_CNT; // 0734
    REG_WED_WPDMA_RX_D_RX0_CRX_IDX  WED_WPDMA_RX_D_RX0_CRX_IDX; // 0738
    REG_WED_WPDMA_RX_D_RX0_DRX_IDX  WED_WPDMA_RX_D_RX0_DRX_IDX; // 073C
    REG_WED_WPDMA_RX_D_RX1_BASE     WED_WPDMA_RX_D_RX1_BASE; // 0740
    REG_WED_WPDMA_RX_D_RX1_CNT      WED_WPDMA_RX_D_RX1_CNT; // 0744
    REG_WED_WPDMA_RX_D_RX1_CRX_IDX  WED_WPDMA_RX_D_RX1_CRX_IDX; // 0748
    REG_WED_WPDMA_RX_D_RX1_DRX_IDX  WED_WPDMA_RX_D_RX1_DRX_IDX; // 074C
    REG_WED_WPDMA_RX_D_RX0_THRES_CFG WED_WPDMA_RX_D_RX0_THRES_CFG; // 0750
    REG_WED_WPDMA_RX_D_RX1_THRES_CFG WED_WPDMA_RX_D_RX1_THRES_CFG; // 0754
    REG_WED_WPDMA_RX_D_INFO         WED_WPDMA_RX_D_INFO; // 0758
    REG_WED_WPDMA_RX_D_GLO_CFG      WED_WPDMA_RX_D_GLO_CFG; // 075C
    REG_WED_WPDMA_RX_D_RST_IDX      WED_WPDMA_RX_D_RST_IDX; // 0760
    REG_WED_WPDMA_RX_D_LOAD_DRV_IDX WED_WPDMA_RX_D_LOAD_DRV_IDX; // 0764
    REG_WED_WPDMA_RX_D_LOAD_CRX_IDX WED_WPDMA_RX_D_LOAD_CRX_IDX; // 0768
    REG_WED_WPDMA_RX_D_GLO_CFG_ADDR WED_WPDMA_RX_D_GLO_CFG_ADDR; // 076C
    REG_WED_WPDMA_RX_D_GLO_CFG_ADDR_H WED_WPDMA_RX_D_GLO_CFG_ADDR_H; // 0770
    REG_WED_WPDMA_RX_D_RX0_MIB      WED_WPDMA_RX_D_RX0_MIB; // 0774
    REG_WED_WPDMA_RX_D_RX1_MIB      WED_WPDMA_RX_D_RX1_MIB; // 0778
    REG_WED_WPDMA_RX_D_RX0_RECYCLE_MIB WED_WPDMA_RX_D_RX0_RECYCLE_MIB; // 077C
    REG_WED_WPDMA_RX_D_RX1_RECYCLE_MIB WED_WPDMA_RX_D_RX1_RECYCLE_MIB; // 0780
    REG_WED_WPDMA_RX_D_RX0_PROCESSED_MIB WED_WPDMA_RX_D_RX0_PROCESSED_MIB; // 0784
    REG_WED_WPDMA_RX_D_RX1_PROCESSED_MIB WED_WPDMA_RX_D_RX1_PROCESSED_MIB; // 0788
    REG_WED_WPDMA_RX_D_RX_COHERENT_MIB WED_WPDMA_RX_D_RX_COHERENT_MIB; // 078C
    REG_WED_WPDMA_RX_D_ERR_STS      WED_WPDMA_RX_D_ERR_STS; // 0790
    UINT32                          rsv_0794[3];      // 0794..079C
    REG_WED_WPDMA_RX_D_TKID_CFG     WED_WPDMA_RX_D_TKID_CFG; // 07A0
    REG_WED_WPDMA_RX_D_TKID_RX0_BASE WED_WPDMA_RX_D_TKID_RX0_BASE; // 07A4
    REG_WED_WPDMA_RX_D_TKID_RX1_BASE WED_WPDMA_RX_D_TKID_RX1_BASE; // 07A8
    REG_WED_WPDMA_RX_D_TKID_RX_BASE_H WED_WPDMA_RX_D_TKID_RX_BASE_H; // 07AC
    REG_WED_WPDMA_RX_D_BUS_CFG      WED_WPDMA_RX_D_BUS_CFG; // 07B0
    REG_WED_WPDMA_RX_D_PREF_CFG     WED_WPDMA_RX_D_PREF_CFG; // 07B4
    REG_WED_WPDMA_RX_D_PREF_RX0_SIDX WED_WPDMA_RX_D_PREF_RX0_SIDX; // 07B8
    REG_WED_WPDMA_RX_D_PREF_RX1_SIDX WED_WPDMA_RX_D_PREF_RX1_SIDX; // 07BC
    REG_WED_WPDMA_RX_D_PREF_FIFO_CFG WED_WPDMA_RX_D_PREF_FIFO_CFG; // 07C0
    UINT32                          rsv_07C4[2];      // 07C4..07C8
    REG_WED_WPDMA_RX_D_CIDX_WR_CFG  WED_WPDMA_RX_D_CIDX_WR_CFG; // 07CC
    REG_WED_WPDMA_RX_D_RING0_CFG_ADDR WED_WPDMA_RX_D_RING0_CFG_ADDR; // 07D0
    REG_WED_WPDMA_RX_D_RING0_CFG_ADDR_H WED_WPDMA_RX_D_RING0_CFG_ADDR_H; // 07D4
    REG_WED_WPDMA_RX_D_RING1_CFG_ADDR WED_WPDMA_RX_D_RING1_CFG_ADDR; // 07D8
    REG_WED_WPDMA_RX_D_RING1_CFG_ADDR_H WED_WPDMA_RX_D_RING1_CFG_ADDR_H; // 07DC
    REG_WED_WPDMA_RX_D_AW_RAN_DBG0  WED_WPDMA_RX_D_AW_RAN_DBG0; // 07E0
    REG_WED_WPDMA_RX_D_AW_RAN_DBG1  WED_WPDMA_RX_D_AW_RAN_DBG1; // 07E4
    REG_WED_WPDMA_RX_D_AW_RAN_DBG2  WED_WPDMA_RX_D_AW_RAN_DBG2; // 07E8
    REG_WED_WPDMA_RX_D_AW_RAN_DBG3  WED_WPDMA_RX_D_AW_RAN_DBG3; // 07EC
    REG_WED_WPDMA_RX_D_AW_RAN_DBG4  WED_WPDMA_RX_D_AW_RAN_DBG4; // 07F0
    REG_WED_WPDMA_RX_D_AW_RAN_DBG5  WED_WPDMA_RX_D_AW_RAN_DBG5; // 07F4
    UINT32                          rsv_07F8[2];      // 07F8..07FC
    REG_WED_WDMA_TX0_BASE           WED_WDMA_TX0_BASE; // 0800
    REG_WED_WDMA_TX0_CNT            WED_WDMA_TX0_CNT; // 0804
    REG_WED_WDMA_TX0_CTX_IDX        WED_WDMA_TX0_CTX_IDX; // 0808
    REG_WED_WDMA_TX0_DTX_IDX        WED_WDMA_TX0_DTX_IDX; // 080C
    REG_WED_WDMA_TX0_MIB            WED_WDMA_TX0_MIB; // 0810
    UINT32                          rsv_0814[59];     // 0814..08FC
    REG_WED_WDMA_RX0_BASE           WED_WDMA_RX0_BASE; // 0900
    REG_WED_WDMA_RX0_CNT            WED_WDMA_RX0_CNT; // 0904
    REG_WED_WDMA_RX0_CRX_IDX        WED_WDMA_RX0_CRX_IDX; // 0908
    REG_WED_WDMA_RX0_DRX_IDX        WED_WDMA_RX0_DRX_IDX; // 090C
    REG_WED_WDMA_RX1_BASE           WED_WDMA_RX1_BASE; // 0910
    REG_WED_WDMA_RX1_CNT            WED_WDMA_RX1_CNT; // 0914
    REG_WED_WDMA_RX1_CRX_IDX        WED_WDMA_RX1_CRX_IDX; // 0918
    REG_WED_WDMA_RX1_DRX_IDX        WED_WDMA_RX1_DRX_IDX; // 091C
    UINT32                          rsv_0920[8];      // 0920..093C
    REG_WED_WDMA_RX0_THRES_CFG      WED_WDMA_RX0_THRES_CFG; // 0940
    REG_WED_WDMA_RX1_THRES_CFG      WED_WDMA_RX1_THRES_CFG; // 0944
    UINT32                          rsv_0948[2];      // 0948..094C
    REG_WED_WDMA_RX_PREF_CFG        WED_WDMA_RX_PREF_CFG; // 0950
    REG_WED_WDMA_RX_PREF_RX0_SIDX   WED_WDMA_RX_PREF_RX0_SIDX; // 0954
    REG_WED_WDMA_RX_PREF_RX1_SIDX   WED_WDMA_RX_PREF_RX1_SIDX; // 0958
    REG_WED_WDMA_RX_PREF_FIFO_CFG   WED_WDMA_RX_PREF_FIFO_CFG; // 095C
    REG_WED_WDMA_RX_CIDX_WR_CFG     WED_WDMA_RX_CIDX_WR_CFG; // 0960
    UINT32                          rsv_0964[39];     // 0964..09FC
    REG_WED_WDMA_INFO               WED_WDMA_INFO;    // 0A00
    REG_WED_WDMA_GLO_CFG            WED_WDMA_GLO_CFG; // 0A04
    REG_WED_WDMA_RST_IDX            WED_WDMA_RST_IDX; // 0A08
    REG_WED_WDMA_LOAD_IDX           WED_WDMA_LOAD_IDX; // 0A0C
    REG_WED_WDMA_LOAD_DRV_IDX       WED_WDMA_LOAD_DRV_IDX; // 0A10
    REG_WED_WDMA_LOAD_CRX_IDX       WED_WDMA_LOAD_CRX_IDX; // 0A14
    UINT32                          rsv_0A18;         // 0A18
    REG_WED_WDMA_SPR                WED_WDMA_SPR;     // 0A1C
    REG_WED_WDMA_INT_STA_REC        WED_WDMA_INT_STA_REC; // 0A20
    REG_WED_WDMA_INT_CLR            WED_WDMA_INT_CLR; // 0A24
    REG_WED_WDMA_INT_TRIG           WED_WDMA_INT_TRIG; // 0A28
    REG_WED_WDMA_INT_CTRL           WED_WDMA_INT_CTRL; // 0A2C
    UINT32                          rsv_0A30[3];      // 0A30..0A38
    REG_WED_WDMA_INT_MON            WED_WDMA_INT_MON; // 0A3C
    UINT32                          rsv_0A40[24];     // 0A40..0A9C
    REG_WED_WDMA_CFG_BASE           WED_WDMA_CFG_BASE; // 0AA0
    REG_WED_WDMA_OFST0              WED_WDMA_OFST0;   // 0AA4
    REG_WED_WDMA_OFST1              WED_WDMA_OFST1;   // 0AA8
    REG_WED_WDMA_CFG_BASE_H         WED_WDMA_CFG_BASE_H; // 0AAC
    UINT32                          rsv_0AB0[12];     // 0AB0..0ADC
    REG_WED_WDMA_RX0_MIB            WED_WDMA_RX0_MIB; // 0AE0
    REG_WED_WDMA_RX1_MIB            WED_WDMA_RX1_MIB; // 0AE4
    REG_WED_WDMA_RX0_RECYCLE_MIB    WED_WDMA_RX0_RECYCLE_MIB; // 0AE8
    REG_WED_WDMA_RX1_RECYCLE_MIB    WED_WDMA_RX1_RECYCLE_MIB; // 0AEC
    REG_WED_WDMA_RX0_PROCESSED_MIB  WED_WDMA_RX0_PROCESSED_MIB; // 0AF0
    REG_WED_WDMA_RX1_PROCESSED_MIB  WED_WDMA_RX1_PROCESSED_MIB; // 0AF4
    REG_WED_WDMA_RX_COHERENT_MIB    WED_WDMA_RX_COHERENT_MIB; // 0AF8
    UINT32                          rsv_0AFC;         // 0AFC
    REG_WED_RTQM_GLO_CFG            WED_RTQM_GLO_CFG; // 0B00
    REG_WED_RTQM_RST                WED_RTQM_RST;     // 0B04
    REG_WED_RTQM_IGRS_FIFO_CFG      WED_RTQM_IGRS_FIFO_CFG; // 0B08
    REG_WED_RTQM_PFDBK_FIFO_CFG     WED_RTQM_PFDBK_FIFO_CFG; // 0B0C
    REG_WED_RTQM_PREF_FIFO_CFG      WED_RTQM_PREF_FIFO_CFG; // 0B10
    REG_WED_RTQM_IGRS0_CFG0         WED_RTQM_IGRS0_CFG0; // 0B14
    REG_WED_RTQM_IGRS0_CFG1         WED_RTQM_IGRS0_CFG1; // 0B18
    REG_WED_RTQM_IGRS0_I2HW_DMAD_CNT WED_RTQM_IGRS0_I2HW_DMAD_CNT; // 0B1C
    REG_WED_RTQM_IGRS0_I2H0_DMAD_CNT WED_RTQM_IGRS0_I2H0_DMAD_CNT; // 0B20
    REG_WED_RTQM_IGRS0_I2H1_DMAD_CNT WED_RTQM_IGRS0_I2H1_DMAD_CNT; // 0B24
    REG_WED_RTQM_IGRS0_I2HW_PKT_CNT WED_RTQM_IGRS0_I2HW_PKT_CNT; // 0B28
    REG_WED_RTQM_IGRS0_I2H0_PKT_CNT WED_RTQM_IGRS0_I2H0_PKT_CNT; // 0B2C
    REG_WED_RTQM_IGRS0_I2H1_PKT_CNT WED_RTQM_IGRS0_I2H1_PKT_CNT; // 0B30
    REG_WED_RTQM_IGRS0_FDROP_CNT    WED_RTQM_IGRS0_FDROP_CNT; // 0B34
    REG_WED_RTQM_IGRS0_DBG          WED_RTQM_IGRS0_DBG; // 0B38
    REG_WED_RTQM_IGRS1_CFG0         WED_RTQM_IGRS1_CFG0; // 0B3C
    REG_WED_RTQM_IGRS1_CFG1         WED_RTQM_IGRS1_CFG1; // 0B40
    REG_WED_RTQM_IGRS1_I2HW_DMAD_CNT WED_RTQM_IGRS1_I2HW_DMAD_CNT; // 0B44
    REG_WED_RTQM_IGRS1_I2H0_DMAD_CNT WED_RTQM_IGRS1_I2H0_DMAD_CNT; // 0B48
    REG_WED_RTQM_IGRS1_I2H1_DMAD_CNT WED_RTQM_IGRS1_I2H1_DMAD_CNT; // 0B4C
    REG_WED_RTQM_IGRS1_I2HW_PKT_CNT WED_RTQM_IGRS1_I2HW_PKT_CNT; // 0B50
    REG_WED_RTQM_IGRS1_I2H0_PKT_CNT WED_RTQM_IGRS1_I2H0_PKT_CNT; // 0B54
    REG_WED_RTQM_IGRS1_I2H1_PKT_CNT WED_RTQM_IGRS1_I2H1_PKT_CNT; // 0B58
    REG_WED_RTQM_IGRS1_FDROP_CNT    WED_RTQM_IGRS1_FDROP_CNT; // 0B5C
    REG_WED_RTQM_IGRS1_DBG          WED_RTQM_IGRS1_DBG; // 0B60
    REG_WED_RTQM_IGRS2_CFG0         WED_RTQM_IGRS2_CFG0; // 0B64
    REG_WED_RTQM_IGRS2_CFG1         WED_RTQM_IGRS2_CFG1; // 0B68
    REG_WED_RTQM_IGRS2_I2HW_DMAD_CNT WED_RTQM_IGRS2_I2HW_DMAD_CNT; // 0B6C
    REG_WED_RTQM_IGRS2_I2H0_DMAD_CNT WED_RTQM_IGRS2_I2H0_DMAD_CNT; // 0B70
    REG_WED_RTQM_IGRS2_I2H1_DMAD_CNT WED_RTQM_IGRS2_I2H1_DMAD_CNT; // 0B74
    REG_WED_RTQM_IGRS2_I2HW_PKT_CNT WED_RTQM_IGRS2_I2HW_PKT_CNT; // 0B78
    REG_WED_RTQM_IGRS2_I2H0_PKT_CNT WED_RTQM_IGRS2_I2H0_PKT_CNT; // 0B7C
    REG_WED_RTQM_IGRS2_I2H1_PKT_CNT WED_RTQM_IGRS2_I2H1_PKT_CNT; // 0B80
    REG_WED_RTQM_IGRS2_FDROP_CNT    WED_RTQM_IGRS2_FDROP_CNT; // 0B84
    REG_WED_RTQM_IGRS2_DBG          WED_RTQM_IGRS2_DBG; // 0B88
    REG_WED_RTQM_IGRS3_CFG0         WED_RTQM_IGRS3_CFG0; // 0B8C
    REG_WED_RTQM_IGRS3_CFG1         WED_RTQM_IGRS3_CFG1; // 0B90
    REG_WED_RTQM_IGRS3_I2HW_DMAD_CNT WED_RTQM_IGRS3_I2HW_DMAD_CNT; // 0B94
    REG_WED_RTQM_IGRS3_I2H0_DMAD_CNT WED_RTQM_IGRS3_I2H0_DMAD_CNT; // 0B98
    REG_WED_RTQM_IGRS3_I2H1_DMAD_CNT WED_RTQM_IGRS3_I2H1_DMAD_CNT; // 0B9C
    REG_WED_RTQM_IGRS3_I2HW_PKT_CNT WED_RTQM_IGRS3_I2HW_PKT_CNT; // 0BA0
    REG_WED_RTQM_IGRS3_I2H0_PKT_CNT WED_RTQM_IGRS3_I2H0_PKT_CNT; // 0BA4
    REG_WED_RTQM_IGRS3_I2H1_PKT_CNT WED_RTQM_IGRS3_I2H1_PKT_CNT; // 0BA8
    REG_WED_RTQM_IGRS3_FDROP_CNT    WED_RTQM_IGRS3_FDROP_CNT; // 0BAC
    REG_WED_RTQM_IGRS3_DBG          WED_RTQM_IGRS3_DBG; // 0BB0
    REG_WED_RTQM_EGRS_WEIGHT        WED_RTQM_EGRS_WEIGHT; // 0BB4
    REG_WED_RTQM_ENQ_CFG0           WED_RTQM_ENQ_CFG0; // 0BB8
    REG_WED_RTQM_ENQ_I2Q_DMAD_CNT   WED_RTQM_ENQ_I2Q_DMAD_CNT; // 0BBC
    REG_WED_RTQM_ENQ_I2N_DMAD_CNT   WED_RTQM_ENQ_I2N_DMAD_CNT; // 0BC0
    REG_WED_RTQM_ENQ_I2Q_PKT_CNT    WED_RTQM_ENQ_I2Q_PKT_CNT; // 0BC4
    REG_WED_RTQM_ENQ_I2N_PKT_CNT    WED_RTQM_ENQ_I2N_PKT_CNT; // 0BC8
    REG_WED_RTQM_ENQ_USED_ENTRY_CNT WED_RTQM_ENQ_USED_ENTRY_CNT; // 0BCC
    REG_WED_RTQM_ENQ_ERR_CNT        WED_RTQM_ENQ_ERR_CNT; // 0BD0
    REG_WED_RTQM_DEQ_CFG0           WED_RTQM_DEQ_CFG0; // 0BD4
    REG_WED_RTQM_DEQ_DQ_DMAD_CNT    WED_RTQM_DEQ_DQ_DMAD_CNT; // 0BD8
    REG_WED_RTQM_DEQ_Q2I_DMAD_CNT   WED_RTQM_DEQ_Q2I_DMAD_CNT; // 0BDC
    REG_WED_RTQM_DEQ_DQ_PKT_CNT     WED_RTQM_DEQ_DQ_PKT_CNT; // 0BE0
    REG_WED_RTQM_DEQ_Q2I_PKT_CNT    WED_RTQM_DEQ_Q2I_PKT_CNT; // 0BE4
    REG_WED_RTQM_DEQ_USED_PFDBK_CNT WED_RTQM_DEQ_USED_PFDBK_CNT; // 0BE8
    REG_WED_RTQM_DEQ_ERR_CNT        WED_RTQM_DEQ_ERR_CNT; // 0BEC
    REG_WED_RTQM_DEQ_DBG            WED_RTQM_DEQ_DBG; // 0BF0
    REG_WED_RTQM_ERR_STS            WED_RTQM_ERR_STS; // 0BF4
    REG_WED_RTQM_QUEUE_CFG0         WED_RTQM_QUEUE_CFG0; // 0BF8
    REG_WED_RTQM_QUEUE_ACC_CFG      WED_RTQM_QUEUE_ACC_CFG; // 0BFC
    UINT32                          rsv_0C00;         // 0C00
    REG_WED_RROQM_GLO_CFG           WED_RROQM_GLO_CFG; // 0C04
    REG_WED_RROQM_RST_IDX           WED_RROQM_RST_IDX; // 0C08
    REG_WED_RROQM_DLY_INT           WED_RROQM_DLY_INT; // 0C0C
    REG_WED_RTQM_QUEUE_ACC_WR0      WED_RTQM_QUEUE_ACC_WR0; // 0C10
    REG_WED_RTQM_QUEUE_ACC_WR1      WED_RTQM_QUEUE_ACC_WR1; // 0C14
    REG_WED_RTQM_QUEUE_ACC_WR2      WED_RTQM_QUEUE_ACC_WR2; // 0C18
    REG_WED_RTQM_QUEUE_ACC_WR3      WED_RTQM_QUEUE_ACC_WR3; // 0C1C
    REG_WED_RTQM_QUEUE_ACC_WR4      WED_RTQM_QUEUE_ACC_WR4; // 0C20
    REG_WED_RTQM_QUEUE_ACC_RD0      WED_RTQM_QUEUE_ACC_RD0; // 0C24
    REG_WED_RTQM_QUEUE_ACC_RD1      WED_RTQM_QUEUE_ACC_RD1; // 0C28
    REG_WED_RTQM_QUEUE_ACC_RD2      WED_RTQM_QUEUE_ACC_RD2; // 0C2C
    REG_WED_RTQM_QUEUE_ACC_RD3      WED_RTQM_QUEUE_ACC_RD3; // 0C30
    REG_WED_RTQM_QUEUE_ACC_RD4      WED_RTQM_QUEUE_ACC_RD4; // 0C34
    REG_WED_RTQM_QUEUE_CNT          WED_RTQM_QUEUE_CNT; // 0C38
    REG_WED_RTQM_DBG_CFG            WED_RTQM_DBG_CFG; // 0C3C
    REG_WED_RROQM_MIOD_CTRL0        WED_RROQM_MIOD_CTRL0; // 0C40
    REG_WED_RROQM_MIOD_CTRL1        WED_RROQM_MIOD_CTRL1; // 0C44
    REG_WED_RROQM_MIOD_CTRL2        WED_RROQM_MIOD_CTRL2; // 0C48
    REG_WED_RROQM_MIOD_CTRL3        WED_RROQM_MIOD_CTRL3; // 0C4C
    REG_WED_RROQM_FDBK_CTRL0        WED_RROQM_FDBK_CTRL0; // 0C50
    REG_WED_RROQM_FDBK_CTRL1        WED_RROQM_FDBK_CTRL1; // 0C54
    REG_WED_RROQM_FDBK_CTRL2        WED_RROQM_FDBK_CTRL2; // 0C58
    REG_WED_RROQM_FDBK_CTRL3        WED_RROQM_FDBK_CTRL3; // 0C5C
    REG_WED_RTQM_DBG_MON0           WED_RTQM_DBG_MON0; // 0C60
    REG_WED_RTQM_DBG_MON1           WED_RTQM_DBG_MON1; // 0C64
    REG_WED_RTQM_DBG_MON2           WED_RTQM_DBG_MON2; // 0C68
    REG_WED_RTQM_DBG_MON3           WED_RTQM_DBG_MON3; // 0C6C
    REG_WED_RTQM_DBG_ENQ_DATA0      WED_RTQM_DBG_ENQ_DATA0; // 0C70
    REG_WED_RTQM_DBG_ENQ_DATA1      WED_RTQM_DBG_ENQ_DATA1; // 0C74
    REG_WED_RTQM_DBG_ENQ_DATA2      WED_RTQM_DBG_ENQ_DATA2; // 0C78
    REG_WED_RTQM_DBG_ENQ_DATA3      WED_RTQM_DBG_ENQ_DATA3; // 0C7C
    REG_WED_RROQ_BASE_L             WED_RROQ_BASE_L;  // 0C80
    REG_WED_RROQ_BASE_H             WED_RROQ_BASE_H;  // 0C84
    REG_WED_RTQM_AGE_CFG0           WED_RTQM_AGE_CFG0; // 0C88
    REG_WED_RROQM_MIOD_CFG          WED_RROQM_MIOD_CFG; // 0C8C
    REG_WED_RTQM_AGE_CFG1           WED_RTQM_AGE_CFG1; // 0C90
    REG_WED_RTQM_AGE_FIFO_CFG       WED_RTQM_AGE_FIFO_CFG; // 0C94
    REG_WED_RTQM_AGE_PKT_CNT        WED_RTQM_AGE_PKT_CNT; // 0C98
    REG_WED_RTQM_PFDBK_CNT          WED_RTQM_PFDBK_CNT; // 0C9C
    UINT32                          rsv_0CA0[3];      // 0CA0..0CA8
    REG_WED_RROQM_FDBK_REC_CTRL     WED_RROQM_FDBK_REC_CTRL; // 0CAC
    REG_WED_RROQM_FDBK_REC0         WED_RROQM_FDBK_REC0; // 0CB0
    REG_WED_RROQM_FDBK_REC1         WED_RROQM_FDBK_REC1; // 0CB4
    REG_WED_RROQM_FDBK_REC2         WED_RROQM_FDBK_REC2; // 0CB8
    REG_WED_RROQM_FDBK_REC3         WED_RROQM_FDBK_REC3; // 0CBC
    REG_WED_RROQM_MID_MIB           WED_RROQM_MID_MIB; // 0CC0
    REG_WED_RROQM_MOD_MIB           WED_RROQM_MOD_MIB; // 0CC4
    REG_WED_RROQM_MOD_COH_MIB       WED_RROQM_MOD_COH_MIB; // 0CC8
    UINT32                          rsv_0CCC;         // 0CCC
    REG_WED_RROQM_FDBK_MIB          WED_RROQM_FDBK_MIB; // 0CD0
    REG_WED_RROQM_FDBK_COH_MIB      WED_RROQM_FDBK_COH_MIB; // 0CD4
    UINT32                          rsv_0CD8[2];      // 0CD8..0CDC
    REG_WED_RROQM_FDBK_IND_MIB      WED_RROQM_FDBK_IND_MIB; // 0CE0
    REG_WED_RROQM_FDBK_ENQ_MIB      WED_RROQM_FDBK_ENQ_MIB; // 0CE4
    REG_WED_RROQM_FDBK_ANC_MIB      WED_RROQM_FDBK_ANC_MIB; // 0CE8
    REG_WED_RROQM_FDBK_ANC2H_MIB    WED_RROQM_FDBK_ANC2H_MIB; // 0CEC
    UINT32                          rsv_0CF0[4];      // 0CF0..0CFC
    REG_WED_TX_BM_DYN_ALLOC_TH      WED_TX_BM_DYN_ALLOC_TH; // 0D00
    REG_WED_TX_BM_DYN_ALLOC_CFG     WED_TX_BM_DYN_ALLOC_CFG; // 0D04
    REG_WED_TX_BM_ADD_BASE          WED_TX_BM_ADD_BASE; // 0D08
    REG_WED_TX_BM_ADD_PTR           WED_TX_BM_ADD_PTR; // 0D0C
    REG_WED_TX_BM_TOTAL_DMAD_IDX    WED_TX_BM_TOTAL_DMAD_IDX; // 0D10
    REG_WED_TX_BM_ADD_BASE_H        WED_TX_BM_ADD_BASE_H; // 0D14
    UINT32                          rsv_0D18[26];     // 0D18..0D7C
    REG_WED_RX_BM_RX_DMAD           WED_RX_BM_RX_DMAD; // 0D80
    REG_WED_RX_BM_BASE              WED_RX_BM_BASE;   // 0D84
    REG_WED_RX_BM_INIT_PTR          WED_RX_BM_INIT_PTR; // 0D88
    REG_WED_RX_BM_PTR               WED_RX_BM_PTR;    // 0D8C
    REG_WED_RX_BM_BLEN              WED_RX_BM_BLEN;   // 0D90
    REG_WED_RX_BM_STS               WED_RX_BM_STS;    // 0D94
    REG_WED_RX_BM_INTF2             WED_RX_BM_INTF2;  // 0D98
    REG_WED_RX_BM_INTF              WED_RX_BM_INTF;   // 0D9C
    UINT32                          rsv_0DA0[2];      // 0DA0..0DA4
    REG_WED_RX_BM_ERR_STS           WED_RX_BM_ERR_STS; // 0DA8
    REG_WED_RX_BM_OPT_CTRL          WED_RX_BM_OPT_CTRL; // 0DAC
    UINT32                          rsv_0DB0;         // 0DB0
    REG_WED_RX_BM_DYN_ALLOC_TH      WED_RX_BM_DYN_ALLOC_TH; // 0DB4
    REG_WED_RX_BM_DYN_ALLOC_CFG     WED_RX_BM_DYN_ALLOC_CFG; // 0DB8
    REG_WED_RX_BM_ADD_BASE          WED_RX_BM_ADD_BASE; // 0DBC
    REG_WED_RX_BM_ADD_PTR           WED_RX_BM_ADD_PTR; // 0DC0
    REG_WED_RX_BM_TOTAL_DMAD_IDX    WED_RX_BM_TOTAL_DMAD_IDX; // 0DC4
    REG_WED_RX_BM_ADD_BASE_H        WED_RX_BM_ADD_BASE_H; // 0DC8
    UINT32                          rsv_0DCC[13];     // 0DCC..0DFC
    REG_RRO_IND_CMD_0_SIGNATURE     RRO_IND_CMD_0_SIGNATURE; // 0E00
    REG_IND_CMD_0_CTRL_0            IND_CMD_0_CTRL_0; // 0E04
    REG_IND_CMD_0_CTRL_1            IND_CMD_0_CTRL_1; // 0E08
    REG_IND_CMD_0_CTRL_2            IND_CMD_0_CTRL_2; // 0E0C
    REG_RRO_CONF_0                  RRO_CONF_0;       // 0E10
    REG_RRO_CONF_1                  RRO_CONF_1;       // 0E14
    REG_ADDR_ELEM_CONF_0            ADDR_ELEM_CONF_0; // 0E18
    REG_ADDR_ELEM_CONF_1            ADDR_ELEM_CONF_1; // 0E1C
    REG_ADDR_ELEM_BASE_TBL_CONF     ADDR_ELEM_BASE_TBL_CONF; // 0E20
    REG_ADDR_ELEM_BASE_TBL_WDATA    ADDR_ELEM_BASE_TBL_WDATA; // 0E24
    REG_ADDR_ELEM_BASE_TBL_RDATA    ADDR_ELEM_BASE_TBL_RDATA; // 0E28
    UINT32                          rsv_0E2C;         // 0E2C
    REG_PN_CONF_0                   PN_CONF_0;        // 0E30
    REG_PN_CONF_WDATA_L             PN_CONF_WDATA_L;  // 0E34
    REG_PN_CONF_WDATA_M             PN_CONF_WDATA_M;  // 0E38
    REG_PN_CONF_RDATA_L             PN_CONF_RDATA_L;  // 0E3C
    REG_PN_CONF_RDATA_M             PN_CONF_RDATA_M;  // 0E40
    REG_RRO_MSDU_PG_RING0_CFG0      RRO_MSDU_PG_RING0_CFG0; // 0E44
    REG_RRO_MSDU_PG_RING0_CFG1      RRO_MSDU_PG_RING0_CFG1; // 0E48
    REG_RRO_MSDU_PG_RING1_CFG0      RRO_MSDU_PG_RING1_CFG0; // 0E4C
    REG_RRO_MSDU_PG_RING1_CFG1      RRO_MSDU_PG_RING1_CFG1; // 0E50
    REG_RRO_MSDU_PG_RING2_CFG0      RRO_MSDU_PG_RING2_CFG0; // 0E54
    REG_RRO_MSDU_PG_RING2_CFG1      RRO_MSDU_PG_RING2_CFG1; // 0E58
    REG_RRO_MSDU_PG_0_CTRL0         RRO_MSDU_PG_0_CTRL0; // 0E5C
    REG_RRO_MSDU_PG_0_CTRL1         RRO_MSDU_PG_0_CTRL1; // 0E60
    REG_RRO_MSDU_PG_0_CTRL2         RRO_MSDU_PG_0_CTRL2; // 0E64
    REG_RRO_MSDU_PG_1_CTRL0         RRO_MSDU_PG_1_CTRL0; // 0E68
    REG_RRO_MSDU_PG_1_CTRL1         RRO_MSDU_PG_1_CTRL1; // 0E6C
    REG_RRO_MSDU_PG_1_CTRL2         RRO_MSDU_PG_1_CTRL2; // 0E70
    REG_RRO_MSDU_PG_2_CTRL0         RRO_MSDU_PG_2_CTRL0; // 0E74
    REG_RRO_MSDU_PG_2_CTRL1         RRO_MSDU_PG_2_CTRL1; // 0E78
    REG_RRO_MSDU_PG_2_CTRL2         RRO_MSDU_PG_2_CTRL2; // 0E7C
    REG_RRO_RX_D_RX0_BASE           RRO_RX_D_RX0_BASE; // 0E80
    REG_RRO_RX_D_RX0_CNT            RRO_RX_D_RX0_CNT; // 0E84
    REG_RRO_RX_D_RX0_CRX_IDX        RRO_RX_D_RX0_CRX_IDX; // 0E88
    REG_RRO_RX_D_RX0_DRX_IDX        RRO_RX_D_RX0_DRX_IDX; // 0E8C
    REG_RRO_RX_D_RX1_BASE           RRO_RX_D_RX1_BASE; // 0E90
    REG_RRO_RX_D_RX1_CNT            RRO_RX_D_RX1_CNT; // 0E94
    REG_RRO_RX_D_RX1_CRX_IDX        RRO_RX_D_RX1_CRX_IDX; // 0E98
    REG_RRO_RX_D_RX1_DRX_IDX        RRO_RX_D_RX1_DRX_IDX; // 0E9C
    REG_RRO_RX_D_RING_CFG_ADDR_0    RRO_RX_D_RING_CFG_ADDR_0; // 0EA0
    REG_RRO_RX_D_RING_CFG_ADDR_1    RRO_RX_D_RING_CFG_ADDR_1; // 0EA4
    REG_RRO_RX_D_RING_CFG_ADDR_2    RRO_RX_D_RING_CFG_ADDR_2; // 0EA8
    UINT32                          rsv_0EAC;         // 0EAC
    REG_RRO_PG_BM_RX_DMAD           RRO_PG_BM_RX_DMAD; // 0EB0
    REG_RRO_PG_BM_BASE              RRO_PG_BM_BASE;   // 0EB4
    REG_RRO_PG_BM_INIT_PTR          RRO_PG_BM_INIT_PTR; // 0EB8
    REG_RRO_PG_BM_PTR               RRO_PG_BM_PTR;    // 0EBC
    REG_RRO_PG_BM_ADD_BASE_H        RRO_PG_BM_ADD_BASE_H; // 0EC0
    REG_RRO_PG_BM_STS               RRO_PG_BM_STS;    // 0EC4
    UINT32                          rsv_0EC8;         // 0EC8
    REG_RRO_PG_BM_INTF              RRO_PG_BM_INTF;   // 0ECC
    REG_RRO_PG_BM_ERR_STS           RRO_PG_BM_ERR_STS; // 0ED0
    REG_RRO_PG_BM_OPT_CTRL          RRO_PG_BM_OPT_CTRL; // 0ED4
    REG_RRO_PG_BM_DYN_ALLOC_TH      RRO_PG_BM_DYN_ALLOC_TH; // 0ED8
    REG_RRO_PG_BM_DYN_ALLOC_CFG     RRO_PG_BM_DYN_ALLOC_CFG; // 0EDC
    REG_RRO_PG_BM_ADD_BASE          RRO_PG_BM_ADD_BASE; // 0EE0
    REG_RRO_PG_BM_ADD_PTR           RRO_PG_BM_ADD_PTR; // 0EE4
    REG_RRO_PG_BM_TOTAL_DMAD_IDX    RRO_PG_BM_TOTAL_DMAD_IDX; // 0EE8
    REG_WED_WPDMA_INT_CTRL_RRO_RX   WED_WPDMA_INT_CTRL_RRO_RX; // 0EEC
    UINT32                          rsv_0EF0;         // 0EF0
    REG_WED_WPDMA_INT_CTRL_RRO_MSDU_PG WED_WPDMA_INT_CTRL_RRO_MSDU_PG; // 0EF4
    UINT32                          rsv_0EF8[2];      // 0EF8..0EFC
    REG_WED_RRO_RX_HW_STS           WED_RRO_RX_HW_STS; // 0F00
    REG_WED_RRO_RX_DBG0             WED_RRO_RX_DBG0;  // 0F04
    REG_WED_RRO_RX_DBG1             WED_RRO_RX_DBG1;  // 0F08
    REG_WED_RRO_RX_DBG2             WED_RRO_RX_DBG2;  // 0F0C
    REG_WED_RRO_RX_DBG3             WED_RRO_RX_DBG3;  // 0F10
    REG_WED_RRO_RX_DBG4             WED_RRO_RX_DBG4;  // 0F14
    UINT32                          rsv_0F18[2];      // 0F18..0F1C
    REG_WED_RX_IND_CMD_CNT0         WED_RX_IND_CMD_CNT0; // 0F20
    REG_WED_RX_IND_CMD_CNT1         WED_RX_IND_CMD_CNT1; // 0F24
    REG_WED_RX_IND_CMD_CNT2         WED_RX_IND_CMD_CNT2; // 0F28
    REG_WED_RX_IND_CMD_CNT3         WED_RX_IND_CMD_CNT3; // 0F2C
    REG_WED_RX_IND_CMD_CNT4         WED_RX_IND_CMD_CNT4; // 0F30
    REG_WED_RX_IND_CMD_CNT5         WED_RX_IND_CMD_CNT5; // 0F34
    REG_WED_RX_IND_CMD_CNT6         WED_RX_IND_CMD_CNT6; // 0F38
    REG_WED_RX_IND_CMD_CNT7         WED_RX_IND_CMD_CNT7; // 0F3C
    REG_WED_RX_IND_CMD_CNT8         WED_RX_IND_CMD_CNT8; // 0F40
    REG_WED_RX_IND_CMD_CNT9         WED_RX_IND_CMD_CNT9; // 0F44
    REG_WED_RX_ADDR_ELEM_CNT0       WED_RX_ADDR_ELEM_CNT0; // 0F48
    REG_WED_RX_ADDR_ELEM_CNT1       WED_RX_ADDR_ELEM_CNT1; // 0F4C
    REG_WED_RX_ADDR_ELEM_CNT2       WED_RX_ADDR_ELEM_CNT2; // 0F50
    REG_WED_RX_ADDR_ELEM_CNT3       WED_RX_ADDR_ELEM_CNT3; // 0F54
    REG_WED_RX_ADDR_ELEM_CNT4       WED_RX_ADDR_ELEM_CNT4; // 0F58
    REG_WED_RX_MSDU_PG_CNT1         WED_RX_MSDU_PG_CNT1; // 0F5C
    REG_WED_RX_MSDU_PG_CNT2         WED_RX_MSDU_PG_CNT2; // 0F60
    REG_WED_RX_MSDU_PG_CNT3         WED_RX_MSDU_PG_CNT3; // 0F64
    REG_WED_RX_MSDU_PG_CNT4         WED_RX_MSDU_PG_CNT4; // 0F68
    REG_WED_RX_MSDU_PG_CNT5         WED_RX_MSDU_PG_CNT5; // 0F6C
    REG_WED_RX_PN_CHK_CNT0          WED_RX_PN_CHK_CNT0; // 0F70
    REG_WED_RX_IND_CMD_AXI0         WED_RX_IND_CMD_AXI0; // 0F74
    REG_WED_RX_ADDR_ELEM_AXI0       WED_RX_ADDR_ELEM_AXI0; // 0F78
    REG_WED_RX_ADDR_ELEM_AXI1       WED_RX_ADDR_ELEM_AXI1; // 0F7C
    REG_WED_RX_ADDR_ELEM_AXI2       WED_RX_ADDR_ELEM_AXI2; // 0F80
    REG_WED_RX_ADDR_ELEM_AXI3       WED_RX_ADDR_ELEM_AXI3; // 0F84
    REG_WED_RX_ADDR_ELEM_AXI4       WED_RX_ADDR_ELEM_AXI4; // 0F88
    REG_WED_RX_MSDU_PG_AXI0         WED_RX_MSDU_PG_AXI0; // 0F8C
    UINT32                          rsv_0F90[11];     // 0F90..0FB8
    REG_DONT_USE_BEHIND_THIS_ADDR   DONT_USE_BEHIND_THIS_ADDR; // 0FBC
}wed0_REGS, *Pwed0_REGS;

// ---------- wed0 Enum Definitions      ----------
// ---------- wed0 C Macro Definitions   ----------


#define WED_ID                                                 0x00000000
#define WED_REV_ID                                             0x00000004
#define WED_MOD_RST                                            0x00000008
#define WED_CTRL                                               0x0000000C
#define WED_AXI_CTRL                                           0x00000010
#define WED_CTRL2                                              0x0000001C
#define WED_EX_INT_STA                                         0x00000020
#define WED_EX_INT_MSK                                         0x00000028
#define WED_EX_INT_MSK1                                        0x0000002C
#define WED_EX_INT_MSK2                                        0x00000030
#define WED_EX_INT_MSK3                                        0x00000034
#define WED_EX_INT_STA_POST_MSK0                               0x00000040
#define WED_EX_INT_STA_POST_MSK1                               0x00000044
#define WED_EX_INT_STA_POST_MSK2                               0x00000048
#define WED_EX_INT_STA_POST_MSK3                               0x0000004C
#define WED_IRQ_MON                                            0x00000050
#define WED_ERR_MON                                            0x00000054
#define WED_ST                                                 0x00000060
#define WED_WPDMA_ST                                           0x00000064
#define WED_WDMA_ST                                            0x00000068
#define WED_BM_ST                                              0x0000006C
#define WED_QM_ST                                              0x00000070
#define WED_WPDMA_D_ST                                         0x00000074
#define WED_TX_BM_CTRL                                         0x00000080
#define WED_TX_BM_BASE                                         0x00000084
#define WED_TX_BM_INIT_PTR                                     0x00000088
#define WED_TX_BM_BLEN                                         0x0000008C
#define WED_TX_BM_STS                                          0x00000090
#define WED_TX_BM_PTR                                          0x00000094
#define WED_TX_BM_INTF                                         0x0000009C
#define WED_TX_BM_DYN_TH                                       0x000000A0
#define WED_TX_BM_ERR_STS                                      0x000000A4
#define WED_TX_BM_RECYC                                        0x000000A8
#define WED_TX_BM_MON_CTRL                                     0x000000AC
#define WED_TX_BM_VB_FREE_0_31                                 0x000000B0
#define WED_TX_BM_VB_FREE_32_63                                0x000000B4
#define WED_TX_BM_VB_USED_0_31                                 0x000000B8
#define WED_TX_BM_VB_USED_32_63                                0x000000BC
#define WED_TX_TKID_CTRL                                       0x000000C0
#define WED_TX_TKID_SPR                                        0x000000C4
#define WED_TX_TKID_TKID                                       0x000000C8
#define WED_TX_TKID_INTF                                       0x000000DC
#define WED_TX_TKID_DYN_TH                                     0x000000E0
#define WED_TX_TKID_STS                                        0x000000E4
#define WED_TX_TKID_RECYC                                      0x000000E8
#define WED_TX_TKID_ALI                                        0x000000EC
#define WED_TX_TKID_VB_FREE_0_31                               0x000000F0
#define WED_TX_TKID_VB_FREE_32_63                              0x000000F4
#define WED_TX_TKID_VB_USED_0_31                               0x000000F8
#define WED_TX_TKID_VB_USED_32_63                              0x000000FC
#define WED_TXD_DW0                                            0x00000100
#define WED_TXD_DW1                                            0x00000104
#define WED_TXD_DW2                                            0x00000108
#define WED_TXD_DW3                                            0x0000010C
#define WED_TXD_DW4                                            0x00000110
#define WED_TXD_DW5                                            0x00000114
#define WED_TXD_DW6                                            0x00000118
#define WED_TXD_DW7                                            0x0000011C
#define WED_TXP_DW0                                            0x00000120
#define WED_TXP_DW1                                            0x00000124
#define WED_TXDP_CTRL                                          0x00000130
#define WED_DBG_CTRL                                           0x00000180
#define WED_DBG_BUS_ADDR_L                                     0x00000184
#define WED_DBG_BUS_ADDR_H                                     0x00000188
#define WED_DBG_BUS_MON                                        0x0000018C
#define WED_DBG_PRB0                                           0x00000190
#define WED_DBG_PRB1                                           0x00000194
#define WED_DBG_PRB2                                           0x00000198
#define WED_DBG_PRB3                                           0x0000019C
#define WED_TX_TKID_FREE_TKID_IN                               0x000001B0
#define WED_TX_TKID_FREE_TKID_LDDRAM                           0x000001B4
#define WED_TX_TKID_FREE_TKID_LDSRAM                           0x000001B8
#define WED_TX_TKID_FREE_TKID_RECYCLE                          0x000001BC
#define WED_TX_FREE_TO_TX_TKID_TKID_MIB                        0x000001C0
#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB                     0x000001C4
#define WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB                     0x000001C8
#define WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB                   0x000001CC
#define WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB                    0x000001D0
#define WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB                     0x000001D4
#define WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB                  0x000001D8
#define WED_PMTR_CTRL                                          0x000001DC
#define WED_PMTR_TGT                                           0x000001E0
#define WED_PMTR_TGT_ST                                        0x000001E4
#define WED_PMTR_LTCY_MAX0_1                                   0x000001E8
#define WED_PMTR_LTCY_MAX2_3                                   0x000001EC
#define WED_PMTR_LTCY_ACC0                                     0x000001F0
#define WED_PMTR_LTCY_ACC1                                     0x000001F4
#define WED_PMTR_LTCY_ACC2                                     0x000001F8
#define WED_PMTR_LTCY_ACC3                                     0x000001FC
#define WED_INT_STA                                            0x00000200
#define WED_INT_MSK                                            0x00000204
#define WED_GLO_CFG                                            0x00000208
#define WED_RST_IDX                                            0x0000020C
#define WED_DLY_INT_CFG                                        0x00000210
#define WED_DLY_INT_CFG1                                       0x00000214
#define WED_SPR                                                0x0000021C
#define WED_INT_MSK1                                           0x00000224
#define WED_INT_MSK2                                           0x00000228
#define WED_INT_MSK3                                           0x0000022C
#define WED_INT_STA_POST_MSK0                                  0x00000230
#define WED_INT_STA_POST_MSK1                                  0x00000234
#define WED_INT_STA_POST_MSK2                                  0x00000238
#define WED_INT_STA_POST_MSK3                                  0x0000023C
#define WED_SPR_0X24C                                          0x0000024C
#define WED_SPR_0X25C                                          0x0000025C
#define WED_TX0_MIB                                            0x000002A0
#define WED_TX1_MIB                                            0x000002A4
#define WED_TX_COHERENT_MIB                                    0x000002D0
#define WED_RX0_MIB                                            0x000002E0
#define WED_RX1_MIB                                            0x000002E4
#define WED_RX_DMA_MIB                                         0x000002F0
#define WED_TX0_CTRL0                                          0x00000300
#define WED_TX0_CTRL1                                          0x00000304
#define WED_TX0_CTRL2                                          0x00000308
#define WED_TX0_CTRL3                                          0x0000030C
#define WED_TX1_CTRL0                                          0x00000310
#define WED_TX1_CTRL1                                          0x00000314
#define WED_TX1_CTRL2                                          0x00000318
#define WED_TX1_CTRL3                                          0x0000031C
#define WED_SCR0                                               0x000003C0
#define WED_SCR1                                               0x000003C4
#define WED_SCR2                                               0x000003C8
#define WED_SCR3                                               0x000003CC
#define WED_SCR4                                               0x000003D0
#define WED_SCR5                                               0x000003D4
#define WED_SCR6                                               0x000003D8
#define WED_SCR7                                               0x000003DC
#define WED_RX0_CTRL0                                          0x00000400
#define WED_RX0_CTRL1                                          0x00000404
#define WED_RX0_CTRL2                                          0x00000408
#define WED_RX0_CTRL3                                          0x0000040C
#define WED_RX1_CTRL0                                          0x00000410
#define WED_RX1_CTRL1                                          0x00000414
#define WED_RX1_CTRL2                                          0x00000418
#define WED_RX1_CTRL3                                          0x0000041C
#define WED_RX_BASE_PTR_0                                      0x00000420
#define WED_RX_MAX_CNT_0                                       0x00000424
#define WED_RX_CRX_IDX_0                                       0x00000428
#define WED_RX_DRX_IDX_0                                       0x0000042C
#define WED_RX_BASE_PTR_1                                      0x00000430
#define WED_RX_MAX_CNT_1                                       0x00000434
#define WED_RX_CRX_IDX_1                                       0x00000438
#define WED_RX_DRX_IDX_1                                       0x0000043C
#define WED_WPDMA_INT_STA_REC                                  0x00000500
#define WED_WPDMA_INT_TRIG                                     0x00000504
#define WED_WPDMA_GLO_CFG                                      0x00000508
#define WED_WPDMA_RST_IDX                                      0x0000050C
#define WED_WPDMA_BUS_CFG                                      0x00000510
#define WED_WPDMA_TX_DLY_CIDX                                  0x00000514
#define WED_WPDMA_CTRL                                         0x00000518
#define WED_WPDMA_FORCE_PROC                                   0x0000051C
#define WED_WPDMA_INT_CTRL                                     0x00000520
#define WED_WPDMA_INT_MSK                                      0x00000524
#define WED_WPDMA_INT_CLR                                      0x00000528
#define WED_WPDMA_INT_MON                                      0x0000052C
#define WED_WPDMA_INT_CTRL_TX                                  0x00000530
#define WED_WPDMA_INT_CTRL_RX                                  0x00000534
#define WED_WPDMA_INT_CTRL_TX_FREE                             0x00000538
#define WED_WPDMA_SPR                                          0x0000053C
#define WED_PCIE_MSIS_TRIG                                     0x00000540
#define WED_PCIE_MSIS_REC                                      0x00000544
#define WED_PCIE_MSIS_CLR                                      0x00000548
#define WED_PCIE_INTS_CLR                                      0x00000550
#define WED_PCIE_EP_INTS_CLR                                   0x00000554
#define WED_PCIE_CFG_ADDR_H                                    0x0000055C
#define WED_PCIE_CFG_ADDR_INTS                                 0x00000560
#define WED_PCIE_CFG_ADDR_INTM                                 0x00000564
#define WED_PCIE_CFG_ADDR_MSIS                                 0x00000568
#define WED_PCIE_CFG_ADDR_INTS_EP                              0x0000056C
#define WED_PCIE_INTS_TRIG                                     0x00000570
#define WED_PCIE_INTS_REC                                      0x00000574
#define WED_PCIE_INTM_REC                                      0x00000578
#define WED_PCIE_INT_CTRL                                      0x0000057C
#define WED_WPDMA_CFG_ADDR_INTS                                0x00000580
#define WED_WPDMA_CFG_ADDR_INTM                                0x00000584
#define WED_WPDMA_CFG_ADDR_TX                                  0x00000588
#define WED_WPDMA_CFG_ADDR_TX_FREE                             0x0000058C
#define WED_WPDMA_CFG_ADDR_INT_H                               0x00000590
#define WED_WPDMA_CFG_ADDR_TX_H                                0x00000598
#define WED_WPDMA_CFG_ADDR_TX_FREE_H                           0x0000059C
#define WED_WPDMA_TX0_MIB                                      0x000005A0
#define WED_WPDMA_TX1_MIB                                      0x000005A4
#define WED_WPDMA_TX_COHERENT_MIB                              0x000005D0
#define WED_WPDMA_RX0_MIB                                      0x000005E0
#define WED_WPDMA_RX1_MIB                                      0x000005E4
#define WED_WPDMA_RX_COHERENT_MIB                              0x000005F0
#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB                        0x000005F8
#define WED_WPDMA_TX0_CTRL0                                    0x00000600
#define WED_WPDMA_TX0_CTRL1                                    0x00000604
#define WED_WPDMA_TX0_CTRL2                                    0x00000608
#define WED_WPDMA_TX0_CTRL3                                    0x0000060C
#define WED_WPDMA_TX1_CTRL0                                    0x00000610
#define WED_WPDMA_TX1_CTRL1                                    0x00000614
#define WED_WPDMA_TX1_CTRL2                                    0x00000618
#define WED_WPDMA_TX1_CTRL3                                    0x0000061C
#define RRO_RX_D_DRV_MON_R0_WRAP                               0x00000640
#define RRO_RX_D_DRV_MON_R1_WRAP                               0x00000644
#define RRO_RX_D_DRV_MON_R2_WRAP                               0x00000648
#define RRO_RX_D_DRV_MON0                                      0x0000064C
#define RRO_RX_D_DRV_MON1                                      0x00000650
#define RRO_RX_D_DRV_MON2                                      0x00000654
#define RRO_RX_D_DRV_ERR_COMP0                                 0x00000658
#define RRO_RX_D_DRV_ERR_COMP1                                 0x0000065C
#define RRO_RX_D_DRV_ERR_COMP2                                 0x00000660
#define RRO_RX_D_DRV_ERR_COMP3                                 0x00000664
#define RRO_RX_D_DRV_LEAGLE_START                              0x00000668
#define RRO_RX_D_DRV_LEAGLE_END                                0x0000066C
#define RRO_RX_PG_DRV_MON_R0_WRAP                              0x00000670
#define RRO_RX_PG_DRV_MON_R1_WRAP                              0x00000674
#define RRO_RX_PG_DRV_MON_R2_WRAP                              0x00000678
#define RRO_RX_PG_DRV_MON0                                     0x0000067C
#define RRO_RX_PG_DRV_MON1                                     0x00000680
#define RRO_RX_PG_DRV_MON2                                     0x00000684
#define RRO_RX_PG_DRV_ERR_COMP0                                0x00000688
#define RRO_RX_PG_DRV_ERR_COMP1                                0x0000068C
#define RRO_RX_PG_DRV_ERR_COMP2                                0x00000690
#define RRO_RX_PG_DRV_ERR_COMP3                                0x00000694
#define RRO_RX_PG_DRV_LEAGLE_START                             0x00000698
#define RRO_RX_PG_DRV_LEAGLE_END                               0x0000069C
#define RRO_RX_D_DRV_ERR_SWITCH_ADDR                           0x000006A0
#define RRO_RX_PG_DRV_ERR_SWITCH_ADDR                          0x000006A4
#define RRO_RX_D_DRV_IN_MIB                                    0x000006A8
#define RRO_RX_D_DRV_OU_MIB                                    0x000006AC
#define RRO_RX_D_DRV_DESC_MIB                                  0x000006B0
#define RRO_RX_PG_DRV_IN_MIB                                   0x000006B4
#define RRO_RX_PG_DRV_OU_MIB                                   0x000006B8
#define RRO_RX_PG_DRV_DESC_MIB                                 0x000006BC
#define WED_WPDMA_RX0_CTRL0                                    0x00000700
#define WED_WPDMA_RX0_CTRL1                                    0x00000704
#define WED_WPDMA_RX0_CTRL2                                    0x00000708
#define WED_WPDMA_RX0_CTRL3                                    0x0000070C
#define WED_WPDMA_RX1_CTRL0                                    0x00000710
#define WED_WPDMA_RX1_CTRL1                                    0x00000714
#define WED_WPDMA_RX1_CTRL2                                    0x00000718
#define WED_WPDMA_RX1_CTRL3                                    0x0000071C
#define WED_WPDMA_RX_D_RX0_BASE                                0x00000730
#define WED_WPDMA_RX_D_RX0_CNT                                 0x00000734
#define WED_WPDMA_RX_D_RX0_CRX_IDX                             0x00000738
#define WED_WPDMA_RX_D_RX0_DRX_IDX                             0x0000073C
#define WED_WPDMA_RX_D_RX1_BASE                                0x00000740
#define WED_WPDMA_RX_D_RX1_CNT                                 0x00000744
#define WED_WPDMA_RX_D_RX1_CRX_IDX                             0x00000748
#define WED_WPDMA_RX_D_RX1_DRX_IDX                             0x0000074C
#define WED_WPDMA_RX_D_RX0_THRES_CFG                           0x00000750
#define WED_WPDMA_RX_D_RX1_THRES_CFG                           0x00000754
#define WED_WPDMA_RX_D_INFO                                    0x00000758
#define WED_WPDMA_RX_D_GLO_CFG                                 0x0000075C
#define WED_WPDMA_RX_D_RST_IDX                                 0x00000760
#define WED_WPDMA_RX_D_LOAD_DRV_IDX                            0x00000764
#define WED_WPDMA_RX_D_LOAD_CRX_IDX                            0x00000768
#define WED_WPDMA_RX_D_GLO_CFG_ADDR                            0x0000076C
#define WED_WPDMA_RX_D_GLO_CFG_ADDR_H                          0x00000770
#define WED_WPDMA_RX_D_RX0_MIB                                 0x00000774
#define WED_WPDMA_RX_D_RX1_MIB                                 0x00000778
#define WED_WPDMA_RX_D_RX0_RECYCLE_MIB                         0x0000077C
#define WED_WPDMA_RX_D_RX1_RECYCLE_MIB                         0x00000780
#define WED_WPDMA_RX_D_RX0_PROCESSED_MIB                       0x00000784
#define WED_WPDMA_RX_D_RX1_PROCESSED_MIB                       0x00000788
#define WED_WPDMA_RX_D_RX_COHERENT_MIB                         0x0000078C
#define WED_WPDMA_RX_D_ERR_STS                                 0x00000790
#define WED_WPDMA_RX_D_TKID_CFG                                0x000007A0
#define WED_WPDMA_RX_D_TKID_RX0_BASE                           0x000007A4
#define WED_WPDMA_RX_D_TKID_RX1_BASE                           0x000007A8
#define WED_WPDMA_RX_D_TKID_RX_BASE_H                          0x000007AC
#define WED_WPDMA_RX_D_BUS_CFG                                 0x000007B0
#define WED_WPDMA_RX_D_PREF_CFG                                0x000007B4
#define WED_WPDMA_RX_D_PREF_RX0_SIDX                           0x000007B8
#define WED_WPDMA_RX_D_PREF_RX1_SIDX                           0x000007BC
#define WED_WPDMA_RX_D_PREF_FIFO_CFG                           0x000007C0
#define WED_WPDMA_RX_D_CIDX_WR_CFG                             0x000007CC
#define WED_WPDMA_RX_D_RING0_CFG_ADDR                          0x000007D0
#define WED_WPDMA_RX_D_RING0_CFG_ADDR_H                        0x000007D4
#define WED_WPDMA_RX_D_RING1_CFG_ADDR                          0x000007D8
#define WED_WPDMA_RX_D_RING1_CFG_ADDR_H                        0x000007DC
#define WED_WPDMA_RX_D_AW_RAN_DBG0                             0x000007E0
#define WED_WPDMA_RX_D_AW_RAN_DBG1                             0x000007E4
#define WED_WPDMA_RX_D_AW_RAN_DBG2                             0x000007E8
#define WED_WPDMA_RX_D_AW_RAN_DBG3                             0x000007EC
#define WED_WPDMA_RX_D_AW_RAN_DBG4                             0x000007F0
#define WED_WPDMA_RX_D_AW_RAN_DBG5                             0x000007F4
#define WED_WDMA_TX0_BASE                                      0x00000800
#define WED_WDMA_TX0_CNT                                       0x00000804
#define WED_WDMA_TX0_CTX_IDX                                   0x00000808
#define WED_WDMA_TX0_DTX_IDX                                   0x0000080C
#define WED_WDMA_TX0_MIB                                       0x00000810
#define WED_WDMA_RX0_BASE                                      0x00000900
#define WED_WDMA_RX0_CNT                                       0x00000904
#define WED_WDMA_RX0_CRX_IDX                                   0x00000908
#define WED_WDMA_RX0_DRX_IDX                                   0x0000090C
#define WED_WDMA_RX1_BASE                                      0x00000910
#define WED_WDMA_RX1_CNT                                       0x00000914
#define WED_WDMA_RX1_CRX_IDX                                   0x00000918
#define WED_WDMA_RX1_DRX_IDX                                   0x0000091C
#define WED_WDMA_RX0_THRES_CFG                                 0x00000940
#define WED_WDMA_RX1_THRES_CFG                                 0x00000944
#define WED_WDMA_RX_PREF_CFG                                   0x00000950
#define WED_WDMA_RX_PREF_RX0_SIDX                              0x00000954
#define WED_WDMA_RX_PREF_RX1_SIDX                              0x00000958
#define WED_WDMA_RX_PREF_FIFO_CFG                              0x0000095C
#define WED_WDMA_RX_CIDX_WR_CFG                                0x00000960
#define WED_WDMA_INFO                                          0x00000A00
#define WED_WDMA_GLO_CFG                                       0x00000A04
#define WED_WDMA_RST_IDX                                       0x00000A08
#define WED_WDMA_LOAD_IDX                                      0x00000A0C
#define WED_WDMA_LOAD_DRV_IDX                                  0x00000A10
#define WED_WDMA_LOAD_CRX_IDX                                  0x00000A14
#define WED_WDMA_SPR                                           0x00000A1C
#define WED_WDMA_INT_STA_REC                                   0x00000A20
#define WED_WDMA_INT_CLR                                       0x00000A24
#define WED_WDMA_INT_TRIG                                      0x00000A28
#define WED_WDMA_INT_CTRL                                      0x00000A2C
#define WED_WDMA_INT_MON                                       0x00000A3C
#define WED_WDMA_CFG_BASE                                      0x00000AA0
#define WED_WDMA_OFST0                                         0x00000AA4
#define WED_WDMA_OFST1                                         0x00000AA8
#define WED_WDMA_CFG_BASE_H                                    0x00000AAC
#define WED_WDMA_RX0_MIB                                       0x00000AE0
#define WED_WDMA_RX1_MIB                                       0x00000AE4
#define WED_WDMA_RX0_RECYCLE_MIB                               0x00000AE8
#define WED_WDMA_RX1_RECYCLE_MIB                               0x00000AEC
#define WED_WDMA_RX0_PROCESSED_MIB                             0x00000AF0
#define WED_WDMA_RX1_PROCESSED_MIB                             0x00000AF4
#define WED_WDMA_RX_COHERENT_MIB                               0x00000AF8
#define WED_RTQM_GLO_CFG                                       0x00000B00
#define WED_RTQM_RST                                           0x00000B04
#define WED_RTQM_IGRS_FIFO_CFG                                 0x00000B08
#define WED_RTQM_PFDBK_FIFO_CFG                                0x00000B0C
#define WED_RTQM_PREF_FIFO_CFG                                 0x00000B10
#define WED_RTQM_IGRS0_CFG0                                    0x00000B14
#define WED_RTQM_IGRS0_CFG1                                    0x00000B18
#define WED_RTQM_IGRS0_I2HW_DMAD_CNT                           0x00000B1C
#define WED_RTQM_IGRS0_I2H0_DMAD_CNT                           0x00000B20
#define WED_RTQM_IGRS0_I2H1_DMAD_CNT                           0x00000B24
#define WED_RTQM_IGRS0_I2HW_PKT_CNT                            0x00000B28
#define WED_RTQM_IGRS0_I2H0_PKT_CNT                            0x00000B2C
#define WED_RTQM_IGRS0_I2H1_PKT_CNT                            0x00000B30
#define WED_RTQM_IGRS0_FDROP_CNT                               0x00000B34
#define WED_RTQM_IGRS0_DBG                                     0x00000B38
#define WED_RTQM_IGRS1_CFG0                                    0x00000B3C
#define WED_RTQM_IGRS1_CFG1                                    0x00000B40
#define WED_RTQM_IGRS1_I2HW_DMAD_CNT                           0x00000B44
#define WED_RTQM_IGRS1_I2H0_DMAD_CNT                           0x00000B48
#define WED_RTQM_IGRS1_I2H1_DMAD_CNT                           0x00000B4C
#define WED_RTQM_IGRS1_I2HW_PKT_CNT                            0x00000B50
#define WED_RTQM_IGRS1_I2H0_PKT_CNT                            0x00000B54
#define WED_RTQM_IGRS1_I2H1_PKT_CNT                            0x00000B58
#define WED_RTQM_IGRS1_FDROP_CNT                               0x00000B5C
#define WED_RTQM_IGRS1_DBG                                     0x00000B60
#define WED_RTQM_IGRS2_CFG0                                    0x00000B64
#define WED_RTQM_IGRS2_CFG1                                    0x00000B68
#define WED_RTQM_IGRS2_I2HW_DMAD_CNT                           0x00000B6C
#define WED_RTQM_IGRS2_I2H0_DMAD_CNT                           0x00000B70
#define WED_RTQM_IGRS2_I2H1_DMAD_CNT                           0x00000B74
#define WED_RTQM_IGRS2_I2HW_PKT_CNT                            0x00000B78
#define WED_RTQM_IGRS2_I2H0_PKT_CNT                            0x00000B7C
#define WED_RTQM_IGRS2_I2H1_PKT_CNT                            0x00000B80
#define WED_RTQM_IGRS2_FDROP_CNT                               0x00000B84
#define WED_RTQM_IGRS2_DBG                                     0x00000B88
#define WED_RTQM_IGRS3_CFG0                                    0x00000B8C
#define WED_RTQM_IGRS3_CFG1                                    0x00000B90
#define WED_RTQM_IGRS3_I2HW_DMAD_CNT                           0x00000B94
#define WED_RTQM_IGRS3_I2H0_DMAD_CNT                           0x00000B98
#define WED_RTQM_IGRS3_I2H1_DMAD_CNT                           0x00000B9C
#define WED_RTQM_IGRS3_I2HW_PKT_CNT                            0x00000BA0
#define WED_RTQM_IGRS3_I2H0_PKT_CNT                            0x00000BA4
#define WED_RTQM_IGRS3_I2H1_PKT_CNT                            0x00000BA8
#define WED_RTQM_IGRS3_FDROP_CNT                               0x00000BAC
#define WED_RTQM_IGRS3_DBG                                     0x00000BB0
#define WED_RTQM_EGRS_WEIGHT                                   0x00000BB4
#define WED_RTQM_ENQ_CFG0                                      0x00000BB8
#define WED_RTQM_ENQ_I2Q_DMAD_CNT                              0x00000BBC
#define WED_RTQM_ENQ_I2N_DMAD_CNT                              0x00000BC0
#define WED_RTQM_ENQ_I2Q_PKT_CNT                               0x00000BC4
#define WED_RTQM_ENQ_I2N_PKT_CNT                               0x00000BC8
#define WED_RTQM_ENQ_USED_ENTRY_CNT                            0x00000BCC
#define WED_RTQM_ENQ_ERR_CNT                                   0x00000BD0
#define WED_RTQM_DEQ_CFG0                                      0x00000BD4
#define WED_RTQM_DEQ_DQ_DMAD_CNT                               0x00000BD8
#define WED_RTQM_DEQ_Q2I_DMAD_CNT                              0x00000BDC
#define WED_RTQM_DEQ_DQ_PKT_CNT                                0x00000BE0
#define WED_RTQM_DEQ_Q2I_PKT_CNT                               0x00000BE4
#define WED_RTQM_DEQ_USED_PFDBK_CNT                            0x00000BE8
#define WED_RTQM_DEQ_ERR_CNT                                   0x00000BEC
#define WED_RTQM_DEQ_DBG                                       0x00000BF0
#define WED_RTQM_ERR_STS                                       0x00000BF4
#define WED_RTQM_QUEUE_CFG0                                    0x00000BF8
#define WED_RTQM_QUEUE_ACC_CFG                                 0x00000BFC
#define WED_RROQM_GLO_CFG                                      0x00000C04
#define WED_RROQM_RST_IDX                                      0x00000C08
#define WED_RROQM_DLY_INT                                      0x00000C0C
#define WED_RTQM_QUEUE_ACC_WR0                                 0x00000C10
#define WED_RTQM_QUEUE_ACC_WR1                                 0x00000C14
#define WED_RTQM_QUEUE_ACC_WR2                                 0x00000C18
#define WED_RTQM_QUEUE_ACC_WR3                                 0x00000C1C
#define WED_RTQM_QUEUE_ACC_WR4                                 0x00000C20
#define WED_RTQM_QUEUE_ACC_RD0                                 0x00000C24
#define WED_RTQM_QUEUE_ACC_RD1                                 0x00000C28
#define WED_RTQM_QUEUE_ACC_RD2                                 0x00000C2C
#define WED_RTQM_QUEUE_ACC_RD3                                 0x00000C30
#define WED_RTQM_QUEUE_ACC_RD4                                 0x00000C34
#define WED_RTQM_QUEUE_CNT                                     0x00000C38
#define WED_RTQM_DBG_CFG                                       0x00000C3C
#define WED_RROQM_MIOD_CTRL0                                   0x00000C40
#define WED_RROQM_MIOD_CTRL1                                   0x00000C44
#define WED_RROQM_MIOD_CTRL2                                   0x00000C48
#define WED_RROQM_MIOD_CTRL3                                   0x00000C4C
#define WED_RROQM_FDBK_CTRL0                                   0x00000C50
#define WED_RROQM_FDBK_CTRL1                                   0x00000C54
#define WED_RROQM_FDBK_CTRL2                                   0x00000C58
#define WED_RROQM_FDBK_CTRL3                                   0x00000C5C
#define WED_RTQM_DBG_MON0                                      0x00000C60
#define WED_RTQM_DBG_MON1                                      0x00000C64
#define WED_RTQM_DBG_MON2                                      0x00000C68
#define WED_RTQM_DBG_MON3                                      0x00000C6C
#define WED_RTQM_DBG_ENQ_DATA0                                 0x00000C70
#define WED_RTQM_DBG_ENQ_DATA1                                 0x00000C74
#define WED_RTQM_DBG_ENQ_DATA2                                 0x00000C78
#define WED_RTQM_DBG_ENQ_DATA3                                 0x00000C7C
#define WED_RROQ_BASE_L                                        0x00000C80
#define WED_RROQ_BASE_H                                        0x00000C84
#define WED_RTQM_AGE_CFG0                                      0x00000C88
#define WED_RROQM_MIOD_CFG                                     0x00000C8C
#define WED_RTQM_AGE_CFG1                                      0x00000C90
#define WED_RTQM_AGE_FIFO_CFG                                  0x00000C94
#define WED_RTQM_AGE_PKT_CNT                                   0x00000C98
#define WED_RTQM_PFDBK_CNT                                     0x00000C9C
#define WED_RROQM_FDBK_REC_CTRL                                0x00000CAC
#define WED_RROQM_FDBK_REC0                                    0x00000CB0
#define WED_RROQM_FDBK_REC1                                    0x00000CB4
#define WED_RROQM_FDBK_REC2                                    0x00000CB8
#define WED_RROQM_FDBK_REC3                                    0x00000CBC
#define WED_RROQM_MID_MIB                                      0x00000CC0
#define WED_RROQM_MOD_MIB                                      0x00000CC4
#define WED_RROQM_MOD_COH_MIB                                  0x00000CC8
#define WED_RROQM_FDBK_MIB                                     0x00000CD0
#define WED_RROQM_FDBK_COH_MIB                                 0x00000CD4
#define WED_RROQM_FDBK_IND_MIB                                 0x00000CE0
#define WED_RROQM_FDBK_ENQ_MIB                                 0x00000CE4
#define WED_RROQM_FDBK_ANC_MIB                                 0x00000CE8
#define WED_RROQM_FDBK_ANC2H_MIB                               0x00000CEC
#define WED_TX_BM_DYN_ALLOC_TH                                 0x00000D00
#define WED_TX_BM_DYN_ALLOC_CFG                                0x00000D04
#define WED_TX_BM_ADD_BASE                                     0x00000D08
#define WED_TX_BM_ADD_PTR                                      0x00000D0C
#define WED_TX_BM_TOTAL_DMAD_IDX                               0x00000D10
#define WED_TX_BM_ADD_BASE_H                                   0x00000D14
#define WED_RX_BM_RX_DMAD                                      0x00000D80
#define WED_RX_BM_BASE                                         0x00000D84
#define WED_RX_BM_INIT_PTR                                     0x00000D88
#define WED_RX_BM_PTR                                          0x00000D8C
#define WED_RX_BM_BLEN                                         0x00000D90
#define WED_RX_BM_STS                                          0x00000D94
#define WED_RX_BM_INTF2                                        0x00000D98
#define WED_RX_BM_INTF                                         0x00000D9C
#define WED_RX_BM_ERR_STS                                      0x00000DA8
#define WED_RX_BM_OPT_CTRL                                     0x00000DAC
#define WED_RX_BM_DYN_ALLOC_TH                                 0x00000DB4
#define WED_RX_BM_DYN_ALLOC_CFG                                0x00000DB8
#define WED_RX_BM_ADD_BASE                                     0x00000DBC
#define WED_RX_BM_ADD_PTR                                      0x00000DC0
#define WED_RX_BM_TOTAL_DMAD_IDX                               0x00000DC4
#define WED_RX_BM_ADD_BASE_H                                   0x00000DC8
#define RRO_IND_CMD_0_SIGNATURE                                0x00000E00
#define IND_CMD_0_CTRL_0                                       0x00000E04
#define IND_CMD_0_CTRL_1                                       0x00000E08
#define IND_CMD_0_CTRL_2                                       0x00000E0C
#define RRO_CONF_0                                             0x00000E10
#define RRO_CONF_1                                             0x00000E14
#define ADDR_ELEM_CONF_0                                       0x00000E18
#define ADDR_ELEM_CONF_1                                       0x00000E1C
#define ADDR_ELEM_BASE_TBL_CONF                                0x00000E20
#define ADDR_ELEM_BASE_TBL_WDATA                               0x00000E24
#define ADDR_ELEM_BASE_TBL_RDATA                               0x00000E28
#define PN_CONF_0                                              0x00000E30
#define PN_CONF_WDATA_L                                        0x00000E34
#define PN_CONF_WDATA_M                                        0x00000E38
#define PN_CONF_RDATA_L                                        0x00000E3C
#define PN_CONF_RDATA_M                                        0x00000E40
#define RRO_MSDU_PG_RING0_CFG0                                 0x00000E44
#define RRO_MSDU_PG_RING0_CFG1                                 0x00000E48
#define RRO_MSDU_PG_RING1_CFG0                                 0x00000E4C
#define RRO_MSDU_PG_RING1_CFG1                                 0x00000E50
#define RRO_MSDU_PG_RING2_CFG0                                 0x00000E54
#define RRO_MSDU_PG_RING2_CFG1                                 0x00000E58
#define RRO_MSDU_PG_0_CTRL0                                    0x00000E5C
#define RRO_MSDU_PG_0_CTRL1                                    0x00000E60
#define RRO_MSDU_PG_0_CTRL2                                    0x00000E64
#define RRO_MSDU_PG_1_CTRL0                                    0x00000E68
#define RRO_MSDU_PG_1_CTRL1                                    0x00000E6C
#define RRO_MSDU_PG_1_CTRL2                                    0x00000E70
#define RRO_MSDU_PG_2_CTRL0                                    0x00000E74
#define RRO_MSDU_PG_2_CTRL1                                    0x00000E78
#define RRO_MSDU_PG_2_CTRL2                                    0x00000E7C
#define RRO_RX_D_RX0_BASE                                      0x00000E80
#define RRO_RX_D_RX0_CNT                                       0x00000E84
#define RRO_RX_D_RX0_CRX_IDX                                   0x00000E88
#define RRO_RX_D_RX0_DRX_IDX                                   0x00000E8C
#define RRO_RX_D_RX1_BASE                                      0x00000E90
#define RRO_RX_D_RX1_CNT                                       0x00000E94
#define RRO_RX_D_RX1_CRX_IDX                                   0x00000E98
#define RRO_RX_D_RX1_DRX_IDX                                   0x00000E9C
#define RRO_RX_D_RING_CFG_ADDR_0                               0x00000EA0
#define RRO_RX_D_RING_CFG_ADDR_1                               0x00000EA4
#define RRO_RX_D_RING_CFG_ADDR_2                               0x00000EA8
#define RRO_PG_BM_RX_DMAD                                      0x00000EB0
#define RRO_PG_BM_BASE                                         0x00000EB4
#define RRO_PG_BM_INIT_PTR                                     0x00000EB8
#define RRO_PG_BM_PTR                                          0x00000EBC
#define RRO_PG_BM_ADD_BASE_H                                   0x00000EC0
#define RRO_PG_BM_STS                                          0x00000EC4
#define RRO_PG_BM_INTF                                         0x00000ECC
#define RRO_PG_BM_ERR_STS                                      0x00000ED0
#define RRO_PG_BM_OPT_CTRL                                     0x00000ED4
#define RRO_PG_BM_DYN_ALLOC_TH                                 0x00000ED8
#define RRO_PG_BM_DYN_ALLOC_CFG                                0x00000EDC
#define RRO_PG_BM_ADD_BASE                                     0x00000EE0
#define RRO_PG_BM_ADD_PTR                                      0x00000EE4
#define RRO_PG_BM_TOTAL_DMAD_IDX                               0x00000EE8
#define WED_WPDMA_INT_CTRL_RRO_RX                              0x00000EEC
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG                         0x00000EF4
#define WED_RRO_RX_HW_STS                                      0x00000F00
#define WED_RRO_RX_DBG0                                        0x00000F04
#define WED_RRO_RX_DBG1                                        0x00000F08
#define WED_RRO_RX_DBG2                                        0x00000F0C
#define WED_RRO_RX_DBG3                                        0x00000F10
#define WED_RRO_RX_DBG4                                        0x00000F14
#define WED_RX_IND_CMD_CNT0                                    0x00000F20
#define WED_RX_IND_CMD_CNT1                                    0x00000F24
#define WED_RX_IND_CMD_CNT2                                    0x00000F28
#define WED_RX_IND_CMD_CNT3                                    0x00000F2C
#define WED_RX_IND_CMD_CNT4                                    0x00000F30
#define WED_RX_IND_CMD_CNT5                                    0x00000F34
#define WED_RX_IND_CMD_CNT6                                    0x00000F38
#define WED_RX_IND_CMD_CNT7                                    0x00000F3C
#define WED_RX_IND_CMD_CNT8                                    0x00000F40
#define WED_RX_IND_CMD_CNT9                                    0x00000F44
#define WED_RX_ADDR_ELEM_CNT0                                  0x00000F48
#define WED_RX_ADDR_ELEM_CNT1                                  0x00000F4C
#define WED_RX_ADDR_ELEM_CNT2                                  0x00000F50
#define WED_RX_ADDR_ELEM_CNT3                                  0x00000F54
#define WED_RX_ADDR_ELEM_CNT4                                  0x00000F58
#define WED_RX_MSDU_PG_CNT1                                    0x00000F5C
#define WED_RX_MSDU_PG_CNT2                                    0x00000F60
#define WED_RX_MSDU_PG_CNT3                                    0x00000F64
#define WED_RX_MSDU_PG_CNT4                                    0x00000F68
#define WED_RX_MSDU_PG_CNT5                                    0x00000F6C
#define WED_RX_PN_CHK_CNT0                                     0x00000F70
#define WED_RX_IND_CMD_AXI0                                    0x00000F74
#define WED_RX_ADDR_ELEM_AXI0                                  0x00000F78
#define WED_RX_ADDR_ELEM_AXI1                                  0x00000F7C
#define WED_RX_ADDR_ELEM_AXI2                                  0x00000F80
#define WED_RX_ADDR_ELEM_AXI3                                  0x00000F84
#define WED_RX_ADDR_ELEM_AXI4                                  0x00000F88
#define WED_RX_MSDU_PG_AXI0                                    0x00000F8C
#define DONT_USE_BEHIND_THIS_ADDR                              0x00000FBC

#endif


#define WED_ID_FLD_MODE                                        REG_FLD(32, 0)

#define WED_REV_ID_FLD_MAJOR                                   REG_FLD(4, 28)
#define WED_REV_ID_FLD_MINOR                                   REG_FLD(12, 16)
#define WED_REV_ID_FLD_BRANCH                                  REG_FLD(8, 8)
#define WED_REV_ID_FLD_ECO                                     REG_FLD(8, 0)

#define WED_MOD_RST_FLD_WED                                    REG_FLD(1, 31)
#define WED_MOD_RST_FLD_TX_PAO                                 REG_FLD(1, 22)
#define WED_MOD_RST_FLD_RX_ROUTE_QM                            REG_FLD(1, 21)
#define WED_MOD_RST_FLD_RX_RRO_QM                              REG_FLD(1, 20)
#define WED_MOD_RST_FLD_WDMA_INT_AGT                           REG_FLD(1, 19)
#define WED_MOD_RST_FLD_WDMA_RX_DRV                            REG_FLD(1, 17)
#define WED_MOD_RST_FLD_WDMA_TX_DRV                            REG_FLD(1, 16)
#define WED_MOD_RST_FLD_WED_RX_DMA                             REG_FLD(1, 13)
#define WED_MOD_RST_FLD_WED_TX_DMA                             REG_FLD(1, 12)
#define WED_MOD_RST_FLD_WPDMA_INT_AGT                          REG_FLD(1, 11)
#define WED_MOD_RST_FLD_WPDMA_RX_D_DRV                         REG_FLD(1, 10)
#define WED_MOD_RST_FLD_WPDMA_RX_DRV                           REG_FLD(1, 9)
#define WED_MOD_RST_FLD_WPDMA_TX_DRV                           REG_FLD(1, 8)
#define WED_MOD_RST_FLD_TX_TKID_ALI                            REG_FLD(1, 5)
#define WED_MOD_RST_FLD_TX_FREE_AGT                            REG_FLD(1, 4)
#define WED_MOD_RST_FLD_RRO_RX_TO_PG                           REG_FLD(1, 3)
#define WED_MOD_RST_FLD_RX_PG_BM                               REG_FLD(1, 2)
#define WED_MOD_RST_FLD_RX_BM                                  REG_FLD(1, 1)
#define WED_MOD_RST_FLD_TX_BM                                  REG_FLD(1, 0)

#define WED_CTRL_FLD_MIB_RD_CLR                                REG_FLD(1, 28)
#define WED_CTRL_FLD_ETH_DMAD_FMT                              REG_FLD(1, 25)
#define WED_CTRL_FLD_FINAL_DIDX_RD                             REG_FLD(1, 24)
#define WED_CTRL_FLD_WED_TX_PAO_BUSY                           REG_FLD(1, 23)
#define WED_CTRL_FLD_WED_TX_PAO_EN                             REG_FLD(1, 22)
#define WED_CTRL_FLD_WED_TX_TKID_ALI_BUSY                      REG_FLD(1, 21)
#define WED_CTRL_FLD_WED_TX_TKID_ALI_EN                        REG_FLD(1, 20)
#define WED_CTRL_FLD_WED_TX_TKID_BUSY                          REG_FLD(1, 18)
#define WED_CTRL_FLD_WED_RX_ROUTE_QM_BUSY                      REG_FLD(1, 17)
#define WED_CTRL_FLD_WED_RX_ROUTE_QM_EN                        REG_FLD(1, 16)
#define WED_CTRL_FLD_WED_RX_RRO_QM_BUSY                        REG_FLD(1, 15)
#define WED_CTRL_FLD_WED_RX_RRO_QM_EN                          REG_FLD(1, 14)
#define WED_CTRL_FLD_WED_RX_BM_BUSY                            REG_FLD(1, 13)
#define WED_CTRL_FLD_WED_RX_BM_EN                              REG_FLD(1, 12)
#define WED_CTRL_FLD_WED_TX_FREE_AGT_BUSY                      REG_FLD(1, 11)
#define WED_CTRL_FLD_WED_TX_FREE_AGT_EN                        REG_FLD(1, 10)
#define WED_CTRL_FLD_WED_TX_BM_BUSY                            REG_FLD(1, 9)
#define WED_CTRL_FLD_WED_TX_BM_EN                              REG_FLD(1, 8)
#define WED_CTRL_FLD_WED_RX_PG_BM_BUSY                         REG_FLD(1, 7)
#define WED_CTRL_FLD_WED_RX_PG_BM_EN                           REG_FLD(1, 6)
#define WED_CTRL_FLD_WED_RX_IND_CMD_EN                         REG_FLD(1, 5)
#define WED_CTRL_FLD_WDMA_INT_AGT_BUSY                         REG_FLD(1, 3)
#define WED_CTRL_FLD_WDMA_INT_AGT_EN                           REG_FLD(1, 2)
#define WED_CTRL_FLD_WPDMA_INT_AGT_BUSY                        REG_FLD(1, 1)
#define WED_CTRL_FLD_WPDMA_INT_AGT_EN                          REG_FLD(1, 0)

#define WED_AXI_CTRL_FLD_MI3_CTRL_UPD                          REG_FLD(1, 31)
#define WED_AXI_CTRL_FLD_MI3_LOCK_ERR_FLAG                     REG_FLD(1, 30)
#define WED_AXI_CTRL_FLD_MI3_OUTSTD_EXTEND_EN                  REG_FLD(1, 29)
#define WED_AXI_CTRL_FLD_MI3_QOS_ON                            REG_FLD(1, 28)
#define WED_AXI_CTRL_FLD_MI2_CTRL_UPD                          REG_FLD(1, 27)
#define WED_AXI_CTRL_FLD_MI2_LOCK_ERR_FLAG                     REG_FLD(1, 26)
#define WED_AXI_CTRL_FLD_MI2_OUTSTD_EXTEND_EN                  REG_FLD(1, 25)
#define WED_AXI_CTRL_FLD_MI2_QOS_ON                            REG_FLD(1, 24)
#define WED_AXI_CTRL_FLD_MI1_CTRL_UPD                          REG_FLD(1, 23)
#define WED_AXI_CTRL_FLD_MI1_LOCK_ERR_FLAG                     REG_FLD(1, 22)
#define WED_AXI_CTRL_FLD_MI1_OUTSTD_EXTEND_EN                  REG_FLD(1, 21)
#define WED_AXI_CTRL_FLD_MI1_QOS_ON                            REG_FLD(1, 20)
#define WED_AXI_CTRL_FLD_MI0_CTRL_UPD                          REG_FLD(1, 19)
#define WED_AXI_CTRL_FLD_MI0_LOCK_ERR_FLAG                     REG_FLD(1, 18)
#define WED_AXI_CTRL_FLD_MI0_OUTSTD_EXTEND_EN                  REG_FLD(1, 17)
#define WED_AXI_CTRL_FLD_MI0_QOS_ON                            REG_FLD(1, 16)
#define WED_AXI_CTRL_FLD_SL0_ARTHRES                           REG_FLD(2, 10)
#define WED_AXI_CTRL_FLD_SL0_AWTHRES                           REG_FLD(2, 8)
#define WED_AXI_CTRL_FLD_CSR_ERRMID_SET_BIRQ                   REG_FLD(1, 4)
#define WED_AXI_CTRL_FLD_CSR_ERRMID_SET_RIRQ                   REG_FLD(1, 3)
#define WED_AXI_CTRL_FLD_R_BUSY                                REG_FLD(1, 2)
#define WED_AXI_CTRL_FLD_W_BUSY                                REG_FLD(1, 1)
#define WED_AXI_CTRL_FLD_CSR_CG_DISABLE                        REG_FLD(1, 0)

#define WED_CTRL2_FLD_GLO_ARULTRA_FORCE                        REG_FLD(1, 31)
#define WED_CTRL2_FLD_GLO_ARULTRA                              REG_FLD(2, 28)
#define WED_CTRL2_FLD_GLO_AWULTRA_FORCE                        REG_FLD(1, 27)
#define WED_CTRL2_FLD_GLO_AWULTRA                              REG_FLD(2, 24)
#define WED_CTRL2_FLD_COHERENT_WAIT_MAXIMUM                    REG_FLD(8, 0)

#define WED_EX_INT_STA_FLD_FDBK_LEAVE_FULL                     REG_FLD(1, 31)
#define WED_EX_INT_STA_FLD_ERR_MON                             REG_FLD(1, 30)
#define WED_EX_INT_STA_FLD_BUS_MON                             REG_FLD(1, 28)
#define WED_EX_INT_STA_FLD_MID_RDY                             REG_FLD(1, 27)
#define WED_EX_INT_STA_FLD_RX_DRV_DMAD_RECYCLE                 REG_FLD(1, 24)
#define WED_EX_INT_STA_FLD_RX_DRV_COHERENT                     REG_FLD(1, 18)
#define WED_EX_INT_STA_FLD_RX_DIDX_FIN1                        REG_FLD(1, 17)
#define WED_EX_INT_STA_FLD_RX_DIDX_FIN0                        REG_FLD(1, 16)
#define WED_EX_INT_STA_FLD_RX_BM_H_BUF                         REG_FLD(1, 15)
#define WED_EX_INT_STA_FLD_RX_BM_L_BUF                         REG_FLD(1, 14)
#define WED_EX_INT_STA_FLD_RX_PG_BM_H_BUF                      REG_FLD(1, 13)
#define WED_EX_INT_STA_FLD_RX_PG_BM_L_BUF                      REG_FLD(1, 12)
#define WED_EX_INT_STA_FLD_TX_TKID_HTH                         REG_FLD(1, 11)
#define WED_EX_INT_STA_FLD_TX_TKID_LTH                         REG_FLD(1, 10)
#define WED_EX_INT_STA_FLD_TX_BM_HTH                           REG_FLD(1, 9)
#define WED_EX_INT_STA_FLD_TX_BM_LTH                           REG_FLD(1, 8)
#define WED_EX_INT_STA_FLD_TX_DIDX_FIN1                        REG_FLD(1, 3)
#define WED_EX_INT_STA_FLD_TX_DIDX_FIN0                        REG_FLD(1, 2)

#define WED_EX_INT_MSK_FLD_EN                                  REG_FLD(32, 0)

#define WED_EX_INT_MSK1_FLD_EN                                 REG_FLD(32, 0)

#define WED_EX_INT_MSK2_FLD_EN                                 REG_FLD(32, 0)

#define WED_EX_INT_MSK3_FLD_EN                                 REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK0_FLD_EVENT                     REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK1_FLD_EVENT                     REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK2_FLD_EVENT                     REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK3_FLD_EVENT                     REG_FLD(32, 0)

#define WED_IRQ_MON_FLD_WED_IRQ                                REG_FLD(4, 28)
#define WED_IRQ_MON_FLD_CONNSYS_IRQ                            REG_FLD(3, 16)
#define WED_IRQ_MON_FLD_WDMA_IRQ                               REG_FLD(3, 8)
#define WED_IRQ_MON_FLD_PCIE_MSI_IRQ                           REG_FLD(3, 4)
#define WED_IRQ_MON_FLD_PCIE_IRQ                               REG_FLD(3, 0)

#define WED_ERR_MON_FLD_MID_TAKE_IDX_ERR                       REG_FLD(1, 30)
#define WED_ERR_MON_FLD_RX_ROUTE_QM_ERR                        REG_FLD(1, 29)
#define WED_ERR_MON_FLD_WPDMA_RX_D_DRV_ERR                     REG_FLD(1, 26)
#define WED_ERR_MON_FLD_RX_DRV_GET_BM_DMAD_SKIP                REG_FLD(1, 25)
#define WED_ERR_MON_FLD_TX_DMA_W_RESP_ERR                      REG_FLD(1, 23)
#define WED_ERR_MON_FLD_TX_DMA_R_RESP_ERR                      REG_FLD(1, 22)
#define WED_ERR_MON_FLD_RX_DRV_BM_DMAD_ERROR                   REG_FLD(1, 21)
#define WED_ERR_MON_FLD_RX_DRV_BM_DMAD_COHERENT                REG_FLD(1, 20)
#define WED_ERR_MON_FLD_RX_DRV_INTI_WDMA_ENABLED               REG_FLD(1, 19)
#define WED_ERR_MON_FLD_RX_DRV_W_RESP_ERR                      REG_FLD(1, 17)
#define WED_ERR_MON_FLD_RX_DRV_R_RESP_ERR                      REG_FLD(1, 16)
#define WED_ERR_MON_FLD_RX_BM_DMAD_RD_ERR                      REG_FLD(1, 13)
#define WED_ERR_MON_FLD_RX_BM_FREE_AT_EMPTY                    REG_FLD(1, 12)
#define WED_ERR_MON_FLD_TX_BM_DMAD_RD_ERR                      REG_FLD(1, 11)
#define WED_ERR_MON_FLD_TX_BM_FREE_AT_EMPTY                    REG_FLD(1, 10)
#define WED_ERR_MON_FLD_TX_PAO_ERR                             REG_FLD(1, 7)
#define WED_ERR_MON_FLD_TF_TKID_FIFO_INVLD                     REG_FLD(1, 4)
#define WED_ERR_MON_FLD_TX_TKID_FREE_ALREADY_FREE              REG_FLD(1, 2)
#define WED_ERR_MON_FLD_TF_TKID_WO_PYLD                        REG_FLD(1, 1)
#define WED_ERR_MON_FLD_TF_LEN_ERR                             REG_FLD(1, 0)

#define WED_ST_FLD_TX_ST                                       REG_FLD(8, 8)
#define WED_ST_FLD_RX_ST                                       REG_FLD(8, 0)

#define WED_WPDMA_ST_FLD_INT_AGT_ST                            REG_FLD(8, 16)
#define WED_WPDMA_ST_FLD_TX_DRV_ST                             REG_FLD(8, 8)
#define WED_WPDMA_ST_FLD_RX_DRV_ST                             REG_FLD(8, 0)

#define WED_WDMA_ST_FLD_RX_PREF_DMAD_ST                        REG_FLD(3, 26)
#define WED_WDMA_ST_FLD_AXI_W_AFTER_AW_ST                      REG_FLD(2, 24)
#define WED_WDMA_ST_FLD_INT_AGT_ST                             REG_FLD(8, 16)
#define WED_WDMA_ST_FLD_TX_DRV_ST                              REG_FLD(8, 8)
#define WED_WDMA_ST_FLD_RX_DRV_ST                              REG_FLD(8, 0)

#define WED_BM_ST_FLD_FREE_AGT_ST                              REG_FLD(8, 24)
#define WED_BM_ST_FLD_TX_BM_ST                                 REG_FLD(8, 8)
#define WED_BM_ST_FLD_RX_BM_ST                                 REG_FLD(8, 0)

#define WED_QM_ST_FLD_RROQM_ST                                 REG_FLD(8, 0)

#define WED_WPDMA_D_ST_FLD_PREF_DMAD_ST                        REG_FLD(3, 12)
#define WED_WPDMA_D_ST_FLD_AXI_W_AFTER_AW_ST                   REG_FLD(2, 8)
#define WED_WPDMA_D_ST_FLD_RX_DRV_ST                           REG_FLD(8, 0)

#define WED_TX_BM_CTRL_FLD_SCAN                                REG_FLD(1, 29)
#define WED_TX_BM_CTRL_FLD_PAUSE                               REG_FLD(1, 28)
#define WED_TX_BM_CTRL_FLD_LEGACY_EN                           REG_FLD(1, 26)
#define WED_TX_BM_CTRL_FLD_FREE_AXI_RESP_SEL                   REG_FLD(1, 25)
#define WED_TX_BM_CTRL_FLD_RSV_GRP_NUM                         REG_FLD(9, 16)
#define WED_TX_BM_CTRL_FLD_VLD_GRP_NUM                         REG_FLD(9, 0)

#define WED_TX_BM_BASE_FLD_PTR                                 REG_FLD(32, 0)

#define WED_TX_BM_INIT_PTR_FLD_INIT_SW_TAIL_IDX                REG_FLD(1, 16)
#define WED_TX_BM_INIT_PTR_FLD_SW_TAIL_IDX                     REG_FLD(16, 0)

#define WED_TX_BM_BLEN_FLD_BASE_PTR_H                          REG_FLD(8, 16)
#define WED_TX_BM_BLEN_FLD_BYTE_LEN                            REG_FLD(14, 0)

#define WED_TX_BM_STS_FLD_DMAD_RD                              REG_FLD(1, 25)
#define WED_TX_BM_STS_FLD_DMAD_VLD                             REG_FLD(1, 24)
#define WED_TX_BM_STS_FLD_DMAD_NUM                             REG_FLD(5, 16)
#define WED_TX_BM_STS_FLD_FREE_NUM                             REG_FLD(16, 0)

#define WED_TX_BM_PTR_FLD_HEAD_IDX                             REG_FLD(16, 16)
#define WED_TX_BM_PTR_FLD_TAIL_IDX                             REG_FLD(16, 0)

#define WED_TX_BM_INTF_FLD_FREE_SKBID_RD                       REG_FLD(1, 29)
#define WED_TX_BM_INTF_FLD_FREE_SKBID_VLD                      REG_FLD(1, 28)
#define WED_TX_BM_INTF_FLD_FREE_SKBID                          REG_FLD(16, 0)

#define WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM                        REG_FLD(9, 16)
#define WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM                       REG_FLD(9, 0)

#define WED_TX_BM_ERR_STS_FLD_FREE_AT_EMPTY                    REG_FLD(1, 7)
#define WED_TX_BM_ERR_STS_FLD_DMAD_AXI_RD_OVER_TAIL            REG_FLD(1, 3)
#define WED_TX_BM_ERR_STS_FLD_DMAD_FULL_AXI_RD                 REG_FLD(1, 2)
#define WED_TX_BM_ERR_STS_FLD_DMAD_AXI_RD_MORE                 REG_FLD(1, 1)
#define WED_TX_BM_ERR_STS_FLD_DMAD_AXI_RD_LESS                 REG_FLD(1, 0)

#define WED_TX_BM_RECYC_FLD_START_GRP_IDX                      REG_FLD(9, 0)

#define WED_TX_BM_MON_CTRL_FLD_VB_PGSEL                        REG_FLD(2, 0)

#define WED_TX_BM_VB_FREE_0_31_FLD_GRP                         REG_FLD(32, 0)

#define WED_TX_BM_VB_FREE_32_63_FLD_GRP                        REG_FLD(32, 0)

#define WED_TX_BM_VB_USED_0_31_FLD_GRP                         REG_FLD(32, 0)

#define WED_TX_BM_VB_USED_32_63_FLD_GRP                        REG_FLD(32, 0)

#define WED_TX_TKID_CTRL_FLD_SCAN                              REG_FLD(1, 29)
#define WED_TX_TKID_CTRL_FLD_PAUSE                             REG_FLD(1, 28)
#define WED_TX_TKID_CTRL_FLD_FREE_FORMAT                       REG_FLD(1, 27)
#define WED_TX_TKID_CTRL_FLD_FREE_WAIT                         REG_FLD(1, 26)
#define WED_TX_TKID_CTRL_FLD_FORCE_DRAM                        REG_FLD(1, 25)
#define WED_TX_TKID_CTRL_FLD_RSV_GRP_NUM                       REG_FLD(8, 16)
#define WED_TX_TKID_CTRL_FLD_VB_PGSEL                          REG_FLD(1, 12)
#define WED_TX_TKID_CTRL_FLD_VLD_GRP_NUM                       REG_FLD(8, 0)

#define WED_TX_TKID_SPR_FLD_RSV_REG                            REG_FLD(32, 0)

#define WED_TX_TKID_TKID_FLD_END_ID                            REG_FLD(16, 16)
#define WED_TX_TKID_TKID_FLD_START_ID                          REG_FLD(16, 0)

#define WED_TX_TKID_INTF_FLD_FREE_TKID_RD                      REG_FLD(1, 29)
#define WED_TX_TKID_INTF_FLD_FREE_TKID_VLD                     REG_FLD(1, 28)
#define WED_TX_TKID_INTF_FLD_FREE_TKFIFO_FDEP                  REG_FLD(10, 16)
#define WED_TX_TKID_INTF_FLD_FREE_TKID                         REG_FLD(16, 0)

#define WED_TX_TKID_DYN_TH_FLD_HI_GRP_NUM                      REG_FLD(8, 16)
#define WED_TX_TKID_DYN_TH_FLD_LOW_GRP_NUM                     REG_FLD(8, 0)

#define WED_TX_TKID_STS_FLD_FSM                                REG_FLD(12, 16)
#define WED_TX_TKID_STS_FLD_FREE_NUM                           REG_FLD(16, 0)

#define WED_TX_TKID_RECYC_FLD_START_GRP_IDX                    REG_FLD(8, 0)

#define WED_TX_TKID_ALI_FLD_FIFO_RD_ERR                        REG_FLD(1, 29)
#define WED_TX_TKID_ALI_FLD_FIFO_WR_ERR                        REG_FLD(1, 28)
#define WED_TX_TKID_ALI_FLD_FIFO_CNT                           REG_FLD(12, 16)
#define WED_TX_TKID_ALI_FLD_FIFO_FULL                          REG_FLD(4, 8)
#define WED_TX_TKID_ALI_FLD_FIFO_EMPTY                         REG_FLD(4, 4)
#define WED_TX_TKID_ALI_FLD_FIFO_RST                           REG_FLD(1, 1)

#define WED_TX_TKID_VB_FREE_0_31_FLD_GRP                       REG_FLD(32, 0)

#define WED_TX_TKID_VB_FREE_32_63_FLD_GRP                      REG_FLD(32, 0)

#define WED_TX_TKID_VB_USED_0_31_FLD_GRP                       REG_FLD(32, 0)

#define WED_TX_TKID_VB_USED_32_63_FLD_GRP                      REG_FLD(32, 0)

#define WED_TXD_DW0_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW1_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW2_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW3_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW4_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW5_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW6_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW7_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXP_DW0_FLD_VALUE                                  REG_FLD(16, 0)

#define WED_TXP_DW1_FLD_VALUE                                  REG_FLD(16, 16)

#define WED_TXDP_CTRL_FLD_TXD_ADDR_SEL                         REG_FLD(1, 11)
#define WED_TXDP_CTRL_FLD_WCID_BIT                             REG_FLD(1, 10)
#define WED_TXDP_CTRL_FLD_DW_OVERWR                            REG_FLD(10, 0)

#define WED_DBG_CTRL_FLD_TX_TKID_BM_MEM_DBG                    REG_FLD(1, 28)
#define WED_DBG_CTRL_FLD_TX_BM_MEM_PGSEL                       REG_FLD(2, 26)
#define WED_DBG_CTRL_FLD_BUS_MON_AR_EN                         REG_FLD(1, 25)
#define WED_DBG_CTRL_FLD_BUS_MON_AW_EN                         REG_FLD(1, 24)
#define WED_DBG_CTRL_FLD_PRB_DW_SEL_TO_CHIP                    REG_FLD(2, 8)
#define WED_DBG_CTRL_FLD_PRB_SEL                               REG_FLD(6, 0)

#define WED_DBG_BUS_ADDR_L_FLD_ADDRESS                         REG_FLD(32, 0)

#define WED_DBG_BUS_ADDR_H_FLD_ADDRESS                         REG_FLD(8, 0)

#define WED_DBG_BUS_MON_FLD_STATUS                             REG_FLD(32, 0)

#define WED_DBG_PRB0_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_DBG_PRB1_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_DBG_PRB2_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_DBG_PRB3_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_TX_TKID_FREE_TKID_IN_FLD_CNT                       REG_FLD(32, 0)

#define WED_TX_TKID_FREE_TKID_LDDRAM_FLD_CNT                   REG_FLD(32, 0)

#define WED_TX_TKID_FREE_TKID_LDSRAM_FLD_CNT                   REG_FLD(32, 0)

#define WED_TX_TKID_FREE_TKID_RECYCLE_FLD_CNT                  REG_FLD(32, 0)

#define WED_TX_FREE_TO_TX_TKID_TKID_MIB_FLD_CNT                REG_FLD(32, 0)

#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_FLD_CNT             REG_FLD(32, 0)

#define WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB_FLD_CNT             REG_FLD(32, 0)

#define WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB_FLD_CNT           REG_FLD(32, 0)

#define WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB_FLD_CNT            REG_FLD(32, 0)

#define WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB_FLD_CNT             REG_FLD(32, 0)

#define WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB_FLD_CNT          REG_FLD(32, 0)

#define WED_PMTR_CTRL_FLD_EN                                   REG_FLD(1, 31)
#define WED_PMTR_CTRL_FLD_ACC_DIV                              REG_FLD(4, 24)
#define WED_PMTR_CTRL_FLD_MAX_DIV                              REG_FLD(4, 16)

#define WED_PMTR_TGT_FLD_MST3                                  REG_FLD(5, 24)
#define WED_PMTR_TGT_FLD_MST2                                  REG_FLD(5, 16)
#define WED_PMTR_TGT_FLD_MST1                                  REG_FLD(5, 8)
#define WED_PMTR_TGT_FLD_MST0                                  REG_FLD(5, 0)

#define WED_PMTR_TGT_ST_FLD_MST3                               REG_FLD(8, 24)
#define WED_PMTR_TGT_ST_FLD_MST2                               REG_FLD(8, 16)
#define WED_PMTR_TGT_ST_FLD_MST1                               REG_FLD(8, 8)
#define WED_PMTR_TGT_ST_FLD_MST0                               REG_FLD(8, 0)

#define WED_PMTR_LTCY_MAX0_1_FLD_MST1_CNT                      REG_FLD(16, 16)
#define WED_PMTR_LTCY_MAX0_1_FLD_MST0_CNT                      REG_FLD(16, 0)

#define WED_PMTR_LTCY_MAX2_3_FLD_MST3_CNT                      REG_FLD(16, 16)
#define WED_PMTR_LTCY_MAX2_3_FLD_MST2_CNT                      REG_FLD(16, 0)

#define WED_PMTR_LTCY_ACC0_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC1_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC2_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC3_FLD_CNT                             REG_FLD(32, 0)

#define WED_INT_STA_FLD_EVENT_31                               REG_FLD(1, 31)
#define WED_INT_STA_FLD_EVENT_30                               REG_FLD(1, 30)
#define WED_INT_STA_FLD_EVENT_29                               REG_FLD(1, 29)
#define WED_INT_STA_FLD_EVENT_28                               REG_FLD(1, 28)
#define WED_INT_STA_FLD_EVENT_27                               REG_FLD(1, 27)
#define WED_INT_STA_FLD_EVENT_26                               REG_FLD(1, 26)
#define WED_INT_STA_FLD_EVENT_25                               REG_FLD(1, 25)
#define WED_INT_STA_FLD_EVENT_24                               REG_FLD(1, 24)
#define WED_INT_STA_FLD_EVENT_23                               REG_FLD(1, 23)
#define WED_INT_STA_FLD_EVENT_22                               REG_FLD(1, 22)
#define WED_INT_STA_FLD_EVENT_21                               REG_FLD(1, 21)
#define WED_INT_STA_FLD_EVENT_20                               REG_FLD(1, 20)
#define WED_INT_STA_FLD_EVENT_19                               REG_FLD(1, 19)
#define WED_INT_STA_FLD_EVENT_18                               REG_FLD(1, 18)
#define WED_INT_STA_FLD_EVENT_17                               REG_FLD(1, 17)
#define WED_INT_STA_FLD_EVENT_16                               REG_FLD(1, 16)
#define WED_INT_STA_FLD_EVENT_15                               REG_FLD(1, 15)
#define WED_INT_STA_FLD_EVENT_14                               REG_FLD(1, 14)
#define WED_INT_STA_FLD_EVENT_13                               REG_FLD(1, 13)
#define WED_INT_STA_FLD_EVENT_12                               REG_FLD(1, 12)
#define WED_INT_STA_FLD_EVENT_11                               REG_FLD(1, 11)
#define WED_INT_STA_FLD_EVENT_10                               REG_FLD(1, 10)
#define WED_INT_STA_FLD_EVENT_9                                REG_FLD(1, 9)
#define WED_INT_STA_FLD_EVENT_8                                REG_FLD(1, 8)
#define WED_INT_STA_FLD_EVENT_7                                REG_FLD(1, 7)
#define WED_INT_STA_FLD_EVENT_6                                REG_FLD(1, 6)
#define WED_INT_STA_FLD_EVENT_5                                REG_FLD(1, 5)
#define WED_INT_STA_FLD_EVENT_4                                REG_FLD(1, 4)
#define WED_INT_STA_FLD_EVENT_3                                REG_FLD(1, 3)
#define WED_INT_STA_FLD_EVENT_2                                REG_FLD(1, 2)
#define WED_INT_STA_FLD_EVENT_1                                REG_FLD(1, 1)
#define WED_INT_STA_FLD_EVENT_0                                REG_FLD(1, 0)

#define WED_INT_MSK_FLD_EN                                     REG_FLD(32, 0)

#define WED_GLO_CFG_FLD_CSR_CLKGATE_BYP                        REG_FLD(1, 30)
#define WED_GLO_CFG_FLD_BYTE_SWAP                              REG_FLD(1, 29)
#define WED_GLO_CFG_FLD_RX_DMA_BURST_CFG                       REG_FLD(2, 11)
#define WED_GLO_CFG_FLD_WCOMPLETE_SEL                          REG_FLD(1, 9)
#define WED_GLO_CFG_FLD_DESC_32B_E                             REG_FLD(1, 8)
#define WED_GLO_CFG_FLD_BIG_ENDIAN                             REG_FLD(1, 7)
#define WED_GLO_CFG_FLD_TX_WB_DDONE                            REG_FLD(1, 6)
#define WED_GLO_CFG_FLD_RX_DMA_BUSY                            REG_FLD(1, 3)
#define WED_GLO_CFG_FLD_RX_DMA_EN                              REG_FLD(1, 2)
#define WED_GLO_CFG_FLD_TX_DMA_BUSY                            REG_FLD(1, 1)
#define WED_GLO_CFG_FLD_TX_DMA_EN                              REG_FLD(1, 0)

#define WED_RST_IDX_FLD_WPDMA_DRX_IDX1                         REG_FLD(1, 31)
#define WED_RST_IDX_FLD_WPDMA_DRX_IDX0                         REG_FLD(1, 30)
#define WED_RST_IDX_FLD_DRX_IDX1                               REG_FLD(1, 7)
#define WED_RST_IDX_FLD_DRX_IDX0                               REG_FLD(1, 6)
#define WED_RST_IDX_FLD_DTX_IDX1                               REG_FLD(1, 1)
#define WED_RST_IDX_FLD_DTX_IDX0                               REG_FLD(1, 0)

#define WED_DLY_INT_CFG_FLD_TX_DLY_INT_EN                      REG_FLD(1, 31)
#define WED_DLY_INT_CFG_FLD_TX_MAX_PINT                        REG_FLD(7, 24)
#define WED_DLY_INT_CFG_FLD_TX_MAX_PTIME                       REG_FLD(8, 16)
#define WED_DLY_INT_CFG_FLD_RX_DLY_INT_EN                      REG_FLD(1, 15)
#define WED_DLY_INT_CFG_FLD_RX_MAX_PINT                        REG_FLD(7, 8)
#define WED_DLY_INT_CFG_FLD_RX_MAX_PTIME                       REG_FLD(8, 0)

#define WED_DLY_INT_CFG1_FLD_TX_FREE_DLY_INT_EN                REG_FLD(1, 15)
#define WED_DLY_INT_CFG1_FLD_TX_FREE_MAX_PINT                  REG_FLD(7, 8)
#define WED_DLY_INT_CFG1_FLD_TX_FREE_MAX_PTIME                 REG_FLD(8, 0)

#define WED_SPR_FLD_RSV_REG                                    REG_FLD(32, 0)

#define WED_INT_MSK1_FLD_EN                                    REG_FLD(32, 0)

#define WED_INT_MSK2_FLD_EN                                    REG_FLD(32, 0)

#define WED_INT_MSK3_FLD_EN                                    REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK0_FLD_EVENT                        REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK1_FLD_EVENT                        REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK2_FLD_EVENT                        REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK3_FLD_EVENT                        REG_FLD(32, 0)

#define WED_SPR_0X24C_FLD_RSV_REG                              REG_FLD(32, 0)

#define WED_SPR_0X25C_FLD_RSV_REG                              REG_FLD(32, 0)

#define WED_TX0_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_TX1_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_TX_COHERENT_MIB_FLD_CNT                            REG_FLD(32, 0)

#define WED_RX0_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_RX1_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_RX_DMA_MIB_FLD_RD_CNT                              REG_FLD(16, 16)
#define WED_RX_DMA_MIB_FLD_WR_CNT                              REG_FLD(16, 0)

#define WED_TX0_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_TX0_CTRL1_FLD_BASE_PTR_H                           REG_FLD(8, 16)
#define WED_TX0_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_TX0_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_TX0_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_TX1_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_TX1_CTRL1_FLD_BASE_PTR_H                           REG_FLD(8, 16)
#define WED_TX1_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_TX1_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_TX1_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_SCR0_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR1_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR2_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR3_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR4_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR5_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR6_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR7_FLD_REG                                       REG_FLD(32, 0)

#define WED_RX0_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_RX0_CTRL1_FLD_BASE_PTR_H                           REG_FLD(8, 16)
#define WED_RX0_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_RX0_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_RX0_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_RX1_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_RX1_CTRL1_FLD_BASE_PTR_H                           REG_FLD(8, 16)
#define WED_RX1_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_RX1_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_RX1_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_RX_BASE_PTR_0_FLD_BASE_PTR                         REG_FLD(32, 0)

#define WED_RX_MAX_CNT_0_FLD_BASE_PTR_H                        REG_FLD(8, 16)
#define WED_RX_MAX_CNT_0_FLD_MAX_CNT                           REG_FLD(12, 0)

#define WED_RX_CRX_IDX_0_FLD_CRX_IDX                           REG_FLD(12, 0)

#define WED_RX_DRX_IDX_0_FLD_DRX_IDX                           REG_FLD(12, 0)

#define WED_RX_BASE_PTR_1_FLD_BASE_PTR                         REG_FLD(32, 0)

#define WED_RX_MAX_CNT_1_FLD_BASE_PTR_H                        REG_FLD(8, 16)
#define WED_RX_MAX_CNT_1_FLD_MAX_CNT                           REG_FLD(12, 0)

#define WED_RX_CRX_IDX_1_FLD_CRX_IDX                           REG_FLD(12, 0)

#define WED_RX_DRX_IDX_1_FLD_DRX_IDX                           REG_FLD(12, 0)

#define WED_WPDMA_INT_STA_REC_FLD_INT_STA                      REG_FLD(32, 0)

#define WED_WPDMA_INT_TRIG_FLD_INT_TRIG                        REG_FLD(32, 0)

#define WED_WPDMA_GLO_CFG_FLD_RX_2B_OFFSET                     REG_FLD(1, 31)
#define WED_WPDMA_GLO_CFG_FLD_TX_DDONE_CHK                     REG_FLD(1, 30)
#define WED_WPDMA_GLO_CFG_FLD_BYTE_SWAP                        REG_FLD(1, 29)
#define WED_WPDMA_GLO_CFG_FLD_TX_DMAD_DW3_ZERO                 REG_FLD(1, 28)
#define WED_WPDMA_GLO_CFG_FLD_TXD_VER                          REG_FLD(1, 26)
#define WED_WPDMA_GLO_CFG_FLD_TX_DDONE_CHK_LAST                REG_FLD(1, 25)
#define WED_WPDMA_GLO_CFG_FLD_TX_TKID_KEEP                     REG_FLD(1, 24)
#define WED_WPDMA_GLO_CFG_FLD_RSV1                             REG_FLD(2, 22)
#define WED_WPDMA_GLO_CFG_FLD_RX_DDONE2_WR                     REG_FLD(1, 21)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_EVENT_PKT_FMT_CHK         REG_FLD(1, 20)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_UNSUPPORT_FMT_BYP         REG_FLD(1, 19)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_UNS_VER_FORCE_4           REG_FLD(1, 18)
#define WED_WPDMA_GLO_CFG_FLD_RSV2                             REG_FLD(2, 16)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_EVENT_PKT_FMT_VER         REG_FLD(4, 12)
#define WED_WPDMA_GLO_CFG_FLD_RSV0                             REG_FLD(4, 8)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING1_CRX_SYNC            REG_FLD(1, 7)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_CRX_SYNC            REG_FLD(1, 6)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING1_PKT_PROC            REG_FLD(1, 5)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_PKT_PROC            REG_FLD(1, 4)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_BUSY                      REG_FLD(1, 3)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN                        REG_FLD(1, 2)
#define WED_WPDMA_GLO_CFG_FLD_TX_DRV_BUSY                      REG_FLD(1, 1)
#define WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN                        REG_FLD(1, 0)

#define WED_WPDMA_RST_IDX_FLD_CRX_IDX1                         REG_FLD(1, 17)
#define WED_WPDMA_RST_IDX_FLD_CRX_IDX0                         REG_FLD(1, 16)
#define WED_WPDMA_RST_IDX_FLD_CTX_IDX1                         REG_FLD(1, 1)
#define WED_WPDMA_RST_IDX_FLD_CTX_IDX0                         REG_FLD(1, 0)

#define WED_WPDMA_BUS_CFG_FLD_RX_RD_BND_4KB_BST                REG_FLD(1, 17)
#define WED_WPDMA_BUS_CFG_FLD_RX_WR_BND_4KB_BST                REG_FLD(1, 16)
#define WED_WPDMA_BUS_CFG_FLD_TX_RD_BND_4KB_BST                REG_FLD(1, 1)
#define WED_WPDMA_BUS_CFG_FLD_TX_WR_BND_4KB_BST                REG_FLD(1, 0)

#define WED_WPDMA_TX_DLY_CIDX_FLD_EN                           REG_FLD(1, 31)
#define WED_WPDMA_TX_DLY_CIDX_FLD_MAX_PIDX                     REG_FLD(7, 24)
#define WED_WPDMA_TX_DLY_CIDX_FLD_MAX_PTIME                    REG_FLD(8, 16)
#define WED_WPDMA_TX_DLY_CIDX_FLD_RING_LOWTH                   REG_FLD(8, 8)

#define WED_WPDMA_CTRL_FLD_SDL1_FIXED                          REG_FLD(1, 31)
#define WED_WPDMA_CTRL_FLD_SDL1_MAX                            REG_FLD(14, 0)

#define WED_WPDMA_FORCE_PROC_FLD_MSDU_PG_INT2                  REG_FLD(1, 26)
#define WED_WPDMA_FORCE_PROC_FLD_MSDU_PG_INT1                  REG_FLD(1, 25)
#define WED_WPDMA_FORCE_PROC_FLD_MSDU_PG_INT0                  REG_FLD(1, 24)
#define WED_WPDMA_FORCE_PROC_FLD_TX_FREE_DONE1                 REG_FLD(1, 17)
#define WED_WPDMA_FORCE_PROC_FLD_TX_FREE_DONE0                 REG_FLD(1, 16)
#define WED_WPDMA_FORCE_PROC_FLD_RRO_RX_INT1                   REG_FLD(1, 13)
#define WED_WPDMA_FORCE_PROC_FLD_RRO_RX_INT0                   REG_FLD(1, 12)
#define WED_WPDMA_FORCE_PROC_FLD_RX_DONE1                      REG_FLD(1, 9)
#define WED_WPDMA_FORCE_PROC_FLD_RX_DONE0                      REG_FLD(1, 8)
#define WED_WPDMA_FORCE_PROC_FLD_TX_DONE1                      REG_FLD(1, 1)
#define WED_WPDMA_FORCE_PROC_FLD_TX_DONE0                      REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_FLD_MSK_EN                          REG_FLD(1, 28)
#define WED_WPDMA_INT_CTRL_FLD_POLL_MODE                       REG_FLD(2, 24)
#define WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY                      REG_FLD(1, 23)
#define WED_WPDMA_INT_CTRL_FLD_SIG_SRC                         REG_FLD(1, 22)
#define WED_WPDMA_INT_CTRL_FLD_SUBRT_ADV                       REG_FLD(1, 21)
#define WED_WPDMA_INT_CTRL_FLD_MSK_EN_POLA                     REG_FLD(1, 20)
#define WED_WPDMA_INT_CTRL_FLD_SRC_SEL                         REG_FLD(2, 16)
#define WED_WPDMA_INT_CTRL_FLD_POLL_PRD                        REG_FLD(8, 8)
#define WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY_PRD                  REG_FLD(8, 0)

#define WED_WPDMA_INT_MSK_FLD_VALUE                            REG_FLD(32, 0)

#define WED_WPDMA_INT_CLR_FLD_INT_CLR                          REG_FLD(32, 0)

#define WED_WPDMA_INT_MON_FLD_TX_FREE_DONE_INT1                REG_FLD(1, 17)
#define WED_WPDMA_INT_MON_FLD_TX_FREE_DONE_INT0                REG_FLD(1, 16)
#define WED_WPDMA_INT_MON_FLD_MSDU_PG_INT2                     REG_FLD(1, 14)
#define WED_WPDMA_INT_MON_FLD_MSDU_PG_INT1                     REG_FLD(1, 13)
#define WED_WPDMA_INT_MON_FLD_MSDU_PG_INT0                     REG_FLD(1, 12)
#define WED_WPDMA_INT_MON_FLD_RRO_RX_INT1                      REG_FLD(1, 11)
#define WED_WPDMA_INT_MON_FLD_RRO_RX_INT0                      REG_FLD(1, 10)
#define WED_WPDMA_INT_MON_FLD_RX_DONE_INT1                     REG_FLD(1, 9)
#define WED_WPDMA_INT_MON_FLD_RX_DONE_INT0                     REG_FLD(1, 8)
#define WED_WPDMA_INT_MON_FLD_TX_DONE_INT1                     REG_FLD(1, 1)
#define WED_WPDMA_INT_MON_FLD_TX_DONE_INT0                     REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_TX_FLD_DLY_MERGE_UNP_IMM            REG_FLD(1, 31)
#define WED_WPDMA_INT_CTRL_TX_FLD_DLY_MERGE_BSEL               REG_FLD(5, 26)
#define WED_WPDMA_INT_CTRL_TX_FLD_DLY_MERGE_EN                 REG_FLD(1, 24)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_TRIG1                REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_CLR1                 REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_EN1                  REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_TRIG0                REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_CLR0                 REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_EN0                  REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_RX_FLD_DLY_MERGE_UNP_IMM            REG_FLD(1, 31)
#define WED_WPDMA_INT_CTRL_RX_FLD_DLY_MERGE_BSEL               REG_FLD(5, 26)
#define WED_WPDMA_INT_CTRL_RX_FLD_DLY_MERGE_EN                 REG_FLD(1, 24)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_TRIG1                REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_CLR1                 REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_EN1                  REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_TRIG0                REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_CLR0                 REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_EN0                  REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_TRIG1      REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_CLR1       REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_EN1        REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_TRIG0      REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_CLR0       REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_EN0        REG_FLD(1, 0)

#define WED_WPDMA_SPR_FLD_RSV_REG                              REG_FLD(32, 0)

#define WED_PCIE_MSIS_TRIG_FLD_CFG                             REG_FLD(32, 0)

#define WED_PCIE_MSIS_REC_FLD_REG                              REG_FLD(32, 0)

#define WED_PCIE_MSIS_CLR_FLD_CFG                              REG_FLD(32, 0)

#define WED_PCIE_INTS_CLR_FLD_CFG                              REG_FLD(32, 0)

#define WED_PCIE_EP_INTS_CLR_FLD_CFG                           REG_FLD(32, 0)

#define WED_PCIE_CFG_ADDR_H_FLD_EP_PTR                         REG_FLD(8, 16)
#define WED_PCIE_CFG_ADDR_H_FLD_RC_PTR                         REG_FLD(8, 0)

#define WED_PCIE_CFG_ADDR_INTS_FLD_PTR                         REG_FLD(32, 0)

#define WED_PCIE_CFG_ADDR_INTM_FLD_PTR                         REG_FLD(32, 0)

#define WED_PCIE_CFG_ADDR_MSIS_FLD_PTR                         REG_FLD(32, 0)

#define WED_PCIE_CFG_ADDR_INTS_EP_FLD_PTR                      REG_FLD(32, 0)

#define WED_PCIE_INTS_TRIG_FLD_INTS_TRIG                       REG_FLD(32, 0)

#define WED_PCIE_INTS_REC_FLD_INTS                             REG_FLD(32, 0)

#define WED_PCIE_INTM_REC_FLD_INTM                             REG_FLD(32, 0)

#define WED_PCIE_INT_CTRL_FLD_IRQ_MSI_SEL                      REG_FLD(1, 25)
#define WED_PCIE_INT_CTRL_FLD_MSI_EN                           REG_FLD(1, 24)
#define WED_PCIE_INT_CTRL_FLD_CLR_PCIE_BF_WPDMA                REG_FLD(1, 22)
#define WED_PCIE_INT_CTRL_FLD_IRQ_FILTER                       REG_FLD(1, 21)
#define WED_PCIE_INT_CTRL_FLD_MSK_EN_POLA                      REG_FLD(1, 20)
#define WED_PCIE_INT_CTRL_FLD_EXT_INCPT_DIS                    REG_FLD(1, 18)
#define WED_PCIE_INT_CTRL_FLD_SRC_SEL                          REG_FLD(2, 16)
#define WED_PCIE_INT_CTRL_FLD_POLL_MODE                        REG_FLD(2, 12)
#define WED_PCIE_INT_CTRL_FLD_POLL_PRD                         REG_FLD(8, 0)

#define WED_WPDMA_CFG_ADDR_INTS_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_INTM_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_TX_FLD_PTR                          REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_TX_FREE_FLD_PTR                     REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_INT_H_FLD_PTR                       REG_FLD(8, 0)

#define WED_WPDMA_CFG_ADDR_TX_H_FLD_PTR                        REG_FLD(8, 0)

#define WED_WPDMA_CFG_ADDR_TX_FREE_H_FLD_PTR                   REG_FLD(8, 0)

#define WED_WPDMA_TX0_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_TX1_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_TX_COHERENT_MIB_FLD_CNT                      REG_FLD(32, 0)

#define WED_WPDMA_RX0_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_RX1_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_RX_COHERENT_MIB_FLD_CNT                      REG_FLD(32, 0)

#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB_FLD_CNT                REG_FLD(32, 0)

#define WED_WPDMA_TX0_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_TX0_CTRL1_FLD_BASE_PTR_H                     REG_FLD(8, 16)
#define WED_WPDMA_TX0_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_TX0_CTRL2_FLD_CPU_IDX_INT                    REG_FLD(12, 16)
#define WED_WPDMA_TX0_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_TX0_CTRL3_FLD_RLS_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_TX0_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_TX1_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_TX1_CTRL1_FLD_BASE_PTR_H                     REG_FLD(8, 16)
#define WED_WPDMA_TX1_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_TX1_CTRL2_FLD_CPU_IDX_INT                    REG_FLD(12, 16)
#define WED_WPDMA_TX1_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_TX1_CTRL3_FLD_RLS_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_TX1_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define RRO_RX_D_DRV_MON_R0_WRAP_FLD_CNT                       REG_FLD(32, 0)

#define RRO_RX_D_DRV_MON_R1_WRAP_FLD_CNT                       REG_FLD(32, 0)

#define RRO_RX_D_DRV_MON_R2_WRAP_FLD_CNT                       REG_FLD(32, 0)

#define RRO_RX_D_DRV_MON0_FLD_DATA                             REG_FLD(32, 0)

#define RRO_RX_D_DRV_MON1_FLD_DATA                             REG_FLD(32, 0)

#define RRO_RX_D_DRV_MON2_FLD_DATA                             REG_FLD(32, 0)

#define RRO_RX_D_DRV_ERR_COMP0_FLD_DATA                        REG_FLD(32, 0)

#define RRO_RX_D_DRV_ERR_COMP1_FLD_DATA                        REG_FLD(32, 0)

#define RRO_RX_D_DRV_ERR_COMP2_FLD_DATA                        REG_FLD(32, 0)

#define RRO_RX_D_DRV_ERR_COMP3_FLD_DATA                        REG_FLD(32, 0)

#define RRO_RX_D_DRV_LEAGLE_START_FLD_WADDR                    REG_FLD(32, 0)

#define RRO_RX_D_DRV_LEAGLE_END_FLD_WADDR                      REG_FLD(32, 0)

#define RRO_RX_PG_DRV_MON_R0_WRAP_FLD_CNT                      REG_FLD(32, 0)

#define RRO_RX_PG_DRV_MON_R1_WRAP_FLD_CNT                      REG_FLD(32, 0)

#define RRO_RX_PG_DRV_MON_R2_WRAP_FLD_CNT                      REG_FLD(32, 0)

#define RRO_RX_PG_DRV_MON0_FLD_DATA                            REG_FLD(32, 0)

#define RRO_RX_PG_DRV_MON1_FLD_DATA                            REG_FLD(32, 0)

#define RRO_RX_PG_DRV_MON2_FLD_DATA                            REG_FLD(32, 0)

#define RRO_RX_PG_DRV_ERR_COMP0_FLD_DATA                       REG_FLD(32, 0)

#define RRO_RX_PG_DRV_ERR_COMP1_FLD_DATA                       REG_FLD(32, 0)

#define RRO_RX_PG_DRV_ERR_COMP2_FLD_DATA                       REG_FLD(32, 0)

#define RRO_RX_PG_DRV_ERR_COMP3_FLD_DATA                       REG_FLD(32, 0)

#define RRO_RX_PG_DRV_LEAGLE_START_FLD_WADDR                   REG_FLD(32, 0)

#define RRO_RX_PG_DRV_LEAGLE_END_FLD_WADDR                     REG_FLD(32, 0)

#define RRO_RX_D_DRV_ERR_SWITCH_ADDR_FLD_BASE                  REG_FLD(32, 0)

#define RRO_RX_PG_DRV_ERR_SWITCH_ADDR_FLD_BASE                 REG_FLD(32, 0)

#define RRO_RX_D_DRV_IN_MIB_FLD_CNT                            REG_FLD(32, 0)

#define RRO_RX_D_DRV_OU_MIB_FLD_CNT                            REG_FLD(32, 0)

#define RRO_RX_D_DRV_DESC_MIB_FLD_CNT                          REG_FLD(32, 0)

#define RRO_RX_PG_DRV_IN_MIB_FLD_CNT                           REG_FLD(32, 0)

#define RRO_RX_PG_DRV_OU_MIB_FLD_CNT                           REG_FLD(32, 0)

#define RRO_RX_PG_DRV_DESC_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_WPDMA_RX0_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_RX0_CTRL1_FLD_BASE_PTR_H                     REG_FLD(8, 16)
#define WED_WPDMA_RX0_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_RX0_CTRL2_FLD_DRV_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_RX0_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_RX0_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_RX1_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_RX1_CTRL1_FLD_BASE_PTR_H                     REG_FLD(8, 16)
#define WED_WPDMA_RX1_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_RX1_CTRL2_FLD_DRV_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_RX1_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_RX1_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_BASE_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_CNT_FLD_BASE_PTR_H                  REG_FLD(8, 16)
#define WED_WPDMA_RX_D_RX0_CNT_FLD_MAX                         REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_CRX_IDX_FLD_DRV_IDX                 REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX0_CRX_IDX_FLD_CRX_IDX                 REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_DRX_IDX_FLD_DRX_IDX_MIRO            REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX1_BASE_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_CNT_FLD_BASE_PTR_H                  REG_FLD(8, 16)
#define WED_WPDMA_RX_D_RX1_CNT_FLD_MAX                         REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX1_CRX_IDX_FLD_DRV_IDX                 REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX1_CRX_IDX_FLD_CRX_IDX                 REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX1_DRX_IDX_FLD_DRX_IDX_MIRO            REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX0_THRES_CFG_FLD_WAIT_BM_CNT_MAX       REG_FLD(16, 0)

#define WED_WPDMA_RX_D_RX1_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX1_THRES_CFG_FLD_WAIT_BM_CNT_MAX       REG_FLD(16, 0)

#define WED_WPDMA_RX_D_INFO_FLD_INDEX_WIDTH                    REG_FLD(4, 24)
#define WED_WPDMA_RX_D_INFO_FLD_BASE_PTR_WIDTH                 REG_FLD(8, 16)
#define WED_WPDMA_RX_D_INFO_FLD_RX_RING_NUM                    REG_FLD(8, 8)

#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXD_READ_LEN                REG_FLD(8, 24)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXD_READ_LEN_MASK           GENMASK(31, 24)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD REG_FLD(1, 23)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_CNT_SRC   REG_FLD(1, 22)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE        REG_FLD(1, 21)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE   REG_FLD(1, 20)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN   REG_FLD(1, 19)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE REG_FLD(1, 18)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_AXI_W_AFTER_AW_EN           REG_FLD(1, 17)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_WAIT_COHERENT_EN            REG_FLD(1, 16)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_WCOMPLETE_SEL               REG_FLD(1, 15)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DISABLE_DMAD_HANDLE_PROC    REG_FLD(1, 14)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DISABLE_IRQ_HANDLE_PROC     REG_FLD(1, 13)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXDRV_DISABLED_FSM_AUTO_IDLE REG_FLD(1, 12)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_RXEN_SEL         REG_FLD(5, 7)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_ENABLING_RXDMA_BYPASS REG_FLD(1, 6)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_BYPASS           REG_FLD(1, 5)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG      REG_FLD(1, 4)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_FSM_RETURN_IDLE             REG_FLD(1, 3)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXDRV_CLKGATE_BYP           REG_FLD(1, 2)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_BUSY                 REG_FLD(1, 1)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN                   REG_FLD(1, 0)

#define WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX1                    REG_FLD(1, 25)
#define WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX0                    REG_FLD(1, 24)
#define WED_WPDMA_RX_D_RST_IDX_FLD_DRX_IDX_ALL                 REG_FLD(1, 20)
#define WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX1                    REG_FLD(1, 17)
#define WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX0                    REG_FLD(1, 16)

#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_LOAD_DRV_IDX1          REG_FLD(1, 31)
#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_DRV_IDX1               REG_FLD(12, 16)
#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_LOAD_DRV_IDX0          REG_FLD(1, 15)
#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_DRV_IDX0               REG_FLD(12, 0)

#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_LOAD_CRX_IDX1          REG_FLD(1, 31)
#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_CRX_IDX1               REG_FLD(12, 16)
#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_LOAD_CRX_IDX0          REG_FLD(1, 15)
#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_CRX_IDX0               REG_FLD(12, 0)

#define WED_WPDMA_RX_D_GLO_CFG_ADDR_FLD_ADDRESS                REG_FLD(32, 0)

#define WED_WPDMA_RX_D_GLO_CFG_ADDR_H_FLD_ADDRESS              REG_FLD(8, 0)

#define WED_WPDMA_RX_D_RX0_MIB_FLD_PKT_CNT                     REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_MIB_FLD_PKT_CNT                     REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_RECYCLE_MIB_FLD_CNT                 REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_RECYCLE_MIB_FLD_CNT                 REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_PROCESSED_MIB_FLD_PKT_CNT           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_PROCESSED_MIB_FLD_PKT_CNT           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX_COHERENT_MIB_FLD_CNT                 REG_FLD(32, 0)

#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_PREF_DMAD_ERR        REG_FLD(1, 9)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_PREF_R_RESP_ERR      REG_FLD(1, 8)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_GET_BM_DMAD_SKIP     REG_FLD(1, 7)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_DMAD_RECYCLE         REG_FLD(1, 6)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_BM_DMAD_ERROR        REG_FLD(1, 5)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_BM_DMAD_COHERENT     REG_FLD(1, 4)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_INTI_WDMA_ENABLED    REG_FLD(1, 3)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_COHERENT             REG_FLD(1, 2)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_W_RESP_ERR           REG_FLD(1, 1)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_R_RESP_ERR           REG_FLD(1, 0)

#define WED_WPDMA_RX_D_TKID_CFG_FLD_TKID_RING_EN               REG_FLD(1, 0)

#define WED_WPDMA_RX_D_TKID_RX0_BASE_FLD_PTR                   REG_FLD(32, 0)

#define WED_WPDMA_RX_D_TKID_RX1_BASE_FLD_PTR                   REG_FLD(32, 0)

#define WED_WPDMA_RX_D_TKID_RX_BASE_H_FLD_RX1_PTR_H            REG_FLD(8, 16)
#define WED_WPDMA_RX_D_TKID_RX_BASE_H_FLD_RX0_PTR_H            REG_FLD(8, 0)

#define WED_WPDMA_RX_D_BUS_CFG_FLD_ARULTRA                     REG_FLD(2, 12)
#define WED_WPDMA_RX_D_BUS_CFG_FLD_AWULTRA                     REG_FLD(2, 8)
#define WED_WPDMA_RX_D_BUS_CFG_FLD_RD_BND_4KB_BST              REG_FLD(1, 1)
#define WED_WPDMA_RX_D_BUS_CFG_FLD_WR_BND_4KB_BST              REG_FLD(1, 0)

#define WED_WPDMA_RX_D_PREF_CFG_FLD_LOW_THRES                  REG_FLD(6, 16)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_BURST_SIZE                 REG_FLD(5, 8)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_RD_BND_4KB_BST             REG_FLD(1, 6)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_WR_BND_4KB_BST             REG_FLD(1, 5)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_DDONE_POLARITY             REG_FLD(1, 4)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_DDONE_CHK                  REG_FLD(1, 3)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_DMAD_SIZE                  REG_FLD(1, 2)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_BUSY                       REG_FLD(1, 1)
#define WED_WPDMA_RX_D_PREF_CFG_FLD_ENABLE                     REG_FLD(1, 0)

#define WED_WPDMA_RX_D_PREF_RX0_SIDX_FLD_IDX_OW                REG_FLD(1, 31)
#define WED_WPDMA_RX_D_PREF_RX0_SIDX_FLD_IDX_OW_VAL            REG_FLD(12, 16)
#define WED_WPDMA_RX_D_PREF_RX0_SIDX_FLD_IDX_CLR               REG_FLD(1, 15)
#define WED_WPDMA_RX_D_PREF_RX0_SIDX_FLD_START_IDX             REG_FLD(12, 0)

#define WED_WPDMA_RX_D_PREF_RX1_SIDX_FLD_IDX_OW                REG_FLD(1, 31)
#define WED_WPDMA_RX_D_PREF_RX1_SIDX_FLD_IDX_OW_VAL            REG_FLD(12, 16)
#define WED_WPDMA_RX_D_PREF_RX1_SIDX_FLD_IDX_CLR               REG_FLD(1, 15)
#define WED_WPDMA_RX_D_PREF_RX1_SIDX_FLD_START_IDX             REG_FLD(12, 0)

#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING1_FREE_CNT        REG_FLD(6, 26)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING1_USED_CNT        REG_FLD(6, 20)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING1_EMPTY           REG_FLD(1, 18)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING1_FULL            REG_FLD(1, 17)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING1_CLEAR           REG_FLD(1, 16)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING0_FREE_CNT        REG_FLD(6, 10)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING0_USED_CNT        REG_FLD(6, 4)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING0_EMPTY           REG_FLD(1, 2)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING0_FULL            REG_FLD(1, 1)
#define WED_WPDMA_RX_D_PREF_FIFO_CFG_FLD_RING0_CLEAR           REG_FLD(1, 0)

#define WED_WPDMA_RX_D_CIDX_WR_CFG_FLD_CIDX_DLY_THRES          REG_FLD(16, 16)
#define WED_WPDMA_RX_D_CIDX_WR_CFG_FLD_CIDX_DLY_CNT_MAX        REG_FLD(8, 8)
#define WED_WPDMA_RX_D_CIDX_WR_CFG_FLD_CIDX_DLY_CNT_CLR        REG_FLD(2, 2)
#define WED_WPDMA_RX_D_CIDX_WR_CFG_FLD_CIDX_DLY_THRES_EN       REG_FLD(1, 0)

#define WED_WPDMA_RX_D_RING0_CFG_ADDR_FLD_ADDRESS              REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RING0_CFG_ADDR_H_FLD_ADDRESS            REG_FLD(8, 0)

#define WED_WPDMA_RX_D_RING1_CFG_ADDR_FLD_ADDRESS              REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RING1_CFG_ADDR_H_FLD_ADDRESS            REG_FLD(8, 0)

#define WED_WPDMA_RX_D_AW_RAN_DBG0_FLD_ERR_CLR                 REG_FLD(1, 2)
#define WED_WPDMA_RX_D_AW_RAN_DBG0_FLD_ERR                     REG_FLD(1, 1)
#define WED_WPDMA_RX_D_AW_RAN_DBG0_FLD_ENABLE                  REG_FLD(1, 0)

#define WED_WPDMA_RX_D_AW_RAN_DBG1_FLD_ADDR_MAX_31_0           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_AW_RAN_DBG2_FLD_ADDR_MIN_31_0           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_AW_RAN_DBG3_FLD_ADDR_MIN_47_32          REG_FLD(16, 16)
#define WED_WPDMA_RX_D_AW_RAN_DBG3_FLD_ADDR_MAX_47_32          REG_FLD(16, 0)

#define WED_WPDMA_RX_D_AW_RAN_DBG4_FLD_ERR_ADDR_31_0           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_AW_RAN_DBG5_FLD_ERR_LEN                 REG_FLD(4, 16)
#define WED_WPDMA_RX_D_AW_RAN_DBG5_FLD_ERR_ADDR_47_16          REG_FLD(16, 0)

#define WED_WDMA_TX0_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_TX0_CNT_FLD_BASE_PTR_H                        REG_FLD(8, 16)
#define WED_WDMA_TX0_CNT_FLD_MAX                               REG_FLD(12, 0)

#define WED_WDMA_TX0_CTX_IDX_FLD_CTX_IDX                       REG_FLD(12, 0)

#define WED_WDMA_TX0_DTX_IDX_FLD_DTX_IDX                       REG_FLD(12, 0)

#define WED_WDMA_TX0_MIB_FLD_RD_CNT                            REG_FLD(16, 16)
#define WED_WDMA_TX0_MIB_FLD_WR_CNT                            REG_FLD(16, 0)

#define WED_WDMA_RX0_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_RX0_CNT_FLD_BASE_PTR_H                        REG_FLD(8, 16)
#define WED_WDMA_RX0_CNT_FLD_MAX                               REG_FLD(16, 0)

#define WED_WDMA_RX0_CRX_IDX_FLD_DRV_IDX                       REG_FLD(16, 16)
#define WED_WDMA_RX0_CRX_IDX_FLD_CRX_IDX                       REG_FLD(16, 0)

#define WED_WDMA_RX0_DRX_IDX_FLD_DRX_IDX_MIRO                  REG_FLD(16, 0)

#define WED_WDMA_RX1_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_RX1_CNT_FLD_BASE_PTR_H                        REG_FLD(8, 16)
#define WED_WDMA_RX1_CNT_FLD_MAX                               REG_FLD(16, 0)

#define WED_WDMA_RX1_CRX_IDX_FLD_DRV_IDX                       REG_FLD(16, 16)
#define WED_WDMA_RX1_CRX_IDX_FLD_CRX_IDX                       REG_FLD(16, 0)

#define WED_WDMA_RX1_DRX_IDX_FLD_DRX_IDX_MIRO                  REG_FLD(16, 0)

#define WED_WDMA_RX0_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES      REG_FLD(16, 16)
#define WED_WDMA_RX0_THRES_CFG_FLD_WAIT_BM_CNT_MAX             REG_FLD(16, 0)

#define WED_WDMA_RX1_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES      REG_FLD(16, 16)
#define WED_WDMA_RX1_THRES_CFG_FLD_WAIT_BM_CNT_MAX             REG_FLD(16, 0)

#define WED_WDMA_RX_PREF_CFG_FLD_DDONE2_CLR                    REG_FLD(1, 29)
#define WED_WDMA_RX_PREF_CFG_FLD_DDONE2_CLR_MET                REG_FLD(1, 28)
#define WED_WDMA_RX_PREF_CFG_FLD_DDONE2_BUSY                   REG_FLD(1, 27)
#define WED_WDMA_RX_PREF_CFG_FLD_DDONE2_EN                     REG_FLD(1, 26)
#define WED_WDMA_RX_PREF_CFG_FLD_RX1_SIDX_CLR                  REG_FLD(1, 25)
#define WED_WDMA_RX_PREF_CFG_FLD_RX0_SIDX_CLR                  REG_FLD(1, 24)
#define WED_WDMA_RX_PREF_CFG_FLD_RX1_SIDX_OW                   REG_FLD(1, 23)
#define WED_WDMA_RX_PREF_CFG_FLD_RX0_SIDX_OW                   REG_FLD(1, 22)
#define WED_WDMA_RX_PREF_CFG_FLD_LOW_THRES                     REG_FLD(6, 16)
#define WED_WDMA_RX_PREF_CFG_FLD_DMAD_ERR                      REG_FLD(1, 14)
#define WED_WDMA_RX_PREF_CFG_FLD_R_RESP_ERR                    REG_FLD(1, 13)
#define WED_WDMA_RX_PREF_CFG_FLD_BURST_SIZE                    REG_FLD(5, 8)
#define WED_WDMA_RX_PREF_CFG_FLD_RD_BND_4KB_BST                REG_FLD(1, 6)
#define WED_WDMA_RX_PREF_CFG_FLD_WR_BND_4KB_BST                REG_FLD(1, 5)
#define WED_WDMA_RX_PREF_CFG_FLD_DDONE_POLARITY                REG_FLD(1, 4)
#define WED_WDMA_RX_PREF_CFG_FLD_DDONE_CHK                     REG_FLD(1, 3)
#define WED_WDMA_RX_PREF_CFG_FLD_DMAD_SIZE                     REG_FLD(1, 2)
#define WED_WDMA_RX_PREF_CFG_FLD_BUSY                          REG_FLD(1, 1)
#define WED_WDMA_RX_PREF_CFG_FLD_ENABLE                        REG_FLD(1, 0)

#define WED_WDMA_RX_PREF_RX0_SIDX_FLD_IDX_OW_VAL               REG_FLD(16, 16)
#define WED_WDMA_RX_PREF_RX0_SIDX_FLD_START_IDX                REG_FLD(16, 0)

#define WED_WDMA_RX_PREF_RX1_SIDX_FLD_IDX_OW_VAL               REG_FLD(16, 16)
#define WED_WDMA_RX_PREF_RX1_SIDX_FLD_START_IDX                REG_FLD(16, 0)

#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING1_FREE_CNT           REG_FLD(6, 26)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING1_USED_CNT           REG_FLD(6, 20)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING1_EMPTY              REG_FLD(1, 18)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING1_FULL               REG_FLD(1, 17)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING1_CLEAR              REG_FLD(1, 16)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING0_FREE_CNT           REG_FLD(6, 10)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING0_USED_CNT           REG_FLD(6, 4)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING0_EMPTY              REG_FLD(1, 2)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING0_FULL               REG_FLD(1, 1)
#define WED_WDMA_RX_PREF_FIFO_CFG_FLD_RING0_CLEAR              REG_FLD(1, 0)

#define WED_WDMA_RX_CIDX_WR_CFG_FLD_CIDX_DLY_THRES             REG_FLD(16, 16)
#define WED_WDMA_RX_CIDX_WR_CFG_FLD_CIDX_DLY_CNT_MAX           REG_FLD(8, 8)
#define WED_WDMA_RX_CIDX_WR_CFG_FLD_CIDX_DLY_CNT_CLR           REG_FLD(2, 2)
#define WED_WDMA_RX_CIDX_WR_CFG_FLD_CIDX_DLY_THRES_EN          REG_FLD(1, 0)

#define WED_WDMA_INFO_FLD_INDEX_WIDTH                          REG_FLD(4, 24)
#define WED_WDMA_INFO_FLD_BASE_PTR_WIDTH                       REG_FLD(8, 16)
#define WED_WDMA_INFO_FLD_RX_RING_NUM                          REG_FLD(8, 8)
#define WED_WDMA_INFO_FLD_TX_RING_NUM                          REG_FLD(8, 0)

#define WED_WDMA_GLO_CFG_FLD_RXDRV_CLKGATE_BYP                 REG_FLD(1, 30)
#define WED_WDMA_GLO_CFG_FLD_TX_DRV_DTX_DLY_DIS                REG_FLD(1, 29)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD REG_FLD(1, 28)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_CNT_SRC         REG_FLD(1, 27)
#define WED_WDMA_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG            REG_FLD(1, 26)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE              REG_FLD(1, 25)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE         REG_FLD(1, 24)
#define WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN         REG_FLD(1, 23)
#define WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE REG_FLD(1, 22)
#define WED_WDMA_GLO_CFG_FLD_AXI_W_AFTER_AW_EN                 REG_FLD(1, 21)
#define WED_WDMA_GLO_CFG_FLD_WAIT_COHERENT_EN                  REG_FLD(1, 20)
#define WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE                   REG_FLD(1, 19)
#define WED_WDMA_GLO_CFG_FLD_INIT_PHASE_BYPASS                 REG_FLD(1, 18)
#define WED_WDMA_GLO_CFG_FLD_INIT_PHASE_ENABLING_RXDMA_BYPASS  REG_FLD(1, 17)
#define WED_WDMA_GLO_CFG_FLD_WCOMPLETE_SEL                     REG_FLD(1, 16)
#define WED_WDMA_GLO_CFG_FLD_DISABLE_DMAD_HANDLE_PROC          REG_FLD(1, 15)
#define WED_WDMA_GLO_CFG_FLD_DISABLE_IRQ_HANDLE_PROC           REG_FLD(1, 14)
#define WED_WDMA_GLO_CFG_FLD_RXDRV_DISABLED_FSM_AUTO_IDLE      REG_FLD(1, 13)
#define WED_WDMA_GLO_CFG_FLD_TX_DRV_BURST_CFG                  REG_FLD(2, 11)
#define WED_WDMA_GLO_CFG_FLD_RXDRV_DDONE2_CHK                  REG_FLD(1, 10)
#define WED_WDMA_GLO_CFG_FLD_LS_CHK_BYP                        REG_FLD(1, 9)
#define WED_WDMA_GLO_CFG_FLD_TX_DDONE_CHK_LAST                 REG_FLD(1, 8)
#define WED_WDMA_GLO_CFG_FLD_BIG_ENDIAN                        REG_FLD(1, 7)
#define WED_WDMA_GLO_CFG_FLD_TX_WB_DDONE                       REG_FLD(1, 6)
#define WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE                      REG_FLD(2, 4)
#define WED_WDMA_GLO_CFG_FLD_RX_DRV_BUSY                       REG_FLD(1, 3)
#define WED_WDMA_GLO_CFG_FLD_RX_DRV_EN                         REG_FLD(1, 2)
#define WED_WDMA_GLO_CFG_FLD_TX_DDONE_CHK                      REG_FLD(1, 1)
#define WED_WDMA_GLO_CFG_FLD_TX_DRV_EN                         REG_FLD(1, 0)

#define WED_WDMA_RST_IDX_FLD_DRV_IDX1                          REG_FLD(1, 25)
#define WED_WDMA_RST_IDX_FLD_DRV_IDX0                          REG_FLD(1, 24)
#define WED_WDMA_RST_IDX_FLD_DRX_IDX_ALL                       REG_FLD(1, 20)
#define WED_WDMA_RST_IDX_FLD_CTX_IDX1                          REG_FLD(1, 19)
#define WED_WDMA_RST_IDX_FLD_CTX_IDX0                          REG_FLD(1, 18)
#define WED_WDMA_RST_IDX_FLD_CRX_IDX1                          REG_FLD(1, 17)
#define WED_WDMA_RST_IDX_FLD_CRX_IDX0                          REG_FLD(1, 16)

#define WED_WDMA_LOAD_IDX_FLD_CRX_IDX1                         REG_FLD(1, 9)
#define WED_WDMA_LOAD_IDX_FLD_CRX_IDX0                         REG_FLD(1, 8)
#define WED_WDMA_LOAD_IDX_FLD_DRV_IDX1                         REG_FLD(1, 1)
#define WED_WDMA_LOAD_IDX_FLD_DRV_IDX0                         REG_FLD(1, 0)

#define WED_WDMA_LOAD_DRV_IDX_FLD_DRV_IDX1                     REG_FLD(16, 16)
#define WED_WDMA_LOAD_DRV_IDX_FLD_DRV_IDX0                     REG_FLD(16, 0)

#define WED_WDMA_LOAD_CRX_IDX_FLD_CRX_IDX1                     REG_FLD(16, 16)
#define WED_WDMA_LOAD_CRX_IDX_FLD_CRX_IDX0                     REG_FLD(16, 0)

#define WED_WDMA_SPR_FLD_RSV_REG                               REG_FLD(32, 0)

#define WED_WDMA_INT_STA_REC_FLD_RX_COHERENT                   REG_FLD(1, 31)
#define WED_WDMA_INT_STA_REC_FLD_RX_DLY_INT                    REG_FLD(1, 30)
#define WED_WDMA_INT_STA_REC_FLD_TX_COHERENT                   REG_FLD(1, 29)
#define WED_WDMA_INT_STA_REC_FLD_TX_DLY_INT                    REG_FLD(1, 28)
#define WED_WDMA_INT_STA_REC_FLD_RX_DONE1                      REG_FLD(1, 17)
#define WED_WDMA_INT_STA_REC_FLD_RX_DONE0                      REG_FLD(1, 16)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE3                      REG_FLD(1, 3)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE2                      REG_FLD(1, 2)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE1                      REG_FLD(1, 1)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE0                      REG_FLD(1, 0)

#define WED_WDMA_INT_CLR_FLD_INT_CLR                           REG_FLD(32, 0)

#define WED_WDMA_INT_TRIG_FLD_INT_TRIG                         REG_FLD(32, 0)

#define WED_WDMA_INT_CTRL_FLD_SUBRT_ADV                        REG_FLD(1, 21)
#define WED_WDMA_INT_CTRL_FLD_EXT_INCPT_DIS                    REG_FLD(1, 18)
#define WED_WDMA_INT_CTRL_FLD_SRC_SEL                          REG_FLD(2, 16)
#define WED_WDMA_INT_CTRL_FLD_POLL_MODE                        REG_FLD(2, 12)
#define WED_WDMA_INT_CTRL_FLD_POLL_PRD                         REG_FLD(8, 0)

#define WED_WDMA_INT_MON_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define WED_WDMA_INT_MON_FLD_RX_DONE_INT0                      REG_FLD(1, 16)

#define WED_WDMA_CFG_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_OFST0_FLD_GLO_CFG                             REG_FLD(16, 16)
#define WED_WDMA_OFST0_FLD_INTS                                REG_FLD(16, 0)

#define WED_WDMA_OFST1_FLD_RX0_CTRL                            REG_FLD(16, 16)
#define WED_WDMA_OFST1_FLD_TX0_CTRL                            REG_FLD(16, 0)

#define WED_WDMA_CFG_BASE_H_FLD_PTR                            REG_FLD(8, 0)

#define WED_WDMA_RX0_MIB_FLD_PKT_CNT                           REG_FLD(32, 0)

#define WED_WDMA_RX1_MIB_FLD_PKT_CNT                           REG_FLD(32, 0)

#define WED_WDMA_RX0_RECYCLE_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_WDMA_RX1_RECYCLE_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_WDMA_RX0_PROCESSED_MIB_FLD_PKT_CNT                 REG_FLD(32, 0)

#define WED_WDMA_RX1_PROCESSED_MIB_FLD_PKT_CNT                 REG_FLD(32, 0)

#define WED_WDMA_RX_COHERENT_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_GLO_CFG_FLD_CG_DIS                            REG_FLD(1, 31)
#define WED_RTQM_GLO_CFG_FLD_STATUS                            REG_FLD(13, 16)
#define WED_RTQM_GLO_CFG_FLD_ENABLE                            REG_FLD(9, 0)

#define WED_RTQM_RST_FLD_RST                                   REG_FLD(12, 0)

#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS3_EMPTY                 REG_FLD(1, 31)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS3_FULL                  REG_FLD(1, 30)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS3_CLEAR                 REG_FLD(1, 29)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS3_CNT                   REG_FLD(5, 24)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS2_EMPTY                 REG_FLD(1, 23)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS2_FULL                  REG_FLD(1, 22)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS2_CLEAR                 REG_FLD(1, 21)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS2_CNT                   REG_FLD(5, 16)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS1_EMPTY                 REG_FLD(1, 15)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS1_FULL                  REG_FLD(1, 14)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS1_CLEAR                 REG_FLD(1, 13)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS1_CNT                   REG_FLD(5, 8)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS0_EMPTY                 REG_FLD(1, 7)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS0_FULL                  REG_FLD(1, 6)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS0_CLEAR                 REG_FLD(1, 5)
#define WED_RTQM_IGRS_FIFO_CFG_FLD_IGRS0_CNT                   REG_FLD(5, 0)

#define WED_RTQM_PFDBK_FIFO_CFG_FLD_A2Q_EMPTY                  REG_FLD(1, 26)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_A2Q_FULL                   REG_FLD(1, 25)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_A2Q_CLEAR                  REG_FLD(1, 24)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_A2Q_CNT                    REG_FLD(5, 16)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_P2Q_EMPTY                  REG_FLD(1, 15)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_P2Q_FULL                   REG_FLD(1, 14)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_P2Q_CLEAR                  REG_FLD(1, 13)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_P2Q_GFFDIS                 REG_FLD(1, 12)
#define WED_RTQM_PFDBK_FIFO_CFG_FLD_P2Q_CNT                    REG_FLD(11, 0)

#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_7_EMPTY                REG_FLD(1, 31)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_7_FULL                 REG_FLD(1, 30)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_7_CLEAR                REG_FLD(1, 29)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_6_EMPTY                REG_FLD(1, 27)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_6_FULL                 REG_FLD(1, 26)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_6_CLEAR                REG_FLD(1, 25)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_5_EMPTY                REG_FLD(1, 23)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_5_FULL                 REG_FLD(1, 22)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_5_CLEAR                REG_FLD(1, 21)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_4_EMPTY                REG_FLD(1, 19)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_4_FULL                 REG_FLD(1, 18)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_4_CLEAR                REG_FLD(1, 17)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_3_EMPTY                REG_FLD(1, 15)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_3_FULL                 REG_FLD(1, 14)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_3_CLEAR                REG_FLD(1, 13)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_2_EMPTY                REG_FLD(1, 11)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_2_FULL                 REG_FLD(1, 10)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_2_CLEAR                REG_FLD(1, 9)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_1_EMPTY                REG_FLD(1, 7)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_1_FULL                 REG_FLD(1, 6)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_1_CLEAR                REG_FLD(1, 5)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_0_EMPTY                REG_FLD(1, 3)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_0_FULL                 REG_FLD(1, 2)
#define WED_RTQM_PREF_FIFO_CFG_FLD_PREF_0_CLEAR                REG_FLD(1, 1)

#define WED_RTQM_IGRS0_CFG0_FLD_HOST_RING_SRC                  REG_FLD(2, 28)
#define WED_RTQM_IGRS0_CFG0_FLD_DMAD_MOD_METHOD                REG_FLD(1, 27)
#define WED_RTQM_IGRS0_CFG0_FLD_DMAD_MOD_EN                    REG_FLD(7, 20)
#define WED_RTQM_IGRS0_CFG0_FLD_DMAD_CLR_PPE_FLD               REG_FLD(2, 18)
#define WED_RTQM_IGRS0_CFG0_FLD_SCH_REQ_ANY_TIME               REG_FLD(1, 16)
#define WED_RTQM_IGRS0_CFG0_FLD_DEST_CHK_V1                    REG_FLD(8, 8)
#define WED_RTQM_IGRS0_CFG0_FLD_DEST_CHK_V0                    REG_FLD(3, 4)
#define WED_RTQM_IGRS0_CFG0_FLD_FORCE_DROP                     REG_FLD(1, 3)
#define WED_RTQM_IGRS0_CFG0_FLD_DEST_FORCE                     REG_FLD(3, 0)

#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_PPE_VLD               REG_FLD(1, 31)
#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_PPE_ENTRY             REG_FLD(15, 16)
#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_CRSN                  REG_FLD(5, 11)
#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_DDONE                 REG_FLD(1, 10)
#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_LS                    REG_FLD(1, 9)
#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_RING_INFO             REG_FLD(2, 7)
#define WED_RTQM_IGRS0_CFG1_FLD_DMAD_MOD_TO_HOST               REG_FLD(1, 6)

#define WED_RTQM_IGRS0_I2HW_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS0_I2H0_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS0_I2H1_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS0_I2HW_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS0_I2H0_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS0_I2H1_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS0_FDROP_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_IGRS0_DBG_FLD_SEQ_ID_CLR                      REG_FLD(1, 31)
#define WED_RTQM_IGRS0_DBG_FLD_SEQ_ID_MON                      REG_FLD(15, 16)
#define WED_RTQM_IGRS0_DBG_FLD_SEQ_ID_ENABLE                   REG_FLD(1, 15)
#define WED_RTQM_IGRS0_DBG_FLD_SCH_RESULT_RLS_MON              REG_FLD(3, 12)
#define WED_RTQM_IGRS0_DBG_FLD_SCH_RESULT_MON                  REG_FLD(3, 9)
#define WED_RTQM_IGRS0_DBG_FLD_SCH_REQ_MON                     REG_FLD(3, 6)
#define WED_RTQM_IGRS0_DBG_FLD_SCH_RLS_MANUAL                  REG_FLD(1, 5)
#define WED_RTQM_IGRS0_DBG_FLD_SCH_RLS_SET                     REG_FLD(1, 4)
#define WED_RTQM_IGRS0_DBG_FLD_XOP_CLR                         REG_FLD(1, 3)
#define WED_RTQM_IGRS0_DBG_FLD_NEXT_DMAD_SOP_MON               REG_FLD(1, 2)
#define WED_RTQM_IGRS0_DBG_FLD_CURR_DMAD_SOP_MON               REG_FLD(1, 1)
#define WED_RTQM_IGRS0_DBG_FLD_CURR_DMAD_EOP_MON               REG_FLD(1, 0)

#define WED_RTQM_IGRS1_CFG0_FLD_HOST_RING_SRC                  REG_FLD(2, 28)
#define WED_RTQM_IGRS1_CFG0_FLD_DMAD_MOD_METHOD                REG_FLD(1, 27)
#define WED_RTQM_IGRS1_CFG0_FLD_DMAD_MOD_EN                    REG_FLD(7, 20)
#define WED_RTQM_IGRS1_CFG0_FLD_DMAD_CLR_PPE_FLD               REG_FLD(2, 18)
#define WED_RTQM_IGRS1_CFG0_FLD_SCH_REQ_ANY_TIME               REG_FLD(1, 16)
#define WED_RTQM_IGRS1_CFG0_FLD_DEST_CHK_V1                    REG_FLD(8, 8)
#define WED_RTQM_IGRS1_CFG0_FLD_DEST_CHK_V0                    REG_FLD(3, 4)
#define WED_RTQM_IGRS1_CFG0_FLD_FORCE_DROP                     REG_FLD(1, 3)
#define WED_RTQM_IGRS1_CFG0_FLD_DEST_FORCE                     REG_FLD(3, 0)

#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_PPE_VLD               REG_FLD(1, 31)
#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_PPE_ENTRY             REG_FLD(15, 16)
#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_CRSN                  REG_FLD(5, 11)
#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_DDONE                 REG_FLD(1, 10)
#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_LS                    REG_FLD(1, 9)
#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_RING_INFO             REG_FLD(2, 7)
#define WED_RTQM_IGRS1_CFG1_FLD_DMAD_MOD_TO_HOST               REG_FLD(1, 6)

#define WED_RTQM_IGRS1_I2HW_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS1_I2H0_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS1_I2H1_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS1_I2HW_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS1_I2H0_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS1_I2H1_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS1_FDROP_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_IGRS1_DBG_FLD_SEQ_ID_CLR                      REG_FLD(1, 31)
#define WED_RTQM_IGRS1_DBG_FLD_SEQ_ID_MON                      REG_FLD(15, 16)
#define WED_RTQM_IGRS1_DBG_FLD_SEQ_ID_ENABLE                   REG_FLD(1, 15)
#define WED_RTQM_IGRS1_DBG_FLD_SCH_RESULT_RLS_MON              REG_FLD(3, 12)
#define WED_RTQM_IGRS1_DBG_FLD_SCH_RESULT_MON                  REG_FLD(3, 9)
#define WED_RTQM_IGRS1_DBG_FLD_SCH_REQ_MON                     REG_FLD(3, 6)
#define WED_RTQM_IGRS1_DBG_FLD_SCH_RLS_MANUAL                  REG_FLD(1, 5)
#define WED_RTQM_IGRS1_DBG_FLD_SCH_RLS_SET                     REG_FLD(1, 4)
#define WED_RTQM_IGRS1_DBG_FLD_XOP_CLR                         REG_FLD(1, 3)
#define WED_RTQM_IGRS1_DBG_FLD_NEXT_DMAD_SOP_MON               REG_FLD(1, 2)
#define WED_RTQM_IGRS1_DBG_FLD_CURR_DMAD_SOP_MON               REG_FLD(1, 1)
#define WED_RTQM_IGRS1_DBG_FLD_CURR_DMAD_EOP_MON               REG_FLD(1, 0)

#define WED_RTQM_IGRS2_CFG0_FLD_HOST_RING_SRC                  REG_FLD(2, 28)
#define WED_RTQM_IGRS2_CFG0_FLD_DMAD_MOD_METHOD                REG_FLD(1, 27)
#define WED_RTQM_IGRS2_CFG0_FLD_DMAD_MOD_EN                    REG_FLD(7, 20)
#define WED_RTQM_IGRS2_CFG0_FLD_DMAD_CLR_PPE_FLD               REG_FLD(2, 18)
#define WED_RTQM_IGRS2_CFG0_FLD_SCH_REQ_ANY_TIME               REG_FLD(1, 16)
#define WED_RTQM_IGRS2_CFG0_FLD_DEST_CHK_V1                    REG_FLD(8, 8)
#define WED_RTQM_IGRS2_CFG0_FLD_DEST_CHK_V0                    REG_FLD(3, 4)
#define WED_RTQM_IGRS2_CFG0_FLD_FORCE_DROP                     REG_FLD(1, 3)
#define WED_RTQM_IGRS2_CFG0_FLD_DEST_FORCE                     REG_FLD(3, 0)

#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_PPE_VLD               REG_FLD(1, 31)
#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_PPE_ENTRY             REG_FLD(15, 16)
#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_CRSN                  REG_FLD(5, 11)
#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_DDONE                 REG_FLD(1, 10)
#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_LS                    REG_FLD(1, 9)
#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_RING_INFO             REG_FLD(2, 7)
#define WED_RTQM_IGRS2_CFG1_FLD_DMAD_MOD_TO_HOST               REG_FLD(1, 6)

#define WED_RTQM_IGRS2_I2HW_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS2_I2H0_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS2_I2H1_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS2_I2HW_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS2_I2H0_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS2_I2H1_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS2_FDROP_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_IGRS2_DBG_FLD_SEQ_ID_CLR                      REG_FLD(1, 31)
#define WED_RTQM_IGRS2_DBG_FLD_SEQ_ID_MON                      REG_FLD(15, 16)
#define WED_RTQM_IGRS2_DBG_FLD_SEQ_ID_ENABLE                   REG_FLD(1, 15)
#define WED_RTQM_IGRS2_DBG_FLD_SCH_RESULT_RLS_MON              REG_FLD(3, 12)
#define WED_RTQM_IGRS2_DBG_FLD_SCH_RESULT_MON                  REG_FLD(3, 9)
#define WED_RTQM_IGRS2_DBG_FLD_SCH_REQ_MON                     REG_FLD(3, 6)
#define WED_RTQM_IGRS2_DBG_FLD_SCH_RLS_MANUAL                  REG_FLD(1, 5)
#define WED_RTQM_IGRS2_DBG_FLD_SCH_RLS_SET                     REG_FLD(1, 4)
#define WED_RTQM_IGRS2_DBG_FLD_XOP_CLR                         REG_FLD(1, 3)
#define WED_RTQM_IGRS2_DBG_FLD_NEXT_DMAD_SOP_MON               REG_FLD(1, 2)
#define WED_RTQM_IGRS2_DBG_FLD_CURR_DMAD_SOP_MON               REG_FLD(1, 1)
#define WED_RTQM_IGRS2_DBG_FLD_CURR_DMAD_EOP_MON               REG_FLD(1, 0)

#define WED_RTQM_IGRS3_CFG0_FLD_HOST_RING_SRC                  REG_FLD(2, 28)
#define WED_RTQM_IGRS3_CFG0_FLD_DMAD_MOD_METHOD                REG_FLD(1, 27)
#define WED_RTQM_IGRS3_CFG0_FLD_DMAD_MOD_EN                    REG_FLD(7, 20)
#define WED_RTQM_IGRS3_CFG0_FLD_DMAD_CLR_PPE_FLD               REG_FLD(2, 18)
#define WED_RTQM_IGRS3_CFG0_FLD_SCH_REQ_ANY_TIME               REG_FLD(1, 16)
#define WED_RTQM_IGRS3_CFG0_FLD_DEST_CHK_V1                    REG_FLD(8, 8)
#define WED_RTQM_IGRS3_CFG0_FLD_DEST_CHK_V0                    REG_FLD(3, 4)
#define WED_RTQM_IGRS3_CFG0_FLD_FORCE_DROP                     REG_FLD(1, 3)
#define WED_RTQM_IGRS3_CFG0_FLD_DEST_FORCE                     REG_FLD(3, 0)

#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_PPE_VLD               REG_FLD(1, 31)
#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_PPE_ENTRY             REG_FLD(15, 16)
#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_CRSN                  REG_FLD(5, 11)
#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_DDONE                 REG_FLD(1, 10)
#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_LS                    REG_FLD(1, 9)
#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_RING_INFO             REG_FLD(2, 7)
#define WED_RTQM_IGRS3_CFG1_FLD_DMAD_MOD_TO_HOST               REG_FLD(1, 6)

#define WED_RTQM_IGRS3_I2HW_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS3_I2H0_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS3_I2H1_DMAD_CNT_FLD_CNT                   REG_FLD(32, 0)

#define WED_RTQM_IGRS3_I2HW_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS3_I2H0_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS3_I2H1_PKT_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_IGRS3_FDROP_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_IGRS3_DBG_FLD_SEQ_ID_CLR                      REG_FLD(1, 31)
#define WED_RTQM_IGRS3_DBG_FLD_SEQ_ID_MON                      REG_FLD(15, 16)
#define WED_RTQM_IGRS3_DBG_FLD_SEQ_ID_ENABLE                   REG_FLD(1, 15)
#define WED_RTQM_IGRS3_DBG_FLD_SCH_RESULT_RLS_MON              REG_FLD(3, 12)
#define WED_RTQM_IGRS3_DBG_FLD_SCH_RESULT_MON                  REG_FLD(3, 9)
#define WED_RTQM_IGRS3_DBG_FLD_SCH_REQ_MON                     REG_FLD(3, 6)
#define WED_RTQM_IGRS3_DBG_FLD_SCH_RLS_MANUAL                  REG_FLD(1, 5)
#define WED_RTQM_IGRS3_DBG_FLD_SCH_RLS_SET                     REG_FLD(1, 4)
#define WED_RTQM_IGRS3_DBG_FLD_XOP_CLR                         REG_FLD(1, 3)
#define WED_RTQM_IGRS3_DBG_FLD_NEXT_DMAD_SOP_MON               REG_FLD(1, 2)
#define WED_RTQM_IGRS3_DBG_FLD_CURR_DMAD_SOP_MON               REG_FLD(1, 1)
#define WED_RTQM_IGRS3_DBG_FLD_CURR_DMAD_EOP_MON               REG_FLD(1, 0)

#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS3_ITF3_WEIGHT             REG_FLD(2, 30)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS3_ITF2_WEIGHT             REG_FLD(2, 28)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS3_ITF1_WEIGHT             REG_FLD(2, 26)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS3_ITF0_WEIGHT             REG_FLD(2, 24)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS2_ITF3_WEIGHT             REG_FLD(2, 22)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS2_ITF2_WEIGHT             REG_FLD(2, 20)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS2_ITF1_WEIGHT             REG_FLD(2, 18)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS2_ITF0_WEIGHT             REG_FLD(2, 16)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS1_ITF3_WEIGHT             REG_FLD(2, 14)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS1_ITF2_WEIGHT             REG_FLD(2, 12)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS1_ITF1_WEIGHT             REG_FLD(2, 10)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS1_ITF0_WEIGHT             REG_FLD(2, 8)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS0_ITF3_WEIGHT             REG_FLD(2, 6)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS0_ITF2_WEIGHT             REG_FLD(2, 4)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS0_ITF1_WEIGHT             REG_FLD(2, 2)
#define WED_RTQM_EGRS_WEIGHT_FLD_EGRS0_ITF0_WEIGHT             REG_FLD(2, 0)

#define WED_RTQM_ENQ_CFG0_FLD_SEQ_ID_MON                       REG_FLD(16, 16)
#define WED_RTQM_ENQ_CFG0_FLD_TXDMAD_FPORT                     REG_FLD(4, 12)
#define WED_RTQM_ENQ_CFG0_FLD_TXDMAD_OFLD                      REG_FLD(4, 8)
#define WED_RTQM_ENQ_CFG0_FLD_RXDMAD_OFST_UNIT                 REG_FLD(2, 6)
#define WED_RTQM_ENQ_CFG0_FLD_SEQ_ID_CLR                       REG_FLD(1, 5)
#define WED_RTQM_ENQ_CFG0_FLD_SEQ_ID_ENABLE                    REG_FLD(1, 4)
#define WED_RTQM_ENQ_CFG0_FLD_DMAD_TKID_INS                    REG_FLD(1, 3)
#define WED_RTQM_ENQ_CFG0_FLD_ENQ_ERR_AUTO_FREE                REG_FLD(1, 2)
#define WED_RTQM_ENQ_CFG0_FLD_ENQ_ERR_NO_ENQ                   REG_FLD(1, 1)
#define WED_RTQM_ENQ_CFG0_FLD_FORCE_DROP                       REG_FLD(1, 0)

#define WED_RTQM_ENQ_I2Q_DMAD_CNT_FLD_CNT                      REG_FLD(32, 0)

#define WED_RTQM_ENQ_I2N_DMAD_CNT_FLD_CNT                      REG_FLD(32, 0)

#define WED_RTQM_ENQ_I2Q_PKT_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_ENQ_I2N_PKT_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_ENQ_USED_ENTRY_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_ENQ_ERR_CNT_FLD_CNT                           REG_FLD(32, 0)

#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_TO_HOST                 REG_FLD(1, 26)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_PPE_VLD                 REG_FLD(1, 25)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_CRSN                    REG_FLD(5, 20)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_DDONE                   REG_FLD(1, 19)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_LS                      REG_FLD(1, 18)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_RING_INFO               REG_FLD(2, 16)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_MOD_EN                      REG_FLD(6, 10)
#define WED_RTQM_DEQ_CFG0_FLD_DEQ_FSM_FORCE_IDLE               REG_FLD(1, 9)
#define WED_RTQM_DEQ_CFG0_FLD_DEQ_FSM                          REG_FLD(1, 8)
#define WED_RTQM_DEQ_CFG0_FLD_DEQ_ERR_DEST                     REG_FLD(2, 6)
#define WED_RTQM_DEQ_CFG0_FLD_DEQ_ERR_INS_ADDR                 REG_FLD(1, 5)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_PFDBK_MERGE_DIS             REG_FLD(1, 4)
#define WED_RTQM_DEQ_CFG0_FLD_SG_DMAD_PFDBK_DEFAULT            REG_FLD(1, 3)
#define WED_RTQM_DEQ_CFG0_FLD_SG_DMAD_PFDBK_MERGE              REG_FLD(1, 2)
#define WED_RTQM_DEQ_CFG0_FLD_DMAD_PVLD_UPD                    REG_FLD(1, 1)
#define WED_RTQM_DEQ_CFG0_FLD_FORCE_DROP                       REG_FLD(1, 0)

#define WED_RTQM_DEQ_DQ_DMAD_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_DEQ_Q2I_DMAD_CNT_FLD_CNT                      REG_FLD(32, 0)

#define WED_RTQM_DEQ_DQ_PKT_CNT_FLD_CNT                        REG_FLD(32, 0)

#define WED_RTQM_DEQ_Q2I_PKT_CNT_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_DEQ_USED_PFDBK_CNT_FLD_CNT                    REG_FLD(32, 0)

#define WED_RTQM_DEQ_ERR_CNT_FLD_CNT                           REG_FLD(32, 0)

#define WED_RTQM_DEQ_DBG_FLD_DEQ_ERR_MOD_PPE_VLD               REG_FLD(1, 31)
#define WED_RTQM_DEQ_DBG_FLD_DEQ_ERR_MOD_PPE_ENTRY             REG_FLD(15, 16)
#define WED_RTQM_DEQ_DBG_FLD_DEQ_ERR_MOD_CRSN                  REG_FLD(5, 11)
#define WED_RTQM_DEQ_DBG_FLD_DEQ_ERR_MOD_LS                    REG_FLD(1, 10)
#define WED_RTQM_DEQ_DBG_FLD_DEQ_ERR_MOD_EN                    REG_FLD(4, 6)
#define WED_RTQM_DEQ_DBG_FLD_XOP_CLR                           REG_FLD(1, 3)
#define WED_RTQM_DEQ_DBG_FLD_NEXT_DMAD_SOP_MON                 REG_FLD(1, 2)
#define WED_RTQM_DEQ_DBG_FLD_CURR_DMAD_SOP_MON                 REG_FLD(1, 1)
#define WED_RTQM_DEQ_DBG_FLD_CURR_DMAD_EOP_MON                 REG_FLD(1, 0)

#define WED_RTQM_ERR_STS_FLD_Q2N_ITF_FIFO_W_ERR                REG_FLD(1, 25)
#define WED_RTQM_ERR_STS_FLD_PFDBK_ITF_FIFO_R_ERR              REG_FLD(1, 24)
#define WED_RTQM_ERR_STS_FLD_Q2H1_ITF_FIFO_W_ERR               REG_FLD(1, 23)
#define WED_RTQM_ERR_STS_FLD_Q2H0_ITF_FIFO_W_ERR               REG_FLD(1, 22)
#define WED_RTQM_ERR_STS_FLD_Q2BM_ITF_FIFO_R_ERR               REG_FLD(1, 21)
#define WED_RTQM_ERR_STS_FLD_CR_ITF_FIFO_W_ERR                 REG_FLD(1, 20)
#define WED_RTQM_ERR_STS_FLD_PN_ITF_FIFO_W_ERR                 REG_FLD(1, 19)
#define WED_RTQM_ERR_STS_FLD_RRO_ITF_FIFO_W_ERR                REG_FLD(1, 18)
#define WED_RTQM_ERR_STS_FLD_AGE_SRCH_FIFO_R_ERR               REG_FLD(1, 17)
#define WED_RTQM_ERR_STS_FLD_AGE_SRCH_FIFO_W_ERR               REG_FLD(1, 16)
#define WED_RTQM_ERR_STS_FLD_QFREE_SRCH_FIFO_R_ERR             REG_FLD(1, 15)
#define WED_RTQM_ERR_STS_FLD_QFREE_SRCH_FIFO_W_ERR             REG_FLD(1, 14)
#define WED_RTQM_ERR_STS_FLD_PFDBK_AGE_FIFO_R_ERR              REG_FLD(1, 13)
#define WED_RTQM_ERR_STS_FLD_PFDBK_AGE_FIFO_W_ERR              REG_FLD(1, 12)
#define WED_RTQM_ERR_STS_FLD_PFDBK_FIFO_R_ERR                  REG_FLD(1, 11)
#define WED_RTQM_ERR_STS_FLD_PFDBK_FIFO_W_ERR                  REG_FLD(1, 10)
#define WED_RTQM_ERR_STS_FLD_DEQ_ERR                           REG_FLD(1, 9)
#define WED_RTQM_ERR_STS_FLD_ENQ_ERR                           REG_FLD(1, 8)
#define WED_RTQM_ERR_STS_FLD_IGRS3_FIFO_R_ERR                  REG_FLD(1, 7)
#define WED_RTQM_ERR_STS_FLD_IGRS3_FIFO_W_ERR                  REG_FLD(1, 6)
#define WED_RTQM_ERR_STS_FLD_IGRS2_FIFO_R_ERR                  REG_FLD(1, 5)
#define WED_RTQM_ERR_STS_FLD_IGRS2_FIFO_W_ERR                  REG_FLD(1, 4)
#define WED_RTQM_ERR_STS_FLD_IGRS1_FIFO_R_ERR                  REG_FLD(1, 3)
#define WED_RTQM_ERR_STS_FLD_IGRS1_FIFO_W_ERR                  REG_FLD(1, 2)
#define WED_RTQM_ERR_STS_FLD_IGRS0_FIFO_R_ERR                  REG_FLD(1, 1)
#define WED_RTQM_ERR_STS_FLD_IGRS0_FIFO_W_ERR                  REG_FLD(1, 0)

#define WED_RTQM_QUEUE_CFG0_FLD_DEQ_ERR_CHK                    REG_FLD(4, 4)
#define WED_RTQM_QUEUE_CFG0_FLD_ENQ_ERR_CHK                    REG_FLD(4, 0)

#define WED_RTQM_QUEUE_ACC_CFG_FLD_ADDRESS                     REG_FLD(10, 22)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_WR_CMD_ERR                  REG_FLD(10, 12)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_LINK_HEAD_CLR_ALL      REG_FLD(1, 11)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_AGE_CLR_ALL            REG_FLD(1, 10)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_FETCHABLE_CLR_ALL      REG_FLD(1, 9)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_ENQUEUE_CLR_ALL        REG_FLD(1, 8)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_LINK_HEAD_SET_ALL      REG_FLD(1, 7)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_AGE_SET_ALL            REG_FLD(1, 6)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_FETCHABLE_SET_ALL      REG_FLD(1, 5)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_FLAG_ENQUEUE_SET_ALL        REG_FLD(1, 4)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_RD                          REG_FLD(1, 1)
#define WED_RTQM_QUEUE_ACC_CFG_FLD_WR                          REG_FLD(1, 0)

#define WED_RROQM_GLO_CFG_FLD_RROQM_BYPASS                     REG_FLD(1, 24)
#define WED_RROQM_GLO_CFG_FLD_WAIT_FDBK_COH_EN                 REG_FLD(1, 17)
#define WED_RROQM_GLO_CFG_FLD_WAIT_MOD_COH_EN                  REG_FLD(1, 16)
#define WED_RROQM_GLO_CFG_FLD_ARULTRA                          REG_FLD(2, 12)
#define WED_RROQM_GLO_CFG_FLD_AWULTRA                          REG_FLD(2, 8)
#define WED_RROQM_GLO_CFG_FLD_RROQM_BUSY                       REG_FLD(1, 1)

#define WED_RROQM_RST_IDX_FLD_FDBK                             REG_FLD(1, 4)
#define WED_RROQM_RST_IDX_FLD_MIOD                             REG_FLD(1, 0)

#define WED_RROQM_DLY_INT_FLD_MID_RDY_EN                       REG_FLD(1, 15)
#define WED_RROQM_DLY_INT_FLD_MID_RDY_MAX_PINT                 REG_FLD(7, 8)
#define WED_RROQM_DLY_INT_FLD_MID_RDY_MAX_PTIME                REG_FLD(8, 0)

#define WED_RTQM_QUEUE_ACC_WR0_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_WR1_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_WR2_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_WR3_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_WR4_FLD_WR_CMD                      REG_FLD(10, 16)
#define WED_RTQM_QUEUE_ACC_WR4_FLD_DATA_RSV                    REG_FLD(6, 10)
#define WED_RTQM_QUEUE_ACC_WR4_FLD_LINK                        REG_FLD(10, 0)

#define WED_RTQM_QUEUE_ACC_RD0_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_RD1_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_RD2_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_RD3_FLD_DATA                        REG_FLD(32, 0)

#define WED_RTQM_QUEUE_ACC_RD4_FLD_FLAGS                       REG_FLD(4, 16)
#define WED_RTQM_QUEUE_ACC_RD4_FLD_DATA_RSV                    REG_FLD(6, 10)
#define WED_RTQM_QUEUE_ACC_RD4_FLD_LINK                        REG_FLD(10, 0)

#define WED_RTQM_QUEUE_CNT_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_DBG_CFG_FLD_ENQ_DDONE2_CTRL                   REG_FLD(2, 18)
#define WED_RTQM_DBG_CFG_FLD_ENQ_DDONE_DW2_CTRL                REG_FLD(2, 16)
#define WED_RTQM_DBG_CFG_FLD_ENQ_DDONE_DW1_CTRL                REG_FLD(2, 14)
#define WED_RTQM_DBG_CFG_FLD_ENQ_DDONE_CTRL                    REG_FLD(2, 12)
#define WED_RTQM_DBG_CFG_FLD_PPE_FDBK_DROP                     REG_FLD(1, 10)
#define WED_RTQM_DBG_CFG_FLD_DBG_ENQ_VLD                       REG_FLD(1, 9)
#define WED_RTQM_DBG_CFG_FLD_DBG_ENQ_WR                        REG_FLD(1, 8)
#define WED_RTQM_DBG_CFG_FLD_DBG_MON_SEL                       REG_FLD(8, 0)

#define WED_RROQM_MIOD_CTRL0_FLD_BASE_PTR                      REG_FLD(32, 0)

#define WED_RROQM_MIOD_CTRL1_FLD_BASE_PTR_EXT                  REG_FLD(8, 16)
#define WED_RROQM_MIOD_CTRL1_FLD_MAX_CNT                       REG_FLD(12, 0)

#define WED_RROQM_MIOD_CTRL2_FLD_TAKE_IDX                      REG_FLD(12, 0)

#define WED_RROQM_MIOD_CTRL3_FLD_PUT_IDX                       REG_FLD(12, 0)

#define WED_RROQM_FDBK_CTRL0_FLD_BASE_PTR                      REG_FLD(32, 0)

#define WED_RROQM_FDBK_CTRL1_FLD_BASE_PTR_EXT                  REG_FLD(8, 16)
#define WED_RROQM_FDBK_CTRL1_FLD_MAX_CNT                       REG_FLD(12, 0)

#define WED_RROQM_FDBK_CTRL2_FLD_CPU_IDX                       REG_FLD(12, 0)

#define WED_RROQM_FDBK_CTRL3_FLD_PREFETCH_IDX                  REG_FLD(12, 16)
#define WED_RROQM_FDBK_CTRL3_FLD_DMA_IDX                       REG_FLD(12, 0)

#define WED_RTQM_DBG_MON0_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_MON1_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_MON2_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_MON3_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_ENQ_DATA0_FLD_DMAD                        REG_FLD(32, 0)

#define WED_RTQM_DBG_ENQ_DATA1_FLD_DMAD                        REG_FLD(32, 0)

#define WED_RTQM_DBG_ENQ_DATA2_FLD_DMAD                        REG_FLD(32, 0)

#define WED_RTQM_DBG_ENQ_DATA3_FLD_DMAD                        REG_FLD(32, 0)

#define WED_RROQ_BASE_L_FLD_PTR                                REG_FLD(32, 0)

#define WED_RROQ_BASE_H_FLD_PTR                                REG_FLD(8, 0)

#define WED_RTQM_AGE_CFG0_FLD_DFDBK_TO_HOST                    REG_FLD(1, 31)
#define WED_RTQM_AGE_CFG0_FLD_DFDBK_PPE_ENTRY                  REG_FLD(15, 16)
#define WED_RTQM_AGE_CFG0_FLD_DFDBK_CSRN                       REG_FLD(5, 8)
#define WED_RTQM_AGE_CFG0_FLD_FLUSH_EN                         REG_FLD(1, 3)
#define WED_RTQM_AGE_CFG0_FLD_FSM_FORCE_IDLE                   REG_FLD(1, 2)
#define WED_RTQM_AGE_CFG0_FLD_FSM                              REG_FLD(2, 0)

#define WED_RROQM_MIOD_CFG_FLD_MIOD_ENTRY_DW                   REG_FLD(7, 16)
#define WED_RROQM_MIOD_CFG_FLD_MOD_DW                          REG_FLD(6, 8)
#define WED_RROQM_MIOD_CFG_FLD_MID_DW                          REG_FLD(6, 0)

#define WED_RTQM_AGE_CFG1_FLD_AGE_TIME                         REG_FLD(32, 0)

#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_7_EMPTY                 REG_FLD(1, 31)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_7_FULL                  REG_FLD(1, 30)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_7_CLEAR                 REG_FLD(1, 29)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_6_EMPTY                 REG_FLD(1, 27)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_6_FULL                  REG_FLD(1, 26)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_6_CLEAR                 REG_FLD(1, 25)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_5_EMPTY                 REG_FLD(1, 23)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_5_FULL                  REG_FLD(1, 22)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_5_CLEAR                 REG_FLD(1, 21)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_4_EMPTY                 REG_FLD(1, 19)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_4_FULL                  REG_FLD(1, 18)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_4_CLEAR                 REG_FLD(1, 17)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_3_EMPTY                 REG_FLD(1, 15)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_3_FULL                  REG_FLD(1, 14)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_3_CLEAR                 REG_FLD(1, 13)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_2_EMPTY                 REG_FLD(1, 11)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_2_FULL                  REG_FLD(1, 10)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_2_CLEAR                 REG_FLD(1, 9)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_1_EMPTY                 REG_FLD(1, 7)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_1_FULL                  REG_FLD(1, 6)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_1_CLEAR                 REG_FLD(1, 5)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_0_EMPTY                 REG_FLD(1, 3)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_0_FULL                  REG_FLD(1, 2)
#define WED_RTQM_AGE_FIFO_CFG_FLD_PREF_0_CLEAR                 REG_FLD(1, 1)

#define WED_RTQM_AGE_PKT_CNT_FLD_CNT                           REG_FLD(32, 0)

#define WED_RTQM_PFDBK_CNT_FLD_CNT                             REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC_CTRL_FLD_PAGE                       REG_FLD(2, 8)
#define WED_RROQM_FDBK_REC_CTRL_FLD_CMD_EN                     REG_FLD(4, 0)

#define WED_RROQM_FDBK_REC0_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC1_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC2_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC3_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_MID_MIB_FLD_CNT                              REG_FLD(32, 0)

#define WED_RROQM_MOD_MIB_FLD_CNT                              REG_FLD(32, 0)

#define WED_RROQM_MOD_COH_MIB_FLD_CNT                          REG_FLD(32, 0)

#define WED_RROQM_FDBK_MIB_FLD_CNT                             REG_FLD(32, 0)

#define WED_RROQM_FDBK_COH_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_IND_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_ENQ_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_ANC_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_ANC2H_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_TX_BM_DYN_ALLOC_TH_FLD_H_BUF_TH                    REG_FLD(16, 16)
#define WED_TX_BM_DYN_ALLOC_TH_FLD_L_BUF_TH                    REG_FLD(16, 0)

#define WED_TX_BM_DYN_ALLOC_CFG_FLD_HW_SUB_pause               REG_FLD(1, 31)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_HW_ADDSUB_ACK              REG_FLD(1, 30)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_SW_ADD_OVERFLOW_ERR        REG_FLD(1, 29)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_SW_SUB_UNDERFLOW_ERR       REG_FLD(1, 28)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_SW_SUB_RDY                 REG_FLD(1, 18)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_SW_ADD_BUF_REQ             REG_FLD(1, 17)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_SW_SUB_BUF_REQ             REG_FLD(1, 16)
#define WED_TX_BM_DYN_ALLOC_CFG_FLD_SW_ADDSUB_IDX_CNT          REG_FLD(16, 0)

#define WED_TX_BM_ADD_BASE_FLD_PTR                             REG_FLD(32, 0)

#define WED_TX_BM_ADD_PTR_FLD_HEAD2_IDX                        REG_FLD(16, 16)
#define WED_TX_BM_ADD_PTR_FLD_SW_ADD_IDX_REMAIN                REG_FLD(16, 0)

#define WED_TX_BM_TOTAL_DMAD_IDX_FLD_SW_TOTAL_RX_BM_CNT        REG_FLD(16, 0)

#define WED_TX_BM_ADD_BASE_H_FLD_PTR                           REG_FLD(8, 0)

#define WED_RX_BM_RX_DMAD_FLD_BASE_PTR_H                       REG_FLD(8, 16)
#define WED_RX_BM_RX_DMAD_FLD_SDL0                             REG_FLD(14, 0)

#define WED_RX_BM_BASE_FLD_PTR                                 REG_FLD(32, 0)

#define WED_RX_BM_INIT_PTR_FLD_INIT_SW_TAIL_IDX                REG_FLD(1, 16)
#define WED_RX_BM_INIT_PTR_FLD_SW_TAIL_IDX                     REG_FLD(16, 0)

#define WED_RX_BM_PTR_FLD_HEAD_IDX                             REG_FLD(16, 16)
#define WED_RX_BM_PTR_FLD_TAIL_IDX                             REG_FLD(16, 0)

#define WED_RX_BM_BLEN_FLD_BYTE_LEN                            REG_FLD(14, 0)

#define WED_RX_BM_STS_FLD_DMAD1_RD                             REG_FLD(1, 29)
#define WED_RX_BM_STS_FLD_DMAD1_VLD                            REG_FLD(1, 28)
#define WED_RX_BM_STS_FLD_DMAD1_NUM                            REG_FLD(2, 24)
#define WED_RX_BM_STS_FLD_DMAD0_RD                             REG_FLD(1, 21)
#define WED_RX_BM_STS_FLD_DMAD0_VLD                            REG_FLD(1, 20)
#define WED_RX_BM_STS_FLD_DMAD0_NUM                            REG_FLD(2, 16)
#define WED_RX_BM_STS_FLD_DMAD_NUM                             REG_FLD(5, 8)

#define WED_RX_BM_INTF2_FLD_DMAD_EXCHG_RD                      REG_FLD(1, 29)
#define WED_RX_BM_INTF2_FLD_DMAD_EXCHG_VLD                     REG_FLD(1, 28)
#define WED_RX_BM_INTF2_FLD_DMAD_EXCHG                         REG_FLD(16, 0)

#define WED_RX_BM_INTF_FLD_DMAD_FREE_RD                        REG_FLD(1, 29)
#define WED_RX_BM_INTF_FLD_DMAD_FREE_VLD                       REG_FLD(1, 28)
#define WED_RX_BM_INTF_FLD_DMAD_FREE                           REG_FLD(16, 0)

#define WED_RX_BM_ERR_STS_FLD_FREE_AT_EMPTY                    REG_FLD(1, 7)
#define WED_RX_BM_ERR_STS_FLD_DMAD_AXI_RD_OVER_TAIL            REG_FLD(1, 3)
#define WED_RX_BM_ERR_STS_FLD_DMAD_FULL_AXI_RD                 REG_FLD(1, 2)
#define WED_RX_BM_ERR_STS_FLD_DMAD_AXI_RD_MORE                 REG_FLD(1, 1)
#define WED_RX_BM_ERR_STS_FLD_DMAD_AXI_RD_LESS                 REG_FLD(1, 0)

#define WED_RX_BM_OPT_CTRL_FLD_FREE_AXI_RESP_SEL               REG_FLD(1, 0)

#define WED_RX_BM_DYN_ALLOC_TH_FLD_H_BUF_TH                    REG_FLD(16, 16)
#define WED_RX_BM_DYN_ALLOC_TH_FLD_L_BUF_TH                    REG_FLD(16, 0)

#define WED_RX_BM_DYN_ALLOC_CFG_FLD_HW_SUB_pause               REG_FLD(1, 31)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_HW_ADDSUB_ACK              REG_FLD(1, 30)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_SW_ADD_OVERFLOW_ERR        REG_FLD(1, 29)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_SW_SUB_UNDERFLOW_ERR       REG_FLD(1, 28)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_SW_SUB_RDY                 REG_FLD(1, 18)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_SW_ADD_BUF_REQ             REG_FLD(1, 17)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_SW_SUB_BUF_REQ             REG_FLD(1, 16)
#define WED_RX_BM_DYN_ALLOC_CFG_FLD_SW_ADDSUB_IDX_CNT          REG_FLD(16, 0)

#define WED_RX_BM_ADD_BASE_FLD_PTR                             REG_FLD(32, 0)

#define WED_RX_BM_ADD_PTR_FLD_HEAD2_IDX                        REG_FLD(16, 16)
#define WED_RX_BM_ADD_PTR_FLD_SW_ADD_IDX_REMAIN                REG_FLD(16, 0)

#define WED_RX_BM_TOTAL_DMAD_IDX_FLD_SW_TOTAL_RX_BM_CNT        REG_FLD(16, 0)

#define WED_RX_BM_ADD_BASE_H_FLD_PTR                           REG_FLD(8, 0)

#define RRO_IND_CMD_0_SIGNATURE_FLD_VLD                        REG_FLD(1, 31)
#define RRO_IND_CMD_0_SIGNATURE_FLD_MAGIC_CNT                  REG_FLD(3, 28)
#define RRO_IND_CMD_0_SIGNATURE_FLD_SW_PROC_IDX                REG_FLD(12, 16)
#define RRO_IND_CMD_0_SIGNATURE_FLD_DMA_IDX                    REG_FLD(12, 0)

#define IND_CMD_0_CTRL_0_FLD_INIT_MAGIC_CNT_PROC_IDX           REG_FLD(1, 31)
#define IND_CMD_0_CTRL_0_FLD_MAGIC_CNT                         REG_FLD(3, 28)
#define IND_CMD_0_CTRL_0_FLD_PREFETCH_IND_CMD_FREE_CNT         REG_FLD(4, 16)
#define IND_CMD_0_CTRL_0_FLD_PROC_IDX                          REG_FLD(12, 0)

#define IND_CMD_0_CTRL_1_FLD_RRO_IND_CMD_BASE_L                REG_FLD(28, 4)

#define IND_CMD_0_CTRL_2_FLD_RRO_IND_CMD_BASE_M                REG_FLD(4, 16)
#define IND_CMD_0_CTRL_2_FLD_MAX_CNT_11_4                      REG_FLD(8, 4)

#define RRO_CONF_0_FLD_ACK_SN_BASE_0_L                         REG_FLD(32, 0)

#define RRO_CONF_1_FLD_MAX_WIN_SZ                              REG_FLD(3, 29)
#define RRO_CONF_1_FLD_ACK_SN_BASE_0_M                         REG_FLD(4, 16)
#define RRO_CONF_1_FLD_PARTICULAR_SE_ID                        REG_FLD(12, 0)

#define ADDR_ELEM_CONF_0_FLD_PARTICULAR_SE_ID_ADDR_BASE_L      REG_FLD(32, 0)

#define ADDR_ELEM_CONF_1_FLD_CLR_ADDR_ELEM_AXI_OP              REG_FLD(2, 30)
#define ADDR_ELEM_CONF_1_FLD_PREFETCH_ADDR_ELEM_FREE_CNT       REG_FLD(4, 16)
#define ADDR_ELEM_CONF_1_FLD_CLR_ADDR_ELEM_WSTRB               REG_FLD(8, 8)
#define ADDR_ELEM_CONF_1_FLD_PARTICULAR_SE_ID_ADDR_BASE_M      REG_FLD(4, 0)

#define ADDR_ELEM_BASE_TBL_CONF_FLD_WR                         REG_FLD(1, 31)
#define ADDR_ELEM_BASE_TBL_CONF_FLD_RD                         REG_FLD(1, 30)
#define ADDR_ELEM_BASE_TBL_CONF_FLD_WR_RDY                     REG_FLD(1, 29)
#define ADDR_ELEM_BASE_TBL_CONF_FLD_RD_RDY                     REG_FLD(1, 28)
#define ADDR_ELEM_BASE_TBL_CONF_FLD_BASE_TBL_OFST              REG_FLD(7, 0)

#define ADDR_ELEM_BASE_TBL_WDATA_FLD_W_DATA                    REG_FLD(32, 0)

#define ADDR_ELEM_BASE_TBL_RDATA_FLD_R_DATA                    REG_FLD(32, 0)

#define PN_CONF_0_FLD_PN_WR                                    REG_FLD(1, 31)
#define PN_CONF_0_FLD_PN_RD                                    REG_FLD(1, 30)
#define PN_CONF_0_FLD_PN_WR_RDY                                REG_FLD(1, 29)
#define PN_CONF_0_FLD_PN_RD_RDY                                REG_FLD(1, 28)
#define PN_CONF_0_FLD_SE_ID                                    REG_FLD(12, 0)

#define PN_CONF_WDATA_L_FLD_PN_31_0                            REG_FLD(32, 0)

#define PN_CONF_WDATA_M_FLD_IS_FIRST                           REG_FLD(1, 17)
#define PN_CONF_WDATA_M_FLD_NEED_CHECK_PN                      REG_FLD(1, 16)
#define PN_CONF_WDATA_M_FLD_PN_47_32                           REG_FLD(16, 0)

#define PN_CONF_RDATA_L_FLD_PN_31_0                            REG_FLD(32, 0)

#define PN_CONF_RDATA_M_FLD_IS_FIRST                           REG_FLD(1, 17)
#define PN_CONF_RDATA_M_FLD_NEED_CHECK_PN                      REG_FLD(1, 16)
#define PN_CONF_RDATA_M_FLD_PN_47_32                           REG_FLD(16, 0)

#define RRO_MSDU_PG_RING0_CFG0_FLD_MSDU_PG_ADDR_R0_L           REG_FLD(32, 0)

#define RRO_MSDU_PG_RING0_CFG1_FLD_MSDU_PG_ADDR_R0_M           REG_FLD(4, 0)

#define RRO_MSDU_PG_RING1_CFG0_FLD_MSDU_PG_ADDR_R1_L           REG_FLD(32, 0)

#define RRO_MSDU_PG_RING1_CFG1_FLD_MSDU_PG_ADDR_R1_M           REG_FLD(4, 0)

#define RRO_MSDU_PG_RING2_CFG0_FLD_MSDU_PG_ADDR_R2_L           REG_FLD(32, 0)

#define RRO_MSDU_PG_RING2_CFG1_FLD_DRV_EN                      REG_FLD(1, 31)
#define RRO_MSDU_PG_RING2_CFG1_FLD_INT_PROC_DISABLE            REG_FLD(1, 30)
#define RRO_MSDU_PG_RING2_CFG1_FLD_FORCE_IDLE_WHEN_DRV_DISABLE REG_FLD(1, 29)
#define RRO_MSDU_PG_RING2_CFG1_FLD_FORCE_RETURN_IDLE           REG_FLD(1, 28)
#define RRO_MSDU_PG_RING2_CFG1_FLD_INIT_FLAG_CLR               REG_FLD(1, 27)
#define RRO_MSDU_PG_RING2_CFG1_FLD_DRV_CLR                     REG_FLD(1, 26)
#define RRO_MSDU_PG_RING2_CFG1_FLD_DRV_DBG_CLR                 REG_FLD(1, 25)
#define RRO_MSDU_PG_RING2_CFG1_FLD_DMAD_DBG_EN                 REG_FLD(1, 24)
#define RRO_MSDU_PG_RING2_CFG1_FLD_ERR_LAT_OPT                 REG_FLD(1, 23)
#define RRO_MSDU_PG_RING2_CFG1_FLD_BUSY_FLAG                   REG_FLD(1, 22)
#define RRO_MSDU_PG_RING2_CFG1_FLD_POLL_FLAG                   REG_FLD(1, 21)
#define RRO_MSDU_PG_RING2_CFG1_FLD_INIT_COMPLETE_FLAG          REG_FLD(1, 20)
#define RRO_MSDU_PG_RING2_CFG1_FLD_FORCE_IDLE_WHEN_DRV_ERROR   REG_FLD(1, 19)
#define RRO_MSDU_PG_RING2_CFG1_FLD_R2_INT_FLAG                 REG_FLD(1, 18)
#define RRO_MSDU_PG_RING2_CFG1_FLD_R1_INT_FLAG                 REG_FLD(1, 17)
#define RRO_MSDU_PG_RING2_CFG1_FLD_R0_INT_FLAG                 REG_FLD(1, 16)
#define RRO_MSDU_PG_RING2_CFG1_FLD_POLL_CYCLE                  REG_FLD(8, 8)
#define RRO_MSDU_PG_RING2_CFG1_FLD_MSDU_PG_ADDR_R2_M           REG_FLD(4, 0)

#define RRO_MSDU_PG_0_CTRL0_FLD_BASE_PTR_L                     REG_FLD(32, 0)

#define RRO_MSDU_PG_0_CTRL1_FLD_MAGIC_CNT                      REG_FLD(4, 28)
#define RRO_MSDU_PG_0_CTRL1_FLD_BASE_PTR_M                     REG_FLD(8, 16)
#define RRO_MSDU_PG_0_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define RRO_MSDU_PG_0_CTRL2_FLD_DMA_IDX_MIRO                   REG_FLD(12, 16)
#define RRO_MSDU_PG_0_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define RRO_MSDU_PG_1_CTRL0_FLD_BASE_PTR_L                     REG_FLD(32, 0)

#define RRO_MSDU_PG_1_CTRL1_FLD_MAGIC_CNT                      REG_FLD(4, 28)
#define RRO_MSDU_PG_1_CTRL1_FLD_BASE_PTR_M                     REG_FLD(8, 16)
#define RRO_MSDU_PG_1_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define RRO_MSDU_PG_1_CTRL2_FLD_DMA_IDX_MIRO                   REG_FLD(12, 16)
#define RRO_MSDU_PG_1_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define RRO_MSDU_PG_2_CTRL0_FLD_BASE_PTR_L                     REG_FLD(32, 0)

#define RRO_MSDU_PG_2_CTRL1_FLD_MAGIC_CNT                      REG_FLD(4, 28)
#define RRO_MSDU_PG_2_CTRL1_FLD_BASE_PTR_M                     REG_FLD(8, 16)
#define RRO_MSDU_PG_2_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define RRO_MSDU_PG_2_CTRL2_FLD_BURST_MODE_DISABLE             REG_FLD(1, 31)
#define RRO_MSDU_PG_2_CTRL2_FLD_DMA_IDX_MIRO                   REG_FLD(12, 16)
#define RRO_MSDU_PG_2_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define RRO_RX_D_RX0_BASE_FLD_PTR                              REG_FLD(32, 0)

#define RRO_RX_D_RX0_CNT_FLD_MAGIC_CNT                         REG_FLD(4, 28)
#define RRO_RX_D_RX0_CNT_FLD_PTR_H                             REG_FLD(8, 16)
#define RRO_RX_D_RX0_CNT_FLD_MAX                               REG_FLD(12, 0)

#define RRO_RX_D_RX0_CRX_IDX_FLD_DRV_IDX                       REG_FLD(12, 16)
#define RRO_RX_D_RX0_CRX_IDX_FLD_CRX_IDX                       REG_FLD(12, 0)

#define RRO_RX_D_RX0_DRX_IDX_FLD_DRX_IDX_MIRO                  REG_FLD(12, 0)

#define RRO_RX_D_RX1_BASE_FLD_PTR                              REG_FLD(32, 0)

#define RRO_RX_D_RX1_CNT_FLD_MAGIC_CNT                         REG_FLD(4, 28)
#define RRO_RX_D_RX1_CNT_FLD_PTR_H                             REG_FLD(8, 16)
#define RRO_RX_D_RX1_CNT_FLD_MAX                               REG_FLD(12, 0)

#define RRO_RX_D_RX1_CRX_IDX_FLD_DRV_IDX                       REG_FLD(12, 16)
#define RRO_RX_D_RX1_CRX_IDX_FLD_CRX_IDX                       REG_FLD(12, 0)

#define RRO_RX_D_RX1_DRX_IDX_FLD_DRX_IDX_MIRO                  REG_FLD(12, 0)

#define RRO_RX_D_RING_CFG_ADDR_0_FLD_ADDR_RRO_R0_L             REG_FLD(32, 0)

#define RRO_RX_D_RING_CFG_ADDR_1_FLD_ADDR_RRO_R1_L             REG_FLD(32, 0)

#define RRO_RX_D_RING_CFG_ADDR_2_FLD_DRV_EN                    REG_FLD(1, 31)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_INT_PROC_DISABLE          REG_FLD(1, 30)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_FORCE_IDLE_WHEN_DRV_DISABLE REG_FLD(1, 29)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_FORCE_RETURN_IDLE         REG_FLD(1, 28)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_INIT_FLAG_CLR             REG_FLD(1, 27)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_DRV_CLR                   REG_FLD(1, 26)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_DRV_DBG_CLR               REG_FLD(1, 25)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_DMAD_DBG_EN               REG_FLD(1, 24)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_ERR_LAT_OPT               REG_FLD(1, 23)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_BUSY_FLAG                 REG_FLD(1, 22)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_POLL_FLAG                 REG_FLD(1, 21)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_INIT_COMPLETE_FLAG        REG_FLD(1, 20)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_FORCE_IDLE_WHEN_DRV_ERROR REG_FLD(1, 19)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_BURST_MODE_DISABLE        REG_FLD(1, 18)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_R1_INT_FLAG               REG_FLD(1, 17)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_R0_INT_FLAG               REG_FLD(1, 16)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_POLL_CYCLE                REG_FLD(8, 8)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_ADDR_RRO_R1_M             REG_FLD(4, 4)
#define RRO_RX_D_RING_CFG_ADDR_2_FLD_ADDR_RRO_R0_M             REG_FLD(4, 0)

#define RRO_PG_BM_RX_DMAD_FLD_BASE_PTR_H                       REG_FLD(8, 16)
#define RRO_PG_BM_RX_DMAD_FLD_SDL0                             REG_FLD(14, 0)

#define RRO_PG_BM_BASE_FLD_PTR                                 REG_FLD(32, 0)

#define RRO_PG_BM_INIT_PTR_FLD_INIT_SW_TAIL_IDX                REG_FLD(1, 16)
#define RRO_PG_BM_INIT_PTR_FLD_SW_TAIL_IDX                     REG_FLD(16, 0)

#define RRO_PG_BM_PTR_FLD_HEAD_IDX                             REG_FLD(16, 16)
#define RRO_PG_BM_PTR_FLD_TAIL_IDX                             REG_FLD(16, 0)

#define RRO_PG_BM_ADD_BASE_H_FLD_PTR                           REG_FLD(8, 0)

#define RRO_PG_BM_STS_FLD_DMAD0_RD                             REG_FLD(1, 21)
#define RRO_PG_BM_STS_FLD_DMAD0_VLD                            REG_FLD(1, 20)
#define RRO_PG_BM_STS_FLD_DMAD0_NUM                            REG_FLD(2, 16)
#define RRO_PG_BM_STS_FLD_DMAD_NUM                             REG_FLD(5, 8)

#define RRO_PG_BM_INTF_FLD_DMAD_FREE_RD                        REG_FLD(1, 29)
#define RRO_PG_BM_INTF_FLD_DMAD_FREE_VLD                       REG_FLD(1, 28)
#define RRO_PG_BM_INTF_FLD_DMAD_FREE                           REG_FLD(16, 0)

#define RRO_PG_BM_ERR_STS_FLD_FREE_AT_EMPTY                    REG_FLD(1, 7)
#define RRO_PG_BM_ERR_STS_FLD_DMAD_AXI_RD_OVER_TAIL            REG_FLD(1, 3)
#define RRO_PG_BM_ERR_STS_FLD_DMAD_FULL_AXI_RD                 REG_FLD(1, 2)
#define RRO_PG_BM_ERR_STS_FLD_DMAD_AXI_RD_MORE                 REG_FLD(1, 1)
#define RRO_PG_BM_ERR_STS_FLD_DMAD_AXI_RD_LESS                 REG_FLD(1, 0)

#define RRO_PG_BM_OPT_CTRL_FLD_FREE_AXI_RESP_SEL               REG_FLD(1, 0)

#define RRO_PG_BM_DYN_ALLOC_TH_FLD_H_BUF_TH                    REG_FLD(16, 16)
#define RRO_PG_BM_DYN_ALLOC_TH_FLD_L_BUF_TH                    REG_FLD(16, 0)

#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_HW_SUB_pause               REG_FLD(1, 31)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_HW_ADDSUB_ACK              REG_FLD(1, 30)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_SW_ADD_OVERFLOW_ERR        REG_FLD(1, 29)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_SW_SUB_UNDERFLOW_ERR       REG_FLD(1, 28)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_SW_SUB_RDY                 REG_FLD(1, 18)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_SW_ADD_BUF_REQ             REG_FLD(1, 17)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_SW_SUB_BUF_REQ             REG_FLD(1, 16)
#define RRO_PG_BM_DYN_ALLOC_CFG_FLD_SW_ADDSUB_IDX_CNT          REG_FLD(16, 0)

#define RRO_PG_BM_ADD_BASE_FLD_PTR                             REG_FLD(32, 0)

#define RRO_PG_BM_ADD_PTR_FLD_HEAD2_IDX                        REG_FLD(16, 16)
#define RRO_PG_BM_ADD_PTR_FLD_SW_ADD_IDX_REMAIN                REG_FLD(16, 0)

#define RRO_PG_BM_TOTAL_DMAD_IDX_FLD_SW_TOTAL_RX_BM_CNT        REG_FLD(16, 0)

#define WED_WPDMA_INT_CTRL_RRO_RX_FLD_RRO_RX_DONE_TRIG1        REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_RRO_RX_FLD_RRO_RX_DONE_CLR1         REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_RRO_RX_FLD_RRO_RX_DONE_EN1          REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_RRO_RX_FLD_RRO_RX_DONE_TRIG0        REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_RRO_RX_FLD_RRO_RX_DONE_CLR0         REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_RRO_RX_FLD_RRO_RX_DONE_EN0          REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_TRIG2   REG_FLD(5, 18)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_CLR2    REG_FLD(1, 17)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_EN2     REG_FLD(1, 16)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_TRIG1   REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_CLR1    REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_EN1     REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_TRIG0   REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_CLR0    REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_FLD_RRO_PG_DONE_EN0     REG_FLD(1, 0)

#define WED_RRO_RX_HW_STS_FLD_PN_CHK_CS                        REG_FLD(3, 18)
#define WED_RRO_RX_HW_STS_FLD_MSDU_INFO_PG_CS                  REG_FLD(2, 16)
#define WED_RRO_RX_HW_STS_FLD_CLR_ADDR_ACKSN_CS                REG_FLD(3, 13)
#define WED_RRO_RX_HW_STS_FLD_IND2ADDR_ELEM_CS                 REG_FLD(3, 10)
#define WED_RRO_RX_HW_STS_FLD_FETCH_IND_CMD_CS                 REG_FLD(2, 8)
#define WED_RRO_RX_HW_STS_FLD_RX_PN_CHK_BUSY                   REG_FLD(1, 3)
#define WED_RRO_RX_HW_STS_FLD_RX_MSDU_INFO_PG_BUSY             REG_FLD(1, 2)
#define WED_RRO_RX_HW_STS_FLD_RX_ADDR_ELEM_BUSY                REG_FLD(1, 1)
#define WED_RRO_RX_HW_STS_FLD_RX_IND_CMD_BUSY                  REG_FLD(1, 0)

#define WED_RRO_RX_DBG0_FLD_rx_ind_cmd_vld                     REG_FLD(1, 31)
#define WED_RRO_RX_DBG0_FLD_rx_ind_cmd_rd                      REG_FLD(1, 30)
#define WED_RRO_RX_DBG0_FLD_rx_ind_cmd                         REG_FLD(6, 24)
#define WED_RRO_RX_DBG0_FLD_RX_PG_MEM_RA                       REG_FLD(4, 20)
#define WED_RRO_RX_DBG0_FLD_RX_PG_MEM_WA                       REG_FLD(4, 16)
#define WED_RRO_RX_DBG0_FLD_ADDR_ELEM_RA                       REG_FLD(4, 12)
#define WED_RRO_RX_DBG0_FLD_ADDR_ELEM_WA_B64                   REG_FLD(4, 8)
#define WED_RRO_RX_DBG0_FLD_IND_CMD_RA                         REG_FLD(4, 4)
#define WED_RRO_RX_DBG0_FLD_IND_CMD_WA_B64                     REG_FLD(4, 0)

#define WED_RRO_RX_DBG1_FLD_addr_elem_aw_err                   REG_FLD(1, 29)
#define WED_RRO_RX_DBG1_FLD_addr_elem_wr_align_err             REG_FLD(1, 28)
#define WED_RRO_RX_DBG1_FLD_acksn_last_cnt                     REG_FLD(12, 16)
#define WED_RRO_RX_DBG1_FLD_parti_addr_elem_vld                REG_FLD(1, 15)
#define WED_RRO_RX_DBG1_FLD_parti_addr_elem_rd                 REG_FLD(1, 14)
#define WED_RRO_RX_DBG1_FLD_parti_addr_elem                    REG_FLD(14, 0)

#define WED_RRO_RX_DBG2_FLD_rx_pg_dmad_free_vld                REG_FLD(1, 31)
#define WED_RRO_RX_DBG2_FLD_rx_pg_dmad_free_rd                 REG_FLD(1, 30)
#define WED_RRO_RX_DBG2_FLD_rx_pg_dmad_free                    REG_FLD(14, 16)
#define WED_RRO_RX_DBG2_FLD_hif_rxd_ext_vld                    REG_FLD(1, 15)
#define WED_RRO_RX_DBG2_FLD_hif_rxd_ext_rd                     REG_FLD(1, 14)
#define WED_RRO_RX_DBG2_FLD_hif_rxd_ext                        REG_FLD(14, 0)

#define WED_RRO_RX_DBG3_FLD_rx_msdu2rtqm_wr                    REG_FLD(1, 31)
#define WED_RRO_RX_DBG3_FLD_rx_msdu2rtqm_rdy                   REG_FLD(1, 30)
#define WED_RRO_RX_DBG3_FLD_rx_msdu2rtqm_wdata                 REG_FLD(14, 16)

#define WED_RRO_RX_DBG4_FLD_clr_addr_acksn_wdata               REG_FLD(32, 0)

#define WED_RX_IND_CMD_CNT0_FLD_dbg_cnt_en                     REG_FLD(1, 31)
#define WED_RX_IND_CMD_CNT0_FLD_all_dbg_cnt_rst                REG_FLD(1, 30)
#define WED_RX_IND_CMD_CNT0_FLD_dbg_cnt_rdclr_en               REG_FLD(1, 29)
#define WED_RX_IND_CMD_CNT0_FLD_total_ind_cmd_cnt              REG_FLD(28, 0)

#define WED_RX_IND_CMD_CNT1_FLD_ind_cmd_fetch1_cnt             REG_FLD(28, 0)

#define WED_RX_IND_CMD_CNT2_FLD_ind_cmd_fetch2_cnt             REG_FLD(28, 0)

#define WED_RX_IND_CMD_CNT3_FLD_ind_cmd_fetch3_cnt             REG_FLD(28, 0)

#define WED_RX_IND_CMD_CNT4_FLD_ind_cmd_fetch4_cnt             REG_FLD(28, 0)

#define WED_RX_IND_CMD_CNT5_FLD_ind_cmd_fetch5_cnt             REG_FLD(24, 0)

#define WED_RX_IND_CMD_CNT6_FLD_ind_cmd_fetch6_cnt             REG_FLD(24, 0)

#define WED_RX_IND_CMD_CNT7_FLD_ind_cmd_fetch7_cnt             REG_FLD(24, 0)

#define WED_RX_IND_CMD_CNT8_FLD_ind_cmd_fetch8_cnt             REG_FLD(24, 0)

#define WED_RX_IND_CMD_CNT9_FLD_magic_cnt_fail_cnt             REG_FLD(16, 0)

#define WED_RX_ADDR_ELEM_CNT0_FLD_total_addr_elem_cnt          REG_FLD(32, 0)

#define WED_RX_ADDR_ELEM_CNT1_FLD_total_1st_sig_fail_cnt       REG_FLD(16, 16)
#define WED_RX_ADDR_ELEM_CNT1_FLD_total_sig_fail_cnt           REG_FLD(16, 0)

#define WED_RX_ADDR_ELEM_CNT2_FLD_clr_addr_cnt                 REG_FLD(20, 0)

#define WED_RX_ADDR_ELEM_CNT3_FLD_acksn_cnt                    REG_FLD(28, 0)

#define WED_RX_ADDR_ELEM_CNT4_FLD_acksn_last_cnt               REG_FLD(28, 0)

#define WED_RX_MSDU_PG_CNT1_FLD_pg_cnt1_cnt                    REG_FLD(28, 0)

#define WED_RX_MSDU_PG_CNT2_FLD_pg_cnt2_cnt                    REG_FLD(24, 0)

#define WED_RX_MSDU_PG_CNT3_FLD_pg_cnt3_cnt                    REG_FLD(24, 0)

#define WED_RX_MSDU_PG_CNT4_FLD_pg_cnt4_cnt                    REG_FLD(24, 0)

#define WED_RX_MSDU_PG_CNT5_FLD_pg_cnt5_cnt                    REG_FLD(24, 0)

#define WED_RX_PN_CHK_CNT0_FLD_pn_chk_fail_cnt                 REG_FLD(16, 0)

#define WED_RX_IND_CMD_AXI0_FLD_araddr_reg                     REG_FLD(32, 0)

#define WED_RX_ADDR_ELEM_AXI0_FLD_araddr_reg                   REG_FLD(32, 0)

#define WED_RX_ADDR_ELEM_AXI1_FLD_awaddr_reg                   REG_FLD(32, 0)

#define WED_RX_ADDR_ELEM_AXI2_FLD_awaddr_clr_addr_lat          REG_FLD(32, 0)

#define WED_RX_ADDR_ELEM_AXI3_FLD_wdata0                       REG_FLD(32, 0)

#define WED_RX_ADDR_ELEM_AXI4_FLD_wdata1                       REG_FLD(32, 0)

#define WED_RX_MSDU_PG_AXI0_FLD_araddr_reg                     REG_FLD(32, 0)

#define DONT_USE_BEHIND_THIS_ADDR_FLD_value                    REG_FLD(32, 0)


































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































#ifdef __cplusplus
}
#endif

#endif // __wed0_REGS_H__
