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

#include <gui/setting_screen/BTDeviceSearchListView.hpp>
#include <BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <gui/common/CommonUI.hpp>
#include <gui/common/CommonService.hpp>
#include <touchgfx/Color.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>
#include <touchgfx/Drawable.hpp>
#ifndef SIMULATOR
#include "bt_connection_app.h"
#include "syslog.h"
#else
#include <gui/model/BTConnections.hpp>
#endif

BTDeviceSearchListView::~BTDeviceSearchListView()
{
}

void BTDeviceSearchListView::setupScreen()
{
#ifndef SIMULATOR
    LOG_I(tgfx, "Current UI: Settings BT Device Search List\r\n");
#endif

    screenContainer.setPosition(0, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);
    add(screenContainer);

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
    screenContainer.add(backgroundImage);
    //add(backgroundImage);
    // Add title
    titleTxt.setTypedText(TypedText(T_BT_CONNECTION_TITLE));
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    titleTxt.setPosition(0, CommonUI::TITLE_Y, HAL::DISPLAY_WIDTH, CommonUI::TITLE_FONT_HEIGHT);
    titleTxt.setAlpha(127);

    // Initialize list elements
#ifndef DYNAMIC_BITMAP_LOADER
    menuId = BITMAP_LIST_MENU_BACKGROUND_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo menuInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_LIST_MENU_BACKGROUND_INT_ID);
    //create dynamic bitmap matching file dimensions
    menuId = Bitmap::dynamicBitmapCreate(menuInfo.width, menuInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", menuId);
    if (menuId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(menuId);
        uint32_t destLen = menuInfo.width * menuInfo.height * 2;
        uint32_t srcLen = menuInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, menuInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif
    refreshSearchList();

    // Remember to add widgets to container.
    // They must be added from bottom and out, or else upper layer(s)
    // may block view of elements below.
    scrollCnt.add(list);
    screenContainer.add(titleTxt);
    screenContainer.add(scrollCnt);
    //add(titleTxt);
    //add(scrollCnt);
#ifdef SIMULATOR
    gotoMenuButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoMenuButton.setXY(CommonUI::BACK_BUTTON_X, CommonUI::BACK_BUTTON_Y);
    screenContainer.add(gotoMenuButton);
    //add(gotoMenuButton);
#endif
}

void BTDeviceSearchListView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (backgroundId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(backgroundId);
    }
    if (menuId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(menuId);
    }
#endif
}

void BTDeviceSearchListView::listElementClicked(MenuListElement &element)
{
    // go to pairing view.
    if( (BT_CONNECTION_APP_STATE_START_SEARCH == bt_connection_app_get_state())
        || (BT_CONNECTION_APP_STATE_SEARCHING == bt_connection_app_get_state()))  {
        bt_connection_app_cancel_search();
    }
    
    bt_connection_connect_a2dp_source(element.getMenuIndex());
	
    bt_connection_app_set_state(BT_CONNECTION_APP_STATE_PAIRING);

    presenter->gotoBTDevOptionView();
}

void BTDeviceSearchListView::handleTickEvent()
{
    if ((bt_connection_search_have_update()) == true) { //search list update.
        refreshSearchList();
        bt_connection_search_set_update_flag(false);
    }
}
void BTDeviceSearchListView::handleKeyEvent(uint8_t key)
{
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    if( (BT_CONNECTION_APP_STATE_START_SEARCH == bt_connection_app_get_state())
        || (BT_CONNECTION_APP_STATE_SEARCHING == bt_connection_app_get_state())) {
        bt_connection_app_cancel_search();
    }
    
    presenter->backBTConnectionView();// maybe not require.
}

