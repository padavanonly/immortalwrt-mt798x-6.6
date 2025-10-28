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
 ***************************************************************************

	Module Name:
	ee_flash.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RTMP_FLASH_SUPPORT

#include	"rt_config.h"
#include "hdev/hdev.h"

/*decision flash api by compiler flag*/
#ifdef PROPRIETARY_DRIVER_SUPPORT
/*
* @ used for proprietary driver support, can't read/write mtd on driver
* @ read: mtd flash patrition use request firmware to load
* @ write: write not support, use ated to write to flash
*/
static void flash_bin_read(void *ctrl, UCHAR *p, ULONG a, ULONG b)
{
	struct _RTMP_ADAPTER *ad = ((struct hdev_ctrl *) ctrl)->priv;
	UCHAR *buffer = NULL;
	UINT32 len = 0;
	UCHAR *name = get_dev_eeprom_binary(ad);

	/*load from request firmware*/
	os_load_code_from_bin(ad, &buffer, name, &len);

	if (len > 0 && buffer != NULL) {
		os_move_mem(p, buffer + a, b);
		os_free_mem(buffer);
	} else if (buffer != NULL)
		os_free_mem(buffer);
}

static void flash_bin_write(void *ctrl, UCHAR *p, ULONG a, ULONG b)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
		 "proprietary driver not support flash write, will write on ated.\n");
}

#define flash_read(_ctrl, _ptr, _offset, _len) flash_bin_read(_ctrl, _ptr, _offset, _len)
#define flash_write(_ctrl, _ptr, _offset, _len) flash_bin_write(_ctrl, _ptr, _offset, _len)

#else

/*============================================================================*/
#ifdef RA_MTD_RW_BY_NUM

/*
* @ The flag "CONFIG_RALINK_FLASH_API" is used for APSoC Linux SDK
*/

#if defined(CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif
extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);

#define flash_read(_ctrl, _ptr, _offset, _len) ra_mtd_read(MTD_NUM_FACTORY, 0, (size_t)_len, _ptr)
#define flash_write(_ctrl, _ptr, _offset, _len) ra_mtd_write(MTD_NUM_FACTORY, 0, (size_t)_len, _ptr)

#else

#ifdef CONFIG_WIFI_MTD
/*
* @ used mtd mode flash partition from proprietary driver mt_wifi_mtd.c
*/
int mt_mtd_write_nm_wifi(char *name, loff_t to, size_t len, const u_char *buf);
int mt_mtd_read_nm_wifi(char *name, loff_t from, size_t len, u_char *buf);

#define flash_read(_ctrl, _ptr, _offset, _len) mt_mtd_read_nm_wifi("Factory", _offset, (size_t)_len, _ptr)
#define flash_write(_ctrl, _ptr, _offset, _len) mt_mtd_write_nm_wifi("Factory", _offset, (size_t)_len, _ptr)

#else
/*
* @ use sdk export func.
*/

extern int ra_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);

#define flash_read(_ctrl, _ptr, _offset, _len) ra_mtd_read_nm("Factory", _offset, (size_t)_len, _ptr)
#define flash_write(_ctrl, _ptr, _offset, _len) ra_mtd_write_nm("Factory", _offset, (size_t)_len, _ptr)

#endif /*CONFIG_WIFI_MTD*/
#endif /*RA_MTD_RW_BY_NUM*/
#endif /*CONFIG_PROPRIETERY_DRIVER*/


void RtmpFlashRead(
	void *hdev_ctrl,
	UCHAR *p,
	ULONG a,
	ULONG b)
{
	flash_read(hdev_ctrl, p, a, b);
}

void RtmpFlashWrite(
	void *hdev_ctrl,
	UCHAR *p,
	ULONG a,
	ULONG b)
{
	flash_write(hdev_ctrl, p, a, b);
}


static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start);

BOOLEAN rtmp_ee_init_check(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	return cap->ee_inited;
}

