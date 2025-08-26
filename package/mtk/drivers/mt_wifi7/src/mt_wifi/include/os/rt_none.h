/****************************************************************************

    Module Name:
	rt_linux.h

    Abstract:
	Any OS related definition/MACRO is defined here.

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RT_NONE_H__
#define __RT_NONE_H__

#undef LINUX
#undef __KERNEL__


#include <stdarg.h>  // va_list
#include <stddef.h>  // size_t, NULL
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <execinfo.h>


#include "none_ctype.h"
#include "none_type.h"
#include "none_wfpkt.h"
#include "none_cmm.h"
#ifdef HWIFI_SUPPORT
#include "hwifi_main.h"
#endif /* HWIFI_SUPPORT */
#ifdef CFG_CFG80211_SUPPORT
#include "cfg80211.h"
#endif /* CFG_CFG80211_SUPPORT */

#undef GNU_DISABLE_STACK_CHECK_START
#define GNU_DISABLE_STACK_CHECK_START \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wstack-usage=\"")

#undef GNU_DISABLE_STACK_CHECK_END
#define GNU_DISABLE_STACK_CHECK_END \
	_Pragma("GCC diagnostic pop")

#undef DO_PRAGMA
#define DO_PRAGMA(x) _Pragma(#x)

#define _LINUX_BITOPS_H		// BIT(n)


#define WIRELESS_EXT	11
#define IW_SCAN_MAX_DATA  100 // lib/cmm_info.c

#define PACKET_BROADCAST	1		/* To all		*/
#define PACKET_MULTICAST	2		/* To group		*/

#define ETH_P_IPV6   0x86DD	// lib/rtmp.h    -- ConvertMulticastIP2MAC()
#define ETH_P_IP     0x0800     // lib/rtmp.h
#define ETH_P_ARP    0x0806     /* Address Resolution packet    */

struct ethhdr {
	unsigned char h_dest[ETH_ALEN];      // Destination MAC address
	unsigned char h_source[ETH_ALEN];    // Source MAC address
	unsigned short h_proto;                      // Protocol type
} __packed;

struct iphdr {
#ifndef CFG_BIG_ENDIAN
	UCHAR	ihl:4;
	UCHAR	version:4;
#else
	UCHAR	version:4;
	UCHAR	ihl:4;
#endif
	UCHAR	tos;
	UINT16	tot_len;
	UINT16	id;
	UINT16	frag_off;
	UCHAR	ttl;
	UCHAR	protocol;
	UINT16	check;
	UINT16	saddr;
	UINT32	daddr;
};

#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

struct udphdr {
	UINT16	source;
	UINT16	dest;
	UINT16	len;
	UINT16	check;
};

struct tcphdr {
	UINT16	source;
	UINT16	dest;
	UINT32	seq;
	UINT32	ack_seq;
#ifdef CFG_BIG_ENDIAN
	UINT16	doff:4,
		res1:4,
		cwr:1,
		ece:1,
		urg:1,
		ack:1,
		psh:1,
		rst:1,
		syn:1,
		fin:1;
#else
	UINT16	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		ece:1,
		cwr:1;
#endif
	UINT16	window;
	UINT16	check;
	UINT16	urg_ptr;
};

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define wait_queue_head_t int

typedef long ssize_t;
typedef long time_t;

#define GFP_ATOMIC	(1)

#ifndef CONFIG_GOOGLE_UT
#define INT_MAX                 ((int)(~0U>>1))
#define ULLONG_MAX		(~0ULL)
#endif /* !CONFIG_GOOGLE_UT */

enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};

int hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
			      int groupsize, char *linebuf, size_t linebuflen,
			      bool ascii);

