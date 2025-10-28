/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: warp
	warp_proc.c
*/
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include "warp_utility.h"
#include "wdma.h"
#include "wed.h"
#include "warp.h"
#include "warp_hw.h"
#ifdef WED_PAO_SUPPORT
#include "regs/reg_v3/pao_hw.h"
#endif
#include <linux/stdarg.h>
#include "mcu/warp_woif.h"
#include "mcu/warp_wo.h"
#include "warp_meminfo_list.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

/*define proc*/
#define PROC_ROOT_DIR		"warp_ctrl"	/* global dir for warp */
#define PROC_TRACE_DIR		"tracer"	/* tracer */
#define PROC_WHNAT_DIR		"warp"		/* specific for each warp */
#define PROC_CFG_DIR		"cfg"		/* all of related H/W status */
#define PROC_CR_DIR		"cr"		/* get specific domain cr */
#define PROC_CTRL_DIR		"ctrl"		/* get/set control warp */
#define PROC_STAT_DIR		"stat"		/* all of status for debug */
#define PROC_TX_DIR		"tx"		/* tx top information */
#define PROC_RX_DIR		"rx"		/* rx top information */
#define PROC_WED_DIR		"wed"		/* wed specific information */
#define PROC_WDMA_DIR		"wdma"		/* wdma specic information */
#define PROC_RXBM_DIR		"rxbm"		/* rxbm specic information */
#define PROC_WO_DIR     	"wo"		/* wo specic information */
#define PROC_WOEE_DIR		"wo_aee"	/* wo exception information */
#define PROC_CONF_DIR		"conf"		/* WARP configuration */
#define PROC_PAO_DIR		"pao"		/* PAO information */
#define PROC_RTQM_DIR		"rtqm"		/* Route QM information */
#define PROC_RRO_DIR		"rro"		/* RRO related information */

/*define CR type */
enum {
	WHNAT_CR_START = 0,
	WHNAT_CR_WED = WHNAT_CR_START,
	WHNAT_CR_WDMA,
	WHNAT_CR_WIFI,
	WHNAT_CR_WOCPU,
	WHNAT_CR_END
};

struct proc_global {
	int rxbm_idx; //for rxbm
};

static struct proc_global proc_settings;

/**
 * strtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
char *__strtok;
char *strtok(char *s, const char *ct)
{
	char *sbegin, *send;

	sbegin = (s) ? s : __strtok;

	if (!sbegin)
		return NULL;

	sbegin += strspn(sbegin, ct);

	if (*sbegin == '\0') {
		__strtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, ct);

	if (send && *send != '\0')
		*send++ = '\0';

	__strtok = send;
	return sbegin;
}

/*
*
*/
static void
warp_hal_cr_handle(
	struct warp_entry *entry,
	char type,
	char is_write,
	unsigned long addr,
	unsigned int *cr_value)
{
	struct wed_entry *wed = &entry->wed;
	struct wdma_entry *wdma = &entry->wdma;
	struct wifi_entry *wifi = &entry->wifi;
#ifdef WED_RX_HW_RRO_2_0
	struct warp_fwdl_ctrl *fwdl_ctrl = &(entry->woif.fwdl_ctrl);
#endif

	warp_dbg(WARP_DBG_OFF, "%s(): Type=%d, IsWrite=%d, Addr=%lx\n",
			 __func__, type, is_write, addr);

	switch (type) {
	case WHNAT_CR_WED:
		if (is_write)
			warp_io_write32(wed, addr, *cr_value);
		else
			warp_io_read32(wed, addr, cr_value);

		break;

	case WHNAT_CR_WDMA:
		if (is_write)
			warp_io_write32(wdma, addr, *cr_value);
		else
			warp_io_read32(wdma, addr, cr_value);

		break;

	case WHNAT_CR_WIFI:
		if (is_write)
			warp_io_write32(wifi, addr, *cr_value);
		else
			warp_io_read32(wifi, addr, cr_value);

		break;
#ifdef WED_RX_HW_RRO_2_0
	case WHNAT_CR_WOCPU:
		if (is_write)
			fwdl_io_write32(fwdl_ctrl, addr, *cr_value);
		else
			fwdl_io_read32(fwdl_ctrl, addr, cr_value);
		break;
#endif
	default:
		break;
	}
}


/*
* WDMA
*/
static int
wdma_proc_show(struct seq_file *seq, void *v)
{
	struct wdma_entry *wdma = (struct wdma_entry *)seq->private;

	seq_printf(seq, "wdma: %p\n", wdma);
	seq_puts(seq, "WDMA_PROC_BASIC\t: echo 0 > wdma\n");
	seq_puts(seq, "WDMA_PROC_RX_CELL\t: echo 1 [ringid] [idx] > wdma\n");

	return 0;
}

/*
*
*/
static int
wdma_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wdma_proc_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
wdma_proc_write(struct file *file, const char __user *buff,
		size_t len1, loff_t *ppos)

{
	char value[64] = {0};
	char *end;
	char choice;
	struct wdma_entry *wdma = (struct wdma_entry *)pde_data(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		choice = warp_str_tol(value, &end, 10);
		warp_dbg(WARP_DBG_OFF, "%s(): Choice=%d\n", __func__, choice);
		wdma_proc_handle(wdma, choice, value);
	}

	return len1;
}


/*
 * WED proc operation
*/
/*
*
*/
static int
wed_proc_show(struct seq_file *seq, void *v)
{
	struct wed_entry *wed = (struct wed_entry *)seq->private;

	seq_printf(seq, "wed: %p\n", wed);
	seq_printf(seq, "WED_VERSION: %d.%d \n", wed->ver, wed->sub_ver);
	seq_printf(seq, "WED_BRANCH/ECO: %d/%d \n", wed->branch, wed->eco);
	seq_puts(seq, "WED_PROC_TX_RING_BASIC\t: echo 0 > wed\n");
	seq_puts(seq, "WED_PROC_TX_BUF_BASIC\t: echo 1 > wed\n");
	seq_puts(seq, "WED_PROC_TX_BUF_INFO\t: echo 2 [tkid] > wed\n");
	seq_puts(seq, "WED_PROC_TX_RING_CELL\t: echo 3 [ringid] [idx] > wed\n");
	seq_puts(seq, "WED_PROC_TX_RING_RAW\t: echo 4 [ringid] [idx] > wed\n");
	seq_puts(seq, "WED_PROC_DBG_INFO\t: echo 5 > wed\n");
	seq_puts(seq, "WED_PROC_TX_FREE_CNT\t: echo 8 > wed\n");
	seq_puts(seq, "WED_PROC_TX_RESET\t: echo 9 > wed\n");
	seq_puts(seq, "WED_PROC_RX_RESET\t: echo 10 > wed\n");
#if defined(CONFIG_WARP_HW_DDR_PROF)
	seq_puts(seq, "WED_PROC_TX_DDR_PROF\t: echo 11 > wed\n");
	seq_puts(seq, "WED_PROC_RX_DDR_PROF\t: echo 12 > wed\n");
	seq_puts(seq, "WED_PROC_OFF_DDR_PROF\t: echo 13 > wed\n");
#endif /* CONFIG_WARP_HW_DDR_PROF */
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	seq_puts(seq, "WED_PROC_TX_DYNAMIC_FREE\t: echo 6 > wed\n");
	seq_puts(seq, "WED_PROC_TX_DYNAMIC_ALLOC\t: echo 7 > wed\n");
	seq_puts(seq, "WED_PROC_TXBM_STAT\t: echo 14 > wed\n");
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	seq_puts(seq, "WED_PROC_RXBM_STAT\t: echo 15 > wed\n");
	seq_puts(seq, "WED_PROC_RX_DYNAMIC_RECYCLE\t: echo 16 > wed\n");
	seq_puts(seq, "WED_PROC_RX_DYNAMIC_ALLOC\t: echo 17 > wed\n");
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */
	seq_puts(seq, "WED_PROC_SER_ERR_CNT\t: echo 18 > wed\n");
#ifdef WED_PAO_SUPPORT
	seq_puts(seq, "WED_PROC_PAO_WCID_STAT\t: echo 19 [wcid] > wed\n");
	seq_puts(seq, "WED_PROC_PAO_SET_WCID_STAT\t: echo 20 [wcid] [amsdu_num] [amsdu_len] [vlan] [hdrt]> wed\n");
#endif
#ifdef WED_MEM_LEAK_DBG
	seq_puts(seq, "WED_PROC_MEM_INFO\t: echo 21 > wed\n");
#endif /*WED_MEM_LEAK_DBG*/
#ifdef WED_RX_HW_RRO_3_0
	seq_puts(seq, "WED_PROC_RRO_SE_GET_PN\t: echo 22 [se_id] > wed\n");
#endif
	return 0;
}

