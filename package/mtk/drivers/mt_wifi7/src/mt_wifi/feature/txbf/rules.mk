TXBF_PATH := $(PARENT_DIR)/feature/txbf

obj_feature += \
	$(TXBF_PATH)/bf.o\
	$(TXBF_PATH)/ap_mumimo.o\
	$(TXBF_PATH)/cmm_mumimo.o\
	$(TXBF_PATH)/cmm_txbf_cal_mt.o\
	$(TXBF_PATH)/cmm_txbf_mt.o\
	$(TXBF_PATH)/txbf_wrapper_embedded.o