#define pr_info(fmt, ...) \
	none_printk(fmt, ##__VA_ARGS__)

#define pr_info_ratelimited(fmt, ...) \
	none_printk(fmt, ##__VA_ARGS__)

#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })

long simple_strtol(const char *cp, char **endp, unsigned int base);
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);


int kstrtol(const char *s, unsigned int base, long *res);
int kstrtoul(const char *s, unsigned int base, unsigned long *res);
int kstrtou8(const char *s, unsigned int base, u8 *res);
int kstrtou16(const char *s, unsigned int base, u16 *res);
int kstrtou32(const char *s, unsigned int base, u32 *res);

INLINE unsigned int smp_processor_id(void);

/* math64.h */
/**
 * div64_u64 - unsigned 64bit divide with 64bit divisor
 */
INLINE u64 div64_u64(u64 dividend, u64 divisor);

/**
 * div_u64_rem - unsigned 64bit divide with 32bit divisor with remainder
 *
 * This is commonly provided by 32bit archs to provide an optimized 64bit
 * divide.
 */
INLINE u64 div_u64_rem(u64 dividend, u32 divisor, u32 *remainder);

INLINE u64 div_u64(u64 dividend, u32 divisor);

/*
 * bits ops
*/
#ifdef CONFIG_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif /* CONFIG_64BIT */

#define BITS_TO_LONGS(nr) \
	(((nr) + (8 * sizeof(unsigned long)) - 1) / (8 * sizeof(unsigned long)))

#define BIT_MASK(nr)		((1UL) << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)

/* linux/types.h */
//typedef int atomic_t;


#define PAGE_SIZE  (1<<12) // 4096


INLINE int atomic_read(atomic_t *v);
INLINE void atomic_set(atomic_t *v, int i);
INLINE void atomic_add(int i, atomic_t *v);
INLINE void atomic_sub(int i, atomic_t *v);
INLINE void atomic_inc(atomic_t *v);
INLINE void atomic_dec(atomic_t *v);
INLINE void set_bit(unsigned int nr,  unsigned long *p);
#define __set_bit set_bit
INLINE void clear_bit(unsigned int nr,  unsigned long *p);
INLINE int test_and_clear_bit(unsigned int nr,  unsigned long *p);
INLINE bool test_bit(long nr, const  unsigned long *addr);

#define panic(_x)
#define dump_stack()
#define EXPORT_SYMBOL(_x)
#define cpu_to_le32(_x)	(_x)
#define cpu_to_le16(_x) (_x)


extern unsigned long jiffies;

/* printk */
#define KERN_INFO
#define KERN_WARNING
#define printk	none_printk

/* barrier */

#define mmiowb()
#define wmb()		// memory barrier

#define phys_to_virt(_x)	(_x)


typedef void (*smp_call_func_t)(void *info);
INLINE int smp_call_function_single(
	int cpuid, smp_call_func_t func, void *info, int wait);

#define memcpy(t, f, n) __builtin_memcpy(t, f, n)
#define memset(s, c, count) __builtin_memset(s, c, count)
#define memcmp __builtin_memcmp
#define memmove __builtin_memmove
#define strlen  __builtin_strlen
#define snprintf __builtin_snprintf
#define sprintf  __builtin_sprintf
#define strncpy __builtin_strncpy
#define strncmp __builtin_strncmp
#define strstr	__builtin_strstr
#define strcmp __builtin_strcmp
#define sscanf __builtin_sscanf
#define vsnprintf __builtin_vsnprintf


#define strspn __builtin_strspn
#define strchr __builtin_strchr
#define strcat __builtin_strcat
#define strcpy __builtin_strcpy
#define strncat __builtin_strncat
#define ffs __builtin_ffs

#ifndef CONFIG_GOOGLE_UT
char *strpbrk(const char *cs, const char *ct);
#endif /* !CONFIG_GOOGLE_UT */
char *strsep(char **s, const char *ct);

#define true 1
#define false 0

/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/

#define L1_PROFILE_PATH	"/etc/wireless/l1profile.dat"
#define L1PROFILE_INDEX_LEN             10
#define	L1PROFILE_ATTRNAME_LEN          30
#define	L2PROFILE_PATH_LEN              80

#define DEFAULT_PROFILE_PATH0        "/etc/Wireless/RT2860AP/RT2860AP.dat"
#define DEFAULT_PROFILE_PATH1        "/etc/Wireless/iNIC/iNIC_ap.dat"
#ifndef AP_DRIVER_VERSION
#define AP_DRIVER_VERSION	"8.1.1.0"
#endif
#ifndef STA_DRIVER_VERSION
#define STA_DRIVER_VERSION           "8.1.1.0"
#endif

#ifdef WAPP_SUPPORT
#define WAPP_SUPPORT_VERSION         "v5.0.0.0"
#endif /* WAPP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
extern const struct iw_handler_def rt28xx_iw_handler_def;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
extern	const struct iw_handler_def rt28xx_ap_iw_handler_def;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

/***********************************************************************************
 *	Compiler related definitions
 ***********************************************************************************/
#define IN
#define OUT
#define INOUT
#define NDIS_STATUS		INT

#define DEVEXIT
#define DEVEXIT_P
#define DEVINIT
#define DEVINITDATA

/***********************************************************************************
 *	OS Specific definitions and data structures
***********************************************************************************/
typedef struct napi_struct	OS_NAPI_STRUCT;
typedef struct none_net_device	OS_NET_STRUCT;
typedef struct none_device	OS_DEVICE_STRUCT;

INLINE const char *dev_name(const struct none_device *dev);

typedef struct net_device_stats NET_DEV_STATS;
/* #ifdef RTMP_RBUS_SUPPORT */
/* typedef struct platform_device * PPCI_DEV; */
/* #else */
typedef struct none_pci_dev		*PPCI_DEV;
/* #endif */	/*RTMP_RBUS_SUPPORT*/
typedef struct none_net_device	*PNET_DEV;
typedef struct none_wireless_dev	*PWIRELESS_DEV;
typedef void				*PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		*PPNDIS_PACKET;
typedef ra_dma_addr_t			NDIS_PHYSICAL_ADDRESS;
typedef ra_dma_addr_t			*PNDIS_PHYSICAL_ADDRESS;
typedef void				*NDIS_HANDLE;
typedef char				*PNDIS_BUFFER;
typedef struct ifreq		NET_IOCTL;
typedef struct ifreq		*PNET_IOCTL;


typedef int RTMP_OS_PID;

typedef struct none_semaphore	OS_SEM;

typedef int (*HARD_START_XMIT_FUNC)(struct net_pkt_blk *skb,
				    struct none_net_device *net_dev);

#ifdef RTMP_MAC_PCI
#ifndef PCI_DEVICE
#define PCI_DEVICE(vend, dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif /* PCI_DEVICE */
#endif /* RTMP_MAC_PCI */

#define RT_MOD_INC_USE_COUNT()	do {} while (0)
#define RT_MOD_DEC_USE_COUNT() do {} while (0)

#define RTMP_INC_REF(_A)		0
#define RTMP_DEC_REF(_A)		0
#define RTMP_GET_REF(_A)		0

#if defined(CONFIG_FAST_NAT_SUPPORT)
#define RT_MOD_HNAT_DEREG(_net_dev) \
	do {\
		if (ppe_dev_unregister_hook != NULL) \
			ppe_dev_unregister_hook(_net_dev);	\
	} while (0)
#define RT_MOD_HNAT_REG(_net_dev) \
	do {\
		if (ppe_dev_register_hook != NULL) \
			ppe_dev_register_hook(_net_dev);	\
	} while (0)
#else
#define RT_MOD_HNAT_DEREG(_net_dev) do {} while (0)
#define RT_MOD_HNAT_REG(_net_dev) do {} while (0)
#endif /*CONFIG_FAST_NAT_SUPPORT*/

/***********************************************************************************
 *	Network related constant definitions
 ***********************************************************************************/
enum {
	ERROR_NO_RING = 1,
	ERROR_NO_RING_ALL,
	ERROR_NO_TOKEN,
	ERROR_NO_TX_RESOURCE,
	ERROR_CHECK_NEXT_TX_RESOURCE,
};

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define MAC_ADDR_LEN		6

#define NDIS_STATUS_SUCCESS                     0x00
#define NDIS_STATUS_FAILURE                     0x01
#define NDIS_STATUS_INVALID_DATA		0x02
#define NDIS_STATUS_RESOURCES                   0x03
#define NDIS_STATUS_PKT_REQUEUE			0x04
#define NDIS_STATUS_MORE_PROCESSING_REQUIRED             0x05
#define NDIS_STATUS_TIMEOUT                     0x06


#define NDIS_SET_PACKET_STATUS(_p, _status)			do {} while (0)
#define NdisWriteErrorLogEntry(_a, _b, _c, _d)		do {} while (0)

/* statistics counter */
#define STATS_INC_RX_PACKETS(_pAd, _dev)
#define STATS_INC_TX_PACKETS(_pAd, _dev)

#define STATS_INC_RX_BYTESS(_pAd, _dev, len)
#define STATS_INC_TX_BYTESS(_pAd, _dev, len)

#define STATS_INC_RX_ERRORS(_pAd, _dev)
#define STATS_INC_TX_ERRORS(_pAd, _dev)

#define STATS_INC_RX_DROPPED(_pAd, _dev)
#define STATS_INC_TX_DROPPED(_pAd, _dev)


/***********************************************************************************
 *	Ralink Specific network related constant definitions
 ***********************************************************************************/
#ifdef CONFIG_STA_SUPPORT
#define NDIS_PACKET_TYPE_DIRECTED	0
#define NDIS_PACKET_TYPE_MULTICAST	1
#define NDIS_PACKET_TYPE_BROADCAST	2
#define NDIS_PACKET_TYPE_ALL_MULTICAST	3
#define NDIS_PACKET_TYPE_PROMISCUOUS	4
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_VHT_AC
#define MAX_PACKETS_IN_QUEUE		1024 /*(512)*/
#else
#define MAX_PACKETS_IN_QUEUE		(512)
#endif /* DOT11_VHT_AC */


/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/


/***********************************************************************************
 *	OS file operation related data structure definitions
 ***********************************************************************************/
typedef struct none_file *RTMP_OS_FD;

typedef struct _OS_FS_INFO_ {
	int				fsuid;
	int				fsgid;
	int	fs;
} OS_FS_INFO;

typedef struct _RTMP_OS_FD {
	RTMP_OS_FD fsFd;
	OS_FS_INFO fsInfo;
	INT Status;
	UINT32 fsize;
} RTMP_OS_FD_EXT;

#define IS_FILE_OPEN_ERR(_fd)	((_fd) < 0)

/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
struct mutex {
};

struct delayed_work {
	struct work_struct work;
	struct workqueue_struct *wq;
	int cpu;
};

struct workqueue_struct {
	char name[IFNAMSIZ];
};

typedef struct completion OS_EVENT;
typedef NDIS_PACKET OS_PACKET;
typedef NDIS_PHYSICAL_ADDRESS OS_PHYSICAL_ADDRESS;

typedef OS_LOCK			OS_NDIS_SPIN_LOCK;
typedef struct workqueue_struct    OS_WORKQUEUE_STRUCT;
typedef struct delayed_work        OS_DELAYED_WORK;
typedef struct mutex		OS_NDIS_MUTEX;

/* The macro to get the current OS system time */
#define GET_CURRENT_SYSTIME(_systime_p)	{ *(_systime_p) = (OS_SYSTIME) 0xFFFF; }

#define DEFINE_SPINLOCK(lockname) \
    static spinlock_t lockname

/* */
/*  spin_lock enhanced for Nested spin lock */
/* */
#define OS_NdisAllocateSpinLock(__lock)		((void) __lock)
#define spin_lock_init(__lock)

#define OS_NdisFreeSpinLock(lock)	\
	do {} while (0)

#define spin_lock_bh(__lock)
#define spin_unlock_bh(__lock)
#define mutex_init(__lock)
#define mutex_lock(__lock)
#define mutex_unlock(__lock)

/* TODO: implement correct handle. */
#define INIT_WORK(__work, __work_fn)
#define schedule_work(__work)

/* sample, use semaphore lock to replace IRQ lock, 2007/11/15 */
#define OS_IRQ_LOCK(__lock, __irqflags)			\
	do {			\
		(void) __lock;	\
		__irqflags = 0;	\
	} while (0)

#define OS_IRQ_UNLOCK(__lock, __irqflags)			\
	do {			\
		(void) __lock;	\
		__irqflags = 0;	\
	} while (0)

#define OS_INT_LOCK(__lock, __irqflags)			\
	do {			\
		(void) __lock;	\
		__irqflags = 0;	\
	} while (0)

#define OS_INT_UNLOCK(__lock, __irqflags)		\
	do {			\
		(void) __lock;	\
		__irqflags = 0;	\
	} while (0)

#ifndef wait_event_interruptible_timeout
#define __wait_event_interruptible_timeout(wq, condition, ret) \
	do { \
		wait_queue_t __wait; \
		init_waitqueue_entry(&__wait, current); \
		add_wait_queue(&wq, &__wait); \
		for (;;) { \
			set_current_state(TASK_INTERRUPTIBLE); \
			if (condition) \
				break; \
			if (!signal_pending(current)) { \
				ret = schedule_timeout(ret); \
				if (!ret) \
					break; \
				continue; \
			} \
			ret = -ERESTARTSYS; \
			break; \
		} \
		current->state = TASK_RUNNING; \
		remove_wait_queue(&wq, &__wait); \
	} while (0)

#define wait_event_interruptible_timeout(wq, condition, timeout) \
	({ \
		long __ret = timeout; \
		if (!(condition)) \
			_wait_event_interruptible_timeout(wq, \
					condition, __ret); \
		__ret; \
	})
#endif

#define OS_SEM_EVENT_INIT_LOCKED(_pSema)	//sema_init((_pSema), 0)
#define OS_SEM_EVENT_INIT(_pSema)		//sema_init((_pSema), 1)
#define OS_SEM_EVENT_DESTORY(_pSema)		//do {} while (0)
#define OS_SEM_EVENT_WAIT(_pSema, _status)	\
		(_status = 0) //((_status) = down_interruptible((_pSema)))
#define OS_SEM_EVENT_UP(_pSema)			//up(_pSema)
#define OS_SEM_EVENT_TIMEOUT(_pSema, _time, _status)	//((_status) = down_timeout((_pSema), (_time)))
#define OS_SEM_EVENT_TRYLOCK(_pSema, _status)	//((_status) = down_trylock((_pSema)))

#define RTCMDUp					OS_RTCMDUp
#define NDIS_MUTEX				OS_NDIS_MUTEX
#define NDIS_SPIN_LOCK				OS_NDIS_SPIN_LOCK
#define NdisAllocateSpinLock(__pReserved, __pLock)	\
		OS_NdisAllocateSpinLock(__pLock)
#define NdisFreeSpinLock			OS_NdisFreeSpinLock
#define RTMP_SPIN_LOCK				OS_SPIN_LOCK
#define RTMP_SPIN_UNLOCK			OS_SPIN_UNLOCK
#define RTMP_SPIN_LOCK_IRQ			OS_SPIN_LOCK_IRQ
#define RTMP_SPIN_UNLOCK_IRQ			OS_SPIN_UNLOCK_IRQ
#define RTMP_SPIN_LOCK_IRQSAVE			OS_SPIN_LOCK_IRQSAVE
#define RTMP_SPIN_UNLOCK_IRQRESTORE		OS_SPIN_UNLOCK_IRQRESTORE
#define RTMP_IRQ_LOCK				OS_IRQ_LOCK
#define RTMP_IRQ_UNLOCK				OS_IRQ_UNLOCK
#define RTMP_INT_LOCK				OS_INT_LOCK
#define RTMP_INT_UNLOCK				OS_INT_UNLOCK
#define RTMP_OS_SEM				OS_SEM
#define RTMP_OS_ATOMIC				atomic_t
#define RTMP_OS_COMPLETE_ALL(__pCompletion)	//complete_all(__pCompletion)

#define NdisAcquireSpinLock			OS_SEM_LOCK
#define NdisReleaseSpinLock			OS_SEM_UNLOCK

#define RTMP_SEM_EVENT_INIT_LOCKED(__pSema, __pSemaList)	\
		OS_SEM_EVENT_INIT_LOCKED(__pSema)
#define RTMP_SEM_EVENT_INIT(__pSema, __pSemaList)	\
		OS_SEM_EVENT_INIT(__pSema)
#define RTMP_SEM_EVENT_DESTORY			OS_SEM_EVENT_DESTORY
#define RTMP_SEM_EVENT_WAIT			OS_SEM_EVENT_WAIT
#define RTMP_SEM_EVENT_UP			OS_SEM_EVENT_UP
#define RTMP_SEM_EVENT_TIMEOUT			OS_SEM_EVENT_TIMEOUT
#define RTMP_SEM_EVENT_TRYLOCK			OS_SEM_EVENT_TRYLOCK

#define RtmpMLMEUp				OS_RTMP_MlmeUp

#define RTMP_OS_ATMOIC_INIT(__pAtomic, __pAtomicList)
#define RTMP_OS_ATMOIC_DESTROY(__pAtomic)
#define RTMP_THREAD_PID_KILL(__PID)		\
		KILL_THREAD_PID(__PID, SIGTERM, 1)

#ifdef KTHREAD_SUPPORT
#define RTMP_WAIT_EVENT_INTERRUPTIBLE(_Status, _pTask) \
	do { \
		wait_event_interruptible(_pTask->kthread_q, \
		_pTask->kthread_running || kthread_should_stop()); \
		_pTask->kthread_running = FALSE; \
		if (kthread_should_stop()) {	\
			(_Status) = -1;		\
			break;			\
		} else				\
			(_Status) = 0;		\
	} while (0)
#endif

#ifdef KTHREAD_SUPPORT
#define RTMP_WAIT_EVENT_INTERRUPTIBLE_COND(_Status, _pTask, _Cond) \
	do { \
		wait_event_interruptible(_pTask->kthread_q,   \
		_pTask->kthread_running || kthread_should_stop() || (_Cond)); \
		_pTask->kthread_running = FALSE; \
		if (kthread_should_stop()) {	\
			(_Status) = -1;		\
			break;			\
		} else				\
			(_Status) = 0;		\
	} while (0)
#endif

#ifdef KTHREAD_SUPPORT
#define WAKE_UP(_pTask) \
	do { \
		if ((_pTask)->kthread_task) { \
			(_pTask)->kthread_running = TRUE; \
			wake_up(&(_pTask)->kthread_q); \
		} \
	} while (0)
#endif

#define OS_SCHEDULE()

#define rcu_assign_pointer(p, v) do {} while (0)
#define rcu_dereference(p) (p)

struct none_int_callback {
	void (*sem_lock_fn)(NDIS_SPIN_LOCK *_lock);
	void (*sem_unlock_fn)(NDIS_SPIN_LOCK *_lock);
};

extern struct none_int_callback g_int_callback;

#define OS_SEM_LOCK		g_int_callback.sem_lock_fn
#define OS_SEM_UNLOCK		g_int_callback.sem_unlock_fn
#define OS_NdisAcquireSpinLock		OS_SEM_LOCK
#define OS_NdisReleaseSpinLock		OS_SEM_UNLOCK

/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#define MEM_ALLOC_FLAG      (GFP_ATOMIC) /*(GFP_DMA | GFP_ATOMIC) */


#define vmalloc			malloc
#define vfree			free


#define NdisMoveMemory(Destination, Source, Length)	\
		memmove(Destination, Source, Length)
#define NdisCopyMemory(Destination, Source, Length)	\
		memcpy(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)		\
		memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)	\
		memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)	\
		memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)	\
		(!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	\
		(!memcmp(Source1, Source2, Length))

#define MlmeAllocateMemory(_pAd, _ppVA)		\
			os_alloc_mem(_pAd, _ppVA, MAX_MGMT_PKT_LEN)
#define MlmeFreeMemory(_pVA)			os_free_mem(_pVA)
#define kfree free

/* The macro to copy the MAC address */
#define COPY_MAC_ADDR(Addr1, Addr2)	memcpy((Addr1), (Addr2), MAC_ADDR_LEN)

/* The macro to check if two MAC addresses are equal */
#define EQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr)   \
	(!memcmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

/* The macro to check if two MAC addresses are equal */
#define EQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr)   \
	(!memcmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

/* The macro to check if two MAC addresses are not equal */
#define UNEQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr) \
	(memcmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

#define CONTAINER_OF(ptr, type, member)   none_container_of(ptr, type, member)


#define ether_addr_equal(__DestAddr, __SrcAddr) EQUAL_MAC_ADDR(__DestAddr, __SrcAddr)
/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
#define RTMP_OS_MGMT_TASK_FLAGS	CLONE_VM

#define	THREAD_PID_INIT_VALUE	-1
#define RT_GET_OS_PID(_x, _pid)		(_x = _pid)
#define RTMP_GET_OS_PID(_x, _pid)	(_x = _pid)
#define	GET_PID_NUMBER(_v)	(_v)
#define CHECK_PID_LEGALITY(_pid)	((_pid) >= 0)
#define KILL_THREAD_PID(_A, _B, _C)	//kill_proc((_A), (_B), (_C))

#define ATE_KILL_THREAD_PID(PID)		KILL_THREAD_PID(PID, SIGTERM, 1)

typedef int (*cast_fn)(void *);
typedef INT (*RTMP_OS_TASK_CALLBACK)(ULONG);

struct none_net_task {
	void					*funcPtr;
	unsigned long				data;
	unsigned long		taskStatus;
	char						taskName[8];
};

typedef struct none_net_task  OS_NET_TASK_STRUCT;
typedef struct none_net_task  *POS_NET_TASK_STRUCT;

/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
#define NONE_HZ	(1000)
#define OS_HZ	NONE_HZ
#define HZ	NONE_HZ

struct RtmpNoneTimer {
	unsigned int	sys_clk;
	unsigned int	counter_hdl;
	unsigned int	alarm_hdl;
	unsigned int	alarm_obj;
	PVOID		data;
};

typedef struct RtmpNoneTimer	NDIS_MINIPORT_TIMER;
typedef struct RtmpNoneTimer	RTMP_OS_TIMER;

typedef void (*TIMER_FUNCTION)(struct RtmpNoneTimer *);

#define OS_WAIT(_time) \
	{	\
			RtmpusecDelay(_time * 1000);\
	}

#define typecheck(type, x) \
	({      type __dummy; \
		typeof(x) __dummy2; \
		(void)(&__dummy == &__dummy2); \
		1; \
	})
#define RTMP_TIME_AFTER_EQ(a, b)	\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(a) - (long)(b) >= 0))

#define RTMP_TIME_BEFORE(a, b)	RTMP_TIME_AFTER_EQ(b, a)
#define RTMP_TIME_AFTER(a, b)   !RTMP_TIME_BEFORE(a, b)

#define ONE_TICK 1

INLINE void NdisGetSystemUpTime(ULONG *time);

/* Subtract two ktime_t variables. rem = lhs -rhs: */
#define ktime_sub(lhs, rhs)	((lhs) - (rhs))


/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/

struct os_cookie {
#ifdef RTMP_MAC_PCI
	PPCI_DEV pci_dev;
	PPCI_DEV parent_pci_dev;
	USHORT DeviceID;
#endif /* RTMP_MAC_PCI */

	OS_DEVICE_STRUCT * pDev;
	UINT32 pAd_va;

#if defined(RTMP_MAC_PCI) || defined(RTMP_MAC_USB)

#ifdef UAPSD_SUPPORT
	RTMP_NET_TASK_STRUCT uapsd_eosp_sent_task;
#endif /* UAPSD_SUPPORT */

#endif

	RTMP_OS_PID			apd_pid; /*802.1x daemon pid */
	unsigned long			apd_pid_nr;
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	/*	RT_SIGNAL_STRUC			RTSignal; */
	RTMP_OS_PID			IappPid; /*IAPP daemon pid */
	unsigned long			IappPid_nr;
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	INT						ioctl_if_type;
	INT					ioctl_if;
	struct _SECURITY_CONFIG *pSecConfig;
#ifdef FW_DUMP_SUPPORT
	struct proc_dir_entry *proc_fwdump_dir;
	struct proc_dir_entry *proc_fwdump_file;
	CHAR fwdump_dir_name[11];
#endif

#ifdef HWIFI_SUPPORT
	struct mtk_mac_dev *mac_dev;
#endif /* HWIFI_SUPPORT */
};

typedef struct os_cookie	*POS_COOKIE;



/***********************************************************************************
 *	OS debugging and printing related definitions and data structure
 ***********************************************************************************/
void none_printk(const char *fmt, ...);

#define MTWF_PRINT none_printk

#undef  ASSERT
#ifdef DBG
#define ASSERT(x)                                                         \
	{                                                                 \
		if (!(x)) {                                               \
			none_printk(__FILE__ ":%d assert " #x "failed\n", \
			__LINE__);    \
			/* panic("Unexpected error occurs!\n");		*/\
		}                                                         \
	}
#else
#define ASSERT(x)
#endif /* DBG */


/*********************************************************************************************************
	The following code are not revised, temporary put it here.
  *********************************************************************************************************/


/***********************************************************************************
 * Device DMA Access related definitions and data structures.
 **********************************************************************************/

ra_dma_addr_t none_pci_map_single(void *pDev, void *ptr,
				size_t size, int sd_idx, int direction);


#define PCI_MAP_SINGLE_DEV(_pAd, _ptr, _size, _sd_idx, _dir)	\
	none_pci_map_single(_pAd, _ptr, _size, _sd_idx, _dir)
/*
static INLINE int none_dma_mapping_error(void *handle, ra_dma_addr_t ptr)
{
	return 0;
}

#define PCI_DMA_MAPPING_ERROR(os_handle, _ptr)	\
	none_dma_mapping_error((os_handle)->pDev, _ptr)
*/
#define PCI_UNMAP_SINGLE(_pAd, _ptr, _size, _dir) \
		((void) _pAd)

#define PCI_ALLOC_CONSISTENT(_pci_dev, _size, _ptr)

#define PCI_FREE_CONSISTENT(_pci_dev, _size, _virtual_addr, _physical_addr)

#define CONFIG_WIFI_PAGE_ALLOC_SKB

#ifndef NET_SKB_PAD
#define NET_SKB_PAD	32
#endif
#define DEV_ALLOC_SKB(_Pkt, _length)					\
	(_Pkt = RtmpOSNetPktAlloc(NULL, _length))



#define OS_PAGE_REF(_p) atomic_read(&_p->_count)

/*only for debug usage, need to strip on MP release*/
#define CONFIG_DBG_OOM

#define SKB_BUF_HEADROOM_RSV	(NET_SKB_PAD)
#define SKB_BUF_TAILROOM_RSV	(sizeof(UINT32)) /* just make compiler happy */

#define SKB_BUF_HEADTAIL_RSV	(SKB_BUF_HEADROOM_RSV + SKB_BUF_TAILROOM_RSV)
/* Need to do below miniume size protect for build_skb method,
   to avoid DATAABORT issue. */
#define SKB_BUF_MINIMUN_SIZE	(1984)


//#ifdef CONFIG_WIFI_BUILD_SKB
#define DEV_ALLOC_FRAG(_Pkt, _length)		\
	do {\
		PNDIS_PACKET  tmp_pkt;	\
		_Pkt = &tmp_pkt; /* just make compiler happy */ \
		if (_Pkt != NULL) {	\
			MEM_DBG_PKT_ALLOC_INC(_Pkt);		\
		};	\
	} while (0)

#define DEV_FREE_FRAG_BUF(_Pkt)		\
	do {\
		if (_Pkt != NULL) {	\
			MEM_DBG_PKT_FREE_INC(_Pkt);		\
		     \
		};	\
	} while (0)

#define DEV_BUILD_SKB(_Pkt, data_ptr, _length)				\
	do {\
		PNDIS_PACKET  tmp_pkt;	\
		if ((_length) == 0) {	\
			_Pkt = &tmp_pkt;	\
		} else {						\
			_Pkt = &tmp_pkt;\
		};	\
	} while (0)

#define DEV_SKB_PTR_ADJUST(_Pkt, _length, _buf_ptr, _buf_pad)		\
	do {\
		if (_Pkt != NULL) {					\
			MEM_DBG_PKT_FREE_INC(_buf_ptr);		\
			MEM_DBG_PKT_ALLOC_INC(_Pkt);		\
		};	\
	} while (0)

//#endif /* CONFIG_WIFI_BUILD_SKB */

/*
 * ULONG
 * RTMP_GetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressLow(PhysicalAddress)	(PhysicalAddress)

/*
 * ULONG
 * RTMP_GetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressHigh(PhysicalAddress)		(0)

/*
 * VOID
 * RTMP_SetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressLow(PhysicalAddress, Value)	\
	(PhysicalAddress = Value)

/*
 * VOID
 * RTMP_SetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressHigh(PhysicalAddress, Value)

#define NdisMIndicateStatus(_w, _x, _y, _z)

#define RTMP_USB_URB_DATA_GET(__pUrb)		(((purbb_t)__pUrb)->context)
#define RTMP_USB_URB_STATUS_GET(__pUrb)		(((purbb_t)__pUrb)->status)
#define RTMP_USB_URB_LEN_GET(__pUrb)	(((purbb_t)__pUrb)->actual_length)

/***********************************************************************************
 *	Network Related data structure and marco definitions
 ***********************************************************************************/
#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)	(TRUE)

#define _RTMP_OS_NETDEV_GET_PRIV(_pNetDev)	(NULL /*(_pNetDev)->ml_priv*/)

#define RTMP_OS_NETDEV_GET_DEVNAME(_pNetDev)	((_pNetDev)->name)
#define RTMP_OS_NETDEV_GET_PHYADDR(_pNetDev)	RtmpOsNetDevGetPhyAddr(_pNetDev)

/* Get & Set NETDEV interface hardware type */
#define RTMP_OS_NETDEV_GET_TYPE(_pNetDev)		((_pNetDev)->type)
#define RTMP_OS_NETDEV_SET_TYPE(_pNetDev, _type)   ((_pNetDev)->type = (_type))
#ifdef SNIFFER_MT7615
#define RTMP_OS_NETDEV_SET_TYPE_MONITOR(_pNetDev)	\
	RTMP_OS_NETDEV_SET_TYPE(_pNetDev, 0xFFFF)
#else
#define RTMP_OS_NETDEV_SET_TYPE_MONITOR(_pNetDev)	\
	RTMP_OS_NETDEV_SET_TYPE(_pNetDev, 0xFF00)
#endif
#define RTMP_OS_NETDEV_START_QUEUE(_pNetDev)		do {} while (0)
#define RTMP_OS_NETDEV_STOP_QUEUE(_pNetDev)		do {} while (0)
#define RTMP_OS_NETDEV_WAKE_QUEUE(_pNetDev)		do {} while (0)
#define RTMP_OS_NETDEV_CARRIER_ON(_pNetDev)		do {} while (0)
#define RTMP_OS_NETDEV_CARRIER_OFF(_pNetDev)		do {} while (0)

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
		((PNDIS_PACKET)(pEntry))

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
		((struct _QUEUE_ENTRY *)(pPacket))

#define GET_SG_LIST_FROM_PACKET(_p, _sc)	\
	rt_get_sg_list_from_packet(_p, _sc)

#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)	\
	RTMPFreeNdisPacket(_pAd, _pPacket)


#define RELEASE_NDIS_PACKET_IRQ(_pAd, _pPacket, _Status)	\
	RTMPFreeNdisPacketIRQ(_pAd, _pPacket)


/*
 * packet helper
 *	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
#define RTPKT_TO_OSPKT(_p)		((WF_PKT_T *)(_p))
#define OSPKT_TO_RTPKT(_p)		((PNDIS_PACKET)(_p))

#define GET_OS_PKT_DATAPTR(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->data)
#define SET_OS_PKT_DATAPTR(_pkt, _dataPtr)	\
	((RTPKT_TO_OSPKT(_pkt)->data) = (_dataPtr))

#define GET_OS_PKT_LEN(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->len)
#define SET_OS_PKT_LEN(_pkt, _len)	\
	((RTPKT_TO_OSPKT(_pkt)->len) = (_len))

#define GET_OS_PKT_DATATAIL(_pkt) \
	((RTPKT_TO_OSPKT(_pkt)->data) + (RTPKT_TO_OSPKT(_pkt)->len))
#define SET_OS_PKT_DATATAIL(_pkt, _len)	do {} while (0)

#define GET_OS_PKT_HEAD(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->head)

#define GET_OS_PKT_END(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->end)

#define GET_OS_PKT_NETDEV(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->dev)
#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
	((RTPKT_TO_OSPKT(_pkt)->dev) = (_pNetDev))

#define GET_OS_PKT_TYPE(_pkt) \
	(RTPKT_TO_OSPKT(_pkt))

#define GET_PKT_TYPE(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->pkt_type)

#define GET_OS_PKT_NEXT(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->next)


#define OS_PKT_CLONED(_pkt)		(RTPKT_TO_OSPKT(_pkt))
#define OS_PKT_COPY(_pkt, _newSkb)  \
	do {\
		_newSkb = RTPKT_TO_OSPKT(_pkt);	\
		if (_newSkb != NULL)					\
			MEM_DBG_PKT_ALLOC_INC(_newSkb);			\
	} while (0)

#define OS_PKT_TAIL_ADJUST(_pkt, _removedTagLen)

#define OS_PKT_HEAD_BUF_EXTEND(_pkt, _offset)	\
		(GET_OS_PKT_DATAPTR(_pkt) = GET_OS_PKT_DATAPTR(_pkt)+_offset)

#define OS_PKT_TAIL_BUF_EXTEND(_pkt, _Len)	\
		(GET_OS_PKT_DATAPTR(_pkt) = GET_OS_PKT_DATAPTR(_pkt)+_Len)

#define OS_PKT_RESERVE(_pkt, _Len)

#define RTMP_OS_PKT_INIT(__pRxPacket, __pNetDev, __pData, __DataSize)	\
	{								\
		PNDIS_PACKET __pRxPkt;					\
		__pRxPkt = RTPKT_TO_OSPKT(__pRxPacket);			\
		SET_OS_PKT_NETDEV(__pRxPkt, __pNetDev);			\
		SET_OS_PKT_DATAPTR(__pRxPkt, __pData);			\
		SET_OS_PKT_LEN(__pRxPkt, __DataSize);			\
		SET_OS_PKT_DATATAIL(__pRxPkt, __DataSize);		\
	}

#define OS_PKT_CLONE(_pAd, _pkt, _src, _flag)		\
	do {\
		_src = _pkt;				\
		if (_src != NULL)				\
			MEM_DBG_PKT_ALLOC_INC(_src);		\
	} while (0)


#define OS_NTOHS(_Val) \
	(((_Val)))
#define OS_HTONS(_Val) \
	(((_Val)))
#define OS_NTOHL(_Val) \
	(((_Val)))
#define OS_HTONL(_Val) \
	(((_Val)))

#define CB_OFF  10
#define CB_LEN 37
#define GET_OS_PKT_CB(_p)		(RTPKT_TO_OSPKT(_p)->cb)
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))

#define RTMP_QueryPacketInfo(_pPacket, _pSrcBufVA, _SrcBufLen)	\
	{\
		_pSrcBufVA = GET_OS_PKT_DATAPTR(_pPacket);	\
		_SrcBufLen = GET_OS_PKT_LEN(_pPacket);		\
	}

/***********************************************************************************
 *	64 bit operand
 ***********************************************************************************/
# define none_do_div(n, base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
})

#define mod_64bit(_a, _b) none_do_div(_a, _b)


/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/

#ifdef CONFIG_FAST_NAT_SUPPORT
extern int (*ra_sw_nat_hook_tx)(struct net_pkt_blk *skb, int gmac_no);
extern int (*ra_sw_nat_hook_rx)(struct net_pkt_blk *skb);
extern void (*ppe_dev_register_hook)(VOID  *dev);
extern void (*ppe_dev_unregister_hook)(VOID  *dev);
extern int (*ppe_del_entry_by_mac)(unsigned char *mac);

#endif /*CONFIG_FAST_NAT_SUPPORT*/

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);
int rt28xx_packet_xmit(VOID *skb);



#ifdef RTMP_MAC_PCI
/* function declarations */
#define IRQ_HANDLE_TYPE  void

IRQ_HANDLE_TYPE
rt2860_interrupt(int irq, void *dev_instance);

IRQ_HANDLE_TYPE
rt2860_sw_interrupt(int irq, void *dev_instance);

#endif /* RTMP_MAC_PCI */

INT rt28xx_ioctl(PNET_DEV net_dev, struct ifreq *rq, INT cmd);
int rt28xx_send_packets(struct net_pkt_blk *skb, struct none_net_device *ndev);

extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);


