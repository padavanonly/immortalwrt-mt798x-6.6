ccflags-y += $(MODULE_FLAGS) -Inet/nat -I$(PKG_BUILD_DIR)../warp -I$(PKG_BUILD_DIR)../warp/mcu -DCONFIG_WARP_VERSION=$(CONFIG_WARP_VERSION)

ifeq ($(CONFIG_WARP_DBG_SUPPORT), y)
ccflags-y += -DCONFIG_WARP_DBG_SUPPORT
endif

ifeq ($(CONFIG_MTK_MEMORY_SHRINK), y)
ccflags-y += -DMEMORY_SHRINK
endif

ifeq ($(CONFIG_WARP_MEMORY_LEAK_DBG), y)
ccflags-y += -DMEM_LEAK_DBG
endif

ifeq ($(CONFIG_ARCH_DMA_ADDR_T_64BIT), y)
ccflags-y += -DCONFIG_WARP_64BIT_SUPPORT
endif

ccflags-y += -DWARP_CHIP_SET="\"$(CONFIG_WARP_CHIPSET)\""

ccflags-y += -Werror -Wall -I$(srctree)/drivers/misc/mediatek/include/

all: build_tools
build_tools:
	gcc -g tools/bin2h.c -o tools/bin2h
	tools/bin2h

MODULE_NAME=mtk_warp

OBJS:= warp_main.o
OBJS+= warp_utility.o
OBJS+= wed.o
OBJS+= warp_proc.o
OBJS+= warp_wifi.o
OBJS+= warp_bus.o
OBJS+= warp_ser.o
ifeq ($(CONFIG_MULDF_HELPER), y)
OBJS+= warp_dbg.o
endif

ifneq (, $(filter y, $(CONFIG_TARGET_mediatek) $(CONFIG_TARGET_gem6xxx)))
	OBJS+= warp_platform/warp_mtk.o
	ccflags-y += -DCONFIG_WARP_PLATFORM_MTK
endif

ifeq ($(CONFIG_MULDF_HELPER), y)
OBJS+= warp_muldf.o
endif

#HW
REG_DIR=regs
ifeq ($(CONFIG_WARP_VERSION), 1)
	OBJS+= $(REG_DIR)/reg_v1/warp_hw_v1.o
	OBJS+= wdma.o
	OBJS+= warp_tx_bm.o
	ccflags-y += -DCONFIG_WARP_V1

	ifeq ($(CONFIG_WARP_CHIPSET), mt7622)
		OBJS+= $(REG_DIR)/reg_v1/warp_mt7622.o
		OBJS+= $(REG_DIR)/reg_v1/warp_hw_mtk.o
	endif

else ifeq ($(CONFIG_WARP_VERSION), 2)
	OBJS+= $(REG_DIR)/reg_v2/warp_hw_v2.o
	OBJS+= $(REG_DIR)/reg_v2/warp_hw_ccif.o
	OBJS+= wdma.o
	OBJS+= warp_tx_bm.o
	OBJS+= warp_rx_bm.o
	ccflags-y += -DCONFIG_WARP_V2
	ccflags-y += -DCONFIG_WARP_HW_RRO_WO

	ifeq ($(CONFIG_WARP_CHIPSET), colgin)
		OBJS+= $(REG_DIR)/reg_v2/warp_mt6890.o
	endif

	ifeq ($(CONFIG_WARP_CHIPSET), mt7915)
		OBJS+= $(REG_DIR)/reg_v2/warp_mt7915.o
	endif

	ifeq ($(CONFIG_WARP_CHIPSET), mt7986)
		OBJS+= $(REG_DIR)/reg_v2/warp_mt7986.o
	endif

	ifeq ($(CONFIG_WARP_CHIPSET), mt7916)
		OBJS+= $(REG_DIR)/reg_v2/warp_mt7916.o
	endif

	ifeq ($(CONFIG_WARP_CHIPSET), mt7981)
		OBJS+= $(REG_DIR)/reg_v2/warp_mt7981.o
	endif

	ifeq ($(CONFIG_WARP_CHIPSET), mt6990)
		OBJS+= $(REG_DIR)/reg_v2/warp_mt6990.o
	endif

