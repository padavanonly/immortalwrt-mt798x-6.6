/******************************************************************************
*
* Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* FILE NAME  :   wlan_ioctl.c
* VERSION    :   1.0
* DESCRIPTION:   io-control handling routines.
*
* AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
* CREATE DATE:   01/09/2011
*
* HISTORY    :
* 01   01/09/2011  zhengxinggu     Create.
* 02   05/30/2014  yangyang		   Modified to control ralink driver
*
******************************************************************************/
#include "rt_config.h"
#include "rtmp_comm.h"
#include "wlanif/wlan_headers.h"
#include "mirrorEnd.h"
#include "bridge.h"
#include "gpio.h"

#ifndef EXCLUDE_WPS
#include "wlan_debug.h"
#endif

#ifdef END_DEV_NAME
#undef END_DEV_NAME
#endif
#define END_DEV_NAME(_pEnd) RtmpOsGetNetDevName(_pEnd)

INT Set_AP_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_HideSSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
BOOL IsDutCaled(RTMP_ADAPTER *pAd);
STATUS os_wlanStart(RTMP_ADAPTER *pAd, UINT32 unit);
void RTMPSetCountryCode(RTMP_ADAPTER *pAd, RTMP_STRING *CountryCode);
struct wifi_dev *get_curr_wdev(struct _RTMP_ADAPTER *pAd, UCHAR idx);
int iwpriv_2G(char *pCmd);
int iwpriv(char *pCmd);

#define RADIO_TGTPWRDELTA_LEN		(0x68)
#define MFG_DATA_SIZE				(1104 + RADIO_TGTPWRDELTA_LEN)
#define IS_DUT_CALED_MAGICSIZE		2
#define IS_1M_FLASH_MAGICSIZE		14
#define RADIO_FREQERR_SIZE			1
#define RADIO_REFPWR_SIZE			54
#define RADIO_CRC_OFFSET			8
#define RADIO_CRC_LEN				4
#define HIGH_TX_PWR					100
#define MEDIUM_TX_PWR				50
#define LOW_TX_PWR					25

typedef enum _WLAN_ADV_TX_PWR
{
	WLAN_ADV_TX_POWER_MIN = 1,
	WLAN_ADV_TX_POWER_HIGH = WLAN_ADV_TX_POWER_MIN,
	WLAN_ADV_TX_POWER_MEDIUM,
	WLAN_ADV_TX_POWER_LOW, 
	WLAN_ADV_TX_POWER_MAX = WLAN_ADV_TX_POWER_LOW
} WLAN_ADV_TX_PWR;

#define RSSI_BASE (-96)
#define APCLI_DEFAULT_INDEX 0

#define COUNTRY_CN 18

UINT32 g_tpwdDebug = WL_DBG_DEF;	/* golbal debug level mask */

static UMACDBGLVLMAP_T l_dbgLvlMap[] =
{
	{"NONE",		WL_DBG_NONE},
	{"INFO"	,		WL_DBG_INFO},
	{"ERROR",		WL_DBG_ERROR},
	{"WARN"	,		WL_DBG_WARN},
	{"TRACE",		WL_DBG_TRACE},
	{"ASSOC",		WL_DBG_ASSOC},
	{"PWR"	,		WL_DBG_PWR},
	{"WPA"	,		WL_DBG_WPA},
	{"WPS"	,		WL_DBG_WPS},
	{"BR"	,		WL_DBG_BR},
	{"MCAST",		WL_DBG_MCAST},	
	{"AMPDU",		WL_DBG_AMPDU},
	{"MFG"	,		WL_DBG_MFG},
	{"INT"	,		WL_DBG_INT},
	{"TX99" ,		WL_DBG_TX99},
	{"RATE"	,		WL_DBG_RATE},
	{"A34DTC",		WL_DBG_A34DETECT},
	{"TURBO",		WL_DBG_TURBO},
	{NULL,			WL_DBG_NONE}
};

extern COUNTRY_CODE_TO_COUNTRY_REGION allCountry[];
void APUpdate(RTMP_ADAPTER *pAd, INT idx)
{
	if (idx >= MAX_BEACON_NUM || idx < 0)
		return;
	APStop(pAd, &pAd->ApCfg.MBSSID[idx], AP_BSS_OPER_BY_RF);
	APStartUp(pAd, &pAd->ApCfg.MBSSID[idx], AP_BSS_OPER_BY_RF);

}

