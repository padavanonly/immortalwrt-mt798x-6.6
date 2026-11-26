LINUX_OS_PATH := $(PARENT_DIR)/os/linux

obj_os += \
	$(LINUX_OS_PATH)/android_priv_cmd.o\
	$(LINUX_OS_PATH)/ap_ioctl.o\
	$(LINUX_OS_PATH)/diag.o\
	$(LINUX_OS_PATH)/inf_ppa.o\
	$(LINUX_OS_PATH)/mt_fwdump.o\
	$(LINUX_OS_PATH)/multi_main_dev.o\
	$(LINUX_OS_PATH)/pci_main_dev.o\
	$(LINUX_OS_PATH)/rbus_prop_dev.o\
	$(LINUX_OS_PATH)/rt_linux.o\
	$(LINUX_OS_PATH)/rt_main_dev.o\
	$(LINUX_OS_PATH)/rt_pci_rbus.o\
	$(LINUX_OS_PATH)/rt_proc.o\
	$(LINUX_OS_PATH)/rt_profile.o\
	$(LINUX_OS_PATH)/rt_rbus_pci_drv.o\
	$(LINUX_OS_PATH)/rt_rbus_pci_util.o\
	$(LINUX_OS_PATH)/rt_txrx_hook.o\
	$(LINUX_OS_PATH)/sta_ioctl.o\
	$(LINUX_OS_PATH)/tm.o\
	$(LINUX_OS_PATH)/wbsys_main_dev.o\
	$(LINUX_OS_PATH)/mt_data.o
