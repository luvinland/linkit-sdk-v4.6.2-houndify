
APP_PATH = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))

LIBSENSORSREEN   = $(APP_PATH)/src/sensor_demo

C_FILES  += $(LIBSENSORSREEN)/sensor_demo.c

CFLAGS  += -I$(SOURCE_DIR)/$(APP_PATH)/inc/sensor_demo
