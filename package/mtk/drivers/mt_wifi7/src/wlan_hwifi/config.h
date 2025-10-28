/*
 * Copyright (c) [2020] MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __MTK_CONFIG_H
#define __MTK_CONFIG_H

#ifdef CONFIG_HWIFI_HE_SUPPORT
#define MAC80211_HE_SUPPORT
#define MAC80211_ADV_FEATURES
#endif

#define MAX_BAND_NUM 3
#define MAX_INTERFACES		4
#define MAX_WMM_SETS		4
#define WTBL_SIZE			288
#define MAX_WMM_Q			4 /*BK BE VI VO*/


/*driver can support wifi system max cnt*/
#define WSYS_CAR_MAX 4
#define WSYS_BSS_MAX 64
#define WSYS_STA_MAX 1024

#define WSYS_MLD_BSS_MAX 72 /* MLD: 0~64 for AP, 65 for STA */
#define WSYS_MLD_STA_MAX 312
#define WSYS_PHY_MAX 8
#define MAX_TRANS_NUM 16

enum {
	MTK_EERPOM_MODE_HEADER,
	MTK_EEPROM_MODE_EFUSE,
};

#define DEFAULT_EEPROM_MDOE MTK_EEPROM_MODE_EFUSE


#ifdef HW_DBG
#define hwf_printk(fmt, args...) pr_info(fmt, ## args)
#else
#define hwf_printk(fmt, args...)
#endif

#endif
