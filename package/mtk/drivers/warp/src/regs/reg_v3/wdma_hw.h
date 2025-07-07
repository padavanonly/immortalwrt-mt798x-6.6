#ifndef __WDMA0_v3_REGS_H__
#define __WDMA0_v3_REGS_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef REG_BASE_C_MODULE
// ----------------- WDMA0_v3 Bit Field Definitions -------------------

#define REG_FLD(width, shift) (shift)
#define PACKING
typedef unsigned int UINT32;
typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_BASE_PTR               : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_BASE_PTR_0, *PREG_DMA_TX_BASE_PTR_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_MAX_CNT                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_MAX_CNT_0, *PREG_DMA_TX_MAX_CNT_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_CTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_CTX_IDX_0, *PREG_DMA_TX_CTX_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DTX_IDX_0, *PREG_DMA_TX_DTX_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_BASE_PTR               : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_BASE_PTR_1, *PREG_DMA_TX_BASE_PTR_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_MAX_CNT                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_MAX_CNT_1, *PREG_DMA_TX_MAX_CNT_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_CTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_CTX_IDX_1, *PREG_DMA_TX_CTX_IDX_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DTX_IDX_1, *PREG_DMA_TX_DTX_IDX_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_BASE_PTR               : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_BASE_PTR_2, *PREG_DMA_TX_BASE_PTR_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_MAX_CNT                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_MAX_CNT_2, *PREG_DMA_TX_MAX_CNT_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_CTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_CTX_IDX_2, *PREG_DMA_TX_CTX_IDX_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DTX_IDX_2, *PREG_DMA_TX_DTX_IDX_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_BASE_PTR               : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_BASE_PTR_3, *PREG_DMA_TX_BASE_PTR_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_MAX_CNT                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_MAX_CNT_3, *PREG_DMA_TX_MAX_CNT_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_CTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_CTX_IDX_3, *PREG_DMA_TX_CTX_IDX_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DTX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DTX_IDX_3, *PREG_DMA_TX_DTX_IDX_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_BASE_PTR               : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_BASE_PTR_0, *PREG_DMA_RX_BASE_PTR_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_MAX_CNT                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MAX_CNT_0, *PREG_DMA_RX_MAX_CNT_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_CRX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_CRX_IDX_0, *PREG_DMA_RX_CRX_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DRX_IDX_0, *PREG_DMA_RX_DRX_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_BASE_PTR               : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_BASE_PTR_1, *PREG_DMA_RX_BASE_PTR_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_MAX_CNT                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MAX_CNT_1, *PREG_DMA_RX_MAX_CNT_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_CRX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_CRX_IDX_1, *PREG_DMA_RX_CRX_IDX_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_DRX_IDX                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DRX_IDX_1, *PREG_DMA_RX_DRX_IDX_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_RING_NUM               : 8;
        FIELD RX_RING_NUM               : 8;
        FIELD BASE_PTR_WIDTH            : 8;
        FIELD INDEX_WIDTH               : 4;
        FIELD DMA_REVISION              : 4;
    } Bits;
    UINT32 Raw;
} REG_DMA_INFO, *PREG_DMA_INFO;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DMA_EN                 : 1;
        FIELD TX_DMA_BUSY               : 1;
        FIELD RX_DMA_EN                 : 1;
        FIELD RX_DMA_BUSY               : 1;
        FIELD TX_BURST_4KB_BND_EN       : 1;
        FIELD RX_BURST_4KB_BND_EN       : 1;
        FIELD TX_WB_DDONE               : 1;
        FIELD PAYLOAD_BYTE_SWAP         : 1;
        FIELD rsv_8                     : 1;
        FIELD TX_CHK_DDONE              : 1;
        FIELD TX_SCH_RST                : 1;
        FIELD DMA_BT_SIZE               : 3;
        FIELD OTSD_THRES                : 4;
        FIELD CDM_FCNT_THRES            : 4;
        FIELD rsv_22                    : 2;
        FIELD LB_MODE                   : 1;
        FIELD PAYLOAD_BYTE_SWAP_SEL     : 1;
        FIELD DMAD_BYTE_SWAP_SEL        : 1;
        FIELD PKT_WCOMP                 : 1;
        FIELD DEC_WCOMP                 : 1;
        FIELD DMAD_BYTE_SWAP            : 1;
        FIELD rsv_30                    : 1;
        FIELD RX_2B_OFFSET              : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_GLO_CFG0, *PREG_DMA_GLO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RST_DTX_IDX0              : 1;
        FIELD RST_DTX_IDX1              : 1;
        FIELD RST_DTX_IDX2              : 1;
        FIELD RST_DTX_IDX3              : 1;
        FIELD rsv_4                     : 12;
        FIELD RST_DRX_IDX0              : 1;
        FIELD RST_DRX_IDX1              : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_DMA_RST_IDX, *PREG_DMA_RST_IDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FREEQ_THRES               : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_DMA_FREEQ_THRES, *PREG_DMA_FREEQ_THRES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STATUS, *PREG_DMA_INT_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_MASK, *PREG_DMA_INT_MASK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_AXI_BURST_WR_ERR_RESP  : 1;
        FIELD TX_AXI_BURST_RD_ERR_RESP  : 1;
        FIELD TX_AXI_PREF_WR_ERR_RESP   : 1;
        FIELD TX_AXI_PREF_RD_ERR_RESP   : 1;
        FIELD TX_AXI_WRBK_WR_ERR_RESP   : 1;
        FIELD TX_AXI_WRBK_RD_ERR_RESP   : 1;
        FIELD rsv_6                     : 2;
        FIELD RX_AXI_BURST_WR_ERR_RESP  : 1;
        FIELD RX_AXI_BURST_RD_ERR_RESP  : 1;
        FIELD RX_AXI_PREF_WR_ERR_RESP   : 1;
        FIELD RX_AXI_PREF_RD_ERR_RESP   : 1;
        FIELD RX_AXI_WRBK_WR_ERR_RESP   : 1;
        FIELD RX_AXI_WRBK_RD_ERR_RESP   : 1;
        FIELD RX_CMD_EOF_ERR_RESP       : 1;
        FIELD RX_CDM_EOF_ERR_RESP       : 1;
        FIELD TX_PREF_RD_UD_ERR_RESP    : 1;
        FIELD TX_PREF_WR_OF_ERR_RESP    : 1;
        FIELD TX_WRBK_RD_UD_ERR_RESP    : 1;
        FIELD TX_WRBK_WR_OF_ERR_RESP    : 1;
        FIELD RX_PREF_RD_UD_ERR_RESP    : 1;
        FIELD RX_PREF_WR_OF_ERR_RESP    : 1;
        FIELD RX_WRBK_RD_UD_ERR_RESP    : 1;
        FIELD RX_WRBK_WR_OF_ERR_RESP    : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DMA_ERR_INT_STATUS, *PREG_DMA_ERR_INT_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_BST_THRES              : 4;
        FIELD RX_BST_THRES              : 4;
        FIELD TX_SEG1_EN                : 1;
        FIELD RX_SCATHER_BYPASS         : 1;
        FIELD RX_JBP_ID_EN              : 1;
        FIELD RX_MERGE_ID_EN            : 1;
        FIELD rsv_12                    : 1;
        FIELD TX_PREF_CHK_DDONE2_EN     : 1;
        FIELD TX_PREF_CHK_DDONE2_BUSY   : 1;
        FIELD TX_PREF_CHK_DDONE2_CLR    : 1;
        FIELD TX_PREF_CHK_DDONE2_METHOD_CLR : 1;
        FIELD RX_PREF_CHK_DDONE2_EN     : 1;
        FIELD RX_PREF_CHK_DDONE2_BUSY   : 1;
        FIELD RX_PREF_CHK_DDONE2_CLR    : 1;
        FIELD RX_PREF_CHK_DDONE2_METHOD_CLR : 1;
        FIELD PREF_CHK_DDONE_DW0        : 1;
        FIELD PREF_CHK_DDONE_DW1        : 1;
        FIELD PREF_CHK_DDONE_DW2        : 1;
        FIELD PREF_CHK_DDONE_DW3        : 1;
        FIELD PREF_CHK_DDONE_POL        : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_GLO_CFG1, *PREG_DMA_GLO_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_PAR_FIFO_CLEAR         : 1;
        FIELD TX_PAR_FIFO_EMPTY         : 1;
        FIELD TX_PAR_FIFO_FULL          : 1;
        FIELD rsv_3                     : 1;
        FIELD TX_CMD_FIFO_CLEAR         : 1;
        FIELD TX_CMD_FIFO_EMPTY         : 1;
        FIELD TX_CMD_FIFO_FULL          : 1;
        FIELD rsv_7                     : 1;
        FIELD TX_DMAD_FIFO_CLEAR        : 1;
        FIELD TX_DMAD_FIFO_EMPTY        : 1;
        FIELD TX_DMAD_FIFO_FULL         : 1;
        FIELD rsv_11                    : 1;
        FIELD TX_ARR_FIFO_CLEAR         : 1;
        FIELD TX_ARR_FIFO_EMPTY         : 1;
        FIELD TX_ARR_FIFO_FULL          : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_XDMA_FIFO_CFG0, *PREG_DMA_TX_XDMA_FIFO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RX_PAR_FIFO_CLEAR         : 1;
        FIELD RX_PAR_FIFO_EMPTY         : 1;
        FIELD RX_PAR_FIFO_FULL          : 1;
        FIELD rsv_3                     : 1;
        FIELD RX_CMD_FIFO_CLEAR         : 1;
        FIELD RX_CMD_FIFO_EMPTY         : 1;
        FIELD RX_CMD_FIFO_FULL          : 1;
        FIELD rsv_7                     : 1;
        FIELD RX_DMAD_FIFO_CLEAR        : 1;
        FIELD RX_DMAD_FIFO_EMPTY        : 1;
        FIELD RX_DMAD_FIFO_FULL         : 1;
        FIELD rsv_11                    : 1;
        FIELD RX_ARR_FIFO_CLEAR         : 1;
        FIELD RX_ARR_FIFO_EMPTY         : 1;
        FIELD RX_ARR_FIFO_FULL          : 1;
        FIELD RX_LEN_FIFO_CLEAR         : 1;
        FIELD RX_LEN_FIFO_EMPTY         : 1;
        FIELD RX_LEN_FIFO_FULL          : 1;
        FIELD RX_WID_FIFO_CLEAR         : 1;
        FIELD RX_WID_FIFO_EMPTY         : 1;
        FIELD RX_WID_FIFO_FULL          : 1;
        FIELD RX_BID_FIFO_CLEAR         : 1;
        FIELD RX_BID_FIFO_EMPTY         : 1;
        FIELD RX_BID_FIFO_FULL          : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_XDMA_FIFO_CFG0, *PREG_DMA_RX_XDMA_FIFO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_STS_GRP0          : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STS_GRP0, *PREG_DMA_INT_STS_GRP0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_STS_GRP1          : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STS_GRP1, *PREG_DMA_INT_STS_GRP1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_STS_GRP2          : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STS_GRP2, *PREG_DMA_INT_STS_GRP2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_STS_GRP3          : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STS_GRP3, *PREG_DMA_INT_STS_GRP3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_GRP1              : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_GRP1, *PREG_DMA_INT_GRP1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_GRP2              : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_GRP2, *PREG_DMA_INT_GRP2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DMA_INT_GRP3              : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_GRP3, *PREG_DMA_INT_GRP3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD AXI_CG_DISABLE            : 1;
        FIELD rsv_1                     : 1;
        FIELD AXI_QOS_ON                : 1;
        FIELD AXI_OUTSTANDING_EXTEND    : 1;
        FIELD AXI_ERRMID_SET_BIRQ       : 1;
        FIELD AXI_ERRMID_SET_RIRQ       : 1;
        FIELD AXI_LOCK_ERROR            : 1;
        FIELD rsv_7                     : 1;
        FIELD AXI_W_BUSY                : 1;
        FIELD AXI_R_BUSY                : 1;
        FIELD AXI_CTRL_UPDATED          : 1;
        FIELD rsv_11                    : 1;
        FIELD AXI_ULTRA_TXDMA           : 2;
        FIELD AXI_ULTRA_RXDMA           : 2;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_BUS_CFG, *PREG_DMA_BUS_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD AXI_PREULTRA_THRES        : 11;
        FIELD rsv_11                    : 4;
        FIELD AXI_PREULTRA_EN           : 1;
        FIELD AXI_ULTRA_THRES           : 11;
        FIELD rsv_27                    : 4;
        FIELD AXI_ULTRA_EN              : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_ULTRA_CFG, *PREG_DMA_ULTRA_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_PKT_NUM               : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_XFER_CNT_CFG1, *PREG_DMA_XFER_CNT_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SDL                       : 16;
        FIELD SDL_EN                    : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_DMA_SDL_CFG, *PREG_DMA_SDL_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD IDLE_MASK                 : 3;
        FIELD rsv_3                     : 1;
        FIELD AXI_IDLE                  : 1;
        FIELD DMA_IDLE                  : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_DMA_IDLE_MASK, *PREG_DMA_IDLE_MASK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_RATE0                 : 10;
        FIELD MIN_RATE_RATIO0           : 2;
        FIELD MAX_WEIGHT0               : 2;
        FIELD MAX_RATE_ULMT0            : 1;
        FIELD MAX_BKT_SIZE0             : 1;
        FIELD MAX_RATE1                 : 10;
        FIELD MIN_RATE_RATIO1           : 2;
        FIELD MAX_WEIGHT1               : 2;
        FIELD MAX_RATE_ULMT1            : 1;
        FIELD MAX_BKT_SIZE1             : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_SCH_Q01_CFG, *PREG_DMA_SCH_Q01_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MAX_RATE2                 : 10;
        FIELD MIN_RATE_RATIO2           : 2;
        FIELD MAX_WEIGHT2               : 2;
        FIELD MAX_RATE_ULMT2            : 1;
        FIELD MAX_BKT_SIZE2             : 1;
        FIELD MAX_RATE3                 : 10;
        FIELD MIN_RATE_RATIO3           : 2;
        FIELD MAX_WEIGHT3               : 2;
        FIELD MAX_RATE_ULMT3            : 1;
        FIELD MAX_BKT_SIZE3             : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_SCH_Q23_CFG, *PREG_DMA_SCH_Q23_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STATUS_0, *PREG_DMA_INT_STATUS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_MASK_0, *PREG_DMA_INT_MASK_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STATUS_1, *PREG_DMA_INT_STATUS_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_MASK_1, *PREG_DMA_INT_MASK_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STATUS_2, *PREG_DMA_INT_STATUS_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_MASK_2, *PREG_DMA_INT_MASK_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_STATUS_3, *PREG_DMA_INT_STATUS_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_DONE_INT0              : 1;
        FIELD TX_DONE_INT1              : 1;
        FIELD TX_DONE_INT2              : 1;
        FIELD TX_DONE_INT3              : 1;
        FIELD rsv_4                     : 4;
        FIELD TX_DONE_DLY_INT0          : 1;
        FIELD TX_DONE_DLY_INT1          : 1;
        FIELD TX_DONE_DLY_INT2          : 1;
        FIELD TX_DONE_DLY_INT3          : 1;
        FIELD rsv_12                    : 4;
        FIELD RX_DONE_INT0              : 1;
        FIELD RX_DONE_INT1              : 1;
        FIELD rsv_18                    : 2;
        FIELD RX_DONE_DLY_INT0          : 1;
        FIELD RX_DONE_DLY_INT1          : 1;
        FIELD rsv_22                    : 6;
        FIELD TX_DLY_INT                : 1;
        FIELD TX_COHERENT               : 1;
        FIELD RX_DLY_INT                : 1;
        FIELD RX_COHERENT               : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_INT_MASK_3, *PREG_DMA_INT_MASK_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_TXMAX_PTIME         : 8;
        FIELD RING0_TXMAX_PINT          : 7;
        FIELD RING0_TXDLY_INT_EN        : 1;
        FIELD RING1_TXMAX_PTIME         : 8;
        FIELD RING1_TXMAX_PINT          : 7;
        FIELD RING1_TXDLY_INT_EN        : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DELAY_INT_CFG_0, *PREG_DMA_TX_DELAY_INT_CFG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING2_TXMAX_PTIME         : 8;
        FIELD RING2_TXMAX_PINT          : 7;
        FIELD RING2_TXDLY_INT_EN        : 1;
        FIELD RING3_TXMAX_PTIME         : 8;
        FIELD RING3_TXMAX_PINT          : 7;
        FIELD RING3_TXDLY_INT_EN        : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DELAY_INT_CFG_1, *PREG_DMA_TX_DELAY_INT_CFG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_RXMAX_PTIME         : 8;
        FIELD RING0_RXMAX_PINT          : 7;
        FIELD RING0_RXDLY_INT_EN        : 1;
        FIELD RING1_RXMAX_PTIME         : 8;
        FIELD RING1_RXMAX_PINT          : 7;
        FIELD RING1_RXDLY_INT_EN        : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DELAY_INT_CFG_0, *PREG_DMA_RX_DELAY_INT_CFG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_FC_ASRT_THRES       : 12;
        FIELD rsv_12                    : 4;
        FIELD RING0_FC_DASRT_THRES      : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_FC_CFG_0, *PREG_DMA_RX_FC_CFG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING1_FC_ASRT_THRES       : 12;
        FIELD rsv_12                    : 4;
        FIELD RING1_FC_DASRT_THRES      : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_FC_CFG_1, *PREG_DMA_RX_FC_CFG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PREF_EN                   : 1;
        FIELD BUSY                      : 1;
        FIELD DMAD_SIZE                 : 1;
        FIELD DDONE_CHK                 : 1;
        FIELD DDONE_POLARITY            : 1;
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD AXI_RRESP_ERR             : 1;
        FIELD BURST_SIZE                : 5;
        FIELD CURR_STATE                : 3;
        FIELD LOW_THRES                 : 6;
        FIELD AXI_ULTRA                 : 2;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DMA_PREF_TX_CFG, *PREG_DMA_PREF_TX_CFG;

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
} REG_DMA_PREF_TX_FIFO_CFG0, *PREG_DMA_PREF_TX_FIFO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING2_CLEAR               : 1;
        FIELD RING2_FULL                : 1;
        FIELD RING2_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING2_USED_CNT            : 6;
        FIELD RING2_FREE_CNT            : 6;
        FIELD RING3_CLEAR               : 1;
        FIELD RING3_FULL                : 1;
        FIELD RING3_EMPTY               : 1;
        FIELD rsv_19                    : 1;
        FIELD RING3_USED_CNT            : 6;
        FIELD RING3_FREE_CNT            : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_PREF_TX_FIFO_CFG1, *PREG_DMA_PREF_TX_FIFO_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD PREF_EN                   : 1;
        FIELD BUSY                      : 1;
        FIELD DMAD_SIZE                 : 1;
        FIELD DDONE_CHK                 : 1;
        FIELD DDONE_POLARITY            : 1;
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD AXI_RRESP_ERR             : 1;
        FIELD BURST_SIZE                : 5;
        FIELD CURR_STATE                : 3;
        FIELD LOW_THRES                 : 6;
        FIELD AXI_ULTRA                 : 2;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DMA_PREF_RX_CFG, *PREG_DMA_PREF_RX_CFG;

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
} REG_DMA_PREF_RX_FIFO_CFG0, *PREG_DMA_PREF_RX_FIFO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_RING0_SIDX_CLR         : 1;
        FIELD TX_RING1_SIDX_CLR         : 1;
        FIELD TX_RING2_SIDX_CLR         : 1;
        FIELD TX_RING3_SIDX_CLR         : 1;
        FIELD RX_RING0_SIDX_CLR         : 1;
        FIELD RX_RING1_SIDX_CLR         : 1;
        FIELD rsv_6                     : 2;
        FIELD TX_RING0_SIDX_OW          : 1;
        FIELD TX_RING1_SIDX_OW          : 1;
        FIELD TX_RING2_SIDX_OW          : 1;
        FIELD TX_RING3_SIDX_OW          : 1;
        FIELD RX_RING0_SIDX_OW          : 1;
        FIELD RX_RING1_SIDX_OW          : 1;
        FIELD rsv_14                    : 2;
        FIELD MON_SEL                   : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} REG_DMA_PREF_SIDX_CFG, *PREG_DMA_PREF_SIDX_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX                 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_PREF_SIDX_MON, *PREG_DMA_PREF_SIDX_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX_OW_VAL          : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_PREF_SIDX_OW, *PREG_DMA_PREF_SIDX_OW;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 4;
        FIELD BYTE_CNT_UNIT             : 3;
        FIELD CLR_CTRL                  : 1;
        FIELD BYTE_CNT_SAT              : 1;
        FIELD PKT_CNT_SAT               : 1;
        FIELD rsv_10                    : 18;
        FIELD MON_SEL                   : 4;
    } Bits;
    UINT32 Raw;
} REG_DMA_XFER_CNT_CFG, *PREG_DMA_XFER_CNT_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MON                       : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_XFER_CNT_MON0, *PREG_DMA_XFER_CNT_MON0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD MON                       : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_XFER_CNT_MON1, *PREG_DMA_XFER_CNT_MON1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD AXI_PREULTRA_THRES        : 11;
        FIELD rsv_11                    : 4;
        FIELD AXI_PREULTRA_EN           : 1;
        FIELD AXI_ULTRA_THRES           : 11;
        FIELD rsv_27                    : 4;
        FIELD AXI_ULTRA_EN              : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_ULTRA_CFG, *PREG_DMA_RX_ULTRA_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BUSY                      : 1;
        FIELD DMAD_SIZE                 : 1;
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD AXI_ULTRA                 : 2;
        FIELD BURST_SIZE                : 5;
        FIELD CURR_STATE                : 3;
        FIELD WRBK_THRES                : 6;
        FIELD rsv_20                    : 1;
        FIELD FLUSH_TIMER_EN            : 1;
        FIELD MAX_PENDING_TIME          : 8;
        FIELD WRBK_EN                   : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_TX_CFG, *PREG_DMA_WRBK_TX_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_CLEAR               : 1;
        FIELD RING0_FULL                : 1;
        FIELD RING0_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING0_USED_CNT            : 6;
        FIELD rsv_10                    : 10;
        FIELD RING0_FREE_CNT            : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_TX_FIFO_CFG0, *PREG_DMA_WRBK_TX_FIFO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING1_CLEAR               : 1;
        FIELD RING1_FULL                : 1;
        FIELD RING1_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING1_USED_CNT            : 6;
        FIELD rsv_10                    : 10;
        FIELD RING1_FREE_CNT            : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_TX_FIFO_CFG1, *PREG_DMA_WRBK_TX_FIFO_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING2_CLEAR               : 1;
        FIELD RING2_FULL                : 1;
        FIELD RING2_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING2_USED_CNT            : 6;
        FIELD rsv_10                    : 10;
        FIELD RING2_FREE_CNT            : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_TX_FIFO_CFG2, *PREG_DMA_WRBK_TX_FIFO_CFG2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING3_CLEAR               : 1;
        FIELD RING3_FULL                : 1;
        FIELD RING3_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING3_USED_CNT            : 6;
        FIELD rsv_10                    : 10;
        FIELD RING3_FREE_CNT            : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_TX_FIFO_CFG3, *PREG_DMA_WRBK_TX_FIFO_CFG3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BUSY                      : 1;
        FIELD DMAD_SIZE                 : 1;
        FIELD WR_BND_4KB_BST            : 1;
        FIELD RD_BND_4KB_BST            : 1;
        FIELD AXI_ULTRA                 : 2;
        FIELD BURST_SIZE                : 5;
        FIELD CURR_STATE                : 3;
        FIELD WRBK_THRES                : 6;
        FIELD rsv_20                    : 1;
        FIELD FLUSH_TIMER_EN            : 1;
        FIELD MAX_PENDING_TIME          : 8;
        FIELD WRBK_EN                   : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_RX_CFG, *PREG_DMA_WRBK_RX_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING0_CLEAR               : 1;
        FIELD RING0_FULL                : 1;
        FIELD RING0_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING0_USED_CNT            : 6;
        FIELD rsv_10                    : 10;
        FIELD RING0_FREE_CNT            : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_RX_FIFO_CFG0, *PREG_DMA_WRBK_RX_FIFO_CFG0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RING1_CLEAR               : 1;
        FIELD RING1_FULL                : 1;
        FIELD RING1_EMPTY               : 1;
        FIELD rsv_3                     : 1;
        FIELD RING1_USED_CNT            : 6;
        FIELD rsv_10                    : 10;
        FIELD RING1_FREE_CNT            : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_RX_FIFO_CFG1, *PREG_DMA_WRBK_RX_FIFO_CFG1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TX_RING0_SIDX_CLR         : 1;
        FIELD TX_RING1_SIDX_CLR         : 1;
        FIELD TX_RING2_SIDX_CLR         : 1;
        FIELD TX_RING3_SIDX_CLR         : 1;
        FIELD RX_RING0_SIDX_CLR         : 1;
        FIELD RX_RING1_SIDX_CLR         : 1;
        FIELD rsv_6                     : 2;
        FIELD TX_RING0_SIDX_OW          : 1;
        FIELD TX_RING1_SIDX_OW          : 1;
        FIELD TX_RING2_SIDX_OW          : 1;
        FIELD TX_RING3_SIDX_OW          : 1;
        FIELD RX_RING0_SIDX_OW          : 1;
        FIELD RX_RING1_SIDX_OW          : 1;
        FIELD rsv_14                    : 2;
        FIELD MON_SEL                   : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_SIDX_CFG, *PREG_DMA_WRBK_SIDX_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX                 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_SIDX_MON, *PREG_DMA_WRBK_SIDX_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD START_IDX_OW_VAL          : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_WRBK_SIDX_OW, *PREG_DMA_WRBK_SIDX_OW;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_sig_sel             : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_DMA_DBG_CFG, *PREG_DMA_DBG_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_0           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_0, *PREG_DMA_TX_DBG_MON_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_1           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_1, *PREG_DMA_TX_DBG_MON_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_2           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_2, *PREG_DMA_TX_DBG_MON_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_3           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_3, *PREG_DMA_TX_DBG_MON_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_4           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_4, *PREG_DMA_TX_DBG_MON_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_5           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_5, *PREG_DMA_TX_DBG_MON_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_6           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_6, *PREG_DMA_TX_DBG_MON_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_7           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_7, *PREG_DMA_TX_DBG_MON_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_8           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_TX_DBG_MON_8, *PREG_DMA_TX_DBG_MON_8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_0           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_0, *PREG_DMA_RX_DBG_MON_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_1           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_1, *PREG_DMA_RX_DBG_MON_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_2           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_2, *PREG_DMA_RX_DBG_MON_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_3           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_3, *PREG_DMA_RX_DBG_MON_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_4           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_4, *PREG_DMA_RX_DBG_MON_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_5           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_5, *PREG_DMA_RX_DBG_MON_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_6           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_6, *PREG_DMA_RX_DBG_MON_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_7           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_7, *PREG_DMA_RX_DBG_MON_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD debug_monitor_8           : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_DBG_MON_8, *PREG_DMA_RX_DBG_MON_8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ENABLE                    : 1;
        FIELD BUSY                      : 1;
        FIELD rsv_2                     : 2;
        FIELD EN_BLOCK_RESV             : 1;
        FIELD EN_BLOCK_FREE             : 1;
        FIELD EN_DATA_WR                : 1;
        FIELD EN_DATA_RD                : 1;
        FIELD RREADY_ALWAYS_RDY         : 1;
        FIELD rsv_9                     : 3;
        FIELD AXI_RRESP_ERR             : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_MON_SEL               : 4;
        FIELD DBG_MON_SEL_FLAG          : 4;
        FIELD DBG_MON_SEL_DATA_CTRL     : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_CFG, *PREG_DMA_RX_MULTI_ID_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD BLOCK_FULL                : 1;
        FIELD BLOCK_EMPTY               : 1;
        FIELD BLOCK_OVERFLOW            : 1;
        FIELD BLOCK_UNDERFLOW           : 1;
        FIELD rsv_4                     : 4;
        FIELD BLOCK_HEAD_IDX            : 4;
        FIELD BLOCK_TAIL_IDX            : 4;
        FIELD BLOCK_HEAD_IDX_CLR        : 1;
        FIELD BLOCK_HEAD_IDX_INC        : 1;
        FIELD BLOCK_HEAD_IDX_OW         : 1;
        FIELD rsv_19                    : 1;
        FIELD BLOCK_HEAD_IDX_OW_VAL     : 4;
        FIELD BLOCK_TAIL_IDX_CLR        : 1;
        FIELD BLOCK_TAIL_IDX_INC        : 1;
        FIELD BLOCK_TAIL_IDX_OW         : 1;
        FIELD rsv_27                    : 1;
        FIELD BLOCK_TAIL_IDX_OW_VAL     : 4;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_BLK_STS, *PREG_DMA_RX_MULTI_ID_BLK_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA_FULL                 : 16;
        FIELD DATA_EMPTY                : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_DATA_STS0, *PREG_DMA_RX_MULTI_ID_DATA_STS0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA0_OV                  : 1;
        FIELD DATA1_OV                  : 1;
        FIELD DATA2_OV                  : 1;
        FIELD DATA3_OV                  : 1;
        FIELD DATA4_OV                  : 1;
        FIELD DATA5_OV                  : 1;
        FIELD DATA6_OV                  : 1;
        FIELD DATA7_OV                  : 1;
        FIELD DATA8_OV                  : 1;
        FIELD DATA9_OV                  : 1;
        FIELD DATA10_OV                 : 1;
        FIELD DATA11_OV                 : 1;
        FIELD DATA12_OV                 : 1;
        FIELD DATA13_OV                 : 1;
        FIELD DATA14_OV                 : 1;
        FIELD DATA15_OV                 : 1;
        FIELD DATA0_UD                  : 1;
        FIELD DATA1_UD                  : 1;
        FIELD DATA2_UD                  : 1;
        FIELD DATA3_UD                  : 1;
        FIELD DATA4_UD                  : 1;
        FIELD DATA5_UD                  : 1;
        FIELD DATA6_UD                  : 1;
        FIELD DATA7_UD                  : 1;
        FIELD DATA8_UD                  : 1;
        FIELD DATA9_UD                  : 1;
        FIELD DATA10_UD                 : 1;
        FIELD DATA11_UD                 : 1;
        FIELD DATA12_UD                 : 1;
        FIELD DATA13_UD                 : 1;
        FIELD DATA14_UD                 : 1;
        FIELD DATA15_UD                 : 1;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_DATA_STS1, *PREG_DMA_RX_MULTI_ID_DATA_STS1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATA_HEAD_IDX_CLR         : 16;
        FIELD DATA_TAIL_IDX_CLR         : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_DATA_STS2, *PREG_DMA_RX_MULTI_ID_DATA_STS2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD IDX_OW                    : 1;
        FIELD IDX_INC                   : 1;
        FIELD rsv_2                     : 2;
        FIELD IDX_OW_VAL                : 5;
        FIELD rsv_9                     : 7;
        FIELD FLAG_DLAST_CLR            : 16;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_DATA_STS3, *PREG_DMA_RX_MULTI_ID_DATA_STS3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DBG_MON                   : 32;
    } Bits;
    UINT32 Raw;
} REG_DMA_RX_MULTI_ID_DBG_MON, *PREG_DMA_RX_MULTI_ID_DBG_MON;

