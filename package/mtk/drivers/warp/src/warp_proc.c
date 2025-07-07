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
#define PROC_AMSDU_FIFO_DIR	"amsdu_fifo"	/* AMSDU FIFO information */
#define PROC_AMSDU_ENG0_DIR	"amsdu_eng0"	/* AMSDU ENGINE0 information */
#define PROC_AMSDU_ENG1_DIR	"amsdu_eng1"	/* AMSDU ENGINE1 information */
#define PROC_AMSDU_ENG2_DIR	"amsdu_eng2"	/* AMSDU ENGINE2 information */
#define PROC_AMSDU_ENG3_DIR	"amsdu_eng3"	/* AMSDU ENGINE3 information */
#define PROC_AMSDU_ENG4_DIR	"amsdu_eng4"	/* AMSDU ENGINE4 information */
#define PROC_AMSDU_ENG5_DIR	"amsdu_eng5"	/* AMSDU ENGINE5 information */
#define PROC_AMSDU_ENG6_DIR	"amsdu_eng6"	/* AMSDU ENGINE6 information */
#define PROC_AMSDU_ENG7_DIR	"amsdu_eng7"	/* AMSDU ENGINE7 information */
#define PROC_AMSDU_ENG8_DIR	"amsdu_eng8"	/* AMSDU ENGINE8 information */
#define PROC_QMEM_DIR		"qmem"		/* QMEM information */
#define PROC_QMEM_HEAD_DIR	"qmem_head"	/* QMEM head pointer information */
#define PROC_QMEM_TAIL_DIR	"qmem_tail"	/* QMEM tail pointer information */
#define PROC_QMEM_PRE_DIR	"qmem_pre"	/* QMEM pre pointer information */
#define PROC_HIFTXD_FETCH_DIR	"hiftxd_fetch"	/* HIFTXD fetch module information */
#define PROC_RTQM_IGRS0_DIR	"rtqm_igrs0"    /* Route QM IGRS0 information */
#define PROC_RTQM_IGRS1_DIR	"rtqm_igrs1"    /* Route QM IGRS1 information */
#define PROC_RTQM_IGRS2_DIR	"rtqm_igrs2"    /* Route QM IGRS2 information */
#define PROC_RTQM_IGRS3_DIR	"rtqm_igrs3"    /* Route QM IGRS3 information */
#define PROC_RTQM_ENQ_DIR	"rtqm_enq"	/* Route QM ENQ information */
#define PROC_RTQM_DEQ_DIR	"rtqm_deq"	/* Route QM DEQ information */
#define PROC_RROQM_DIR		"rro"		/* RRO related information */

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
#ifdef CONFIG_WED_HW_RRO_SUPPORT
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
#ifdef CONFIG_WED_HW_RRO_SUPPORT
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
	seq_puts(seq, "WED_PROC_PAO_WCID_STAT\t: echo 19 [wcid] > wed\n");
	seq_puts(seq, "WED_PROC_PAO_SET_WCID_STAT\t: echo 20 [wcid] [amsdu_num] [amsdu_len] [vlan] [hdrt]> wed\n");
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
#ifdef CONFIG_WED_HW_RRO_SUPPORT
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

