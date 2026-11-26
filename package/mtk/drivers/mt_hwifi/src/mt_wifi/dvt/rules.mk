DVT_OBJ_PATH := $(PARENT_DIR)/dvt


obj_dvt += \
        $(DVT_OBJ_PATH)/mdvt.o

ifeq ($(CONFIG_WIFI_SYSDVT), y)
obj_dvt += \
        $(DVT_OBJ_PATH)/framework_dvt.o\
        $(DVT_OBJ_PATH)/wrap_dvt.o\
        $(DVT_OBJ_PATH)/apps_dvt.o\
        $(DVT_OBJ_PATH)/lp_dvt.o\
        $(DVT_OBJ_PATH)/txcmdsu_dvt.o\
        $(DVT_OBJ_PATH)/mucop_dvt.o\
        $(DVT_OBJ_PATH)/mdvt.o \
	$(DVT_OBJ_PATH)/sdo_admctrl_dvt.o \
	$(DVT_OBJ_PATH)/sdo_admctrl_dvt_setting.o
endif

ifeq ($(CONFIG_VERIFICATION_MODE),y)
obj_dvt += \
        $(DVT_OBJ_PATH)/verification_mode.o
endif

ifeq ($(CONFIG_PRE_CFG_SUPPORT),y)
obj_dvt += \
        $(DVT_OBJ_PATH)/pre_cfg.o
endif
