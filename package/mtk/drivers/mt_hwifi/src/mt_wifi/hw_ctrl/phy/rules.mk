HW_CTRL_PHY_PATH := $(PARENT_DIR)/hw_ctrl/phy

obj_hw_ctrl += \
	$(HW_CTRL_PHY_PATH)/txpwr/txpwr.o\
	$(HW_CTRL_PHY_PATH)/phystate/phystate.o\
	$(HW_CTRL_PHY_PATH)/rate_ctrl/ra_cfg.o\
	$(HW_CTRL_PHY_PATH)/rate_ctrl/ra_ctrl_mt.o\
	$(HW_CTRL_PHY_PATH)/rate_ctrl/ra_ctrl_mt_drv.o\
	$(HW_CTRL_PHY_PATH)/rate_ctrl/ra_wrapper_embedded.o\
	$(HW_CTRL_PHY_PATH)/mt_phy.o\
	$(HW_CTRL_PHY_PATH)/phy.o

ifeq ($(CONFIG_SINGLE_SKU),y)
obj_hw_ctrl += \
	$(HW_CTRL_PHY_PATH)/txpwr/single_sku.o
endif