/*
*
*/
static int
warp_proc_conf_show(struct seq_file *seq, void *v)
{
	struct warp_ctrl *ctrl = (struct warp_ctrl *)seq->private;
	u8 idx = 0;

	for (idx = 0 ; idx < ctrl->warp_num ; idx++) {
		seq_printf(seq, "[ S/W settings for warp%d ]\n", idx);
		seq_puts(seq, "\t[ TXBM ]\n");
		if (ctrl->sw_conf[idx].txbm.vld_group)
			seq_printf(seq, "\tInitial total size: %d\n", ctrl->sw_conf[idx].txbm.vld_group);
		if (ctrl->sw_conf[idx].txbm.max_group)
			seq_printf(seq, "\tMaximum total size: %d\n", ctrl->sw_conf[idx].txbm.max_group);
		seq_printf(seq, "\tRX WDMA ring depth: %d\n", ctrl->sw_conf[idx].rx_wdma_ring_depth);
		seq_printf(seq, "\t\tDynamic TXBM enabled: %s\n", (ctrl->sw_conf[idx].txbm.enable) ? "True" : "False");
		if (ctrl->sw_conf[idx].txbm.enable) {
			seq_printf(seq, "\t\tMaximum group number: %d \n", ctrl->sw_conf[idx].txbm.max_group);
			seq_printf(seq, "\t\tExtend/Shrink quota: %d \n", ctrl->sw_conf[idx].txbm.alt_quota);
			seq_printf(seq, "\t\tBuffer low threshold: %d\n", ctrl->sw_conf[idx].txbm.buf_low);
			seq_printf(seq, "\t\tBuffer high threshold: %d\n", ctrl->sw_conf[idx].txbm.buf_high);
			seq_printf(seq, "\t\tBudge refill watermark: %d\n", ctrl->sw_conf[idx].txbm.budget_refill_watermark);
			seq_printf(seq, "\t\tBudge limit: %d (Dynamically applied)\n", ctrl->sw_conf[idx].txbm.budget_limit);
		}
		seq_printf(seq, "\t[ RXBM ]\n");
		if (ctrl->sw_conf[idx].rxbm.vld_group)
			seq_printf(seq, "\tInitial total size: %d\n", ctrl->sw_conf[idx].rxbm.vld_group);
		if (ctrl->sw_conf[idx].rxbm.max_group)
			seq_printf(seq, "\tMaximum total size: %d\n", ctrl->sw_conf[idx].rxbm.max_group);
		seq_printf(seq, "\tTX WDMA ring depth: %d\n", ctrl->sw_conf[idx].tx_wdma_ring_depth);
		seq_printf(seq, "\t\tDynamic RXBM enabled: %s\n", (ctrl->sw_conf[idx].rxbm.enable) ? "True" : "False");
		if (ctrl->sw_conf[idx].rxbm.enable) {
			seq_printf(seq, "\t\tMaximum group number: %d \n", ctrl->sw_conf[idx].rxbm.max_group);
			seq_printf(seq, "\t\tExtend/Shrink quota: %d \n", ctrl->sw_conf[idx].rxbm.alt_quota);
			seq_printf(seq, "\t\tBuffer low threshold: %d\n", ctrl->sw_conf[idx].rxbm.buf_low);
			seq_printf(seq, "\t\tBuffer high threshold: %d\n", ctrl->sw_conf[idx].rxbm.buf_high);
			seq_printf(seq, "\t\tBudge refill watermark: %d\n", ctrl->sw_conf[idx].rxbm.budget_refill_watermark);
			seq_printf(seq, "\t\tBudge limit: %d (Dynamically applied)\n", ctrl->sw_conf[idx].rxbm.budget_limit);
			seq_printf(seq, "\t\tRecycle postponed: %d (Dynamically applied)\n", ctrl->sw_conf[idx].rxbm.recycle_postponed);
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
			struct sw_conf_t *sw_conf = &ctrl->sw_conf[warp_idx];

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

#ifdef CONFIG_WED_HW_RRO_SUPPORT

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
#ifdef CONFIG_WED_HW_RRO_SUPPORT
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
#endif	/* CONFIG_WED_HW_RRO_SUPPORT */
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
#endif /*CONFIG_WED_HW_RRO_SUPPORT*/

/*
 * global file operation
*/
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

#ifdef CONFIG_WED_HW_RRO_SUPPORT
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

#ifdef CONFIG_WED_HW_RRO_SUPPORT

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

#ifdef WED_PAO_SUPPORT
static ssize_t
warp_pao_amsdu_fifo_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MIN_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_fifo_cnt(wed, buf, len);
	
	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return len;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_fifo = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_fifo_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_fifo = {
	.read = warp_pao_amsdu_fifo_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng0_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng0_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng0 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng0_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng0 = {
	.read = warp_pao_amsdu_eng0_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng1_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng1_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng1 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng1_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng1 = {
	.read = warp_pao_amsdu_eng1_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng2_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng2_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng2 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng2_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng2 = {
	.read = warp_pao_amsdu_eng2_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng3_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng3_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng3 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng3_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng3 = {
	.read = warp_pao_amsdu_eng3_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng4_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng4_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng4 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng4_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng4 = {
	.read = warp_pao_amsdu_eng4_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng5_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng5_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng5 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng5_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng5 = {
	.read = warp_pao_amsdu_eng5_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng6_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng6_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng6 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng6_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng6 = {
	.read = warp_pao_amsdu_eng6_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng7_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng7_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng7 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng7_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng7 = {
	.read = warp_pao_amsdu_eng7_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_amsdu_eng8_read_debugfs(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_amsdu_eng8_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_amsdu_eng8 = {
	.proc_open = simple_open,
	.proc_read = warp_pao_amsdu_eng8_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_amsdu_eng8 = {
	.read = warp_pao_amsdu_eng8_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_qmem_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_qmem_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_qmem = {
	.proc_open = simple_open,
	.proc_read = warp_pao_qmem_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_qmem = {
	.read = warp_pao_qmem_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_qmem_head_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_qmem_head_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_qmem_head = {
	.proc_open = simple_open,
	.proc_read = warp_pao_qmem_head_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_qmem_head = {
	.read = warp_pao_qmem_head_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_qmem_tail_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_qmem_tail_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_qmem_tail = {
	.proc_open = simple_open,
	.proc_read = warp_pao_qmem_tail_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_qmem_tail = {
	.read = warp_pao_qmem_tail_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_qmem_pre_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_qmem_pre_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_qmem_pre = {
	.proc_open = simple_open,
	.proc_read = warp_pao_qmem_pre_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_qmem_pre = {
	.read = warp_pao_qmem_pre_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_pao_hiftxd_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_pao_hiftxd_in_cnt(wed, buf, len);
	len = warp_pao_hiftxd_ou0_cnt(wed, buf, len);
	len = warp_pao_hiftxd_ou1_cnt(wed, buf, len);
	len = warp_pao_hiftxd_buf_cnt(wed, buf, len);
	len = warp_pao_hiftxd_msdu_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_pao_hiftxd = {
	.proc_open = simple_open,
	.proc_read = warp_pao_hiftxd_read_debugfs,
};
#else
static const struct file_operations proc_warp_pao_hiftxd = {
	.read = warp_pao_hiftxd_read_debugfs,
	.open = simple_open,
};
#endif

#endif

#ifdef WED_RX_HW_RRO
static ssize_t
warp_rtqm_igrs0_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rtqm_igrs0_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm_igrs0 = {
	.proc_open = simple_open,
	.proc_read = warp_rtqm_igrs0_read_debugfs,
};
#else
static const struct file_operations proc_warp_rtqm_igrs0 = {
	.read = warp_rtqm_igrs0_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_rtqm_igrs1_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rtqm_igrs1_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm_igrs1 = {
	.proc_open = simple_open,
	.proc_read = warp_rtqm_igrs1_read_debugfs,
};
#else
static const struct file_operations proc_warp_rtqm_igrs1 = {
	.read = warp_rtqm_igrs1_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_rtqm_igrs2_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rtqm_igrs2_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm_igrs2 = {
	.proc_open = simple_open,
	.proc_read = warp_rtqm_igrs2_read_debugfs,
};
#else
static const struct file_operations proc_warp_rtqm_igrs2 = {
	.read = warp_rtqm_igrs2_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_rtqm_igrs3_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rtqm_igrs3_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm_igrs3 = {
	.proc_open = simple_open,
	.proc_read = warp_rtqm_igrs3_read_debugfs,
};
#else
static const struct file_operations proc_warp_rtqm_igrs3 = {
	.read = warp_rtqm_igrs3_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_rtqm_enq_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rtqm_enq_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm_enq = {
	.proc_open = simple_open,
	.proc_read = warp_rtqm_enq_read_debugfs,
};
#else
static const struct file_operations proc_warp_rtqm_enq = {
	.read = warp_rtqm_enq_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_rtqm_deq_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rtqm_deq_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rtqm_deq = {
	.proc_open = simple_open,
	.proc_read = warp_rtqm_deq_read_debugfs,
};
#else
static const struct file_operations proc_warp_rtqm_deq = {
	.read = warp_rtqm_deq_read_debugfs,
	.open = simple_open,
};
#endif

static ssize_t
warp_rro_read_debugfs(struct file *file,
			char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct warp_entry *entry = pde_data(file_inode(file));
	struct wed_entry *wed = &entry->wed;
	char *buf;
	ssize_t len = 0, ret;

	buf = kzalloc(DEBUGFS_MAX_BUF_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	len = warp_rro_cnt(wed, buf, len);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

	kfree(buf);

	return ret;

	return 0;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_warp_rro = {
	.proc_open = simple_open,
	.proc_read = warp_rro_read_debugfs,
};
#else
static const struct file_operations proc_warp_rro = {
	.read = warp_rro_read_debugfs,
	.open = simple_open,
};
#endif
#endif

/*
*
*/
int
warp_entry_proc_init(struct warp_ctrl *warp_ctrl, struct warp_entry *warp)
{
	int ret = 0;
	struct proc_dir_entry *root = (struct proc_dir_entry *)warp_ctrl->proc;
	struct proc_dir_entry *proc;
	char name[64] = "";
#ifdef WED_RX_HW_RRO
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
		goto err1;
	}

	warp->proc_stat = (void *)proc;
	proc = proc_create_data(PROC_CR_DIR, 0, warp->proc, &proc_warp_cr_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_CR_DIR);
		goto err2;
	}

	warp->proc_cr = (void *)proc;
	proc = proc_create_data(PROC_CFG_DIR, 0, warp->proc, &proc_warp_cfg_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_CFG_DIR);
		goto err3;
	}

	warp->proc_cfg = (void *)proc;
	proc = proc_create_data(PROC_TX_DIR, 0, warp->proc, &proc_warp_tx_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_TX_DIR);
		goto err4;
	}

	warp->proc_tx = (void *)proc;
	proc = proc_create_data(PROC_RX_DIR, 0, warp->proc, &proc_warp_rx_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RX_DIR);
		goto err4;
	}

	warp->proc_rx = (void *)proc;
	proc = proc_create_data(PROC_CTRL_DIR, 0, warp->proc, &proc_ctrl_fops, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_CTRL_DIR);
		goto err5;
	}

	warp->proc_ctrl = (void *)proc;

#ifdef WED_PAO_SUPPORT

	proc = proc_create_data(PROC_AMSDU_FIFO_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_fifo, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_FIFO_DIR);
		goto err6;
	}

	warp->proc_amsdu_fifo = (void *)proc;
	
	proc = proc_create_data(PROC_AMSDU_ENG0_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng0, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG0_DIR);
		goto err7;
	}

	warp->proc_amsdu_eng0 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG1_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng1, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG1_DIR);
		goto err8;
	}

	warp->proc_amsdu_eng1 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG2_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng2, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG2_DIR);
		goto err9;
	}

	warp->proc_amsdu_eng2 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG3_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng3, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG3_DIR);
		goto err10;
	}

	warp->proc_amsdu_eng3 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG4_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng4, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG4_DIR);
		goto err11;
	}

	warp->proc_amsdu_eng4 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG5_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng5, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG5_DIR);
		goto err12;
	}

	warp->proc_amsdu_eng5 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG6_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng6, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG6_DIR);
		goto err13;
	}

	warp->proc_amsdu_eng6 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG7_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng7, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG7_DIR);
		goto err14;
	}

	warp->proc_amsdu_eng7 = (void *)proc;

	proc = proc_create_data(PROC_AMSDU_ENG8_DIR, 0, warp->proc,
					&proc_warp_pao_amsdu_eng8, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_AMSDU_ENG8_DIR);
		goto err15;
	}

	warp->proc_amsdu_eng8 = (void *)proc;

	proc = proc_create_data(PROC_QMEM_DIR, 0, warp->proc,
					&proc_warp_pao_qmem, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_QMEM_DIR);
		goto err16;
	}

	warp->proc_qmem = (void *)proc;

	proc = proc_create_data(PROC_QMEM_HEAD_DIR, 0, warp->proc,
					&proc_warp_pao_qmem_head, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_QMEM_HEAD_DIR);
		goto err17;
	}

	warp->proc_qmem_head = (void *)proc;

	proc = proc_create_data(PROC_QMEM_TAIL_DIR, 0, warp->proc,
					&proc_warp_pao_qmem_tail, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_QMEM_TAIL_DIR);
		goto err18;
	}

	warp->proc_qmem_tail = (void *)proc;

	proc = proc_create_data(PROC_QMEM_PRE_DIR, 0, warp->proc,
					&proc_warp_pao_qmem_pre, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_QMEM_PRE_DIR);
		goto err19;
	}

	warp->proc_qmem_pre = (void *)proc;

	proc = proc_create_data(PROC_HIFTXD_FETCH_DIR, 0, warp->proc,
					&proc_warp_pao_hiftxd, warp);

	if (!proc) {
		warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_HIFTXD_FETCH_DIR);
		goto err20;
	}

	warp->proc_hiftxd = (void *)proc;
#endif

#ifdef WED_RX_HW_RRO
	if (wifi->hw.hw_cap & BIT(WIFI_HW_CAP_RRO)) {
		proc = proc_create_data(PROC_RTQM_IGRS0_DIR, 0, warp->proc,
						&proc_warp_rtqm_igrs0, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_IGRS0_DIR);
			goto err21;
		}

		warp->proc_rtqm_igrs0 = (void *)proc;

		proc = proc_create_data(PROC_RTQM_IGRS1_DIR, 0, warp->proc,
						&proc_warp_rtqm_igrs1, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_IGRS1_DIR);
			goto err22;
		}

		warp->proc_rtqm_igrs1 = (void *)proc;

		proc = proc_create_data(PROC_RTQM_IGRS2_DIR, 0, warp->proc,
						&proc_warp_rtqm_igrs2, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_IGRS2_DIR);
			goto err23;
		}

		warp->proc_rtqm_igrs2 = (void *)proc;

		proc = proc_create_data(PROC_RTQM_IGRS3_DIR, 0, warp->proc,
						&proc_warp_rtqm_igrs3, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_IGRS3_DIR);
			goto err24;
		}

		warp->proc_rtqm_igrs3 = (void *)proc;

		proc = proc_create_data(PROC_RTQM_ENQ_DIR, 0, warp->proc,
						&proc_warp_rtqm_enq, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_ENQ_DIR);
			goto err25;
		}

		warp->proc_rtqm_enq = (void *)proc;

		proc = proc_create_data(PROC_RTQM_DEQ_DIR, 0, warp->proc,
						&proc_warp_rtqm_deq, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RTQM_DEQ_DIR);
			goto err26;
		}

		warp->proc_rtqm_deq = (void *)proc;

		proc = proc_create_data(PROC_RROQM_DIR, 0, warp->proc,
						&proc_warp_rro, warp);

		if (!proc) {
			warp_dbg(WARP_DBG_ERR, "create %s failed!!!\n", PROC_RROQM_DIR);
			goto err27;
		}

		warp->proc_rro = (void *)proc;
	}
