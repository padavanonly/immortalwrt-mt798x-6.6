include $(TOP_DIR)/feature/band_steering/rules.mk
include $(TOP_DIR)/feature/ft/rules.mk
include $(TOP_DIR)/feature/hs_r2/rules.mk
include $(TOP_DIR)/feature/igmp/rules.mk
include $(TOP_DIR)/feature/map/rules.mk
include $(TOP_DIR)/feature/mbo/rules.mk
include $(TOP_DIR)/feature/mwds/rules.mk
include $(TOP_DIR)/feature/oce/rules.mk
include $(TOP_DIR)/feature/ps/rules.mk
include $(TOP_DIR)/feature/rrm/rules.mk
include $(TOP_DIR)/feature/twt/rules.mk
include $(TOP_DIR)/feature/txbf/rules.mk
include $(TOP_DIR)/feature/vow/rules.mk
include $(TOP_DIR)/feature/wapp/rules.mk
include $(TOP_DIR)/feature/whc/rules.mk
include $(TOP_DIR)/feature/wps/rules.mk
include $(TOP_DIR)/feature/red/rules.mk
include $(TOP_DIR)/feature/cosr/rules.mk
include $(TOP_DIR)/feature/mbss_11v/rules.mk
include $(TOP_DIR)/feature/t2lm/rules.mk
ifeq ($(CONFIG_FTM_SUPPORT), y)
include $(TOP_DIR)/feature/ftm/rules.mk
endif
include $(TOP_DIR)/feature/mlr/rules.mk
