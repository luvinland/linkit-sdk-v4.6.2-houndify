###################################################
# Sources
###################################################
APP_PATH_SRC_MODEM = project/mt2523_hdk/apps/modem_wifi5931_ref_design/src/modem_demo

C_FILES += $(APP_PATH_SRC_MODEM)/gprs_api.c \
           $(APP_PATH_SRC_MODEM)/sio_test.c
           
ifeq ($(MTK_LCD_ENABLE),y)
C_FILES += $(APP_PATH_SRC_MODEM)/main_screen.c \
           $(APP_PATH_SRC_MODEM)/nw_ui.c 
endif

C_FILES += $(APP_PATH_SRC_MODEM)/nw_app_main.c \
           $(APP_PATH_SRC_MODEM)/nw_atci.c


###################################################
# include path
###################################################

CFLAGS += -I$(SOURCE_DIR)/project/mt2523_hdk/apps/modem_wifi5931_ref_design/inc/modem_demo



###################################################
# definition
###################################################
ifeq ($(MTK_MODEM_LOWER_POWER),y)
CFLAGS+= -D__MODEM_LOWER_POWER__
endif