/*
*
*/
static void
warp_proc_handle(struct seq_file *seq, struct warp_entry *entry)
{
	seq_printf(seq, "Idx\t: %d\n", entry->idx);
	seq_printf(seq, "Cookie\t: %p\n", entry->wifi.hw.priv);
	seq_printf(seq, "WIFI base\t: %lx\n", entry->wifi.base_addr);
	seq_printf(seq, "Pdev\t: %p\n", entry->pdev);
	seq_printf(seq, "Pdriver\t: %p\n", &entry->pdriver);
	seq_printf(seq, "wdma\t: %p\n", &entry->wdma);
	seq_printf(seq, "wed\t: %p\n", &entry->wed);
	seq_printf(seq, "log_evel\t: %d\n", warp_log_get());
}

/*
*
*/
static int
wed_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wed_proc_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
wed_proc_write(struct file *file, const char __user *buff,
	       size_t len1, loff_t *ppos)
{
	char value[64] = {0};
	char *end;
	char choice;
	struct wed_entry *wed = (struct wed_entry *)pde_data(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		choice = warp_str_tol(value, &end, 10);
		warp_dbg(WARP_DBG_OFF, "%s(): Choice=%d\n", __func__, choice);
		wed_proc_handle(wed, choice, value);
	}

	return len1;
}

/*
* WHNAT
*/
/*
*
*/
static int
warp_proc_cr_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	/*usage*/
	/* Example:
	 1. read wo0 heartbit: echo 3 0 0x200 > /proc/warp_ctrl/warp0/cr
	 2. read wo1 heartbit: echo 3 0 0x204 > /proc/warp_ctrl/warp0/cr
	 */
	seq_printf(seq, "WED\t: PA base addr=%x\n", warp->wed.cr_base_addr);
	seq_printf(seq, "WED\t: base addr=%lx\n", warp->wed.base_addr);
	seq_printf(seq, "WDMA\t: base addr=%lx\n", warp->wdma.base_addr);
	seq_printf(seq, "WIFI\t: base addr=%lx\n", warp->wifi.base_addr);
#ifdef WED_RX_HW_RRO_2_0
	seq_printf(seq, "WOCPU\t: base addr=%lx\n", (unsigned long)warp->woif.fwdl_ctrl.boot_setting.base_addr);
#endif
	seq_puts(seq, "echo [0:WED|1:WDMA|2:WIFI|3:WOCPU] [0:READ|1:WRITE] [ADDR] {VALUE} > cr\n");

	return 0;
}

/*
*
*/
static int
warp_proc_cr_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_cr_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
warp_proc_cr_write(struct file *file, const char __user *buff,
		   size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char *token;
	char is_write;
	char type;
	char *s = value;
	unsigned long addr;
	u32 cr_value = 0;
	struct warp_entry *warp = (struct warp_entry *)pde_data(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, " ");
		type = warp_str_tol(token, &end, 10);
		token = strsep(&s, " ");
		is_write = warp_str_tol(token, &end, 10);
		token = strsep(&s, " ");
		addr = warp_str_tol(token, &end, 16);

		if (is_write) {
			token = strsep(&s, " ");
			cr_value = warp_str_tol(token, &end, 16);
		}

		warp_hal_cr_handle(warp, type, is_write, addr, &cr_value);
		warp_dbg(WARP_DBG_OFF, "%s():Type=%d,IsWrite=%d,Addr=0x%lx, Value=0x%x(%d)\n",
			 __func__, type, is_write, addr, cr_value, cr_value);
	}

	return len1;
}

/*
* CTRL operate
*/
enum {
	TYPE_HW_TX_EN = 0,
	TYPE_CR_MIRROR_EN = 1,
};

/*
*
*/
static void
warp_proc_ctrl_handle(
	struct warp_entry *entry,
	char type,
	char en)
{
	struct warp_cfg *cfg = &entry->cfg;

	switch (type) {
	case TYPE_HW_TX_EN:
		if (cfg->hw_tx_en != en) {
			cfg->hw_tx_en = en;
		}
		break;
	default:
		warp_dbg(WARP_DBG_OFF, "%s(): set wrong type: %d!\n", __func__, type);
		break;
	}
}

/*
*
*/
static int
warp_proc_ctrl_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;
	/*usage*/

	seq_printf(seq, "HW_TX_EN\t: %s\n", warp->cfg.hw_tx_en ? "TRUE" : "FALSE");
	seq_puts(seq, "echo [0:HW_TX_EN] [0:FALSE|1:TRUE] > ctrl\n");

	return 0;
}

/*
*
*/
static int
warp_proc_ctrl_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_ctrl_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
warp_proc_ctrl_write(struct file *file, const char __user *buff,
		     size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char *token;
	char is_true;
	char type;
	char *s = value;
	struct warp_entry *warp = (struct warp_entry *)pde_data(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, " ");
		type = warp_str_tol(token, &end, 10);
		token = strsep(&s, " ");
		is_true = warp_str_tol(token, &end, 10);

		warp_proc_ctrl_handle(warp, type, is_true);
		warp_dbg(WARP_DBG_OFF, "%s():Type=%d,IsTrue=%d\n",
			 __func__, type, is_true);
	}

	return len1;
}

/*
 * WARP trace tool operation
*/

