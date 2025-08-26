ifeq ($(CONFIG_WPA3_SUPPORT),y)
SEC_WPA3_PATH := $(PARENT_DIR)/common/security/wpa3

obj_common += \
	$(SEC_WPA3_PATH)/sae.o\
	$(SEC_WPA3_PATH)/bn_lib.o\
	$(SEC_WPA3_PATH)/ecc.o

ifeq ($(CONFIG_OWE_SUPPORT),y)
obj_common += \
	$(SEC_WPA3_PATH)/owe.o
endif
endif
