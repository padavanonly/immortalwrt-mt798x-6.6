include $(TOP_DIR)/chip/rules.mk
include $(TOP_DIR)/chip/mcu/rules.mk
include $(TOP_DIR)/chip/mac_mt/rules.mk
ifneq ($(findstring mt7915,$(CHIPSET)),)
include $(TOP_DIR)/chip/mt7915/rules.mk
endif
ifneq ($(findstring bellwether,$(CHIPSET)),)
include $(TOP_DIR)/chip/bellwether/rules.mk
endif
ifneq ($(findstring mt7990,$(CHIPSET)),)
include $(TOP_DIR)/chip/mt7990/rules.mk
endif
ifneq ($(findstring mt7992,$(CHIPSET)),)
include $(TOP_DIR)/chip/mt7992/rules.mk
endif
ifneq ($(findstring mt7993,$(CHIPSET)),)
include $(TOP_DIR)/chip/mt7993/rules.mk
endif
