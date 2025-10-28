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
/****************************************************************************
 ****************************************************************************

    Module Name:
	rt_linux_cmm.h

    Abstract:
	Common OS structure/definition in LINUX whatever OS ABL.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#ifndef __RT_LINUX_CMM_H__
#define __RT_LINUX_CMM_H__

/* declare new chipset function here */

#ifdef MT_MAC
#define RTMP_DRV_NAME   "mt_drv"
#else
#define RTMP_DRV_NAME	"rt2860"
#endif /* MT_MAC */

/*****************************************************************************
 *	OS task related data structure and definitions
 ******************************************************************************/
#define RTMP_OS_TASK_INIT(__pTask, __pTaskName, __pAd)		\
	RtmpOSTaskInit(__pTask, __pTaskName, __pAd, &(__pAd)->RscTaskMemList, &(__pAd)->RscSemMemList);

/* rt_linux.h */
#define RTMP_OS_TASK				OS_TASK

#define RTMP_OS_TASK_GET(__pTask)							\
	(__pTask)

#define RTMP_OS_TASK_DATA_GET(__pTask)						\
	((__pTask)->priv)

#define RTMP_OS_TASK_IS_KILLED(__pTask)						\
	((__pTask)->task_killed)

#ifdef KTHREAD_SUPPORT
#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	WAKE_UP(pTask);
#else
#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	RTMP_SEM_EVENT_UP(&(pTask)->taskSema);
#endif /* KTHREAD_SUPPORT */

#ifdef KTHREAD_SUPPORT
#define RTMP_OS_TASK_LEGALITY(__pTask)						\
	((__pTask)->kthread_task != NULL)
#else
#define RTMP_OS_TASK_LEGALITY(__pTask)						\
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

/* rt_linux.h */
#define RTMP_OS_FS_INFO				OS_FS_INFO

/*****************************************************************************
 *	OS semaphore related data structure and definitions
 ******************************************************************************/

#define NDIS_SPIN_LOCK							OS_NDIS_SPIN_LOCK
#define NdisAllocateSpinLock(__pReserved, __pLock)	OS_NdisAllocateSpinLock(__pLock)
#define NdisFreeSpinLock						OS_NdisFreeSpinLock
#define RTMP_SPIN_LOCK							OS_SPIN_LOCK
#define RTMP_SPIN_UNLOCK						OS_SPIN_UNLOCK
#define RTMP_SPIN_LOCK_IRQ						OS_SPIN_LOCK_IRQ
#define RTMP_SPIN_UNLOCK_IRQ					OS_SPIN_UNLOCK_IRQ
#define RTMP_SPIN_LOCK_IRQSAVE(__pLock, __pFlags) \
	spin_lock_irqsave((spinlock_t *)(__pLock), (*__pFlags))
#define RTMP_SPIN_UNLOCK_IRQRESTORE(__pLock, __pFlags) \
	spin_unlock_irqrestore((spinlock_t *)(__pLock), (*__pFlags))
#define RTMP_IRQ_LOCK							OS_IRQ_LOCK
#define RTMP_IRQ_UNLOCK							OS_IRQ_UNLOCK
#define RTMP_INT_LOCK							OS_INT_LOCK
#define RTMP_INT_UNLOCK							OS_INT_UNLOCK
#define RTMP_OS_SEM								OS_SEM
#define RTMP_OS_ATOMIC							atomic_t

#define NdisAcquireSpinLock						OS_SEM_LOCK
#define NdisReleaseSpinLock						OS_SEM_UNLOCK

#define RTMP_SEM_EVENT_INIT_LOCKED(__pSema, __pSemaList)	OS_SEM_EVENT_INIT_LOCKED(__pSema)
#define RTMP_SEM_EVENT_INIT(__pSema, __pSemaList)			OS_SEM_EVENT_INIT(__pSema)
#define RTMP_SEM_EVENT_DESTORY					OS_SEM_EVENT_DESTORY
#define RTMP_SEM_EVENT_WAIT						OS_SEM_EVENT_WAIT
#define RTMP_SEM_EVENT_UP						OS_SEM_EVENT_UP
#define RTMP_SEM_EVENT_TIMEOUT					OS_SEM_EVENT_TIMEOUT
#define RTMP_SEM_EVENT_TRYLOCK					OS_SEM_EVENT_TRYLOCK

#define RtmpMLMEUp								OS_RTMP_MlmeUp

#define RTMP_OS_ATMOIC_INIT(__pAtomic, __pAtomicList)
#define RTMP_OS_ATMOIC_DESTROY(__pAtomic)
#define RTMP_THREAD_PID_KILL(__PID)				KILL_THREAD_PID(__PID, SIGTERM, 1)

/*****************************************************************************
 *	OS task related data structure and definitions
 ******************************************************************************/

/* rt_linux.h */
#define RTMP_NET_TASK_STRUCT		OS_NET_TASK_STRUCT
#define PRTMP_NET_TASK_STRUCT		POS_NET_TASK_STRUCT

typedef struct completion RTMP_OS_COMPLETION;

#define RTMP_OS_INIT_COMPLETION(__pCompletion)	\
	init_completion(__pCompletion)

#define RTMP_OS_REINIT_COMPLETION(__pCompletion)	\
	reinit_completion(__pCompletion)

#define RTMP_OS_EXIT_COMPLETION(__pCompletion)	\
	complete(__pCompletion)

#define RTMP_OS_COMPLETE(__pCompletion)	\
	complete(__pCompletion)

#define RTMP_OS_COMPLETE_ALL(__pCompletion)	\
	complete_all(__pCompletion)

#define RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(__pCompletion, __Timeout)	\
	wait_for_completion_timeout(__pCompletion, __Timeout)

#ifdef WORKQUEUE_BH
#define RTMP_OS_TASKLET_SCHE(__pTasklet)							\
	schedule_work(__pTasklet)
#define RTMP_OS_TASKLET_INIT(__pAd, __pTasklet, __pFunc, __Data)	\
	INIT_WORK((struct work_struct *)__pTasklet, (work_func_t)__pFunc)
#define RTMP_OS_TASKLET_KILL(__pTasklet) \
	cancel_work_sync(__pTasklet)
#else
#define RTMP_OS_TASKLET_SCHE(__pTasklet)							\
	tasklet_hi_schedule(__pTasklet)
#define RTMP_OS_TASKLET_INIT(__pAd, __pTasklet, __pFunc, __Data)	\
	tasklet_init(__pTasklet, __pFunc, __Data)
#define RTMP_OS_TASKLET_KILL(__pTasklet)							\
	tasklet_kill(__pTasklet)
#endif /* WORKQUEUE_BH */

#define RTMP_NET_TASK_DATA_ASSIGN(__Tasklet, __Data)		\
	((__Tasklet)->data = (unsigned long)__Data)

/*****************************************************************************
 *	OS definition related data structure and definitions
 ******************************************************************************/


#define RTMP_OS_PCI_VENDOR_ID			PCI_VENDOR_ID
#define RTMP_OS_PCI_DEVICE_ID			PCI_DEVICE_ID

#define ra_dma_addr_t					dma_addr_t

/***********************************************************************************
 *	Others
 ***********************************************************************************/
#define APCLI_IF_UP_CHECK(pAd, ifidx) (RtmpOSNetDevIsUp((pAd)->StaCfg[(ifidx)].wdev.if_dev) == TRUE)

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

#endif /* __RT_LINUX_CMM_H__ */