#endif

	warp_dbg(WARP_DBG_INF, "create %s ok!!!\n", name);
	return 0;

#ifdef WED_RX_HW_RRO
err27:
	remove_proc_entry(PROC_RTQM_DEQ_DIR, warp->proc);
err26:
	remove_proc_entry(PROC_RTQM_ENQ_DIR, warp->proc);
err25:
	remove_proc_entry(PROC_RTQM_IGRS3_DIR, warp->proc);
err24:
	remove_proc_entry(PROC_RTQM_IGRS2_DIR, warp->proc);
err23:
	remove_proc_entry(PROC_RTQM_IGRS1_DIR, warp->proc);
err22:
	remove_proc_entry(PROC_RTQM_IGRS0_DIR, warp->proc);
err21:
	remove_proc_entry(PROC_HIFTXD_FETCH_DIR, warp->proc);
#endif

#ifdef WED_PAO_SUPPORT
err20:
	remove_proc_entry(PROC_QMEM_PRE_DIR, warp->proc);
err19:
	remove_proc_entry(PROC_QMEM_TAIL_DIR, warp->proc);
err18:
	remove_proc_entry(PROC_QMEM_HEAD_DIR, warp->proc);
err17:
	remove_proc_entry(PROC_QMEM_DIR, warp->proc);
