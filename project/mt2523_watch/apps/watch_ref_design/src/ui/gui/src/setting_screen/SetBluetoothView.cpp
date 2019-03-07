/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <gui/setting_screen/SetBluetoothView.hpp>
#include "BitmapDatabase.hpp"
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/EasingEquations.hpp>
#include <gui/common/CommonUI.hpp>
#include <gui/common/CommonService.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>

#ifndef SIMULATOR
#include "syslog.h"
#endif

#ifndef SIMULATOR
#include "bt_init.h"
#include "bt_system.h"
#include "bt_callback_manager.h"
#endif

SetBluetoothView::SetBluetoothView() :
    DemoView<SetBluetoothPresenter>(),
    onState(CommonService::getBluetoothStatus()),
    onToggleButtonClicked(this, &SetBluetoothView::toggleButtonClickedHandler)
{
}

SetBluetoothView::~SetBluetoothView()
{
}

void SetBluetoothView::setupScreen()
{
#ifndef SIMULATOR
    if (onState) {
        LOG_I(tgfx, "Current UI: Settings Set Bluetooth (on)\r\n");
    } else {
        LOG_I(tgfx, "Current UI: Settings Set Bluetooth (off)\r\n");
    }
#endif

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage.setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    const DynamicBitmapDatabase::BitmapInfo backgroundInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_COMMON_BACKGROUND_INT_ID);
    //create dynamic bitmap matching file dimensions
    backgroundId = Bitmap::dynamicBitmapCreate(backgroundInfo.width, backgroundInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", backgroundId);
    if (backgroundId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(backgroundId);
        uint32_t destLen = backgroundInfo.width * backgroundInfo.height * 2;
        uint32_t srcLen = backgroundInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, backgroundInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        backgroundImage.setBitmap(Bitmap(backgroundId));
    }
#endif
    backgroundImage.setXY(0, 0);
    add(backgroundImage);

    // Add title
    titleTxt.setTypedText(TypedText(T_BLUETOOTH_TITLE));
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    titleTxt.setPosition(0, CommonUI::TITLE_Y, HAL::DISPLAY_WIDTH, CommonUI::TITLE_FONT_HEIGHT);
    titleTxt.setAlpha(127);
    add(titleTxt);

    if (onState) {
        buttonTxt.setTypedText(TypedText(T_ON_TEXT));
    } else {
        buttonTxt.setTypedText(TypedText(T_OFF_TEXT));
    }
    buttonTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    buttonTxt.setPosition(CommonUI::TOGGLE_BUTTON_TEXT_X, CommonUI::TOGGLE_BUTTON_TEXT_Y, CommonUI::TOGGLE_BUTTON_TEXT_WIDTH, CommonUI::TOGGLE_BUTTON_TEXT_HEIGHT);
    add(buttonTxt);

#ifndef DYNAMIC_BITMAP_LOADER
    if (onState) {
        button.setBitmaps(Bitmap(BITMAP_SWITCH_ON_INT_ID), Bitmap(BITMAP_SWITCH_OFF_INT_ID));
    } else {
        button.setBitmaps(Bitmap(BITMAP_SWITCH_OFF_INT_ID), Bitmap(BITMAP_SWITCH_ON_INT_ID));
    }
#else
    const DynamicBitmapDatabase::BitmapInfo switchOnInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_SWITCH_ON_INT_ID);
    //create dynamic bitmap matching file dimensions
    switchOnId = Bitmap::dynamicBitmapCreate(switchOnInfo.width, switchOnInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", switchOnId);
    if (switchOnId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(switchOnId);
        uint32_t destLen = switchOnInfo.width * switchOnInfo.height * 2;
        uint32_t srcLen = switchOnInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, switchOnInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }

    const DynamicBitmapDatabase::BitmapInfo switchOffInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_SWITCH_OFF_INT_ID);
    //create dynamic bitmap matching file dimensions
    switchOffId = Bitmap::dynamicBitmapCreate(switchOffInfo.width, switchOffInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", switchOffId);
    if (switchOffId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(switchOffId);
        uint32_t destLen = switchOffInfo.width * switchOffInfo.height * 2;
        uint32_t srcLen = switchOffInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, switchOffInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }

    if (onState) {
        button.setBitmaps(Bitmap(switchOnId), Bitmap(switchOffId));
    } else {
        button.setBitmaps(Bitmap(switchOffId), Bitmap(switchOnId));
    }
#endif

    button.setAction(onToggleButtonClicked);
    button.setPosition(CommonUI::TOGGLE_BUTTON_X, CommonUI::TOGGLE_BUTTON_Y, CommonUI::TOGGLE_BUTTON_WIDTH, CommonUI::TOGGLE_BUTTON_HEIGHT);
    add(button);

    if (btPowerSwitching) {
        button.setTouchable(false);
    }

#ifdef SIMULATOR
    gotoSettingButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoSettingButton.setXY(CommonUI::BACK_BUTTON_X, CommonUI::BACK_BUTTON_Y);
    add(gotoSettingButton);
#endif
}

void SetBluetoothView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (backgroundId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(backgroundId);
    }
    if (switchOnId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(switchOnId);
    }
    if (switchOffId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(switchOffId);
    }
#endif
}

bool SetBluetoothView::btPowerSignal = false;
bool SetBluetoothView::btPowerSwitching = false;

void SetBluetoothView::handleTickEvent()
{
#ifndef SIMULATOR
    if (btPowerSignal == true) {
        button.setTouchable(true);
        btPowerSignal = false;
    }
#endif
}

void SetBluetoothView::handleKeyEvent(uint8_t key)
{
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    presenter->backOptionSettingSelected();
}

void SetBluetoothView::toggleButtonClickedHandler(const AbstractButton &button)
{
    onState = !onState;
    if (onState) {
#ifndef SIMULATOR
        LOG_I(tgfx, "bt_power_on\r\n");
        btPowerSwitching = true;
        bt_power_on((bt_bd_addr_ptr_t) bt_get_public_addr(), (bt_bd_addr_ptr_t) bt_get_random_addr());
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
#endif
        buttonTxt.setTypedText(TypedText(T_ON_TEXT));
    } else {
#ifndef SIMULATOR
        LOG_I(tgfx, "bt_power_off\r\n");
        btPowerSwitching = true;
        bt_power_off();
#endif
        buttonTxt.setTypedText(TypedText(T_OFF_TEXT));
    }
    buttonTxt.invalidate();
    this->button.setTouchable(false);
}

void SetBluetoothView::init()
{
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM, (void*) &SetBluetoothView::btCallbackManagerEventCallback);
}

bt_status_t SetBluetoothView::btCallbackManagerEventCallback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_ON_CNF:
        {
            LOG_I(tgfx, "BT_POWER_ON_CNF\r\n");
            btPowerSignal = true;
            btPowerSwitching = false;
            CommonService::setBluetoothStatus(true);
        }
        break;

        case BT_POWER_OFF_CNF:
        {
            LOG_I(tgfx, "BT_POWER_OFF_CNF\r\n");
            btPowerSignal = true;
            btPowerSwitching = false;
            CommonService::setBluetoothStatus(false);
        }
        break;

        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}
