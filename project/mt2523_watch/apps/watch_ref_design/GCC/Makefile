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

#######################################################
# Project vairables assignment

SOURCE_DIR = ../../../../..

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
# Touch panel Config
-include $(SOURCE_DIR)/driver/board/mt2523_watch/touch_panel/module.mk
# Keypad Config
-include $(SOURCE_DIR)/driver/board/mt2523_watch/keypad/module.mk
# LZMA decoder
include $(SOURCE_DIR)/middleware/third_party/lzma_decoder/module.mk

ifeq ($(MTK_PORT_SERVICE_ENABLE), y)
# add PORT SERVICE files
include $(SOURCE_DIR)/middleware/MTK/port_service/module.mk
endif

DEBUG = 0
BUILD_DIR = $(PWD)/Build

# Project name
PROJ_NAME       = $(notdir $(abspath $(dir $(PWD))))
PROJ_PATH = $(PWD)
OUTPATH = $(PWD)/Build
OUTPATH_LIB = $(SOURCE_DIR)/out/mt2523_watch/watch_ref_design/

# Sources or Lib
# RTOS source files
include $(SOURCE_DIR)/kernel/rtos/FreeRTOS/module.mk

# mems driver files
include $(SOURCE_DIR)/driver/board/component/mems/module.mk
include $(SOURCE_DIR)/driver/board/mt2523_watch/mems_init/module.mk

# battery_management files
include $(SOURCE_DIR)/middleware/MTK/battery_management/module.mk

CFLAGS += -DMT2523_WATCH_SENSOR_CALIBRATION

# sensor subsys files
include $(SOURCE_DIR)/middleware/MTK/sensor_subsys/module.mk
include sensor_demo.mk

include gnss_demo.mk

#bluetooth connection manager
include $(SOURCE_DIR)/middleware/MTK/bluetooth_service/bt_device_manager_module.mk
SENSOR_FUSION_ALGO = $(SOURCE_DIR)/prebuilt/middleware/MTK/fusion_algo/common/lib_core
$(info SENSOR_FUSION_ALGO is $(SENSOR_FUSION_ALGO))
ifeq "$(wildcard $(SENSOR_FUSION_ALGO))" ""
    $(info No sensor fusion algorithm)
else
    CFLAGS += -DSENSOR_DEMO
endif

# clover driver files
include $(SOURCE_DIR)/driver/board/component/biosensor/mt2511/src/module.mk

# GNSS files
include $(SOURCE_DIR)/middleware/MTK/gnss/module.mk

# kernel service files
include $(SOURCE_DIR)/kernel/service/module.mk

# HAL driver files
include $(SOURCE_DIR)/driver/board/mt25x3_hdk/module.mk
include $(SOURCE_DIR)/driver/chip/mt2523/module.mk

# Bluetooth module
include $(SOURCE_DIR)/middleware/MTK/bluetooth/module.mk

# bt callback manager module
include $(SOURCE_DIR)/middleware/MTK/bt_callback_manager/module.mk

# NVDM files
include $(SOURCE_DIR)/middleware/MTK/nvdm/module.mk

# BT codec files
include $(SOURCE_DIR)/prebuilt/driver/board/component/bt_codec/module.mk
# bt notify module
include $(SOURCE_DIR)/middleware/MTK/bt_notify/module.mk

# Ancs lib
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/MTK/ble_ancs/src_protected/),)
include $(SOURCE_DIR)/middleware/MTK/ble_ancs/module.mk
else
include $(SOURCE_DIR)/prebuilt/middleware/MTK/ble_ancs/module.mk
endif

# fatfs files
include $(SOURCE_DIR)/middleware/third_party/fatfs/module.mk

# bt components le module
include $(SOURCE_DIR)/middleware/MTK/bluetooth_service/module.mk

# bt source module
include $(SOURCE_DIR)/middleware/MTK/bt_source/module.mk

ifeq ($(MTK_USB_DEMO_ENABLED), y)
# add USB CLASS files
include $(SOURCE_DIR)/middleware/MTK/usb/module.mk
endif

# add ATCI files
include $(SOURCE_DIR)/middleware/MTK/atci/module.mk

# Audio middleware module
include $(SOURCE_DIR)/middleware/MTK/audio/module.mk

# BT Sink module
include $(SOURCE_DIR)/middleware/MTK/bt_sink/module.mk

# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC = $(APP_PATH)/src

APP_FILES = $(APP_PATH)/GCC/syscalls.c

