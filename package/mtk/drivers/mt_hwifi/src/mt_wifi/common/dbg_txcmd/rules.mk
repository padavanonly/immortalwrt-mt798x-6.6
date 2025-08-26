CMM_DBG_TXCMD_PATH := $(PARENT_DIR)/common/dbg_txcmd

obj_common += \
	$(CMM_DBG_TXCMD_PATH)/dbg_txcmd_framework.o \
	$(CMM_DBG_TXCMD_PATH)/dbg_txcmd.o
