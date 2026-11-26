/*
 * Copyright (c) [2020] MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include "core.h"
#include "bus.h"

static int
mtk_trinfo_debugfs(struct seq_file *s, void *data)
{
	struct mtk_bus_trans *trans = to_bus_trans(dev_get_drvdata(s->private));
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_tk_dbg *tk_dbg = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM].tk_dbg;
	struct mtk_tk_dbg *tk_dbg_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC].tk_dbg;
	struct mtk_rx_tk_dbg *rx_tk_dbg = &dev->hw_ctrl.rx_tk_mgmt.rx_tk_dbg;
	struct mtk_bus_dbg *bus_dbg = &trans->bus_dbg;

	seq_puts(s, "===================tx token counter==============\n");
	seq_printf(s, "request_cnt: %u\n", tk_dbg->request_cnt);
	seq_printf(s, "release_cnt: %u\n", tk_dbg->release_cnt);
	seq_printf(s, "free_tkid_cnt: %u\n", tk_dbg->free_tkid_cnt);
	seq_printf(s, "request_err_cnt: %u\n", tk_dbg->request_err_cnt);
	seq_printf(s, "release_err_cnt: %u\n", tk_dbg->release_err_cnt);
	seq_printf(s, "free_tkid_err_cnt: %u\n", tk_dbg->free_tkid_err_cnt);
	seq_printf(s, "free_tkid_oor_cnt: %u\n", tk_dbg->free_tkid_oor_cnt);
	seq_printf(s, "tk_entry_pkt_null: %u\n", tk_dbg->tk_entry_pkt_null);

	seq_puts(s, "===================rx token counter==============\n");
	seq_printf(s, "request_cnt: %u\n", rx_tk_dbg->request_cnt);
	seq_printf(s, "release_cnt: %u\n", rx_tk_dbg->release_cnt);
	seq_printf(s, "request_err_cnt: %u\n", rx_tk_dbg->request_err_cnt);

	if (!dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC].max_tx_tk_nums)
		goto dump_bus;

	seq_puts(s, "=================bmc token counter============\n");
	seq_printf(s, "request_cnt: %u\n", tk_dbg_bmc->request_cnt);
	seq_printf(s, "release_cnt: %u\n", tk_dbg_bmc->release_cnt);
	seq_printf(s, "free_tkid_cnt: %u\n", tk_dbg_bmc->free_tkid_cnt);
	seq_printf(s, "request_err_cnt: %u\n", tk_dbg_bmc->request_err_cnt);
	seq_printf(s, "release_err_cnt: %u\n", tk_dbg_bmc->release_err_cnt);
	seq_printf(s, "free_tkid_err_cnt: %u\n", tk_dbg_bmc->free_tkid_err_cnt);
	seq_printf(s, "free_tkid_oor_cnt: %u\n", tk_dbg_bmc->free_tkid_oor_cnt);
	seq_printf(s, "tk_entry_pkt_null: %u\n", tk_dbg_bmc->tk_entry_pkt_null);

dump_bus:
	seq_puts(s, "===================bus0 counter==============\n");
	seq_printf(s, "tx_cnt: %u\n", bus_dbg->tx_cnt);
	seq_printf(s, "tx_err_cnt: %u\n", bus_dbg->tx_err_cnt);
	seq_printf(s, "rx_cnt: %u\n", bus_dbg->rx_cnt);
	seq_printf(s, "rx_err_cnt: %u\n", bus_dbg->rx_err_cnt);
	seq_printf(s, "free_notify_cnt: %u\n", bus_dbg->free_notify_cnt);
	seq_printf(s, "free_notify_err_cnt: %u\n", bus_dbg->free_notify_err_cnt);
	seq_printf(s, "txs_cnt: %u\n", bus_dbg->txs_cnt);
	seq_printf(s, "txs_err_cnt: %u\n", bus_dbg->txs_err_cnt);
	seq_printf(s, "sdo_event_cnt: %u\n", bus_dbg->sdo_event_cnt);
	seq_printf(s, "sdo_event_err_cnt: %u\n", bus_dbg->sdo_event_err_cnt);
	seq_printf(s, "rx_leagcy_drop_cnt: %u\n", bus_dbg->rx_leagcy_drop_cnt);
	seq_printf(s, "rx_pn_check_drop_cnt: %u\n", bus_dbg->rx_pn_check_drop_cnt);
	seq_printf(s, "rx_incmd_reason_drop_cnt: %u\n", bus_dbg->rx_incmd_reason_drop_cnt);
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	seq_puts(s, "=================bus0 rx process counter============\n");
	seq_printf(s, "kfifo full drop cnts: %u\n", bus_dbg->kfifo_full_drop_by_hwifi_cnt);
	seq_printf(s, "hit kfifo buffer full cnts: %u\n", bus_dbg->kfifo_full_cnt);
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

	trans = trans->next;
	if (!trans)
		return 0;

	bus_dbg = &trans->bus_dbg;
	seq_puts(s, "===================bus1 counter==============\n");
	seq_printf(s, "tx_cnt: %u\n", bus_dbg->tx_cnt);
	seq_printf(s, "tx_err_cnt: %u\n", bus_dbg->tx_err_cnt);
	seq_printf(s, "rx_cnt: %u\n", bus_dbg->rx_cnt);
	seq_printf(s, "rx_err_cnt: %u\n", bus_dbg->rx_err_cnt);
	seq_printf(s, "free_notify_cnt: %u\n", bus_dbg->free_notify_cnt);
	seq_printf(s, "free_notify_err_cnt: %u\n", bus_dbg->free_notify_err_cnt);
	seq_printf(s, "txs_cnt: %u\n", bus_dbg->txs_cnt);
	seq_printf(s, "txs_err_cnt: %u\n", bus_dbg->txs_err_cnt);
	seq_printf(s, "sdo_event_cnt: %u\n", bus_dbg->sdo_event_cnt);
	seq_printf(s, "sdo_event_err_cnt: %u\n", bus_dbg->sdo_event_err_cnt);
	seq_printf(s, "rx_leagcy_drop_cnt: %u\n", bus_dbg->rx_leagcy_drop_cnt);
	seq_printf(s, "rx_pn_check_drop_cnt: %u\n", bus_dbg->rx_pn_check_drop_cnt);
	seq_printf(s, "rx_incmd_reason_drop_cnt: %u\n", bus_dbg->rx_incmd_reason_drop_cnt);
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	seq_puts(s, "=================bus1 rx process counter============\n");
	seq_printf(s, "kfifo full drop cnts: %u\n", bus_dbg->kfifo_full_drop_by_hwifi_cnt);
	seq_printf(s, "hit kfifo buffer full cnts: %u\n", bus_dbg->kfifo_full_cnt);
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

	return 0;
}

int
mtk_dbg_exit_device(struct mtk_hw_dev *dev)
{
	return 0;
}

#ifdef CONFIG_HWIFI_DBG
static ssize_t
mtk_dbg_ctl_read_debugfs(struct file *file,
							char __user *ubuf,
							size_t count, loff_t *ppos)
{
	char buf[256] = {0};
	int len = 0;

	len += scnprintf(buf + len, sizeof(buf) - len,
					"mtk_dbg_mask: 0x%x\n",
					mtk_dbg_mask);

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t
mtk_dbg_ctl_write_debugfs(struct file *file,
							const char __user *ubuf,
							size_t count, loff_t *ppos)
{
	char buf[256] = {0};
	int ret, rc;
	int dbg_mask;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = sscanf(buf, "%x", &dbg_mask);

	if (ret != 1) {
		ret = -EINVAL;
		goto done;
	}

	if (dbg_mask > MTK_ANY) {
		ret = -EINVAL;
		goto done;
	} else {
		mtk_dbg_mask = dbg_mask;
	}

	ret = count;
done:
	return ret;
}

static const struct file_operations fops_mtk_dbg_ctl = {
	.read = mtk_dbg_ctl_read_debugfs,
	.write = mtk_dbg_ctl_write_debugfs,
	.open = simple_open,
};

static int
mtk_dbg_mask_debugfs(struct seq_file *s, void *data)
{
	seq_printf(s, "debug_mask: 0x%x\n", mtk_dbg_mask);
	seq_printf(s, "\tMTK_CORE(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(0) ? 1 : 0, BIT(0));
	seq_printf(s, "\tMTK_MCU(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(1) ? 1 : 0, BIT(1));
	seq_printf(s, "\tMTK_BUS(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(2) ? 1 : 0, BIT(2));
	seq_printf(s, "\tMTK_MAC(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(3) ? 1 : 0, BIT(3));
	seq_printf(s, "\tMTK_TX(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(4) ? 1 : 0, BIT(4));
	seq_printf(s, "\tMTK_RX(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(5) ? 1 : 0, BIT(5));
	seq_printf(s, "\tMTK_TXD(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(6) ? 1 : 0, BIT(6));
	seq_printf(s, "\tMTK_TXD_DUMP(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(7) ? 1 : 0, BIT(7));
	seq_printf(s, "\tMTK_RXD(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(8) ? 1 : 0, BIT(8));
	seq_printf(s, "\tMTK_RXD_DUMP(%d): 0x%lx\n",
		   			mtk_dbg_mask & BIT(9) ? 1 : 0, BIT(9));
	seq_printf(s, "\tMTK_TK_TX(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(10) ? 1 : 0, BIT(10));
	seq_printf(s, "\tMTK_TK_RX(%d): 0x%lx\n",
					mtk_dbg_mask & BIT(11) ? 1 : 0, BIT(11));
	seq_printf(s, "\tMTK_ANY: 0xffffffff\n");

	return 0;
}

static int
mtk_dbglog_debugfs_register(struct mtk_hw_dev *dev)
{
	struct dentry *dir;

	dir = mt_debugfs_create_file("dbg_ctl", 0644,
							dev->sys_idx.dir,
							dev,
							&fops_mtk_dbg_ctl);

	if (!dir)
		return -ENOMEM;

#if (KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE)
	dir = mt_debugfs_create_devm_seqfile(dev->dev, "dbg_mask",
			dev->sys_idx.dir, mtk_dbg_mask_debugfs);
	if (!dir)
		return -ENOMEM;
#else
	mt_debugfs_create_devm_seqfile(dev->dev, "dbg_mask",
			dev->sys_idx.dir, mtk_dbg_mask_debugfs);
#endif

	return 0;
}
#endif

static ssize_t
mtk_dbg_info_write(struct file *file,
					const char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	int ret, rc;
	char buf[MAX_BUF_LEN] = {0};

	if (sizeof(buf) - 1 < count) {
		ret = -EOVERFLOW;
		dev_err(dev->dev, "%s(): MAX_BUF_LEN error: %zu\n", __func__,
			count - (sizeof(buf) - 1));
		goto done;
	}

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		dev_err(dev->dev, "%s(): simple_write_to_buffer error\n", __func__);
		goto done;
	}

	if (rc != count) {
		ret = -EIO;
		dev_err(dev->dev, "%s(): lose args from ubuf\n", __func__);
		goto done;
	}

	buf[rc] = '\0';

	ret = mtk_dbg_info(MTK_DBGFS_PATH, dev, buf);
	if (ret)
		goto done;

	ret = count;
done:
	return ret;
}

static const struct file_operations fops_mtk_dbg_info = {
	.write = mtk_dbg_info_write,
	.open = simple_open
};

static int
mtk_dbg_info_register(struct mtk_hw_dev *dev)
{
	struct dentry *dir;

	if (!dev->sys_idx.dir) {
		dev_err(dev->dev, "%s(): null pointer: dir\n", __func__);
		return -EINVAL;
	}

	dir = mt_debugfs_create_file("dbg_info", 0644,
							dev->sys_idx.dir,
							dev,
							&fops_mtk_dbg_info);

	if (!dir) {
		dev_err(dev->dev, "%s(): failed to create debugfs file\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

int
mtk_dbg_init_device(struct mtk_hw_dev *dev)
{
	struct dentry *dir = dev->sys_idx.dir;
	struct dentry *new_dir;
	int ret;
#if (KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE)

	new_dir = mt_debugfs_create_devm_seqfile(dev->dev, "trinfo",
		dir, mtk_trinfo_debugfs);

	if (IS_ERR_OR_NULL(new_dir))
		return -ENOMEM;
#else
	mt_debugfs_create_devm_seqfile(dev->dev, "trinfo",
		dir, mtk_trinfo_debugfs);

	new_dir = mt_debugfs_lookup("trinfo", dir);

	if (IS_ERR_OR_NULL(new_dir))
		return -ENOMEM;
#endif

#ifdef CONFIG_HWIFI_DBG
	if (mtk_dbglog_debugfs_register(dev))
		return -ENOMEM;
#endif

	ret = mtk_dbg_info_register(dev);
	if (ret)
		return ret;

	return 0;
}
