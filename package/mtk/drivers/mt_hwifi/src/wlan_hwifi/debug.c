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

#include <linux/vmalloc.h>
#include "core.h"

#ifdef CONFIG_HWIFI_DBG
unsigned int mtk_dbg_mask = 0;
EXPORT_SYMBOL(mtk_dbg_mask);

void mtk_dbg(enum mtk_debug_mask dbg_mask,
			const char *fmt, ...)
{
	if (mtk_dbg_mask & dbg_mask) {
		struct va_format vaf;
		va_list args;

		va_start(args, fmt);

		vaf.fmt = fmt;
		vaf.va = &args;

		pr_debug("%pV", &vaf);

		va_end(args);
	}
}
EXPORT_SYMBOL(mtk_dbg);

void mtk_dbg_dump(enum mtk_debug_mask dbg_mask,
				const char *msg,
				const void *buf, size_t len)
{
	if (mtk_dbg_mask & dbg_mask)
		print_hex_dump(KERN_DEBUG, msg, DUMP_PREFIX_OFFSET, 16, 1,
							buf, len, 1);
}
EXPORT_SYMBOL(mtk_dbg_dump);
#endif

struct mtk_hw_dev *
get_hwdev(enum mtk_dbg_path path, void *data)
{
	struct mtk_hw_dev *dev = NULL;

	switch (path) {
	case MTK_IOCTL_PATH:
		dev = ((struct mtk_hw_phy *)data)->hw_dev;
		break;
	case MTK_DBGFS_PATH:
		dev = data;
		break;
	default:
		pr_err("%s(): invalid path\n", __func__);
		break;
	}

	return dev;
}

#ifdef CONFIG_HWIFI_DBG_ISR
int mtk_show_isrinfo(enum mtk_dbg_path path, void *data, char *arg)
{
	struct mtk_hw_dev *dev = get_hwdev(path, data);
	struct mtk_bus_trans *trans;
	int ret = 0;
	u8 i;

	if (!dev) {
		pr_err("%s(): null pointer: dev\n", __func__);
		return -EINVAL;
	}

	if (!dev->dbg_ops) {
		pr_err("%s(): null pointer: dbg_ops\n", __func__);
		return -EOPNOTSUPP;
	}

	if (!dev->dbg_ops->show_isr_info) {
		pr_err("%s(): null pointer: show_isr_info\n", __func__);
		return -EOPNOTSUPP;
	}

	trans = dev->bus_trans;

	for (i = 0; trans; i++) {
		ret = dev->dbg_ops->show_isr_info(trans, i);
		if (ret)
			return ret;

		trans = trans->next;
	}

	return ret;
}
#endif

struct mtk_cmm_dbg mtk_cmm_dbg_tab[] = {
#ifdef CONFIG_HWIFI_DBG_ISR
	{"isrinfo", mtk_show_isrinfo},
#endif
	{NULL, NULL}
};

int mtk_dbg_info(enum mtk_dbg_path path, void *data, char *arg)
{
	struct mtk_cmm_dbg *cmm_dbg;
	char *value;

	if (!arg || !*arg) {
		pr_err("%s(): No argument is input\n", __func__);
		return -EINVAL;
	}

	value = strchr(arg, '-');

	if (value) {
		if (strlen(value) > 1) {
			*value = '\0';
			value++;
		} else
			value = NULL;
	}

	for (cmm_dbg = mtk_cmm_dbg_tab; cmm_dbg->name; cmm_dbg++) {
		if (strcmp(arg, cmm_dbg->name))
			continue;

		if (!cmm_dbg->proc) {
			pr_err("%s(): null pointer: proc\n", __func__);
			return -EOPNOTSUPP;
		}

		return cmm_dbg->proc(path, data, value);
	}

	pr_err("%s(): Unable to find the argument: %s\n", __func__, arg);
	return -EINVAL;
}
EXPORT_SYMBOL(mtk_dbg_info);
