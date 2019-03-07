APP_PATH = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
# W269 source files
GT_PROJ_SRC = $(APP_PATH)/third_party/gt_proj/src
GT_PROJ_FILES = $(GT_PROJ_SRC)/gt_main.c \
				$(GT_PROJ_SRC)/app_manager.c \
				$(GT_PROJ_SRC)/app_status_bar.c \
				$(GT_PROJ_SRC)/app/app_watch.c \
				$(GT_PROJ_SRC)/app/app_pedometer.c \
				$(GT_PROJ_SRC)/app/app_bloodp.c \
				$(GT_PROJ_SRC)/app/app_hrv.c \
				$(GT_PROJ_SRC)/app/app_sleep.c \
				$(GT_PROJ_SRC)/app/app_heart.c
				

C_FILES += $(GT_PROJ_FILES)

# Include gt path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/third_party/gt_proj/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/sensor_subsys/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/include