// ----------------- WDMA0_v3  Grouping Definitions -------------------
// ----------------- WDMA0_v3 Register Definition -------------------
typedef volatile PACKING struct
{
    REG_DMA_TX_BASE_PTR_0           DMA_TX_BASE_PTR_0; // 4800
    REG_DMA_TX_MAX_CNT_0            DMA_TX_MAX_CNT_0; // 4804
    REG_DMA_TX_CTX_IDX_0            DMA_TX_CTX_IDX_0; // 4808
    REG_DMA_TX_DTX_IDX_0            DMA_TX_DTX_IDX_0; // 480C
    REG_DMA_TX_BASE_PTR_1           DMA_TX_BASE_PTR_1; // 4810
    REG_DMA_TX_MAX_CNT_1            DMA_TX_MAX_CNT_1; // 4814
    REG_DMA_TX_CTX_IDX_1            DMA_TX_CTX_IDX_1; // 4818
    REG_DMA_TX_DTX_IDX_1            DMA_TX_DTX_IDX_1; // 481C
    REG_DMA_TX_BASE_PTR_2           DMA_TX_BASE_PTR_2; // 4820
    REG_DMA_TX_MAX_CNT_2            DMA_TX_MAX_CNT_2; // 4824
    REG_DMA_TX_CTX_IDX_2            DMA_TX_CTX_IDX_2; // 4828
    REG_DMA_TX_DTX_IDX_2            DMA_TX_DTX_IDX_2; // 482C
    REG_DMA_TX_BASE_PTR_3           DMA_TX_BASE_PTR_3; // 4830
    REG_DMA_TX_MAX_CNT_3            DMA_TX_MAX_CNT_3; // 4834
    REG_DMA_TX_CTX_IDX_3            DMA_TX_CTX_IDX_3; // 4838
    REG_DMA_TX_DTX_IDX_3            DMA_TX_DTX_IDX_3; // 483C
    UINT32                          rsv_4840[48];     // 4840..48FC
    REG_DMA_RX_BASE_PTR_0           DMA_RX_BASE_PTR_0; // 4900
    REG_DMA_RX_MAX_CNT_0            DMA_RX_MAX_CNT_0; // 4904
    REG_DMA_RX_CRX_IDX_0            DMA_RX_CRX_IDX_0; // 4908
    REG_DMA_RX_DRX_IDX_0            DMA_RX_DRX_IDX_0; // 490C
    REG_DMA_RX_BASE_PTR_1           DMA_RX_BASE_PTR_1; // 4910
    REG_DMA_RX_MAX_CNT_1            DMA_RX_MAX_CNT_1; // 4914
    REG_DMA_RX_CRX_IDX_1            DMA_RX_CRX_IDX_1; // 4918
    REG_DMA_RX_DRX_IDX_1            DMA_RX_DRX_IDX_1; // 491C
    UINT32                          rsv_4920[56];     // 4920..49FC
    REG_DMA_INFO                    DMA_INFO;         // 4A00
    REG_DMA_GLO_CFG0                DMA_GLO_CFG0;     // 4A04
    REG_DMA_RST_IDX                 DMA_RST_IDX;      // 4A08
    UINT32                          rsv_4A0C;         // 4A0C
    REG_DMA_FREEQ_THRES             DMA_FREEQ_THRES;  // 4A10
    UINT32                          rsv_4A14[3];      // 4A14..4A1C
    REG_DMA_INT_STATUS              DMA_INT_STATUS;   // 4A20
    UINT32                          rsv_4A24;         // 4A24
    REG_DMA_INT_MASK                DMA_INT_MASK;     // 4A28
    UINT32                          rsv_4A2C;         // 4A2C
    REG_DMA_ERR_INT_STATUS          DMA_ERR_INT_STATUS; // 4A30
    REG_DMA_GLO_CFG1                DMA_GLO_CFG1;     // 4A34
    REG_DMA_TX_XDMA_FIFO_CFG0       DMA_TX_XDMA_FIFO_CFG0; // 4A38
    REG_DMA_RX_XDMA_FIFO_CFG0       DMA_RX_XDMA_FIFO_CFG0; // 4A3C
    REG_DMA_INT_STS_GRP0            DMA_INT_STS_GRP0; // 4A40
    REG_DMA_INT_STS_GRP1            DMA_INT_STS_GRP1; // 4A44
    REG_DMA_INT_STS_GRP2            DMA_INT_STS_GRP2; // 4A48
    REG_DMA_INT_STS_GRP3            DMA_INT_STS_GRP3; // 4A4C
    REG_DMA_INT_GRP1                DMA_INT_GRP1;     // 4A50
    REG_DMA_INT_GRP2                DMA_INT_GRP2;     // 4A54
    REG_DMA_INT_GRP3                DMA_INT_GRP3;     // 4A58
    UINT32                          rsv_4A5C;         // 4A5C
    REG_DMA_BUS_CFG                 DMA_BUS_CFG;      // 4A60
    REG_DMA_ULTRA_CFG               DMA_ULTRA_CFG;    // 4A64
    REG_DMA_XFER_CNT_CFG1           DMA_XFER_CNT_CFG1; // 4A68
    UINT32                          rsv_4A6C;         // 4A6C
    REG_DMA_SDL_CFG                 DMA_SDL_CFG;      // 4A70
    REG_DMA_IDLE_MASK               DMA_IDLE_MASK;    // 4A74
    UINT32                          rsv_4A78[2];      // 4A78..4A7C
    REG_DMA_SCH_Q01_CFG             DMA_SCH_Q01_CFG;  // 4A80
    REG_DMA_SCH_Q23_CFG             DMA_SCH_Q23_CFG;  // 4A84
    UINT32                          rsv_4A88[2];      // 4A88..4A8C
    REG_DMA_INT_STATUS_0            DMA_INT_STATUS_0; // 4A90
    REG_DMA_INT_MASK_0              DMA_INT_MASK_0;   // 4A94
    REG_DMA_INT_STATUS_1            DMA_INT_STATUS_1; // 4A98
    REG_DMA_INT_MASK_1              DMA_INT_MASK_1;   // 4A9C
    REG_DMA_INT_STATUS_2            DMA_INT_STATUS_2; // 4AA0
    REG_DMA_INT_MASK_2              DMA_INT_MASK_2;   // 4AA4
    REG_DMA_INT_STATUS_3            DMA_INT_STATUS_3; // 4AA8
    REG_DMA_INT_MASK_3              DMA_INT_MASK_3;   // 4AAC
    REG_DMA_TX_DELAY_INT_CFG_0      DMA_TX_DELAY_INT_CFG_0; // 4AB0
    REG_DMA_TX_DELAY_INT_CFG_1      DMA_TX_DELAY_INT_CFG_1; // 4AB4
    UINT32                          rsv_4AB8[2];      // 4AB8..4ABC
    REG_DMA_RX_DELAY_INT_CFG_0      DMA_RX_DELAY_INT_CFG_0; // 4AC0
    UINT32                          rsv_4AC4;         // 4AC4
    REG_DMA_RX_FC_CFG_0             DMA_RX_FC_CFG_0;  // 4AC8
    REG_DMA_RX_FC_CFG_1             DMA_RX_FC_CFG_1;  // 4ACC
    REG_DMA_PREF_TX_CFG             DMA_PREF_TX_CFG;  // 4AD0
    REG_DMA_PREF_TX_FIFO_CFG0       DMA_PREF_TX_FIFO_CFG0; // 4AD4
    REG_DMA_PREF_TX_FIFO_CFG1       DMA_PREF_TX_FIFO_CFG1; // 4AD8
    REG_DMA_PREF_RX_CFG             DMA_PREF_RX_CFG;  // 4ADC
    REG_DMA_PREF_RX_FIFO_CFG0       DMA_PREF_RX_FIFO_CFG0; // 4AE0
    REG_DMA_PREF_SIDX_CFG           DMA_PREF_SIDX_CFG; // 4AE4
    REG_DMA_PREF_SIDX_MON           DMA_PREF_SIDX_MON; // 4AE8
    REG_DMA_PREF_SIDX_OW            DMA_PREF_SIDX_OW; // 4AEC
    REG_DMA_XFER_CNT_CFG            DMA_XFER_CNT_CFG; // 4AF0
    REG_DMA_XFER_CNT_MON0           DMA_XFER_CNT_MON0; // 4AF4
    REG_DMA_XFER_CNT_MON1           DMA_XFER_CNT_MON1; // 4AF8
    REG_DMA_RX_ULTRA_CFG            DMA_RX_ULTRA_CFG; // 4AFC
    REG_DMA_WRBK_TX_CFG             DMA_WRBK_TX_CFG;  // 4B00
    REG_DMA_WRBK_TX_FIFO_CFG0       DMA_WRBK_TX_FIFO_CFG0; // 4B04
    REG_DMA_WRBK_TX_FIFO_CFG1       DMA_WRBK_TX_FIFO_CFG1; // 4B08
    REG_DMA_WRBK_TX_FIFO_CFG2       DMA_WRBK_TX_FIFO_CFG2; // 4B0C
    REG_DMA_WRBK_TX_FIFO_CFG3       DMA_WRBK_TX_FIFO_CFG3; // 4B10
    UINT32                          rsv_4B14[12];     // 4B14..4B40
    REG_DMA_WRBK_RX_CFG             DMA_WRBK_RX_CFG;  // 4B44
    REG_DMA_WRBK_RX_FIFO_CFG0       DMA_WRBK_RX_FIFO_CFG0; // 4B48
    REG_DMA_WRBK_RX_FIFO_CFG1       DMA_WRBK_RX_FIFO_CFG1; // 4B4C
    UINT32                          rsv_4B50[14];     // 4B50..4B84
    REG_DMA_WRBK_SIDX_CFG           DMA_WRBK_SIDX_CFG; // 4B88
    UINT32                          rsv_4B8C;         // 4B8C
    REG_DMA_WRBK_SIDX_MON           DMA_WRBK_SIDX_MON; // 4B90
    REG_DMA_WRBK_SIDX_OW            DMA_WRBK_SIDX_OW; // 4B94
    REG_DMA_DBG_CFG                 DMA_DBG_CFG;      // 4B98
    REG_DMA_TX_DBG_MON_0            DMA_TX_DBG_MON_0; // 4B9C
    REG_DMA_TX_DBG_MON_1            DMA_TX_DBG_MON_1; // 4BA0
    REG_DMA_TX_DBG_MON_2            DMA_TX_DBG_MON_2; // 4BA4
    REG_DMA_TX_DBG_MON_3            DMA_TX_DBG_MON_3; // 4BA8
    REG_DMA_TX_DBG_MON_4            DMA_TX_DBG_MON_4; // 4BAC
    REG_DMA_TX_DBG_MON_5            DMA_TX_DBG_MON_5; // 4BB0
    REG_DMA_TX_DBG_MON_6            DMA_TX_DBG_MON_6; // 4BB4
    REG_DMA_TX_DBG_MON_7            DMA_TX_DBG_MON_7; // 4BB8
    REG_DMA_TX_DBG_MON_8            DMA_TX_DBG_MON_8; // 4BBC
    REG_DMA_RX_DBG_MON_0            DMA_RX_DBG_MON_0; // 4BC0
    REG_DMA_RX_DBG_MON_1            DMA_RX_DBG_MON_1; // 4BC4
    REG_DMA_RX_DBG_MON_2            DMA_RX_DBG_MON_2; // 4BC8
    REG_DMA_RX_DBG_MON_3            DMA_RX_DBG_MON_3; // 4BCC
    REG_DMA_RX_DBG_MON_4            DMA_RX_DBG_MON_4; // 4BD0
    REG_DMA_RX_DBG_MON_5            DMA_RX_DBG_MON_5; // 4BD4
    REG_DMA_RX_DBG_MON_6            DMA_RX_DBG_MON_6; // 4BD8
    REG_DMA_RX_DBG_MON_7            DMA_RX_DBG_MON_7; // 4BDC
    REG_DMA_RX_DBG_MON_8            DMA_RX_DBG_MON_8; // 4BE0
    REG_DMA_RX_MULTI_ID_CFG         DMA_RX_MULTI_ID_CFG; // 4BE4
    REG_DMA_RX_MULTI_ID_BLK_STS     DMA_RX_MULTI_ID_BLK_STS; // 4BE8
    REG_DMA_RX_MULTI_ID_DATA_STS0   DMA_RX_MULTI_ID_DATA_STS0; // 4BEC
    REG_DMA_RX_MULTI_ID_DATA_STS1   DMA_RX_MULTI_ID_DATA_STS1; // 4BF0
    REG_DMA_RX_MULTI_ID_DATA_STS2   DMA_RX_MULTI_ID_DATA_STS2; // 4BF4
    REG_DMA_RX_MULTI_ID_DATA_STS3   DMA_RX_MULTI_ID_DATA_STS3; // 4BF8
    REG_DMA_RX_MULTI_ID_DBG_MON     DMA_RX_MULTI_ID_DBG_MON; // 4BFC
}WDMA0_v3_REGS, *PWDMA0_v3_REGS;

