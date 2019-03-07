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

#include <gui/setting_screen/MenuListElement.hpp>
#include <BitmapDatabase.hpp>
#include <touchgfx/Color.hpp>
#include <gui/common/CommonUI.hpp>
#include <touchgfx/hal/HAL.hpp>
#ifndef SIMULATOR
#include "FreeRTOS.h"
#endif

MenuListElement::MenuListElement()
    : Container()
{
    menuIndex = 0;
    clickAbort = false;
    viewCallback = NULL;
}

void MenuListElement::setupListElement(uint8_t index, const Bitmap &bg, const Bitmap &bmp, TypedText t)
{
    menuIndex = index;

    // Setup background
    background.setBitmap(bg);
    background.setXY(CommonUI::CLIENT_X, 0);

    image.setBitmap(bmp);
    image.setXY(CommonUI::CLIENT_X + CommonUI::CLIENT_ITEM_HEIGHT / 4, CommonUI::CLIENT_ITEM_HEIGHT / 4);

    // Setup text
    listTxt.setTypedText(t);
    listTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    listTxt.setXY(CommonUI::CLIENT_ITEM_TEXT_X, CommonUI::CLIENT_ITEM_TEXT_Y);

    // Setup MenuListElement dimensions
    setWidth(CommonUI::CLIENT_ITEM_WIDTH + 2 * CommonUI::CLIENT_X);
    setHeight(CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);

    add(background);
    add(image);
    add(listTxt);

    setTouchable(true);
}

void MenuListElement::setupListElement(uint8_t index, const Bitmap &bg, const Bitmap &bmp, TextAreaWithOneWildcard *t)
{
    menuIndex = index;

    // Setup background
    background.setBitmap(bg);
    background.setXY(CommonUI::CLIENT_X, 0);

    image.setBitmap(bmp);
    image.setXY(CommonUI::CLIENT_X + CommonUI::CLIENT_ITEM_HEIGHT / 4, CommonUI::CLIENT_ITEM_HEIGHT / 4);

    // Setup text
    t->setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    t->setXY(CommonUI::CLIENT_ITEM_TEXT_X, CommonUI::CLIENT_ITEM_TEXT_Y);

    // Setup MenuListElement dimensions
    setWidth(CommonUI::CLIENT_ITEM_WIDTH + 2 * CommonUI::CLIENT_X);
    setHeight(CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);

    add(background);
    add(image);
    add(*t);

    setTouchable(true);
}

void MenuListElement::setupListElementEx(uint8_t index, const Bitmap &bg, const Bitmap &bmp, TextAreaWithOneWildcard *t)
{
    menuIndex = index;

    // Setup background
    background.setBitmap(bg);
    background.setXY(CommonUI::CLIENT_X, 0);

    image.setBitmap(bmp);
    image.setXY(CommonUI::CLIENT_X, 0);

    // Setup text
    t->setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    t->setXY(CommonUI::CLIENT_ITEM_TEXT_X, CommonUI::CLIENT_ITEM_TEXT_Y);

    // Setup MenuListElement dimensions
    setWidth(CommonUI::CLIENT_ITEM_WIDTH + 2 * CommonUI::CLIENT_X);
    setHeight(CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);

    add(background);
    add(image);
    add(*t);

    setTouchable(true);
}

void MenuListElement::setupLastElement()
{
    // Setup MenuListElement dimensions
    //setHeight(CommonUI::CLIENT_ITEM_LAST_HEIGHT + CommonUI::CLIENT_ITEM_GAP);
    //setWidth(HAL::DISPLAY_WIDTH);

    //setTouchable(false);
}

void MenuListElement::removeListElement(uint8_t index, const Bitmap &bmp, TextAreaWithOneWildcard *t)
{
    remove(background);
    remove(image);
    remove(*t);

    setTouchable(false);
}

void MenuListElement::handleClickEvent(const ClickEvent &evt)
{
    // Inform the view of the event
    if (viewCallback->isValid() && evt.getType() == ClickEvent::RELEASED) {
        if (clickAbort == false) {
            viewCallback->execute(*this);
        } else {
            clickAbort = false;
        }
    }
}

void MenuListElement::handleDragEvent(const DragEvent &evt)
{
    if (abs(evt.getDeltaY()) > 0) {
        clickAbort = true;
    }
}
