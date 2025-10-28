#ifndef __HWIFI_MAC_H__
#define __HWIFI_MAC_H__

#if !(defined(LINUX) || defined(NONE))
#define LINUX
#endif

#ifdef LINUX
#include <linux/spinlock_types.h>
#endif /* LINUX */
#include "ser_cmm.h"
#include "physical_device.h"
#include "wlan_tr.h"

#define MAC_ADDR_LEN 6

#define MCU_CMD_EEPROM_TIMEOUT 30
#define MCU_CMD_SPECIAL_TIMEOUT 20

#define MCU_CMD_TIMEOUT 3

struct wifi_dev;
struct _TX_BLK;
struct _RX_BLK;
struct sk_buff;

/**
 * enum mtk_hw_flags - hardware flags
 *
 * @MAC_EFUSE_READY: Indicate hardware efuse is ready should
 *      not load efuse again.
 */
enum mtk_hw_flags {
	MAC_EFUSE_READY,
	MAC_BA_OFFLOAD,
	MAC_RX_OFFLOAD,
	MAC_SW_AMSDU,
	MAC_HW_RRO,
	MAC_MULTI_BUS,
	MAC_CHIP_OPTION,
	MAC_PARSE_TX_PAYLOAD,
	MAC_MAX_BA_WSIZE_SCENE_MLO,
};

/**
 * struct mtk_mac_cap_info - MAC capabiliy information
 *
 * This structure contains the configure for MAC capability
 *
 * @sw_amsdu_num: (If not scatter and gather) Maximum MSDU number for A-MSDU.
 *                (If scatter and gather) Maximum buffer number for A-MSDU.
 * @sw_amsdu_length: Maximum length for A-MSDU
 * @hif_txd_ver_sdo: HIF TXD version for SDO HW path
 * @rx_path_type: Rx path type settings for PP and RRO
 * @rro_bypass_type: RRO bypass settings for MDP
 * @mld_dest_type: MLD destination settings for MDP
 * @txfreedone_path: TxFreeDone event's source (WA or MAC)
 */
struct mtk_mac_cap_info {
	u32 sw_max_amsdu_num;
	u32 sw_max_amsdu_len;
	u16 max_ba_wsize_scene_mlo;
	/* For WM/WA info */
	u32 hif_txd_ver_sdo;
	u8 rx_path_type;
	u8 rro_bypass_type;
	u8 mld_dest_type;
	u8 txfreedone_path;
};

/**
 * struct mtk_mac_hw - hardware information and state
 *
 * This structure contains the configure and hardware information
 * for an 80211 phy.
 *
 * @pdev: This point to the bus device for do some device control
 *      and memory allocation.
 * @band_idx: Indicate 80211 phy hw band indx for mcu payload to use.
 * @chip_id: Indicate chip id. Wlan driver can use this value to do
 *      some chip specific decisions.
 * @phy_num: Indicate number of phy for this hw is prepared
 * @bss_priv_size: size (in bytes) of the drv_priv data area within
 *      &struct mtk_mac_bss.
 * @sta_priv_size: size (in bytes) of the drv_priv data area within
 *      &struct mtk_mac_sta.
 * @txq_priv_size: size (in bytes) of the drv_priv data area within
 *      &struct mtk_mac_txq.
 * @flags: hardware flags, see &enum mtk_hw_flags
 * @cap_info: hardware capability information
 * @priv: pointer to private area that was allocated for driver use
 *      along with this structure.
 */
struct mtk_mac_hw {
	struct mtk_mac_sw_ops *sw_ops;
	struct device *pdev;
	u8 band_idx;
	u32 chip_id;
	u32 hw_id;
	u32 phy_num;
	u32 bss_priv_size;
	u32 sta_priv_size;
	u32 txq_priv_size;
	unsigned long flags;
	struct mtk_mac_cap_info cap_info;
	void *priv;
};

/**
 * struct mtk_mac_sw_ops - callbacks from hwifi driver to wlan driver
 *
 * This structure contains various callbacks that wlan driver must
 * handle.
 *
 * @ba_trig_event: Notify wlan driver to establish BA session
 */