// ---------- WDMA0_v3 Enum Definitions      ----------
// ---------- WDMA0_v3 C Macro Definitions   ----------


#define  WDMA_TX_BASE_PTR_0                                      0x0000000000
#define  WDMA_TX_MAX_CNT_0                                       0x0000000004
#define  WDMA_TX_CTX_IDX_0                                       0x0000000008
#define  WDMA_TX_DTX_IDX_0                                       0x000000000C
#define  WDMA_TX_BASE_PTR_1                                      0x0000000010
#define  WDMA_TX_MAX_CNT_1                                       0x0000000014
#define  WDMA_TX_CTX_IDX_1                                       0x0000000018
#define  WDMA_TX_DTX_IDX_1                                       0x000000001C
#define  WDMA_TX_BASE_PTR_2                                      0x0000000020
#define  WDMA_TX_MAX_CNT_2                                       0x0000000024
#define  WDMA_TX_CTX_IDX_2                                       0x0000000028
#define  WDMA_TX_DTX_IDX_2                                       0x000000002C
#define  WDMA_TX_BASE_PTR_3                                      0x0000000030
#define  WDMA_TX_MAX_CNT_3                                       0x0000000034
#define  WDMA_TX_CTX_IDX_3                                       0x0000000038
#define  WDMA_TX_DTX_IDX_3                                       0x000000003C
#define  WDMA_RX_BASE_PTR_0                                      0x0000000100
#define  WDMA_RX_MAX_CNT_0                                       0x0000000104
#define  WDMA_RX_CRX_IDX_0                                       0x0000000108
#define  WDMA_RX_DRX_IDX_0                                       0x000000010C
#define  WDMA_RX_BASE_PTR_1                                      0x0000000110
#define  WDMA_RX_MAX_CNT_1                                       0x0000000114
#define  WDMA_RX_CRX_IDX_1                                       0x0000000118
#define  WDMA_RX_DRX_IDX_1                                       0x000000011C
#define  WDMA_INFO                                               0x0000000200
#define  WDMA_GLO_CFG0                                           0x0000000204
#define  WDMA_RST_IDX                                            0x0000000208
#define  WDMA_FREEQ_THRES                                        0x0000000210
#define  WDMA_INT_STATUS                                         0x0000000220
#define  WDMA_INT_MASK                                           0x0000000228
#define  WDMA_ERR_INT_STATUS                                     0x0000000230
#define  WDMA_GLO_CFG1                                           0x0000000234
#define  WDMA_TX_XDMA_FIFO_CFG0                                  0x0000000238
#define  WDMA_RX_XDMA_FIFO_CFG0                                  0x000000023C
#define  WDMA_INT_STS_GRP0                                       0x0000000240
#define  WDMA_INT_STS_GRP1                                       0x0000000244
#define  WDMA_INT_STS_GRP2                                       0x0000000248
#define  WDMA_INT_STS_GRP3                                       0x000000024C
#define  WDMA_INT_GRP1                                           0x0000000250
#define  WDMA_INT_GRP2                                           0x0000000254
#define  WDMA_INT_GRP3                                           0x0000000258
#define  WDMA_BUS_CFG                                            0x0000000260
#define  WDMA_ULTRA_CFG                                          0x0000000264
#define  WDMA_XFER_CNT_CFG1                                      0x0000000268
#define  WDMA_SDL_CFG                                            0x0000000270
#define  WDMA_IDLE_MASK                                          0x0000000274
#define  WDMA_SCH_Q01_CFG                                        0x0000000280
#define  WDMA_SCH_Q23_CFG                                        0x0000000284
#define  WDMA_INT_STATUS_0                                       0x0000000290
#define  WDMA_INT_MASK_0                                         0x0000000294
#define  WDMA_INT_STATUS_1                                       0x0000000298
#define  WDMA_INT_MASK_1                                         0x000000029C
#define  WDMA_INT_STATUS_2                                       0x00000002A0
#define  WDMA_INT_MASK_2                                         0x00000002A4
#define  WDMA_INT_STATUS_3                                       0x00000002A8
#define  WDMA_INT_MASK_3                                         0x00000002AC
#define  WDMA_TX_DELAY_INT_CFG_0                                 0x00000002B0
#define  WDMA_TX_DELAY_INT_CFG_1                                 0x00000002B4
#define  WDMA_RX_DELAY_INT_CFG_0                                 0x00000002C0
#define  WDMA_RX_FC_CFG_0                                        0x00000002C8
#define  WDMA_RX_FC_CFG_1                                        0x00000002CC
#define  WDMA_PREF_TX_CFG                                        0x00000002D0
#define  WDMA_PREF_TX_FIFO_CFG0                                  0x00000002D4
#define  WDMA_PREF_TX_FIFO_CFG1                                  0x00000002D8
#define  WDMA_PREF_RX_CFG                                        0x00000002DC
#define  WDMA_PREF_RX_FIFO_CFG0                                  0x00000002E0
#define  WDMA_PREF_SIDX_CFG                                      0x00000002E4
#define  WDMA_PREF_SIDX_MON                                      0x00000002E8
#define  WDMA_PREF_SIDX_OW                                       0x00000002EC
#define  WDMA_XFER_CNT_CFG                                       0x00000002F0
#define  WDMA_XFER_CNT_MON0                                      0x00000002F4
#define  WDMA_XFER_CNT_MON1                                      0x00000002F8
#define  WDMA_RX_ULTRA_CFG                                       0x00000002FC
#define  WDMA_WRBK_TX_CFG                                        0x0000000300
#define  WDMA_WRBK_TX_FIFO_CFG0                                  0x0000000304
#define  WDMA_WRBK_TX_FIFO_CFG1                                  0x0000000308
#define  WDMA_WRBK_TX_FIFO_CFG2                                  0x000000030C
#define  WDMA_WRBK_TX_FIFO_CFG3                                  0x0000000310
#define  WDMA_WRBK_RX_CFG                                        0x0000000344
#define  WDMA_WRBK_RX_FIFO_CFG0                                  0x0000000348
#define  WDMA_WRBK_RX_FIFO_CFG1                                  0x000000034C
#define  WDMA_WRBK_SIDX_CFG                                      0x0000000388
#define  WDMA_WRBK_SIDX_MON                                      0x0000000390
#define  WDMA_WRBK_SIDX_OW                                       0x0000000394
#define  WDMA_DBG_CFG                                            0x0000000398
#define  WDMA_TX_DBG_MON_0                                       0x000000039C
#define  WDMA_TX_DBG_MON_1                                       0x00000003A0
#define  WDMA_TX_DBG_MON_2                                       0x00000003A4
#define  WDMA_TX_DBG_MON_3                                       0x00000003A8
#define  WDMA_TX_DBG_MON_4                                       0x00000003AC
#define  WDMA_TX_DBG_MON_5                                       0x00000003B0
#define  WDMA_TX_DBG_MON_6                                       0x00000003B4
#define  WDMA_TX_DBG_MON_7                                       0x00000003B8
#define  WDMA_TX_DBG_MON_8                                       0x00000003BC
#define  WDMA_RX_DBG_MON_0                                       0x00000003C0
#define  WDMA_RX_DBG_MON_1                                       0x00000003C4
#define  WDMA_RX_DBG_MON_2                                       0x00000003C8
#define  WDMA_RX_DBG_MON_3                                       0x00000003CC
#define  WDMA_RX_DBG_MON_4                                       0x00000003D0
#define  WDMA_RX_DBG_MON_5                                       0x00000003D4
#define  WDMA_RX_DBG_MON_6                                       0x00000003D8
#define  WDMA_RX_DBG_MON_7                                       0x00000003DC
#define  WDMA_RX_DBG_MON_8                                       0x00000003E0
#define  WDMA_RX_MULTI_ID_CFG                                    0x00000003E4
#define  WDMA_RX_MULTI_ID_BLK_STS                                0x00000003E8
#define  WDMA_RX_MULTI_ID_DATA_STS0                              0x00000003EC
#define  WDMA_RX_MULTI_ID_DATA_STS1                              0x00000003F0
#define  WDMA_RX_MULTI_ID_DATA_STS2                              0x00000003F4
#define  WDMA_RX_MULTI_ID_DATA_STS3                              0x00000003F8
#define  WDMA_RX_MULTI_ID_DBG_MON                                0x00000003FC

