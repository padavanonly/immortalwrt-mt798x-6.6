/*
 * Copyright (c) [2023], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	ee_i2cprom.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"


INT rtmp_ee_write_to_i2c_eeprom(RTMP_ADAPTER *pAd)
{
	UINT remain_block = EEPROM_SIZE;
	USHORT offset = 0;
	UCHAR *value;
	struct physical_device *ph_dev = pAd->physical_dev;

	while (remain_block >= I2C_EEPROM_BLOCK) {
		value = &(ph_dev->EEPROMImage[offset]);
		MtCmdEepromAccessWrite(pAd, offset, value, I2C_EEPROM_BLOCK);
		offset = offset + I2C_EEPROM_BLOCK;
		remain_block = remain_block - I2C_EEPROM_BLOCK;
	}
	if (remain_block > 0) {
		value = &(ph_dev->EEPROMImage[offset]);
		MtCmdEepromAccessWrite(pAd, offset, value, remain_block);
	}
	return TRUE;
}

static BOOLEAN  valid_i2c_eeprom_chip_id(RTMP_ADAPTER *pAd)
{
	UINT16 i2c_eeprom_chip_id = 0;

	rtmp_ee_i2c_eeprom_read16(pAd, 0, &i2c_eeprom_chip_id);
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
		"i2c_eeprom_chip_id=%x, pAd->ChipID=%x\n", i2c_eeprom_chip_id, pAd->ChipID);

	if ((pAd->ChipID & 0x0000ffff) == i2c_eeprom_chip_id)
		return TRUE;

	if (IS_MT7992(pAd)) {
		if ((i2c_eeprom_chip_id == 0x7991))
			return TRUE;
	}

	if ((pAd->ChipID & 0x0000ffff) != i2c_eeprom_chip_id) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_ERROR,
			"i2c eeprom is invalid chip id: %04x\n", i2c_eeprom_chip_id);
	}

	return FALSE;
}

VOID rtmp_ee_load_from_i2c_eeprom(RTMP_ADAPTER *pAd)
{
	UINT remain_block;
	USHORT offset = 0;
	UINT isVaild = 0;

	EEPROM_CONTROL *pE2pCtrl = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct physical_device *ph_dev = pAd->physical_dev;

	pE2pCtrl = PD_GET_E2P_CTRL_PTR(ph_dev);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_INFO,
			 "Load EEPROM buffer from i2c eeprom\n");

	if (!valid_i2c_eeprom_chip_id(pAd))
		return;

	NdisZeroMemory(ph_dev->EEPROMImage, cap->EEPROM_DEFAULT_BIN_SIZE);
	remain_block = cap->EEPROM_DEFAULT_BIN_SIZE;

	while (remain_block >= I2C_EEPROM_BLOCK) {
		MtCmdEepromAccessRead(pAd, offset, &ph_dev->EEPROMImage[offset],
			&isVaild, I2C_EEPROM_BLOCK);
		offset = offset + I2C_EEPROM_BLOCK;
		remain_block = remain_block - I2C_EEPROM_BLOCK;
	}
	if (remain_block > 0) {
		MtCmdEepromAccessRead(pAd, offset, &ph_dev->EEPROMImage[offset],
			&isVaild, remain_block);
	}

	pE2pCtrl->e2pCurMode = E2P_EEPROM_MODE;
	pE2pCtrl->e2pSource = E2P_SRC_FROM_EEPROM;
	/* Change to BIN eeprom buffer mode */
	RtmpChipOpsEepromHook(pAd, E2P_BIN_MODE);
}

INT rtmp_ee_i2c_eeprom_init(struct _RTMP_ADAPTER *pAd)
{
	rtmp_ee_load_from_i2c_eeprom(pAd);
	return 0;
}
BOOLEAN rtmp_ee_i2c_eeprom_read16(struct _RTMP_ADAPTER *pAd, UINT32 Offset, USHORT *pValue)
{
	UCHAR block[2] = "";
	UINT isVaild = 0;

	MtCmdEepromAccessRead(pAd, Offset, &block[0], &isVaild, 2);

	*pValue = (block[0] | (block[1] << 8));

	if (isVaild) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_EEPROM, DBG_LVL_DEBUG,
				 "Not found valid block for this offset (%x)\n",
				  Offset);
		return FALSE;
	} else
		return TRUE;

}
int rtmp_ee_i2c_eeprom_write16(struct _RTMP_ADAPTER *pAd, UINT32 Offset, USHORT data)
{
	UCHAR block[2] = "";

	block[0] = data & 0xff;
	block[1] = data >> 8 & 0xff;
	MtCmdEepromAccessWrite(pAd, Offset, &block[0], 2);

	return 0;
}

