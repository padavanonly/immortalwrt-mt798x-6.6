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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/rcupdate.h>
#include <linux/types.h>
#include <linux/timekeeping.h>
#include <net/cfg80211.h>
#include <linux/skbuff.h>
#include <linux/oom.h>
#include <linux/debugfs.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include "mt_linux.h"

#ifdef MT_WIFI_COMMON_SUPPORT

void mt_rcu_read_lock(void)
{
	rcu_read_lock();
}
EXPORT_SYMBOL(mt_rcu_read_lock);

void mt_rcu_read_unlock(void)
{
	rcu_read_unlock();
}
EXPORT_SYMBOL(mt_rcu_read_unlock);

void mt_pci_walk_bus(struct pci_bus *top,
		int (*cb)(struct pci_dev *, void *), void *userdata)
{
	pci_walk_bus(top, cb, userdata);
}
EXPORT_SYMBOL(mt_pci_walk_bus);

unsigned long mt_kallsyms_lookup_name(const char *name)
{
#if (KERNEL_VERSION(5, 7, 0) > LINUX_VERSION_CODE)
	return kallsyms_lookup_name(name);
#else
	return 0; /* kernel version > 5.7, don't support this API*/
#endif
}
EXPORT_SYMBOL(mt_kallsyms_lookup_name);

void mt_call_rcu(struct rcu_head *head, rcu_callback_t func)
{
	call_rcu(head, func);
}
EXPORT_SYMBOL(mt_call_rcu);

void mt_synchronize_rcu(void)
{
	synchronize_rcu();
}
EXPORT_SYMBOL(mt_synchronize_rcu);


ktime_t mt_ktime_get(void)
{
	return ktime_get();
}
EXPORT_SYMBOL(mt_ktime_get);


ktime_t mt_ktime_get_with_offset(enum tk_offsets offs)
{
	return ktime_get_with_offset(offs);
}
EXPORT_SYMBOL(mt_ktime_get_with_offset);

u64 mt_ktime_get_boottime_ns(void)
{
	return ktime_get_boottime_ns();
}
EXPORT_SYMBOL(mt_ktime_get_boottime_ns);

#ifdef RT_CFG80211_SUPPORT
int mt_cfg80211_vendor_cmd_reply(struct sk_buff *skb)
{
	return cfg80211_vendor_cmd_reply(skb);
}
EXPORT_SYMBOL(mt_cfg80211_vendor_cmd_reply);
#endif

int mt_register_oom_notifier(struct notifier_block *nb)
{
	return register_oom_notifier(nb);
}
EXPORT_SYMBOL(mt_register_oom_notifier);

int mt_unregister_oom_notifier(struct notifier_block *nb)
{
	return unregister_oom_notifier(nb);
}
EXPORT_SYMBOL(mt_unregister_oom_notifier);


void mt_debugfs_remove(struct dentry *dentry)
{
	debugfs_remove(dentry);
}
EXPORT_SYMBOL(mt_debugfs_remove);

void mt_debugfs_remove_recursive(struct dentry *dentry)
{
	debugfs_remove_recursive(dentry);
}
EXPORT_SYMBOL(mt_debugfs_remove_recursive);

struct dentry *mt_debugfs_create_file(const char *name, umode_t mode,
				      struct dentry *parent, void *data,
				      const struct file_operations *fops)
{
	return debugfs_create_file(name, mode, parent, data, fops);
}
EXPORT_SYMBOL(mt_debugfs_create_file);

struct dentry *mt_debugfs_create_dir(const char *name, struct dentry *parent)
{
	return debugfs_create_dir(name, parent);
}
EXPORT_SYMBOL(mt_debugfs_create_dir);

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
struct dentry *mt_debugfs_create_devm_seqfile(struct device *dev, const char *name,
				 struct dentry *parent,
				 int (*read_fn)(struct seq_file *s, void *data))
{
	return debugfs_create_devm_seqfile(dev, name, parent, read_fn);
}
#else
void mt_debugfs_create_devm_seqfile(struct device *dev, const char *name,
				 struct dentry *parent,
				 int (*read_fn)(struct seq_file *s, void *data))
{
	debugfs_create_devm_seqfile(dev, name, parent, read_fn);
}
#endif
EXPORT_SYMBOL(mt_debugfs_create_devm_seqfile);