#endif


#define  WDMA_TX_BASE_PTR_0_FLD_TX_BASE_PTR                      REG_FLD(32, 0)

#define  WDMA_TX_MAX_CNT_0_FLD_TX_MAX_CNT                        REG_FLD(16, 0)

#define  WDMA_TX_CTX_IDX_0_FLD_TX_CTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_DTX_IDX_0_FLD_TX_DTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_BASE_PTR_1_FLD_TX_BASE_PTR                      REG_FLD(32, 0)

#define  WDMA_TX_MAX_CNT_1_FLD_TX_MAX_CNT                        REG_FLD(16, 0)

#define  WDMA_TX_CTX_IDX_1_FLD_TX_CTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_DTX_IDX_1_FLD_TX_DTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_BASE_PTR_2_FLD_TX_BASE_PTR                      REG_FLD(32, 0)

#define  WDMA_TX_MAX_CNT_2_FLD_TX_MAX_CNT                        REG_FLD(16, 0)

#define  WDMA_TX_CTX_IDX_2_FLD_TX_CTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_DTX_IDX_2_FLD_TX_DTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_BASE_PTR_3_FLD_TX_BASE_PTR                      REG_FLD(32, 0)

#define  WDMA_TX_MAX_CNT_3_FLD_TX_MAX_CNT                        REG_FLD(16, 0)

#define  WDMA_TX_CTX_IDX_3_FLD_TX_CTX_IDX                        REG_FLD(16, 0)

#define  WDMA_TX_DTX_IDX_3_FLD_TX_DTX_IDX                        REG_FLD(16, 0)

#define  WDMA_RX_BASE_PTR_0_FLD_RX_BASE_PTR                      REG_FLD(32, 0)

#define  WDMA_RX_MAX_CNT_0_FLD_RX_MAX_CNT                        REG_FLD(16, 0)

#define  WDMA_RX_CRX_IDX_0_FLD_RX_CRX_IDX                        REG_FLD(16, 0)

#define  WDMA_RX_DRX_IDX_0_FLD_RX_DRX_IDX                        REG_FLD(16, 0)

#define  WDMA_RX_BASE_PTR_1_FLD_RX_BASE_PTR                      REG_FLD(32, 0)

#define  WDMA_RX_MAX_CNT_1_FLD_RX_MAX_CNT                        REG_FLD(16, 0)

#define  WDMA_RX_CRX_IDX_1_FLD_RX_CRX_IDX                        REG_FLD(16, 0)

#define  WDMA_RX_DRX_IDX_1_FLD_RX_DRX_IDX                        REG_FLD(16, 0)

#define  WDMA_INFO_FLD_DMA_REVISION                              REG_FLD(4, 28)
#define  WDMA_INFO_FLD_INDEX_WIDTH                               REG_FLD(4, 24)
#define  WDMA_INFO_FLD_BASE_PTR_WIDTH                            REG_FLD(8, 16)
#define  WDMA_INFO_FLD_RX_RING_NUM                               REG_FLD(8, 8)
#define  WDMA_INFO_FLD_TX_RING_NUM                               REG_FLD(8, 0)