err16:
	remove_proc_entry(PROC_AMSDU_ENG8_DIR, warp->proc);
err15:
	remove_proc_entry(PROC_AMSDU_ENG7_DIR, warp->proc);
err14:
	remove_proc_entry(PROC_AMSDU_ENG6_DIR, warp->proc);
err13:
	remove_proc_entry(PROC_AMSDU_ENG5_DIR, warp->proc);
err12:
	remove_proc_entry(PROC_AMSDU_ENG4_DIR, warp->proc);
err11:
	remove_proc_entry(PROC_AMSDU_ENG3_DIR, warp->proc);
err10:
	remove_proc_entry(PROC_AMSDU_ENG2_DIR, warp->proc);
err9:
	remove_proc_entry(PROC_AMSDU_ENG1_DIR, warp->proc);
err8:
	remove_proc_entry(PROC_AMSDU_ENG0_DIR, warp->proc);
err7:
	remove_proc_entry(PROC_AMSDU_FIFO_DIR, warp->proc);
err6:
	remove_proc_entry(PROC_CTRL_DIR, warp->proc);
#endif
err5:
	remove_proc_entry(PROC_RX_DIR, warp->proc);
err4:
	remove_proc_entry(PROC_TX_DIR, warp->proc);
err3:
	remove_proc_entry(PROC_CR_DIR, warp->proc);
