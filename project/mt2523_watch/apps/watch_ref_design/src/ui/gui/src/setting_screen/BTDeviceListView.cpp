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


#include <gui/setting_screen/BTDeviceListView.hpp>
#include <BitmapDatabase.hpp>
#include <touchgfx/Color.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>
#include <gui/common/CommonUI.hpp>
#ifndef SIMULATOR
#include "bt_connection_app.h"
#include "syslog.h"
#else
#include <gui/model/BTConnections.hpp>
#endif
BTDeviceListView::~BTDeviceListView()
{
}

void BTDeviceListView::setupScreen()
{
    int i;
    uint32_t size = 0, result = 0;
    uint8_t tmp_buf[3] = {0};
    uint8_t buffer[18] = {0};
    uint32_t index = 0;

#ifndef SIMULATOR
    LOG_I(tgfx, "Current UI: Settings BT Device List\r\n");
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
#ifdef SIMULATOR

    gotoSettingButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoSettingButton.setXY(10, 5);
    add(gotoSettingButton);
#endif
    // Add title
    titleTxt.setTypedText(TypedText(T_BT_CONNECTION_TITLE));
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    titleTxt.setPosition(0, CommonUI::TITLE_Y, HAL::DISPLAY_WIDTH, CommonUI::TITLE_FONT_HEIGHT);
    titleTxt.setAlpha(127);
    add(titleTxt);

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
    // element 0 is the button to 'add new device'.
    listElements[0].setupListElement(0, Bitmap(menuId), Bitmap(BITMAP_BG_BTCONNECTION_ITEM_NEWDEVICE_INT_ID), TypedText(T_BT_NEW_DEVICE));
    m_style[0] = BTDeviceListView::STYLE_NEW_DEVICE;
    // get connected device from BTConnections
    int32_t connectedNum = (int32_t)bt_connection_app_get_connected_list_item_count();
    int elementNum = 0;

    for (i = 0; i < connectedNum; ++i) {
        Unicode::UnicodeChar truncate[30] = { 0 };
        Unicode::UnicodeChar tail[30] = { '.', '.', '.' };

        bt_connection_app_connected_list_t *conn_dev = bt_connection_app_get_connected_list(i);

        int array_size = bt_connection_app_get_arry_size(conn_dev->dev_name);

        LOG_I(BT_CONNECTION, "array_size: %d\r\n", array_size);
         
        if (array_size > 11) {
            Unicode::strncpy(truncate, conn_dev->dev_name, 8);
            Unicode::snprintf(devNameBuffer[i], 30, "%s%s", truncate, tail);
        } else {
            Unicode::strncpy(devNameBuffer[i], conn_dev->dev_name, 30);

            LOG_I(BT_CONNECTION, "conn_list->name: %s\r\n", conn_dev->dev_name);
            LOG_I(BT_CONNECTION, "conn_list->devNameBuffer[i]: %s\r\n", devNameBuffer[i]);
        }

        devName[i].setWildcard(devNameBuffer[i]);
        devName[i].setTypedText(TypedText(T_BT_DEVICE_NAME));

        listElements[i + 1].setupListElementEx(i + 1,
                Bitmap(menuId),
                Bitmap(BITMAP_LIST_MENU_OVERLAY_INT_ID),
                &devName[i]);
        m_style[i + 1] = BTDeviceListView::STYLE_CONNECTED_DEVICE;

    }

    // get paired device from BTConnections db

    // read btcm paired address from nvram
    size = 12;
    memset(g_bt_connection_app_paired_list, 0 , 
           sizeof(bt_connection_app_pair_list_t) * BT_CONNECTION_APP_PAIR_LIST_MAX_NUMBER);
    result = nvdm_read_data_item(TOUCHGFX_NVDM_GROUP, 
                              BLUETOOTH_ADDR_KEY, 
                             (uint8_t *)buffer, 
                             &size);
    //p_bt_addr = &(g_bt_connection_app_paired_list[0].bt_addr);
    if(NVDM_STATUS_OK == result) {
      for (index = 0; index < 6; ++index) {
         tmp_buf[0] = buffer[2 * index];
         tmp_buf[1] = buffer[2 * index + 1];
         g_bt_connection_app_paired_list[0].bt_addr[index] = (uint8_t)strtoul((char *)tmp_buf, NULL, 16);
         Unicode::snprintf(g_bt_connection_app_paired_list[0].dev_name,
                BT_CONNECTION_APP_DEVICE_NAME_LENGTH,
                "%2X:%2X:%2X%:%2X:%2X%:%2X",
                g_bt_connection_app_paired_list[0].bt_addr[5],
                g_bt_connection_app_paired_list[0].bt_addr[4],
                g_bt_connection_app_paired_list[0].bt_addr[3],
                g_bt_connection_app_paired_list[0].bt_addr[2],
                g_bt_connection_app_paired_list[0].bt_addr[1],
                g_bt_connection_app_paired_list[0].bt_addr[0]);
    }             
        g_bt_connection_cntx_t.pair_list_item_count = 1;
    }else{
        g_bt_connection_cntx_t.pair_list_item_count = 0;
    }
    LOG_I(tgfx, "result = %d, btaddr = %2X:%2X:%2X%:%2X:%2X%:%2X, size = %d\r\n",
              result, 
              g_bt_connection_app_paired_list[0].bt_addr[0],
              g_bt_connection_app_paired_list[0].bt_addr[1],
              g_bt_connection_app_paired_list[0].bt_addr[2],
              g_bt_connection_app_paired_list[0].bt_addr[3],
              g_bt_connection_app_paired_list[0].bt_addr[4],
              g_bt_connection_app_paired_list[0].bt_addr[5],
              size);

    LOG_I(tgfx, "connected = %2X:%2X:%2X%:%2X:%2X%:%2X\r\n",
              g_bt_connection_app_connected_list[0].bt_addr[0],
              g_bt_connection_app_connected_list[0].bt_addr[1],
              g_bt_connection_app_connected_list[0].bt_addr[2],
              g_bt_connection_app_connected_list[0].bt_addr[3],
              g_bt_connection_app_connected_list[0].bt_addr[4],
              g_bt_connection_app_connected_list[0].bt_addr[5]);

    //if contains in connected device need to remove
    // need to update paired list if they are same
    if (memcmp(g_bt_connection_app_paired_list[0].bt_addr,
               g_bt_connection_app_connected_list[0].bt_addr, 
               sizeof(bt_bd_addr_t)) == 0) {
        memset(g_bt_connection_app_paired_list, 0 , 
               sizeof(bt_connection_app_pair_list_t) * BT_CONNECTION_APP_PAIR_LIST_MAX_NUMBER);
        g_bt_connection_cntx_t.pair_list_item_count = 0;
    }
        
    int32_t pairedNum = connectedNum + bt_connection_app_get_pair_list_item_count();

    for (i = connectedNum; i < pairedNum; ++i) {
        Unicode::UnicodeChar truncate[30] = { 0 };
        Unicode::UnicodeChar tail[30] = { '.', '.', '.' };

        bt_connection_app_pair_list_t *paired_dev = bt_connection_app_get_paired_list(i);

        int array_size = bt_connection_app_get_arry_size(paired_dev->dev_name);

        if (array_size > 11) {
            Unicode::strncpy(truncate, paired_dev->dev_name, 8);
            Unicode::snprintf(devNameBuffer[i], 30, "%s%s", truncate, tail);
        } else {
            Unicode::strncpy(devNameBuffer[i], paired_dev->dev_name, 30);

            LOG_I(BT_CONNECTION, "paired_dev->name: %s\r\n", paired_dev->dev_name);
            LOG_I(BT_CONNECTION, "paired_dev->devNameBuffer[i]: %s\r\n", devNameBuffer[i]);
        }

        devName[i].setWildcard(devNameBuffer[i]);
        devName[i].setTypedText(TypedText(T_BT_DEVICE_NAME));

        listElements[i + 1].setupListElementEx(i + 1,
                Bitmap(menuId),
                Bitmap(BITMAP_BG_BTCONNECTION_ITEM_NEWDEVICE_INT_ID),
                &devName[i]);
        m_style[i + 1] = BTDeviceListView::STYLE_PAIRED_DEVICE;

    }

	elementNum = pairedNum + 1;
    listElements[elementNum].setupLastElement();
    for (i = 0; i < elementNum; ++i) {
        listElements[i].setAction(listElementClickedCallback);
        list.add(listElements[i]);
    }

    if (elementNum == 1) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);
    } else if (elementNum == 2) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 2 * CommonUI::CLIENT_ITEM_HEIGHT + 2 * CommonUI::CLIENT_ITEM_GAP);
    } else if (elementNum >= 3) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT - 2 * CommonUI::TITLE_HEIGHT - 2 * CommonUI::CLIENT_Y);
    }
    scrollCnt.add(list);
    add(scrollCnt);
}

