ifeq ($(CONFIG_DOT11W_PMF_SUPPORT),y)
SEC_PMF_PATH := $(PARENT_DIR)/common/security/pmf

obj_common += \
	$(SEC_PMF_PATH)/pmf.o
endif
