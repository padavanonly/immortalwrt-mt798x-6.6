STA_PATH := $(PARENT_DIR)/sta

obj_sta += \
	$(STA_PATH)/sanity.o\
	$(STA_PATH)/sta.o\
	$(STA_PATH)/sta_cfg.o\
	$(STA_PATH)/sta_data.o\
	$(STA_PATH)/sta_sec.o\
	$(STA_PATH)/wpa.o
