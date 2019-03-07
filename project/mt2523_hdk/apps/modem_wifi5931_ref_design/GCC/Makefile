# Copyright Statement:
#
# (C) 2005-2016  MediaTek Inc. All rights reserved.
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
# Without the prior written permission of MediaTek and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
# if you have agreed to and been bound by the applicable license agreement with
# MediaTek ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of MediaTek Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#

SOURCE_DIR = ../../../../..
SUPPORT_TESTFRAMEWORK=true
PWD= $(shell pwd)
SDK_PATH    = $(abspath $(PWD)/$(SOURCE_DIR))
FEATURE ?= feature.mk
include $(FEATURE)
# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# EPT Config
-include $(SOURCE_DIR)/driver/board/mt25x3_hdk/ept/module.mk
# Keypad Config
-include $(SOURCE_DIR)/driver/board/mt25x3_hdk/keypad/module.mk
ifeq ($(MTK_MODEM_ON_HDK_ENABLE),y)
# Touch panel Config
-include $(SOURCE_DIR)/driver/board/mt25x3_hdk/touch_panel/module.mk
endif

###################################################
# Sources or Lib
# httpclient
include $(SOURCE_DIR)/middleware/third_party/httpclient/module.mk

RAM_BOOTING=0

DEBUG = 0
FLOAT_TYPE = hard
BUILD_DIR = $(PWD)/Build

# Project name
PROJ_NAME ?= modem_wifi5931_ref_design
PROJ_PATH = $(PWD)
OUTPATH = $(PWD)/Build


###################################################
# Sources or Lib
# RTOS source files
include $(SOURCE_DIR)/kernel/rtos/FreeRTOS/module.mk

ifeq ($(MTK_MODEM_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/gdi/module.mk
endif

ifneq ($(MTK_BT_NORMAL_MODE_ENABLE), y)
ifdef TOOL_MODULE
CFLAGS += -DMTK_TOOL_MODULE_ENABLE
include $(TOOL_MODULE)
endif
endif

ifndef FLAG_STORE_FOLDER
FLAG_STORE_FOLDER = $(OUTPATH)
endif

# mems driver files
include $(SOURCE_DIR)/driver/board/component/mems/module.mk
include $(SOURCE_DIR)/driver/board/mt25x3_hdk/mems_init/module.mk

ifeq ($(MTK_PORT_SERVICE_ENABLE), y)
# add PORT SERVICE files
include $(SOURCE_DIR)/middleware/MTK/port_service/module.mk
endif

ifeq ($(MTK_USB_DEMO_ENABLED), y)
# add USB CLASS files
include $(SOURCE_DIR)/middleware/MTK/usb/module.mk
endif

# GNSS middleware files
include $(SOURCE_DIR)/middleware/MTK/gnss/module.mk


# fatfs files
ifeq ($(MTK_FATFS_ENABLE), y)
include $(SOURCE_DIR)/middleware/third_party/fatfs/module.mk
endif

# FOTA files
include $(SOURCE_DIR)/middleware/MTK/fota/module.mk

# FOTA download manager files
#include $(SOURCE_DIR)/project/mt2523_hdk/apps/fota_download_manager/module.mk

#include mbedtls
include $(SOURCE_DIR)/middleware/third_party/mbedtls/module.mk

# HAL driver files
include $(SOURCE_DIR)/driver/board/mt25x3_hdk/module.mk
include $(SOURCE_DIR)/driver/chip/mt2523/module.mk


include $(SOURCE_DIR)/middleware/MTK/nvdm/module.mk


# BT codec files
include $(SOURCE_DIR)/prebuilt/driver/board/component/bt_codec/module.mk

# MTK_SMART_BATTERY_ENABLE define
ifeq ($(MTK_SMART_BATTERY_ENABLE), y)
CFLAGS	+= -DMTK_SMART_BATTERY_ENABLE
endif

# battery_management files
include $(SOURCE_DIR)/middleware/MTK/battery_management/module.mk

# camera files
#include $(SOURCE_DIR)/middleware/MTK/camera/module.mk
#CFLAGS += -DMTK_ATCI_CAMERA_ENABLE

# Audio module files
#include $(SOURCE_DIR)/middleware/MTK/audio/module.mk

# mp3 codec library
#LIBS += $(SOURCE_DIR)/middleware/MTK/audio/mp3_codec/lib/arm_cm4/libmp3dec.a

# MTK_BT_DONGLE_TEST define
#ifeq ($(MTK_BT_DONGLE_TEST), y)
#CFLAGS	+= -D__MTK_BT_DONGLE_TEST__
#endif

# bluetooth libs
#include $(SOURCE_DIR)/middleware/MTK/bluetooth/module.mk

#include ping
include $(SOURCE_DIR)/middleware/third_party/ping/module.mk

#include lwip
include $(SOURCE_DIR)/middleware/third_party/lwip/module.mk

# kernel service files
include $(SOURCE_DIR)/kernel/service/module.mk

#include sntp
include $(SOURCE_DIR)/middleware/third_party/sntp/module.mk

#include mqtt
include $(SOURCE_DIR)/middleware/third_party/mqtt/module.mk

#include iperf
include $(SOURCE_DIR)/middleware/MTK/iperf/module.mk

#slp
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/MTK/slp/src_protected/),)
include $(SOURCE_DIR)/middleware/MTK/slp/module.mk
else
include $(SOURCE_DIR)/prebuilt/middleware/MTK/slp/module.mk
endif


