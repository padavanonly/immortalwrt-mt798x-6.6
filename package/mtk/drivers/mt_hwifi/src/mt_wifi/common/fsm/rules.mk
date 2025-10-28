CMM_FSM_PATH := $(PARENT_DIR)/common/fsm

obj_common += \
        $(CMM_FSM_PATH)/ap_mgmt_assoc.o\
        $(CMM_FSM_PATH)/ap_mgmt_auth.o\
        $(CMM_FSM_PATH)/ap_mgmt_cntl.o\
        $(CMM_FSM_PATH)/ap_mgmt_sync.o\
        $(CMM_FSM_PATH)/fsm_assoc.o\
        $(CMM_FSM_PATH)/fsm_auth.o\
        $(CMM_FSM_PATH)/fsm_cntl.o\
        $(CMM_FSM_PATH)/fsm_sync.o\
        $(CMM_FSM_PATH)/sta_mgmt_assoc.o\
        $(CMM_FSM_PATH)/sta_mgmt_auth.o\
        $(CMM_FSM_PATH)/sta_mgmt_cntl.o\
        $(CMM_FSM_PATH)/sta_mgmt_sync.o