APP_FILES += $(APP_PATH_SRC)/ept_gpio_var.c
APP_FILES += $(APP_PATH_SRC)/ept_eint_var.c
APP_FILES += $(APP_PATH_SRC)/sys_init.c
APP_FILES += $(APP_PATH_SRC)/regions_init.c
APP_FILES += $(APP_PATH_SRC)/system_mt2523.c
APP_FILES += $(APP_PATH_SRC)/task_def.c
APP_FILES += $(APP_PATH_SRC)/battery/battery_message.c


C_FILES += $(APP_PATH_SRC)/hci_log.c
C_FILES += $(APP_PATH_SRC)/bt_init.c
C_FILES += $(APP_PATH_SRC)/gatt_service.c
C_FILES += $(APP_PATH_SRC)/Notification/bt_notify_app.c
C_FILES += $(APP_PATH_SRC)/Notification/bt_notify_app_list.c
C_FILES += $(APP_PATH_SRC)/Notification/ble_ancs_app.c
C_FILES += $(APP_PATH_SRC)/Notification/bt_ancs_common.c
C_FILES += $(APP_PATH_SRC)/bt_gattc_discovery.c
C_FILES += $(APP_PATH_SRC)/bt_app_common.c
C_FILES += $(APP_PATH_SRC)/ble_find_me_client/ble_find_me_client.c
C_FILES += $(APP_PATH_SRC)/sys_init.c

C_FILES += $(APP_PATH_SRC)/dt/src/data_transfer.c
C_FILES += $(APP_PATH_SRC)/AudioPlayer/audio_player_main.c

C_FILES += $(APP_FILES)
C_FILES += driver/board/mt25x3_hdk/ept/src/bsp_gpio_ept_config.c

#CXX_FILES for .cpp
CXX_FILES += $(APP_PATH_SRC)/main.cpp

# add startup file to build
S_FILES += $(APP_PATH)/GCC/startup_mt2523.s

