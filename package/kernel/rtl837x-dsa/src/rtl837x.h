/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Realtek RTL8372/RTL8372N/RTL8373 DSA Switch Driver for Linux 6.6+
 *
 * Synthesizes airjinkela/rtl837x-dsa-driver with logicog/RTLPlayground
 * hardware register sequences and restores full Linux 6.6.x compatibility.
 */
#ifndef _RTL837X_DSA_H_
#define _RTL837X_DSA_H_

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/if_bridge.h>
#include <linux/if_vlan.h>
#include <linux/leds.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/phylink.h>
#include <linux/regmap.h>
#include <linux/rtnetlink.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <net/dsa.h>
#include <linux/dsa/8021q.h>
#include "tag.h"
#include "tag_8021q.h"

/* Linux 6.6 <-> 6.12/6.18 DSA & GPIO API compatibility wrappers */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 7, 0)
static inline struct net_device *rtl837x_dsa_cpu_master(const struct dsa_port *dp)
{
	return dp->master;
}
static inline struct net_device *rtl837x_dsa_port_user_dev(const struct dsa_port *dp)
{
	return dp->slave;
}
#else
static inline struct net_device *rtl837x_dsa_cpu_master(const struct dsa_port *dp)
{
	return dp->conduit;
}
static inline struct net_device *rtl837x_dsa_port_user_dev(const struct dsa_port *dp)
{
	return dp->user;
}
#endif

/*
 * Dedicated 802.1Q DSA tag protocol ID for Linux 6.6 MediaTek PPE VLAN offload.
 * ID 30 is already assigned to DSA_TAG_PROTO_MXL862_8021Q_VALUE in net/dsa.h.
 */
#define DSA_TAG_PROTO_RTL837X_8021Q_VAL		31
#define DSA_TAG_PROTO_RTL837X_8021Q		((enum dsa_tag_protocol)DSA_TAG_PROTO_RTL837X_8021Q_VAL)
#define RTL837X_TAG_8021Q_NAME			"rtl837x-8021q"

/* Clause-22 indirect SMI host registers at switch PHY address (0x1d) */
#define RTL837X_MDIO_CTRL_REG			21
#define RTL837X_MDIO_ADDRESS_REG		22
#define RTL837X_MDIO_DATA_LOW_REG		23
#define RTL837X_MDIO_DATA_HIGH_REG		24
#define RTL837X_MDIO_CMD_BUSY			BIT(2)
#define RTL837X_MDIO_WRITE_CMD			0x0019
#define RTL837X_MDIO_READ_CMD			0x001b

/* Chip ID, reset & global registers (logicog/RTLPlayground rtl837x_regs.h) */
#define RTL837X_REG_CHIP_ID			0x0004
#define RTL837X_CHIP_ID_MODEL_MASK		GENMASK(31, 16)
#define RTL837X_CHIP_ID_8372			0x8372
#define RTL837X_CHIP_ID_8373			0x8373
#define RTL837X_REG_CHIP_INFO			0x000c
#define RTL837X_CHIP_SUB_N_MASK			GENMASK(15, 8)
#define RTL837X_CHIP_SUB_IS_N			0x70
#define RTL837X_REG_RESET			0x0024
#define RTL837X_RESET_SOC_BIT			BIT(0)
#define RTL837X_RESET_NIC_BIT			BIT(2)

/* SerDes indirect access & mode registers */
#define RTL837X_SDS_INDACS_CMD_REG		0x03f8
#define RTL837X_SDS_INDACS_TRIGGER		BIT(15)
#define RTL837X_SDS_INDACS_WR			BIT(14)
#define RTL837X_SDS_INDACS_REGAD		GENMASK(11, 7)
#define RTL837X_SDS_INDACS_PAGE			GENMASK(6, 1)
#define RTL837X_SDS_INDACS_INDEX		BIT(0)
#define RTL837X_SDS_INDACS_READ_DATA_REG	0x03fc
#define RTL837X_SDS_INDACS_WRITE_DATA_REG	0x0400

#define RTL837X_SDS_MODE_SEL_REG		0x7b20
#define RTL837X_SDS0_MODE_SEL_MASK		GENMASK(4, 0)
#define RTL837X_SDS1_MODE_SEL_MASK		GENMASK(9, 5)
#define RTL837X_SDS0_USX_SUB_MODE_MASK		GENMASK(14, 10)
#define RTL837X_SDS1_USX_SUB_MODE_MASK		GENMASK(20, 16)

