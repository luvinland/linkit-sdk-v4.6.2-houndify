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

#include <gui/setting_screen/SetClockView.hpp>
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

#ifndef CLOCK_LIST_STYLE

SetClockView::SetClockView() :
    DemoView<SetClockPresenter>(),
    slideMenuElementSelectedCallback(this, &SetClockView::slideMenuElementSelectedHandler),
    onButtonPressed(this, &SetClockView::buttonPressedHandler)
{
}

SetClockView::~SetClockView()
{
}

void SetClockView::setupScreen()
{
#ifndef SIMULATOR
    LOG_I(tgfx, "Current UI: Settings Set Clock\r\n");
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
    titleTxt.setTypedText(TypedText(T_CLOCK_STYLE_TITLE));
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    titleTxt.setPosition(0, CommonUI::TITLE_Y, HAL::DISPLAY_WIDTH, CommonUI::TITLE_FONT_HEIGHT);
    titleTxt.setAlpha(127);
    add(titleTxt);

    // Initialize slide menu elements
#ifndef DYNAMIC_BITMAP_LOADER
    BitmapId id[] = {BITMAP_CLOCKSTYLE_CLOCK1_ACTIVE_INT_ID, BITMAP_CLOCKSTYLE_CLOCK2_ACTIVE_INT_ID, BITMAP_CLOCKSTYLE_CLOCK3_ACTIVE_INT_ID};
    for (int i = 0; i < numberOfElements; i++) {
        clockActiveId[i] = id[i];
    }
#else
    uint16_t index[] = {DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK1_ACTIVE_INT_ID, DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK2_ACTIVE_INT_ID, DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK3_ACTIVE_INT_ID};
    for (int i = 0; i < numberOfElements; i++) {
        const DynamicBitmapDatabase::BitmapInfo clockInfo = DynamicBitmapDatabase::getInstanceInfo(index[i]);
        //create dynamic bitmap matching file dimensions
        clockActiveId[i] = Bitmap::dynamicBitmapCreate(clockInfo.width, clockInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", clockActiveId[i]);
        if (clockActiveId[i] != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(clockActiveId[i]);
            uint32_t destLen = clockInfo.width * clockInfo.height * 2;
            uint32_t srcLen = clockInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, clockInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        }
    }
#endif

#ifndef DYNAMIC_BITMAP_LOADER
    BitmapId id2[] = {BITMAP_CLOCKSTYLE_CLOCK1_INACTIVE_INT_ID, BITMAP_CLOCKSTYLE_CLOCK2_INACTIVE_INT_ID, BITMAP_CLOCKSTYLE_CLOCK3_INACTIVE_INT_ID};
    for (int i = 0; i < numberOfElements; i++) {
        clockInactiveId[i] = id2[i];
    }
#else
    uint16_t index2[] = {DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK1_INACTIVE_INT_ID, DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK2_INACTIVE_INT_ID, DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK3_INACTIVE_INT_ID};
    for (int i = 0; i < numberOfElements; i++) {
        const DynamicBitmapDatabase::BitmapInfo clockInfo = DynamicBitmapDatabase::getInstanceInfo(index2[i]);
        //create dynamic bitmap matching file dimensions
        clockInactiveId[i] = Bitmap::dynamicBitmapCreate(clockInfo.width, clockInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", clockInactiveId[i]);
        if (clockInactiveId[i] != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(clockInactiveId[i]);
            uint32_t destLen = clockInfo.width * clockInfo.height * 2;
            uint32_t srcLen = clockInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, clockInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        }
    }
#endif

    slideMenu.setPosition(0, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);
    switch (CommonService::getClockStyle()) {
        case CommonService::ANALOG_CLOCK:
            slideMenu.setup(numberOfElements, 0);
            break;
        case CommonService::DIGITAL_CLOCK:
            slideMenu.setup(numberOfElements, 2);
            break;
        case CommonService::COMPOUND_CLOCK:
            slideMenu.setup(numberOfElements, 1);
            break;
        default:
            break;
    }
    slideMenu.setBitmapsForElement(0, clockInactiveId[0], clockActiveId[0]);
    slideMenu.setBitmapsForElement(1, clockInactiveId[1], clockActiveId[1]);
    slideMenu.setBitmapsForElement(2, clockInactiveId[2], clockActiveId[2]);
    slideMenu.setAnimationDuration(8);
    slideMenu.setElementSelectedCallback(slideMenuElementSelectedCallback);
    slideMenu.setTouchable(true);
    add(slideMenu);

    // Invisible button for navigating in the side menu
    menuLeft.setPosition(CommonUI::SLIDE_MENU_LEFT_BUTTON_X, CommonUI::SLIDE_MENU_LEFT_BUTTON_Y, CommonUI::SLIDE_MENU_LEFT_BUTTON_WIDTH, CommonUI::SLIDE_MENU_LEFT_BUTTON_HEIGHT);
    menuLeft.setAction(onButtonPressed);
    add(menuLeft);

    // Invisible button for navigating in the side menu
    menuRight.setPosition(CommonUI::SLIDE_MENU_RIGHT_BUTTON_X, CommonUI::SLIDE_MENU_RIGHT_BUTTON_Y, CommonUI::SLIDE_MENU_RIGHT_BUTTON_WIDTH, CommonUI::SLIDE_MENU_RIGHT_BUTTON_HEIGHT);
    menuRight.setAction(onButtonPressed);
    add(menuRight);

#ifdef SIMULATOR
    gotoSettingButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoSettingButton.setXY(CommonUI::BACK_BUTTON_X, CommonUI::BACK_BUTTON_Y);
    add(gotoSettingButton);
#endif

    slideMenuElementSelectedHandler(slideMenu);
}

void SetClockView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (backgroundId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(backgroundId);
    }
    for (int i = 0; i < numberOfElements; i++) {
        if (clockActiveId[i] != BITMAP_INVALID) {
            Bitmap::dynamicBitmapDelete(clockActiveId[i]);
        }
    }
    for (int i = 0; i < numberOfElements; i++) {
        if (clockInactiveId[i] != BITMAP_INVALID) {
            Bitmap::dynamicBitmapDelete(clockInactiveId[i]);
        }
    }
#endif
}

void SetClockView::handleKeyEvent(uint8_t key)
{
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    presenter->backOptionSettingSelected();
}

void SetClockView::slideMenuElementSelectedHandler(const HorizontalSlideMenu &menu)
{
    int selectedElement = slideMenu.getSelectedElementIndex();

    // Setup menu up/down button. First set default then handle special cases
    menuLeft.setTouchable(true);
    menuRight.setTouchable(true);

    if (selectedElement == 0) {
        menuLeft.setTouchable(false);
    } else if (selectedElement == slideMenu.getSize() - 1) {
        menuRight.setTouchable(false);
    }

    // Expand the active area of the buttons to make them easier to activate
    menuLeft.invalidate();
    menuRight.invalidate();

    switch (selectedElement) {
        case 0:
            CommonService::setClockStyle(CommonService::ANALOG_CLOCK);
            break;
        case 2:
            CommonService::setClockStyle(CommonService::DIGITAL_CLOCK);
            break;
        case 1:
            CommonService::setClockStyle(CommonService::COMPOUND_CLOCK);
            break;
        default:
            break;
    }
}

void SetClockView::buttonPressedHandler(const AbstractButton &button)
{
    if (&button == &menuLeft) {
        slideMenu.animateRight();
    } else if (&button == &menuRight) {
        slideMenu.animateLeft();
    }
}

#else

SetClockView::SetClockView() :
    DemoView<SetClockPresenter>(),
    listElementClickedCallback(this, &SetClockView::listElementClicked)
{
}

SetClockView::~SetClockView()
{
}

void SetClockView::setupScreen()
{
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
    titleTxt.setTypedText(TypedText(T_CLOCK_STYLE_TITLE));
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
    listElements[0].setupListElement(0, Bitmap(menuId), Bitmap(BITMAP_INVALID), TypedText(T_ANALOG_CLOCK_TEXT));
    listElements[1].setupListElement(1, Bitmap(menuId), Bitmap(BITMAP_INVALID), TypedText(T_DIGITAL_CLOCK_TEXT));
    listElements[2].setupListElement(2, Bitmap(menuId), Bitmap(BITMAP_INVALID), TypedText(T_COMPOUND_CLOCK_TEXT));
    listElements[3].setupLastElement();

    overlay.setBitmap(Bitmap(BITMAP_LIST_MENU_OVERLAY_INT_ID));
    switch (CommonService::getClockStyle()) {
        case CommonService::ANALOG_CLOCK:
            overlay.setXY(CommonUI::CLIENT_X, 0);
            break;
        case CommonService::DIGITAL_CLOCK:
            overlay.setXY(CommonUI::CLIENT_X, CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);
            break;
        case CommonService::COMPOUND_CLOCK:
            overlay.setXY(CommonUI::CLIENT_X, 2 * (CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP));
            break;
        default:
            break;
    }

    for (uint8_t i = 0; i < numberOfListElements + 1; ++i) {
        listElements[i].setAction(listElementClickedCallback);
        list.add(listElements[i]);
    }

    // Position and set the size of the scrollable container.
    // The width is the area is the list element width plus some extra
    // for space between element and scrollbar
    scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT - 2 * CommonUI::TITLE_HEIGHT - 2 * CommonUI::CLIENT_Y);
    scrollCnt.add(list);
    scrollCnt.add(overlay);
    add(scrollCnt);

#ifdef SIMULATOR
    gotoSettingButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoSettingButton.setXY(CommonUI::BACK_BUTTON_X, CommonUI::BACK_BUTTON_Y);
    add(gotoSettingButton);
#endif
}

void SetClockView::tearDownScreen()
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

void SetClockView::handleKeyEvent(uint8_t key)
{
    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    presenter->backOptionSettingSelected();
}

void SetClockView::listElementClicked(MenuListElement &element)
{
    switch (element.getMenuIndex()) {
        case 0:
            overlay.moveTo(CommonUI::CLIENT_X, scrollCnt.getContainedArea().y);
            CommonService::setClockStyle(CommonService::ANALOG_CLOCK);
            break;
        case 1:
            overlay.moveTo(CommonUI::CLIENT_X, CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP + scrollCnt.getContainedArea().y);
            CommonService::setClockStyle(CommonService::DIGITAL_CLOCK);
            break;
        case 2:
            overlay.moveTo(CommonUI::CLIENT_X, 2 * (CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP) + scrollCnt.getContainedArea().y);
            CommonService::setClockStyle(CommonService::COMPOUND_CLOCK);
            break;
        default:
            break;
    }
}

#endif
