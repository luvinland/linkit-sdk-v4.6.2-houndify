# source files

#include $(SOURCE_DIR)/middleware/protected/slp/module.mk

GNSS_APP_SRC_PATH = project/mt2523_hdk/apps/modem_wifi5931_ref_design/src/gnss_example
GNSS_APP_INC_PATH = project/mt2523_hdk/apps/modem_wifi5931_ref_design/inc/gnss_example
#BT_NOTIFY_SRC = $(GNSS_APP_SRC_PATH)/btnotify_src
#C_FILES += $(BT_NOTIFY_SRC)/ble_dogp_service.c
#C_FILES += $(BT_NOTIFY_SRC)/ble_dogp_adp_service.c
#C_FILES += $(BT_NOTIFY_SRC)/ble_gatts_srv_common.c
#C_FILES += $(BT_NOTIFY_SRC)/bt_notify_conversion.c
#C_FILES += $(BT_NOTIFY_SRC)/bt_notify_data_parse.c
#C_FILES += $(BT_NOTIFY_SRC)/xml_main.c
C_FILES += $(GNSS_APP_SRC_PATH)/epo_demo.c
#C_FILES += $(GNSS_APP_SRC_PATH)/epo_download.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_app.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_bridge_task.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_demo.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_ring_buffer.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_timer.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_uart_bridge.c
C_FILES += $(GNSS_APP_SRC_PATH)/wepodownload.c
C_FILES += $(GNSS_APP_SRC_PATH)/httpc_download_daem.c
C_FILES += $(GNSS_APP_SRC_PATH)/slp_app.c
#C_FILES += $(GNSS_APP_SRC_PATH)/hci_log.c
C_FILES += $(GNSS_APP_SRC_PATH)/gnss_test_cmd.c
#LIBS += $(SOURCE_DIR)/project/mt2523_hdk/apps/modem_wifi5931_ref_design/lib/libbtnotify.a

# include files

CFLAGS += -I$(SOURCE_DIR)/$(GNSS_APP_INC_PATH)/
#CFLAGS += -I$(SOURCE_DIR)/$(GNSS_APP_INC_PATH)/btnotify_inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt2523/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/nvdm/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/nvdm/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/httpclient/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/sntp/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/include/mbedtls
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/slp/inc
