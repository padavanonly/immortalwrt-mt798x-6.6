/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	none_cmm.h

    Abstract:
	Common OS structure/definition in NONE OS whatever OS ABL.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */



#ifndef __NONE_CMM_H__
#define __NONE_CMM_H__

/* declare new chipset function here */
#define RTMP_DRV_NAME   "mt_drv"

/*****************************************************************************
 *	OS task related data structure and definitions
 ******************************************************************************/

/* rt_linux.h */
#define RTMP_OS_TASK				OS_TASK

#define RTMP_OS_TASK_GET(__pTask)					\
	(__pTask)

#define RTMP_OS_TASK_DATA_GET(__pTask)					\
	((__pTask)->priv)

#define RTMP_OS_TASK_IS_KILLED(__pTask)					\
	((__pTask)->task_killed)

#ifdef KTHREAD_SUPPORT
#define RTMP_OS_TASK_WAKE_UP(__pTask)					\
	WAKE_UP(pTask)
#else
#define RTMP_OS_TASK_WAKE_UP(__pTask)					\
	RTMP_SEM_EVENT_UP(&(pTask)->taskSema)
#endif /* KTHREAD_SUPPORT */

#ifdef KTHREAD_SUPPORT
#define RTMP_OS_TASK_LEGALITY(__pTask)					\
	((__pTask)->kthread_task != NULL)
#else
#define RTMP_OS_TASK_LEGALITY(__pTask)					\
	CHECK_PID_LEGALITY((__pTask)->taskPID)
#endif /* KTHREAD_SUPPORT */

/*****************************************************************************
 * Timer related definitions and data structures.
 ******************************************************************************/
/* rt_linux.h */
#define NDIS_MINIPORT_TIMER			OS_NDIS_MINIPORT_TIMER
#define RTMP_OS_TIMER				OS_TIMER

#define RTMP_OS_FREE_TIMER(__pAd)
#define RTMP_OS_FREE_LOCK(__pAd)
#define RTMP_OS_FREE_TASKLET(__pAd)
#define RTMP_OS_FREE_TASK(__pAd)
#define RTMP_OS_FREE_SEM(__pAd)
#define RTMP_OS_FREE_ATOMIC(__pAd)

/*****************************************************************************
 *	OS file operation related data structure definitions
 ******************************************************************************/
/* if you add any new type, please also modify RtmpOSFileOpen() */
#define RTMP_FILE_RDONLY			0x0F01
#define RTMP_FILE_WRONLY			0x0F02
#define RTMP_FILE_CREAT				0x0F03
#define RTMP_FILE_TRUNC				0x0F04
#define RTMP_FILE_APPEND			0x0F05

/* rt_linux.h */
#define RTMP_OS_FS_INFO				OS_FS_INFO

/*****************************************************************************
 *	OS semaphore related data structure and definitions
 ******************************************************************************/

/*****************************************************************************
 *	OS task related data structure and definitions
 ******************************************************************************/

/* rt_linux.h */
#define RTMP_NET_TASK_STRUCT		OS_NET_TASK_STRUCT
#define PRTMP_NET_TASK_STRUCT		POS_NET_TASK_STRUCT

#define RTMP_NAPI_STRUCT		OS_NAPI_STRUCT
#define RTMP_NET_STRUCT			OS_NET_STRUCT


typedef struct none_completion RTMP_OS_COMPLETION;


#define RTMP_OS_INIT_COMPLETION(__pCompletion)	\
	RtmpOsInitCompletion(__pCompletion)

#define RTMP_OS_REINIT_COMPLETION(__pCompletion)	\
	RtmpOsReinitCompletion(__pCompletion)

#define RTMP_OS_EXIT_COMPLETION(__pCompletion)	\
	RtmpOsExitCompletion(__pCompletion)

#define RTMP_OS_COMPLETE(__pCompletion)	\
	RtmpOsComplete(__pCompletion)

#define RTMP_OS_COMPLETION_DONE(__pCompletion)	\
	RtmpOsCompletionDone(__pCompletion)

#define RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(__pCompletion, __Timeout)	\
	RtmpOsWaitForCompletionTimeout(__pCompletion, __Timeout)

#define RTMP_OS_TASKLET_SCHE(__pTasklet)			\
	RtmpOsTaskletSche(__pTasklet)

#define RTMP_OS_TASKLET_INIT(__pAd, __pTasklet, __pFunc, __Data)	\
	RtmpOsTaskletInit(__pTasklet, __pFunc, __Data, NULL)