/* SerDes internal pages & bits (RTLPlayground + rtl837x-dsa-driver) */
#define RTL837X_SDS_PAGE_SDS			0x00
#define RTL837X_SDS_PAGE_FIB			0x02
#define RTL837X_SDS_PAGE_TGR_PRO_1		0x05
#define RTL837X_SDS_PAGE_PCS_64B66B		0x06
#define RTL837X_SDS_PAGE_WDIG			0x1f
#define RTL837X_SDS_PAGE_ANA_COM		0x21
#define RTL837X_SDS_PAGE_ANA_10G		0x36
#define RTL837X_SDS_8B10B_RX_INV		BIT(9)
#define RTL837X_SDS_8B10B_TX_INV		BIT(8)
#define RTL837X_SDS_64B66B_RX_INV		BIT(13)
#define RTL837X_SDS_64B66B_TX_INV		BIT(14)

enum rtl837x_sds_mode {
	RTL837X_SDS_MODE_SGMII		= 0x02,
	RTL837X_SDS_MODE_1000BASEX	= 0x04,
	RTL837X_SDS_MODE_100FX		= 0x05,
	RTL837X_SDS_MODE_USXGMII	= 0x0d,
	RTL837X_SDS_MODE_HSGMII		= 0x12,
	RTL837X_SDS_MODE_2500BASEX	= 0x16,
	RTL837X_SDS_MODE_10GBASER	= 0x1a,
	RTL837X_SDS_MODE_OFF		= 0x1f,
};

/* MIB indirect registers */
#define RTL837X_MIB_CTRL_REG			0x0f60
#define RTL837X_MIB_BUSY_MASK			BIT(0)
#define RTL837X_MIB_PORT_IDX_MASK		GENMASK(6, 3)
#define RTL837X_MIB_ID_MASK			GENMASK(14, 7)
#define RTL837X_MIB_DATA_LO_REG			0x0f64
#define RTL837X_MIB_DATA_HI_REG			0x0f68

/* Per-port MAC L2 control & MTU registers (QinQ-aware) */
#define RTL837X_MAC_L2_GLOBAL_CTRL_REG		0x1200
#define RTL837X_RS_LAYER_CONFIG_REG		0x1208
#define RTL837X_RS_LINK_FAULT_INDI_OFF		BIT(4)
#define RTL837X_MAC_L2_PORT_CTRL_REG(p)		(0x1238 + ((p) << 8))
#define RTL837X_MAC_L2_PORT_8021Q_TAG_BIT	BIT(6)
#define RTL837X_MAC_L2_PORT_DFLT_8021Q_VAL	0x0f77
#define RTL837X_MAC_L2_PORT_DFLT_RTLTAG_VAL	0x0f37
#define RTL837X_MAC_MAX_LEN_REG(p)		(0x1250 + ((p) << 8))
#define RTL837X_MAC_MAX_LEN_LO_MASK		GENMASK(13, 0)
#define RTL837X_MAC_MAX_LEN_HI_MASK		GENMASK(27, 14)
#define RTL837X_MAC_MAX_LEN_INC_TAG		BIT(28)
#define RTL837X_MAC_MAX_LEN_MAX			0x3fff
#define RTL837X_TAG_QINQ_OVERHEAD		8

/* VLAN / SVLAN / MSTP / Port isolation registers */
#define RTL837X_VLAN_ACCEPT_FRAME_TYPE_REG	0x4e10
#define RTL837X_VLAN_CTRL_REG			0x4e14
#define RTL837X_VLAN_CTRL_TABLE_RST		BIT(0)
#define RTL837X_VLAN_CTRL_CVLAN_FILTER		BIT(2)
#define RTL837X_VLAN_PORT_IGR_FLTR_REG		0x4e18
#define RTL837X_VLAN_PVID_CTRL_REG(p)		(0x4e1c + (((p) >> 1) << 2))
#define RTL837X_VLAN_L2_LRN_DIS_REG		0x4e30
#define RTL837X_MSTI_CTRL_REG(fid)		(0x4f00 + ((fid) << 2))
#define RTL837X_PORT_ISOLATION_REG(p)		(0x5000 + ((p) << 2))

/* SVLAN registers used by tag_8021q hardware port tagging */
#define RTL837X_SVLAN_TRAP_CTRL_REG		0x4c00
#define RTL837X_VS_CTRL_REG			0x4c04
#define RTL837X_VS_GLB_CTRL_REG			0x4c08
#define RTL837X_VS_UPLINK_PORT_REG		0x4c0c
#define RTL837X_VS_PORT_DFLT_SVID_REG(p)	(0x4c10 + (((p) >> 1) << 2))

/* L2 LUT & hardware aging registers */
#define RTL837X_L2_CTRL_REG			0x5350
#define RTL837X_L2_AGING_TIME_MASK		GENMASK(20, 0)
#define RTL837X_LUT_PORT_LEARN_LIMIT_REG(p)	(0x5384 + ((p) << 2))
#define RTL837X_LEARN_LIMIT_MAX			2112
#define RTL837X_L2_FLUSH_CMD_REG		0x53d4
#define RTL837X_L2_FLUSH_PORT_MASK		GENMASK(9, 0)
#define RTL837X_L2_FLUSH_ACT			BIT(16)
#define RTL837X_L2_FLUSH_BUSY			BIT(17)
#define RTL837X_L2_FLUSH_CTRL_REG		0x53dc
#define RTL837X_L2_FLUSH_XID_REG		0x53e0
#define RTL837X_L2_FLUSH_ALL_REG		0x53e4

