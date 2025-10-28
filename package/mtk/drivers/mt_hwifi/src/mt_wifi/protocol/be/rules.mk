PROTOCOL_EHT_PATH := $(PARENT_DIR)/protocol/be

obj_protocol += \
	$(PROTOCOL_EHT_PATH)/be_eht.o\
	$(PROTOCOL_EHT_PATH)/config_eht.o\
	$(PROTOCOL_EHT_PATH)/mgmt_eht.o