RTMP_ADAPTER * wlGetInfAdapter(END_OBJ *pEndObj)
{
	POS_COOKIE pObj;
	int infIdx;
	RTMP_ADAPTER *pAd = NULL;

	if ( pEndObj == NULL)
		return NULL;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	if (pAd == NULL)
		return NULL;

	/* determine this ioctl command is comming from which interface. */
	if (pEndObj->pFuncTable == &RtmpMAINEndFuncTable)
	{
		pObj = (POS_COOKIE)pAd->OS_Cookie;
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		/*DBGPRINT(DBG_LVL_INFO, ("rt28xx_ioctl I/F(ra%d)\n", pObj->ioctl_if));*/
	}
#ifdef MBSS_SUPPORT
	else if (pEndObj->pFuncTable == &RtmpMBSSEndFuncTable)
	{
		pObj = (POS_COOKIE)pAd->OS_Cookie;
		pObj->ioctl_if_type = INT_MBSSID;
		for (infIdx = 1; infIdx < pAd->ApCfg.BssidNum; infIdx++)
		{
			if (pAd->ApCfg.MBSSID[infIdx].wdev.if_dev== pEndObj)
			{
				pObj->ioctl_if = infIdx;
				/*DBGPRINT(DBG_LVL_INFO, ("rt28xx_ioctl I/F(ra%d)\n", infIdx));*/
				break;
			}
		}
		
		/* Interface not found! */
		if(infIdx == pAd->ApCfg.BssidNum)
		{
			/*DBGPRINT(DBG_LVL_ERROR, ("rt28xx_ioctl can not find I/F\n"));*/
			return NULL;
		}
		MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
	}
#endif /* MBSS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (pEndObj->pFuncTable == &RtmpApCliEndFuncTable)
	{
		pObj = (POS_COOKIE)pAd->OS_Cookie;
		pObj->ioctl_if_type = INT_APCLI;
		for (infIdx = 0; infIdx < MAX_APCLI_NUM; infIdx++)
		{
			if (pAd->StaCfg[infIdx].wdev.if_dev == pEndObj)
			{
				pObj->ioctl_if = infIdx;

				break;
			}

			if(infIdx == MAX_APCLI_NUM)
			{
				/*DBGPRINT(DBG_LVL_ERROR, ("rt28xx_ioctl can not find I/F\n"));*/
				return NULL;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif /* APCLI_SUPPORT */
    else
	{
		/*DBGPRINT(DBG_LVL_WARN, ("IOCTL is not supported in WDS interface\n"));*/
		return NULL;
	}
	return pAd;
}



/* 
 * fn		INT wlSetSSID(END_OBJ *pEnd, PSTRING ssid)
 * brief	
 * details	
 *
 * param[in]	
 * param[out]	
 *
 * return	
 * retval	
 *
 * note		Written by Liu Liming. 2012-02-15. 
 */
INT wlSetSSID(END_OBJ *pEnd, RTMP_STRING * ssid)
{
	RTMP_ADAPTER *pAd = NULL;

	pAd = wlGetInfAdapter(pEnd);

	if (pAd == NULL)
	{
		return FALSE;
	}
	return Set_AP_SSID_Proc(pAd, ssid);
}

INT wlSetBssid(END_OBJ *pEnd, RTMP_STRING *bssid)
{
	RTMP_ADAPTER *pAd = NULL;
	INT success = TRUE;

	pAd = wlGetInfAdapter(pEnd);

	if (pAd == NULL)
	{
		return FALSE;
	}
	success = RT_CfgSetMacAddress(pAd, bssid, 0, OPMODE_AP);
	APStop(pAd, &pAd->ApCfg.MBSSID[MAIN_MBSSID], AP_BSS_OPER_ALL);
	APStartUp(pAd, &pAd->ApCfg.MBSSID[MAIN_MBSSID], AP_BSS_OPER_ALL);
	return success;
}


#ifdef APCLI_SUPPORT
INT wlSetApCliSSID(END_OBJ *pEnd, RTMP_STRING *ssid)
{
	RTMP_ADAPTER *pAd = NULL;
	
	pAd = wlGetInfAdapter(pEnd);
	
	if (pAd == NULL)
	{
		return FALSE;
	}
	return Set_ApCli_Ssid_Proc(pAd, ssid);
}

INT wlSetApCliEnable(END_OBJ *pEnd, INT enable)
{
	RTMP_ADAPTER *pAd = NULL;
	char buf[5];
	INT ret = 0;

	pAd = wlGetInfAdapter(pEnd);

	if (pAd == NULL)
	{
		return FALSE;
	}
	ret = snprintf(buf, sizeof(buf), "%d", enable);
	if (os_snprintf_error(sizeof(buf), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");
	return Set_ApCli_Enable_Proc(pAd, (RTMP_STRING *)buf);
}

INT wlSetApCliBSSID(END_OBJ *pEnd, RTMP_STRING *bssid)
{
	RTMP_ADAPTER *pAd = NULL;

	pAd = wlGetInfAdapter(pEnd);

	if (pAd == NULL)
	{
		return FALSE;
	}
	return Set_ApCli_Bssid_Proc(pAd, bssid);
}

#endif /* APCLI_SUPPORT */
/*
 * fn		INT wlSetSSIDAdv(END_OBJ *pEnd, INT enable)
 * brief	
 * details	
 *
 * param[in]	
 * param[out]	
 *
 * return
 * retval
 *
 * note		Written by Liu Liming. 2012-02-15.
 */
INT wlSetSSIDAdv(END_OBJ *pEnd, INT enable)
{
	RTMP_ADAPTER *pAd = NULL;
	char temp[10];
	INT ret = 0

	pAd = wlGetInfAdapter(pEnd);

	if (pAd == NULL)
	{
		return FALSE;
	}
	ret = snprintf(temp, sizeof(temp), "%d", 1 - enable);
	if (os_snprintf_error(sizeof(temp), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");
	return Set_HideSSID_Proc(pAd, temp);
}

static STATUS _ioctlGetDebug(UINT32 *val)
{
	*val = g_tpwdDebug;
	return OK;
}

static STATUS _ioctlSetDebug(UINT32 val)
{
	g_tpwdDebug = val;
	return OK;
}
	
static INLINE STATUS _ioctlShowDbgLvls(RTMP_ADAPTER *pAd)
{
	#ifdef TPWD_DEBUG

	UMACDBGLVLMAP_T *dbgLvlMap = NULL; 
 
	for (dbgLvlMap = l_dbgLvlMap; dbgLvlMap->lvlName; dbgLvlMap++)
	{
		my_printf("%s\t%10u(0x%08x)\n", dbgLvlMap->lvlName, dbgLvlMap->lvlFlag, dbgLvlMap->lvlFlag);
	}
	
	#else
	
	my_printf("debug setting is disabled, you might need to recompile the whole WLAN driver\n");

	#endif

	return OK;
}


static STATUS _ioctlGetSsid(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *ssid)
{
#ifdef APCLI_SUPPORT 
	STA_ADMIN_CONFIG *pApcliEntry = &pAd->StaCfg[GET_VAP_IDX(unit)];
#endif
	
	if (IS_APVAP_UNIT(unit))
	{
		NdisMoveMemory(ssid, pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].Ssid, pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].SsidLen);
	}
#ifdef APCLI_SUPPORT 
	else
	{
		NdisMoveMemory(ssid, pApcliEntry->CfgSsid, pApcliEntry->CfgSsidLen);
	}
#endif

	return OK;
}

static STATUS _ioctlGetBssid(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *bssid)
{
	struct wifi_dev *wdev = NULL;
	if (IS_APVAP_UNIT(unit))
	{
		wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
	}
	else
	{
		wdev = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev;
	}

	if (wdev != NULL)
	{
		NdisMoveMemory(bssid, wdev->bssid, MAC_ADDR_LEN);
		return OK;
	}

	return ERROR;
}

static STATUS _ioctlSetSsid(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *ssid, UINT32 len)
{
#ifdef APCLI_SUPPORT 
	STA_ADMIN_CONFIG *pApcliEntry = &pAd->StaCfg[GET_VAP_IDX(unit)];
#endif
	
	if (len > MAX_SSID_LEN)
	{
		return ERROR;
	}

	if (IS_APVAP_UNIT(unit))
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].SsidLen = len;
		NdisZeroMemory(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].Ssid, ssid, len);
	}
#ifdef APCLI_SUPPORT 
	else
	{
		pApcliEntry->CfgSsidLen = len;
		NdisZeroMemory(pApcliEntry->CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pApcliEntry->CfgSsid, ssid, len);
	}
#endif
	return OK;
}

IMPORT BOOL wlanDrvStartDone;
/* need to modefied */
static STATUS _ioctlStopVap(RTMP_ADAPTER *pAd, UINT32 unit)
{
#ifdef APCLI_SUPPORT 
	STA_ADMIN_CONFIG *pApcliEntry = &pAd->StaCfg[GET_VAP_IDX(unit)];
#endif

	
	DBG_INFO("stop vap %d\n", unit);

	if (IS_APVAP_UNIT(unit))
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bcn_buf.bBcnSntReq = FALSE;
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bAllowBeaconing = FALSE;

		if (taskIdSelf() != taskNameToId("tRootTask"))
		{
			if (NULL != pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.if_dev)
			{
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.if_dev);
			}

			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/* kick out all STAs behind the bss.*/
				MbssKickOutStas(pAd, GET_VAP_IDX(unit), REASON_DISASSOC_INACTIVE);
			}
			
			UpdateBeaconHandler(
				pAd,
				&pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev,
				BCN_REASON(BCN_UPDATE_IF_STATE_CHG));

		}
	}
#ifdef APCLI_SUPPORT 
	else
	{
		if (taskIdSelf() != taskNameToId("tRootTask"))
		{
			pApcliEntry->ApcliInfStat.Enable = FALSE;
			if (wlanDrvStartDone)
			{
				ApCliIfDown(pAd);
			}

			RTMP_OS_NETDEV_STOP_QUEUE(pApcliEntry->wdev.if_dev);
			//bridgePortRemove(END_DEV_NAME(pApcliEntry->wdev.if_dev), APCLI_DEFAULT_INDEX, LAN_BRIDGE_INDEX);
			BOOL bridgePortRemove(char *pDevName, int unitNum, int brIndex);
			netJobAdd((FUNCPTR)bridgePortRemove, END_DEV_NAME(pApcliEntry->wdev.if_dev), APCLI_DEFAULT_INDEX, LAN_BRIDGE_INDEX, 0, 0);
		}
	}
#endif
	return OK;
}

static STATUS _ioctlStartVap(RTMP_ADAPTER *pAd, UINT32 unit)
{
#ifdef APCLI_SUPPORT 
	STA_ADMIN_CONFIG *pApcliEntry = &pAd->StaCfg[GET_VAP_IDX(unit)];
#endif

	DBG_INFO("start vap %d\n", unit);

	//TODO
	if (((FALSE == IsDutCaled(pAd)) || (TRUE == OS_IS_DEFAULT_MAC())) && (!ATE_ON(pAd)))
	{
		return ERROR;
	}

	if (IS_APVAP_UNIT(unit))
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bcn_buf.bBcnSntReq = TRUE;
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bAllowBeaconing = TRUE;
		pAd->startByApp = TRUE;

		if (taskIdSelf() != taskNameToId("tRootTask"))
		{
			if (NULL != pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.if_dev)
			{
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.if_dev);
			}
			APUpdate(pAd, GET_VAP_IDX(unit));

			UpdateBeaconHandler(
				pAd,
				&pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev,
				BCN_REASON(BCN_UPDATE_IF_STATE_CHG));
		}
	}
