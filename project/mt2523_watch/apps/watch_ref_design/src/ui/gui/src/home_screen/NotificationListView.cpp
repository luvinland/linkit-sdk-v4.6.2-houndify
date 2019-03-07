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

#include <gui/home_screen/NotificationListView.hpp>
#include <BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <gui/common/CommonUI.hpp>
#include <touchgfx/Color.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>

#ifndef SIMULATOR
#include "bt_notify_app_list.h"
#include "syslog.h"
#endif

NotificationListView::NotificationListView()
{
    memset(notiListContentBuffer, 0, sizeof(notiListContentBuffer));
    memset(notiListContentBuffer1, 0, sizeof(notiListContentBuffer1));
}

void NotificationListView::setupScreen()
{
#ifndef SIMULATOR
    LOG_I(NOTIFY_APP, "details screen setup\r\n");
    LOG_I(tgfx, "Current UI: Notifications Message Detail\r\n");
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
    titleTxt.setTypedText(TypedText(T_NOTIFICATIONS_DETAILS_TITLE));
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    titleTxt.setPosition(0, CommonUI::TITLE_Y, HAL::DISPLAY_WIDTH, CommonUI::TITLE_FONT_HEIGHT);
    titleTxt.setAlpha(127);
    add(titleTxt);

#ifdef SIMULATOR
    Unicode::snprintf(notiListContentBuffer, 150, "%s", L"111 1111111111112222222222 22222233333333333333444444444444444445555555555555555566666666666666666667777777777777778888888888888889999999999999991111111111122222222222222333333333333333334444444444444445555555555555555");
#else
    bt_notify_app_list_t *list_item = bt_notify_app_list_get_item();
    LOG_I(NOTIFY_APP, "type:%d\r\n", list_item->notification_type);

    switch (list_item->notification_type) {
        case BT_NOTIFY_APP_LIST_NOTIFICATION:  {
            Unicode::snprintf(notiListContentBuffer, 150, "%s content:%s sender:%s action_name:%s timestamp:%d.",
                              list_item->notification.title,
                              list_item->notification.content,
                              list_item->notification.sender,
                              list_item->notification.action_content[0].action_name,
                              list_item->notification.timestamp);
        }
        break;
        case BT_NOTIFY_APP_LIST_SMS:  {
            Unicode::snprintf(notiListContentBuffer, 300, "sender_number:%d sender:%s title:%s content:%s timestamp:%d.",
                              list_item->sms.sender_number,
                              list_item->sms.sender,
                              list_item->sms.title,
                              list_item->sms.content,
                              list_item->sms.timestamp);
        }
        break;
        case BT_NOTIFY_APP_LIST_MISSED_CALL:  {
            Unicode::snprintf(notiListContentBuffer, 300, "missed_call_count:%d sender:%s sender_number:%s timestamp:%d.",
                              list_item->missed_call.missed_call_count,
                              list_item->missed_call.sender,
                              list_item->missed_call.sender_number,
                              list_item->missed_call.timestamp);
        }
        break;
        case BT_NOTIFY_APP_LIST_ANCS:
            Unicode::snprintf(notiListContentBuffer, 300, "%s", list_item->data.data, list_item->data.timestamp);
            break;
        default:
            break;
    }
#endif

#ifndef SIMULATOR
    LOG_I(NOTIFY_APP, "1details: %s\r\n", notiListContentBuffer);
#endif

    for (int i = 1;; i++) {
        int array_size = 0;
        while (notiListContentBuffer[array_size] != '\0') {
            array_size++;
        }
        if (array_size > 16) {
            Unicode::strncpy(notiListContentBuffer1 + ((i - 1) * 16) + (i - 1), notiListContentBuffer, 16);
            Unicode::strncpy(notiListContentBuffer1 + (i * 16) + (i - 1), "\n", 2);
            Unicode::strncpy(notiListContentBuffer, notiListContentBuffer + 16, sizeof(notiListContentBuffer) - 16);
        } else {
            Unicode::strncpy(notiListContentBuffer1 + ((i - 1) * 16) + (i - 1), notiListContentBuffer, array_size);
            Unicode::strncpy(notiListContentBuffer1 + ((i - 1) * 16) + (i - 1) + array_size, "\0", 2);
            break;
        }
    }

#ifndef SIMULATOR
    LOG_I(NOTIFY_APP, "2details: %s\r\n", notiListContentBuffer1);
#endif

    notiListContent.setWildcard(notiListContentBuffer1);
    notiListContent.setTypedText(TypedText(T_NOTIFICATIONS_ITEM));
    notiListContent.setPosition(60, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);
    notiListContent.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT - CommonUI::TITLE_HEIGHT - CommonUI::CLIENT_Y - 50);
    scrollCnt.add(notiListContent);
    add(scrollCnt);

#ifdef SIMULATOR
    gotoHomeButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoHomeButton.setXY(CommonUI::BACK_BUTTON_X, CommonUI::BACK_BUTTON_Y);
    add(gotoHomeButton);
#endif
}

void NotificationListView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (backgroundId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(backgroundId);
    }
#endif
}

void NotificationListView::handleKeyEvent(uint8_t key)
{
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    presenter->backOptionHomeSelected();
}

