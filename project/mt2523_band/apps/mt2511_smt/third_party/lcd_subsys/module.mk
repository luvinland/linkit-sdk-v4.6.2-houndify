
###################################################
APP_PATH = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
CFLAGS += -DGT_LCD_SUBSYS_SUPPORT

LCD_SUBSYS_SRC = $(APP_PATH)/third_party/lcd_subsys
LCD_SUBSYS_FILES = $(LCD_SUBSYS_SRC)/src/lcd_manager.c

# LCD gdi files
LCD_GDI_FILES = $(LCD_SUBSYS_SRC)/src/gdi/font_16x16.c 
LCD_GDI_FILES += $(LCD_SUBSYS_SRC)/src/bw_gdi_api.c 

C_FILES += $(LCD_SUBSYS_FILES) $(LCD_GDI_FILES)
###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/third_party/lcd_subsys/inc
#LIBS += $(SOURCE_DIR)/$(APP_PATH)/third_party/lcd_subsys/src/gdi/bw_gdi_api.a

