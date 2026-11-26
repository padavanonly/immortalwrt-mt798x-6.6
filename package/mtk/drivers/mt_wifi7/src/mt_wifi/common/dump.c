/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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
 ** Id: common/dump.c
 */

/*! \file   "dump.c"
 *    \brief  Provide memory dump function for debugging.
 *
 *    Provide memory dump function for debugging.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
//#include "rtmp_comm.h"
#include "rt_config.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#ifndef IS_NOT_ALIGN_4
#define IS_NOT_ALIGN_4(_value)      (((_value) & 0x3) ? TRUE : FALSE)
#endif /* IS_NOT_ALIGN_4 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */


/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is called to dump a segment of memory in bytes.
 *
 * \param[in] str   Debug title string
 *
 * \param[in] pSrcBufVA   Pointer to the starting address of the memory
 *                           to be dumped.
 * \param[in] SrcBufLen       Length of the memory to be dumped.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/

void hex_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
#ifdef DBG
	hex_dump_with_lvl(str, pSrcBufVA, SrcBufLen, DBG_LVL_INFO);
#endif /* DBG */
}

void hex_dump_with_lvl(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen, INT dbglvl)
{
#ifdef DBG
	unsigned char *pt;
	int x;

	if ((DebugLevel < dbglvl) || (dbglvl < 0))
		return;

	pt = pSrcBufVA;
	MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, dbglvl,
			"%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);

	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, dbglvl, "0x%04x : ", x);

		MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, dbglvl,
			"%02x ", ((unsigned char)pt[x]));

		if (x % 16 == 15)
			MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, dbglvl, "\n");
	}

	MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, dbglvl, " ");
	MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, dbglvl, "\n");
#endif /* DBG */
}

void hex_dump_with_cat_and_lvl(
	char *str, UCHAR *pSrcBufVA, UINT SrcBufLen, INT dbgcat, INT dbg_sub_cat, INT dbglvl)
{
#ifdef DBG
	int x;

	if ((dbgcat < 0) || (dbglvl < 0)) {
		printk(KERN_ERR "%s: Invalid Parameters!\n", __func__);
		return;
	}

	if (!((dbg_sub_cat) & (DebugSubCategory[dbglvl][dbgcat])))
		return;

	MTWF_DBG_NP(dbgcat, dbg_sub_cat, dbglvl,
			 "%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);

	if (!pSrcBufVA)
		return;

	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			MTWF_DBG_NP(dbgcat, dbg_sub_cat, dbglvl,
					"0x%04x : ", x);

		MTWF_DBG_NP(dbgcat, dbg_sub_cat, dbglvl,
			"%02x ", ((unsigned char)pSrcBufVA[x]));

		if (x % 16 == 15)
			MTWF_DBG_NP(dbgcat, dbg_sub_cat, dbglvl, "\n");
	}

	MTWF_DBG_NP(dbgcat, dbg_sub_cat, dbglvl, "\n");
#endif /* DBG */
}

