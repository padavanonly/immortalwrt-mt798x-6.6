HW_CTRL_PATH := $(PARENT_DIR)/hw_ctrl

obj_hw_ctrl += \
	$(HW_CTRL_PATH)/coex.o\
	$(HW_CTRL_PATH)/hw_ctrl_basic.o\
	$(HW_CTRL_PATH)/cmm_asic.o\
	$(HW_CTRL_PATH)/greenap.o\
	$(HW_CTRL_PATH)/hdev_ctrl.o\
	$(HW_CTRL_PATH)/hw_init.o\
	$(HW_CTRL_PATH)/cmm_asic_mt_fmac.o\
	$(HW_CTRL_PATH)/ee_flash.o\
	$(HW_CTRL_PATH)/hw_ctrl_ops_v1.o\
	$(HW_CTRL_PATH)/ee_prom.o\
	$(HW_CTRL_PATH)/ee_i2cprom.o\
	$(HW_CTRL_PATH)/eeprom.o\
	$(HW_CTRL_PATH)/pp_cmd.o\
	$(HW_CTRL_PATH)/hw_ctrl_ops_v2.o\
	$(HW_CTRL_PATH)/sr_cmd.o\
	$(HW_CTRL_PATH)/hw_ctrl_cmd.o\
	$(HW_CTRL_PATH)/ee_efuse.o\
	$(HW_CTRL_PATH)/cmm_chip.o\
	$(HW_CTRL_PATH)/cmm_asic_mt_fw.o\
	$(HW_CTRL_PATH)/hwifi_main.o\
	$(HW_CTRL_PATH)/hw_ctrl.o\
	$(HW_CTRL_PATH)/physical_device.o\
	$(HW_CTRL_PATH)/physical_device_thread.o\
	$(HW_CTRL_PATH)/physical_device_workq.o\
	$(HW_CTRL_PATH)/cmm_chip_mt.o
