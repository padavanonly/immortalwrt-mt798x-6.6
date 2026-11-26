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
*/
/*
 ***************************************************************************
 ***************************************************************************

    Module Name:
    rt_proc.c

    Abstract:
    Create and register proc file system for ralink device

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "rt_config.h"

#ifdef CFG_SUPPORT_CSI
#define CSI_PROC_ROOT			"wlan"
#define PROC_CSI_DATA_NAME		"csi_data"
#define PROC_CSI_DBG_NAME		"csi_debug"
#define CSI_DATA_DIR_PATH		"/proc/net/wlan"

static ssize_t procCSIDataRead(struct file *filp,
	char __user *buf, size_t count, loff_t *f_pos);

static ssize_t procCSIDataPrepare(
	UINT_8 *buf,
	struct CSI_INFO_T *prCSIInfo,
	struct CSI_DATA_T *prCSIData);

static int procCSIDataOpen(struct inode *n, struct file *f);
static int procCSIDataRelease(struct inode *n, struct file *f);

static int proc_csi_dbg_open(struct inode *inode, struct file *file);

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
static const struct file_operations csi_debug_ops = {
	.owner = THIS_MODULE,
	.open = proc_csi_dbg_open,
	.read	= seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations csidata_ops = {
	.owner = THIS_MODULE,
	.read = procCSIDataRead,
	.open = procCSIDataOpen,
	.release = procCSIDataRelease,
};
#else
static const struct proc_ops csi_debug_ops = {
	.proc_open = proc_csi_dbg_open,
	.proc_read	= seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static const struct proc_ops csidata_ops = {
	.proc_read = procCSIDataRead,
	.proc_open = procCSIDataOpen,
	.proc_release = procCSIDataRelease,
};
#endif
#endif


#ifdef CFG_SUPPORT_CSI
static struct proc_dir_entry *csi_proc_dir;
static INT8 csi_if_num;	/*csi proc interface counter*/

static int proc_csi_dbg_info_show(struct seq_file *seq, void *v)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)seq->private;
	UCHAR csi_mode;
	struct wifi_dev *pwdev;
	struct CSI_INFO_T *prCSIInfo = NULL;
	UCHAR band_idx = 0;
	PCSI_STA pCSISta = NULL;
	UCHAR csi_sta_num = 0;
	u8 idx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	prCSIInfo = &pAd->rCSIInfo;
	csi_mode = prCSIInfo->CSI_report_mode;

	if (!csi_mode) {
		seq_printf(seq, "report_mode: %s\n", "DISABLE");
		return 0;
	}

	seq_printf(seq, "report_mode: %s\n", (csi_mode == CSI_PROC)?"PROC" : "NETLINK");

	seq_printf(seq, "csi_fw_version: %03d\n", prCSIInfo->FWVer);

	seq_printf(seq, "csi_buffer_cnt: used(%d)-total(%d)\n", prCSIInfo->u4CSIBufferUsed, CSI_RING_SIZE);

	/*show sta list*/
	NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
	seq_puts(seq, "###dump csi sta mac list###\n");
	DlListForEach(pCSISta, &prCSIInfo->CSIStaList, CSI_STA, List) {
		seq_printf(seq, "STA%02d-band%d-%02x:%02x:%02x:%02x:%02x:%02x\n",
			csi_sta_num, pCSISta->band_idx, PRINT_MAC(pCSISta->Addr));
		csi_sta_num++;
	}
	NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);

	seq_puts(seq, "###CSI active mode station list###\n");
	for (idx = 1; idx <= MAX_CSI_ACTIVE_STA_NUM; idx++) {
		if (BIT(idx) & prCSIInfo->wcid_bitmap) {
			pEntry = entry_get(pAd, idx);
			seq_printf(seq, "STA#%d: %pM - wcid(%d)\n", idx, pEntry->Addr, pEntry->wcid);
		}
	}

	pwdev = prCSIInfo->csi_wdev;

	seq_printf(seq, "band%d_status: %s\n", band_idx, (pwdev)?"ON" : "OFF");

	if (pwdev) {

		band_idx = HcGetBandByWdev(pwdev);

		seq_printf(seq, "band%d_channel: %d\n", band_idx, pwdev->channel);
		seq_printf(seq, "band%d_MAC: "MACSTR"\n", band_idx, MAC2STR(pwdev->bssid));

		if (prCSIInfo->ucValue2[CSI_CONFIG_FRAME_TYPE])
			seq_printf(seq, "band%d_frame_type(Subtype+Type): %d\n", band_idx, prCSIInfo->ucValue2[CSI_CONFIG_FRAME_TYPE]);

		if (prCSIInfo->ucValue1[CSI_CONFIG_OUTPUT_FORMAT])
			seq_printf(seq, "band%d_tone_process: %s\n", band_idx,
			(prCSIInfo->ucValue1[CSI_CONFIG_OUTPUT_FORMAT] == CSI_OUTPUT_TONE_MASKED)?"tone_mask" : "tone_mask_reorder");
		else
			seq_printf(seq, "band%d_tone_process: %s\n", band_idx, "raw_tone");

		seq_printf(seq, "band%d_protocol: %03d\n", band_idx, prCSIInfo->protocol_filter);
	}

	return 0;
}


