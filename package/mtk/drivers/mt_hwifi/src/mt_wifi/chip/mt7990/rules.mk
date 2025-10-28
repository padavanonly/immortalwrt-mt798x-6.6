ifneq ($(findstring mt7990,$(CHIPSET)),)
CHIP_7990_PATH := $(PARENT_DIR)/chip/mt7990

obj_chips += \
	$(CHIP_7990_PATH)/mt7990.o\
	$(CHIP_7990_PATH)/mt7990_dbg.o\
	$(CHIP_7990_PATH)/mt7990_ser.o
endif