#ifdef WARP_CPU_TRACER
/*
*
*/
static int
warp_proc_trace_show(struct seq_file *seq, void *v)
{
	struct warp_ctrl *ctrl = (struct warp_ctrl *)seq->private;
	struct warp_cputracer *tracer = &ctrl->bus.tracer;

	seq_puts(seq, "CFG:\n");
	seq_printf(seq, "Irq ID: %d\n", tracer->irq);
	seq_printf(seq, "BaseAddr: %lx\n", tracer->base_addr);
	seq_printf(seq, "Address: 0x%x\n", tracer->trace_addr);
	seq_printf(seq, "Mask: 0x%x\n", tracer->trace_mask);
	seq_puts(seq, "echo 0 [ 0 | 1 ] > trace, for enable or disable trace\n");
	seq_puts(seq, "echo 1 [ phy_addr] > trace, for set watch address\n");
	seq_puts(seq, "echo 2 [ mask ] > trace, for set watch mask\n");

	return 0;
}

/*
*
*/
static int
warp_proc_trace_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_trace_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
warp_proc_trace_write(struct file *file, const char __user *buff,
		      size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char choice;
	char *token;
	char *s = value;
	unsigned int tmp;
	struct warp_ctrl *ctrl = (struct warp_ctrl *)pde_data(file_inode(file));
	struct warp_cputracer *tracer = &ctrl->bus.tracer;

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, " ");
		choice = warp_str_tol(token, &end, 10);
		token = strsep(&s, " ");

		switch (choice) {
		case WHNAT_TRACE_EN:
			tmp = warp_str_tol(token, &end, 10);
			tracer->trace_en = tmp;
			break;

		case WHNAT_TRACE_ADDR:
			tmp = warp_str_tol(token, &end, 16);
			tracer->trace_addr = tmp;
			break;

		case WHNAT_TRACE_MSK:
			tmp = warp_str_tol(token, &end, 16);
			tracer->trace_mask = tmp;
			break;
		}

		warp_dbg(WARP_DBG_OFF, "Choice:%d\n", choice);
		warp_dbg(WARP_DBG_OFF, "EN:%d\n", tracer->trace_en);
		warp_dbg(WARP_DBG_OFF, "ADDR:%x\n", tracer->trace_addr);
		warp_dbg(WARP_DBG_OFF, "MASK:%x\n", tracer->trace_mask);
		warp_trace_set_hw(tracer);
	}

	return len1;
}
#endif

/*
*
*/
static int
warp_proc_conf_show(struct seq_file *seq, void *v)
{
	struct warp_ctrl *ctrl = (struct warp_ctrl *)seq->private;
	struct sw_conf_t *sw_conf;
	u8 idx = 0;

	for (idx = 0 ; idx < ctrl->warp_num ; idx++) {
		sw_conf = &ctrl->entry[idx].sw_conf;

		seq_printf(seq, "[ S/W settings for warp%d ]\n", idx);
		seq_puts(seq, "\t[ TXBM ]\n");
		if (sw_conf->txbm.vld_group)
			seq_printf(seq, "\tInitial total size: %d\n", sw_conf->txbm.vld_group);
		if (sw_conf->txbm.max_group)
			seq_printf(seq, "\tMaximum total size: %d\n", sw_conf->txbm.max_group);
		seq_printf(seq, "\tRX WDMA ring depth: %d\n", sw_conf->rx_wdma_ring_depth);
		seq_printf(seq, "\t\tDynamic TXBM enabled: %s\n",
			(sw_conf->txbm.enable) ? "True" : "False");
		if (sw_conf->txbm.enable) {
			seq_printf(seq, "\t\tMaximum group number: %d\n", sw_conf->txbm.max_group);
			seq_printf(seq, "\t\tExtend/Shrink quota: %d\n", sw_conf->txbm.alt_quota);
			seq_printf(seq, "\t\tBuffer low threshold: %d\n", sw_conf->txbm.buf_low);
			seq_printf(seq, "\t\tBuffer high threshold: %d\n", sw_conf->txbm.buf_high);
			seq_printf(seq, "\t\tBudge refill watermark: %d\n",
				sw_conf->txbm.budget_refill_watermark);
			seq_printf(seq, "\t\tBudge limit: %d (Dynamically applied)\n",
				sw_conf->txbm.budget_limit);
		}
		seq_printf(seq, "\t[ RXBM ]\n");
		if (sw_conf->rxbm.vld_group)
			seq_printf(seq, "\tInitial total size: %d\n", sw_conf->rxbm.vld_group);
		if (sw_conf->rxbm.max_group)
			seq_printf(seq, "\tMaximum total size: %d\n", sw_conf->rxbm.max_group);
		seq_printf(seq, "\tTX WDMA ring depth: %d\n", sw_conf->tx_wdma_ring_depth);
		seq_printf(seq, "\t\tDynamic RXBM enabled: %s\n",
			(sw_conf->rxbm.enable) ? "True" : "False");
		if (sw_conf->rxbm.enable) {
			seq_printf(seq, "\t\tMaximum group number: %d\n", sw_conf->rxbm.max_group);
			seq_printf(seq, "\t\tExtend/Shrink quota: %d\n", sw_conf->rxbm.alt_quota);
			seq_printf(seq, "\t\tBuffer low threshold: %d\n", sw_conf->rxbm.buf_low);
			seq_printf(seq, "\t\tBuffer high threshold: %d\n", sw_conf->rxbm.buf_high);
			seq_printf(seq, "\t\tBudge refill watermark: %d\n",
				sw_conf->rxbm.budget_refill_watermark);
			seq_printf(seq, "\t\tBudge limit: %d (Dynamically applied)\n",
				sw_conf->rxbm.budget_limit);
			seq_printf(seq, "\t\tRecycle postponed: %d (Dynamically applied)\n",
				sw_conf->rxbm.recycle_postponed);
		}
	}

	seq_puts(seq, "\nConfiguration format: $(warp_idx):$(bm):$(option)=$(value) in decimal\n");
	seq_puts(seq, "\tBM module: 0-TX BM ; 1-RX BM\n");
	seq_puts(seq, "\tOptions: 0-Dynamic TXBM on/off ;\n");
	seq_puts(seq, "\t\t1-WDMA RX ring depth ; 2-WDMA TX ring depth ;\n");
	seq_puts(seq, "\t\t3-extend/shrink quota ; 4-budget limitation ;\n");
	seq_puts(seq, "\t\t5-budget refill watermark ; 6-extend threshold ;\n");
	seq_puts(seq, "\t\t7-shrink threshold ; 8-initial group size ;\n");
	seq_puts(seq, "\t\t9-max available groups ; 10-recycle postponed\n");

	return 0;
}

