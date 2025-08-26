/******************************************************************************
 *
 * Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
 * All rights reserved.
 *
 * FILE NAME  :   wlan_if.h
 * VERSION    :   1.0
 * DESCRIPTION:   Define the interfaces.
 *
 * AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
 * CREATE DATE:   10/17/2011
 *
 * HISTORY    :
 * 01   10/17/2011  zhengxinggu     Create.
 * 02   05/30/2014  yangyang        Modify it for ralink interfaces.
 ******************************************************************************/
#ifndef _WLAN_IF_H_
#define _WLAN_IF_H_

/* The os interfaces needed to be called by app */
STATUS wlanInit_dualBand(UINT32 devno);
STATUS	os_wlanInit_2G(UINT32 devno);
STATUS	os_wlanInit(UINT32 devno);
STATUS	os_wlanExit(void);
STATUS	os_wlanIoctlEx(UINT32 devno, UINT32 unit, UINT32 cmd, void *data, UINT32 len);
STATUS	os_wlanIoctlEx_2G(UINT32 devno, UINT32 unit, UINT32 cmd, void * data, UINT32 len);
int iwpriv_2G(char *pCmd);
int iwpriv(char *pCmd);

#endif /* _WLAN_IF_H_ */