#define  WDMA_GLO_CFG0_FLD_RX_2B_OFFSET                          REG_FLD(1, 31)
#define  WDMA_GLO_CFG0_FLD_DMAD_BYTE_SWAP                        REG_FLD(1, 29)
#define  WDMA_GLO_CFG0_FLD_DEC_WCOMP                             REG_FLD(1, 28)
#define  WDMA_GLO_CFG0_FLD_PKT_WCOMP                             REG_FLD(1, 27)
#define  WDMA_GLO_CFG0_FLD_DMAD_BYTE_SWAP_SEL                    REG_FLD(1, 26)
#define  WDMA_GLO_CFG0_FLD_PAYLOAD_BYTE_SWAP_SEL                 REG_FLD(1, 25)
#define  WDMA_GLO_CFG0_FLD_LB_MODE                               REG_FLD(1, 24)
#define  WDMA_GLO_CFG0_FLD_CDM_FCNT_THRES                        REG_FLD(4, 18)
#define  WDMA_GLO_CFG0_FLD_OTSD_THRES                            REG_FLD(4, 14)
#define  WDMA_GLO_CFG0_FLD_DMA_BT_SIZE                           REG_FLD(3, 11)
#define  WDMA_GLO_CFG0_FLD_TX_SCH_RST                            REG_FLD(1, 10)
#define  WDMA_GLO_CFG0_FLD_TX_CHK_DDONE                          REG_FLD(1, 9)
#define  WDMA_GLO_CFG0_FLD_PAYLOAD_BYTE_SWAP                     REG_FLD(1, 7)
#define  WDMA_GLO_CFG0_FLD_TX_WB_DDONE                           REG_FLD(1, 6)
#define  WDMA_GLO_CFG0_FLD_RX_BURST_4KB_BND_EN                   REG_FLD(1, 5)
#define  WDMA_GLO_CFG0_FLD_TX_BURST_4KB_BND_EN                   REG_FLD(1, 4)
#define  WDMA_GLO_CFG0_FLD_RX_DMA_BUSY                           REG_FLD(1, 3)
#define  WDMA_GLO_CFG0_FLD_RX_DMA_EN                             REG_FLD(1, 2)
#define  WDMA_GLO_CFG0_FLD_TX_DMA_BUSY                           REG_FLD(1, 1)
#define  WDMA_GLO_CFG0_FLD_TX_DMA_EN                             REG_FLD(1, 0)

#define  WDMA_RST_IDX_FLD_RST_DRX_IDX1                           REG_FLD(1, 17)
#define  WDMA_RST_IDX_FLD_RST_DRX_IDX0                           REG_FLD(1, 16)
#define  WDMA_RST_IDX_FLD_RST_DTX_IDX3                           REG_FLD(1, 3)
#define  WDMA_RST_IDX_FLD_RST_DTX_IDX2                           REG_FLD(1, 2)
#define  WDMA_RST_IDX_FLD_RST_DTX_IDX1                           REG_FLD(1, 1)
#define  WDMA_RST_IDX_FLD_RST_DTX_IDX0                           REG_FLD(1, 0)

#define  WDMA_FREEQ_THRES_FLD_FREEQ_THRES                        REG_FLD(4, 0)

#define  WDMA_INT_STATUS_FLD_RX_COHERENT                         REG_FLD(1, 31)
#define  WDMA_INT_STATUS_FLD_RX_DLY_INT                          REG_FLD(1, 30)
#define  WDMA_INT_STATUS_FLD_TX_COHERENT                         REG_FLD(1, 29)
#define  WDMA_INT_STATUS_FLD_TX_DLY_INT                          REG_FLD(1, 28)
#define  WDMA_INT_STATUS_FLD_RX_DONE_DLY_INT1                    REG_FLD(1, 21)
#define  WDMA_INT_STATUS_FLD_RX_DONE_DLY_INT0                    REG_FLD(1, 20)
#define  WDMA_INT_STATUS_FLD_RX_DONE_INT1                        REG_FLD(1, 17)
#define  WDMA_INT_STATUS_FLD_RX_DONE_INT0                        REG_FLD(1, 16)
#define  WDMA_INT_STATUS_FLD_TX_DONE_DLY_INT3                    REG_FLD(1, 11)
#define  WDMA_INT_STATUS_FLD_TX_DONE_DLY_INT2                    REG_FLD(1, 10)
#define  WDMA_INT_STATUS_FLD_TX_DONE_DLY_INT1                    REG_FLD(1, 9)
#define  WDMA_INT_STATUS_FLD_TX_DONE_DLY_INT0                    REG_FLD(1, 8)
#define  WDMA_INT_STATUS_FLD_TX_DONE_INT3                        REG_FLD(1, 3)
#define  WDMA_INT_STATUS_FLD_TX_DONE_INT2                        REG_FLD(1, 2)
#define  WDMA_INT_STATUS_FLD_TX_DONE_INT1                        REG_FLD(1, 1)
#define  WDMA_INT_STATUS_FLD_TX_DONE_INT0                        REG_FLD(1, 0)

#define  WDMA_INT_MASK_FLD_RX_COHERENT                           REG_FLD(1, 31)
#define  WDMA_INT_MASK_FLD_RX_DLY_INT                            REG_FLD(1, 30)
#define  WDMA_INT_MASK_FLD_TX_COHERENT                           REG_FLD(1, 29)
#define  WDMA_INT_MASK_FLD_TX_DLY_INT                            REG_FLD(1, 28)
#define  WDMA_INT_MASK_FLD_RX_DONE_DLY_INT1                      REG_FLD(1, 21)
#define  WDMA_INT_MASK_FLD_RX_DONE_DLY_INT0                      REG_FLD(1, 20)
#define  WDMA_INT_MASK_FLD_RX_DONE_INT1                          REG_FLD(1, 17)
#define  WDMA_INT_MASK_FLD_RX_DONE_INT0                          REG_FLD(1, 16)
#define  WDMA_INT_MASK_FLD_TX_DONE_DLY_INT3                      REG_FLD(1, 11)
#define  WDMA_INT_MASK_FLD_TX_DONE_DLY_INT2                      REG_FLD(1, 10)
#define  WDMA_INT_MASK_FLD_TX_DONE_DLY_INT1                      REG_FLD(1, 9)
#define  WDMA_INT_MASK_FLD_TX_DONE_DLY_INT0                      REG_FLD(1, 8)
#define  WDMA_INT_MASK_FLD_TX_DONE_INT3                          REG_FLD(1, 3)
#define  WDMA_INT_MASK_FLD_TX_DONE_INT2                          REG_FLD(1, 2)
#define  WDMA_INT_MASK_FLD_TX_DONE_INT1                          REG_FLD(1, 1)
#define  WDMA_INT_MASK_FLD_TX_DONE_INT0                          REG_FLD(1, 0)

#define  WDMA_ERR_INT_STATUS_FLD_RX_WRBK_WR_OF_ERR_RESP          REG_FLD(1, 23)
#define  WDMA_ERR_INT_STATUS_FLD_RX_WRBK_RD_UD_ERR_RESP          REG_FLD(1, 22)
#define  WDMA_ERR_INT_STATUS_FLD_RX_PREF_WR_OF_ERR_RESP          REG_FLD(1, 21)
#define  WDMA_ERR_INT_STATUS_FLD_RX_PREF_RD_UD_ERR_RESP          REG_FLD(1, 20)
#define  WDMA_ERR_INT_STATUS_FLD_TX_WRBK_WR_OF_ERR_RESP          REG_FLD(1, 19)
#define  WDMA_ERR_INT_STATUS_FLD_TX_WRBK_RD_UD_ERR_RESP          REG_FLD(1, 18)
#define  WDMA_ERR_INT_STATUS_FLD_TX_PREF_WR_OF_ERR_RESP          REG_FLD(1, 17)
#define  WDMA_ERR_INT_STATUS_FLD_TX_PREF_RD_UD_ERR_RESP          REG_FLD(1, 16)
#define  WDMA_ERR_INT_STATUS_FLD_RX_CDM_EOF_ERR_RESP             REG_FLD(1, 15)
#define  WDMA_ERR_INT_STATUS_FLD_RX_CMD_EOF_ERR_RESP             REG_FLD(1, 14)
#define  WDMA_ERR_INT_STATUS_FLD_RX_AXI_WRBK_RD_ERR_RESP         REG_FLD(1, 13)
#define  WDMA_ERR_INT_STATUS_FLD_RX_AXI_WRBK_WR_ERR_RESP         REG_FLD(1, 12)
#define  WDMA_ERR_INT_STATUS_FLD_RX_AXI_PREF_RD_ERR_RESP         REG_FLD(1, 11)
#define  WDMA_ERR_INT_STATUS_FLD_RX_AXI_PREF_WR_ERR_RESP         REG_FLD(1, 10)
#define  WDMA_ERR_INT_STATUS_FLD_RX_AXI_BURST_RD_ERR_RESP        REG_FLD(1, 9)
#define  WDMA_ERR_INT_STATUS_FLD_RX_AXI_BURST_WR_ERR_RESP        REG_FLD(1, 8)
#define  WDMA_ERR_INT_STATUS_FLD_TX_AXI_WRBK_RD_ERR_RESP         REG_FLD(1, 5)
#define  WDMA_ERR_INT_STATUS_FLD_TX_AXI_WRBK_WR_ERR_RESP         REG_FLD(1, 4)
#define  WDMA_ERR_INT_STATUS_FLD_TX_AXI_PREF_RD_ERR_RESP         REG_FLD(1, 3)
#define  WDMA_ERR_INT_STATUS_FLD_TX_AXI_PREF_WR_ERR_RESP         REG_FLD(1, 2)
#define  WDMA_ERR_INT_STATUS_FLD_TX_AXI_BURST_RD_ERR_RESP        REG_FLD(1, 1)
#define  WDMA_ERR_INT_STATUS_FLD_TX_AXI_BURST_WR_ERR_RESP        REG_FLD(1, 0)

#define  WDMA_GLO_CFG1_FLD_PREF_CHK_DDONE_POL                    REG_FLD(1, 25)
#define  WDMA_GLO_CFG1_FLD_PREF_CHK_DDONE_DW3                    REG_FLD(1, 24)
#define  WDMA_GLO_CFG1_FLD_PREF_CHK_DDONE_DW2                    REG_FLD(1, 23)
#define  WDMA_GLO_CFG1_FLD_PREF_CHK_DDONE_DW1                    REG_FLD(1, 22)
#define  WDMA_GLO_CFG1_FLD_PREF_CHK_DDONE_DW0                    REG_FLD(1, 21)
#define  WDMA_GLO_CFG1_FLD_RX_PREF_CHK_DDONE2_METHOD_CLR         REG_FLD(1, 20)
#define  WDMA_GLO_CFG1_FLD_RX_PREF_CHK_DDONE2_CLR                REG_FLD(1, 19)
#define  WDMA_GLO_CFG1_FLD_RX_PREF_CHK_DDONE2_BUSY               REG_FLD(1, 18)
#define  WDMA_GLO_CFG1_FLD_RX_PREF_CHK_DDONE2_EN                 REG_FLD(1, 17)
#define  WDMA_GLO_CFG1_FLD_TX_PREF_CHK_DDONE2_METHOD_CLR         REG_FLD(1, 16)
#define  WDMA_GLO_CFG1_FLD_TX_PREF_CHK_DDONE2_CLR                REG_FLD(1, 15)
#define  WDMA_GLO_CFG1_FLD_TX_PREF_CHK_DDONE2_BUSY               REG_FLD(1, 14)
#define  WDMA_GLO_CFG1_FLD_TX_PREF_CHK_DDONE2_EN                 REG_FLD(1, 13)
#define  WDMA_GLO_CFG1_FLD_RX_MERGE_ID_EN                        REG_FLD(1, 11)
#define  WDMA_GLO_CFG1_FLD_RX_JBP_ID_EN                          REG_FLD(1, 10)
#define  WDMA_GLO_CFG1_FLD_RX_SCATHER_BYPASS                     REG_FLD(1, 9)
#define  WDMA_GLO_CFG1_FLD_TX_SEG1_EN                            REG_FLD(1, 8)
#define  WDMA_GLO_CFG1_FLD_RX_BST_THRES                          REG_FLD(4, 4)
#define  WDMA_GLO_CFG1_FLD_TX_BST_THRES                          REG_FLD(4, 0)

#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_ARR_FIFO_FULL             REG_FLD(1, 14)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_ARR_FIFO_EMPTY            REG_FLD(1, 13)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_ARR_FIFO_CLEAR            REG_FLD(1, 12)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_DMAD_FIFO_FULL            REG_FLD(1, 10)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_DMAD_FIFO_EMPTY           REG_FLD(1, 9)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_DMAD_FIFO_CLEAR           REG_FLD(1, 8)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_CMD_FIFO_FULL             REG_FLD(1, 6)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_CMD_FIFO_EMPTY            REG_FLD(1, 5)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_CMD_FIFO_CLEAR            REG_FLD(1, 4)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_PAR_FIFO_FULL             REG_FLD(1, 2)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_PAR_FIFO_EMPTY            REG_FLD(1, 1)
#define  WDMA_TX_XDMA_FIFO_CFG0_FLD_TX_PAR_FIFO_CLEAR            REG_FLD(1, 0)

