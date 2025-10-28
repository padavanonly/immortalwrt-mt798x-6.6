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

#ifndef __MTK_CHIP_H
#define __MTK_CHIP_H
#include <linux/list.h>
#include "bus.h"
#include "core.h"
#include "mcu/mcu.h"

#define MAX_VEC_NUM 8
struct mtk_mib_status;

/** struct mtk_chip_mgmt - Chip manager
 *
 * @param idrm Assign chip idx
 * @param mutex Use for protect chip related register/unregister
 */
struct mtk_chip_mgmt {
	struct mtk_idr_mgmt idrm;
	struct mutex mutex;
};

/** struct mtk_chip_ctrl_ops - Chip operations
 *
 *  Chip specific operations
 *
 * @param driver_own Driver own operater
 * @param check_fwdl_state Check chip fwdl status
 * @param set_filter Set chip MAC filter
 * @param mac_mib_update Update chip mib counter
 * @param hw_init Initial chip hardware
 * @param hw_l05reset chip L05reset
*/
struct mtk_chip_ctrl_ops {
	int (*driver_own)(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans, bool drv_own);
	int (*check_fwdl_state)(struct mtk_hw_dev *dev, u32 state);
	void (*set_filter)(struct mtk_hw_dev *dev, u8 band, unsigned long flag);
	void (*mac_mib_update)(struct mtk_hw_dev *dev, u8 band_idx, struct mtk_mib_status *mib);
	int (*hw_init)(struct mtk_hw_dev *dev);
	int (*hw_reset)(struct mtk_hw_dev *dev);
	int (*hw_chip_reset)(struct mtk_hw_dev *dev);
	int (*get_pc_value)(struct mtk_hw_dev *dev, u32 *value);
};

enum {
	FW_FLAG_WM,
	FW_FLAG_DSP,
	FW_FLAG_WA,
	FW_FLAG_WO,
	FW_FLAG_MAX
};

enum {
	MAC_TYPE_NONE,
	MAC_TYPE_FMAC,
	MAC_TYPE_BMAC,
	MAC_TYPE_MAX
};

/* Need to align WA enum */
enum hif_txd_ver {
	HIF_TXD_V0_0 = 0x0,
	HIF_TXD_V0_1 = 0x1,
	HIF_TXD_V1_0 = 0x10,
	HIF_TXD_V2_0 = 0x20,
	HIF_TXD_V2_1 = 0x21,
	HIF_TXD_V2_2 = 0x22,
	HIF_TXD_V3_0 = 0x30,
};

enum mac_txd_ver {
	MAC_TXD_V1,
	MAC_TXD_V2,
	MAC_TXD_V3,
	MAC_TXD_V4,
	MAC_TXD_V5,
	MAC_TXD_V6,
};

enum HW_SER_LEVEL {
	HW_SER_LV_0_0 = 0,
	HW_SER_LV_0_5 = 5,
	HW_SER_LV_1_0 = 10,
	HW_SER_LV_10_0 = 100,
};

enum SER_LV_10_0_EVT {
	SER_LV_10_0_EVT_RRO_PA_NOT_MATCH,
};

/** struct mtk_chip_mcu_info
 *
 * @param fw Firmware file with path
 * @param rom_patch Firmware rom patch file with path
 * @param opt MCU options
 */
struct mtk_chip_mcu_info {
	enum mtk_mcu_load_method applied_method;
	char *fw[MCU_FW_MODE_MAX];
	enum mtk_mcu_fw_mode fw_mode;
	char *rom_patch;
	u32 opt;
	char fw_ver[10];
	char build_date[15];
	const char *rom_patch_hdr;
	const char *fw_hdr[MCU_FW_MODE_MAX];
	u32 fw_hdr_len[MCU_FW_MODE_MAX];
};

enum mtk_hw_limit {
	LIMIT_NO_DROP_IP_FRAG_OLD_PKT,
	LIMIT_SET_ADDBA_SSN,
	LIMIT_SET_ADDBA_TID,
	LIMIT_SET_BMC_WCID,
	LIMIT_SET_OUTSTANDING,
	LIMIT_NO_DROP_OLD_PKT,
};

enum mtk_hw_cap {
	CAP_PAO,
	CAP_RRO,
	CAP_RSS,
	CAP_PN_CHK,
	CAP_OFFLOAD_TXD,
};

