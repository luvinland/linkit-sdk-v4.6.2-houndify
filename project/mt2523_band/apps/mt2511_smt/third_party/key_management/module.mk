
###################################################
APP_PATH = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
CFLAGS += -DGT_KEY_MANAGER_SUPPORT

KEY_MGR_SRC = $(APP_PATH)/third_party/key_management
KEY_MGR_FILES = $(KEY_MGR_SRC)/src/key_manager.c

C_FILES += $(KEY_MGR_FILES)
###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/third_party/key_management/inc