err2:
	remove_proc_entry(PROC_STAT_DIR, warp->proc);
err1:
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
#ifdef WED_RX_HW_RRO
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
	if (warp->proc_amsdu_fifo) {
		remove_proc_entry(PROC_AMSDU_FIFO_DIR, proc);
		warp->proc_amsdu_fifo = NULL;
	}

	if (warp->proc_amsdu_eng0) {
		remove_proc_entry(PROC_AMSDU_ENG0_DIR, proc);
		warp->proc_amsdu_eng0 = NULL;
	}
	
	if (warp->proc_amsdu_eng1) {
		remove_proc_entry(PROC_AMSDU_ENG1_DIR, proc);
		warp->proc_amsdu_eng1 = NULL;
	}

	if (warp->proc_amsdu_eng2) {
		remove_proc_entry(PROC_AMSDU_ENG2_DIR, proc);
		warp->proc_amsdu_eng2 = NULL;
	}

	if (warp->proc_amsdu_eng3) {
		remove_proc_entry(PROC_AMSDU_ENG3_DIR, proc);
		warp->proc_amsdu_eng3 = NULL;
	}

	if (warp->proc_amsdu_eng4) {
		remove_proc_entry(PROC_AMSDU_ENG4_DIR, proc);
		warp->proc_amsdu_eng4 = NULL;
	}

	if (warp->proc_amsdu_eng5) {
		remove_proc_entry(PROC_AMSDU_ENG5_DIR, proc);
		warp->proc_amsdu_eng5 = NULL;
	}

	if (warp->proc_amsdu_eng6) {
		remove_proc_entry(PROC_AMSDU_ENG6_DIR, proc);
		warp->proc_amsdu_eng6 = NULL;
	}

	if (warp->proc_amsdu_eng7) {
		remove_proc_entry(PROC_AMSDU_ENG7_DIR, proc);
		warp->proc_amsdu_eng7 = NULL;
	}

	if (warp->proc_amsdu_eng8) {
		remove_proc_entry(PROC_AMSDU_ENG8_DIR, proc);
		warp->proc_amsdu_eng8 = NULL;
	}

	if (warp->proc_qmem) {
		remove_proc_entry(PROC_QMEM_DIR, proc);
		warp->proc_qmem = NULL;
	}

	if (warp->proc_qmem_head) {
		remove_proc_entry(PROC_QMEM_HEAD_DIR, proc);
		warp->proc_qmem = NULL;
	}

	if (warp->proc_qmem_tail) {
		remove_proc_entry(PROC_QMEM_TAIL_DIR, proc);
		warp->proc_qmem = NULL;
	}

	if (warp->proc_qmem_pre) {
		remove_proc_entry(PROC_QMEM_PRE_DIR, proc);
		warp->proc_qmem = NULL;
	}

	if (warp->proc_hiftxd) {
		remove_proc_entry(PROC_HIFTXD_FETCH_DIR, proc);
		warp->proc_hiftxd = NULL;
	}