struct mtk_mac_sw_ops {
	int (*ba_trig_event)(struct mtk_mac_hw *hw, u16 wcid, u8 tid);
};

/**
 * enum mtk_mac_tx_flags - tx 80211 data information
 *
 * @MTK_TX_FLAG_MCAST: Indicate the tx data is a multi-cast frame.
 * @MTK_TX_FLAG_BEACON: Indicate the tx data is a beacon frame.
 * @MTK_TX_FLAG_MGMT: Indicate the tx data is a management frame.
 * @MTK_TX_FLAG_ACQ: Indicate the tx data need to enqueue hardware ACQ.
 * @MTK_TX_FLAG_BIP: Indicate the tx data need to process BIP.
 * @MTK_TX_FLAG_PROTECT: Indicate the tx data is protected frame.
 * @MTK_TX_FLAG_NOACK: Indicate the tx data ACK policy is no ack frame.
 * @MTK_TX_FLAG_CCK: Indicate the tx data need to fix as CCK rate.
 * @MTK_TX_FLAG_QOSDATA: Indicate the tx data is Qos data frame.
 * @MTK_TX_FLAG_MCU_OFFLOAD: Indicate the tx data need offload CPU's help.
 * @MTK_TX_FLAG_HDR_TRANS: Indicate the tx data need hardwared header trans.
 * @MTK_TX_FLAG_APPLY_TXD: Indicate the tx data ignore process on offload CPU.
 * @MTK_TX_FLAG_ADDBA: Indicate the tx data is AddBA frame.
 * @MTK_TX_FLAG_DIRECT_TX: Indicate the tx data is a direct tx frame ?.
 */
enum mtk_mac_tx_flags {
	MTK_TX_FLAG_MCAST = BIT(0),
	MTK_TX_FLAG_BEACON = BIT(1),
	MTK_TX_FLAG_MGMT = BIT(2),
	MTK_TX_FLAG_ACQ = BIT(3),
	MTK_TX_FLAG_BIP = BIT(4),
	MTK_TX_FLAG_PROTECT = BIT(5),
	MTK_TX_FLAG_NOACK = BIT(6),
	MTK_TX_FLAG_CCK = BIT(7),
	MTK_TX_FLAG_QOSDATA = BIT(8),
	MTK_TX_FLAG_MCU_OFFLOAD = BIT(9),
	MTK_TX_FLAG_HDR_TRANS = BIT(10),
	MTK_TX_FLAG_APPLY_TXD = BIT(11),
	MTK_TX_FLAG_ADDBA = BIT(12),
	MTK_TX_FLAG_DIRECT_TX = BIT(13),
	MTK_TX_FLAG_HW_AMSDU = BIT(14),
	MTK_TX_FLAG_TXS_MCU = BIT(15),
	MTK_TX_FLAG_TXS_HOST = BIT(16),
	MTK_TX_FLAG_FORCE_LINK = BIT(17),
};

/**
 * struct mtk_mac_txq - txq for a specific station and tid
 *
 * @bss: The bss that this station is association.
 * @sta: The sta that this txq is owned.
 * @tid: The tid value this txq is owned.
 * @ac: The ac that this tid is owned.
 * @frames: The skb frame list head to indcate a set of msdu frames
 *     belong to the same AMSDU frame.
 * @drv_priv: The hardware area for this txq.
 */
struct mtk_mac_txq {
	struct mtk_mac_bss *bss;
	struct mtk_mac_sta *sta;
	u8 tid;
	u8 ac;
	u8 drv_priv[] __aligned(sizeof(void *));
};

/**
 * mtk_mac_sta_type - station types
 *
 * @MAC_STA_TYPE_NORMAL: Indicate this station is a non-MLD STA.
 * @MAC_STA_TYPE_GROUP: Indicate this is a station entry for BMC packets.
 * @MAC_STA_TYPE_MLD: Indicate this is a MLD STA.
 */
enum mtk_mac_sta_type {
	MAC_STA_TYPE_NORMAL,
	MAC_STA_TYPE_GROUP,
	MAC_STA_TYPE_MLD,
};

