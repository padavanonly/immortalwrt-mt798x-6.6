#ifndef __NONE_TYPE_H__
#define __NONE_TYPE_H__

#ifndef __packed
#define __packed    __attribute__((__packed__))
#endif

#ifndef CONFIG_GOOGLE_UT
typedef  char		bool;
#endif /* !CONFIG_GOOGLE_UT */

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long __s64;
typedef unsigned long long __u64;


typedef __u32 u32;
typedef __s32 s32;

typedef __u16 u16;
typedef __s16 s16;

typedef __u8  u8;
typedef __s8  s8;


typedef __s64 s64;
typedef __u64 u64;

/* linux/types.h */

/* bsd */
typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;

/* sysv */
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

#ifndef CONFIG_GOOGLE_UT
typedef s8			int8_t;
typedef s16			int16_t;
typedef s32			int32_t;
typedef u8			uint8_t;
typedef u16			uint16_t;
typedef u32			uint32_t;
typedef u64			uint64_t;
#endif /* !CONFIG_GOOGLE_UT */

#ifndef USHRT_MAX
#define USHRT_MAX	((unsigned short)~0U)
#endif

#ifndef U16_MAX
#define U16_MAX		((u16)~0U)
#endif

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN	6
#endif

struct none_pid {
	char name[32];
};

struct none_semaphore {
	char name[32];
};

struct none_pci_dev {
	char name[32];
};

struct none_device {
	char name[32];
	unsigned short	type;
	unsigned long state;
	unsigned long flags;
};

struct none_net_device {
	char name[32];
	unsigned short		type;
	unsigned long state;
	unsigned long flags;
	unsigned char broadcast[MAC_ADDR_LEN];
};
#define net_device none_net_device

struct none_completion {
	unsigned int done;
	unsigned int wait;
};

struct none_net_device_stats {
	unsigned long	rx_packets;
	unsigned long	tx_packets;
	unsigned long	rx_bytes;
	unsigned long	tx_bytes;
	unsigned long	rx_errors;
	unsigned long	tx_errors;
	unsigned long	rx_dropped;
	unsigned long	tx_dropped;
	unsigned long	multicast;
	unsigned long	collisions;
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;
	unsigned long	rx_crc_errors;
	unsigned long	rx_frame_errors;
	unsigned long	rx_fifo_errors;
	unsigned long	rx_missed_errors;
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef ETH_HLEN
#define ETH_HLEN 14
#endif

#define copy_to_user(x, y, z)     (memcpy(x, y, z) == NULL)
#define copy_from_user(x, y, z)   (memcpy(x, y, z) == NULL)


#define __ALIGN_KERNEL(x, a)	__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))

#define SKB_DATA_ALIGN(X)	ALIGN(X, 4)


#define none_offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)
#define none_container_of(ptr, type, member) ({  \
		const typeof(((type *)0)->member) * __mptr = (ptr);  \
		(type *)((char *)__mptr - none_offsetof(type, member)); })

#define INLINE

INLINE void udelay(unsigned int us);
INLINE void msleep(unsigned int ms);
INLINE void mdelay(unsigned int ms);

#define __NEW_UTS_LEN 64

struct new_utsname {
	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};

struct none_file {
	int flags;
};

#ifndef O_RDONLY
#define O_RDONLY			RTMP_FILE_RDONLY
#endif /* O_RDONLY */

#ifndef O_WRONLY
#define O_WRONLY			RTMP_FILE_WRONLY
#endif /* O_WRONLY */

#ifndef O_CREAT
#define O_CREAT				RTMP_FILE_CREAT
#endif /* O_CREAT */

#ifndef O_TRUNC
#define O_TRUNC				RTMP_FILE_TRUNC
#endif /* O_TRUNC */

#ifndef O_APPEND
#define O_APPEND			RTMP_FILE_APPEND
#endif /* O_APPEND */

