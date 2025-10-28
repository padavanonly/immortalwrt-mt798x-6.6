CMM_MGMT_PATH := $(PARENT_DIR)/common/mgmt

obj_common += \
	$(CMM_MGMT_PATH)/be_phy.o\
	$(CMM_MGMT_PATH)/mgmt_dev.o\
	$(CMM_MGMT_PATH)/mgmt_entrytb.o\
	$(CMM_MGMT_PATH)/mgmt_hw.o