/*
*
*/
static int
warp_proc_conf_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_conf_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
warp_proc_conf_write(struct file *file, const char __user *buff,
		      size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	u8 choice = 0, warp_idx = 0, bm = 0;
	int ret = 0;
	char *token;
	char *s = value;
	unsigned int tmp;
	struct warp_ctrl *ctrl = (struct warp_ctrl *)pde_data(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, ":");
		if (token)
			warp_idx = warp_str_tol(token, &end, 10);
		else
			goto err_out;
		token = strsep(&s, ":");
		if (token)
			bm = warp_str_tol(token, &end, 10);
		else
			goto err_out;
		token = strsep(&s, "=");
		if (token)
			choice = warp_str_tol(token, &end, 10);
		else
			goto err_out;

		if (warp_idx < ctrl->warp_num) {
			struct sw_conf_t *sw_conf = &ctrl->entry[warp_idx].sw_conf;

			if (choice < WARP_SW_CONF_MAX) {
				struct dybm_conf_t *dybm_conf = NULL;
				u8 bm_str[3] = "TX";

				tmp = warp_str_tol(s, &end, 10);

				if (bm == 0)
					dybm_conf = &sw_conf->txbm;
				else {
					dybm_conf = &sw_conf->rxbm;
					ret = snprintf(bm_str, 3, "%s", "RX");
					if (ret < 0)
						warp_dbg(WARP_DBG_ERR, "%s is wrong string.\n",
							bm_str);
				}

				switch (choice) {
				case WARP_DYBM_EN:
					warp_dbg(WARP_DBG_OFF, "%sBM alter size dynamically change from %d to %d\n",
							 bm_str, dybm_conf->enable, tmp);
					dybm_conf->enable = tmp;
					if (dybm_conf->max_group == 0) {
						warp_dbg(WARP_DBG_OFF, "%sBM total max groups change from %d to %d\n",
								 bm_str, dybm_conf->max_group, MAX_GROUP_SIZE);
						dybm_conf->max_group = MAX_GROUP_SIZE;
					}
					break;
				case WARP_RX_WDMA_RING_DEP:
					if (tmp > 0xffff) {
						warp_dbg(WARP_DBG_ERR, "Exceed maximum, correct to %d\n", 0xffff);
						tmp = 0xffff;
					}
					warp_dbg(WARP_DBG_OFF, "WDMA RX ring depth change from %d to %d\n",
							 sw_conf->rx_wdma_ring_depth, tmp);
					sw_conf->rx_wdma_ring_depth = tmp;
					break;
				case WARP_TX_WDMA_RING_DEP:
					if (tmp > 0xffff) {
						warp_dbg(WARP_DBG_ERR, "Exceed maximum, correct to %d\n", 0xffff);
						tmp = 0xffff;
					}
					warp_dbg(WARP_DBG_OFF, "WDMA TX ring depth change from %d to %d\n",
							 sw_conf->tx_wdma_ring_depth, tmp);
					sw_conf->tx_wdma_ring_depth = tmp;
					break;
				case WARP_DYBM_BUDGE:
					warp_dbg(WARP_DBG_OFF, "%sBM extend/shrink quota change from %d to %d\n",
							 bm_str, dybm_conf->alt_quota, tmp);
					dybm_conf->alt_quota = tmp;
					break;
				case WARP_DYBM_BUDGE_REFILL_MARK:
					if (tmp >= dybm_conf->budget_limit) {
						warp_dbg(WARP_DBG_OFF, "%sBM budge refill watermark should not exceed budget limit! Force as %d\n",
							 bm_str, dybm_conf->budget_limit/2);
						tmp = dybm_conf->budget_limit/2;
					}
					warp_dbg(WARP_DBG_OFF, "%sBM budge refill watermark change from %d to %d\n",
							 bm_str, dybm_conf->budget_refill_watermark, tmp);
					dybm_conf->budget_refill_watermark = tmp;
					break;
				case WARP_DYBM_BUDGE_LMT:
					warp_dbg(WARP_DBG_OFF, "%sBM budge change from %d to %d\n",
							 bm_str, dybm_conf->budget_limit, tmp);
					dybm_conf->budget_limit = tmp;
					break;
				case WARP_DYBM_L_THRD:
					warp_dbg(WARP_DBG_OFF, "%sBM buffer low shreshold change from %d to %d\n",
							 bm_str, dybm_conf->buf_low, tmp);
					dybm_conf->buf_low = tmp;
					break;
				case WARP_DYBM_H_THRD:
					warp_dbg(WARP_DBG_OFF, "%sBM buffer high shreshold change from %d to %d\n",
							 bm_str, dybm_conf->buf_high, tmp);
					dybm_conf->buf_high = tmp;
					break;
				case WARP_DYBM_VLD_GRP:
					warp_dbg(WARP_DBG_OFF, "%sBM total valid groups change from %d to %d\n",
							 bm_str, dybm_conf->vld_group, tmp);
					dybm_conf->vld_group = tmp;
					break;
				case WARP_DYBM_MAX_GRP:
					warp_dbg(WARP_DBG_OFF, "%sBM total max groups change from %d to %d\n",
							 bm_str, dybm_conf->max_group, tmp);
					dybm_conf->max_group = tmp;
					break;
				case WARP_DYBM_RECYCLE_POSTPONED:
					warp_dbg(WARP_DBG_OFF, "%sBM recyle event postponed from %d to %d\n",
							 bm_str, dybm_conf->recycle_postponed, tmp);
					dybm_conf->recycle_postponed = tmp;
					break;
				default:
					warp_dbg(WARP_DBG_OFF, "%sBM unknown option %d\n",
							 bm_str, choice);
				}
			} else
				warp_dbg(WARP_DBG_OFF, "Not supported item(%d)\n", choice);
		} else {
			warp_dbg(WARP_DBG_ERR, "invlaid warp index:%d\n", warp_idx);
		}
	}

err_out:
	return len1;
}


/*
*
*/
static int
warp_proc_state_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	warp_proc_handle(seq, warp);

	seq_puts(seq, "echo [LOG_LEVEL] > stat\n");
	seq_puts(seq, "LOG_LEVEL:\n");
	seq_puts(seq, "(0):WARP_DBG_OFF\n");
	seq_puts(seq, "(1):WARP_DBG_ERR\n");
	seq_puts(seq, "(2):WARP_DBG_WAR\n");
	seq_puts(seq, "(3):WARP_DBG_INF\n");
	seq_puts(seq, "(4):WARP_DBG_LOU\n");
	seq_puts(seq, "(5):WARP_DBG_ALL\n");
	return 0;
}

/*
*
*/
static int
warp_proc_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_state_show, pde_data(file_inode(file)));
}

/*
*
*/
static ssize_t
warp_proc_state_write(struct file *file, const char __user *buff,
		      size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char log;
	struct warp_entry *warp = (struct warp_entry *)pde_data(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		log = warp_str_tol(value, &end, 10);

		switch (log) {
		case 10:
			warp_ser_trigger_hw(&warp->wifi);
			return len1;
		case 11:
			wed_ser_dump(&warp->wed);
			return len1;
		}
		warp_dbg(WARP_DBG_OFF, "%s(): LogLevel=%d\n", __func__, log);
		warp_log_set(log);
	}

	return len1;
}

/*
*
*/
static int
warp_proc_cfg_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	warp_procinfo_dump_cfg_hw(warp, seq);
	return 0;
}

/*
*
*/
static int
warp_proc_cfg_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_cfg_show, pde_data(file_inode(file)));
}

