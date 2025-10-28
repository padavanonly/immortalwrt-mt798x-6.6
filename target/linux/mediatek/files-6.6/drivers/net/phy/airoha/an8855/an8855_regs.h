/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023 Airoha Inc.
 * Author: Min Yao <min.yao@airoha.com>
 */

#ifndef _AN8855_REGS_H_
#define _AN8855_REGS_H_

#include <linux/bitops.h>

#define BITS(m, n)	 (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))

/* Values of Egress TAG Control */
#define ETAG_CTRL_UNTAG			0
#define ETAG_CTRL_TAG			2
#define ETAG_CTRL_SWAP			1
#define ETAG_CTRL_STACK			3

#define VTCR					0x10200600
#define VAWD0					0x10200604
#define VAWD1					0x10200608
#define VARD0					0x10200618
#define VARD1					0x1020061C

/* Fields of VTCR */
#define VTCR_BUSY				BIT(31)
#define VTCR_FUNC_S				12
#define VTCR_FUNC_M				0xf000
#define VTCR_VID_S				0
#define VTCR_VID_M				0xfff

/* Values of VTCR_FUNC */
#define VTCR_READ_VLAN_ENTRY	0
#define VTCR_WRITE_VLAN_ENTRY	1

/* VLAN entry fields */
#define IVL_MAC					BIT(5)
#define EG_CON					BIT(11)
#define VTAG_EN					BIT(10)
#define PORT_MEM_S				26
#define PORT_MEM_M				0xfc000000
#define VENTRY_VALID			BIT(0)
#define ETAG_M					0x3fff000
#define PORT_ETAG_S(p)			(((p) * 2) + 12)
#define PORT_ETAG_M				0x03

#define PORT_CTRL_BASE			0x10208000
#define PORT_CTRL_PORT_OFFSET	0x200
#define PORT_CTRL_REG(p, r)		(PORT_CTRL_BASE + \
					(p) * PORT_CTRL_PORT_OFFSET +  (r))
#define PCR(p)					PORT_CTRL_REG(p, 0x04)
#define PVC(p)					PORT_CTRL_REG(p, 0x10)
#define PORTMATRIX(p)			PORT_CTRL_REG(p, 0x44)
#define PVID(p)					PORT_CTRL_REG(p, 0x48)

#define GRP_PORT_VID_M			0xfff

/* Values of PORT_VLAN */
#define PORT_MATRIX_MODE		0
#define FALLBACK_MODE			1
#define CHECK_MODE				2
#define SECURITY_MODE			3

/* Fields of PVC */
#define STAG_VPID_S				16
#define STAG_VPID_M				0xffff0000
#define VLAN_ATTR_S				6
#define VLAN_ATTR_M				0xc0
#define PVC_STAG_REPLACE		BIT(11)
#define PVC_PORT_STAG			BIT(5)

/* Values of VLAN_ATTR */
#define VA_USER_PORT			0
#define VA_STACK_PORT			1
#define VA_TRANSLATION_PORT		2
#define VA_TRANSPARENT_PORT		3

#define PORT_MATRIX_M			((1 << AN8855_NUM_PORTS) - 1)

#define PORT_MAC_CTRL_BASE		0x10210000
#define PORT_MAC_CTRL_PORT_OFFSET	0x200
#define PORT_MAC_CTRL_REG(p, r)		(PORT_MAC_CTRL_BASE + \
					(p) * PORT_MAC_CTRL_PORT_OFFSET + (r))
#define PMCR(p)					PORT_MAC_CTRL_REG(p, 0x00)
#define PMSR(p)					PORT_MAC_CTRL_REG(p, 0x10)

#define GMACCR					(PORT_MAC_CTRL_BASE + 0x3e00)

#define MAX_RX_JUMBO_S			4
#define MAX_RX_JUMBO_M			0xf0
#define MAX_RX_PKT_LEN_S		0
#define MAX_RX_PKT_LEN_M		0x3