#define GET_PAD_FROM_NET_DEV(_pAd, _net_dev)				\
	(_pAd = RTMP_OS_NETDEV_GET_PRIV(_net_dev))
#define GET_WDEV_FROM_NET_DEV(_wdev, _net_dev)	\
	(_wdev = RTMP_OS_NETDEV_GET_WDEV(_net_dev))


#ifdef CONFIG_ATE
/******************************************************************************

	ATE related definitions

******************************************************************************/
#define ate_print printk
#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT
#ifndef CONFIG_STA_SUPPORT
#define EEPROM_BIN_FILE_NAME  "/etc/Wireless/RT2860AP/e2p.bin"
#endif /* CONFIG_STA_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#define EEPROM_BIN_FILE_NAME  "/etc/Wireless/RT2860STA/e2p.bin"
#endif /* CONFIG_STA_SUPPORT */
#else
#endif /* RTMP_MAC_PCI*/


#endif /* CONFIG_ATE */

INT RtmpOSNetDevOpsAlloc(
	IN PVOID *pNetDevOps);
void RtmpOSNetDevOpsFree(PNET_DEV pNetDev);

#define RTMP_OS_MAX_SCAN_DATA_GET()		IW_SCAN_MAX_DATA

#ifdef WIFI_MD_COEX_SUPPORT
UCHAR mt_get_active_pci_num(void);
#endif