#define IEEE80211_TID_NUM 16
#define MGMT_TXQ IEEE80211_TID_NUM
#define TXQ_NUM (IEEE80211_TID_NUM + 1)
/**
 * struct mtk_mac_sta - station information and state
 *
 * @sta_type: Indicate station type see &enum mtk_mac_sta_type
 * @link_wcid: Indicate a station hardware index for a specific link
 * @link_wcid2: Indicate a station hardware index for a specific link (2nd)
 * @mld_sta_idx: Indicate a mld station idx, link stations belong to
 *      a mld station if mld_sta_idx is same assign by wlan driver.
 * @mld_primary_idx: Indicate mld primary idx (use for cmd: starec update)
 * @mld_secondary_idx: Indicate mld secondary idx (use for cmd: starec update)
 * @is_dummy_role: Indicate if dummy mac_sta
 * @sw_wcid: Indicate dummy mac_sta's temporary sw_wcid for tr_entry[] usage
 * @sw_dummy_sta_ref_cnt: Indicate mac_sta's ref cnt
 * @rcu: RCU usage for call_rcu
 * @txq: Pointer to an array of txq per tid belong to this station.
 * @drv_priv: Pointer to hardware area for this station.
 */
struct mtk_mac_sta {
	u8 sta_type;
	u32 link_wcid;
	u32 link_wcid2;
	u32 mld_sta_idx;
	u32 mld_primary_idx;
	u32 mld_secondary_idx;
	bool is_dummy_role;
	u32 sw_wcid;
	atomic_t sw_dummy_sta_ref_cnt;
	struct rcu_head rcu;
	struct mtk_mac_txq *txq[TXQ_NUM];
	u8 drv_priv[0] __aligned(sizeof(void *));
};

/**
 * struct mtk_mac_phy - phy information and state
 *
 * This phy information is used for wlan driver to record phy information only.
 * hwifi doesn't use it.
 *
 * @chan: Indicate parking control channel
 * @cha2: Indicate parking secondary bw80 control channel
 * @cen_chan: Indicate parking central channel
 * @bw: Indicate hw bw.
 * @ext_cha: Indicate external channel value.
 * @state: Indicate phy state
 */
struct mtk_mac_phy {
	spinlock_t lock;
	u8 chan;
	u8 chan2;
	u8 cen_chan;
	u8 bw;
	u8 ext_cha;
	u8 state;
	u8 reason;
#ifdef ANTENNA_CONTROL_SUPPORT
	u8 rx_stream;
#endif
};

/**
 * mtk_mac_mld_type - self mld types
 *
 * @MAC_MLD_TYPE_NONE: Indicate non-MLD.
 * @MAC_MLD_TYPE_SINGLE: Indicate this is single-link MLD group.
 * @MAC_MLD_TYPE_MULTI: Indicate this is multi-link MLD group.
 */
enum mtk_mac_mld_type {
	MAC_MLD_TYPE_NONE,
	MAC_MLD_TYPE_SINGLE,
	MAC_MLD_TYPE_MULTI,
};

/**
 * struct mtk_if_cfg - interface configuration
 *
 * The config information from wlan driver, and used by hwifi driver.
 *
 * @if_type: Indicate interface type see &enum WDEV_TYPE
 * @mld_type: Indicate MLD type assigned from BSS/MLD manager
 *     for hwifi driver to manage mld hw resource.
 * @mld_group_idx: Indicate MLD group idx assgin from BSS/MLD manager
 *     hwifi driver should use this group id to allocate hw resource.
 * @if_addr: MAC address of Interface
 * @mld_addr: MAC address of the MLD that interface affiliate with
 */
struct mtk_if_cfg {
	u32 if_type;
	u32 mld_type;
	u32 mld_group_idx;
	u8 if_addr[MAC_ADDR_LEN];
	u8 mld_addr[MAC_ADDR_LEN];
};

