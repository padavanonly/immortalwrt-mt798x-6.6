CHIP_MCU_PATH := $(PARENT_DIR)/chip/mcu

obj_chips += \
	$(CHIP_MCU_PATH)/andes_core.o\
	$(CHIP_MCU_PATH)/andes_mt.o\
	$(CHIP_MCU_PATH)/fw_cmd.o\
	$(CHIP_MCU_PATH)/cmm_fw_uni_cmd.o\
	$(CHIP_MCU_PATH)/cmm_fw_uni_event.o\
	$(CHIP_MCU_PATH)/mcu.o\
	$(CHIP_MCU_PATH)/mt_cmd.o\
	$(CHIP_MCU_PATH)/mt_fdb.o\
	$(CHIP_MCU_PATH)/wifi_md_coex.o