#define RA_WEXT	0
#define RA_NETLINK	1

#ifdef FW_DUMP_SUPPORT
uint8_t FWDumpProcInit(VOID *pAd);
uint8_t FWDumpProcCreate(VOID *pAd, PCHAR suffix);
VOID FWDumpProcRemove(VOID *pAd);
INT os_set_fwdump_path(VOID *pAd, VOID *arg);
#define RTMP_OS_FWDUMP_PROCINIT(ad)	FWDumpProcInit(ad)
#define RTMP_OS_FWDUMP_PROCCREATE(ad, _suffix)	FWDumpProcCreate(ad, _suffix)
#define RTMP_OS_FWDUMP_PROCREMOVE(ad)	FWDumpProcRemove(ad)
#define RTMP_OS_FWDUMP_SETPATH(ad, ar)	os_set_fwdump_path(ad, ar)
#endif

#define OS_TRACE __builtin_return_address(0)

//typedef struct kref os_kref;


#ifdef DBG_STARVATION
#define STARV_ENTRY_SIZE 2000

struct starv_log_entry;
struct starv_dbg;
struct starv_dbg_block;

struct starv_dbg {
	unsigned int msec;
	unsigned long start_jiffies;
	unsigned long end_jiffies;
	struct starv_dbg_block *block;
};