/**
 * struct mtk_mac_bss - bss information and state
 *
 * @omac_idx: Indicate bss omac idx is acquired from hwifi driver
 *     for wlan driver to make bssinfo cmd.
 * @bss_idx: Indicate bss idx on fw is acquired from hwifi driver
 *     for wlan driver to make bssinfo cmd.
 * @mld_addr_idx: Indicate MLD address table idx in hardware
 *     is acquired from hwifi driver to make bssinfo cmd (MLD TAG).
 * @mld_remap_idx: Indicate MLD remap table idx in hardware
 *     is acquired from hwifi driver (per mld group base) to make
 *     bssinfo cmd.
 * @wmm_idx: Indicate hardware wmm idx for this bss is acquired from
 *     hwifi driver to make bssinfo cmd.
 * @band_idx: Indicate hardware band idx for this bss is acquired from
 *     hwifi driver to make bssinfo cmd.
 * @if_cfg: Indicate interface config, value is assigned by wlan driver
 * @bmc_sta: Pointer to a station entry for bmc packet on this bss.
 * @mac_phy: Pointer to a 80211 phy entry to store phy information and state.
 * @drv_priv: Pointer to the hardware area for this bss.
 */
struct mtk_mac_bss {
	u32 omac_idx;
	u32 bss_idx;
	u32 mld_addr_idx;
	u32 mld_group_addr_idx;
	u32 mld_remap_idx;
	u8 wmm_idx;
	u8 band_idx;
	struct mtk_if_cfg if_cfg;
	struct mtk_mac_sta *bmc_sta;
	struct mtk_mac_phy *mac_phy;
	u8 drv_priv[0] __aligned(sizeof(void *));
};

/**
 *
 * struct mtk_mac_bss_conf - bss configure information
 *
 * @band_idx: Indicate new band_idx wlan driver want to update
 */
struct mtk_mac_bss_conf {
	/*share data*/

	/*separate data*/
	union {
		u32 band_idx;
	} u;
};

/**
 * enum MTK_MAC_BSS_CHANGE_ACTION - bss change action
 *
 * @MAC_BSS_CHANGE_BAND: request change band action for
 *  auto-band feature
 * @MAC_BSS_CHANGE_MAX: Indicate max number of action
 */
enum MTK_MAC_BSS_CHANGE_ACTION {
	MAC_BSS_CHANGE_BAND,
	MAC_BSS_CHANGE_MAX,
};

/**
 * enum MTK_MAC_MCU_ACTION - mcu action type
 *
 * @MAC_MCU_ACT_NA: Indicate this command firmware should don't care?
 * @MAC_MCU_ACT_SET: Indicate this command is set cmd
 * @MAC_MCU_ACT_QUERY: Indicate this command is query cmd and
 *        firmware should response a result.
 */
enum MTK_MAC_MCU_ACTION {
	MAC_MCU_ACT_NA,
	MAC_MCU_ACT_SET,
	MAC_MCU_ACT_QUERY,
};

/**
 * enum MTK_MAC_MCU_DEST - mcu destination type
 *
 * @MAC_MCU_DEST_WM: Cmd destination to WM CPU
 * @MAC_MCU_DEST_WA: Cmd destination to WA CPU
 */
enum MTK_MAC_MCU_DEST {
	MAC_MCU_DEST_WM = 0,
	MAC_MCU_DEST_RVD = 1,
	MAC_MCU_DEST_WA = 2,
	MAC_MCU_DEST_WA_WM = 3,
	MAC_MCU_DEST_WO = 4,
	MAC_MCU_DEST_ALL = 5,
};

/**
 * struct mtk_mac_mcu_msg - cmd hardware information
 *
 * @cmd: Cmd idx.
 * @ext_cmd: Externl cmd idx.
 * @data: Point to cmd payload buffer head.
 * @rx_data: Point to cmd response buffer head for rx response.
 * @len: Indicate buffer size (in bytes) ready for tx.
 * @rx_len: Indicate max rx response size (in bytes) ready for rx response.
 * @is_wait: Indicate if driver need to handle wait rsp.
 * @dest: Indicate this cmd target destination see &enum MTK_MAC_MCU_DEST.
 * @action: Indicate this cmd type see &enum MTK_MAC_MCU_ACTION.
 * @ack: inform fw to ACK this cmd
 * @uni_cmd: uni_cmd or not
 */