else ifeq ($(CONFIG_WARP_VERSION), 3)
	OBJS+= $(REG_DIR)/reg_v3/warp_hw.o
	OBJS+= $(REG_DIR)/reg_v3/warp_hw_ccif.o
	OBJS+= $(REG_DIR)/reg_v3/pao_hw.o
	OBJS+= $(REG_DIR)/reg_v3/txbm_hw_v2.o
	OBJS+= wdma.o
	OBJS+= warp_tx_bm_v2.o
	OBJS+= warp_rx_bm.o
	OBJS+= warp_rx_page_bm.o
	ccflags-y += -DCONFIG_WARP_V3
	ccflags-y += -DCONFIG_WARP_HW_RRO_WO

	ifeq ($(CONFIG_WARP_CHIPSET), mt7988)
		OBJS+= $(REG_DIR)/reg_v3/warp_mt7988.o
	endif

else ifeq ($(CONFIG_WARP_VERSION), 3_1)
	OBJS+= $(REG_DIR)/reg_v3_1/warp_hw.o
	OBJS+= $(REG_DIR)/reg_v3_1/pao_hw.o
	OBJS+= $(REG_DIR)/reg_v3_1/txbm_hw_v2.o
	OBJS+= wdma.o
	OBJS+= warp_tx_bm_v2.o
	OBJS+= warp_rx_bm.o
	OBJS+= warp_rx_page_bm.o
	ccflags-y += -DCONFIG_WARP_V3_1
	ccflags-y += -DCONFIG_WARP_NEW_DTS_FORMAT

	ifeq ($(CONFIG_WARP_CHIPSET), mt7987)
		OBJS+= $(REG_DIR)/reg_v3_1/warp_mt7987.o
	endif

else
	OBJS+= $(REG_DIR)/reg_v3_2/warp_hw.o
	OBJS+= $(REG_DIR)/reg_v3_2/pao_hw.o
	OBJS+= $(REG_DIR)/reg_v3_2/txbm_hw_v2.o
	#OBJS+= $(REG_DIR)/reg_v3_2/wdma_netsys3.o
	OBJS+= $(REG_DIR)/reg_v3_2/wdma_netsys6.o
	#OBJS+= wdma.o
	#OBJS+= warp_tx_bm_v2.o
	#OBJS+= warp_rx_bm.o
	OBJS+= warp_rx_page_bm.o
	OBJS+= warp_rx_tkid.o
	ccflags-y += -DCONFIG_WARP_V3_2
	ccflags-y += -DCONFIG_WARP_NEW_DTS_FORMAT

	ifeq ($(CONFIG_WED_HW_RRO_SUPPORT), y)
		ccflags-y += -DCONFIG_WARP_HW_RRO_WO
		OBJS+= $(REG_DIR)/reg_v3_2/warp_hw_ccif.o
	endif

	ifeq ($(CONFIG_WARP_CHIPSET), mt6988)
		OBJS+= $(REG_DIR)/reg_v3_2/warp_mt6988.o
	endif
endif

ifeq ($(CONFIG_WED_HW_RRO_SUPPORT), y)
	OBJS+= mcu/warp_msg.o
	OBJS+= mcu/warp_evt.o
	OBJS+= mcu/warp_ccif.o
	OBJS+= mcu/warp_cmd.o
	OBJS+= mcu/warp_fwdl.o
	OBJS+= mcu/warp_woif.o
	OBJS+= mcu/warp_wo.o
	OBJS+= mcu/warp_woctrl.o

	ifeq ($(CONFIG_WARP_WO_EMBEDDED_LOAD), y)
		ccflags-y += -DCONFIG_WARP_WO_EMBEDDED_LOAD
	endif
endif

#######################################
# For integrating to kernel source
######################################
obj-m += $(MODULE_NAME).o


$(MODULE_NAME)-objs += $(OBJS)

install:

clean:
	rm -rf *.o *.ko *.modules .*.cmd .*.o *.order $(MODULE_NAME).mod.c tools/bin2h