###################################################
# Location of output folders where auto generated code from assets is placed
ifeq ($(UI_CONFIG), touchgfx)
	CFLAGS += -DUI_TOUCHGFX
	CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/touchgfx/inc
	CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/touchgfx/inc/touchgfx
	CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/touchgfx/inc/touchgfx/hal
	CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lzma_decoder/inc

	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/common
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/home_screen
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/main_menu_screen
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/setting_screen
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/graph_screen
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/model
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/gui/database
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/inc/gnss_demo
	CFLAGS += -I$(OUTPATH)/generated/texts/include
	CFLAGS += -I$(OUTPATH)/generated/images/include
	CFLAGS += -I$(SOURCE_DIR)/project/mt2523_watch/apps/watch_ref_design/src/ui/gui/include/generated


	#middleware source
	TOUCHGFX_PATH := $(SOURCE_DIR)/middleware/third_party/touchgfx
	CXX_FILES += middleware/third_party/touchgfx/src/mt2523HAL.cpp
	CXX_FILES += middleware/third_party/touchgfx/src/OSWrappers.cpp
	CXX_FILES += middleware/third_party/touchgfx/src/mt2523DMA.cpp
	CXX_FILES += middleware/third_party/touchgfx/src/CortexMMCUInstrumentation.cpp
	CXX_FILES += middleware/third_party/touchgfx/src/GPIO.cpp

	CXX_FILES += kernel/service/src/new.cpp

	C_FILES += driver/board/mt2523_watch/lcd/mt25x3_hdk_lcd.c
	C_FILES += driver/board/mt2523_watch/backlight/mt25x3_hdk_backlight.c
	C_FILES += driver/board/component/lcm/RM67160/lcd_RM67160.c
	C_FILES += driver/board/component/lcm/RM69080/lcd_RM69080.c

	#app source
	APPSRC_DIR = project/mt2523_watch/apps/watch_ref_design/src/ui/gui/src
	CXX_FILES += $(APPSRC_DIR)/common/DemoPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/common/FrontendApplication.cpp
	CXX_FILES += $(APPSRC_DIR)/common/CommonService.cpp

	CXX_FILES += $(APPSRC_DIR)/home_screen/HomePresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/home_screen/HomeView.cpp
	CXX_FILES += $(APPSRC_DIR)/home_screen/NotificationListPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/home_screen/NotificationListView.cpp
	CXX_FILES += $(APPSRC_DIR)/home_screen/MassStoragePresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/home_screen/MassStorageView.cpp

	CXX_FILES += $(APPSRC_DIR)/main_menu_screen/MainMenuPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/main_menu_screen/MainMenuView.cpp
	CXX_FILES += $(APPSRC_DIR)/main_menu_screen/DotIndicator.cpp
	CXX_FILES += $(APPSRC_DIR)/main_menu_screen/SwipeContainer.cpp

	CXX_FILES += $(APPSRC_DIR)/setting_screen/DatePicker.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/MenuListElement.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetDatePresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetDateView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetHeartRatePresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetHeartRateView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetTimePresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetTimeView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SettingPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SettingView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/TimePicker.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/WheelSelector.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/WheelSelectorExtra.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetGPSPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetGPSView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetClockPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetClockView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/HorizontalSlideMenu.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/ScreenTimeoutPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/ScreenTimeoutView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetBluetoothPresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetBluetoothView.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetUSBModePresenter.cpp
	CXX_FILES += $(APPSRC_DIR)/setting_screen/SetUSBModeView.cpp

        #################################
        CXX_FILES += $(APPSRC_DIR)/setting_screen/BTDeviceSerachListPresenter.cpp
        CXX_FILES += $(APPSRC_DIR)/setting_screen/BTDeviceSearchListView.cpp
        CXX_FILES += $(APPSRC_DIR)/setting_screen/BTDeviceOpPresenter.cpp
        CXX_FILES += $(APPSRC_DIR)/setting_screen/BTDeviceOpView.cpp
        CXX_FILES += $(APPSRC_DIR)/setting_screen/BTDeviceListPresenter.cpp
        CXX_FILES += $(APPSRC_DIR)/setting_screen/BTDeviceListView.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/AbstractGraph.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/Graph.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/GraphBelow.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/GraphDots.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/GraphLine.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/PainterColoredBitmap.cpp
	CXX_FILES += $(APPSRC_DIR)/graph_screen/PainterVerticalAlpha.cpp

	CXX_FILES += $(APPSRC_DIR)/model/Model.cpp
	CXX_FILES += $(APPSRC_DIR)/database/heart_rate_db.cpp
	CXX_FILES += $(APPSRC_DIR)/database/DynamicBitmapDatabase.cpp

	CXX_FILES += $(APPSRC_DIR)/BoardConfiguration.cpp
	CXX_FILES += $(APP_PATH_SRC)/touch_adapter/mt2523TouchController.cpp

	CXX_FILES += $(APP_PATH_SRC)/bt_connection/bt_connection_app.cpp

	include $(TOUCHGFX_PATH)/module.mk

endif

###################################################
# convert source file to object
C_OBJS   = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
CXX_OBJS = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS   = $(S_FILES:%.s=$(BUILD_DIR)/%.o)

# overwrite FPUFLAGS
FPUFLAGS = -mcpu=cortex-m4 -fsingle-precision-constant -Wdouble-promotion -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS += -DSHOW_UCS2

CFLAGS += -DWATCH_DRIVER_DRIVER_V2

# add for demo only
#CFLAGS += -Dwatch_demo_v2
#CFLAGS += -DCLOCK_LIST_STYLE
CFLAGS += -DDYNAMIC_BITMAP_LOADER

# add for TE enable
ifeq ($(MTK_TE_ENABLE), y)
CFLAGS	+= -DMTK_TE_ENABLE
endif

# LD Flags
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-T$(patsubst $(SDK_PATH)/%,%,$(PWD))/flash.ld -Wl,--gc-sections
ifeq ($(DEBUG_IO), 1)
LDFLAGS += --specs=rdimon.specs -lrdimon
CFLAGS += -DDEBUG_IO
endif
#LDFLAGS = $(CFLAGS)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/Notification
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/ble_find_me_client
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/bt_connection
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/src/dt/inc
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/AudioPlayer
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Include
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Device/MTK/mt2523/Include
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt2523/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/ept/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/lcd
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/backlight
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bt_notify/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/ble_ancs/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bt_callback_manager/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth_service/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bt_source/inc
# CXXFLAGS for .cpp
CXXFLAGS = $(filter-out -std=gnu99 -Wimplicit-function-declaration,$(CFLAGS))
CXXFLAGS += -fno-rtti -fno-exceptions
###################################################
# Module folder path

###################################################
# Rules
.PHONY: proj clean $(MODULE_PATH)

all: proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)

MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH) FPUFLAGS="$(FPUFLAGS)"

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)

proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
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