ifeq ($(MTK_WIFI_CHIP_USE_MT5931),y)
#wndrv
ifneq ($(wildcard $(strip $(SOURCE_DIR))/driver/board/component/wifi/mt5931/),)
include $(SOURCE_DIR)/driver/board/component/wifi/mt5931/module.mk
else
include $(SOURCE_DIR)/prebuilt/driver/board/component/wifi/mt5931/module.mk
endif

ifneq ($(wildcard $(strip $(SOURCE_DIR))/driver/board/mt25x3_hdk/wifi/mt5931/),)
include $(SOURCE_DIR)/driver/board/mt25x3_hdk/wifi/mt5931/module.mk
else
include $(SOURCE_DIR)/prebuilt/driver/board/mt25x3_hdk/wifi/mt5931/module.mk
endif

#minorsupc
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/MTK/minorsupc/),)
include $(SOURCE_DIR)/middleware/MTK/minorsupc/module.mk
else
include $(SOURCE_DIR)/prebuilt/middleware/MTK/minorsupc/module.mk
endif

CFLAGS += -DTCPIP_FOR_MT5931_ENABLE
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/wifi/mt5931/inc
endif

#modem demo
ifeq ($(MTK_MODEM_ENABLE),y)
# add sio files
include $(SOURCE_DIR)/middleware/MTK/sio/module.mk

include modem_demo.mk
endif

# add ATCI files
include $(SOURCE_DIR)/middleware/MTK/atci/module.mk

# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC = project/mt2523_hdk/apps/modem_wifi5931_ref_design/src

APP_FILES = $(APP_PATH_SRC)/main.c \
            $(APP_PATH_SRC)/regions_init.c \
            $(APP_PATH)/GCC/syscalls.c \
            $(APP_PATH_SRC)/hci_log.c \
            $(APP_PATH_SRC)/gnss_cust.c \
            $(APP_PATH_SRC)/atci_cmd.c \
            $(APP_PATH_SRC)/fota_demo/fota_demo.c \
            $(APP_PATH_SRC)/wifi_custom_config.c

ifeq ($(MTK_WIFI_CHIP_USE_MT5931),y)
APP_FILES += $(APP_PATH_SRC)/lwip_network.c
endif

ifeq ($(MTK_MODEM_ON_HDK_ENABLE),n)
APP_FILES += $(APP_PATH_SRC)/common.c
endif

#BT related file
ifeq ($(MTK_MODEM_ON_HDK_ENABLE),n)
APP_FILES += $(APP_PATH_SRC)/bt_init.c \
             $(APP_PATH_SRC)/bt_common.c
CFLAGS += -DBT_DEBUG
endif


ifdef TOOL_APP_MODULE
include $(SOURCE_DIR)/$(APP_PATH_SRC)/$(TOOL_APP_MODULE)/module.mk
CFLAGS+= -DTOOL_APP_MODULE
endif


ifeq ($(MTK_MODEM_ON_HDK_ENABLE), y)
APP_FILES += $(APP_PATH_SRC)/ept_dte/ept_gpio_var.c \
             $(APP_PATH_SRC)/ept_dte/ept_eint_var.c

