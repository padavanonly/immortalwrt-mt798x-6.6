# SPDX-License-Identifier: GPL-2.0-only
ccflags-y += -I$(srctree)/net/dsa
obj-$(CONFIG_NET_DSA_RTL8372N) += rtl8372n_dsa.o
rtl8372n_dsa-y := \
	tag_rtl837x_8021q.o \
	rtl837x_mdio_reg.o \
	rtl837x_serdes.o \
	rtl837x_vlan_l2.o \
	rtl837x_led_gpio.o \
	rtl8372n_dsa_main.o