#endif

#ifdef WED_RX_HW_RRO
	if (wifi->hw.hw_cap & BIT(WIFI_HW_CAP_RRO)) {
		if (warp->proc_rtqm_igrs0) {
			remove_proc_entry(PROC_RTQM_IGRS0_DIR, proc);
			warp->proc_rtqm_igrs0 = NULL;
		}

		if (warp->proc_rtqm_igrs1) {
			remove_proc_entry(PROC_RTQM_IGRS1_DIR, proc);
			warp->proc_rtqm_igrs1 = NULL;
		}

		if (warp->proc_rtqm_igrs2) {
			remove_proc_entry(PROC_RTQM_IGRS2_DIR, proc);
			warp->proc_rtqm_igrs2 = NULL;
		}

		if (warp->proc_rtqm_igrs3) {
			remove_proc_entry(PROC_RTQM_IGRS3_DIR, proc);
			warp->proc_rtqm_igrs3 = NULL;
		}

		if (warp->proc_rtqm_enq) {
			remove_proc_entry(PROC_RTQM_ENQ_DIR, proc);
			warp->proc_rtqm_enq = NULL;
		}

		if (warp->proc_rtqm_deq) {
			remove_proc_entry(PROC_RTQM_DEQ_DIR, proc);
			warp->proc_rtqm_deq = NULL;
		}

		if (warp->proc_rro) {
			remove_proc_entry(PROC_RROQM_DIR, proc);
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
	proc = proc_create_data(PROC_TRACE_DIR, 0, warp_ctrl->proc,
				&proc_warp_trace_fops, warp_ctrl);

	if (!proc)
		goto err1;

	warp_ctrl->proc_trace = proc;

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