struct starv_dbg_block {
	char name[32];
	unsigned int timeout;
	struct starv_log *ctrl;
	void (*timeout_fn)(struct starv_dbg *starv, struct starv_log_entry *entry);
	void (*leave_fn)(struct starv_dbg *starv);
	void (*entry_fn)(struct starv_dbg *starv);
	void (*log_fn)(struct starv_log_entry *entry);
	void *priv;
	/*statistic*/
	unsigned int avg_dur;
	unsigned int max_dur;
	unsigned int min_dur;
	unsigned int timeout_cnt;
	unsigned int count;
	DL_LIST log_head;
	DL_LIST list;
};

struct starv_log_basic {
	unsigned int id;
	unsigned int qsize;
};

struct starv_log_entry {
	void *log;
	unsigned int duration;
	struct starv_dbg_block *block;
	DL_LIST list;
};

struct starv_log {
	struct starv_log_entry pool[STARV_ENTRY_SIZE];
	DL_LIST free;
	DL_LIST block_head;
	NDIS_SPIN_LOCK lock;
};

VOID starv_dbg_init(struct starv_dbg_block *block, struct starv_dbg *starv);
VOID starv_dbg_get(struct starv_dbg *starv);
VOID starv_dbg_put(struct starv_dbg *starv);
VOID starv_log_dump(struct starv_log *ctrl);
INT starv_log_init(struct starv_log *ctrl);
VOID starv_log_exit(struct starv_log *ctrl);
INT register_starv_block(struct starv_dbg_block *bk);
VOID unregister_starv_block(struct starv_dbg_block *bk);
VOID starv_timeout_log_basic(struct starv_log_entry *entry);