/* ITA indirect table access engine */
#define RTL837X_TABLE_CTRL_REG			0x5cac
#define RTL837X_TABLE_EXEC			BIT(0)
#define RTL837X_TABLE_OP_WRITE			BIT(1)
#define RTL837X_TABLE_TYPE_MASK			GENMASK(10, 8)
#define RTL837X_TABLE_ADDR_MASK			GENMASK(28, 16)
#define RTL837X_TABLE_L2_CTRL_REG		0x5cb0
#define RTL837X_TABLE_L2_HIT			BIT(8)
#define RTL837X_TABLE_L2_METHOD_NEXT_UC		0x3000
#define RTL837X_TABLE_WRITE_REG(i)		(0x5cb8 + ((i) << 2))
#define RTL837X_TABLE_READ_REG(i)		(0x5ccc + ((i) << 2))

enum rtl837x_ita_table {
	RTL837X_TBL_CVLAN	= 0x3,
	RTL837X_TBL_L2_UC	= 0x4,
	RTL837X_TBL_L2_MC	= 0x5,
};

/* CPU tag, MAC force & status, SMI, LED, and GPIO registers */
#define RTL837X_CPU_TAG_AWARE_CTRL_REG		0x603c
#define RTL837X_REG_HW_CONF			0x6040
#define RTL837X_MAC_LINK_STS_REG		0x6300
#define RTL837X_MAC_LINK_SPD_STS_REG(p)		(0x6308 + (((p) >> 3) << 2))
#define RTL837X_MAC_LINK_DUP_STS_REG		0x6310
#define RTL837X_MAC_TX_PAUSE_STS_REG		0x6314
#define RTL837X_MAC_RX_PAUSE_STS_REG		0x6318
#define RTL837X_MAC_PORT_EN_REG			0x632c
#define RTL837X_MAC_FORCE_MODE_REG(p)		(0x6344 + ((p) << 2))
#define RTL837X_MAC_FORCE_LINK_MASK		GENMASK(1, 0)

#define RTL837X_SMI_CTRL_REG			0x6400
#define RTL837X_SMI_PHY_CTRL_0_REG		0x6408
#define RTL837X_SMI_PHY_CTRL_1_REG		0x640c
#define RTL837X_SMI_PHY_CTRL_2_REG		0x6410
#define RTL837X_SMI_PHY_CTRL_3_REG		0x6414
#define RTL837X_SMI_PHY_TRIGGER			BIT(0)
#define RTL837X_SMI_PHY_WRITE			BIT(1)
#define RTL837X_SMI_PHY_C45			BIT(2)
#define RTL837X_SMI_PHY_FAIL			BIT(25)

#define RTL837X_REG_LED_MODE			0x6520
#define RTL837X_REG_LED1_0_SET0			0x6548
#define RTL837X_LED_PORT_SET_SEL_REG		0x654c
#define RTL837X_REG_LED_GLB_ACTIVE		0x65d8
#define RTL837X_REG_LED_GLB_IO_EN		0x65dc
/* RTLPlayground verified fix: bit 6 must be set for solid link/speed LED drive */
#define RTL837X_LED_GLB_IO_SOLID_EN		BIT(6)
#define RTL837X_REG_LED_GLB_MUX(i)		(0x65e0 + ((i) << 2))
#define RTL837X_REG_LED_RLDP_1			0x65f8
#define RTL837X_REG_LED_RLDP_2			0x65fc
#define RTL837X_REG_LED_RLDP_3			0x6600

#define RTL837X_CPU_CTRL_REG			0x6720
#define RTL837X_CPU_CTRL_EN			BIT(0)
#define RTL837X_EXT_CPU_CTRL_REG		0x6724
#define RTL837X_VLAN_EGRESS_KEEP_REG(p)		(0x6728 + (((p) / 3) << 2))
#define RTL837X_VLAN_EGRESS_TAG_MODE_REG	0x6738

#define RTL837X_PORT_FLOWCTRL_REG(p)		(0x7124 + ((p) << 2))
#define RTL837X_PORT_EEE_CTRL_REG(p)		(0x7200 + ((p) << 2))

#define RTL837X_GPIO_DIR_0_REG			0x7f70
#define RTL837X_GPIO_OUT_0_REG			0x7f78
#define RTL837X_GPIO_IN_0_REG			0x7f80
#define RTL837X_IO_MUX_SEL_0_REG		0x7f8c