#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_BID_FIFO_FULL             REG_FLD(1, 23)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_BID_FIFO_EMPTY            REG_FLD(1, 22)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_BID_FIFO_CLEAR            REG_FLD(1, 21)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_WID_FIFO_FULL             REG_FLD(1, 20)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_WID_FIFO_EMPTY            REG_FLD(1, 19)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_WID_FIFO_CLEAR            REG_FLD(1, 18)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_LEN_FIFO_FULL             REG_FLD(1, 17)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_LEN_FIFO_EMPTY            REG_FLD(1, 16)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_LEN_FIFO_CLEAR            REG_FLD(1, 15)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_ARR_FIFO_FULL             REG_FLD(1, 14)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_ARR_FIFO_EMPTY            REG_FLD(1, 13)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_ARR_FIFO_CLEAR            REG_FLD(1, 12)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_DMAD_FIFO_FULL            REG_FLD(1, 10)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_DMAD_FIFO_EMPTY           REG_FLD(1, 9)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_DMAD_FIFO_CLEAR           REG_FLD(1, 8)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_CMD_FIFO_FULL             REG_FLD(1, 6)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_CMD_FIFO_EMPTY            REG_FLD(1, 5)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_CMD_FIFO_CLEAR            REG_FLD(1, 4)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_PAR_FIFO_FULL             REG_FLD(1, 2)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_PAR_FIFO_EMPTY            REG_FLD(1, 1)
#define  WDMA_RX_XDMA_FIFO_CFG0_FLD_RX_PAR_FIFO_CLEAR            REG_FLD(1, 0)

#define  WDMA_INT_STS_GRP0_FLD_DMA_INT_STS_GRP0                  REG_FLD(32, 0)

#define  WDMA_INT_STS_GRP1_FLD_DMA_INT_STS_GRP1                  REG_FLD(32, 0)

#define  WDMA_INT_STS_GRP2_FLD_DMA_INT_STS_GRP2                  REG_FLD(32, 0)

#define  WDMA_INT_STS_GRP3_FLD_DMA_INT_STS_GRP3                  REG_FLD(32, 0)

#define  WDMA_INT_GRP1_FLD_DMA_INT_GRP1                          REG_FLD(32, 0)

#define  WDMA_INT_GRP2_FLD_DMA_INT_GRP2                          REG_FLD(32, 0)

#define  WDMA_INT_GRP3_FLD_DMA_INT_GRP3                          REG_FLD(32, 0)

#define  WDMA_BUS_CFG_FLD_AXI_ULTRA_RXDMA                        REG_FLD(2, 14)
#define  WDMA_BUS_CFG_FLD_AXI_ULTRA_TXDMA                        REG_FLD(2, 12)
#define  WDMA_BUS_CFG_FLD_AXI_CTRL_UPDATED                       REG_FLD(1, 10)
#define  WDMA_BUS_CFG_FLD_AXI_R_BUSY                             REG_FLD(1, 9)
#define  WDMA_BUS_CFG_FLD_AXI_W_BUSY                             REG_FLD(1, 8)
#define  WDMA_BUS_CFG_FLD_AXI_LOCK_ERROR                         REG_FLD(1, 6)
#define  WDMA_BUS_CFG_FLD_AXI_ERRMID_SET_RIRQ                    REG_FLD(1, 5)
#define  WDMA_BUS_CFG_FLD_AXI_ERRMID_SET_BIRQ                    REG_FLD(1, 4)
#define  WDMA_BUS_CFG_FLD_AXI_OUTSTANDING_EXTEND                 REG_FLD(1, 3)
#define  WDMA_BUS_CFG_FLD_AXI_QOS_ON                             REG_FLD(1, 2)
#define  WDMA_BUS_CFG_FLD_AXI_CG_DISABLE                         REG_FLD(1, 0)

#define  WDMA_ULTRA_CFG_FLD_AXI_ULTRA_EN                         REG_FLD(1, 31)
#define  WDMA_ULTRA_CFG_FLD_AXI_ULTRA_THRES                      REG_FLD(11, 16)
#define  WDMA_ULTRA_CFG_FLD_AXI_PREULTRA_EN                      REG_FLD(1, 15)
#define  WDMA_ULTRA_CFG_FLD_AXI_PREULTRA_THRES                   REG_FLD(11, 0)

#define  WDMA_XFER_CNT_CFG1_FLD_MAX_PKT_NUM                      REG_FLD(16, 0)

#define  WDMA_SDL_CFG_FLD_SDL_EN                                 REG_FLD(1, 16)
#define  WDMA_SDL_CFG_FLD_SDL                                    REG_FLD(16, 0)

#define  WDMA_IDLE_MASK_FLD_DMA_IDLE                             REG_FLD(1, 5)
#define  WDMA_IDLE_MASK_FLD_AXI_IDLE                             REG_FLD(1, 4)
#define  WDMA_IDLE_MASK_FLD_IDLE_MASK                            REG_FLD(3, 0)

#define  WDMA_SCH_Q01_CFG_FLD_MAX_BKT_SIZE1                      REG_FLD(1, 31)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_RATE_ULMT1                     REG_FLD(1, 30)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_WEIGHT1                        REG_FLD(2, 28)
#define  WDMA_SCH_Q01_CFG_FLD_MIN_RATE_RATIO1                    REG_FLD(2, 26)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_RATE1                          REG_FLD(10, 16)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_BKT_SIZE0                      REG_FLD(1, 15)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_RATE_ULMT0                     REG_FLD(1, 14)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_WEIGHT0                        REG_FLD(2, 12)
#define  WDMA_SCH_Q01_CFG_FLD_MIN_RATE_RATIO0                    REG_FLD(2, 10)
#define  WDMA_SCH_Q01_CFG_FLD_MAX_RATE0                          REG_FLD(10, 0)

#define  WDMA_SCH_Q23_CFG_FLD_MAX_BKT_SIZE3                      REG_FLD(1, 31)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_RATE_ULMT3                     REG_FLD(1, 30)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_WEIGHT3                        REG_FLD(2, 28)
#define  WDMA_SCH_Q23_CFG_FLD_MIN_RATE_RATIO3                    REG_FLD(2, 26)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_RATE3                          REG_FLD(10, 16)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_BKT_SIZE2                      REG_FLD(1, 15)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_RATE_ULMT2                     REG_FLD(1, 14)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_WEIGHT2                        REG_FLD(2, 12)
#define  WDMA_SCH_Q23_CFG_FLD_MIN_RATE_RATIO2                    REG_FLD(2, 10)
#define  WDMA_SCH_Q23_CFG_FLD_MAX_RATE2                          REG_FLD(10, 0)

#define  WDMA_INT_STATUS_0_FLD_RX_COHERENT                       REG_FLD(1, 31)
#define  WDMA_INT_STATUS_0_FLD_RX_DLY_INT                        REG_FLD(1, 30)
#define  WDMA_INT_STATUS_0_FLD_TX_COHERENT                       REG_FLD(1, 29)
#define  WDMA_INT_STATUS_0_FLD_TX_DLY_INT                        REG_FLD(1, 28)
#define  WDMA_INT_STATUS_0_FLD_RX_DONE_DLY_INT1                  REG_FLD(1, 21)
#define  WDMA_INT_STATUS_0_FLD_RX_DONE_DLY_INT0                  REG_FLD(1, 20)
#define  WDMA_INT_STATUS_0_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define  WDMA_INT_STATUS_0_FLD_RX_DONE_INT0                      REG_FLD(1, 16)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_DLY_INT3                  REG_FLD(1, 11)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_DLY_INT2                  REG_FLD(1, 10)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_DLY_INT1                  REG_FLD(1, 9)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_DLY_INT0                  REG_FLD(1, 8)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_INT3                      REG_FLD(1, 3)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_INT2                      REG_FLD(1, 2)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_INT1                      REG_FLD(1, 1)
#define  WDMA_INT_STATUS_0_FLD_TX_DONE_INT0                      REG_FLD(1, 0)

#define  WDMA_INT_MASK_0_FLD_RX_COHERENT                         REG_FLD(1, 31)
#define  WDMA_INT_MASK_0_FLD_RX_DLY_INT                          REG_FLD(1, 30)
#define  WDMA_INT_MASK_0_FLD_TX_COHERENT                         REG_FLD(1, 29)
#define  WDMA_INT_MASK_0_FLD_TX_DLY_INT                          REG_FLD(1, 28)
#define  WDMA_INT_MASK_0_FLD_RX_DONE_DLY_INT1                    REG_FLD(1, 21)
#define  WDMA_INT_MASK_0_FLD_RX_DONE_DLY_INT0                    REG_FLD(1, 20)
#define  WDMA_INT_MASK_0_FLD_RX_DONE_INT1                        REG_FLD(1, 17)
#define  WDMA_INT_MASK_0_FLD_RX_DONE_INT0                        REG_FLD(1, 16)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_DLY_INT3                    REG_FLD(1, 11)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_DLY_INT2                    REG_FLD(1, 10)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_DLY_INT1                    REG_FLD(1, 9)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_DLY_INT0                    REG_FLD(1, 8)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_INT3                        REG_FLD(1, 3)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_INT2                        REG_FLD(1, 2)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_INT1                        REG_FLD(1, 1)
#define  WDMA_INT_MASK_0_FLD_TX_DONE_INT0                        REG_FLD(1, 0)

#define  WDMA_INT_STATUS_1_FLD_RX_COHERENT                       REG_FLD(1, 31)
#define  WDMA_INT_STATUS_1_FLD_RX_DLY_INT                        REG_FLD(1, 30)
#define  WDMA_INT_STATUS_1_FLD_TX_COHERENT                       REG_FLD(1, 29)
#define  WDMA_INT_STATUS_1_FLD_TX_DLY_INT                        REG_FLD(1, 28)
#define  WDMA_INT_STATUS_1_FLD_RX_DONE_DLY_INT1                  REG_FLD(1, 21)
#define  WDMA_INT_STATUS_1_FLD_RX_DONE_DLY_INT0                  REG_FLD(1, 20)
#define  WDMA_INT_STATUS_1_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define  WDMA_INT_STATUS_1_FLD_RX_DONE_INT0                      REG_FLD(1, 16)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_DLY_INT3                  REG_FLD(1, 11)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_DLY_INT2                  REG_FLD(1, 10)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_DLY_INT1                  REG_FLD(1, 9)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_DLY_INT0                  REG_FLD(1, 8)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_INT3                      REG_FLD(1, 3)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_INT2                      REG_FLD(1, 2)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_INT1                      REG_FLD(1, 1)
#define  WDMA_INT_STATUS_1_FLD_TX_DONE_INT0                      REG_FLD(1, 0)

#define  WDMA_INT_MASK_1_FLD_RX_COHERENT                         REG_FLD(1, 31)
#define  WDMA_INT_MASK_1_FLD_RX_DLY_INT                          REG_FLD(1, 30)
#define  WDMA_INT_MASK_1_FLD_TX_COHERENT                         REG_FLD(1, 29)
#define  WDMA_INT_MASK_1_FLD_TX_DLY_INT                          REG_FLD(1, 28)
#define  WDMA_INT_MASK_1_FLD_RX_DONE_DLY_INT1                    REG_FLD(1, 21)
#define  WDMA_INT_MASK_1_FLD_RX_DONE_DLY_INT0                    REG_FLD(1, 20)
#define  WDMA_INT_MASK_1_FLD_RX_DONE_INT1                        REG_FLD(1, 17)
#define  WDMA_INT_MASK_1_FLD_RX_DONE_INT0                        REG_FLD(1, 16)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_DLY_INT3                    REG_FLD(1, 11)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_DLY_INT2                    REG_FLD(1, 10)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_DLY_INT1                    REG_FLD(1, 9)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_DLY_INT0                    REG_FLD(1, 8)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_INT3                        REG_FLD(1, 3)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_INT2                        REG_FLD(1, 2)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_INT1                        REG_FLD(1, 1)
#define  WDMA_INT_MASK_1_FLD_TX_DONE_INT0                        REG_FLD(1, 0)

#define  WDMA_INT_STATUS_2_FLD_RX_COHERENT                       REG_FLD(1, 31)
#define  WDMA_INT_STATUS_2_FLD_RX_DLY_INT                        REG_FLD(1, 30)
#define  WDMA_INT_STATUS_2_FLD_TX_COHERENT                       REG_FLD(1, 29)
#define  WDMA_INT_STATUS_2_FLD_TX_DLY_INT                        REG_FLD(1, 28)
#define  WDMA_INT_STATUS_2_FLD_RX_DONE_DLY_INT1                  REG_FLD(1, 21)
#define  WDMA_INT_STATUS_2_FLD_RX_DONE_DLY_INT0                  REG_FLD(1, 20)
#define  WDMA_INT_STATUS_2_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define  WDMA_INT_STATUS_2_FLD_RX_DONE_INT0                      REG_FLD(1, 16)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_DLY_INT3                  REG_FLD(1, 11)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_DLY_INT2                  REG_FLD(1, 10)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_DLY_INT1                  REG_FLD(1, 9)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_DLY_INT0                  REG_FLD(1, 8)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_INT3                      REG_FLD(1, 3)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_INT2                      REG_FLD(1, 2)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_INT1                      REG_FLD(1, 1)
#define  WDMA_INT_STATUS_2_FLD_TX_DONE_INT0                      REG_FLD(1, 0)

#define  WDMA_INT_MASK_2_FLD_RX_COHERENT                         REG_FLD(1, 31)
#define  WDMA_INT_MASK_2_FLD_RX_DLY_INT                          REG_FLD(1, 30)
#define  WDMA_INT_MASK_2_FLD_TX_COHERENT                         REG_FLD(1, 29)
#define  WDMA_INT_MASK_2_FLD_TX_DLY_INT                          REG_FLD(1, 28)
#define  WDMA_INT_MASK_2_FLD_RX_DONE_DLY_INT1                    REG_FLD(1, 21)
#define  WDMA_INT_MASK_2_FLD_RX_DONE_DLY_INT0                    REG_FLD(1, 20)
#define  WDMA_INT_MASK_2_FLD_RX_DONE_INT1                        REG_FLD(1, 17)
#define  WDMA_INT_MASK_2_FLD_RX_DONE_INT0                        REG_FLD(1, 16)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_DLY_INT3                    REG_FLD(1, 11)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_DLY_INT2                    REG_FLD(1, 10)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_DLY_INT1                    REG_FLD(1, 9)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_DLY_INT0                    REG_FLD(1, 8)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_INT3                        REG_FLD(1, 3)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_INT2                        REG_FLD(1, 2)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_INT1                        REG_FLD(1, 1)
#define  WDMA_INT_MASK_2_FLD_TX_DONE_INT0                        REG_FLD(1, 0)

