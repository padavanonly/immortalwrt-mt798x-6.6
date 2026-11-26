COMMON_PATH := $(PARENT_DIR)/common/bss_mngr

obj_common += \
	$(COMMON_PATH)/bss_mngr_main.o \
	$(COMMON_PATH)/bss_mngr_element.o \
	$(COMMON_PATH)/bss_mngr.o \
        $(COMMON_PATH)/bss_mngr_sec.o
