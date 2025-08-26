#
# Makefile for Airoha AN8855 gigabit switch
#

obj-$(CONFIG_AN8855_GSW)	+= an8855.o

an8855-$(CONFIG_SWCONFIG)	+= an8855_swconfig.o

an8855-y			+= an8855_mdio.o an8855.o \
					an8855_common.o an8855_vlan.o an8855_nl.o