/* Values of MAX_RX_PKT_LEN */
#define RX_PKT_LEN_1518			0
#define RX_PKT_LEN_1536			1
#define RX_PKT_LEN_1522			2
#define RX_PKT_LEN_MAX_JUMBO	3

/* Fields of PMSR */
#define EEE1G_STS			BIT(7)
#define EEE100_STS			BIT(6)
#define RX_FC_STS			BIT(5)
#define TX_FC_STS			BIT(4)
#define MAC_SPD_STS_S		28
#define MAC_SPD_STS_M		0x70000000
#define MAC_DPX_STS			BIT(25)
#define MAC_LNK_STS			BIT(24)

/* Values of MAC_SPD_STS */
#define MAC_SPD_10			0
#define MAC_SPD_100			1
#define MAC_SPD_1000		2
#define MAC_SPD_2500		3

#define MIB_COUNTER_BASE			0x10214000
#define MIB_COUNTER_PORT_OFFSET		0x200
#define MIB_COUNTER_REG(p, r)		(MIB_COUNTER_BASE + \
					(p) * MIB_COUNTER_PORT_OFFSET + (r))
#define STATS_TDPC			0x00
#define STATS_TCRC			0x04
#define STATS_TUPC			0x08
#define STATS_TMPC			0x0C
#define STATS_TBPC			0x10
#define STATS_TCEC			0x14
#define STATS_TSCEC			0x18
#define STATS_TMCEC			0x1C
#define STATS_TDEC			0x20
#define STATS_TLCEC			0x24
#define STATS_TXCEC			0x28
#define STATS_TPPC			0x2C
#define STATS_TL64PC		0x30
#define STATS_TL65PC		0x34
#define STATS_TL128PC		0x38
#define STATS_TL256PC		0x3C
#define STATS_TL512PC		0x40
#define STATS_TL1024PC		0x44
#define STATS_TL1519PC		0x48
#define STATS_TOC			0x4C
#define STATS_TODPC			0x54
#define STATS_TOC2			0x58

#define STATS_RDPC			0x80
#define STATS_RFPC			0x84
#define STATS_RUPC			0x88
#define STATS_RMPC			0x8C
#define STATS_RBPC			0x90
#define STATS_RAEPC			0x94
#define STATS_RCEPC			0x98
#define STATS_RUSPC			0x9C
#define STATS_RFEPC			0xA0
#define STATS_ROSPC			0xA4
#define STATS_RJEPC			0xA8
#define STATS_RPPC			0xAC
#define STATS_RL64PC		0xB0
#define STATS_RL65PC		0xB4
#define STATS_RL128PC		0xB8
#define STATS_RL256PC		0xBC
#define STATS_RL512PC		0xC0
#define STATS_RL1024PC		0xC4
#define STATS_RL1519PC		0xC8
#define STATS_ROC			0xCC
#define STATS_RDPC_CTRL		0xD4
#define STATS_RDPC_ING		0xD8
#define STATS_RDPC_ARL		0xDC
#define STATS_RDPC_FC		0xE0
#define STATS_RDPC_WRED		0xE4
#define STATS_RDPC_MIR		0xE8
#define STATS_ROC2			0xEC
#define STATS_RSFSPC		0xF4
#define STATS_RSFTPC		0xF8
#define STATS_RXCDPC		0xFC

#define RG_CLK_CPU_ICG		0x10005034
#define MCU_ENABLE			BIT(3)

#define RG_TIMER_CTL		0x1000a100
#define WDOG_ENABLE			BIT(25)

#define SYS_CTRL			0x100050C0
#define SW_SYS_RST			BIT(31)

#define INT_MASK			0x100050F0
#define INT_SYS_BIT			BIT(15)

#define SYS_INT_EN			0x1021C010
#define SYS_INT_STS			0x1021C014
#define PHY_LC_INT(p)		BIT(p)

#define CKGCR				0x10213E1C
#define CKG_LNKDN_GLB_STOP	0x01
#define CKG_LNKDN_PORT_STOP	0x02

#define PKG_SEL				0x10000094
#define PAG_SEL_AN8855H		0x2
#endif /* _AN8855_REGS_H_ */