struct mtk_mac_mcu_msg {
	int cmd;
	int ext_cmd;
	void *data;
	void *rx_data;
	u32 len;
	u32 rx_len;
	bool is_wait;
	u8 dest;
	u8 action;
	bool ack;
	bool uni_cmd;
	u8 frag_num;
	u8 frag_total_num;
	int timeout;
};

/**
 * struct mtk_mac_hw_ops - callbacks from wlan driver to hwifi driver
 *
 * This structure contains various callbacks that hwifi driver must
 * handle.
 *
 * @add_interface: Called when an interface attached to hardware is
 *     enabled (interface up) then hwifi should acquire some hw resource
 *     for interface.
 *
 * @remove_interface: Notifies hwifi driver that an interface is going
 *     down then hwifi can release related hw resource and stop tx for
 *     this interface.
 *
 * @add_mld: Called when a MLD group is created and notify hwifi to
 *     acquire hw resource for this MLD group.
 *
 * @remove_mld: Called when a MLD group is destroyed and notify hwifi to
 *     release hw resource for this MLD group.
 *
 * @mld_add_link: Called when a link (AP/STA itself) requests to join
 *     a MLD group and notifies hwifi to acquire hw resource for this
 *     link.
 *
 * @mld_remove_link: Called when a link (AP/STA itself) requests to leave
 *     a MLD group and notifies hwifi to release hw resource for this
 *     link.
 *
 * @add_sta: Notifies hwifi driver about add an associated station, AP/STA
 *     peer etc.
 *
 * @remove_sta: Notifies hwifi driver about removeal of an associated station,
 *     AP/STA peer etc. Note that after the callback returns STA should stop tx.
 *
 * @start: Called before the first interface attached to hardware is enabled.
 *     This should turn on the hardware and mcu cmd/event tx/rx.
 *
 * @stop: Called after last interface is disabled. This should turn off the
 *     hardware and disable mcu cmd/event tx/rx.
 *
 * @mcu_tx: Called when wlan driver transmitted a cmd to hardware mcu. This
 *     handler should response 0 as success or native value when transmitted
 *     is failed, after transmited wlan driver should free the cmd buffer.
 *
 * @tx_queue: Called when wlan driver transmitted a set of MSDU frames (including AMSDU).
 *     This handler should be atomic.
 *
 * @config_phy: Notifies hwifi driver about any phy changes.
 *
 * @bus_io_read: Called when wlan driver need to read CR directly. It's not recommend
 *     to do it in wlan driver.
 *
 * @bus_io_write: Called when wlan driver need to write CR directly. It's not recommend
 *     to do it in wlan driver.
 *
 * @change_bss: Callded when wlan driver update bss configuration
 *
 * @ser_handler: Called when wlan driver enable/disable CR access, write INT to MCU or
 *		 or init/exit DMA and token.
 *
 * @get_max_uwtbl_num: Get max uwtbl number ([inlcude] bcast group)
 *
 * @set_max_uwtbl_sta_num: Set max uwtbl number ([inlcude] bcast group)
 *
 * @get_inf_num: Get active interface number of physical device
 *
 * @set_pao_sta_info: Set pao info. to wed HW.
 *
 * @set_pn_check: Set pn check enable or disable.
 *
 * @get_tx_token_num: Get tx token number from hwifi.
 * @get_free_sta_pool_num: Check remaining free sta number from sta_mgmt of wsys.
 *
 * @get_wtbl_idrm_range_num: get wtbl/group's idrm low/high value
 */