#define RTL837X_MAX_PORTS			9
#define RTL837X_FID_MAX				15
#define RTL837X_GPIO_MAX			28

struct rtl837x_priv;

struct rtl837x_port_priv {
	struct rtl837x_priv *priv;
	struct phylink_pcs pcs;
	int index;
	bool is_serdes;
	u64 rx_pkts;
	u64 tx_pkts;
	u64 rx_bytes;
	u64 tx_bytes;
	u64 rx_drops;
	u64 tx_drops;
};

struct rtl837x_vlan_entry {
	u16 vid;
	u16 member;
	u16 untag;
	u8 fid;
	bool ivl_en;
};

struct rtl837x_priv {
	struct device *dev;
	struct mii_bus *bus;
	struct mii_bus *slave_mii_bus;
	int mdio_addr;
	struct regmap *map;
	struct regmap *map_nolock;
	struct mutex map_lock;
	spinlock_t stats_lock;
	struct gpio_desc *reset_gpio;
	struct gpio_chip gpio_chip;
	bool gpio_registered;
	struct dsa_switch *ds;
	enum dsa_tag_protocol tag_proto;
	u16 chip_model;
	bool is_n_series;
	u8 num_ports;
	u8 cpu_port;
	u16 bridge_mask;
	bool sds_rx_inv[2];
	bool sds_tx_inv[2];
	bool phy_mdi_reverse;
	bool phy_tx_pol_swap;
	bool dsa_8021q_vid[VLAN_N_VID];
	netdev_features_t csum_backup;
	bool csum_overridden;
	struct delayed_work stats_work;
	bool stopping;
	struct rtl837x_port_priv ports[RTL837X_MAX_PORTS];
};

/* tag_rtl837x_8021q.c */
int rtl837x_tag_8021q_driver_register(void);
void rtl837x_tag_8021q_driver_unregister(void);

/* rtl837x_mdio_reg.c */
int rtl837x_ita_exec(struct rtl837x_priv *priv, enum rtl837x_ita_table tbl,
		     bool write, u16 addr, u32 data_in[3], u32 data_out[3]);
int rtl837x_sds_reg_read(struct rtl837x_priv *priv, u8 sds, u8 page, u8 reg,
			 u16 *val);
int rtl837x_sds_reg_write(struct rtl837x_priv *priv, u8 sds, u8 page, u8 reg,
			  u16 val);
int rtl837x_sds_reg_modify(struct rtl837x_priv *priv, u8 sds, u8 page, u8 reg,
			   u16 mask, u16 set);
int rtl837x_phy_read_c45_locked(struct rtl837x_priv *priv, int port, int devad,
				int regnum);
int rtl837x_phy_write_c45_locked(struct rtl837x_priv *priv, int port, int devad,
				 int regnum, u16 val);
int rtl837x_setup_internal_mdio(struct rtl837x_priv *priv);

/* rtl837x_serdes.c */
extern const struct phylink_pcs_ops rtl837x_sds_pcs_ops;
int rtl837x_serdes_port_to_sds(int port);
int rtl837x_serdes_init_polarity(struct rtl837x_priv *priv);
int rtl837x_serdes_config_mode(struct rtl837x_priv *priv, u8 sds,
			       phy_interface_t interface, unsigned int neg_mode);

/* rtl837x_vlan_l2.c */
int rtl837x_vlan_read(struct rtl837x_priv *priv, u16 vid,
		      struct rtl837x_vlan_entry *e);
int rtl837x_vlan_write(struct rtl837x_priv *priv,
		       const struct rtl837x_vlan_entry *e);
int rtl837x_vlan_set_pvid(struct rtl837x_priv *priv, int port, u16 pvid);
int rtl837x_lut_flush_port(struct rtl837x_priv *priv, int port);
int rtl837x_lut_flush_all(struct rtl837x_priv *priv);
int rtl837x_lut_set_ageing_ms(struct rtl837x_priv *priv, unsigned int msec);
int rtl837x_fdb_modify(struct rtl837x_priv *priv, int port,
		       const unsigned char *addr, u16 vid, bool add);
int rtl837x_fdb_dump_port(struct rtl837x_priv *priv, int port,
			  dsa_fdb_dump_cb_t *cb, void *data);
int rtl837x_mdb_modify(struct rtl837x_priv *priv, int port,
		       const unsigned char *addr, u16 vid, bool add);
void rtl837x_stp_state_set(struct rtl837x_priv *priv, int port, u8 state);

/* rtl837x_led_gpio.c */
int rtl837x_gpio_and_led_init(struct rtl837x_priv *priv);

/* rtl8372n_dsa_main.c */
int rtl8372n_switch_detect_and_register(struct rtl837x_priv *priv);
void rtl8372n_switch_teardown_and_unregister(struct rtl837x_priv *priv);

#endif /* _RTL837X_DSA_H_ */
