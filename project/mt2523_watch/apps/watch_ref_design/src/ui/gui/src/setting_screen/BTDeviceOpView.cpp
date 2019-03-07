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

#include <gui/setting_screen/BTDeviceOpView.hpp>
#include <BitmapDatabase.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>

#ifndef SIMULATOR
#include "bt_connection_app.h"
#include "syslog.h"
#else
#include <gui/model/BTConnections.hpp>
#endif

BTDeviceOpView::~BTDeviceOpView()
{
}

void BTDeviceOpView::setupScreen()
{
#ifndef SIMULATOR
    LOG_I(tgfx, "Current UI: Settings BT Device Operation\r\n");
#endif

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

    gotoBTSettingButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoBTSettingButton.setXY(10, 5);
    add(gotoBTSettingButton);
#endif
#ifdef SIMULATOR
    if (STATE_SEARCHING == BTConnections::getInstance()->getState())
#else
    if (BT_CONNECTION_APP_STATE_START_SEARCH == bt_connection_app_get_state())
#endif
    {
        titleTxt.setTypedText(TypedText(T_BT_ADD_NEW_DEVICE));
        titleTxt.setXY(98, 40);
        contentTxt.setTypedText(TypedText(T_BT_SEARCHING));
#ifndef SIMULATOR
		printf("[Yy]BTDeviceOpView::setupScreen\n");

		bt_connection_app_set_state(BT_CONNECTION_APP_STATE_SEARCHING);
        bt_connection_app_scan();
#endif
    }
#ifdef SIMULATOR
    else if (STATE_PAIRING == BTConnections::getInstance()->getState())
#else
    else if (BT_CONNECTION_APP_STATE_PAIRING == bt_connection_app_get_state())
#endif
    {
        titleTxt.setTypedText(TypedText(T_BT_CONNECTION_TITLE));
        titleTxt.setXY(108, 40);
        contentTxt.setTypedText(TypedText(T_BT_CONNECTING));
    }

#ifndef SIMULATOR
    else if (BT_CONNECTION_APP_STATE_CONNECTION_FAIL == bt_connection_app_get_state())
    {
        titleTxt.setTypedText(TypedText(T_BT_CONNECTION_TITLE));
        titleTxt.setXY(108, 40);
        contentTxt.setTypedText(TypedText(T_BT_CONNECTION_FAIL));
        tickcounter = 0;
    }
#endif

#ifdef SIMULATOR
    else if (STATE_DISCONNECTING == BTConnections::getInstance()->getState())
#else
    else if (BT_CONNECTION_APP_STATE_DISCONNECTING == bt_connection_app_get_state())
#endif
    {
        titleTxt.setTypedText(TypedText(T_BT_DISCONNECT));
        titleTxt.setXY(124, 35);
        contentTxt.setTypedText(TypedText(T_BT_DISCONNECT));
        // should convert string to three line.
    }
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    add(titleTxt);
    contentTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    contentTxt.setXY(128, 150);
    add(contentTxt);

    /* if (STATE_SEARCHING == BTConnections::getInstance()->getState()
         || STATE_PAIRING == BTConnections::getInstance()->getState()) */
#ifdef SIMULATOR
    if (STATE_SEARCHING == BTConnections::getInstance()->getState())
#else
    if (BT_CONNECTION_APP_STATE_SEARCHING == bt_connection_app_get_state())
#endif
    {

#ifndef DYNAMIC_BITMAP_LOADER

        btnSearchCancel.setBitmaps(Bitmap(BITMAP_BTN_CANCEL_LONG_INT_ID), Bitmap(BITMAP_BTN_CANCEL_LONG_INT_ID));
#else
        const DynamicBitmapDatabase::BitmapInfo btnlongInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_BTN_CANCEL_LONG_INT_ID);
        //create dynamic bitmap matching file dimensions
        btnlongId = Bitmap::dynamicBitmapCreate(btnlongInfo.width, btnlongInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", btnlongId);
        if (btnlongId != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(btnlongId);
            uint32_t destLen = btnlongInfo.width * btnlongInfo.height * 2;
            uint32_t srcLen = btnlongInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, btnlongInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
            btnSearchCancel.setBitmaps(Bitmap(btnlongId), Bitmap(btnlongId));
        }
#endif

        btnSearchCancel.setXY(75, 260);
        btnSearchCancel.setAction(buttonClickedCallback);
        add(btnSearchCancel);
        cancelTxt.setTypedText(TypedText(T_CANCEL_TEXT));
        cancelTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
        cancelTxt.setXY(157, 270);
        add(cancelTxt);
    }
#ifdef SIMULATOR
    else if (STATE_DISCONNECTING == BTConnections::getInstance()->getState())
#else
    else if (BT_CONNECTION_APP_STATE_DISCONNECTING == bt_connection_app_get_state())
#endif
    {


#ifndef DYNAMIC_BITMAP_LOADER

        btnDisCancel.setBitmaps(Bitmap(BITMAP_BTN_CANCEL_SHORT_INT_ID), Bitmap(BITMAP_BTN_CANCEL_SHORT_INT_ID));
#else
        const DynamicBitmapDatabase::BitmapInfo btnshortInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_BTN_CANCEL_SHORT_INT_ID);
        //create dynamic bitmap matching file dimensions
        btnshortId = Bitmap::dynamicBitmapCreate(btnshortInfo.width, btnshortInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", btnshortId);
        if (btnshortId != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(btnshortId);
            uint32_t destLen = btnshortInfo.width * btnshortInfo.height * 2;
            uint32_t srcLen = btnshortInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, btnshortInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
            btnDisCancel.setBitmaps(Bitmap(btnshortId), Bitmap(btnshortId));
        }
#endif
        btnDisCancel.setXY(70, 255);
        btnDisCancel.setAction(buttonClickedCallback);
        add(btnDisCancel);
        cancelTxt.setTypedText(TypedText(T_CANCEL_TEXT));
        cancelTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
        cancelTxt.setXY(87, 265);
        add(cancelTxt);
#ifndef DYNAMIC_BITMAP_LOADER

        btnYes.setBitmaps(Bitmap(BITMAP_BTN_YES_SHORT_INT_ID), Bitmap(BITMAP_BTN_YES_SHORT_INT_ID));

#else
        const DynamicBitmapDatabase::BitmapInfo btnyesInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_BTN_YES_SHORT_INT_ID);
        //create dynamic bitmap matching file dimensions
        btnyesId = Bitmap::dynamicBitmapCreate(btnyesInfo.width, btnyesInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", btnyesId);
        if (btnyesId != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(btnyesId);
            uint32_t destLen = btnyesInfo.width * btnyesInfo.height * 2;
            uint32_t srcLen = btnyesInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, btnyesInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
            btnYes.setBitmaps(Bitmap(btnyesId), Bitmap(btnyesId));
        }
#endif

        btnYes.setXY(200, 255);
        btnYes.setAction(buttonClickedCallback);
        add(btnYes);
        YesTxt.setTypedText(TypedText(T_YES_TEXT));
        YesTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
        YesTxt.setXY(237, 265);
        add(YesTxt);
    }
}