struct mtk_mac_hw_ops {
	int (*add_interface)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss);
	int (*remove_interface)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss);
	int (*add_mld)(struct mtk_mac_hw *hw, u32 mld_type, u32 mld_group_idx, u8 *mld_addr);
	int (*remove_mld)(struct mtk_mac_hw *hw, u32 mld_group_idx);
	int (*mld_add_link)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss, u32 mld_group_idx);
	int (*mld_remove_link)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss);
	int (*add_sta)(struct mtk_mac_hw *hw, struct mtk_mac_bss *bss, struct mtk_mac_sta *sta);
	int (*remove_sta)(struct mtk_mac_hw *hw, struct mtk_mac_bss *bss, struct mtk_mac_sta *sta);
	int (*start)(struct mtk_mac_hw *hw);
	int (*stop)(struct mtk_mac_hw *hw);
	int (*mcu_tx)(struct mtk_mac_hw *hw, struct mtk_mac_mcu_msg *mcu_msg);
	int (*tx_check_resource)(struct mtk_mac_hw *hw, struct mtk_mac_txq *txq,
			enum TX_RESOURCE_TYPE tx_frame_type);
	int (*tx_queue)(struct mtk_mac_hw *hw, struct mtk_mac_txq *txq,
					struct _TX_BLK *tx_blk);
	int (*tx_kick)(struct mtk_mac_hw *hw);
	int (*config_phy)(struct mtk_mac_hw *hw, struct mtk_mac_phy *mac_phy);
	int (*bus_io_read)(struct mtk_mac_hw *hw, u32 addr, u32 *val);
	int (*bus_io_write)(struct mtk_mac_hw *hw, u32 addr, u32 val);
	int (*change_bss)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss,
			struct mtk_mac_bss_conf *info, u32 change);
	int (*ser_handler)(struct mtk_mac_hw *hw, u32 action, u32 status);
	int (*get_dbg_info)(struct mtk_mac_hw *hw, char *arg);
	u16 (*get_max_uwtbl_num)(struct mtk_mac_hw *hw, bool all);
	int (*set_max_uwtbl_sta_num)(struct mtk_mac_hw *hw, u16 max_sta_num, u16 max_group_num);
	u16 (*get_inf_num)(struct mtk_mac_hw *hw);
	struct mtk_mac_hw *(*get_mac_hw)(struct mtk_mac_hw *hw, u8 bss_idx);
	void (*get_fw_info)(struct mtk_mac_hw *hw, int mcu_type, char *fw_ver, char *build_date, char *fw_ver_long);
	void (*set_fw_mode)(struct mtk_mac_hw *hw, int mcu_type, unsigned char fw_mode);
	int (*get_mld_id)(struct mtk_mac_hw *hw, struct mtk_mac_sta *sta);
	bool (*init_rro_addr_elem_by_seid)(struct mtk_mac_hw *hw, u16 seid);
	int (*set_pao_sta_info)(struct mtk_mac_hw *hw, u16 wcid, u8 max_amsdu_nums,
				u32 max_amsdu_len, int remove_vlan, int hdrt_mode);
	int (*set_pn_check)(struct mtk_mac_hw *hw, u16 wcid, u16 se_id, bool enable);
	int (*set_particular_to_host)(struct mtk_mac_hw *hw, bool enable);
	int (*get_tx_token_num)(struct mtk_mac_hw *hw, u16 tx_token_num[], u8 max_src_num);
	u32 (*get_free_sta_pool_num)(struct mtk_mac_hw *hw);
	void (*get_wtbl_idrm_range_num)(struct mtk_mac_hw *hw, u16 *low, u16 *high, u8 wtbl_type);
	int (*change_setup_link_sta)(struct mtk_mac_hw *hw, struct mtk_mac_sta *sta);
	int (*get_rro_sp_page_num)(struct mtk_mac_hw *hw, u32 *page_num);
};

/**
 * struct mtk_mac_dev - mac device information and state.
 *
 * This device information is used for connecting hwifi driver to indicate a phy
 * mac_phy is only used in wlan driver not map to hwifi driver
 *
 * @ops: Callbacks for hardware related resource allocate and control
 *    see &struct mtk_mac_hw_ops.
 * @ad: Point to wlan driver adpater.
 * @mac_phy: Physical location for store phy information and state see &struct mac_phy
 * @hw: Hardware information and state see struct mtk_mac_hw
 *    (must put the end of structrue).
 */
struct mtk_mac_dev {
	struct mtk_mac_hw_ops *ops;
	struct _RTMP_ADAPTER *ad;
	struct mtk_mac_phy mac_phy;
	struct mtk_mac_hw hw;
};

/**
 * mtk_mac_alloc_hw - Allocate a new hardware device
 *
 * This must be called once for each hardware device. The returned pointer
 * must be used to refer to this device when calling other functions.
 * wlan driver allocate a private data area for hwifi driver pointed to by
 * @priv in &struct mtk_mac_hw, the size of this area is given as
 * @size.
 *
 * @size: Length of private data
 * @ops: callbacks for this device
 *
 * Return: A pointer to the new hardware device, or %NULL on error.
 */
