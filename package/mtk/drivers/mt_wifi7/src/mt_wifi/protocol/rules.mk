PROTOCOL_PATH := $(PARENT_DIR)/protocol

obj_protocol += \
	$(PROTOCOL_PATH)/be_basic.o\
	$(PROTOCOL_PATH)/config_basic.o\
	$(PROTOCOL_PATH)/protection.o\
	$(PROTOCOL_PATH)/mgmt_phy.o