struct dentry *mt_debugfs_lookup(const char *name, struct dentry *parent)
{
	return debugfs_lookup(name, parent);
}
EXPORT_SYMBOL(mt_debugfs_lookup);

void *mt_idr_find(const struct idr *idr, unsigned long id)
{
	return idr_find(idr, id);
}
EXPORT_SYMBOL(mt_idr_find);

void *mt_idr_remove(struct idr *idr, unsigned long id)
{
	return idr_remove(idr, id);
}
EXPORT_SYMBOL(mt_idr_remove);

int mt_idr_alloc(struct idr *idr, void *ptr, int start, int end, gfp_t gfp)
{
	return idr_alloc(idr, ptr, start, end, gfp);
}
EXPORT_SYMBOL(mt_idr_alloc);


void *mt_devm_kmalloc(struct device *dev, size_t size, gfp_t gfp)
{
	return devm_kmalloc(dev, size, gfp);
}
EXPORT_SYMBOL(mt_devm_kmalloc);

void *mt_devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
	return devm_kmalloc(dev, size, gfp | __GFP_ZERO);
}
EXPORT_SYMBOL(mt_devm_kzalloc);


int mt_init_dummy_netdev(struct net_device *dev)
{
	return init_dummy_netdev(dev);
}
EXPORT_SYMBOL(mt_init_dummy_netdev);


int mt_firmware_request_nowarn(const struct firmware **firmware, const char *name,
				struct device *device)
{
	return firmware_request_nowarn(firmware, name, device);
}
EXPORT_SYMBOL(mt_firmware_request_nowarn);

void mt_get_online_cpus(void)
{
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	get_online_cpus();
#else
	cpus_read_lock();
#endif
}
EXPORT_SYMBOL(mt_get_online_cpus);

void mt_put_online_cpus(void)
{
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	put_online_cpus();
#else
	cpus_read_unlock();
#endif
}
EXPORT_SYMBOL(mt_put_online_cpus);


struct workqueue_struct *mt_alloc_workqueue(const char *fmt, unsigned int flags, int max_active)
{
	return alloc_workqueue(fmt, flags, max_active);
}
EXPORT_SYMBOL(mt_alloc_workqueue);

void mt_destroy_workqueue(struct workqueue_struct *wq)
{
	return destroy_workqueue(wq);
}
EXPORT_SYMBOL(mt_destroy_workqueue);

struct workqueue_struct *mt_create_workqueue(const char *name)
{
	return create_workqueue(name);
}
EXPORT_SYMBOL(mt_create_workqueue);

void mt_init_delayed_work(struct delayed_work *work, work_func_t func)
{
	INIT_DELAYED_WORK(work, func);
}
EXPORT_SYMBOL(mt_init_delayed_work);

void mt_flush_workqueue(struct workqueue_struct *wq)
{
	return flush_workqueue(wq);
}
EXPORT_SYMBOL(mt_flush_workqueue);

void mt_queue_delayed_work(
	struct workqueue_struct *wq,
	struct delayed_work *dwork,
	unsigned long delay)
{
	queue_delayed_work(wq, dwork, delay);
}
EXPORT_SYMBOL(mt_queue_delayed_work);

unsigned long mt_msecs_to_jiffies(unsigned int m)
{
	return msecs_to_jiffies(m);
}
EXPORT_SYMBOL(mt_msecs_to_jiffies);
#endif /* MT_WIFI_COMMON_SUPPORT */


/* CONFIG_LINUX_CRYPTO */
struct crypto_aead *mt_aead_key_setup_encrypt(const char *alg, const u8 key[], size_t key_len, size_t mic_len)
{
	struct crypto_aead *tfm;
	int err;

