PROTOCOL_AX_PATH := $(PARENT_DIR)/protocol/ax

obj_protocol += \
	$(PROTOCOL_AX_PATH)/be_he.o\
	$(PROTOCOL_AX_PATH)/bss_color.o\
	$(PROTOCOL_AX_PATH)/bss_ops.o\
	$(PROTOCOL_AX_PATH)/config_he.o\
	$(PROTOCOL_AX_PATH)/mgmt_he.o\
        $(PROTOCOL_AX_PATH)/bcolor_ctrl.o