#define RTMP_OS_TASKLET_KILL(__pTasklet)			\
	RtmpOsTaskletKill(__pTasklet)

#define RTMP_NET_TASK_DATA_ASSIGN(__pTasklet, __Data)		\
	RtmpOsTaskletDataAssign(__pTasklet, __Data)

#define RTMP_OS_NAPI_SCHE(__pNapi)	RtmpOsTaskletSche(__pNapi)

/*****************************************************************************
 *	OS definition related data structure and definitions
 ******************************************************************************/


#define RTMP_OS_PCI_VENDOR_ID			PCI_VENDOR_ID
#define RTMP_OS_PCI_DEVICE_ID			PCI_DEVICE_ID

typedef unsigned long            ra_dma_addr_t;

/******************************************************************************
 *	Others
 ******************************************************************************/
#define APCLI_IF_UP_CHECK(pAd, ifidx)	\
	(RtmpOSNetDevIsUp((pAd)->StaCfg[(ifidx)].wdev.if_dev) == TRUE)

#ifdef RTMP_MAC_PCI
#ifdef MEMORY_OPTIMIZATION
#define MGMT_RING_SIZE          64/*32*/
#else
#ifdef WIFI_EAP_FEATURE
#define MGMT_RING_SIZE          512
#else /* WIFI_EAP_FEATURE */
#define MGMT_RING_SIZE          128
#endif /* !WIFI_EAP_FEATURE */
#endif
#endif

#define MAX_TX_PROCESS 512

#define RTMP_OS_NETDEV_SET_PRIV		RtmpOsSetNetDevPriv
#define RTMP_OS_NETDEV_GET_PRIV		RtmpOsGetNetDevPriv
#define RTMP_OS_NETDEV_SET_WDEV		RtmpOsSetNetDevWdev
#define RTMP_OS_NETDEV_GET_WDEV		RtmpOsGetNetDevWdev
#define RT_DEV_PRIV_FLAGS_GET		RtmpDevPrivFlagsGet
#define RT_DEV_PRIV_FLAGS_SET		RtmpDevPrivFlagsSet

struct work_struct {
	void			*funcPtr;
	unsigned long		data;
	unsigned long		taskStatus;
	char			taskName[8];
};

struct list_head {
	struct list_head *next, *prev;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct rcu_head {
	struct rcu_head *next;
	void *funcPtr;
};

#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;

	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = NULL;
	n->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

static inline void hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;

	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

static inline void hlist_add_before(struct hlist_node *n,
					struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

#define hlist_first_rcu(head)	(*((struct hlist_node **)(&(head)->first)))
#define hlist_next_rcu(node)	(*((struct hlist_node **)(&(node)->next)))
#define hlist_pprev_rcu(node)	(*((struct hlist_node **)((node)->pprev)))

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_entry_safe(ptr, type, member) \
			({ typeof(ptr) ____ptr = (ptr); \
			   ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
			})

#define hlist_for_each_entry_rcu(pos, head, member, cond...) \
		for (pos = hlist_entry_safe(hlist_first_rcu(head), \
				typeof(*(pos)), member); \
			pos; \
			pos = hlist_entry_safe(hlist_next_rcu( \
				&(pos)->member), typeof(*(pos)), member))

#define hlist_for_each_entry_safe(pos, n, head, member)	  \
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);\
	     pos && ({ n = pos->member.next; 1; });		  \
	     pos = hlist_entry_safe(n, typeof(*pos), member))


#define RTMP_HLIST_HEAD struct hlist_head
#define RTMP_HLIST_NODE struct hlist_node
#define RtmpHashHeadInit(h) INIT_HLIST_HEAD(h)
#define RtmpHashNodeInit(n) INIT_HLIST_NODE(n)
#define RtmpHashListDel(n) hlist_del(n)
#define RtmpHashListDelInit(n) hlist_del_init(n)
#define RtmpHashListAddHead(n, h) hlist_add_head(n, h)
#define RtmpHashListAddBefore(n, h) hlist_add_before(n, h)
#define RtmpHashListForEach(p, h, m) hlist_for_each_entry(p, h, m)
#define RtmpHashListForEachSafe(p, n, h, m) hlist_for_each_entry_safe(p, n, h, m)

static inline int list_empty(const struct list_head *head)
{
	return (head->next == head);
}

typedef struct os_lock  {
	unsigned long	lock;
	unsigned long	flags;
} spinlock_t;
typedef spinlock_t OS_LOCK;

#endif /* __NONE_CMM_H__ */