/*for debug proc*/
static int proc_csi_dbg_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_csi_dbg_info_show, pde_data(file_inode(file)));
}

static int procCSIDataOpen(struct inode *n, struct file *f)
{
	struct CSI_INFO_T *prCSIInfo = NULL;
	RTMP_ADAPTER *pAd = NULL;
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	pAd = (RTMP_ADAPTER *)(pde_data(file_inode(f)));
#else
	pAd = (RTMP_ADAPTER *)(PDE(file_inode(f))->data);
#endif

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"proc pointer get fail!!!\n");
		return -1;
	}

	prCSIInfo = &pAd->rCSIInfo;
	prCSIInfo->bIncomplete = FALSE;		/*no remain pkt bytes left*/

	return 0;
}

static int procCSIDataRelease(struct inode *n, struct file *f)
{
	struct CSI_INFO_T *prCSIInfo = NULL;
	RTMP_ADAPTER *pAd = NULL;
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	pAd = (RTMP_ADAPTER *)(pde_data(file_inode(f)));
#else
	pAd = (RTMP_ADAPTER *)(PDE(file_inode(f))->data);
#endif

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"proc pointer get fail!!!\n");
		return -1;
	}

	prCSIInfo = &pAd->rCSIInfo;
	prCSIInfo->bIncomplete = FALSE;		/*no remain pkt bytes left*/

	return 0;
}
/*
*this function prepare a pkt for APP to use.
 *the format of pkt depends on some specific rules as follows.
*/
static ssize_t procCSIDataPrepare(
	UINT_8 *buf,
	struct CSI_INFO_T *prCSIInfo,
	struct CSI_DATA_T *prCSIData)
{

	INT_32 i4Pos = 0;
	UINT_8 *tmpBuf = buf;
	UINT_16 u2DataSize = prCSIData->u2DataCount * sizeof(INT_16);
	UINT_16 u2Rsvd1Size = prCSIData->ucRsvd1Cnt * sizeof(INT_32);