struct mtk_mac_hw *
mtk_mac_alloc_hw(size_t size, struct mtk_mac_hw_ops *ops);

/**
 * mtk_mac_free_hw - Free hardware description
 *
 * This function frees everything that was allocateed, including the
 * private data for the driver. You must call mtk_mac_hw_unregister()
 * before calling this funciton.
 *
 * @hw: the hardware to free
 */
void
mtk_mac_free_hw(struct mtk_mac_hw *hw);

/**
 * mtk_mac_hw_register - Register hardware device
 *
 * You must all this function before any other functions in
 * wlan driver to run hwifi mode. Note that before a hardware
 * can be registered, you need to fill the contained mtk_mac_dev's
 * information.
 *
 * @hw: the device to register as returned by mtk_mac_alloc_hw()
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mac_hw_register(void *hw_dev, struct mtk_mac_hw *hw);

/**
 * mtk_mac_hw_unregister - Unregister a hardware device
 *
 * This function instructs wlan driver to free allocated resources
 * and unregister netdevices from the networking subsystem.
 *
 * @hw: the hardware to unregister
 */
void
mtk_mac_hw_unregister(struct mtk_mac_hw *hw);

/**
 * mtk_mac_rx_napi - receive frame from NAPI context
 *
 * Use this function to hand received frames to wlan driver.
 * This function may not be called in IRQ context. Calls to
 * this function for a single hardware must be synchronized
 * agginst each other.
 *
 * Tis function must be called with BHs disabled.
 *
 * @hw: The hardware this frame came in on.
 * @sta: The station this frame came from.
 * @skb: The buffer to receive, owned by wlan driver after this call.
 * @napi: the NAPI context.
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mac_rx_napi(struct mtk_mac_hw *hw, struct mtk_mac_sta *sta, struct sk_buff *skb);

int
mtk_mac_rx(struct mtk_mac_hw *hw, struct mtk_mac_sta *sta, struct sk_buff *skb, struct _RX_BLK *rx_blk);

int
mtk_mac_rx_ics(struct mtk_mac_hw *hw, struct sk_buff *skb);

int
mtk_mac_tx_status(struct mtk_mac_hw *hw, struct mtk_tx_status *tx_status);
void mtk_mac_dequeue_by_token(struct mtk_mac_hw *hw, u8 idx);
int mtk_mac_chip_reset(u32 chip_id);

int
mtk_mac_txs_handler(void *hw_dev, struct txs_info_t txs_info);

/**
 * mtk_mac_rx_unsolicited_event - receive a unsolicited event from NAPI context
 *
 * Use this function to hand received event to wlan driver
 * This function may not be called in IRQ context. Calls to
 * this funciton for a single hardware must be synchronized
 * agginst each other.
 *
 * This function must be called with BHs disabled.
 *
 * @hw: The hardware this event came in on.
 * @skb: The buffer to receive, owned by wlan driver after this call.
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mac_rx_unsolicited_event(struct mtk_mac_hw *hw, struct sk_buff *skb);

int
mtk_mac_rx_uni_unsolicited_event(void *physical_dev, struct sk_buff *skb);

/**
 * mtk_rx_ser_event - receive a ser event from NAPI context
 *
 * Use this function to hand received ser event to ser manager to
 * do ser
 *
 *
 * @chip_id: chip id
 * @ser_level: ser level 1/0.5/0
 * @ser_event: ser event
 * @ hw_id: hw_id from hwifi
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_rx_ser_event(u32 chip_id, u32 ser_level, u32 ser_event, u32 hw_id);

static inline u8 mtk_add_main_device(void *hw_dev, u32 chip_id, unsigned long hw_flags, void *mac_cap_info)
{
	return physical_device_add_main_device(hw_dev, chip_id, hw_flags, mac_cap_info);
}

static inline void *mtk_find_physical_device(void *hw_dev)
{
	return physical_device_find_physical_dev(hw_dev);
}

#endif