#define IWEVCUSTOM			0x8C02		/* Driver specific ascii string */
#define IW_PRIV_SIZE_MASK		0x07FF	  /* Max number of those args */
#define SIOCDEVPRIVATE			0x8BE0
#define SIOCIWFIRSTPRIV			SIOCDEVPRIVATE

#define RTPRIV_IOCTL_SET		(SIOCIWFIRSTPRIV + 0x02)

/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
#define PCI_DMA_NONE		3

/*
 *	Data unalignment handling related definitions and data structure
 */
void bad_unaligned_access_length(void) __attribute__((noreturn));

struct UINT64_S {
	UINT64 x __packed;
};
struct UINT32_S {
	UINT32 x __packed;
};
struct UINT16_S {
	uint16_t x __packed;
};


/*
 * Elemental unaligned stores
 */
INLINE void __ustq(UINT64 val, UINT64 *addr);
INLINE void __ustl(UINT32 val, UINT32 *addr);
INLINE void __ustw(uint16_t val, uint16_t *addr);
INLINE USHORT __uldw(UINT16 *addr);
INLINE UINT64 __uldq(UINT64 *addr);
INLINE UINT32 __uldl(UINT32 *addr);

#define __get_unaligned(ptr, size) ({		\
		const void *__gu_p = ptr;		\
		__typeof__(*(ptr)) val;			\
		switch (size) {				\
		case 1:					\
			val = *(uint8_t *)__gu_p;	\
			break;				\
		case 2:					\
			val = __uldw((UINT16 *)__gu_p);	\
			break;				\
		case 4:					\
			val = __uldl((UINT32 *)__gu_p);	\
			break;				\
		case 8:					\
			val = __uldq((UINT64 *)__gu_p);	\
			break;				\
		default:				\
			bad_unaligned_access_length();	\
		};					\
		val;					\
	})

#define __put_unaligned(val, ptr, size)		\
	do {						\
		void *__gu_p = ptr;			\
		switch (size) {				\
		case 1:					\
			*(uint8_t *)__gu_p = val;		\
			break;				\
		case 2:					\
			__ustw(val, __gu_p);		\
			break;				\
		case 4:					\
			__ustl(val, __gu_p);		\
			break;				\
		case 8:					\
			__ustq(val, __gu_p);		\
			break;				\
		default:				\
			bad_unaligned_access_length();	\
		};					\
	} while (0)

#define get_unaligned(ptr) \
	__get_unaligned((ptr), sizeof(*(ptr)))
#define put_unaligned(x, ptr) \
	__put_unaligned((UINT64)(x), (ptr), sizeof(*(ptr)))

#define get_unaligned32(_x)	get_unaligned(_x)
#define get_unalignedlong(_x)	get_unaligned(_x)


/* <linux/cache.h> */
#ifndef ____cacheline_aligned
#define ____cacheline_aligned __attribute__((__aligned__(64)))
#endif


#ifndef ARRAY_SIZE
# define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define __aligned(x)

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memo */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

#ifndef EOPNOTSUPP
#define EOPNOTSUPP	(-RTMP_IO_EOPNOTSUPP)
#endif /* !EOPNOTSUPP */

#ifndef CONFIG_GOOGLE_UT
#define	ENETDOWN	50	/* Network is down */
#define	ETIMEDOUT	60	/* Connection timed out */
#define	ELNRNG		93	/* Link number out of range */
#endif /* CONFIG_GOOGLE_UT */

#define GFP_KERNEL	0

#define dma_addr_t	long
typedef int atomic_t;

#ifndef BIT
#define BIT(n)                          ((uint32_t) 1UL << (n))
#endif /* BIT */

#ifndef BITS
/* bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m, n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

typedef unsigned long mm_segment_t;

typedef s64 ktime_t;
struct timespec64 {
	u64	tv_sec;			/* seconds */
	long	tv_nsec;		/* nanoseconds */
};

#endif /* __NONE_TYPE_H__ */

