PROTOCOL_LEGACY_PATH := $(PARENT_DIR)/protocol/leagcy

obj_protocol += \
	$(PROTOCOL_LEGACY_PATH)/be_ht.o\
	$(PROTOCOL_LEGACY_PATH)/config_ht.o\
	$(PROTOCOL_LEGACY_PATH)/mgmt_ht.o