void hex_dump_always(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	MTWF_PRINT("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			MTWF_PRINT("0x%04x : ", x);
		MTWF_PRINT("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			MTWF_PRINT("\n");
	}
	MTWF_PRINT(" ");
	MTWF_PRINT("\n");
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is called to dump a segment of memory in double words.
 *
 * \param[in] pucStartAddr   Pointer to the starting address of the memory
 *                           to be dumped.
 * \param[in] u4Length       Length of the memory to be dumped.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void hex_dump32(char *str,	UINT32 *pu4StartAddr,	UINT32 u4Length)
{
	UINT8 *pucAddr;

	if (pu4StartAddr == NULL) {
		dump_stack();
		MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				 "Invalid input!!!\n");
		return;
	}

	MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			 "%s: %p, len = %d\n", str, pu4StartAddr, u4Length);

	if (IS_NOT_ALIGN_4((unsigned long)pu4StartAddr)) {
		UINT32 u4ProtrudeLen =
			sizeof(UINT32) - ((unsigned long)pu4StartAddr % 4);

		u4ProtrudeLen =
			((u4Length < u4ProtrudeLen) ? u4Length : u4ProtrudeLen);
		MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			 "pu4StartAddr is not at DW boundary.\n");

		pucAddr = (UINT8 *) &pu4StartAddr[0];
		switch (u4ProtrudeLen) {
		case 1:
			MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"(%p) %02x------\n", pu4StartAddr, pucAddr[0]);
			break;
		case 2:
			MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"(%p) %02x%02x----\n", pu4StartAddr, pucAddr[1], pucAddr[0]);
			break;
		case 3:
			MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"(%p) %02x%02x%02x--\n", pu4StartAddr, pucAddr[2], pucAddr[1], pucAddr[0]);
			break;
		default:
			break;
		}

		u4Length -= u4ProtrudeLen;
		pu4StartAddr = (UINT32 *)
			       ((unsigned long)pu4StartAddr + u4ProtrudeLen);
	}

	while (u4Length > 0) {
		if (u4Length >= 16) {
			MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"(%p) %08x %08x %08x %08x\n",
				pu4StartAddr,
				pu4StartAddr[0], pu4StartAddr[1],
				pu4StartAddr[2], pu4StartAddr[3]);
			pu4StartAddr += 4;
			u4Length -= 16;
		} else {
			switch (u4Length) {
			case 1:
				pucAddr = (UINT8 *) &pu4StartAddr[0];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) ------%02x\n", pu4StartAddr, pucAddr[0]);
				break;
			case 2:
				pucAddr = (UINT8 *) &pu4StartAddr[0];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) ----%02x%02x\n", pu4StartAddr, pucAddr[1], pucAddr[0]);
				break;
			case 3:
				pucAddr = (UINT8 *) &pu4StartAddr[0];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) --%02x%02x%02x\n",
					pu4StartAddr, pucAddr[2], pucAddr[1], pucAddr[0]);
				break;
			case 4:
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x\n", pu4StartAddr, pu4StartAddr[0]);
				break;
			case 5:
				pucAddr = (UINT8 *) &pu4StartAddr[1];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x ------%02x\n", pu4StartAddr, pu4StartAddr[0], pucAddr[0]);
				break;
			case 6:
				pucAddr = (UINT8 *) &pu4StartAddr[1];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x ----%02x%02x\n",
					pu4StartAddr, pu4StartAddr[0], pucAddr[1], pucAddr[0]);
				break;
			case 7:
				pucAddr = (UINT8 *) &pu4StartAddr[1];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x --%02x%02x%02x\n",
					pu4StartAddr, pu4StartAddr[0], pucAddr[2], pucAddr[1], pucAddr[0]);
				break;
			case 8:
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x\n", pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1]);
				break;
			case 9:
				pucAddr = (UINT8 *) &pu4StartAddr[2];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x ------%02x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1], pucAddr[0]);
				break;
			case 10:
				pucAddr = (UINT8 *) &pu4StartAddr[2];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x ----%02x%02x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1], pucAddr[1], pucAddr[0]);
				break;
			case 11:
				pucAddr = (UINT8 *) &pu4StartAddr[2];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x --%02x%02x%02x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1],
					pucAddr[2], pucAddr[1], pucAddr[0]);
				break;
			case 12:
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x %08x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1], pu4StartAddr[2]);
				break;
			case 13:
				pucAddr = (UINT8 *) &pu4StartAddr[3];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x %08x ------%02x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1], pu4StartAddr[2],
					pucAddr[0]);
				break;
			case 14:
				pucAddr = (UINT8 *) &pu4StartAddr[3];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x %08x ----%02x%02x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1], pu4StartAddr[2],
					pucAddr[1], pucAddr[0]);
				break;
			case 15:
			default:
				pucAddr = (UINT8 *) &pu4StartAddr[3];
				MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"(%p) %08x %08x %08x --%02x%02x%02x\n",
					pu4StartAddr, pu4StartAddr[0], pu4StartAddr[1], pu4StartAddr[2],
					pucAddr[2], pucAddr[1], pucAddr[0]);
				break;
			}
			u4Length = 0;
		}
	}
}				/* end of dumpMemory32() */

