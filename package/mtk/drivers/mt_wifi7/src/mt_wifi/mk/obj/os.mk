ifeq ($(TARGET),LINUX)
include $(TOP_DIR)/os/linux/rules.mk
include $(TOP_DIR)/os/linux/cfg80211/rules.mk
endif

ifeq ($(TARGET),NONE)
include $(TOP_DIR)/os/none/rules.mk
endif