APP_FILES += $(APP_PATH_SRC)/ept_dte/ept_dte_gpio_var.c \
             $(APP_PATH_SRC)/ept_dte/ept_dte_eint_var.c
else
APP_FILES += $(APP_PATH_SRC)/ept_common/ept_gpio_var.c \
             $(APP_PATH_SRC)/ept_common/ept_eint_var.c

APP_FILES += $(APP_PATH_SRC)/ept_common/ept_dte_gpio_var.c \
             $(APP_PATH_SRC)/ept_common/ept_dte_eint_var.c
endif

APP_FILES += $(APP_PATH_SRC)/system_mt2523.c

ifeq ($(MTK_LCD_ENABLE),y)
C_FILES += $(APP_PATH_SRC)/resource/FontRes.c
endif

# MVG flash test files
ifeq ($(MTK_MVG_FLASH_TEST_ENABLE), y)
MVG_SRC = project/mt2523_hdk/test_loads/HQA/hqa_flash_test/
C_FILES += $(MVG_SRC)/src/mvg/flash_mtd_sf_ut.c
endif


C_FILES += $(APP_FILES)

# GNSS app files
ifeq ($(MTK_MODEM_ENABLE),y)
include $(SOURCE_DIR)/$(APP_PATH)/GCC/module_gnss.mk
endif

# add startup file to build
S_FILES += $(APP_PATH)/GCC/startup_mt2523.s

C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS = $(S_FILES:%.s=$(BUILD_DIR)/%.o)

# LD Flags
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
ifeq ($(RAM_BOOTING), 1)
CFLAGS+= -DRAM_BOOTING
LDFLAGS += -Wl,-T$(patsubst $(SDK_PATH)/%,%,$(PWD))/ram.ld -Wl,--gc-sections
else
LDFLAGS += -Wl,-T$(patsubst $(SDK_PATH)/%,%,$(PWD))/flash.ld -Wl,--gc-sections
endif
ifeq ($(DEBUG_IO), 1)
LDFLAGS += --specs=rdimon.specs -lrdimon
CFLAGS += -DDEBUG_IO
endif
LDFLAGS += -u _printf_float
#LDFLAGS = $(CFLAGS)

ifeq ($(MTK_LCD_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/resource
endif


ifeq ($(MTK_QUERY_SDK_VERSION),y)
CFLAGS += -DMTK_QUERY_SDK_VERSION
endif

ifeq ($(MTK_BUILD_SMT_LOAD),y)
CFLAGS         += -DMTK_BUILD_SMT_LOAD
endif

ifeq ($(MTK_MODEM_ENABLE),y)
CFLAGS += -DMODEM_ENABLE
CFLAGS += -DMTK_TCPIP_FOR_EXTERNAL_MODULE_ENABLE
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/include
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/ports/include
ifeq ($(MTK_MODEM_ON_HDK_ENABLE),y)
CFLAGS += -DMODEM_ON_HDK_ENABLE
endif
endif

###################################################
# include path
ifeq ($(MTK_LCD_ENABLE),y)
CFLAGS += -DLCD_ENABLE
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/gdi/inc
endif
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Include
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/fota_demo
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Device/MTK/mt2523/Include
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt2523/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/bt_codec/inc
ifeq ($(MTK_NVDM_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/nvdm/inc
endif
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/ept/inc
ifeq ($(MTK_MODEM_ON_HDK_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/ept_dte
else
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/ept_common
endif


###################################################
# Rules
.PHONY: proj clean $(MODULE_PATH)

all: cleanlog proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)

MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)
proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	@echo "export LDFLAGS"
	echo $(LDFLAGS) > $(FLAG_STORE_FOLDER)/LDFLAGS.config
	@echo "export CFLAGS"
	echo $(CFLAGS) > $(FLAG_STORE_FOLDER)/CFLAGS.config
	@echo "export end"
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).dis" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).dis"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@echo $(patsubst $(SOURCE_DIR)/%,%,$(patsubst $(SDK_PATH)/%,%, $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,%,$(OUTPATH)/$(PROJ_NAME).map) -lm -lstdc++ -o $@)) > $(OUTPATH)/link_option.tmp
	@cd $(SOURCE_DIR) && \
	$(patsubst $(SOURCE_DIR)/%,%,$(CC)) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	@rm -f $(OUTPATH)/link_option.tmp
	@$(OBJDUMP) -D $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).dis
	@$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done

include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)