#endif /*DBG_STARVATION*/

struct idr {
	unsigned int idr_base;
	unsigned int idr_next;
};

struct os_idr_mgmt {
	NDIS_SPIN_LOCK lock;
	struct idr id;
	u32 cnt;
	int high;
	int low;
};

int os_idrm_init(struct os_idr_mgmt *idrm, u32 low, u32 high);
void os_idrm_exit(struct os_idr_mgmt *idrm);
int os_idr_register(struct os_idr_mgmt *idrm, u32 *idx, void *data);
void os_idr_unregister(struct os_idr_mgmt *idrm, u32 idx);

#ifdef CONFIG_DBG_QDISC
void os_system_tx_queue_dump(PNET_DEV dev);
#endif /*CONFIG_DBG_QDISC*/


void os_module_init(void);
void os_module_exit(void);

void mt_rcu_read_lock(void);
void mt_rcu_read_unlock(void);

size_t strlcpy(char *dest, const char *src, size_t size);
struct skb_shared_info *skb_shinfo(struct sk_buff *skb);
void *skb_header_pointer(
	struct sk_buff *skb, int offset, int len, void *buffer);
void *skb_pull(struct sk_buff *skb, unsigned int len);
void skb_orphan(struct sk_buff *skb);
#define dev_kfree_skb(p)
unsigned int jiffies_to_msecs(const unsigned long j);
unsigned int jiffies_to_usecs(const unsigned long j);
int timer_pending(RTMP_OS_TIMER *timer);
void net_enable_timestamp(void);
void net_disable_timestamp(void);
u64 div64_ul(u64 dividend, u64 divisor);
void ktime_get_real_ts64(struct timespec64 *tv);
ktime_t timespec64_to_ktime(struct timespec64 ts);
s64 ktime_to_us(const ktime_t kt);
s64 ktime_to_ms(const ktime_t kt);
unsigned int num_online_cpus(void);
void hlist_replace_rcu(
	struct hlist_node *old,
	struct hlist_node *new_node);

