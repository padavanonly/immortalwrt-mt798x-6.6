#ifndef _WLAN_EXDEP_H_
#define _WLAN_EXDEP_H_

#include "vxWorks.h"
#include "netBufLib.h"
/*#include "miniUPnPApi.h"*/
#include "commonType.h"

#include <time.h>
#include <tickLib.h>

#define MAC_ADDR_SIZE				6

#define OS_LAN_UNIT								0
#define OS_WAN_UNIT								1

#define OS_WLAN_OPEN_PORT_UDP_LOCAL(_p)			openPort(LOCAL_PORT, (_p), PKT_UDP)

#ifdef _WLAN_EN_ASSERT_
#define OS_ASSERT(x)							assert(x)
#else
#define OS_ASSERT(x)
#endif

#define OS_SNPRINTF								snprintf

extern BOOL mcbCheckMemAddr(UINT8 *memAddr);
extern UINT8 *mcbAlign(UINT32 memId, UINT32 memSize);
extern STATUS mcbFree(UINT8 *memAddr);
extern BOOL mcbIsMemAddrInUse(UINT8 *memAddr);
extern void sysUDelay(int us);
extern void sysMsDelay(int us);
extern int sysClkRateGet (void);


#define OS_SDMP_MALLOC(_size)					mcbAlign(WLAN_DEBUG_ID, (UINT32)(_size))
#define OS_SDMP_MALLOC_ID(_size, id)				mcbAlign((id), (UINT32)(_size))
#define OS_SDMP_FREE(_addr)						mcbFree((UINT8 *)(_addr))
#define OS_SDMP_MCB_MEM_CHECK(_addr)			mcbCheckMemAddr((UINT8 *)(_addr))
#define OS_SDMP_MCB_MEM_INUSE_CHECK(_addr)		mcbIsMemAddrInUse((UINT8 *)(_addr))
		

#define OS_MALLOC(size)							malloc(size)
#define OS_FREE(addr)							free(addr)

#define OS_MEMCPY(_dst, _src, _len)				memcpy((_dst), (_src), (_len))
#define OS_MEMZERO(_buf, _len)					memset((_buf), 0, (_len))
#define OS_MEMSET(_buf, _ch, _len)				memset((_buf), (_ch), (_len))
#define OS_MEMCMP(_mem1, _mem2, _len)			memcmp((_mem1), (_mem2), (_len))
#define OS_STRLEN(_s)							strlen(_s)
#define OS_STRTOUL(_nptr, _endptr, _base)		strtoul((_nptr), (_endptr), (_base))

#define MAC_ADDR_EQ(a1,a2)        				(OS_MEMCMP((a1), (a2), MAC_ADDR_LEN) == 0)
#define MAC_ADDR_COPY(dst,src)    				OS_MEMCPY((dst), (src), MAC_ADDR_LEN)

#define OS_MDELAY(_ms)							sysMsDelay(_ms)
#define OS_UDELAY(_us)							sysUDelay(_us)

#define OS_RANDOM(_p, _n)						os_random((_p), (_n))
#define OS_DELAY(_n)							sysUDelay(_n)
#define OS_TIME_GET()							(tickGet() / sysClkRateGet())
#define OS_TIME_GET_MS()						(1000 * tickGet() / sysClkRateGet())

#define OS_PKT_FLAG_SET(pMblk, _flags)		((pMblk)->mBlkHdr.mFlags |= (_flags))

typedef M_BLK_ID OS_WLPKT;

#define EXTERN									extern
#define INLINE									__inline__

#define DEV_MAX_VAL_LEN				32
#define DEV_MAX_VAR_LEN 			32

#define WL_DEV_NAME					"ra"
#define WL_5G_DEV_NAME				"rax"

#ifndef __packed
#define __packed    __attribute__((__packed__))
#endif

typedef struct _UPNP_DES_FILE
{
	INT8 *name;
	INT8 *content;
	INT32 len;
}UPNP_DES_FILE;

#endif /* _WLAN_EXDEP_H_ */
