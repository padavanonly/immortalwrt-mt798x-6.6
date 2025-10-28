ifeq ($(CONFIG_CFG80211_SUPPORT),y)
LINUX_OS_PATH := $(PARENT_DIR)/os/linux/cfg80211

obj_os += \
	$(LINUX_OS_PATH)/cfg80211_ap.o\
	$(LINUX_OS_PATH)/cfg80211_apcli.o\
	$(LINUX_OS_PATH)/cfg80211.o\
	$(LINUX_OS_PATH)/cfg80211drv.o\
	$(LINUX_OS_PATH)/cfg80211_inf.o\
	$(LINUX_OS_PATH)/cfg80211_rx.o\
	$(LINUX_OS_PATH)/cfg80211_scan.o\
	$(LINUX_OS_PATH)/cfg80211_tx.o\
	$(LINUX_OS_PATH)/cfg80211_util.o\
	$(LINUX_OS_PATH)/cfg80211_vndr_ap.o\
	$(LINUX_OS_PATH)/cfg80211_vndr_ftm.o
endif
