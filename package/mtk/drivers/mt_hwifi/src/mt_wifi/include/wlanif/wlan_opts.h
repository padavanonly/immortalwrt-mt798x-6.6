#ifndef _WLAN_OPTS_H_
#define _WLAN_OPTS_H_

/* TPWD_DEBUG -
 * Control whether debug features (printouts, assertions) are compiled
 * into the driver.
 */
#ifndef TPWD_DEBUG
#define TPWD_DEBUG 1 /* default: include debug code */
#endif

#define TPWD_VERSION				"1.1.1.3"

#define COMMON_TIMEOUT				1						/* common timer interval (secs) */
#define BCNINTCHK_SCALE				3

/* timeout in sec */
#define NODE_INIT_TIMEOUT			(5  / COMMON_TIMEOUT)	/* initial */
#define NODE_RUN_TIMEOUT			(240/ COMMON_TIMEOUT)	/* authorized */
#define NODE_FRAG_TIMEOUT			(15 / COMMON_TIMEOUT)	/* fragment timeout in sec */
#define NODE_PSQ_TIMEOUT			(5 / COMMON_TIMEOUT)	/* psq timeout */

#define BR_STA_TIMEOUT				(15/ COMMON_TIMEOUT)
#define BREXT_STA_TIMEOUT			(240/ COMMON_TIMEOUT)
#define MCAST_ENTRY_TIMEOUT			(120/ COMMON_TIMEOUT)

#define NON_ERP_OBSS_TIMEOUT		(10 / COMMON_TIMEOUT)	/* not support erp overlap bss detect time out time, 10 seconds */
#define NON_HT_OBSS_TIMEOUT			(10 / COMMON_TIMEOUT)	/* not support HT overlap bss detect time out time, 10 seconds */

#define	ADDBA_RETRY_TIMEOUT			(10 / COMMON_TIMEOUT)
#define	ADDBA_STATE_TIMEOUT			(1 / COMMON_TIMEOUT)
#define	WPA_STATE_TIMEOUT			(1 / COMMON_TIMEOUT)
#define	RADIUS_SSNTO_TIMEOUT		(30 / COMMON_TIMEOUT)

/* timeout in ms */
#define STA_IDLE_TIMEOUT			1000
#define STA_SCAN_TIMEOUT			1000
#define STA_AUTH_TIMEOUT			500
#define STA_ASSOC_TIMEOUT			500
#define STA_4WAYHANDSHAKE_TIMEOUT	1000

#define SCAN_DWELL_TIME				150						/* 150 ms */

#define AMPDU_RX_TIMEOUT			100						/* timeout interval in msec for resp prog */

#define	GKUP_INTERVAL_DEFAULT		(60*60*24)				/* 24 hours */

#if defined(MULTI_INF_SUPPORT)
#define MAX_WLAN_DEV 2
#else
#define MAX_WLAN_DEV				1		/* No multi-device support yet (< 5 for wpa radius) */
#endif

/* 0 : main network
   1 : IPTV
   2 : Guest Network
   4 : STA VAP
*/
#define MAX_VAP_CNT					(UNIT_MAX + 1)
#define MAX_5G_VAP_CNT				(UNIT_5G_MAX + 1)

#define MAX_NODE_CNT                (16 + MAX_VAP_CNT)
#define GUESTNET_MAX_STA            (MAX_NODE_CNT - MAX_VAP_CNT - 1)


#define MAX_NODE_CNT				(16 + MAX_VAP_CNT)
#define NODE_HASH_SIZE				32
#define UMAC_WDS_MAXCNT				128
#define UMAC_EXT_STA_CNT			128
#define MCAST_ENTRY_CNT				32						/* max multicast-to-unicast entries */
#define MAX_BSS_CNT					64
#define MAX_ACL_CNT					64
#define MAX_PIN_FAIL_CNT			10						/* lock the AP after too many failures */

#define MAX_MPDU_LEN				1970					/* 3839/2 + rx_status */

#define MIN_PKTQ_LEN				3						/* min # pkts node can enq */
#define MAX_PSQ_LEN					64						/* max # of pkts enq'able on a single chip */

#define WLAN_BMISS_LIMIT			30

#define MAX_WPS_BTN_TIME			5						/* max press button time (sec) */

#define RCS_ARR_SIZE				4						/* rate ctrl series */

#define STA_RETRY_CNT				5
#define WPA_RETRY_CNT				8

#define OS_EVENT_PORT				1060
#define OS_RADIUS_PORT				1061
#define OS_WPS_PORT					7000
#define OS_UPNP_PORT				7020

#define WLAN_TASK_PRIORITY			160
#define WLAN_STACK_SIZE				1024*16

#define WLAN_AUTO_CHANNEL			0						/* auto channel select */

#define	WLAN_REKEY_TIME_MIN			30						/* Min = 30 secs */
#define WLAN_NO_REKEY				0

#define WLAN_FRAG_THRESHOLD_MAX		2346					/* max fragmentation threshold */
#define WLAN_FRAG_THRESHOLD_MIN		256						/* min fragmentation threshold */
#define WLAN_FRAG_THRESHOLD_DEF		2346

#define WLAN_RTS_MAX				2346
#define WLAN_RTS_MIN				1
#define WLAN_RTS_DEF				2346

#define WLAN_BINTVAL_MAX			1000					/* max beacon interval (TU's) max 10000 */
#define WLAN_BINTVAL_MIN			40						/* min beacon interval (TU's) min 10 */
#define WLAN_BINTVAL_DEF			100

#define WLAN_DTIM_MAX				255						/* max DTIM period */
#define WLAN_DTIM_MIN				1						/* min DTIM period */
#define WLAN_DTIM_DEF				1

#define WLAN_WPS_LOCK_TIME			60

#define WLAN_F_LAN					0x00000001				/* LAN access */

#endif /* _WLAN_OPTS_H_ */
