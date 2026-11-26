HW_CTRL_HDEV_PATH := $(PARENT_DIR)/hw_ctrl/hdev

obj_hw_ctrl += \
	$(HW_CTRL_HDEV_PATH)/hdev_basic.o\
	$(HW_CTRL_HDEV_PATH)/radio_ctrl.o\
	$(HW_CTRL_HDEV_PATH)/wmm_ctrl.o