	tfm = crypto_alloc_aead(alg, 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm))
		return tfm;

	err = crypto_aead_setkey(tfm, key, key_len);
	if (err)
		goto free_aead;
	err = crypto_aead_setauthsize(tfm, mic_len);
	if (err)
		goto free_aead;

	return tfm;

free_aead:
	crypto_free_aead(tfm);
	return ERR_PTR(err);
}
EXPORT_SYMBOL(mt_aead_key_setup_encrypt);

void mt_aead_key_free(struct crypto_aead *tfm)
{
	crypto_free_aead(tfm);
}
EXPORT_SYMBOL(mt_aead_key_free);

int mt_aead_encrypt(struct crypto_aead *tfm, u8 *b_0, u8 *aad, size_t aad_len, u8 *data, size_t data_len, u8 *mic)
{
	size_t mic_len = crypto_aead_authsize(tfm);
	struct scatterlist sg[3];
	struct aead_request *aead_req;
	int reqsize = sizeof(*aead_req) + crypto_aead_reqsize(tfm);
	u8 *__aad;
	int ret;

	aead_req = kzalloc(reqsize + aad_len, GFP_ATOMIC);
	if (!aead_req)
		return -ENOMEM;

	__aad = (u8 *)aead_req + reqsize;
	memcpy(__aad, aad, aad_len);

	sg_init_table(sg, 3);
	sg_set_buf(&sg[0], __aad, aad_len);
	sg_set_buf(&sg[1], data, data_len);
	sg_set_buf(&sg[2], mic, mic_len);

	aead_request_set_tfm(aead_req, tfm);
	aead_request_set_crypt(aead_req, sg, sg, data_len, b_0);
	aead_request_set_ad(aead_req, sg[0].length);

	ret = crypto_aead_encrypt(aead_req);
	kfree(aead_req);

	return ret;
}
EXPORT_SYMBOL(mt_aead_encrypt);

int mt_aead_decrypt(struct crypto_aead *tfm, u8 *b_0, u8 *aad, size_t aad_len, u8 *data, size_t data_len, u8 *mic)
{
	size_t mic_len = crypto_aead_authsize(tfm);
	struct scatterlist sg[3];
	struct aead_request *aead_req;
	int reqsize = sizeof(*aead_req) + crypto_aead_reqsize(tfm);
	u8 *__aad;
	int err;

	if (data_len == 0)
		return -EINVAL;

	aead_req = kzalloc(reqsize + aad_len, GFP_ATOMIC);

	if (!aead_req)
		return -ENOMEM;

	__aad = (u8 *)aead_req + reqsize;
	memcpy(__aad, aad, aad_len);

	sg_init_table(sg, 3);
	sg_set_buf(&sg[0], __aad, aad_len);
	sg_set_buf(&sg[1], data, data_len);
	sg_set_buf(&sg[2], mic, mic_len);

	aead_request_set_tfm(aead_req, tfm);
	aead_request_set_crypt(aead_req, sg, sg, data_len + mic_len, b_0);
	aead_request_set_ad(aead_req, sg[0].length);

	err = crypto_aead_decrypt(aead_req);
	kfree(aead_req);

	return err;
}
EXPORT_SYMBOL(mt_aead_decrypt);

size_t mt_crypto_aead_authsize(struct crypto_aead *tfm)
{
	return crypto_aead_authsize(tfm);
}
EXPORT_SYMBOL(mt_crypto_aead_authsize);

static int __init mt_wifi_cmn_init(void)
{
	int status = 0;

	pr_info("mt_wifi_cmn: loaded");

	return status;
}

static void __exit mt_wifi_cmn_exit(void)
{
	pr_info("mt_wifi_cmn: unloaded");
}



MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Wifi Common Layer");

module_init(mt_wifi_cmn_init);
module_exit(mt_wifi_cmn_exit);
