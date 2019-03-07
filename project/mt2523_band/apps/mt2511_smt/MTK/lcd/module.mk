
###################################################
BOARD_SRC = driver/board/mt25x3_hdk
APP_PATH = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
BOARD_LCD_SRC = $(APP_PATH)/MTK/lcd
COMPONENT_SRC = $(APP_PATH)/MTK/SH1107


C_FILES += $(BOARD_LCD_SRC)/mt25x3_hdk_lcd.c
C_FILES += $(COMPONENT_SRC)/lcd.c
C_FILES += $(BOARD_SRC)/backlight/mt25x3_hdk_backlight.c

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/MTK/SH1107
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/MTK/lcd
CFLAGS += -I$(SOURCE_DIR)/driver/board/component/common
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/backlight