#ifdef APCLI_SUPPORT 
	else
	{
		pApcliEntry->ApcliInfStat.Enable = TRUE;
		RTMP_OS_NETDEV_START_QUEUE(pApcliEntry->wdev.if_dev);

		/*if (taskIdSelf() != taskNameToId("tRootTask"))
		{
			ApCliIfUp(pAd);			
		}*/

        BOOL bridgePortAdd(char *pDevName, int unitNum, int brIndex);
		netJobAdd((FUNCPTR)bridgePortAdd, END_DEV_NAME(pApcliEntry->wdev.if_dev), APCLI_DEFAULT_INDEX, LAN_BRIDGE_INDEX, 0, 0);
		//ret = bridgePortAdd(END_DEV_NAME(pApcliEntry->wdev.if_dev), APCLI_DEFAULT_INDEX, LAN_BRIDGE_INDEX); 
	}
#endif	
	return OK;
}

static STATUS _ioctlStartWlan(RTMP_ADAPTER *pAd, UINT32 unit)
{
	os_wlanStart(pAd, unit);

	return OK;
}

static STATUS _ioctlStopWlan(RTMP_ADAPTER *pAd, UINT32 unit)
{
	APStop(pAd, &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)], AP_BSS_OPER_BY_RF);
	return OK;
}

static STATUS _ioctlGetChannel(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *channel)
{
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_AP_SUPPORT	
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if(IS_APVAP_UNIT(unit))
		{
			wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
		}
#ifdef APCLI_SUPPORT
		else
		{
			wdev = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	*channel = wlan_operate_get_prim_ch(wdev);
	return OK;
}

static STATUS _ioctlSetChannel(RTMP_ADAPTER *pAd,  UINT32 unit, UINT8 channel)
{
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
		if (!wdev)
			return ERROR;

		wdev->channel = channel;

		if (WLAN_AUTO_CHANNEL == channel) {
			pAd->ApCfg.AutoChannelAlg = ChannelAlgBusyTime; /* use Alg 3 in mt7615 */
			pAd->ApCfg.bAutoChannelAtBootup = TRUE;
			/* mt7626 driver does not requre this flag*/
		} else {
			pAd->ApCfg.bAutoChannelAtBootup = FALSE;
		}



		/*sync to other apvap of the same DBDC band */
#ifdef MBSS_SUPPORT
		int i;
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			struct wifi_dev *TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;

			if (!wmode_band_equal(TmpWdev->PhyMode, wdev->PhyMode))
				continue;
			TmpWdev->channel = channel;
		}
#endif
	}
#endif /* CONFIG_AP_SUPPORT */

	DBG_INFO("set channel : %d\n", channel);

	return OK;
}

static STATUS _ioctlSetHideSsid(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 hiden)
{
	if (TRUE == hiden)
	{
		DBG_INFO("set hide ssid");
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].bHideSsid = TRUE;
#ifdef WSC_V2_SUPPORT
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].WscControl.WscV2Info.bWpsEnable = FALSE;
#endif /* WSC_V2_SUPPORT */
	}
	else
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].bHideSsid = FALSE;
	}

	return OK;
}

static STATUS _ioctlSetDomain(RTMP_ADAPTER *pAd, UINT8 countryIdx)
{
	int ret = 0;
	UINT8 tmpBuf[20];
	INT sRet = 0;

	sRet = snprintf(tmpBuf, sizeof(tmpBuf), "%d", allCountry[COUNTRY_CN].CountryNum);
	if (os_snprintf_error(sizeof(tmpBuf), sRet))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");
	ret = RT_CfgSetCountryNum(pAd, (RTMP_STRING *)tmpBuf);

	if (ret == FALSE)
	{
		return ERROR;
	}

	sRet = snprintf(tmpBuf, sizeof(tmpBuf), "%s", allCountry[COUNTRY_CN].IsoName);
	if (os_snprintf_error(sizeof(tmpBuf), sRet))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");
	tmpBuf[3] = 0;
	RTMPSetCountryCode(pAd, (RTMP_STRING *)tmpBuf);

	return OK;
}

static STATUS _ioctlSetRTS(RTMP_ADAPTER *pAd, UINT16 rtsThreshold)
{
	struct wifi_dev *wdev = NULL;
	INT bss_idx;

	if ((rtsThreshold > MAX_RTS_THRESHOLD) || (rtsThreshold < 1))
		rtsThreshold = MAX_RTS_THRESHOLD;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (bss_idx = 0; VALID_MBSS(pAd, bss_idx); bss_idx++) {
			wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;
			wlan_config_set_rts_len_thld(wdev, rtsThreshold);
		}
	}
#endif

	DBG_INFO("set rts threshold : %d\n", rtsThreshold);

	return OK;
}

static STATUS _ioctlSetFrag(RTMP_ADAPTER *pAd, UINT16 fragThreshold)
{
	struct wifi_dev *wdev = NULL;
	INT bss_idx;

	if (fragThreshold > MAX_FRAG_THRESHOLD || fragThreshold < MIN_FRAG_THRESHOLD)
		fragThreshold = MAX_FRAG_THRESHOLD;
	else if (fragThreshold % 2 == 1)
		fragThreshold -= 1;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (bss_idx = 0; VALID_MBSS(pAd, bss_idx); bss_idx++) {
			wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;
			wlan_config_set_frag_thld(wdev, fragThreshold);
		}
	}
#endif /*CONFIG_AP_SUPPORT*/

	DBG_INFO("set fragment threshold : %d\n", fragThreshold);

	return OK;
}

static STATUS _setMbssAuth(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 authMode)
{
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[unit].wdev;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr = NULL;

	if (NULL == wdev)
	{
		DBG_ERR("end of mbssid %d is null!\n", unit);
		return ERROR;
	}
	
	switch (authMode)
	{
	case WLAN_AUTH_NONE:
	case WLAN_AUTH_WEP_OPEN:
		macptr = "OPEN";
		break;
	case WLAN_AUTH_WEP_SHARE:
		macptr = "SHARED";
		break;
	case WLAN_AUTH_WEP_AUTO:
		macptr = "WEPAUTO";
		break;
	case WLAN_AUTH_WPA:
		macptr = "WPA";
		break;
	case WLAN_AUTH_WPA2:
		macptr = "WPA2";
		break;
	case WLAN_AUTH_WPAWPA2:
		macptr = "WPA1WPA2";
		break;
	case WLAN_AUTH_PSK:
		macptr = "WPAPSK";
		break;
	case WLAN_AUTH_PSK2:
		macptr = "WPA2PSK";
		break;
	case WLAN_AUTH_PSKPSK2:
		macptr = "WPAPSKWPA2PSK";
		break;
	default:
		DBG_ERR("unknown auth mode : %d\n", authMode);
		return ERROR;
		break;
	}

	pSecConfig = &wdev->SecConfig;

	SetWdevAuthMode(pSecConfig, macptr);
	wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

	DBG_INFO("set auth mode :%d\n", authMode);

	return OK;
}

#ifdef APCLI_SUPPORT 
static STATUS _setApcliAuth(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 authMode)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr = NULL;
	RTMP_STRING *cipherPtr = NULL;

	switch (authMode)
	{
	case WLAN_AUTH_NONE:
	case WLAN_AUTH_WEP_OPEN:
		macptr = "OPEN";
		cipherPtr = "NONE";
		break;
	case WLAN_AUTH_WEP_SHARE:
		macptr = "SHARED";
		cipherPtr = "WEP";
		break;
	case WLAN_AUTH_WEP_AUTO:
		macptr = "WEPAUTO";
		cipherPtr = "WEP";
		break;
	case WLAN_AUTH_WPA:
		macptr = "WPA";
		cipherPtr = "TKIPAES";
		break;
	case WLAN_AUTH_WPA2:
		macptr = "WPA2";
		cipherPtr = "TKIPAES";
		break;
	case WLAN_AUTH_WPAWPA2:
		macptr = "WPA1WPA2";
		cipherPtr = "TKIPAES";
		break;
	case WLAN_AUTH_PSK:
		macptr = "WPAPSK";
		cipherPtr = "AES";
		break;
	case WLAN_AUTH_PSK2:
		macptr = "WPA2PSK";
		cipherPtr = "AES";
		break;
	case WLAN_AUTH_PSKPSK2:
		macptr = "WPAPSKWPA2PSK";
		cipherPtr = "TKIPAES";
		break;
	default:
		DBG_ERR("unknown auth mode : %d\n", authMode);
		return ERROR;
		break;
	}

	pSecConfig = &pAd->StaCfg[unit].wdev.SecConfig;

	SetWdevAuthMode(pSecConfig, macptr);

	/* Since wlanApp won't configure cipher, we should set it here
	  * according to SLP platform(simple_config.sh) */
	SetWdevEncrypMode(pSecConfig, cipherPtr);

	DBG_INFO("set apcli auth mode :%d\n", authMode);

	return OK;
}

