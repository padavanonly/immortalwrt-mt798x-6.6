/******************************************************************************
*
* Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* FILE NAME  :   wlan_event.h
* VERSION    :   1.0
* DESCRIPTION:   wlan app event protocol definitions.
*
* AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
* CREATE DATE:   01/14/2011
*
* HISTORY    :
* 01   01/14/2011  zhengxinggu     Create.
* 02   15/11/2012  suqiang         Add two WLAN_MSG command type for set tx/rx chain mask.
* 03   05/12/2012  suqiang		   Add cmd type to getting ProductID for MFG.
*
******************************************************************************/
#ifndef _WLAN_EVENT_H_
#define _WLAN_EVENT_H_

#define MAGIC_HI		0x7F54502D
#define MAGIC_LO		0x4C494E4B

#define WDS_DETECT_NLOGGED		0
#define WDS_DETECT_LOGGED		1

#define WDS_DETECT_FINISHED_ADDR3		0
#define WDS_DETECT_FINISHED_ADDR4		1
#define WDS_DETECT_IN_PROGRESS			2

enum
{
	WLAN_MSG_EAP = 0,
	WLAN_MSG_EAPID,
	WLAN_MSG_WPS,
	WLAN_MSG_PROBREQ,

	WLAN_MSG_START_MFG = 0x80,
	WLAN_MSG_STOP_MFG,
	WLAN_MSG_SET_CHAN,
	WLAN_MSG_SET_CHAIN,
	WLAN_MSG_SET_CAL,
	WLAN_MSG_SET_FREQERR,
	WLAN_MSG_SAVE_CAL,
	WLAN_MSG_TX_PKT,
	WLAN_MSG_SET_TXCHAIN,
	WLAN_MSG_SET_RXCHAIN,
	WLAN_MSG_DUAL_CHECK_PRODUCTID,
	WLAN_MSG_GET_PRODUCTID = 0x90,
	WLAN_MSG_START_WARMUP,
	WLAN_MSG_STOP_WARMUP,
	WLAN_MSG_SET_TX99,
	WLAN_MSG_SET_TXCONTINUE,
	WLAN_MSG_SET_TURBOMODE,
	WLAN_MSG_GET_TURBOMODE,
	WLAN_MSG_SET_HWID,
	WLAN_MSG_GET_HWID,
	WLAN_MSG_GET_DEVICEID,
	WLAN_MSG_SET_DEVICEID,
	WLAN_MSG_GET_FWID,
	WLAN_MSG_SET_FWID,
	WLAN_MSG_GET_UID,
	WLAN_MSG_SET_RSA,
	WLAN_MSG_GET_HWIDDES,
	WLAN_MSG_SET_HWIDDES,
	WLAN_MSG_SET_PAPRD,
	WLAN_MSG_SET_CHANNEL,
	WLAN_MSG_SET_PHYMODE,
	WLAN_MSG_NEWSTA
};

struct wlan_upnp_msg
{
	UINT32	addr;
	UINT32	len;
	UINT8	type;
	UINT8	data[SIZE_1024_BYTES];
}__packed;

struct wlan_event_msg
{
	UINT32				magicHi;
	UINT32				magicLo;
	UINT8				devno;
	UINT8				unit;
	UINT8 				type;
	UINT8				val8;
	UINT32				val32;
	UINT16				val16;
	UINT8				macaddr[MAC_ADDR_LEN];
}__packed;

STATUS wlan_eventIndicate(void *pAd, void *wdev, UINT8 *macaddr, UINT32 msg, UINT8 *data, UINT32 len);

#endif /* _WLAN_EVENT_H_ */
