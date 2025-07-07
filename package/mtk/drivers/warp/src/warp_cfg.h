/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	warp_cfg.h
*/

/*configuration*/
/*Label 0*/
#define CFG_INTER_AGENT_SUPPORT 	1

/*not maintain*/
#define CFG_WARP_CPU_TRACER 		0

/*Label 1*/
#define CFG_TX_SUPPORT			1

#define CFG_RX_SUPPORT			1

#ifdef CONFIG_WARP_V2
#define CFG_RX_D_SUPPORT		1
#define CONFIG_WARP_CCIF_SUPPORT 	1
#elif defined(CONFIG_WARP_V3)
#define CFG_RX_D_SUPPORT		1
#define CONFIG_WARP_CCIF_SUPPORT	1
#define CFG_PAO_SUPPORT			1
#define CFG_RX_HW_RRO 			1
#define CFG_RX_HW_RRO_DBG		0
/* MEMCPY must also enable CFG_RX_HW_RRO_DBG */
#define CFG_RX_HW_RRO_DBG_MEMCPY	0
#else
#define CFG_RX_D_SUPPORT		0
#define CONFIG_WARP_CCIF_SUPPORT 	0
#endif

/*Label 2 ATC is removed*/

#define CFG_DELAY_INT_SUPPORT	0

/*Label 3*/
#define CFG_HW_TX_SUPPORT		1
#define CFG_HW_RX_SUPPORT		1

/*Label 4*/
#define CFG_DYNAMIC_BM_SUPPORT	1

/*should remove when feature is ready or fix*/
#define CFG_WORK_AROUND_128_ALIGN	1
/*should not enable since hw fixed*/
#define CFG_WORK_AROUND_INT_POLL	0
/*should not enable since hw fixed*/
#define CFG_WORK_AROUND_WDMA_RETURN_IDLE 0

/*define TXBM size*/
#define MAX_TX_PKT_NUM 16384

/*workaround*/
#if CFG_WORK_AROUND_128_ALIGN
#define WED_WORK_AROUND_128_ALIGN
#endif

#if CFG_WORK_AROUND_INT_POLL
#define WED_WORK_AROUND_INT_POLL
#endif

#if CFG_WORK_AROUND_WDMA_RETURN_IDLE
#define WED_WORK_AROUND_WDMA_RETURN_IDLE
#endif

/*compiler flag rule*/

#if CFG_WARP_CPU_TRACER
#define WARP_CPU_TRACER
#endif

#if CFG_INTER_AGENT_SUPPORT
#define WED_INTER_AGENT_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT)
#define WED_TX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_RX_SUPPORT)
#define WED_RX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_RX_D_SUPPORT)
#define WED_RX_D_SUPPORT
#endif
/* 0: normal mode, 1: bypass mode */
#define WED_HWRRO_MODE_BYPASS 0
#define WED_HWRRO_MODE_KO 1
#define WED_HWRRO_MODE_WOCPU 2
#define WED_HWRRO_MODE 2

#if (defined(CFG_PAO_SUPPORT) && CFG_PAO_SUPPORT)
#define WED_PAO_SUPPORT
#endif

#if (defined(CFG_RX_HW_RRO) && CFG_RX_HW_RRO)
#define WED_RX_HW_RRO
#endif

#if (defined(CFG_RX_HW_RRO_DBG) && CFG_RX_HW_RRO_DBG)
#define WED_RX_HW_RRO_DBG
#if (defined(CFG_RX_HW_RRO_DBG_MEMCPY) && CFG_RX_HW_RRO_DBG_MEMCPY)
#define WED_RX_HW_RRO_DBG_MEMCPY
#endif
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_DELAY_INT_SUPPORT)
#define WED_DELAY_INT_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_HW_TX_SUPPORT)
#define WED_HW_TX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_RX_D_SUPPORT && CFG_HW_RX_SUPPORT)
#define WED_HW_RX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_HW_TX_SUPPORT && CFG_DYNAMIC_BM_SUPPORT)
#ifdef CONFIG_WARP_V2
#define WED_DYNAMIC_TXBM_SUPPORT
#define WED_DYNAMIC_RXBM_SUPPORT
#endif
#endif

#define WED_WDMA_SINGLE_RING

//#define WARP_DVT