#endif
static STATUS _ioctlSetAuth(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 authMode)
{
	STATUS ret = ERROR;

	DBG_INFO("set authMode %d for unit %d\n", authMode, unit);

	if (IS_APVAP_UNIT(unit))
	{
		ret = _setMbssAuth(pAd, GET_VAP_IDX(unit), authMode);
	}
#ifdef APCLI_SUPPORT 
	else
	{
		ret = _setApcliAuth(pAd, GET_VAP_IDX(unit), authMode);
	}
#endif		
	return ret;
}

static STATUS _ioctlSetCipher(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 cipher)
{
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr = NULL;

	DBG_INFO("set cipher %d for unit %d\n", cipher, unit);

	if (NULL == wdev)
	{
		DBG_ERR("end object of mbssid %d is null!\n", unit);
		return ERROR;
	}

	switch (cipher)
	{
	case WLAN_CIPHER_NONE:
		macptr = "NONE";
		break;
	case WLAN_CIPHER_WEP:
		macptr = "WEP";
		break;
	case WLAN_CIPHER_TKIP:
		macptr = "TKIP";
		break;
	case WLAN_CIPHER_AES:
		macptr = "AES";
		break;
	case WLAN_CIPHER_AUTO:
		macptr = "TKIPAES";
		break;
	default:
		DBG_ERR("unknown cipher %d\n", cipher);
		return ERROR;
		break;
	}

	pSecConfig = &wdev->SecConfig;

	SetWdevEncrypMode(pSecConfig, macptr);

	DBG_INFO("set wep status :%s\n", macptr);
	
	return OK;
}

static STATUS _ioctlSetKey(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *key, UINT8 len)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	DBG_INFO("set key = %s for unit %d\n", key, unit);

	if (IS_APVAP_UNIT(unit))
	{
		ULONG len = strlen(key);

		pSecConfig = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.SecConfig;
		if (len < 65)
		{
			os_move_mem(pSecConfig->PSK, key, strlen(key));
			pSecConfig->PSK[strlen(key)] = '\0';
		} else
			pSecConfig->PSK[0] = '\0';
#ifdef WSC_AP_SUPPORT
			NdisZeroMemory(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].WscControl.WpaPsk, 64);
			pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].WscControl.WpaPskLen = 0;
			if ((len >= 8) && (len <= 64))
			{
				NdisMoveMemory(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].WscControl.WpaPsk, key, len);
				pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].WscControl.WpaPskLen = len;
			}
#endif /* WSC_AP_SUPPORT */
	}
#ifdef APCLI_SUPPORT
	else
	{
		pSecConfig = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev.SecConfig; // TODO: do not support DBDC mode.
		
		if (strlen(key) < 65)
		{
			os_move_mem(pSecConfig->PSK, key, strlen(key));
			pSecConfig->PSK[strlen(key)] = '\0';
		} else
			pSecConfig->PSK[0] = '\0';
	}
#endif
	


	return OK;
}
static STATUS _ioctlSetUMode(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 uMode)
{
	struct wifi_dev *wdev = NULL;
	char str[3] = {0};
	UCHAR phyMode = 0;
	USHORT wmode = 0;
	INT i;
	INT ret = 0;

	switch (uMode)
	{
	case WLAN_MODE_11BGN_MIXED:
		phyMode = PHY_11BGN_MIXED;
		break;
	case WLAN_MODE_11A_11N_MIXED:
		phyMode = PHY_11AN_MIXED;
		break;
	case WLAN_MODE_11A_11N_11AC_MIXED:
		phyMode = PHY_11VHT_N_A_MIXED;
		break;
	default:
		DBG_ERR("unknown mode : %d\n", uMode);
		return ERROR;
	}
	ret = snprintf(str, sizeof(str), "%u", phyMode);
	if (os_snprintf_error(sizeof(str), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");

	/* judge 2.4G/5G before config phymode */
	/* pAd->ApCfg.MBSSID[unit].wdev.PhyMode = cfgmode_2_wmode(phyMode); */
	wmode = cfgmode_2_wmode(phyMode);

	/* referenced for Set_Cmm_WirelessMode_Proc routine*/
	if (IS_APVAP_UNIT(unit))
	{
		wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
		wdev->PhyMode = wmode;

		RT_CfgSetMbssWirelessMode(pAd, str);
#ifdef MBSS_SUPPORT
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			struct wifi_dev *TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;

			/*update WmmCapable*/
			if (!wmode_band_equal(TmpWdev->PhyMode, wmode))
				continue;
			TmpWdev->PhyMode = wmode;
			TmpWdev->bWmmCapable = pAd->ApCfg.MBSSID[i].bWmmCapableOrg;
		}
#endif
	}

	/* TODO: need to sync stavap wirelessmode to apvap?  */
	if(IS_STAVAP_UNIT(unit))
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		wdev = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev;
		wdev->PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
#endif /*APCLI_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
		RT_CfgSetWirelessMode(pAd, str, wdev);
	}

	DBG_INFO("set u mode :%d\n", pAd->CommonCfg.cfg_wmode);

	return OK;
}

static STATUS _ioctlSetChanWidth(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 chanWidth)
{
	UCHAR vht_bw = VHT_BW_80;
	UCHAR ht_bw = BW_20;

	switch (chanWidth)
	{
		case WLAN_CHAN_WIDTH_20M:
			vht_bw = VHT_BW_2040;
			ht_bw = BW_20;
			break;

		case WLAN_CHAN_WIDTH_40M:
			vht_bw = VHT_BW_2040;
			ht_bw = BW_40;
			break;

		case WLAN_CHAN_WIDTH_AUTO:
		case WLAN_CHAN_WIDTH_80M:
			vht_bw = VHT_BW_80;
			ht_bw = BW_40;
			break;
		case WLAN_CHAN_WIDTH_80M_ADD_80M:
		default:
			return ERROR;		
	}

	//if (vht_bw == VHT_BW_2040)
	//{
	if (IS_APVAP_UNIT(unit))
	{
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
		/* only config same band wdev */
		/* wlan_config_set_ht_bw_all(&pAd->wpf,ht_bw); */
		wlan_config_set_ht_bw(wdev,ht_bw);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"HT: Channel Width = %s\n", (ht_bw == BW_40) ? "40 MHz" : "20 MHz");
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd){
			struct wifi_dev *tmpWdev;
			int i = 0;
			UCHAR band_idx = HcGetBandByWdev(wdev);

			for (i = 0; i < WDEV_NUM_MAX; i++) {
				tmpWdev = pAd->wdev_list[i];
				if (tmpWdev && HcIsRadioAcq(tmpWdev)
					&& HcGetBandByWdev(tmpWdev) == band_idx)
				{
					wlan_config_set_ht_bw(tmpWdev,ht_bw);
				}
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}
	//}

	/* vht_bw is useless for 2.4G DBDC band so sync all wdev */
	wlan_config_set_vht_bw_all(&pAd->wpf,vht_bw);


#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.bBssCoexEnable = FALSE;
#endif
	return OK;
}