/*
*
*/
static int
warp_proc_tx_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	warp_procinfo_dump_txinfo_hw(warp, seq);
	return 0;
}

/*
*
*/
static int
warp_proc_tx_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_tx_show, pde_data(file_inode(file)));
}

/*
*
*/
static int
warp_proc_rx_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	warp_procinfo_dump_rxinfo_hw(warp, seq);
	return 0;
}

/*
*
*/
static int
warp_proc_rx_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_rx_show, pde_data(file_inode(file)));
}


#ifdef WED_RX_D_SUPPORT
static ssize_t
rxbm_proc_write(struct file *file, const char __user *buff,
		     size_t len1, loff_t *ppos)
{
	char buffer[64] = {0};
	int num, idx1;

	if (buff && !copy_from_user(buffer, buff, len1)) {
		num = sscanf(buffer, "%d", &idx1);
		if (num < 0)
			return -1;
		warp_dbg(WARP_DBG_OFF, "rxbm_proc_write= %d\n", num);
		proc_settings.rxbm_idx = idx1;
	}
	return len1;
}

/*
*
*/
static int
warp_proc_rxbm_show(struct seq_file *seq, void *v)
{
	struct wed_entry *wed = (struct wed_entry *)seq->private;

	wed_procinfo_dump(wed, seq, proc_settings.rxbm_idx);
	return 0;
}

/*
*
*/
static int
rxbm_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_rxbm_show, pde_data(file_inode(file)));
}

#ifdef WED_RX_HW_RRO_2_0

/*
*
*/
static int
wo_ee_proc_show(struct seq_file *seq, void *v)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;
	struct wo_exception_ctrl *wo_dump = &warp->woif.wo_exp_ctrl;
	int size = 0;

	if (wo_dump->log == NULL) {
		seq_puts(seq, "Error: no wo exception log\n");
		return 0;
	}

	size = strlen(wo_dump->log);
	if (size > 0) {
		seq_printf(seq, "=== wo log dump size= %d ====\n", size);
		seq_printf(seq, "%s", (u8*) wo_dump->log);
		seq_puts(seq, "\n=== log end===\n");
	}
	return 0;
}

/*
*
*/
static int
wo_ee_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wo_ee_proc_show, pde_data(file_inode(file)));
}


/*
*
*/
static int
wo_proc_show(struct seq_file *seq, void *v)
{
#ifdef WED_RX_HW_RRO_2_0
	u8 fw_info_str[50] = {0}, i = 0;
	u32 heartbeat = 0;
	struct warp_entry *warp = (struct warp_entry *)seq->private;
	struct woif_entry *woif = &warp->woif;

	warp_fwdl_get_wo_heartbeat(&woif->fwdl_ctrl, &heartbeat, warp->idx);

	seq_puts(seq, "########## WO Firmware ##########\n");
	seq_printf(seq, "Chip ID: 0x%04x\n", woif->fwdl_ctrl.fw_info.chip_id);
	seq_printf(seq, "ECO version:%d\n", woif->fwdl_ctrl.fw_info.eco_ver);
	fw_info_str[0] = '\0';
	for (i = 0 ; i < sizeof(woif->fwdl_ctrl.fw_info.ram_ver) ; i++)
		strncat(fw_info_str, woif->fwdl_ctrl.fw_info.ram_ver+i, sizeof(u8));
	seq_printf(seq, "Version: %s\n", fw_info_str);
	fw_info_str[0] = '\0';
	for (i = 0 ; i < sizeof(woif->fwdl_ctrl.fw_info.ram_built_date) ; i++)
		strncat(fw_info_str, woif->fwdl_ctrl.fw_info.ram_built_date+i, sizeof(u8));
	seq_printf(seq, "Build date: %s\n", fw_info_str);
	seq_puts(seq, "#################################\n");
	seq_printf(seq, "WO MCU status: %s(%4x)\n\n",
				(warp->wed.ser_ctrl.wo_no_response) ? "No response" : "Alive",
				heartbeat);
#endif	/* WED_RX_HW_RRO_2_0 */
	seq_puts(seq, "echo devinfo [IDX] > wo\n");
	seq_puts(seq, "echo bssinfo [IDX] > wo\n");
	seq_puts(seq, "echo starec [IDX] > wo\n");
	seq_puts(seq, "echo starec_ba [IDX] > wo\n");
	seq_puts(seq, "echo bainfo > wo\n");
	seq_puts(seq, "echo bactrl > wo\n");
	seq_puts(seq, "echo fbcmdq > wo\n");
	seq_puts(seq, "echo logflush > wo\n");
	seq_puts(seq, "echo logctrl [idx] > wo, idx=0~3: OFF, UART, HOST, BOTH\n");
	seq_puts(seq, "echo cpustat.en [0|1] > wo, 0: OFF, 1: on\n");
	seq_puts(seq, "echo cpustat.dump > wo, dump cpu stat\n");
	seq_puts(seq, "echo pclrdump > wo, dump wo and lr\n");
	seq_puts(seq, "echo state > wo, dump RX statistics\n");
	seq_puts(seq, "echo rxcnt_ctrl [0|1|2] [wo timer factor] > wo\n"
			"\t\t0: disable\n"
			"\t\t1: enable rxcnt report update from wo, 0 <= wo timer factor <= 20, unit is 150ms\n"
			"\t\t2: enable rxcnt report query from host\n");
	seq_puts(seq, "echo rxcnt_info [TID] [WCID] > wo\n");
	seq_puts(seq, "echo ccif_ring > wo\n");

	return 0;
}


/*
*
*/
static int
wo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wo_proc_show, pde_data(file_inode(file)));
}