typedef void (*rcu_callback_t)(struct rcu_head *head);
void hlist_add_head_rcu(
	struct hlist_node *n,
	struct hlist_head *h);
void hlist_del_rcu(struct hlist_node *n);
void mt_call_rcu(struct rcu_head *head, rcu_callback_t func);
void list_move_tail(
	struct list_head *list,
	struct list_head *head);
void list_add_tail(struct list_head *new_head, struct list_head *head);
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
void INIT_LIST_HEAD(struct list_head *list);
void list_del(struct list_head *entry);
int none_random32(void);
#define os_random32 none_random32
int find_first_bit(unsigned long *bitmap, int size);
int find_last_bit(unsigned long *bitmap, int size);
void *kzalloc(size_t size, int flags);
void mt_synchronize_rcu(void);

/*******************************************************************************
 * Device Register I/O Access related definitions and data structures.
 ******************************************************************************/
#define readl(_x) (*((UINT32 *)(_x)))
#define readb(_x) (*((uint8_t *)(_x)))
#define writel(_v,  _x)							\
	(*(UINT *)(_x) = (_v))

#define writew(_v, _x)			\
	(*(uint16_t *)(_x) = (_v))

#define writeb(_v, _x)			\
	(*(uint8_t *)(_x) = (_v))



/* TODO: We need to find a wrapper function for in_interrupt() */
#define in_interrupt() (0)
//#define in_atomic()	(0)