#define IS_EE_INIT(_ad) (rtmp_ee_init_check(_ad))

/*******************************************************************************
  *
  *	Flash-based EEPROM read/write procedures.
  *		some chips use the flash memory instead of internal EEPROM to save the
  *		calibration info, we need these functions to do the read/write.
  *
  ******************************************************************************/
BOOLEAN rtmp_ee_flash_read(PRTMP_ADAPTER pAd, UINT32 Offset, UINT16 *pValue)
{
	BOOLEAN IsEmpty = 0;
	struct physical_device *ph_dev = pAd->physical_dev;

	if (!IS_EE_INIT(pAd))
		*pValue = 0xffff;
	else
		memcpy(pValue, ph_dev->EEPROMImage + Offset, 2);

	if ((*pValue == 0xffff) || (*pValue == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


int rtmp_ee_flash_write(PRTMP_ADAPTER pAd, UINT32 Offset, USHORT Data)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if (IS_EE_INIT(pAd)) {
		memcpy(ph_dev->EEPROMImage + Offset, &Data, 2);
		/*rt_nv_commit();*/
		/*rt_cfg_commit();*/
		RtmpFlashWrite(pAd->hdev_ctrl, ph_dev->EEPROMImage,
			get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));
	}

	return 0;
}


BOOLEAN rtmp_ee_flash_read_with_range(PRTMP_ADAPTER pAd, UINT32 start, UINT32 Length, UCHAR *pbuf)
{
	BOOLEAN IsEmpty = 0;
	UINT16  u2Loop;
	UCHAR   ucValue = 0;
	struct physical_device *ph_dev = pAd->physical_dev;

	if (!IS_EE_INIT(pAd))
		*pbuf = 0xff;
	else
		memcpy(pbuf, ph_dev->EEPROMImage + start, Length);

	for (u2Loop = 0; u2Loop < Length; u2Loop++)
		ucValue |= pbuf[u2Loop];

	if ((ucValue == 0xff) || (ucValue == 0x00))
		IsEmpty = 1;

	return IsEmpty;
}


int rtmp_ee_flash_write_with_range(PRTMP_ADAPTER pAd, UINT32 start, UINT32 Length, UCHAR *pbuf)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if (IS_EE_INIT(pAd)) {
		memcpy(ph_dev->EEPROMImage + start, pbuf, Length);
		/*rt_nv_commit();*/
		/*rt_cfg_commit();*/
		RtmpFlashWrite(pAd->hdev_ctrl, ph_dev->EEPROMImage,
			get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));
	}

	return 0;
}


VOID rtmp_ee_flash_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if (!IS_EE_INIT(pAd))
		return;

	memcpy(Data, ph_dev->EEPROMImage, get_dev_eeprom_size(pAd));
}


VOID rtmp_ee_flash_write_all(PRTMP_ADAPTER pAd)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if (!IS_EE_INIT(pAd))
		return;

	RtmpFlashWrite(pAd->hdev_ctrl, ph_dev->EEPROMImage,
		get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));
}

