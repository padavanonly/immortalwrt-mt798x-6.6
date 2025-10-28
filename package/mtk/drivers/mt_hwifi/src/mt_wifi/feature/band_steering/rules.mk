BAND_STEERING_PATH := $(PARENT_DIR)/feature/band_steering

obj_feature += \
	$(BAND_STEERING_PATH)/ap_band_steering.o\
	$(BAND_STEERING_PATH)/dyn_steering_ld.o\
	$(BAND_STEERING_PATH)/ccn67_bs.o