/** struct mtk_hw_cap_info
 *
 * @param hw_amsdu_num Maximum buffer num in MAC
 * @param hw_amsdu_length Maximum buffer length in MAC
 * @param sw_max_amsdu_num Maximum buffer num in HIF TXD
 * @param sw_max_amsdu_len Maximum buffer length in HIF TXD
 */

struct mtk_hw_cap_info {
	u32 sw_max_amsdu_num;
	u32 sw_max_amsdu_len;
	u32 max_tx_token;
	u32 max_rx_token;
};

enum rro_bypass_type {
	BMC_NOT_BYPASS_UC_BASED_ON_WTBL = 0,
	ALL_NOT_BYPASS			= 1,
	BMC_BYPASS_UC_BASED_ON_WTBL	= 2,
	ALL_BYPASS			= 3,
	RRO_BYPASS_MAX,
};

enum txfreedone_path {
	TXFREEDONE_FROM_MAC	= 0,
	TXFREEDONE_FROM_WA	= 1,
	TXFREEDONE_MAX,
};

/** struct mtk_chip_hw_cap - Chip hardware capabilities
 *
 * This structure define a chip hardware capabilities
 *
 * @param dev_size Define per hardware device size
 * @param fw_flags Flags to indicate firmware capabilities
 * @param eeprom_default Default eeprom context
 * @param eeprom_size Default eeprom size
 * @param mtxd_sz Size of MAC txd
 * @param mrxd_sz Size of MAC rxd
 * @param mac_type Indicate Hwardware MAC type
 * @param mac_rxd_grp_0_sz Size of MAC rxd group0
 * @param hif_txd_ver HIF TXD version
 * @param mac_txd_ver MAC TXD version
 * @param mac_limit Indicate MAC limitation
 * @param mac_cap Indicate supported MAC featues
 * @param hwres Hardware resource Information
 * @param mcu_infos Indicate per MCU information
 */
struct mtk_chip_hw_cap {
	size_t dev_size;
	unsigned long fw_flags;
	u8 *eeprom_default;
	u32 eeprom_size;
	u16 mtxd_sz;
	u16 mrxd_sz;
	u32 mac_type;
	u8 mac_rxd_grp_0_sz;
	enum mac_txd_ver mac_txd_ver;
	enum hif_txd_ver hif_txd_ver;
	unsigned long mac_cap;
	struct mtk_hw_cap_info cap_info;
	struct mtk_hwres_cap *hwres;
	struct mtk_chip_mcu_info mcu_infos[MCU_MAX];
};

#define IS_CHIP_FMAC(_dev) \
	(_dev->chip_drv->hw_caps->mac_type == MAC_TYPE_FMAC)

enum {
	CHIP_TYPE_MASTER, /**< Master chip id of Multi-BUS */
	CHIP_TYPE_SLAVE,  /**< Slave chip id of Multi-BUS */
	CHIP_TYPE_MAX
};

enum vec_attr {
	VEC_ATTR_NOUSED,
	VEC_ATTR_TXD_RING,
	VEC_ATTR_RXD_RING,
	VEC_ATTR_RX_TFD_RING,
	VEC_ATTR_TX_CMD_RING,
	VEC_ATTR_RX_EVT_RING,
	VEC_ATTR_TX_FWDL_RING,
	VEC_ATTR_RRO_RSS,
	VEC_ATTR_LUMP,
};

/** union attr_info - Vector data attribute information
 * @param band Indicate band_idx bitmap
 * @param mcu Indicate MCU bitmap
 */
union attr_info {
	u8 band;
	u8 mcu;
};

/** struct vector_data - Interrupt vector specific data
 * @param attr enum vec_attr
 * @param ring_num Total number of grouping interrupt instead of lump interrupt
 * @param ring_id Ring number
 * @param int_ena_mask Initialed interrupt enable mask
 */
struct vector_data {
	u8 attr;
	union attr_info info;
	u8 ring_num;
	bool repeat;
	u32 ring_id;
	u32 int_ena_mask;
};

struct mtk_intr_option_set {
	u8 intr_opt_master;
	u8 intr_opt_slave;
};

/** struct mtk_intr_option_desc - Interrupt option description
 * @param irq_type PCI_IRQ_MSIX/PCI_IRQ_LEGACY
 * @param is_rss RSS Ring supported in the descriptor
 * @param vec_num corresponding vector number
 * @param alloc_vec_num Allocate the number of irq_vector from kernel,
 *	  alloc_vec_num is invalid (meaningless) if zero
 * @param vec_data array of interrupt vector related data
 */