static NDIS_STATUS rtmp_ee_flash_reset(RTMP_ADAPTER *pAd, UCHAR *start)
{
	CHAR src[100] = {'\0'};
	RTMP_OS_FS_INFO osFsInfo;
	RTMP_OS_FD srcf;
	INT ret_val;
	EEPROM_CONTROL *pE2pCtrl = PD_GET_E2P_CTRL_PTR(pAd->physical_dev);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->get_bin_image_file != NULL) {
		ops->get_bin_image_file(pAd, src, FALSE);
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
			"bin FileName=%s\n",src);
	} else {
		strncat(src, EEPROM_DEFAULT_FILE_PATH, strlen(EEPROM_DEFAULT_FILE_PATH));
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO, "FileName=%s\n", src);
	}

	pE2pCtrl->e2pSource = E2P_SRC_FROM_BIN;
	RtmpOSFSInfoChange(&osFsInfo, TRUE);

	if (strlen(src)) {
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
					 "--> Error opening file %s\n", src);

			RtmpOSFSInfoChange(&osFsInfo, FALSE);
			if (cap->EEPROM_DEFAULT_BIN != NULL) {
				NdisMoveMemory(start, cap->EEPROM_DEFAULT_BIN,
							   ((cap->EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE) ? cap->EEPROM_DEFAULT_BIN_SIZE : MAX_EEPROM_BUFFER_SIZE));
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
						 "Load EEPROM Buffer from default BIN.\n");
				strncat(pE2pCtrl->BinSource, "Default BIN", strlen("Default BIN"));
				return NDIS_STATUS_SUCCESS;
			} else
				return NDIS_STATUS_FAILURE;
		} else {
			NdisZeroMemory(start,
					((get_dev_eeprom_size(pAd) > cap->EEPROM_DEFAULT_BIN_SIZE) ?
					   get_dev_eeprom_size(pAd) : cap->EEPROM_DEFAULT_BIN_SIZE));
			RtmpOSFileSeek(srcf, get_dev_eeprom_offset(pAd));
			ret_val = RtmpOSFileRead(srcf, start,
						 ((get_dev_eeprom_size(pAd) > cap->EEPROM_DEFAULT_BIN_SIZE) ?
						   get_dev_eeprom_size(pAd) : cap->EEPROM_DEFAULT_BIN_SIZE));

			if (ret_val > 0) {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
						 "Load EEPROM Buffer from %s\n", src);
			} else {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
						 "Read file \"%s\" failed(errCode=%d)!\n", src, ret_val);
			}

			ret_val = RtmpOSFileClose(srcf);
			if (ret_val)
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
					"Error %d closing %s\n", -ret_val, src);
		}
	}

	RtmpOSFSInfoChange(&osFsInfo, FALSE);
	return NDIS_STATUS_SUCCESS;
}

#ifdef LINUX
/* 0 -- Show ee buffer */
/* 1 -- force reset to default */
/* 2 -- Change ee settings */
int	Set_EECMD_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{
	USHORT i;
	struct physical_device *ph_dev = pAd->physical_dev;

	i = os_str_tol(arg, 0, 10);
	switch (i) {
	case 0: {
		USHORT value = 0, k;

		for (k = 0; k < EEPROM_SIZE; k += 2) {
			RT28xx_EEPROM_READ16(pAd, k, value);
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO, "%4.4x ", value);

			if (((k + 2) % 0x20) == 0)
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO, "\n");
		}
	}
	break;

	case 1:
		if (pAd->infType == RTMP_DEV_INF_RBUS) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
				"EEPROM reset to default......\n");
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
				"The last byte of MAC address will be re-generated...\n");

			if (rtmp_ee_flash_reset(pAd, ph_dev->EEPROMImage) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
					"rtmp_ee_flash_reset() failed\n");
				return FALSE;
			}

			/* Random number for the last bytes of MAC address*/
			{
				USHORT  Addr45;
				rtmp_ee_flash_read(pAd, 0x08, &Addr45);
				Addr45 = Addr45 & 0xff;
				Addr45 = Addr45 | (RandomByte(pAd) & 0xf8) << 8;
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO, "Addr45 = %4x\n", Addr45);
				rtmp_ee_flash_write(pAd, 0x08, Addr45);
			}
			rtmp_ee_flash_read(pAd, 0, &i);

			if ((i != 0x2880) && (i != 0x2860)) {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
					"invalid eeprom\n");
				return FALSE;
			}
		}

		break;

	case 2: {
		USHORT offset, value = 0;
		PUCHAR p;
		p = arg + 2;
		offset = os_str_tol(p, 0, 10);
		p += 2;

		while (*p != '\0') {
			if (*p >= '0' && *p <= '9')
				value = (value << 4) + (*p - 0x30);
			else if (*p >= 'a' && *p <= 'f')
				value = (value << 4) + (*p - 0x57);
			else if (*p >= 'A' && *p <= 'F')
				value = (value << 4) + (*p - 0x37);

			p++;
		}

		RT28xx_EEPROM_WRITE16(pAd, offset, value);
	}
	break;

	default:
		break;
	}

	return TRUE;
}
#endif /* LINUX */