#define  WDMA_INT_STATUS_3_FLD_RX_COHERENT                       REG_FLD(1, 31)
#define  WDMA_INT_STATUS_3_FLD_RX_DLY_INT                        REG_FLD(1, 30)
#define  WDMA_INT_STATUS_3_FLD_TX_COHERENT                       REG_FLD(1, 29)
#define  WDMA_INT_STATUS_3_FLD_TX_DLY_INT                        REG_FLD(1, 28)
#define  WDMA_INT_STATUS_3_FLD_RX_DONE_DLY_INT1                  REG_FLD(1, 21)
#define  WDMA_INT_STATUS_3_FLD_RX_DONE_DLY_INT0                  REG_FLD(1, 20)
#define  WDMA_INT_STATUS_3_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define  WDMA_INT_STATUS_3_FLD_RX_DONE_INT0                      REG_FLD(1, 16)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_DLY_INT3                  REG_FLD(1, 11)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_DLY_INT2                  REG_FLD(1, 10)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_DLY_INT1                  REG_FLD(1, 9)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_DLY_INT0                  REG_FLD(1, 8)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_INT3                      REG_FLD(1, 3)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_INT2                      REG_FLD(1, 2)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_INT1                      REG_FLD(1, 1)
#define  WDMA_INT_STATUS_3_FLD_TX_DONE_INT0                      REG_FLD(1, 0)

#define  WDMA_INT_MASK_3_FLD_RX_COHERENT                         REG_FLD(1, 31)
#define  WDMA_INT_MASK_3_FLD_RX_DLY_INT                          REG_FLD(1, 30)
#define  WDMA_INT_MASK_3_FLD_TX_COHERENT                         REG_FLD(1, 29)
#define  WDMA_INT_MASK_3_FLD_TX_DLY_INT                          REG_FLD(1, 28)
#define  WDMA_INT_MASK_3_FLD_RX_DONE_DLY_INT1                    REG_FLD(1, 21)
#define  WDMA_INT_MASK_3_FLD_RX_DONE_DLY_INT0                    REG_FLD(1, 20)
#define  WDMA_INT_MASK_3_FLD_RX_DONE_INT1                        REG_FLD(1, 17)
#define  WDMA_INT_MASK_3_FLD_RX_DONE_INT0                        REG_FLD(1, 16)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_DLY_INT3                    REG_FLD(1, 11)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_DLY_INT2                    REG_FLD(1, 10)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_DLY_INT1                    REG_FLD(1, 9)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_DLY_INT0                    REG_FLD(1, 8)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_INT3                        REG_FLD(1, 3)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_INT2                        REG_FLD(1, 2)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_INT1                        REG_FLD(1, 1)
#define  WDMA_INT_MASK_3_FLD_TX_DONE_INT0                        REG_FLD(1, 0)

#define  WDMA_TX_DELAY_INT_CFG_0_FLD_RING1_TXDLY_INT_EN          REG_FLD(1, 31)
#define  WDMA_TX_DELAY_INT_CFG_0_FLD_RING1_TXMAX_PINT            REG_FLD(7, 24)
#define  WDMA_TX_DELAY_INT_CFG_0_FLD_RING1_TXMAX_PTIME           REG_FLD(8, 16)
#define  WDMA_TX_DELAY_INT_CFG_0_FLD_RING0_TXDLY_INT_EN          REG_FLD(1, 15)
#define  WDMA_TX_DELAY_INT_CFG_0_FLD_RING0_TXMAX_PINT            REG_FLD(7, 8)
#define  WDMA_TX_DELAY_INT_CFG_0_FLD_RING0_TXMAX_PTIME           REG_FLD(8, 0)

#define  WDMA_TX_DELAY_INT_CFG_1_FLD_RING3_TXDLY_INT_EN          REG_FLD(1, 31)
#define  WDMA_TX_DELAY_INT_CFG_1_FLD_RING3_TXMAX_PINT            REG_FLD(7, 24)
#define  WDMA_TX_DELAY_INT_CFG_1_FLD_RING3_TXMAX_PTIME           REG_FLD(8, 16)
#define  WDMA_TX_DELAY_INT_CFG_1_FLD_RING2_TXDLY_INT_EN          REG_FLD(1, 15)
#define  WDMA_TX_DELAY_INT_CFG_1_FLD_RING2_TXMAX_PINT            REG_FLD(7, 8)
#define  WDMA_TX_DELAY_INT_CFG_1_FLD_RING2_TXMAX_PTIME           REG_FLD(8, 0)

#define  WDMA_RX_DELAY_INT_CFG_0_FLD_RING1_RXDLY_INT_EN          REG_FLD(1, 31)
#define  WDMA_RX_DELAY_INT_CFG_0_FLD_RING1_RXMAX_PINT            REG_FLD(7, 24)
#define  WDMA_RX_DELAY_INT_CFG_0_FLD_RING1_RXMAX_PTIME           REG_FLD(8, 16)
#define  WDMA_RX_DELAY_INT_CFG_0_FLD_RING0_RXDLY_INT_EN          REG_FLD(1, 15)
#define  WDMA_RX_DELAY_INT_CFG_0_FLD_RING0_RXMAX_PINT            REG_FLD(7, 8)
#define  WDMA_RX_DELAY_INT_CFG_0_FLD_RING0_RXMAX_PTIME           REG_FLD(8, 0)

#define  WDMA_RX_FC_CFG_0_FLD_RING0_FC_DASRT_THRES               REG_FLD(12, 16)
#define  WDMA_RX_FC_CFG_0_FLD_RING0_FC_ASRT_THRES                REG_FLD(12, 0)

#define  WDMA_RX_FC_CFG_1_FLD_RING1_FC_DASRT_THRES               REG_FLD(12, 16)
#define  WDMA_RX_FC_CFG_1_FLD_RING1_FC_ASRT_THRES                REG_FLD(12, 0)

#define  WDMA_PREF_TX_CFG_FLD_AXI_ULTRA                          REG_FLD(2, 22)
#define  WDMA_PREF_TX_CFG_FLD_LOW_THRES                          REG_FLD(6, 16)
#define  WDMA_PREF_TX_CFG_FLD_CURR_STATE                         REG_FLD(3, 13)
#define  WDMA_PREF_TX_CFG_FLD_BURST_SIZE                         REG_FLD(5, 8)
#define  WDMA_PREF_TX_CFG_FLD_AXI_RRESP_ERR                      REG_FLD(1, 7)
#define  WDMA_PREF_TX_CFG_FLD_RD_BND_4KB_BST                     REG_FLD(1, 6)
#define  WDMA_PREF_TX_CFG_FLD_WR_BND_4KB_BST                     REG_FLD(1, 5)
#define  WDMA_PREF_TX_CFG_FLD_DDONE_POLARITY                     REG_FLD(1, 4)
#define  WDMA_PREF_TX_CFG_FLD_DDONE_CHK                          REG_FLD(1, 3)
#define  WDMA_PREF_TX_CFG_FLD_DMAD_SIZE                          REG_FLD(1, 2)
#define  WDMA_PREF_TX_CFG_FLD_BUSY                               REG_FLD(1, 1)
#define  WDMA_PREF_TX_CFG_FLD_PREF_EN                            REG_FLD(1, 0)

#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING1_FREE_CNT               REG_FLD(6, 26)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING1_USED_CNT               REG_FLD(6, 20)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING1_EMPTY                  REG_FLD(1, 18)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING1_FULL                   REG_FLD(1, 17)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING1_CLEAR                  REG_FLD(1, 16)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING0_FREE_CNT               REG_FLD(6, 10)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING0_USED_CNT               REG_FLD(6, 4)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING0_EMPTY                  REG_FLD(1, 2)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING0_FULL                   REG_FLD(1, 1)
#define  WDMA_PREF_TX_FIFO_CFG0_FLD_RING0_CLEAR                  REG_FLD(1, 0)

#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING3_FREE_CNT               REG_FLD(6, 26)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING3_USED_CNT               REG_FLD(6, 20)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING3_EMPTY                  REG_FLD(1, 18)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING3_FULL                   REG_FLD(1, 17)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING3_CLEAR                  REG_FLD(1, 16)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING2_FREE_CNT               REG_FLD(6, 10)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING2_USED_CNT               REG_FLD(6, 4)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING2_EMPTY                  REG_FLD(1, 2)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING2_FULL                   REG_FLD(1, 1)
#define  WDMA_PREF_TX_FIFO_CFG1_FLD_RING2_CLEAR                  REG_FLD(1, 0)

#define  WDMA_PREF_RX_CFG_FLD_AXI_ULTRA                          REG_FLD(2, 22)
#define  WDMA_PREF_RX_CFG_FLD_LOW_THRES                          REG_FLD(6, 16)
#define  WDMA_PREF_RX_CFG_FLD_CURR_STATE                         REG_FLD(3, 13)
#define  WDMA_PREF_RX_CFG_FLD_BURST_SIZE                         REG_FLD(5, 8)
#define  WDMA_PREF_RX_CFG_FLD_AXI_RRESP_ERR                      REG_FLD(1, 7)
#define  WDMA_PREF_RX_CFG_FLD_RD_BND_4KB_BST                     REG_FLD(1, 6)
#define  WDMA_PREF_RX_CFG_FLD_WR_BND_4KB_BST                     REG_FLD(1, 5)
#define  WDMA_PREF_RX_CFG_FLD_DDONE_POLARITY                     REG_FLD(1, 4)
#define  WDMA_PREF_RX_CFG_FLD_DDONE_CHK                          REG_FLD(1, 3)
#define  WDMA_PREF_RX_CFG_FLD_DMAD_SIZE                          REG_FLD(1, 2)
#define  WDMA_PREF_RX_CFG_FLD_BUSY                               REG_FLD(1, 1)
#define  WDMA_PREF_RX_CFG_FLD_PREF_EN                            REG_FLD(1, 0)

#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING1_FREE_CNT               REG_FLD(6, 26)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING1_USED_CNT               REG_FLD(6, 20)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING1_EMPTY                  REG_FLD(1, 18)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING1_FULL                   REG_FLD(1, 17)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING1_CLEAR                  REG_FLD(1, 16)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING0_FREE_CNT               REG_FLD(6, 10)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING0_USED_CNT               REG_FLD(6, 4)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING0_EMPTY                  REG_FLD(1, 2)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING0_FULL                   REG_FLD(1, 1)
#define  WDMA_PREF_RX_FIFO_CFG0_FLD_RING0_CLEAR                  REG_FLD(1, 0)

#define  WDMA_PREF_SIDX_CFG_FLD_MON_SEL                          REG_FLD(3, 16)
#define  WDMA_PREF_SIDX_CFG_FLD_RX_RING1_SIDX_OW                 REG_FLD(1, 13)
#define  WDMA_PREF_SIDX_CFG_FLD_RX_RING0_SIDX_OW                 REG_FLD(1, 12)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING3_SIDX_OW                 REG_FLD(1, 11)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING2_SIDX_OW                 REG_FLD(1, 10)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING1_SIDX_OW                 REG_FLD(1, 9)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING0_SIDX_OW                 REG_FLD(1, 8)
#define  WDMA_PREF_SIDX_CFG_FLD_RX_RING1_SIDX_CLR                REG_FLD(1, 5)
#define  WDMA_PREF_SIDX_CFG_FLD_RX_RING0_SIDX_CLR                REG_FLD(1, 4)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING3_SIDX_CLR                REG_FLD(1, 3)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING2_SIDX_CLR                REG_FLD(1, 2)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING1_SIDX_CLR                REG_FLD(1, 1)
#define  WDMA_PREF_SIDX_CFG_FLD_TX_RING0_SIDX_CLR                REG_FLD(1, 0)

#define  WDMA_PREF_SIDX_MON_FLD_START_IDX                        REG_FLD(16, 0)

#define  WDMA_PREF_SIDX_OW_FLD_START_IDX_OW_VAL                  REG_FLD(16, 0)

#define  WDMA_XFER_CNT_CFG_FLD_MON_SEL                           REG_FLD(4, 28)
#define  WDMA_XFER_CNT_CFG_FLD_PKT_CNT_SAT                       REG_FLD(1, 9)
#define  WDMA_XFER_CNT_CFG_FLD_BYTE_CNT_SAT                      REG_FLD(1, 8)
#define  WDMA_XFER_CNT_CFG_FLD_CLR_CTRL                          REG_FLD(1, 7)
#define  WDMA_XFER_CNT_CFG_FLD_BYTE_CNT_UNIT                     REG_FLD(3, 4)

#define  WDMA_XFER_CNT_MON0_FLD_MON                              REG_FLD(32, 0)

#define  WDMA_XFER_CNT_MON1_FLD_MON                              REG_FLD(32, 0)

#define  WDMA_RX_ULTRA_CFG_FLD_AXI_ULTRA_EN                      REG_FLD(1, 31)
#define  WDMA_RX_ULTRA_CFG_FLD_AXI_ULTRA_THRES                   REG_FLD(11, 16)
#define  WDMA_RX_ULTRA_CFG_FLD_AXI_PREULTRA_EN                   REG_FLD(1, 15)
#define  WDMA_RX_ULTRA_CFG_FLD_AXI_PREULTRA_THRES                REG_FLD(11, 0)

