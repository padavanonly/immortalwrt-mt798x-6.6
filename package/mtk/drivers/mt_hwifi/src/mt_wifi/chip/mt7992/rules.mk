ifneq ($(findstring mt7992,$(CHIPSET)),)
CHIP_7992_PATH := $(PARENT_DIR)/chip/mt7992

obj_chips += \
	$(CHIP_7992_PATH)/mt7992.o\
	$(CHIP_7992_PATH)/mt7992_dbg.o\
	$(CHIP_7992_PATH)/mt7992_ser.o
endif