static ssize_t
wo_proc_write(struct file *file, const char __user *usr_buff, size_t len1, loff_t *ppos)
{
	u8 *buffer = NULL, *type = NULL, *sub_str = NULL, *input[11] = {0};
	struct warp_entry *warp = (struct warp_entry *)pde_data(file_inode(file));
	int scan_num = 0, input_idx = 0, input_total = 0;
	struct warp_msg_cmd cmd = {0};
	u8 sendmsg_buf[128] = {0};
	struct wo_cmd_query *query = (struct wo_cmd_query *)sendmsg_buf;
	int ret = -1;

	warp_os_alloc_mem((unsigned char **)&buffer, len1+1, GFP_ATOMIC);

	if (buffer && usr_buff) {
		memset(buffer, 0, len1+1);
		if (!copy_from_user(buffer, usr_buff, len1)) {
			buffer[len1-1] = '\0';		/* forcedly replace "new line" character as termenation character */
			warp_dbg(WARP_DBG_INF, "(%s) buffer = %s\n", __func__, buffer);

			for (sub_str = strtok(buffer, " "); sub_str && input_idx < 11; sub_str = strtok(NULL, " "), input_idx++, input_total++) {
				input[input_idx] = sub_str;
				warp_dbg(WARP_DBG_INF, "(%s) arg[%d] = %s(%d)\n", __func__, input_idx, input[input_idx], (u32)strlen(input[input_idx]));
			}

			warp_dbg(WARP_DBG_INF, "(%s) argc=%d\n", __func__, input_total);
			type = input[0];

			if (input_total == 1 && type) {
				cmd.msg_len = 0;
				if (strncmp(type, "bainfo", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_BA_INFO_DUMP;
				} else if (strncmp(type, "bactrl", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_BA_CTRL_DUMP;
				} else if (strncmp(type, "fbcmdq", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_FBCMD_Q_DUMP;
				} else if (strncmp(type, "logflush", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_LOG_FLUSH;
				} else if (strncmp(type, "cpustat.dump", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_CPU_STATS_DUMP;
				} else if (strncmp(type, "state", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_WED_RX_STAT;
				} else if (strncmp(type, "ccif_ring", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_CCIF_RING_DUMP;
				} else if (strncmp(type, "prof_hit_dump", strlen(type)) == 0) {
					wo_profiling_report();
					ret = 0;
					goto error;
				} else if (strncmp(type, "rxcnt_info", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_RXCNT_INFO;
				} else if (strncmp(type, "pclrdump", strlen(type)) == 0) {
					warp_wo_pc_lr_cr_dump(warp->idx);
					ret = 0;
					goto error;
				} else {
					warp_dbg(WARP_DBG_OFF, "(%s) unknown comand string(%s)!\n", __func__, type);
					goto error;
				}
			} else if (input_total > 1) {
				cmd.msg_len = sizeof(struct wo_cmd_query);

				for (input_idx = 1 ; input_idx < input_total ; input_idx++) {
					scan_num = sscanf(input[input_idx], "%d", &query->query_arg0+(input_idx-1));

					if (scan_num < 1) {
						warp_dbg(WARP_DBG_OFF, "(%s) require more input!\n", __func__);
						goto error;
					}
				}

				if (strncmp(type, "devinfo", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_DEV_INFO_DUMP;
				} else if (strncmp(type, "bssinfo", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_BSS_INFO_DUMP;
				} else if (strncmp(type, "starec", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_STA_REC_DUMP;
				} else if (strncmp(type, "starec_ba", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_STA_BA_DUMP;
				} else if (strncmp(type, "logctrl", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_FW_LOG_CTRL;
				} else if (strncmp(type, "cpustat.en", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_CPU_STATS_ENABLE;
				} else if (strncmp(type, "prof_conf", strlen(type)) == 0) {
					if (strstr(input[1], ",")) {
						//TBD
						warp_dbg(WARP_DBG_OFF, "parameter = %s\n", input[1]);
						goto error;
					} else {
						cmd.param.cmd_id = WO_CMD_PROF_CTRL;
					}
				} else if (strncmp(type, "rxcnt_ctrl", strlen(type)) == 0) {
					cmd.param.cmd_id = WO_CMD_RXCNT_CTRL;
				} else {
					warp_dbg(WARP_DBG_OFF, "(%s) unknown comand string(%s)!\n", __func__, type);
					goto error;
				}
			} else {
				warp_dbg(WARP_DBG_OFF, "(%s) malformed input!\n", __func__);
				goto error;
			}
			cmd.msg = sendmsg_buf;
			cmd.param.to_id = MODULE_ID_WO;
			cmd.param.wait_type = WARP_MSG_WAIT_TYPE_NONE;
			cmd.param.timeout = 3000;
			cmd.param.rsp_hdlr = NULL;
			/* Fix me, user_data is a pointer
			cmd.param.user_data = 3; */

			if (cmd.param.cmd_id == WO_CMD_RXCNT_INFO) {
				cmd.param.wait_type = WARP_MSG_WAIT_TYPE_RSP_STATUS;
				cmd.param.rsp_hdlr = wo_rxcnt_update_handle;
				cmd.param.user_data = warp;
			}

			ret = warp_msg_send_cmd(warp->idx , &cmd);
			warp_dbg(WARP_DBG_OFF, "(%s) done\n", __func__);
		} else {
			warp_dbg(WARP_DBG_OFF, "(%s) copy_from_user failed!\n", __func__);
		}
	} else {
		warp_dbg(WARP_DBG_OFF, "(%s) buffer allocated failed!\n", __func__);
		goto error;
	}

	if (ret != 0)
		warp_dbg(WARP_DBG_OFF, "(%s) cmd failed!\n", __func__);

error:
	if (buffer) {
		warp_os_free_mem(buffer);
		buffer = NULL;
	}

	return len1;
}
#endif
#endif /*WED_RX_HW_RRO_2_0*/

/*
 * global file operation
*/
#ifdef WARP_CPU_TRACER
#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_trace_fops = {
	.proc_open = warp_proc_trace_open,
	.proc_write = warp_proc_trace_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_trace_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_trace_open,
	.write  = warp_proc_trace_write,
	.read	= seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif
#endif /* WARP_CPU_TRACER */

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_conf_fops = {
	.proc_open = warp_proc_conf_open,
	.proc_write = warp_proc_conf_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_conf_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_conf_open,
	.write  = warp_proc_conf_write,
	.read	= seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_cr_fops = {
	.proc_open = warp_proc_cr_open,
	.proc_write = warp_proc_cr_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_cr_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_cr_open,
	.write  = warp_proc_cr_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_stat_fops = {
	.proc_open = warp_proc_state_open,
	.proc_write = warp_proc_state_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_stat_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_state_open,
	.write  = warp_proc_state_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_cfg_fops = {
	.proc_open = warp_proc_cfg_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_cfg_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_cfg_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_tx_fops = {
	.proc_open = warp_proc_tx_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_tx_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_tx_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rx_fops = {
	.proc_open = warp_proc_rx_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_rx_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_rx_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_wed_fops = {
	.proc_open = wed_proc_open,
	.proc_write = wed_proc_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_wed_fops = {
	.owner = THIS_MODULE,
	.open = wed_proc_open,
	.write  = wed_proc_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_wdma_fops = {
	.proc_open = wdma_proc_open,
	.proc_write = wdma_proc_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_wdma_fops = {
	.owner = THIS_MODULE,
	.open = wdma_proc_open,
	.write  = wdma_proc_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_ctrl_fops = {
	.proc_open = warp_proc_ctrl_open,
	.proc_write = warp_proc_ctrl_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_ctrl_fops = {
	.owner = THIS_MODULE,
	.open = warp_proc_ctrl_open,
	.write  = warp_proc_ctrl_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef WED_RX_D_SUPPORT

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_rxbm_fops = {
	.proc_open = rxbm_proc_open,
	.proc_write = rxbm_proc_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_rxbm_fops = {
	.owner = THIS_MODULE,
	.open = rxbm_proc_open,
	.write  = rxbm_proc_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef WED_RX_HW_RRO_2_0
#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_wo_fops = {
	.proc_open = wo_proc_open,
	.proc_write = wo_proc_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_wo_fops = {
	.owner = THIS_MODULE,
	.open = wo_proc_open,
	.write  = wo_proc_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_woee_fops = {
	.proc_open = wo_ee_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_woee_fops = {
	.owner = THIS_MODULE,
	.open = wo_ee_proc_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif
#endif

/*
 *  proc register/unregister
*/

#ifdef WED_RX_HW_RRO_2_0

/*
*
*/
int
wo_exep_proc_init(struct warp_entry *warp)
{
	struct proc_dir_entry *proc;
	struct proc_dir_entry *parent_proc =(struct proc_dir_entry *) warp->proc;

	proc = proc_create_data(PROC_WOEE_DIR, 0, parent_proc, &proc_woee_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_OFF, "create %s failed!!!\n", PROC_WOEE_DIR);
		goto err0;
	}
	warp->proc_wo_ee_dump = (void *) proc;
	warp_dbg(WARP_DBG_INF, "wo_exep_proc_init done: %pK\n", parent_proc);

	return 0;
err0:
	remove_proc_entry(PROC_WOEE_DIR, parent_proc);
	return -1;
}

/*
*
*/
void
wo_exep_proc_exit(struct warp_entry *warp)
{
	struct proc_dir_entry *proc = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc_wo_ee_dump = (struct proc_dir_entry *) warp->proc_wo_ee_dump;

	if (proc && proc_wo_ee_dump) {
		remove_proc_entry(PROC_WOEE_DIR, proc);
		warp->proc_wo_ee_dump = NULL;
	}
}



/*
*
*/
int
wo_proc_init(struct warp_entry *warp)
{
	struct proc_dir_entry *proc;
	struct proc_dir_entry *parent_proc =(struct proc_dir_entry *) warp->proc;

	proc = proc_create_data(PROC_WO_DIR, 0, parent_proc, &proc_wo_fops,warp);

	if (!proc) {
		warp_dbg(WARP_DBG_OFF, "create %s failed!!!\n", PROC_WO_DIR);
		goto err0;
	}
	warp->proc_wo = (void *) proc;
	warp_dbg(WARP_DBG_INF, "wo_proc_init done  %pK\n", parent_proc);

	return 0;
err0:
	remove_proc_entry(PROC_WO_DIR, parent_proc);
	return -1;
}

/*
*
*/
void
wo_proc_exit(struct warp_entry *warp)
{
	struct proc_dir_entry *proc = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc_wo = (struct proc_dir_entry *)warp->proc_wo;

	if (proc && proc_wo) {
		remove_proc_entry(PROC_WO_DIR, proc);
		warp->proc_wo = NULL;
	}
}
#endif

int
rxbm_proc_init(struct warp_entry *warp,  struct wed_entry *wed)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc;

	proc = proc_create_data(PROC_RXBM_DIR, 0, root, &proc_rxbm_fops, wed);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!\n", PROC_RXBM_DIR);
		return -1;
	}

	wed->res_ctrl.proc_rxbm = (void *)proc;
	warp_dbg(WARP_DBG_INF, "create %s ok!\n", PROC_RXBM_DIR);
	return 0;
}


/*
*
*/

void
rxbm_proc_exit(struct warp_entry *warp, struct wed_entry *wed)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)wed->res_ctrl.proc_rxbm;

	if (proc && root) {
		remove_proc_entry(PROC_RXBM_DIR, root);
		wed->res_ctrl.proc_rxbm = NULL;
	}
}

#endif


int
wdma_entry_proc_init(struct warp_entry *warp, struct wdma_entry *wdma)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc;

	proc = proc_create_data(PROC_WDMA_DIR, 0, root, &proc_wdma_fops, wdma);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!\n", PROC_WDMA_DIR);
		return -1;
	}

	wdma->proc = (void *)proc;
	warp_dbg(WARP_DBG_INF, "create %s ok!\n", PROC_WDMA_DIR);
	return 0;
}

/*
*
*/
void
wdma_entry_proc_exit(struct warp_entry *warp, struct wdma_entry *wdma)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)wdma->proc;

	if (proc && root)
		remove_proc_entry(PROC_WDMA_DIR, root);
}

/*
*
*/
int
wed_entry_proc_init(struct warp_entry *warp, struct wed_entry *wed)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc;

	proc = proc_create_data(PROC_WED_DIR, 0, root, &proc_wed_fops, wed);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!\n", PROC_WED_DIR);
		return -1;
	}

	wed->proc = (void *)proc;
	warp_dbg(WARP_DBG_OFF, "create %s ok!!!\n", PROC_WED_DIR);
	return 0;
}

/*
*
*/
void
wed_entry_proc_exit(struct warp_entry *warp, struct wed_entry *wed)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)wed->proc;

	if (proc && root)
		remove_proc_entry(PROC_WED_DIR, root);
}

/* TODO: Refine Proc function for PAO */

#ifdef WED_PAO_SUPPORT
static int
warp_proc_pao_show(struct seq_file *seq, void *data)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	return warp_procinfo_dump_pao_hw(warp, seq);
}

static int
warp_proc_pao_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_pao_show, pde_data(file_inode(file)));
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao = {
	.proc_open = warp_proc_pao_open,
	.proc_read = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_pao = {
	.owner = THIS_MODULE,
	.open = warp_proc_pao_open,
	.read = seq_read,
	.llseek   = seq_lseek,
	.release = single_release,
};
#endif
#endif /* WED_PAO_SUPPORT */

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
static int
warp_proc_rtqm_show(struct seq_file *seq, void *data)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	return warp_procinfo_dump_rtqm_hw(warp, seq);
}

static int
warp_proc_rtqm_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_rtqm_show, pde_data(file_inode(file)));
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm = {
	.proc_open = warp_proc_rtqm_open,
	.proc_read = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_rtqm = {
	.owner = THIS_MODULE,
	.open = warp_proc_rtqm_open,
	.read = seq_read,
	.llseek   = seq_lseek,
	.release = single_release,
};
#endif /* HAVE_PROC_OPS */
#endif /* WED_RX_HW_RRO_3_0 || WED_RX_HW_RRO_3_1 */

#ifdef WED_RX_HW_RRO_3_0
static int
warp_proc_rro_show(struct seq_file *seq, void *data)
{
	struct warp_entry *warp = (struct warp_entry *)seq->private;

	return warp_procinfo_dump_rro_hw(warp, seq);
}

static int
warp_proc_rro_open(struct inode *inode, struct file *file)
{
	return single_open(file, warp_proc_rro_show, pde_data(file_inode(file)));
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rro = {
	.proc_open = warp_proc_rro_open,
	.proc_read = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations proc_warp_rro = {
	.open = warp_proc_rro_open,
	.read = seq_read,
	.llseek   = seq_lseek,
	.release = single_release,
};
#endif /* HAVE_PROC_OPS */
#endif /* WED_RX_HW_RRO_3_0 */

int
warp_entry_proc_init(struct warp_ctrl *warp_ctrl, struct warp_entry *warp)
{
	int ret = 0;
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp_ctrl->proc;
	struct proc_dir_entry *proc;
	char name[64] = "";
#ifdef WED_RX_HW_RRO_3_0
	struct wifi_entry *wifi = &warp->wifi;
#endif

	memset(&proc_settings, 0, sizeof(struct proc_global));

	ret = snprintf(name, sizeof(name), "%s%d", PROC_WHNAT_DIR, warp->idx);
	if (ret < 0)
		return -1;
	proc = proc_mkdir(name, root);
	warp->proc = (void *)proc;
	proc = proc_create_data(PROC_STAT_DIR, 0, warp->proc, &proc_warp_stat_fops,
				warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_STAT_DIR);
		goto err_proc_stat;
	}

	warp->proc_stat = (void *)proc;
	proc = proc_create_data(PROC_CR_DIR, 0, warp->proc, &proc_warp_cr_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_CR_DIR);
		goto err_proc_cr;
	}

	warp->proc_cr = (void *)proc;
	proc = proc_create_data(PROC_CFG_DIR, 0, warp->proc, &proc_warp_cfg_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_CFG_DIR);
		goto err_proc_cfg;
	}

	warp->proc_cfg = (void *)proc;
	proc = proc_create_data(PROC_TX_DIR, 0, warp->proc, &proc_warp_tx_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_TX_DIR);
		goto err_proc_tx;
	}

	warp->proc_tx = (void *)proc;
	proc = proc_create_data(PROC_RX_DIR, 0, warp->proc, &proc_warp_rx_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RX_DIR);
		goto err_proc_rx;
	}

	warp->proc_rx = (void *)proc;
	proc = proc_create_data(PROC_CTRL_DIR, 0, warp->proc, &proc_ctrl_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_CTRL_DIR);
		goto err_proc_ctrl;
	}

	warp->proc_ctrl = (void *)proc;

#ifdef WED_PAO_SUPPORT
	proc = proc_create_data(PROC_PAO_DIR, 0, warp->proc, &proc_warp_pao, warp);
	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_PAO_DIR);
		goto err_proc_pao;
	}

	warp->proc_pao = (void *)proc;
#endif
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	proc = proc_create_data(PROC_RTQM_DIR, 0, warp->proc,
					&proc_warp_rtqm, warp);
	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_DIR);
		goto err_proc_rtqm;
	}

	warp->proc_rtqm = (void *)proc;
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi->hw.hw_cap & BIT(WIFI_HW_CAP_RRO_3_0)) {
		proc = proc_create_data(PROC_RRO_DIR, 0, warp->proc,
						&proc_warp_rro, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RRO_DIR);
			goto err_proc_rro;
		}

		warp->proc_rro = (void *)proc;
	}
#endif

	warp_dbg(WARP_DBG_INF, "create %s ok!!!\n", name);
	return 0;

#ifdef WED_RX_HW_RRO_3_0
err_proc_rro:
#endif
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	remove_proc_entry(PROC_RTQM_DIR, warp->proc);
err_proc_rtqm:
#endif
#ifdef WED_PAO_SUPPORT
	remove_proc_entry(PROC_PAO_DIR, warp->proc);
err_proc_pao:
#endif
	remove_proc_entry(PROC_CTRL_DIR, warp->proc);
err_proc_ctrl:
	remove_proc_entry(PROC_RX_DIR, warp->proc);
err_proc_rx:
	remove_proc_entry(PROC_TX_DIR, warp->proc);
err_proc_tx:
	remove_proc_entry(PROC_CFG_DIR, warp->proc);
err_proc_cfg:
	remove_proc_entry(PROC_CR_DIR, warp->proc);
err_proc_cr:
	remove_proc_entry(PROC_STAT_DIR, warp->proc);
err_proc_stat:
	remove_proc_entry(name, root);
	return -1;
}

/*
*
*/
void
warp_entry_proc_exit(struct warp_ctrl *warp_ctrl, struct warp_entry *warp)
{
	int ret = 0;
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp_ctrl->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)warp->proc;
	char name[64] = "";
#ifdef WED_RX_HW_RRO_3_0
	struct wifi_entry *wifi = &warp->wifi;
#endif

	if (warp->proc_cfg) {
		remove_proc_entry(PROC_CFG_DIR, proc);
		warp->proc_cfg = NULL;
	}

	if (warp->proc_tx) {
		remove_proc_entry(PROC_TX_DIR, proc);
		warp->proc_tx = NULL;
	}

	if (warp->proc_rx) {
		remove_proc_entry(PROC_RX_DIR, proc);
		warp->proc_rx = NULL;
	}

	if (warp->proc_cr) {
		remove_proc_entry(PROC_CR_DIR, proc);
		warp->proc_cr = NULL;
	}

	if (warp->proc_stat) {
		remove_proc_entry(PROC_STAT_DIR, proc);
		warp->proc_stat = NULL;
	}

	if (warp->proc_ctrl) {
		remove_proc_entry(PROC_CTRL_DIR, proc);
		warp->proc_ctrl = NULL;
	}

#ifdef WED_PAO_SUPPORT
	if (warp->proc_pao) {
		remove_proc_entry(PROC_PAO_DIR, proc);
		warp->proc_pao = NULL;
	}
#endif

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (warp->proc_rtqm) {
		remove_proc_entry(PROC_RTQM_DIR, proc);
		warp->proc_rtqm = NULL;
	}
#endif

#ifdef WED_RX_HW_RRO_3_0
	if (wifi->hw.hw_cap & BIT(WIFI_HW_CAP_RRO_3_0)) {
		if (warp->proc_rro) {
			remove_proc_entry(PROC_RRO_DIR, proc);
			warp->proc_rro = NULL;
		}
	}
#endif

	ret = snprintf(name, sizeof(name), "%s%d", PROC_WHNAT_DIR, warp->idx);
	if (ret < 0)
		return;

	if (proc) {
		remove_proc_entry(name, root);
		warp->proc = NULL;
	}
}

/*
*
*/
int
warp_ctrl_proc_init(struct warp_ctrl *warp_ctrl)
{
	struct proc_dir_entry *proc;

	proc = proc_mkdir(PROC_ROOT_DIR, NULL);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_ROOT_DIR);
		return -1;
	}

	warp_ctrl->proc = (void *)proc;
#ifdef WARP_CPU_TRACER
	proc = proc_create_data(PROC_TRACE_DIR, 0, warp_ctrl->proc,
				&proc_warp_trace_fops, warp_ctrl);

	if (!proc)
		goto err1;

	warp_ctrl->proc_trace = proc;
#endif
	proc = proc_create_data(PROC_CONF_DIR, 0, warp_ctrl->proc,
				&proc_warp_conf_fops, warp_ctrl);

	if (!proc)
		goto err1;

	warp_ctrl->proc_sw_conf = proc;
	warp_dbg(WARP_DBG_OFF, "create %s ok!!!\n", PROC_ROOT_DIR);
	return 0;
err1:
	remove_proc_entry(PROC_ROOT_DIR, warp_ctrl->proc);
	return 0;
}

/*
*
*/
void
warp_ctrl_proc_exit(struct warp_ctrl *warp_ctrl)
{
	struct proc_dir_entry *proc = (struct proc_dir_entry *)warp_ctrl->proc;

	if (proc) {
		remove_proc_entry(PROC_CONF_DIR, proc);
		remove_proc_entry(PROC_TRACE_DIR, proc);
		remove_proc_entry(PROC_ROOT_DIR, NULL);
	}
}