static BOOLEAN  validFlashEepromID(RTMP_ADAPTER *pAd)
{
	USHORT eeFlashId;
	rtmp_ee_flash_read(pAd, 0, &eeFlashId);
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
		"eeFlashId=%x, pAd->ChipID=%x\n", eeFlashId, pAd->ChipID);

	if ((pAd->ChipID & 0x0000ffff) == eeFlashId)
		return TRUE;

	if (IS_MT7992(pAd)) {
		if ((eeFlashId == 0x7991))
			return TRUE;
	}

	return FALSE;
}


static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct physical_device *ph_dev = pAd->physical_dev;
	UINT32 addrH_offset[] = {EEPROM_BAND0_MAC0, EEPROM_BAND1_MAC0, EEPROM_BAND2_MAC0};
	UINT32 addrM_offset[] = {EEPROM_BAND0_MAC1, EEPROM_BAND1_MAC1, EEPROM_BAND2_MAC1};
	UINT32 addrT_offset[] = {EEPROM_BAND0_MAC2, EEPROM_BAND1_MAC2, EEPROM_BAND2_MAC2};
	UCHAR  zero_mac[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	UCHAR  band_H[] = {0x0, 0x8, 0xc};
	UINT8 bandIdx = hc_get_hw_band_idx(pAd);

	cap->ee_inited = 1;
	pAd->bRandomMac = 0;

	if (validFlashEepromID(pAd) == FALSE) {
		if (rtmp_ee_flash_reset(pAd, start) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
				"rtmp_ee_init(): failed\n");
			return NDIS_STATUS_FAILURE;
		}

		/* Random number for the last bytes of MAC address*/
		{
			USHORT  Addr[3];
			UCHAR   zeroMac[6] = {0};

			rtmp_ee_flash_read(pAd, addrH_offset[bandIdx], &(Addr[0]));
			rtmp_ee_flash_read(pAd, addrM_offset[bandIdx], &(Addr[1]));
			rtmp_ee_flash_read(pAd, addrT_offset[bandIdx], &(Addr[2]));

			if (MAC_ADDR_EQUAL(Addr, zero_mac) && !(IS_BELLWETHER(pAd))) {
				rtmp_ee_flash_read(pAd, addrH_offset[0], &(Addr[0]));
				rtmp_ee_flash_read(pAd, addrM_offset[0], &(Addr[1]));
				rtmp_ee_flash_read(pAd, addrT_offset[0], &(Addr[2]));
				Addr[0] = Addr[0] | band_H[bandIdx];
				*(UINT16 *)(&ph_dev->EEPROMImage[addrH_offset[bandIdx]]) = le2cpu16(Addr[0]);
				*(UINT16 *)(&ph_dev->EEPROMImage[addrM_offset[bandIdx]]) = le2cpu16(Addr[1]);
			}


			Addr[2] = Addr[2] & 0xff;
			Addr[2] = Addr[2] | (RandomByte(pAd) & 0xf8) << 8;
			/* do not random mac again to avoid mac address inconsistent at eagle */
			if (!NdisEqualMemory(zeroMac, pAd->CurrentAddress, MAC_ADDR_LEN))
				Addr[2] = *(UINT16 *)&pAd->CurrentAddress[4];
			*(UINT16 *)(&ph_dev->EEPROMImage[addrT_offset[bandIdx]]) = le2cpu16(Addr[2]);
			pAd->bRandomMac = 1;
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR,
				"The EEPROM in Flash is wrong, use default\n");
		}
		/*write back all to flash, legacy implementation*/
		rtmp_ee_flash_write_all(pAd);

		if (validFlashEepromID(pAd) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_ERROR, "invalid eeprom\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_nv_init(RTMP_ADAPTER *pAd)
{
	EEPROM_CONTROL *pE2pCtrl = PD_GET_E2P_CTRL_PTR(pAd->physical_dev);
	struct physical_device *ph_dev = pAd->physical_dev;

#if defined(MT7990) || defined(MT7992) || defined(MT7993)
		struct _RTMP_CHIP_CAP *cap = NULL;

		cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO, "\n");
	/*
		pAd->EEPROMImage:
		+----------------------------------------------------------------------------------+
		| 1K (calibration) | 9K (DCOC) | 9K (DPDA Part1) | 9K (DPDA Part2G) | 16K (PreCal) |
		+----------------------------------------------------------------------------------+
	 */
	if (ph_dev->EEPROMImage == NULL)
		os_alloc_mem(pAd, &ph_dev->EEPROMImage, get_dev_eeprom_size(pAd));

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
			"\x1b[42m: EEPROM Size[%d]\x1b[m\n", get_dev_eeprom_size(pAd));

	if (ph_dev->EEPROMImage) {
		NdisZeroMemory(ph_dev->EEPROMImage, get_dev_eeprom_size(pAd));
		RtmpFlashRead(pAd->hdev_ctrl, ph_dev->EEPROMImage,
			get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));