#ifndef ntohs
#define ntohs			OS_NTOHS
#endif /* ntohs */
#ifndef htons
#define htons			OS_HTONS
#endif /* htons */
#ifndef ntohl
#define ntohl			OS_NTOHL
#endif /* ntohl */
#ifndef htonl
#define htonl			OS_HTONL
#endif /* htonl */

/* TODO: implement correct setting */
#define abs(x) x

#define KERN_ERR

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
		!list_entry_is_head(pos, head, member); 		\
		pos = n, n = list_next_entry(n, member))

#define list_for_each_entry(pos, head, member)				\
		for (pos = list_first_entry(head, typeof(*pos), member);	\
			!list_entry_is_head(pos, head, member);			\
			pos = list_next_entry(pos, member))


#define mt_msecs_to_jiffies(_m) (_m)
typedef void (*work_func_t)(struct work_struct *work);
void mt_destroy_workqueue(OS_WORKQUEUE_STRUCT *wq);
OS_WORKQUEUE_STRUCT *mt_create_workqueue(const char *name);
void mt_init_delayed_work(struct delayed_work *work, work_func_t func);
void mt_flush_workqueue(OS_WORKQUEUE_STRUCT *wq);
void mt_queue_delayed_work(
	OS_WORKQUEUE_STRUCT *wq,
	struct delayed_work *dwork,
	unsigned long delay);

#ifdef __GNUC__
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif

#ifndef fallthrough
#define fallthrough	do {} while (0)
#endif /* !fallthrough */

#endif /* __RT_LINUX_H__ */