	/* magic number */
	put_unaligned(0xAA, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(0xBB, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(0xCC, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(0xDD, (tmpBuf + i4Pos));
	i4Pos++;

	/*Just bypass total length feild here and update it in the end*/
	i4Pos += 2;

	put_unaligned(CSI_DATA_VER, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->FWVer, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TYPE, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucBw, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TS, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(4, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u4TimeStamp, (UINT_32 *) (tmpBuf + i4Pos));
	i4Pos += 4;

	put_unaligned(CSI_DATA_RSSI, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->cRssi, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_SNR, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucSNR, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_DBW, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucDataBw, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_CH_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucPrimaryChIdx, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TA, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(MAC_ADDR_LEN, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	os_move_mem((tmpBuf + i4Pos), prCSIData->aucTA, MAC_ADDR_LEN);
	i4Pos += MAC_ADDR_LEN;

	put_unaligned(CSI_DATA_EXTRA_INFO, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(4, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u4ExtraInfo, (UINT_32 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_32);

	put_unaligned(CSI_DATA_I, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(u2DataSize, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	os_move_mem((tmpBuf + i4Pos), prCSIData->ac2IData, u2DataSize);
	i4Pos += u2DataSize;

	put_unaligned(CSI_DATA_Q, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(u2DataSize, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	os_move_mem((tmpBuf + i4Pos), prCSIData->ac2QData, u2DataSize);
	i4Pos += u2DataSize;

	if (prCSIInfo->ucValue1[CSI_CONFIG_INFO] & CSI_INFO_RSVD1) {
		put_unaligned(CSI_DATA_RSVD1, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(u2Rsvd1Size, (UINT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		os_move_mem((tmpBuf + i4Pos), prCSIData->ai4Rsvd1, u2Rsvd1Size);
		i4Pos += u2Rsvd1Size;

		put_unaligned(CSI_DATA_RSVD2, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(u2Rsvd1Size, (UINT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		os_move_mem((tmpBuf + i4Pos), prCSIData->au4Rsvd2, u2Rsvd1Size);
		i4Pos += u2Rsvd1Size;

		put_unaligned(CSI_DATA_RSVD3, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(sizeof(INT_32), (INT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		put_unaligned(prCSIData->i4Rsvd3, (INT_32 *) (tmpBuf + i4Pos));
		i4Pos += sizeof(INT_32);
	}

	if (prCSIInfo->ucValue1[CSI_CONFIG_INFO] & CSI_INFO_RSVD2) {
		put_unaligned(CSI_DATA_RSVD4, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(sizeof(UINT_8), (INT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		put_unaligned(prCSIData->ucRsvd4, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos += sizeof(UINT_8);
	}

	put_unaligned(CSI_DATA_TX_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned((UINT_16)(GET_CSI_TX_IDX(prCSIData->Tx_Rx_Idx)), (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	put_unaligned(CSI_DATA_RX_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned((UINT_16)(GET_CSI_RX_IDX(prCSIData->Tx_Rx_Idx)), (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	put_unaligned(CSI_DATA_FRAME_MODE, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucRxMode, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	/* add antenna pattern*/
	put_unaligned(CSI_DATA_H_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_32), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->chain_info, (UINT_32 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_32);

	put_unaligned(CSI_DATA_RX_RATE, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->rx_rate, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	put_unaligned(CSI_DATA_PKT_SN, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->pkt_sn, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	put_unaligned(CSI_DATA_TR_STREAM, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_8), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->tr_stream, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_8);

	/*
	 * The lengths of magic number (4 byte) and total length (2 bytes)
	 * fields should not be counted in the total length value
	*/
	put_unaligned(i4Pos - 6, (UINT_16 *) (tmpBuf + 4));

	return i4Pos;
}

static ssize_t procCSIDataRead(struct file *filp,
	char __user *buf, size_t count, loff_t *f_pos)
{

	UINT_8 *temp = NULL;
	struct CSI_DATA_T *rTmpCSIData = NULL;
	UINT_32 u4CopySize = 0;
	UINT_32 u4StartIdx = 0;
	INT_32 i4Pos = 0;
	struct CSI_INFO_T *prCSIInfo = NULL;
	RTMP_ADAPTER *pAd = NULL;
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	pAd = (RTMP_ADAPTER *)(pde_data(file_inode(filp)));
#else
	pAd = (RTMP_ADAPTER *)(PDE(file_inode(filp))->data);
#endif

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"proc pointer get fail!!!\n");
		return -1;
	}

	prCSIInfo = &pAd->rCSIInfo;
	temp = prCSIInfo->byte_stream;

	if (!temp) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"temp NULL pointer!!!\n");
		return -1;
	}
	/* alloc mem for CSIData */
	os_alloc_mem_suspend(NULL, (UCHAR **)&rTmpCSIData, sizeof(struct CSI_DATA_T));

	if (!rTmpCSIData) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"rTmpCSIData fail to alloc mem!\n");
		return -1;
	}

	os_zero_mem(rTmpCSIData, sizeof(struct CSI_DATA_T));

	if (prCSIInfo->bIncomplete == FALSE) {

		wait_event_interruptible(prCSIInfo->waitq,
			prCSIInfo->u4CSIBufferUsed != 0);
		/*
		*No older CSI data in buffer waiting for reading out, so prepare a new one
		*for reading.
		*/
		os_zero_mem(temp, Max_Stream_Bytes);
		if (wlanPopCSIData(pAd, rTmpCSIData)) {
			i4Pos = procCSIDataPrepare(temp,
				prCSIInfo, rTmpCSIData);
		}

		/*reading the CSI data from 0 byte*/
		u4StartIdx = 0;
		if (i4Pos > count) {
			u4CopySize = count;
			prCSIInfo->u4RemainingDataSize = i4Pos - count;
			prCSIInfo->u4CopiedDataSize = count;
			prCSIInfo->bIncomplete = TRUE;
		} else {
			u4CopySize = i4Pos;
		}
	} else {
		/*if there is one pkt left some bytes to read */
		/* Reading the remaining CSI data in the buffer*/
		u4StartIdx = prCSIInfo->u4CopiedDataSize;
		if (prCSIInfo->u4RemainingDataSize > count) {
			u4CopySize = count;
			prCSIInfo->u4RemainingDataSize -= count;
			prCSIInfo->u4CopiedDataSize += count;
		} else {
			u4CopySize = prCSIInfo->u4RemainingDataSize;
			prCSIInfo->bIncomplete = FALSE;
		}
	}

	if (copy_to_user(buf, &temp[u4StartIdx], u4CopySize)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "copy to user failed\n");
		goto out;
	}

	*f_pos += u4CopySize;

out:
	if (rTmpCSIData) {
		if (rTmpCSIData->ac2IData)
			os_free_mem(rTmpCSIData->ac2IData);
		if (rTmpCSIData->ac2QData)
			os_free_mem(rTmpCSIData->ac2QData);
		os_free_mem(rTmpCSIData);
	}

	return (ssize_t)u4CopySize;

}

int csi_proc_init(RTMP_ADAPTER *pAd)
{
	char csi_proc_name[64] = {0};
	u8 band_idx;
	int ret;

	band_idx = hc_get_hw_band_idx(pAd);

	if (!csi_proc_dir) {
		csi_proc_dir = proc_mkdir(CSI_PROC_ROOT, init_net.proc_net); /* proc /net /wlan */

		if (!csi_proc_dir) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"csi_proc_dir get error!\n");
			return -1;
		}
	}

	/*for debug proc*/
	if (!pAd->rCSIInfo.debug_proc) {
		ret = snprintf(csi_proc_name, sizeof(csi_proc_name), "%s_%d", PROC_CSI_DBG_NAME, band_idx);

		if (os_snprintf_error(sizeof(csi_proc_name), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return -1;
		}

		/* proc /net /wlan /csi_debug_card0 */
		pAd->rCSIInfo.debug_proc = proc_create_data(csi_proc_name, 0664, csi_proc_dir, &csi_debug_ops, pAd);

		if (!pAd->rCSIInfo.debug_proc) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"prEntry is NULL! csi debug proc create fail.\n");
			return -1;
		}

		csi_if_num++;

		os_zero_mem(csi_proc_name, sizeof(csi_proc_name));
	}

	/*for data proc*/
	if (!pAd->rCSIInfo.data_proc) {
		ret = snprintf(csi_proc_name, sizeof(csi_proc_name), "%s_%d", PROC_CSI_DATA_NAME, band_idx);

		if (os_snprintf_error(sizeof(csi_proc_name), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return -1;
		}

		/* proc /net /wlan /csi_data_0 */
		pAd->rCSIInfo.data_proc = proc_create_data(csi_proc_name, 0664, csi_proc_dir, &csidata_ops, pAd);

		if (!pAd->rCSIInfo.data_proc) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"prEntry is NULL! csi data proc create fail.\n");
			return -1;
		}

		csi_if_num++;
	}

	return 0;
}

int csi_proc_deinit(RTMP_ADAPTER *pAd)
{
	char Entry_name[64] = {0};
	u8 band_idx;
	int ret;

	band_idx = hc_get_hw_band_idx(pAd);

	if (!csi_proc_dir) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "csi_proc_dir is NULL!\n");
		return -1;
	}

	/*delect data proc entry*/
	if (pAd->rCSIInfo.data_proc) {
		ret = snprintf(Entry_name, sizeof(Entry_name), "%s_%d", PROC_CSI_DATA_NAME, band_idx);

		if (os_snprintf_error(sizeof(Entry_name), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return -1;
		}

		remove_proc_entry(Entry_name, csi_proc_dir);
		pAd->rCSIInfo.data_proc = NULL;
		csi_if_num--;

		os_zero_mem(Entry_name, sizeof(Entry_name));
	}

	/*delect debug proc entry*/
	if (pAd->rCSIInfo.debug_proc) {

		ret = snprintf(Entry_name, sizeof(Entry_name), "%s_%d", PROC_CSI_DBG_NAME, band_idx);

		if (os_snprintf_error(sizeof(Entry_name), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return -1;
		}

		remove_proc_entry(Entry_name, csi_proc_dir);
		pAd->rCSIInfo.debug_proc = NULL;
		csi_if_num--;
	}

	if (csi_if_num == 0) {
		csi_proc_dir = NULL;
		remove_proc_entry(CSI_PROC_ROOT, init_net.proc_net);	/*delect dir: proc -net -wlan */
	} else if (csi_if_num < 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"csi_if_num is error!(%d)\n", (INT32)csi_if_num);

	return 0;
}
#endif
