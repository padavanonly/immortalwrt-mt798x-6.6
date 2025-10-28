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
	rtmp_os.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RTMP_OS_H__
#define __RTMP_OS_H__

/* Driver Operators */
typedef int (*RTMP_PRINTK)(const char *ftm, ...);
typedef int (*RTMP_SNPRINTF)(char *, ULONG, const char *ftm, ...);

typedef struct _RTMP_OS_ABL_OPS {
	int (*ra_printk)(const char *ftm, ...);
	int (*ra_snprintf)(char *, ULONG, const char *ftm, ...);
} RTMP_OS_ABL_OPS;

extern RTMP_OS_ABL_OPS *pRaOsOps;

#define GNU_DISABLE_STACK_CHECK_START
#define GNU_DISABLE_STACK_CHECK_END
#define DO_PRAGMA(x)

#ifndef CONFIG_RTPCI_AP_RF_OFFSET
#define CONFIG_RTPCI_AP_RF_OFFSET 0
#endif

#ifdef LINUX
#include "os/rt_linux.h"
#endif /* LINUX */





#ifdef NONE
#include "os/rt_none.h"
#endif /* NONE */

#ifndef INLINE
#define INLINE inline
#endif

/* TODO: shiang, temporary put it here! */
#include "os/pkt_meta.h"

/*
	This data structure mainly strip some callback function defined in
	"struct net_device" in kernel source "include/linux/netdevice.h".

	The definition of this data structure may various depends on different
	OS. Use it carefully.
*/
typedef struct _RTMP_OS_NETDEV_OP_HOOK_ {
	void *open;
	void *stop;
	void *xmit;
	void *ioctl;
	void *get_stats;
	void *priv;
	void *get_wstats;
	void *iw_handler;
	void *wdev;
#ifdef HOSTAPD_MBSS_SUPPORT
	void *set_mac_addr;
#endif /*HOSTAPD_MBSS_SUPPORT*/
	int priv_flags;
	unsigned char devAddr[6];
	unsigned char devName[16];
	unsigned char needProtcted;
} RTMP_OS_NETDEV_OP_HOOK, *PRTMP_OS_NETDEV_OP_HOOK;


typedef enum _RTMP_TASK_STATUS_ {
	RTMP_TASK_STAT_UNKNOWN = 0,
	RTMP_TASK_STAT_INITED = 1,
	RTMP_TASK_STAT_RUNNING = 2,
	RTMP_TASK_STAT_STOPED = 4,
} RTMP_TASK_STATUS;
#define RTMP_TASK_CAN_DO_INSERT		(RTMP_TASK_STAT_INITED | RTMP_TASK_STAT_RUNNING)

#define RTMP_OS_TASK_NAME_LEN	16

/* used in UTIL/NETIF module */
typedef struct _RTMP_OS_TASK_ {
	char taskName[RTMP_OS_TASK_NAME_LEN];
	void *priv;
	/*unsigned long                 taskFlags; */
	RTMP_TASK_STATUS taskStatus;
#ifndef KTHREAD_SUPPORT
	RTMP_OS_SEM taskSema;
	RTMP_OS_PID taskPID;
#ifdef LINUX
	struct completion taskComplete;
#endif /* LINUX */
#endif
	unsigned char task_killed;
#ifdef KTHREAD_SUPPORT
	struct task_struct *kthread_task;
	wait_queue_head_t kthread_q;
	BOOLEAN kthread_running;
#endif
} OS_TASK;

#define RTMP_MATOpsInit(__pAd)
#define RTMP_MATPktRxNeedConvert(__pAd, __pDev)				\
	MATPktRxNeedConvert(__pAd, __pDev)
#define RTMP_MATEngineRxHandle(__pAd, __pPkt, __InfIdx)		\
	MATEngineRxHandle(__pAd, __pPkt, __InfIdx)

#ifndef LINUX
typedef struct _os_kref {
	NDIS_SPIN_LOCK lock;
    UCHAR refcount;
} os_kref;
#endif

void os_kref_init(os_kref *kref);
void os_kref_get(os_kref *kref);
int os_kref_put(os_kref *kref, void (*release)(os_kref *kref));

#endif /* __RMTP_OS_H__ */