#define  WDMA_WRBK_TX_CFG_FLD_WRBK_EN                            REG_FLD(1, 30)
#define  WDMA_WRBK_TX_CFG_FLD_MAX_PENDING_TIME                   REG_FLD(8, 22)
#define  WDMA_WRBK_TX_CFG_FLD_FLUSH_TIMER_EN                     REG_FLD(1, 21)
#define  WDMA_WRBK_TX_CFG_FLD_WRBK_THRES                         REG_FLD(6, 14)
#define  WDMA_WRBK_TX_CFG_FLD_CURR_STATE                         REG_FLD(3, 11)
#define  WDMA_WRBK_TX_CFG_FLD_BURST_SIZE                         REG_FLD(5, 6)
#define  WDMA_WRBK_TX_CFG_FLD_AXI_ULTRA                          REG_FLD(2, 4)
#define  WDMA_WRBK_TX_CFG_FLD_RD_BND_4KB_BST                     REG_FLD(1, 3)
#define  WDMA_WRBK_TX_CFG_FLD_WR_BND_4KB_BST                     REG_FLD(1, 2)
#define  WDMA_WRBK_TX_CFG_FLD_DMAD_SIZE                          REG_FLD(1, 1)
#define  WDMA_WRBK_TX_CFG_FLD_BUSY                               REG_FLD(1, 0)

#define  WDMA_WRBK_TX_FIFO_CFG0_FLD_RING0_FREE_CNT               REG_FLD(6, 20)
#define  WDMA_WRBK_TX_FIFO_CFG0_FLD_RING0_USED_CNT               REG_FLD(6, 4)
#define  WDMA_WRBK_TX_FIFO_CFG0_FLD_RING0_EMPTY                  REG_FLD(1, 2)
#define  WDMA_WRBK_TX_FIFO_CFG0_FLD_RING0_FULL                   REG_FLD(1, 1)
#define  WDMA_WRBK_TX_FIFO_CFG0_FLD_RING0_CLEAR                  REG_FLD(1, 0)

#define  WDMA_WRBK_TX_FIFO_CFG1_FLD_RING1_FREE_CNT               REG_FLD(6, 20)
#define  WDMA_WRBK_TX_FIFO_CFG1_FLD_RING1_USED_CNT               REG_FLD(6, 4)
#define  WDMA_WRBK_TX_FIFO_CFG1_FLD_RING1_EMPTY                  REG_FLD(1, 2)
#define  WDMA_WRBK_TX_FIFO_CFG1_FLD_RING1_FULL                   REG_FLD(1, 1)
#define  WDMA_WRBK_TX_FIFO_CFG1_FLD_RING1_CLEAR                  REG_FLD(1, 0)

#define  WDMA_WRBK_TX_FIFO_CFG2_FLD_RING2_FREE_CNT               REG_FLD(6, 20)
#define  WDMA_WRBK_TX_FIFO_CFG2_FLD_RING2_USED_CNT               REG_FLD(6, 4)
#define  WDMA_WRBK_TX_FIFO_CFG2_FLD_RING2_EMPTY                  REG_FLD(1, 2)
#define  WDMA_WRBK_TX_FIFO_CFG2_FLD_RING2_FULL                   REG_FLD(1, 1)
#define  WDMA_WRBK_TX_FIFO_CFG2_FLD_RING2_CLEAR                  REG_FLD(1, 0)

#define  WDMA_WRBK_TX_FIFO_CFG3_FLD_RING3_FREE_CNT               REG_FLD(6, 20)
#define  WDMA_WRBK_TX_FIFO_CFG3_FLD_RING3_USED_CNT               REG_FLD(6, 4)
#define  WDMA_WRBK_TX_FIFO_CFG3_FLD_RING3_EMPTY                  REG_FLD(1, 2)
#define  WDMA_WRBK_TX_FIFO_CFG3_FLD_RING3_FULL                   REG_FLD(1, 1)
#define  WDMA_WRBK_TX_FIFO_CFG3_FLD_RING3_CLEAR                  REG_FLD(1, 0)

#define  WDMA_WRBK_RX_CFG_FLD_WRBK_EN                            REG_FLD(1, 30)
#define  WDMA_WRBK_RX_CFG_FLD_MAX_PENDING_TIME                   REG_FLD(8, 22)
#define  WDMA_WRBK_RX_CFG_FLD_FLUSH_TIMER_EN                     REG_FLD(1, 21)
#define  WDMA_WRBK_RX_CFG_FLD_WRBK_THRES                         REG_FLD(6, 14)
#define  WDMA_WRBK_RX_CFG_FLD_CURR_STATE                         REG_FLD(3, 11)
#define  WDMA_WRBK_RX_CFG_FLD_BURST_SIZE                         REG_FLD(5, 6)
#define  WDMA_WRBK_RX_CFG_FLD_AXI_ULTRA                          REG_FLD(2, 4)
#define  WDMA_WRBK_RX_CFG_FLD_RD_BND_4KB_BST                     REG_FLD(1, 3)
#define  WDMA_WRBK_RX_CFG_FLD_WR_BND_4KB_BST                     REG_FLD(1, 2)
#define  WDMA_WRBK_RX_CFG_FLD_DMAD_SIZE                          REG_FLD(1, 1)
#define  WDMA_WRBK_RX_CFG_FLD_BUSY                               REG_FLD(1, 0)

#define  WDMA_WRBK_RX_FIFO_CFG0_FLD_RING0_FREE_CNT               REG_FLD(6, 20)
#define  WDMA_WRBK_RX_FIFO_CFG0_FLD_RING0_USED_CNT               REG_FLD(6, 4)
#define  WDMA_WRBK_RX_FIFO_CFG0_FLD_RING0_EMPTY                  REG_FLD(1, 2)
#define  WDMA_WRBK_RX_FIFO_CFG0_FLD_RING0_FULL                   REG_FLD(1, 1)
#define  WDMA_WRBK_RX_FIFO_CFG0_FLD_RING0_CLEAR                  REG_FLD(1, 0)

#define  WDMA_WRBK_RX_FIFO_CFG1_FLD_RING1_FREE_CNT               REG_FLD(6, 20)
#define  WDMA_WRBK_RX_FIFO_CFG1_FLD_RING1_USED_CNT               REG_FLD(6, 4)
#define  WDMA_WRBK_RX_FIFO_CFG1_FLD_RING1_EMPTY                  REG_FLD(1, 2)
#define  WDMA_WRBK_RX_FIFO_CFG1_FLD_RING1_FULL                   REG_FLD(1, 1)
#define  WDMA_WRBK_RX_FIFO_CFG1_FLD_RING1_CLEAR                  REG_FLD(1, 0)

#define  WDMA_WRBK_SIDX_CFG_FLD_MON_SEL                          REG_FLD(3, 16)
#define  WDMA_WRBK_SIDX_CFG_FLD_RX_RING1_SIDX_OW                 REG_FLD(1, 13)
#define  WDMA_WRBK_SIDX_CFG_FLD_RX_RING0_SIDX_OW                 REG_FLD(1, 12)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING3_SIDX_OW                 REG_FLD(1, 11)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING2_SIDX_OW                 REG_FLD(1, 10)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING1_SIDX_OW                 REG_FLD(1, 9)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING0_SIDX_OW                 REG_FLD(1, 8)
#define  WDMA_WRBK_SIDX_CFG_FLD_RX_RING1_SIDX_CLR                REG_FLD(1, 5)
#define  WDMA_WRBK_SIDX_CFG_FLD_RX_RING0_SIDX_CLR                REG_FLD(1, 4)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING3_SIDX_CLR                REG_FLD(1, 3)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING2_SIDX_CLR                REG_FLD(1, 2)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING1_SIDX_CLR                REG_FLD(1, 1)
#define  WDMA_WRBK_SIDX_CFG_FLD_TX_RING0_SIDX_CLR                REG_FLD(1, 0)

#define  WDMA_WRBK_SIDX_MON_FLD_START_IDX                        REG_FLD(16, 0)

#define  WDMA_WRBK_SIDX_OW_FLD_START_IDX_OW_VAL                  REG_FLD(16, 0)

#define  WDMA_DBG_CFG_FLD_debug_sig_sel                          REG_FLD(4, 0)

#define  WDMA_TX_DBG_MON_0_FLD_debug_monitor_0                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_1_FLD_debug_monitor_1                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_2_FLD_debug_monitor_2                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_3_FLD_debug_monitor_3                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_4_FLD_debug_monitor_4                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_5_FLD_debug_monitor_5                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_6_FLD_debug_monitor_6                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_7_FLD_debug_monitor_7                   REG_FLD(32, 0)

#define  WDMA_TX_DBG_MON_8_FLD_debug_monitor_8                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_0_FLD_debug_monitor_0                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_1_FLD_debug_monitor_1                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_2_FLD_debug_monitor_2                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_3_FLD_debug_monitor_3                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_4_FLD_debug_monitor_4                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_5_FLD_debug_monitor_5                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_6_FLD_debug_monitor_6                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_7_FLD_debug_monitor_7                   REG_FLD(32, 0)

#define  WDMA_RX_DBG_MON_8_FLD_debug_monitor_8                   REG_FLD(32, 0)

#define  WDMA_RX_MULTI_ID_CFG_FLD_DBG_MON_SEL_DATA_CTRL          REG_FLD(5, 24)
#define  WDMA_RX_MULTI_ID_CFG_FLD_DBG_MON_SEL_FLAG               REG_FLD(4, 20)
#define  WDMA_RX_MULTI_ID_CFG_FLD_DBG_MON_SEL                    REG_FLD(4, 16)
#define  WDMA_RX_MULTI_ID_CFG_FLD_AXI_RRESP_ERR                  REG_FLD(1, 12)
#define  WDMA_RX_MULTI_ID_CFG_FLD_RREADY_ALWAYS_RDY              REG_FLD(1, 8)
#define  WDMA_RX_MULTI_ID_CFG_FLD_EN_DATA_RD                     REG_FLD(1, 7)
#define  WDMA_RX_MULTI_ID_CFG_FLD_EN_DATA_WR                     REG_FLD(1, 6)
#define  WDMA_RX_MULTI_ID_CFG_FLD_EN_BLOCK_FREE                  REG_FLD(1, 5)
#define  WDMA_RX_MULTI_ID_CFG_FLD_EN_BLOCK_RESV                  REG_FLD(1, 4)
#define  WDMA_RX_MULTI_ID_CFG_FLD_BUSY                           REG_FLD(1, 1)
#define  WDMA_RX_MULTI_ID_CFG_FLD_ENABLE                         REG_FLD(1, 0)

#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_TAIL_IDX_OW_VAL      REG_FLD(4, 28)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_TAIL_IDX_OW          REG_FLD(1, 26)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_TAIL_IDX_INC         REG_FLD(1, 25)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_TAIL_IDX_CLR         REG_FLD(1, 24)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_HEAD_IDX_OW_VAL      REG_FLD(4, 20)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_HEAD_IDX_OW          REG_FLD(1, 18)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_HEAD_IDX_INC         REG_FLD(1, 17)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_HEAD_IDX_CLR         REG_FLD(1, 16)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_TAIL_IDX             REG_FLD(4, 12)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_HEAD_IDX             REG_FLD(4, 8)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_UNDERFLOW            REG_FLD(1, 3)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_OVERFLOW             REG_FLD(1, 2)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_EMPTY                REG_FLD(1, 1)
#define  WDMA_RX_MULTI_ID_BLK_STS_FLD_BLOCK_FULL                 REG_FLD(1, 0)

#define  WDMA_RX_MULTI_ID_DATA_STS0_FLD_DATA_EMPTY               REG_FLD(16, 16)
#define  WDMA_RX_MULTI_ID_DATA_STS0_FLD_DATA_FULL                REG_FLD(16, 0)

#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA15_UD                REG_FLD(1, 31)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA14_UD                REG_FLD(1, 30)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA13_UD                REG_FLD(1, 29)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA12_UD                REG_FLD(1, 28)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA11_UD                REG_FLD(1, 27)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA10_UD                REG_FLD(1, 26)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA9_UD                 REG_FLD(1, 25)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA8_UD                 REG_FLD(1, 24)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA7_UD                 REG_FLD(1, 23)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA6_UD                 REG_FLD(1, 22)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA5_UD                 REG_FLD(1, 21)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA4_UD                 REG_FLD(1, 20)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA3_UD                 REG_FLD(1, 19)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA2_UD                 REG_FLD(1, 18)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA1_UD                 REG_FLD(1, 17)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA0_UD                 REG_FLD(1, 16)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA15_OV                REG_FLD(1, 15)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA14_OV                REG_FLD(1, 14)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA13_OV                REG_FLD(1, 13)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA12_OV                REG_FLD(1, 12)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA11_OV                REG_FLD(1, 11)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA10_OV                REG_FLD(1, 10)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA9_OV                 REG_FLD(1, 9)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA8_OV                 REG_FLD(1, 8)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA7_OV                 REG_FLD(1, 7)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA6_OV                 REG_FLD(1, 6)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA5_OV                 REG_FLD(1, 5)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA4_OV                 REG_FLD(1, 4)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA3_OV                 REG_FLD(1, 3)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA2_OV                 REG_FLD(1, 2)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA1_OV                 REG_FLD(1, 1)
#define  WDMA_RX_MULTI_ID_DATA_STS1_FLD_DATA0_OV                 REG_FLD(1, 0)

#define  WDMA_RX_MULTI_ID_DATA_STS2_FLD_DATA_TAIL_IDX_CLR        REG_FLD(16, 16)
#define  WDMA_RX_MULTI_ID_DATA_STS2_FLD_DATA_HEAD_IDX_CLR        REG_FLD(16, 0)

#define  WDMA_RX_MULTI_ID_DATA_STS3_FLD_FLAG_DLAST_CLR           REG_FLD(16, 16)
#define  WDMA_RX_MULTI_ID_DATA_STS3_FLD_IDX_OW_VAL               REG_FLD(5, 4)
#define  WDMA_RX_MULTI_ID_DATA_STS3_FLD_IDX_INC                  REG_FLD(1, 1)
#define  WDMA_RX_MULTI_ID_DATA_STS3_FLD_IDX_OW                   REG_FLD(1, 0)

#define  WDMA_RX_MULTI_ID_DBG_MON_FLD_DBG_MON                    REG_FLD(32, 0)











































































































































































































































































































































#ifdef __cplusplus
}
#endif

#endif // __WDMA0_v3_REGS_H__