static uint32_t list_count = 0;
void BTDeviceSearchListView::refreshSearchList()
{
	LOG_I(BT_CONNECTION, "[BT_CM]refreshSearchList\r\n");
#ifndef SIMULATOR
    Unicode::UnicodeChar truncate[30] = { 0 };
    Unicode::UnicodeChar tail[30] = { '.', '.', '.' };

    LOG_I(BT_CONNECTION, "serach_list: %d\r\n", list_count);

    if (list_count > 0) {
        for (uint8_t i = 0; i < list_count; ++i) {
            listElements[i].removeListElement(i, Bitmap(BITMAP_INVALID), &searchDeviceName[i]);
        }
        for (uint8_t i = 0; i < list_count + 1; ++i) {
            list.remove(listElements[i]);
        }
    }

    list_count = bt_connection_app_get_searched_list_item_count();

    LOG_I(BT_CONNECTION, "list_count: %d\r\n", list_count);

    if (list_count == 0) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 0);
        return;
    }

    for (uint32_t i = 0; i < list_count; i++) {
        bt_connection_app_search_list_t *search_dev = bt_connection_app_get_search_list(i);
        int array_size = bt_notify_get_arry_size(search_dev->dev_name);
		
        LOG_I(BT_CONNECTION, "array_size: %d\r\n", array_size);

        if (array_size > 11) {
            Unicode::strncpy(truncate, search_dev->dev_name, 8);
            Unicode::snprintf(searchDeviceNameBuffer[i], 30, "%s%s", truncate, tail);
        } else {
            Unicode::strncpy(searchDeviceNameBuffer[i], search_dev->dev_name, 30);

            LOG_I(BT_CONNECTION, "serach_list->name: %s\r\n", search_dev->dev_name);
            LOG_I(BT_CONNECTION, "search_list->nameBuffer[i]: %s\r\n", searchDeviceNameBuffer[i]);
        }

        searchDeviceName[i].setWildcard(searchDeviceNameBuffer[i]);
        searchDeviceName[i].setTypedText(TypedText(T_BT_DEVICE_NAME));
        listElements[i].setupListElement(i, Bitmap(menuId), Bitmap(BITMAP_INVALID), &searchDeviceName[i]);

    }
    listElements[list_count].setupLastElement();
#else
    Unicode::UnicodeChar title[30] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'D', 'S', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    Unicode::UnicodeChar truncate[30] = { 0 };
    Unicode::UnicodeChar tail[30] = { '.', '.', '.' };

    if (list_count > 0) {
        for (uint8_t i = 0; i < list_count; ++i) {
            listElements[i].removeListElement(i, Bitmap(BITMAP_INVALID), &searchDeviceName[i]);
        }
        for (uint8_t i = 0; i < list_count + 1; ++i) {
            list.remove(listElements[i]);
        }
    }

    list_count = 5;
    if (list_count == 0) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 0);
        return;
    }

    for (uint32_t i = 0; i < list_count; i++) {
        if (i == 0) {
            Unicode::strncpy(truncate, title, 11);
            Unicode::snprintf(searchDeviceNameBuffer[i], 30, "%s%s", truncate, tail);
        } else {
            Unicode::strncpy(searchDeviceNameBuffer[i], "skhdjksdf", 30);
        }

        searchDeviceName[i].setWildcard(searchDeviceNameBuffer[i]);
        searchDeviceName[i].setTypedText(TypedText(T_BT_DEVICE_NAME));

        listElements[i].setupListElement(i, Bitmap(menuId), Bitmap(BITMAP_INVALID), &searchDeviceName[i]);
    }
    listElements[list_count].setupLastElement();
#endif

    for (uint8_t i = 0; i < list_count + 1; ++i) {
        listElements[i].setAction(listElementClickedCallback);
        list.add(listElements[i]);
    }

    if (list_count == 1) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);
    } else if (list_count == 2) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 2 * CommonUI::CLIENT_ITEM_HEIGHT + 2 * CommonUI::CLIENT_ITEM_GAP);
    } else if (list_count >= 3) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT - 2 * CommonUI::TITLE_HEIGHT - 2 * CommonUI::CLIENT_Y);
    }
    screenContainer.invalidate();
}