struct mtk_intr_option_desc {
	u8 irq_type;
	u8 is_rss;
	u8 vec_num;
	u8 alloc_vec_num;
	struct vector_data vec_data[MAX_VEC_NUM];
};

/** struct mtk_chip_desc - Chip option description
 * @param rx_path_type The rx data flow path settings in PP and RRO
 * @param rro_bypass_type The RRO bypass settings in MDP
 * @param mld_dest_type The rx data flow path settings for MLD in MDP
 * @param txfreedone_path The TxFreeDone path to host (WA or MAC)
 * @param force_rro_disable Indicate if RRO must be disabled in this option
 * @param rro_disable_rro_bypass_type The RRO bypass settings if RRO is disabled
 * @param rro_disable_mld_dest_type The MDP settings if RRO is disabled
 * @param rro_disable_txfreedone_path The TxFreeDone path to host if RRO is disabled
 * @param max_ba_wsize_scene_mlo The maximum BA winsize in certain MLO scenrio
 */
struct mtk_chip_desc {
	u8 rx_path_type;
	u8 rro_bypass_type;
	u8 mld_dest_type;
	enum txfreedone_path txfreedone_path;
	bool force_rro_disable;
	u8 rro_disable_rro_bypass_type;
	u8 rro_disable_mld_dest_type;
	enum txfreedone_path rro_disable_txfreedone_path;
	u16 max_ba_wsize_scene_mlo;
};

struct mtk_chip_option_table {
	char *const option_info;
	struct mtk_chip_desc chip_desc;
	struct mtk_bus_desc bus_desc_master;
	struct mtk_bus_desc bus_desc_slave;
};

/** struct mtk_chip_drv - Chip driver structure format
 *
 * This structure define a chip driver information to framework
 *
 * @param bus_cfg bus information for this chip
 * @param ctl_ops chip specific operations
 * @param hw_caps chip hardware resource capabilities
 * @param device_id indicate chip id
 * @param interface_type indicate which interface type want to probe,
 *         see mtk_interface_type
 * @param dest_2_path indicate mcu destination path for this chip
 * @param mtk_chip_option_table Chip option table for all option type
 * @param mtk_intr_option_set Chip interrupt option set
 * @param chip_opt_table_sz The number of platform option table
 * @param intr_opt_set_sz The number of interrupt option set
 * @param rro_mode To indicate each chip use which RRO mode
 * @param chip_opt Indicate to use which chip option
 * @param intr_opt Indicate to use which interrupt option
 * @param sdb_band_sel Indicate to use which band selection on SDB
 */
struct mtk_chip_drv {
	struct mtk_bus_cfg bus_cfg;
	struct mtk_chip_ctrl_ops *ctl_ops;
	struct mtk_chip_hw_cap *hw_caps;
	u32 device_id;
	u8 interface_type;
	struct mtk_mcu_dest_2_path *dest_2_path;
	struct mtk_chip_option_table *chip_opt_table;
	struct mtk_intr_option_set *intr_opt_set;
	u8 chip_opt_table_sz;
	u8 intr_opt_set_sz;
	u8 rro_mode;
	u8 chip_opt;
	u8 intr_opt;
	u8 sdb_band_sel;
};

struct mtk_chip {
	struct mtk_idr_entry sid;
	struct mtk_chip_drv *drv;
	struct mtk_bus_driver *bus_drv;
	/*must put on last part as the private bus driver physical address*/
	u8 priv_drv[0] __aligned(sizeof(void *));
};


enum free_notify_ver {
	FREE_NOTIFY_VERSION_0 = 0,
	FREE_NOTIFY_VERSION_1,
	FREE_NOTIFY_VERSION_2,
	FREE_NOTIFY_VERSION_3,
	FREE_NOTIFY_VERSION_4,
	FREE_NOTIFY_VERSION_5,
	FREE_NOTIFY_VERSION_6,
	FREE_NOTIFY_VERSION_7,
	FREE_NOTIFY_VERSION_NUM,
};


int mtk_chip_init(struct mtk_hwifi_drv *drv, struct mtk_chip_mgmt *info);
void mtk_chip_exit(struct mtk_chip_mgmt *info);
int mtk_chip_register(struct mtk_chip_drv *chip_drv);
void mtk_chip_unregister(struct mtk_chip_drv *chip_drv);

#endif