static STATUS _ioctlSetTxPwr(RTMP_ADAPTER *pAd,UINT32 unit, UINT8 data)
{
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

#ifndef TLED_BIT	
// TODO: check for DBDC mode
	switch (data)
	{
	case WLAN_ADV_TX_POWER_HIGH:
		pAd->CommonCfg.ucTxPowerPercentage = (ULONG)HIGH_TX_PWR;
		break;
	case WLAN_ADV_TX_POWER_MEDIUM:
		pAd->CommonCfg.ucTxPowerPercentage = (ULONG)MEDIUM_TX_PWR;
		break;
	case WLAN_ADV_TX_POWER_LOW:
		pAd->CommonCfg.ucTxPowerPercentage = (ULONG)LOW_TX_PWR;
		break;
	default:
		DBG_ERR("unknown TxPwr: %d\n", data);
		return ERROR;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pAd->CommonCfg.ucTxPowerDefault = pAd->CommonCfg.ucTxPowerPercentage;
	}
#endif /* CONFIG_STA_SUPPORT */

	TxPowerDropCtrl(pAd, pAd->CommonCfg.ucTxPowerPercentage, band_idx);

	DBG_INFO("set TxPwr: %lu\n", pAd->CommonCfg.ucTxPowerPercentage);

#endif

	return OK;
}

static STATUS _ioctlSetInterval(RTMP_ADAPTER *pAd, UINT16 intval)
{
	if (intval >= WLAN_BINTVAL_MIN && intval <= WLAN_BINTVAL_MAX)
	{
		pAd->CommonCfg.BeaconPeriod = intval;
	}
	else
	{
		pAd->CommonCfg.BeaconPeriod = WLAN_BINTVAL_DEF;
	}

	DBG_INFO("set intval : %d\n", pAd->CommonCfg.BeaconPeriod);

	return OK;
}

static STATUS _ioctlSetDtim(RTMP_ADAPTER *pAd, UINT8 dtim)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (dtim < WLAN_DTIM_MIN)
		{
			DBG_ERR("dtim is out of range, use default value\n");
			pAd->ApCfg.DtimPeriod = WLAN_DTIM_DEF;
		}
		else
		{
			pAd->ApCfg.DtimPeriod = dtim;
		}
	}

	return OK;
}

static STATUS _ioctlSetShortGI(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 sgi)
{
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;

	if (!wdev)
	{
		DBG_INFO("invalid vap index : %d\n", GET_VAP_IDX(unit));
		return ERROR;
	}

	if (TRUE == sgi)
	{
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
		wlan_config_set_vht_sgi(wdev, GI_400);
	}
	else
	{
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
		wlan_config_set_vht_sgi(wdev, GI_800);
	}

	return OK;
}

static STATUS _ioctlSetKeyIdx(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 keyIdx)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	DBG_INFO("set keyIdx %d for unit %d\n", keyIdx, unit);

	if (IS_APVAP_UNIT(unit))
	{
		pSecConfig = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.SecConfig;
	}
#ifdef APCLI_SUPPORT
	else
	{
		pSecConfig = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev.SecConfig;
	}
#endif

	if (pAd->ApCfg.BssidNum <= GET_VAP_IDX(unit))
	{
		DBG_ERR("SetKeyIdx unit: %d is out of range\n", unit);
		return ERROR;
	}
	
	if((MAX_WEP_NKID - 1) < keyIdx)
	{
		DBG_ERR("keyIdx is out of range, use default value\n");
		pSecConfig->PairwiseKeyId = 0;
	}				
	else
	{
		pSecConfig->PairwiseKeyId = (UCHAR)keyIdx;
	}

	return OK;
}

static STATUS _ioctlSetIsolate(RTMP_ADAPTER *pAd, UINT8 unit, UINT8 isolate)
{
	if ((VAP_UNIT_MAX) <= unit)
	{
		return ERROR;
	}

	if (TRUE == isolate)
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].IsolateInterStaTraffic = TRUE;
        /* isolate STAs in different SSID but in same phy dev */
        pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
	}
	else
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].IsolateInterStaTraffic = FALSE;
        pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;
	}

	return OK;
}

static STATUS _ioctlGetStaInfo(RTMP_ADAPTER *pAd, UINT32 unit, WLAN_STA_LIST_T *pStaInfo)
{
	UINT16 i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	WLAN_STA_ENTRY_T *pStaEntry = NULL;
	ULONG DataRate=0;

	pStaInfo->staCnt = 0;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
	{
		pEntry = entry_get(pAd, i);

		if ((NULL == pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_dev)
			|| ((NULL != pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_dev) && (GET_VAP_IDX(unit) != pEntry->apidx))
			|| !(IS_ENTRY_CLIENT(pEntry)))
		{
			continue;
		}

        DataRate=0;
        getRate(pEntry->HTPhyMode, &DataRate);

		pStaEntry = &pStaInfo->staEntry[pStaInfo->staCnt++];
		MAC_ADDR_COPY(pStaEntry->mac, pEntry->Addr);

		if (SST_ASSOC == pEntry->Sst)
		{
			pStaEntry->status = STA_STATUS_ASSOC;
		}
		else
		{
			pStaEntry->status = STA_STATUS_AUTH;
		}

		pStaEntry->assoctime = pEntry->StaConnectTime;
		pStaEntry->inact = pEntry->NoDataIdleCount;
		pStaEntry->rate = DataRate;
		pStaEntry->ccq = (UINT8)(pEntry->RssiSample.LastRssi[0] - RSSI_BASE);
		pStaEntry->txPkts = (UINT32)pEntry->TxPackets.QuadPart;
		pStaEntry->rxPkts = (UINT32)pEntry->RxPackets.QuadPart;
	}

	return OK;
}

static STATUS _ioctlScanReq(RTMP_ADAPTER *pAd, UINT32 unit)
{
	struct wifi_dev *wdev = NULL;

	if (IS_APVAP_UNIT(unit))
	{
		wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
	}
#ifdef APCLI_SUPPORT
	else
	{
		wdev = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev;
	}
#endif

	if (!wdev)
	{
		DBG_INFO("invalid vap index : %d\n", unit);
		return ERROR;
	}

	if (wlanDrvStartDone == FALSE)
	{
		return ERROR;
	}


	ApSiteSurvey_by_wdev(pAd, NULL, SCAN_PASSIVE, FALSE, wdev);


	return OK;
}