void BTDeviceOpView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (backgroundId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(backgroundId);
    }
    if (btnlongId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(btnlongId);
    }
    if (btnshortId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(btnshortId);
    }
    if (btnyesId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(btnyesId);
    }
#endif
}

void BTDeviceOpView::handleKeyEvent(uint8_t key)
{
    /*
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }
    */

    DemoView::handleKeyEvent(key);

    presenter->backOptionSettingSelected();
}

void BTDeviceOpView::buttonClicked(const AbstractButton &source)
{
    LOG_I(BT_CONNECTION, "yy buttonClicked %0x,%0x, %0x, %0x \r\n",
       &btnDisCancel,  &btnSearchCancel, &btnYes, &source);
    /* Determine which button was pressed, and let
    presenter decide how to react on specific action */
    if (&btnDisCancel == &source) {
        LOG_I(BT_CONNECTION, "yy btnDisCancel \r\n");
        presenter->backBTConnectionView();
    } else if (&btnSearchCancel == &source) {
        LOG_I(BT_CONNECTION, "yy btnSearchCancel \r\n");
#ifndef SIMULATOR
        bt_connection_app_cancel_search();
        presenter->backBTConnectionView();
#else
        presenter->gotoBTDevSearchListView();
#endif
    } else if (&btnYes == &source) {

        LOG_I(BT_CONNECTION, "yy btnYes\r\n");
         
	    bt_connection_disconnecct_a2dp_source();

		//presenter->backBTConnectionView();
    }
}

void BTDeviceOpView::handleTickEvent()
{
	//search list update.
    if (bt_connection_search_inqured() == true) {
        presenter->gotoBTDevSearchListView();
        bt_connection_search_inqured_flag(false);
        return;
    }

	// connect result update 
	if (BT_CONNECTION_APP_STATE_PAIRING == bt_connection_app_get_state()) {
		if(bt_connection_app_get_connection_result() == true) {
			bt_connection_app_set_connection_result_flag(false);
            bt_connection_app_set_state(BT_CONNECTION_APP_STATE_CONNECTED);
			presenter->backBTConnectionView();
		}
        return;
	}

    // disconnect result update 
	if (BT_CONNECTION_APP_STATE_DISCONNECTING == bt_connection_app_get_state()) {
		if(bt_connection_app_get_disconnection_result() == true) {
			bt_connection_app_set_disconnection_result_flag(false);
            bt_connection_app_set_state(BT_CONNECTION_APP_STATE_DISCONNECTED);
			presenter->backBTConnectionView();
		}
        return;
	}

}