#ifdef PRE_CAL_MT7915_SUPPORT
		if (IS_MT7915(pAd)) {
			pAd->PreCalImageInfo = ph_dev->EEPROMImage + CAL_PRE_CAL_SIZE_OFFSET;
			pAd->PreCalImage     = ph_dev->EEPROMImage + CAL_FLASH_OFFSET;
			pAd->TxDPDImage      = ph_dev->EEPROMImage + DPD_FLASH_OFFSET;
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
					"\x1b[42m[EEPROMImage - PreCalImageInfo - PreCalImage - TxDPDImage]\x1b[m\n"
					"\x1b[42m[0x%p - 0x%p - 0x%p - 0x%p]\x1b[m\n",
					pAd->EEPROMImage, pAd->PreCalImageInfo, pAd->PreCalImage, pAd->TxDPDImage);
		}
#endif
#ifdef PRE_CAL_BELLWETHER_SUPPORT
		if (IS_BELLWETHER(pAd)) {
			pAd->PreCalImageInfo = ph_dev->EEPROMImage + CAL_PRE_CAL_SIZE_OFFSET;
			pAd->PreCalImage     = ph_dev->EEPROMImage + CAL_FLASH_OFFSET;
			pAd->TxDPDImage      = ph_dev->EEPROMImage + DPD_FLASH_OFFSET;
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
					"\x1b[42m[EEPROMImage - PreCalImageInfo - PreCalImage - TxDPDImage]\x1b[m\n"
					"\x1b[42m[0x%x - 0x%x - 0x%x - 0x%x]\x1b[m\n",
					ph_dev->EEPROMImage, pAd->PreCalImageInfo, pAd->PreCalImage, pAd->TxDPDImage);
		}
#endif
#ifdef PRE_CAL_MT7990_SUPPORT
		if (IS_MT7990(pAd)) {
			pAd->PreCalImageInfo = ph_dev->EEPROMImage + CAL_PRE_CAL_SIZE_OFFSET;
			pAd->PreCalImage     = ph_dev->EEPROMImage + CAL_FLASH_OFFSET;
			pAd->TxDPDImage      = ph_dev->EEPROMImage + cap->prek_ee_info.dpd_flash_offset;
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
					"\x1b[42m[EEPROMImage - PreCalImageInfo - PreCalImage - TxDPDImage]\x1b[m\n"
					"\x1b[42m[0x%p - 0x%p - 0x%p - 0x%p]\x1b[m\n",
					ph_dev->EEPROMImage, pAd->PreCalImageInfo, pAd->PreCalImage, pAd->TxDPDImage);
		}
#endif