STATUS _ioctlScanResults(RTMP_ADAPTER *pAd, UINT32 unit, WLAN_5G_AP_LIST_T *pApStatus)
{
	UINT i = 0;
	UINT j = 0;
	BSS_ENTRY	*pBss = NULL;
	WLAN_5G_AP_ENTRY_T tmpBss;
	WLAN_5G_AP_ENTRY_T *pApEntry = NULL;
	struct wifi_dev *wdev = NULL;
	PBSS_TABLE pScanTab = NULL;
	
	if (IS_APVAP_UNIT(unit))
	{
		wdev = &pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev;
	}
#ifdef APCLI_SUPPORT
	else
	{
		wdev = &pAd->StaCfg[GET_VAP_IDX(unit)].wdev;
	}
#endif

	if (!wdev)
	{
		DBG_INFO("invalid vap index : %d\n", GET_VAP_IDX(unit));
		return ERROR;
	}
	

	if (wlanDrvStartDone == FALSE)
	{
		return ERROR;
	}

	if (scan_in_run_state(pAd, wdev) == TRUE)
	{
		return ERROR;
	}

	pScanTab = get_scan_tab_by_wdev(pAd, wdev);
	/* add check for array overflow if BssNr > 64 */
	pApStatus->apCnt = (pScanTab->BssNr < MAX_BSS_CNT) ? pScanTab->BssNr : MAX_BSS_CNT;
	for (i = 0; i < pApStatus->apCnt; i++)
	{
		pBss = &pScanTab->BssEntry[i];
		pApEntry = &pApStatus->apEntry[i];
		memset(pApEntry, 0, sizeof(WLAN_AP_ENTRY_T));

		MAC_ADDR_COPY(pApEntry->bssid, pBss->Bssid);
		memcpy(pApEntry->ssid, pBss->Ssid, pBss->SsidLen);
		pApEntry->ssid[pBss->SsidLen] = '\0';
		pApEntry->rssi = pBss->Rssi - RSSI_BASE;
		pApEntry->channel = pBss->Channel;

		if (HAS_VHT_CAPS_EXIST(pBss->ie_exists))
		{
			pApEntry->uMode = WLAN_MODE_11A_11N_11AC_MIXED;
		}
		else
		{
			pApEntry->uMode = WLAN_MODE_11A_11N_MIXED;
		}

		if (IS_NO_SECURITY(pBss))
		{
			pApEntry->authMode = WLAN_AUTH_NONE;
		}
		else if (IS_AKM_OPEN(pBss->AKMMap))
		{
			pApEntry->authMode = WLAN_AUTH_WEP_AUTO;
		}
		else if (IS_AKM_WPA1(pBss->AKMMap))
		{
			pApEntry->authMode = WLAN_AUTH_WPA;
		}
		else if (IS_AKM_WPA2(pBss->AKMMap))
		{
			pApEntry->authMode = WLAN_AUTH_WPA2;
		}
		else if (IS_AKM_WPA1PSK(pBss->AKMMap))
		{
			pApEntry->authMode = WLAN_AUTH_PSK;
		}
		else if (IS_AKM_WPA2PSK(pBss->AKMMap))
		{
			pApEntry->authMode = WLAN_AUTH_PSK2;
		}
	}	

	/* sort bss by rssi */
	for (i = 0; i < pApStatus->apCnt; i++)
	{
		for (j = 0; j < pApStatus->apCnt - i - 1; j++)
		{
			if ((j + 1) >= MAX_BSS_CNT)
				break;
			else if (pApStatus->apEntry[j].rssi < pApStatus->apEntry[j + 1].rssi) {
				tmpBss = pApStatus->apEntry[j];
				pApStatus->apEntry[j] = pApStatus->apEntry[j + 1];
				pApStatus->apEntry[j + 1] = tmpBss;
			}
		}
	}

	return OK;
}

#ifdef APCLI_SUPPORT
#endif

/* TODO: no context calls this routine, remove it ? */
static STATUS _ioctlResetDev(RTMP_ADAPTER *pAd)
{
    APStop(pAd, &pAd->ApCfg.MBSSID[MAIN_MBSSID], AP_BSS_OPER_ALL);
    APStartUp(pAd, &pAd->ApCfg.MBSSID[MAIN_MBSSID], AP_BSS_OPER_ALL);

    return OK;
}


static STATUS _ioctlSetTurboLed(RTMP_ADAPTER *pAd, BOOL on)
{
    #ifdef TLED_BIT
	gpioWrite(TLED_BIT, (TRUE == on) ? LED_LIGHTEN : LED_DARKEN);
	#endif
	
	return OK;
}

static STATUS _ioctlGetMfgStatus(RTMP_ADAPTER *pAd, BOOL *isMfg)
{
	if (NULL != isMfg)
	{
#ifdef TLED_BIT
#ifdef CONFIG_ATE
		if ((NULL != pAd) && (ATE_ON(pAd)))
		{
			*isMfg = TRUE;
		}
		else
		{
			*isMfg = FALSE;
		}
#else /* RALINK_ATE */
		*isMfg = FALSE;
#endif
#else
		*isMfg = TRUE;
#endif
	}

	return OK;
}

#ifdef APCLI_SUPPORT 
static STATUS _ioctlGetStatus(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *status)
{
	/* TODO: consider DBDC mode and correct fsm state*/
	STA_ADMIN_CONFIG *pApCliEntry = &pAd->StaCfg[GET_VAP_IDX(unit)];

	if (TRUE == pApCliEntry->ApcliInfStat.Valid)
	{
		if (IS_AKM_OPEN(pApCliEntry->wdev.SecConfig.AKMMap))
		{
			*status = STA_STATE_RUN;
		}

		if (IS_AKM_WPA_CAPABILITY(pApCliEntry->wdev.SecConfig.AKMMap))
		{
			if (WPA_802_1X_PORT_SECURED == pApCliEntry->wdev.PortSecured)
			{
				*status = STA_STATE_RUN;
			}
			else
			{
				*status = STA_STATE_ASSOC;
			}
		}
	}
	else
	{
		/* TODO: idle stats represents both connected and down, fix it */
		switch (pApCliEntry->wdev.cntl_machine.CurrState)
		{
		case CNTL_WAIT_AUTH:
		case CNTL_WAIT_AUTH2:
			*status = STA_STATE_AUTH;
			break;
		case CNTL_WAIT_ASSOC:
			*status = STA_STATE_ASSOC;
			break;
		case CNTL_IDLE:
			*status = STA_STATE_RUN;
			break;
		default:
			*status = STA_STATE_DOWN;
			break;
		}
	}

	return OK;
}
#endif

/* TODO: correct unit for wps ioctl */
#ifndef EXCLUDE_WPS
static STATUS _ioctlSetBcnIe(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *data, UINT32 len)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[unit];
	UINT8 *tmpBuf = pMbss->WscIEBeacon.Value;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[unit].wdev;

	if (len > 255)
	{
		DBG_ERR("length of BCN WPS IE is too long!\n");
		return ERROR;
	}

	if (0 == len)
	{
		/* just set the ValueLen to ZERO will clear this IE from Beacon */
		pMbss->WscIEBeacon.ValueLen = len;
	}
	else
	{
		tmpBuf[0] = IE_WFA_WSC;		/* ie id */
		tmpBuf[1] = len;			/* ie length */

		if ((ARRAY_SIZE(pMbss->WscIEBeacon.Value) >= 2) && len > (ARRAY_SIZE(pMbss->WscIEBeacon.Value) - 2))
			len = (ARRAY_SIZE(pMbss->WscIEBeacon.Value) - 2);

		NdisMoveMemory(&tmpBuf[2], data, len);	/* ie payload */

		pMbss->WscIEBeacon.ValueLen = len + 2;
	}
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

	return OK;
}

static STATUS _ioctlSetRspIe(RTMP_ADAPTER * pAd, UINT32 unit, UINT8 * data, UINT32 len)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[unit];
	UINT8 *tmpBuf = pMbss->WscIEProbeResp.Value;
	
	if (len > 255)
	{
		DBG_ERR("length of probe resp WPS IE is too long!\n");
		return ERROR;
	}

	if (0 == len)
	{
		/* just set the ValueLen to ZERO will clear this IE from ProbeResponse */
		pMbss->WscIEProbeResp.ValueLen = len;
	}
	else
	{
		tmpBuf[0] = IE_WFA_WSC;		/* ie id */
		tmpBuf[1] = len;			/* ie length */
		NdisMoveMemory(&tmpBuf[2], data, len);	/* ie payload */

		pMbss->WscIEProbeResp.ValueLen = len + 2;
	}

	return OK;
}

/* just for test wps */
static STATUS _ioctlSendEvent(RTMP_ADAPTER *pAd)
{
	wlStartWPSByPBC();
	
	return OK;
}

static STATUS _ioctlEapSend(RTMP_ADAPTER *pAd, UINT8 *data, UINT16 len)
{
	UINT8 *mac = data;
	MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, mac);

	if (NULL == pEntry)
	{
		DBG_WPS(__MAC__"can't find the assoc node\n", MAC_PRINT(mac));
		return ERROR;
	}

	APEapPacketSend(pAd, pEntry, data + MAC_ADDR_LEN, len);

	return OK;
}

static STATUS _ioctlSetWpsInProgress(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 flag)
{
	if (flag)
	{
		pAd->ApCfg.MBSSID[unit].bWscInProgress = TRUE;
	}
	else
	{
		pAd->ApCfg.MBSSID[unit].bWscInProgress = FALSE;
	}

	return OK;
}
#endif

static STATUS _ioctlSetLanAccess(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 enable)
{
	if (TRUE == enable)
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].bLanAccessForbidden = FALSE;
	}
	else
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].bLanAccessForbidden = TRUE;
	}

	DBG_INFO("set lan access %s\n", pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].bLanAccessForbidden ? "TRUE" : "FALSE");

	return OK;
}

