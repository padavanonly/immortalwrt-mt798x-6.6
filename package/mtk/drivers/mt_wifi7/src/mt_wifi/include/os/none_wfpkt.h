/**
 *
 * @file   wfpkt.h
 * @brief  Pulblic API for WFDMA packet management
 *
*/

#ifndef __NONE_WF_PKT_H__
#define __NONE_WF_PKT_H__


#include "none_type.h"
/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define NUM_WFDMA_RESRV_PKT_PER_TYPE 5

#define NUM_WFDMA_RESRV_PKT_HIF_TX_TYPE 60
#define NUM_WFDMA_RESRV_PKT_1600_TYPE   32

#define NUM_WFDMA_RESRV_PKTS  (NET_BUF_TYPES * NUM_WFDMA_RESRV_PKT_PER_TYPE \
					+ NUM_WFDMA_RESRV_PKT_HIF_TX_TYPE \
					+ NUM_WFDMA_RESRV_PKT_1600_TYPE)
#define NUM_WFDMA_TOTAL_PKT         \
				(NUM_WFDMA_TX_DESC\
				+ NUM_WFDMA_RX_DESC + NUM_WFDMA_RESRV_PKTS)
#define NUM_WFDMA_TOTAL_PKT_BUF NUM_WFDMA_TOTAL_PKT

#define MAX_WFDMA_PKT_BUF_MEM   (1024*200)

#define MAX_HOST_CTXD_NUM 4

#define MAX_LMAC_CTXD_NUM 4


/* The size should be 8-byte align in WFDMA falcon */
#define NET_BUF_0_SIZE              (0)
#define NET_BUF_64_SIZE             (64)
#define NET_BUF_HIF_TX_PKT_SIZE             \
		(sizeof(HIF_TX_LONG_PKT_T) * MAX_HOST_CTXD_NUM)   // 148 Byte
#define NET_BUF_MAC_TX_PKT_SIZE             \
		(sizeof(MAC_TX_PKT_T) * MAX_LMAC_CTXD_NUM)        // 64 Byte
#define NET_BUF_MAC_RX_PKT_SIZE     sizeof(MAC_RXD_PKT_T)       // 92 Byte
#define NET_BUF_1600_SIZE           (1600)

#define NET_BUF_NOUSE       0xFF
#define NET_BUF_HIF_TX_PKT  0
#define NET_BUF_MAC_TX_PKT  1
#define NET_BUF_MAC_RX_PKT  2
#define NET_BUF_1600        3   /* CMD or Event Pkts */


#define NET_BUF_MIN_IDX     NET_BUF_HIF_TX_PKT
#define NET_BUF_MAX_IDX     NET_BUF_1600
#define NET_BUF_TYPES       (NET_BUF_MAX_IDX - NET_BUF_MIN_IDX + 1)



/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef struct head_list {
	struct node_obj *next;
	UINT32 qlen;
} HEAD_LIST_T, *P_HEAD_LIST_T;

typedef struct node_obj {
	struct node_obj *next;
	struct head_list *list;
} NODE_T, *P_NODE_T, **PP_NODE_T;


typedef struct net_buf_blk {
	/* These two member must be first */
	struct net_buf_blk *next;
	struct net_buf_blk_head *list;
	uint8_t *buf;
	uint16_t ref;
	uint16_t idx;
} WF_BUF_T, *P_WF_BUF_T, **PP_WF_BUF_T;

struct skb_shared_info {
	u_int8_t nr_frags;
};

typedef struct net_pkt_blk {
	struct none_net_device		*dev;
	struct net_pkt_blk *next;
	struct net_pkt_blk_head *list;
	uint8_t *head;
	uint8_t *data;
	struct net_buf_blk *buf_blk;
	uint8_t  cb[64];
	uint16_t len;
	uint8_t pkt_type;
	struct skb_shared_info shinfo;
	union {
		ktime_t		tstamp;
		u64		skb_mstamp_ns; /* earliest departure time */
	};
	uint16_t protocol;
} WF_PKT_T, *P_WF_PKT_T, **PP_WF_PKT_T;
#define sk_buff net_pkt_blk

typedef struct net_pkt_blk_head {
	struct net_pkt_blk *next;
	UINT32 qlen;
} WF_PKT_HEAD_T, *P_WF_PKT_HEAD_T;


typedef struct net_buf_blk_head {
	struct net_buf_blk *next;
	UINT32 qlen;
} WF_BUF_HEAD_T, *P_WF_BUF_HEAD_T;

typedef struct net_buf_type {
	UINT32 nblk;
	UINT32 buf_size;
	int8_t **mem;
	struct net_buf_blk_head freelist;
	struct net_pkt_blk_head pkt_list;
} WF_NET_BUF_T, *P_WF_NET_BUF_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
extern INT32 net_buf_alloc_size[];

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
*****************************************************************************
*/
INT32 halWfPktEnqueue(P_HEAD_LIST_T prList, P_NODE_T prNode);
INT32 halWfPktDequeue(P_HEAD_LIST_T prList, PP_NODE_T pprNode);
INT32 halWfPktAlloc(UINT32 u4SizeIdx, PP_WF_PKT_T pprWfPkt);
INT32 halWfPktFree(P_WF_PKT_T prWfPkt);
INT32 halWfPktNetBufJoinPktBlk(P_WF_BUF_T prWfPktBuf);
UINT32 halWfPktBufSize(INT32 i4SizeIdx);



/*
 * functions are not workable
 * just make compiler happy
 *
*/
static inline void *skb_push(struct net_pkt_blk *net_pkt, unsigned int len)
{
	net_pkt->data -= len;
	net_pkt->len  += len;
	return net_pkt->data;
}

static inline void *skb_put(struct net_pkt_blk *net_pkt, unsigned int len)
{
	net_pkt->len  += len;
	return (net_pkt->data + len);
}

#endif /* __NONE_WF_PKT_H__ */