#ifdef PRE_CAL_MT7992_SUPPORT
		if (IS_MT7992(pAd)) {
			pAd->PreCalImageInfo = ph_dev->EEPROMImage + CAL_PRE_CAL_SIZE_OFFSET;
			pAd->PreCalImage     = ph_dev->EEPROMImage + CAL_FLASH_OFFSET;
			pAd->TxDPDImage      = ph_dev->EEPROMImage + cap->prek_ee_info.dpd_flash_offset;
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
					"\x1b[42m[EEPROMImage - PreCalImageInfo - PreCalImage - TxDPDImage]\x1b[m\n"
					"\x1b[42m[0x%p - 0x%p - 0x%p - 0x%p]\x1b[m\n",
					ph_dev->EEPROMImage, pAd->PreCalImageInfo, pAd->PreCalImage, pAd->TxDPDImage);
		}
#endif

#ifdef PRE_CAL_MT7993_SUPPORT
		if (IS_MT7993(pAd)) {
			pAd->PreCalImageInfo = ph_dev->EEPROMImage + CAL_PRE_CAL_SIZE_OFFSET;
			pAd->PreCalImage     = ph_dev->EEPROMImage + CAL_FLASH_OFFSET;
			pAd->TxDPDImage      = ph_dev->EEPROMImage + cap->prek_ee_info.dpd_flash_offset;
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
					"\x1b[42m[EEPROMImage - PreCalImageInfo - PreCalImage - TxDPDImage]\x1b[m\n"
					"\x1b[42m[0x%p - 0x%p - 0x%p - 0x%p]\x1b[m\n",
					ph_dev->EEPROMImage, pAd->PreCalImageInfo, pAd->PreCalImage, pAd->TxDPDImage);
		}
#endif



#if defined(MT7990)
		if (IS_MT7990(pAd)) {
			pAd->TssiCal2G = ph_dev->EEPROMImage + TSSI_CAL_2G_EEPROM_OFST;
			pAd->TssiCal5G = ph_dev->EEPROMImage + TSSI_CAL_5G_EEPROM_OFST;
			pAd->TxDnlCal  = ph_dev->EEPROMImage + TX_DNL_CAL_EEPROM_OFST;
			pAd->RXGainCal = ph_dev->EEPROMImage + RX_GAIN_CAL_EEPROM_OFST;
		}
#endif

#if defined(MT7992)
		if (IS_MT7992(pAd)) {
			pAd->TssiCal2G = ph_dev->EEPROMImage + TSSI_CAL_2G_EEPROM_OFST;
			pAd->TssiCal5G = ph_dev->EEPROMImage + TSSI_CAL_5G_EEPROM_OFST;
			pAd->TxDnlCal  = ph_dev->EEPROMImage + TX_DNL_CAL_EEPROM_OFST;
			pAd->RXGainCal = ph_dev->EEPROMImage + RX_GAIN_CAL_EEPROM_OFST;
		}
#endif

#if defined(MT7993)
		if (IS_MT7993(pAd)) {
			pAd->TssiCal2G = ph_dev->EEPROMImage + TSSI_CAL_2G_EEPROM_OFST;
			pAd->TssiCal5G = ph_dev->EEPROMImage + TSSI_CAL_5G_EEPROM_OFST;
			pAd->TxDnlCal  = ph_dev->EEPROMImage + TX_DNL_CAL_EEPROM_OFST;
			pAd->RXGainCal = ph_dev->EEPROMImage + RX_GAIN_CAL_EEPROM_OFST;
		}
#endif

		pE2pCtrl->e2pCurMode = E2P_FLASH_MODE;
		pE2pCtrl->e2pSource = E2P_SRC_FROM_FLASH;
		return rtmp_ee_flash_init(pAd, ph_dev->EEPROMImage);
	} else {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_ERROR,
			"Allocate %d-byte-of EEPROMImage failed.\n",
			get_dev_eeprom_size(pAd));
		return NDIS_STATUS_FAILURE;
	}
}

#endif /* RTMP_FLASH_SUPPORT */

