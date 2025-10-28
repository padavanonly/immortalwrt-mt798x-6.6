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

#ifndef __MT_WIFI_CMN_H_
#define __MT_WIFI_CMN_H_

/* CONFIG_LINUX_CRYPTO */
#include <linux/scatterlist.h>
#include <linux/ieee80211.h>
#include <crypto/aead.h>

#ifdef MT_WIFI_COMMON_SUPPORT
#include <linux/version.h>
#include <linux/rcupdate.h>
#include <linux/timekeeping.h>
#include <net/cfg80211.h>
#include <linux/oom.h>
#include <linux/debugfs.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/kallsyms.h>
#include <linux/pci.h>

void mt_rcu_read_lock(void);
void mt_rcu_read_unlock(void);
void mt_pci_walk_bus(struct pci_bus *top,
		int (*cb)(struct pci_dev *, void *), void *userdata);
unsigned long mt_kallsyms_lookup_name(const char *name);
void mt_call_rcu(struct rcu_head *head, rcu_callback_t func);
void mt_synchronize_rcu(void);
ktime_t mt_ktime_get(void);
ktime_t mt_ktime_get_with_offset(enum tk_offsets offs);
u64 mt_ktime_get_boottime_ns(void);
int mt_register_oom_notifier(struct notifier_block *nb);
int mt_unregister_oom_notifier(struct notifier_block *nb);
void mt_debugfs_remove(struct dentry *dentry);
void mt_debugfs_remove_recursive(struct dentry *dentry);
struct dentry *mt_debugfs_create_file(const char *name, umode_t mode,
				struct dentry *parent, void *data,
				const struct file_operations *fops);
struct dentry *mt_debugfs_create_dir(const char *name, struct dentry *parent);
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
struct dentry *mt_debugfs_create_devm_seqfile(struct device *dev, const char *name,
				struct dentry *parent,
				int (*read_fn)(struct seq_file *s, void *data));
#else
void mt_debugfs_create_devm_seqfile(struct device *dev, const char *name,
				struct dentry *parent,
				int (*read_fn)(struct seq_file *s, void *data));
#endif
struct dentry *mt_debugfs_lookup(const char *name, struct dentry *parent);
void *mt_idr_find(const struct idr *idr, unsigned long id);
void *mt_idr_remove(struct idr *idr, unsigned long id);
int mt_idr_alloc(struct idr *idr, void *ptr, int start, int end, gfp_t gfp);
void *mt_devm_kmalloc(struct device *dev, size_t size, gfp_t gfp);
void *mt_devm_kzalloc(struct device *dev, size_t size, gfp_t gfp);
int mt_init_dummy_netdev(struct net_device *dev);
int mt_firmware_request_nowarn(const struct firmware **firmware, const char *name,
				struct device *device);
void mt_get_online_cpus(void);
void mt_put_online_cpus(void);
struct workqueue_struct *mt_alloc_workqueue(const char *fmt, unsigned int flags, int max_active);
struct workqueue_struct *mt_create_workqueue(const char *name);
void mt_init_delayed_work(struct delayed_work *work, work_func_t func);
void mt_destroy_workqueue(struct workqueue_struct *wq);
void mt_flush_workqueue(struct workqueue_struct *wq);
void mt_queue_delayed_work(
	struct workqueue_struct *wq,
	struct delayed_work *dwork,
	unsigned long delay);
unsigned long mt_msecs_to_jiffies(unsigned int m);
#ifndef RT_CFG80211_SUPPORT
static inline int mt_cfg80211_vendor_cmd_reply(struct sk_buff *skb)
{
	return 0;
}
#else
int mt_cfg80211_vendor_cmd_reply(struct sk_buff *skb);
#endif

#else /* MT_WIFI_COMMON_SUPPORT */

#define mt_rcu_read_lock() rcu_read_lock()
#define mt_rcu_read_unlock() rcu_read_unlock()
#define mt_call_rcu(head, func) call_rcu(head, func)
#define mt_synchronize_rcu() synchronize_rcu()
#define mt_ktime_get() ktime_get()
#define mt_ktime_get_with_offset(offs) ktime_get_with_offset(offs)
#define mt_ktime_get_boottime_ns() ktime_get_boottime_ns()
#define mt_register_oom_notifier(nb) register_oom_notifier(nb)
#define mt_unregister_oom_notifier(nb) unregister_oom_notifier(nb)
#define mt_debugfs_remove(dentry) debugfs_remove(dentry)
#define mt_debugfs_remove_recursive(dentry) debugfs_remove_recursive(dentry)
#define mt_debugfs_create_file(name, mode, parent, data, fops) debugfs_create_file(name, mode, parent, data, fops)
#define mt_debugfs_create_dir(name, parent) debugfs_create_dir(name, parent)
#define mt_debugfs_create_devm_seqfile(dev, name, parent, read_fn) debugfs_create_devm_seqfile(dev, name, parent, read_fn)
#define mt_debugfs_lookup(name, parent) debugfs_lookup(name, parent)
#define mt_idr_find(idr, id) idr_find(idr, id)
#define mt_idr_remove(idr, id) idr_remove(idr, id)
#define mt_idr_alloc(idr, ptr, start, end, gfp) idr_alloc(idr, ptr, start, end, gfp)
#define mt_devm_kmalloc(dev, size, gfp) devm_kmalloc(dev, size, gfp)
#define mt_devm_kzalloc(dev, size, gfp) devm_kzalloc(dev, size, gfp)
#define mt_init_dummy_netdev(dev) init_dummy_netdev(dev)
#define mt_firmware_request_nowarn(firmware, name, device) firmware_request_nowarn(firmware, name, device)
#define mt_cfg80211_vendor_cmd_reply(skb) cfg80211_vendor_cmd_reply(skb)
#define mt_create_workqueue(_name) create_workqueue(_name)
#define mt_init_delayed_work(_work, _func) INIT_DELAYED_WORK(_work, _func)
#define mt_flush_workqueue(_workq) flush_workqueue(_workq)
#define mt_destroy_workqueue(_workq) destroy_workqueue(_workq)
#define mt_queue_delayed_work(_workq, _dwork, delay) queue_delayed_work(_workq, _dwork, delay)
#define mt_msecs_to_jiffies(_m) msecs_to_jiffies(_m)
#endif /* !MT_WIFI_COMMON_SUPPORT */

/* CONFIG_LINUX_CRYPTO */
struct crypto_aead *mt_aead_key_setup_encrypt(const char *alg, const u8 key[], size_t key_len, size_t mic_len);
size_t mt_crypto_aead_authsize(struct crypto_aead *tfm);
void mt_aead_key_free(struct crypto_aead *tfm);
int mt_aead_encrypt(struct crypto_aead *tfm, u8 *b_0, u8 *aad, size_t aad_len, u8 *data, size_t data_len, u8 *mic);
int mt_aead_decrypt(struct crypto_aead *tfm, u8 *b_0, u8 *aad, size_t aad_len, u8 *data, size_t data_len, u8 *mic);

#endif /* __MT_WIFI_CMN_H_ */
