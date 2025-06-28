#define VID_RULES 2
#define MAX_VLANS  100
#define MAX_PORTS 13
#define MAX_BRIDGES 16

struct mxl862xx_hw_info {
	u8 max_ports;
	u8 phy_ports;
	u8 cpu_port;
};

struct mxl862xx_filter_ids {
	s16 filters_idx[VID_RULES];
	bool valid;
};

struct mxl862xx_vlan {
	bool used;
	u16 vid;
	/* indexes of filtering rules(entries) used for this VLAN */
	s16 filters_idx[VID_RULES];
	/* Indicates if tags are added for eggress direction. Makes sense only in egress block */
	bool untagged;
};

struct mxl862xx_extended_vlan_block_info {
	bool allocated;
	/* current index of the VID related filtering rules  */
	u16 vid_filters_idx;
	/* current index of the final filtering rules;
	 * counted backwards starting from the block end
	 */
	u16 final_filters_idx;
	/* number of allocated  entries for filtering rules */
	u16 filters_max;
	/* number of entries per vlan */
	u16 entries_per_vlan;
	u16 block_id;
	/* use this for storing indexes of vlan entries
	 * for VLAN delete
	 */
	struct mxl862xx_vlan vlans[MAX_VLANS];
	/* collect released filter entries (pairs) that can be reused */
	struct mxl862xx_filter_ids filter_entries_recycled[MAX_VLANS];
};

struct mxl862xx_port_vlan_info {
	u16 pvid;
	bool filtering;
	/* Allow one-time initial vlan_filtering port attribute configuration. */
	bool filtering_mode_locked;
	/* Only one block can be assigned per port and direction. Take care about releasing
	 * the previous one when overwriting with the new one
	 */
	struct mxl862xx_extended_vlan_block_info ingress_vlan_block_info;
	struct mxl862xx_extended_vlan_block_info egress_vlan_block_info;
};

struct mxl862xx_port_info {
	bool port_enabled;
	bool isolated;
	u16 bridge_id;
	u16 bridge_port_cpu;
	struct net_device *bridge;
	enum dsa_tag_protocol tag_protocol;
	bool ingress_mirror_enabled;
	bool egress_mirror_enabled;
	struct mxl862xx_port_vlan_info vlan;
};

struct mxl862xx_priv {
	struct dsa_switch *ds;
	struct mii_bus *bus;
	struct device *dev;
	int sw_addr;
	const struct mxl862xx_hw_info *hw_info;
	struct mxl862xx_port_info port_info[MAX_PORTS];
	u16 bridge_portmap[MAX_BRIDGES];
	/* Number of simultaneously supported vlans (calculated in the runtime) */
	u16 max_vlans;
	/* pce_table_lock required for kernel 5.16 or later,
	 * since rtnl_lock has been dropped from DSA.port_fdb_{add,del}
	 * might cause dead-locks / hang in previous versions
	 */
	struct mutex pce_table_lock;
};
