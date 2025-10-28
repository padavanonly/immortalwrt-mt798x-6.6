AP_PATH := $(PARENT_DIR)/ap

obj_ap += \
	$(AP_PATH)/ap_autoChSel.o\
	$(AP_PATH)/ap.o\
	$(AP_PATH)/ap_cfg.o\
	$(AP_PATH)/apcli_link_cover.o\
	$(AP_PATH)/ap_data.o\
	$(AP_PATH)/ap_ids.o\
	$(AP_PATH)/ap_mbss.o\
	$(AP_PATH)/ap_mbss_inf.o\
	$(AP_PATH)/ap_mlme.o\
	$(AP_PATH)/ap_muru.o\
	$(AP_PATH)/ap_qload.o\
	$(AP_PATH)/ap_repeater.o\
	$(AP_PATH)/ap_sanity.o\
	$(AP_PATH)/ap_sec.o\
	$(AP_PATH)/ap_wds.o\
	$(AP_PATH)/ap_wds_inf.o\
	$(AP_PATH)/ap_wpa.o\
	$(AP_PATH)/ap_ch_prio.o