void BTDeviceListView::tearDownScreen()
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

void BTDeviceListView::listElementClicked(MenuListElement &element)
{
    // The button of the list element has been pressed
    // so it is removed from the list
    // to check how to deal with this element.
    BTN_STYLE btn = getElementStyle(element.getMenuIndex());
	
    LOG_I(BT_CONNECTION, "yy BTN_STYLE: %d\r\n", btn);
        
    if (BTDeviceListView::STYLE_NEW_DEVICE == btn) {
        // init scan	
		bt_connection_search_data_init();
        
		bt_connection_search_inqured_flag(true);
        
        bt_connection_app_set_state(BT_CONNECTION_APP_STATE_START_SEARCH);
        
        presenter->gotoBTDevOptionView();
        
    } else if (BTDeviceListView::STYLE_CONNECTED_DEVICE == btn) {

        LOG_I(BT_CONNECTION, "BTDeviceListView::STYLE_CONNECTED_DEVICE == btn\r\n");
        
        bt_connection_search_inqured_flag(false);
        
        bt_connection_app_set_state(BT_CONNECTION_APP_STATE_DISCONNECTING);
        
	    presenter->gotoBTDevOptionView();	

    } else if(BTDeviceListView::STYLE_PAIRED_DEVICE == btn) {
        LOG_I(BT_CONNECTION, "BTDeviceListView::STYLE_PAIRED_DEVICE == btn\r\n"); 
        if( (BT_CONNECTION_APP_STATE_START_SEARCH == bt_connection_app_get_state())
            || (BT_CONNECTION_APP_STATE_SEARCHING == bt_connection_app_get_state()))  {
            bt_connection_app_cancel_search();
        }

        bt_connection_search_inqured_flag(false);
        
        bt_connection_connect_a2dp();
	
        bt_connection_app_set_state(BT_CONNECTION_APP_STATE_PAIRING);
    
	    presenter->gotoBTDevOptionView();
    }

}

void BTDeviceListView::handleKeyEvent(uint8_t key)
{
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    presenter->backOptionSettingSelected();
}


void BTDeviceListView::handleTickEvent()
{
	//search list update.
    if (bt_connection_get_status_from_pait_to_connect() == true) {       
        bt_connection_set_status_from_pait_to_connect(false);
        LOG_I(BT_CONNECTION, "BTDeviceListView update\r\n");
        presenter->backBTConnectionView();        
        return;
    }
}