static STATUS _ioctlAddAclMac(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *mac, UINT32 len)
{
	char cmdBuf[128] = {0};
	INT ret = 0;

	struct wifi_dev *wdev = &(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev);

	if (len != MAC_ADDR_LEN)
	{
		return ERROR;
	}

	ret = snprintf(cmdBuf, sizeof(cmdBuf),
			"%s%d set ACLAddEntry=%02x:%02x:%02x:%02x:%02x:%02x",
			RtmpOsGetNetDevName(wdev->if_dev), MAIN_MBSSID,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (os_snprintf_error(sizeof(cmdBuf), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");

	iwpriv(cmdBuf);

	return OK;
}

static STATUS _ioctlDelAclMac(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *mac, UINT32 len)
{
	char cmdBuf[128] = {0};
	INT ret = 0;

	struct wifi_dev *wdev = &(pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev);

	if (len != MAC_ADDR_LEN)
	{
		return ERROR;
	}

	ret = snprintf(cmdBuf, sizeof(cmdBuf),
			"%s%d set ACLDelEntry=%02x:%02x:%02x:%02x:%02x:%02x",
			RtmpOsGetNetDevName(wdev->if_dev), MAIN_MBSSID,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (os_snprintf_error(sizeof(cmdBuf), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");

	iwpriv(cmdBuf);

	return OK;
}

/* TODO: correct unit and device name for wss ioctl */
#ifdef TP_FEATURE_STEER
static STATUS _ioctlAddAclMacSteer(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *mac, UINT32 len)
{
	char cmdBuf[128] = {0};
	INT ret = 0;

	if (len != MAC_ADDR_LEN)
	{
		return ERROR;
	}

	ret = snprintf(cmdBuf, sizeof(cmdBuf),
			"rai%d set ACLAddEntrySteer=%02x:%02x:%02x:%02x:%02x:%02x",
			unit, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (os_snprintf_error(sizeof(cmdBuf), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");

	iwpriv(cmdBuf);

	return OK;
}

static STATUS _ioctlDelAclMacSteer(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *mac, UINT32 len)
{
	char cmdBuf[128] = {0};
	INT ret = 0;

	if (len != MAC_ADDR_LEN)
	{
		return ERROR;
	}

	ret = snprintf(cmdBuf, sizeof(cmdBuf),
			"rai%d set ACLDelEntrySteer=%02x:%02x:%02x:%02x:%02x:%02x",
			unit, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (os_snprintf_error(sizeof(cmdBuf), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");

	iwpriv(cmdBuf);

	return OK;
}

static STATUS _ioctlShowAclMacSteer(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *mac, UINT32 len)
{
	char cmdBuf[128] = {0};
	INT ret = 0;

	if (len != MAC_ADDR_LEN)
	{
		return ERROR;
	}

	ret = snprintf(cmdBuf, sizeof(cmdBuf), "rai%d set ACLShowAllSteer=1", unit);
	if (os_snprintf_error(sizeof(cmdBuf), ret))
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "snprintf error!\n");

	iwpriv(cmdBuf);

	return OK;
}

#endif

static STATUS _ioctlSetAclPolicy(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 policy)
{
	if ((VAP_UNIT_MAX) <= unit)
	{
		return ERROR;
	}

	switch (policy)
	{
	case ACL_POLICY_OPEN:
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].AccessControlList.Policy = 0;
		break;

	case ACL_POLICY_ALLOW:
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].AccessControlList.Policy = 1;
		break;

	case ACL_POLICY_DENY:
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].AccessControlList.Policy = 2;
		break;

	default:
		return ERROR;
	}

	ApUpdateAccessControlList(pAd, GET_VAP_IDX(unit));

	return OK;
}

static STATUS _ioctlSetWme(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 wme)
{
	if ((VAP_UNIT_MAX) <= unit)
	{
		return ERROR;
	}

	if (TRUE == wme)
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bWmmCapable = TRUE;
		
#ifdef MULTI_CLIENT_SUPPORT
		pAd->CommonCfg.bWmm = TRUE;
#endif

	}
	else
	{
		pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bWmmCapable = FALSE;
	}

	pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].bWmmCapableOrg = pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].wdev.bWmmCapable;

	return OK;
}

static STATUS _ioctlSetShortPreamble(RTMP_ADAPTER *pAd, UINT8 disabled)
{
	if (TRUE == disabled)
	{
		pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
	}
	else
	{
		pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto;
	}

	return OK;
}

static STATUS _ioctlSendDeAuth(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 *pStaAddr)
{
	MAC_TABLE_ENTRY *pEntry;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason ;

	pEntry = MacTableLookup(pAd, pStaAddr);

	if (pEntry == NULL)
	{
		return ERROR;
	}

	if (IS_ENTRY_CLIENT(pEntry))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Before reset MacTable, send disassociation packet to client.*/
			if (pEntry->Sst == SST_ASSOC)
			{
				/*  send out a De-authentication request frame*/
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(DBG_LVL_INFO, (" MlmeAllocateMemory fail  ..\n"));
					return ERROR;
				}

				Reason = REASON_DEAUTH_STA_LEAVING;

				DBGPRINT(DBG_LVL_WARN, ("Send DeAuth (Reason=%d) to "MACSTR"\n",
								Reason, MAC2STR(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
									pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
									pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);

				MakeOutgoingFrame(pOutBuffer, &FrameLen,
										sizeof(HEADER_802_11), &DeAuthHdr,
										2, &Reason,
										END_OF_ARGS);

				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
				MlmeFreeMemory(pOutBuffer);
				RtmpusecDelay(5000);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* Delete a entry via WCID */
	MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

	return OK;
}

static  STATUS _ioctlSetVapNewStaNotify(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 enable)
{

	pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].newStaNotifyEnable = enable;

	return OK;
}

static  STATUS _ioctlSetMuMimo(RTMP_ADAPTER *pAd,     UINT8 enable)
{
	pAd->CommonCfg.MUTxRxEnable = enable;

	return OK;
}

static  STATUS _ioctlSetSKU(RTMP_ADAPTER *pAd, UINT32 unit, UINT8 enable)
{
	RTMP_STRING arg[2] = {0};
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* TODO: linux uses rtnl_lock protect os_cookie, woudl vxworls need another lock? */
	pObj->ioctl_if_type = GET_VAP_TYPE(unit);
	pObj->ioctl_if = GET_VAP_IDX(unit);

	snprintf(arg, sizeof(arg), "%d", enable);

	return SetSKUCtrl(pAd, arg);
}

#ifdef TP_FEATURE_STEER
static STATUS _ioctlWssCommon(RTMP_ADAPTER *pAd, void *arg)
{
	STATUS ret = ERROR;
	struct wssCmdReq *req = (struct wssCmdReq *)arg;

	switch(req->cmd)
	{
	case WSS_SET_TPSTEER_ENABLE:
		ret = tps_enable(pAd, req);
		break;
	case WSS_SET_BS_PARAMS:
		ret = tps_set_bs_params(pAd, req);
		break;
	case WSS_SET_TP_PARAMS:
		ret = tps_set_tp_params(pAd, req);
		break;
	case WSS_SEND_BEACON_REQUEST:
		ret = tps_send_beacon_req(pAd, req);
		break;
	case WSS_SEND_BTM_REQUEST:
		ret = tps_send_btm_req(pAd, req);
		break;
	case WSS_SEND_NBR_RESPONSE:
		ret = tps_send_nbr_resp(pAd, req);
		break;
	default:
		break;
	}
	return ret;
}

static STATUS _ioctlWssGetStaList(RTMP_ADAPTER *pAd, UINT32 unit, void *pStaInfo)
{
	UINT16 i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct tps_assoc_list *list = (struct tps_assoc_list *)pStaInfo;
	struct bs_node_associated_ind *staInfo = NULL;
	int max_MCS = 0;

	list->assoc_num = 0;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if ((NULL == pEntry->wdev)
			|| ((NULL != pEntry->wdev) && (unit != pEntry->wdev->func_idx))
			|| !(IS_ENTRY_CLIENT(pEntry)))
		{
			continue;
		}

		staInfo = &list->staInfo[list->assoc_num++];
		MAC_ADDR_COPY(staInfo->macaddr, pEntry->Addr);

		staInfo->assoctime = pEntry->StaConnectTime;
		staInfo->isBTMSupported = (BOOLEAN)(pEntry->ext_cap.BssTransitionManmt);
		staInfo->isRRMSupported = (BOOLEAN)((pEntry->CapabilityInfo & 0x1000) >> RRM_BIT);
		staInfo->datarate_info.max_chwidth = pEntry->HTPhyMode.field.BW;
		staInfo->datarate_info.phymode = pEntry->HTPhyMode.field.MODE;
		staInfo->datarate_info.max_txpower = pEntry->tps_max_tx_power;
		max_MCS = pEntry->MaxHTPhyMode.field.MCS;
		staInfo->datarate_info.max_MCS = max_MCS < 0 ? 0 : max_MCS;
	}

	return OK;
}

#endif

static STATUS _ioctlGetWlanStatus(RTMP_ADAPTER *pAd, UINT8 *status)
{
	*status = wlanDrvStartDone;

	return OK;
}

static STATUS _ioctlGetCapInfo(RTMP_ADAPTER *pAd, UINT8 unit, void *arg)
{
	UINT16 *capInfo = (UINT16 *)arg;

	if (arg == NULL || !VALID_MBSS(pAd, GET_VAP_IDX(unit)))
	{
		return ERROR;
	}

	*capInfo = pAd->ApCfg.MBSSID[GET_VAP_IDX(unit)].CapabilityInfo;

	return OK;
}

STATUS wlan_ioctl(RTMP_ADAPTER *pAd, UINT32 unit, UINT32 cmd, void *arg, UINT32 len)
{
	STATUS ret = OK;

	if ((cmd <= WIOCTL_START) || (cmd >= WIOCTL_END))
	{
		DBG_ERR("command(%d) error. unit = %d\n", cmd, unit);
		return ERROR;
	}
	switch ((unsigned)cmd)
	{
	case WIOCTL_GET_DEBUG:
		ret = _ioctlGetDebug((UINT32*)arg);
		break;
	case WIOCTL_SET_DEBUG:
		ret = _ioctlSetDebug(*(UINT32 *)arg);
		break;
	case WIOCTL_SHOW_DBG_LVLS:
		ret = _ioctlShowDbgLvls(pAd);
		break;
	case WIOCTL_GET_SSID:
		ret = _ioctlGetSsid(pAd, unit, (UINT8 *)arg);
		break;
	case WIOCTL_GET_BSSID:
		ret = _ioctlGetBssid(pAd, unit, (UINT8 *)arg);
		break;
	case WIOCTL_SET_SSID:
		ret = _ioctlSetSsid(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_START_WLAN:
		ret = _ioctlStartWlan(pAd, unit);
		break;
	case WIOCTL_STOP_WLAN:
		ret = _ioctlStopWlan(pAd, unit);
		break;
	case WIOCTL_START_VAP:
		ret = _ioctlStartVap(pAd, unit);
		break;
	case WIOCTL_STOP_VAP:
		ret = _ioctlStopVap(pAd, unit);
		break;
	case WIOCTL_GET_CHANNEL:
		ret = _ioctlGetChannel(pAd, unit, (UINT8 *)arg);
		break;
	case WIOCTL_SET_CHANNEL:
		ret = _ioctlSetChannel(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_HIDESSID:
		ret = _ioctlSetHideSsid(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_DOMAIN:
		ret =  _ioctlSetDomain(pAd, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_RTS:
		ret = _ioctlSetRTS(pAd, *(UINT16 *)arg);
		break;
	case WIOCTL_SET_FRAG:
		ret = _ioctlSetFrag(pAd, *(UINT16 *)arg);
		break;
	case WIOCTL_GET_STAINFO:
		ret = _ioctlGetStaInfo(pAd, unit, arg);
		break;
	case WIOCTL_SET_AUTH:
		ret = _ioctlSetAuth(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_CIPHER:
		ret = _ioctlSetCipher(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_KEY:
		ret = _ioctlSetKey(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_SET_BGNMODE:		
		ret = _ioctlSetUMode(pAd, unit, *(UINT8 *)arg);		
		break;
	case WIOCTL_SET_CHANWIDTH:
		ret = _ioctlSetChanWidth(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_TXPWR:
		ret = _ioctlSetTxPwr(pAd, unit,*(UINT8 *)arg);
		break;
	case WIOCTL_SET_INTVAL:
		ret = _ioctlSetInterval(pAd, *(UINT16 *)arg);
		break;
	case WIOCTL_SET_DTIM:
		ret = _ioctlSetDtim(pAd, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_SGI:
		ret = _ioctlSetShortGI(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_KEYIDX:
		ret = _ioctlSetKeyIdx(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_ISOLATE:
		ret = _ioctlSetIsolate(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SCAN_REQ:
		ret = _ioctlScanReq(pAd, unit);
		break;
	case WIOCTL_SCAN_RESULTS:
		ret = _ioctlScanResults(pAd, unit, (WLAN_5G_AP_LIST_T *)arg);
		break;
#ifdef APCLI_SUPPORT 
	case WIOCTL_GET_STATUS:
		ret = _ioctlGetStatus(pAd, unit, (UINT8 *)arg);
		break;
#endif
	case WIOCTL_SET_ACLPOLICY:
		ret = _ioctlSetAclPolicy(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_ADD_ACLMAC:
		ret = _ioctlAddAclMac(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_DEL_ACLMAC:
		ret = _ioctlDelAclMac(pAd, unit, (UINT8 *)arg, len);
		break;
#ifndef EXCLUDE_WPS
	case WIOCTL_SET_BCNIE:
		ret = _ioctlSetBcnIe(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_SET_RSPIE:
		ret = _ioctlSetRspIe(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_SHOW_PKT:
		ret = _ioctlSendEvent(pAd);
		break;
	case WIOCTL_SEND_EAP:
		ret = _ioctlEapSend(pAd, (UINT8 *)arg, len);
		break;
	case WIOCTL_SET_WPS_IN_PROGRESS:
		ret = _ioctlSetWpsInProgress(pAd, unit, *(UINT8 *)arg);
		break;
#endif

	case WIOCTL_DEAUTH:
		_ioctlSendDeAuth(pAd, unit, (UINT8 *)arg);
		break;
	case WIOCTL_NEW_STA_NOTIFY_ENABLE:
		_ioctlSetVapNewStaNotify(pAd, unit, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_MUMIMO:
		_ioctlSetMuMimo(pAd, *(UINT8 *)arg);
		break;
	case WIOCTL_SET_SKU:
		_ioctlSetSKU(pAd, unit, *(UINT8 *)arg);
		break;
#ifdef TP_FEATURE_STEER
	case WIOCTL_WSS_COMMON:
		ret = _ioctlWssCommon(pAd, (void *)arg);
		break;
	case WIOCTL_WSS_GET_STALIST:
		ret = _ioctlWssGetStaList(pAd, unit, (void *)arg);
		break;
	case WIOCTL_ADD_ACLMAC_STEER:
		ret = _ioctlAddAclMacSteer(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_DEL_ACLMAC_STEER:
		ret = _ioctlDelAclMacSteer(pAd, unit, (UINT8 *)arg, len);
		break;
	case WIOCTL_SHOW_ACL_STEER:
		ret = _ioctlShowAclMacSteer(pAd, unit, (UINT8 *)arg, len);
		break;
#endif
	case WIOCTL_GET_WLAN_STATUS:
		ret = _ioctlGetWlanStatus(pAd, (UINT8 *)arg);
		break;
	case WIOCTL_GET_CAPINFO:
		ret = _ioctlGetCapInfo(pAd, unit, (void *)arg);
		break;
	default:
		ret = ERROR;
		break;
	}

	return ret;
}
