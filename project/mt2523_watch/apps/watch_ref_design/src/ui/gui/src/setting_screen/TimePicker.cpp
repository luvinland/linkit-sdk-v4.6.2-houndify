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

#include <gui/setting_screen/TimePicker.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/EasingEquations.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <gui/common/CommonUI.hpp>

TimePicker::TimePicker() :
    onSelectedElementChanged(this, &TimePicker::selectedElementChangedHandler)
{
    colortype normalTextColor = Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF);
    colortype selectedTextColor = Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF);
    colortype selectedBackgroundColor = Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF);

    hours.setXY(CommonUI::DATETIME_HOURS_X, CommonUI::DATETIME_HOURS_Y);
    hours.setup(CommonUI::DATETIME_HOURS_WIDTH, CommonUI::DATETIME_HOURS_HEIGHT, 0, 80, T_DATEPICKER_HOURS);
    hours.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 80, 40);
    hours.setElementSelectedCallback(onSelectedElementChanged);
    add(hours);

    minutes.setXY(CommonUI::DATETIME_MINUTES_X, CommonUI::DATETIME_MINUTES_Y);
    minutes.setup(CommonUI::DATETIME_MINUTES_WIDTH, CommonUI::DATETIME_HOURS_HEIGHT, 0, 80, T_DATEPICKER_MINUTES);
    minutes.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 80, 40);
    minutes.setElementSelectedCallback(onSelectedElementChanged);
    add(minutes);

    glassOverlay.setBitmap(Bitmap(BITMAP_TIMEPICKER_GLASS_OVERLAY_INT_ID));
    glassOverlay.setXY(CommonUI::DATETIME_OVERLAY_X, CommonUI::DATETIME_OVERLAY_Y);
    add(glassOverlay);
}

TimePicker::~TimePicker()
{
}

void TimePicker::setHour(int index, int duration, EasingEquation equation)
{
    hours.setSelectedIndex(index, duration, equation);
}

int TimePicker::getHour()
{
    return hours.getSelectedIndex();
}

void TimePicker::setMinute(int index, int duration, EasingEquation equation)
{
    minutes.setSelectedIndex(index, duration, equation);
}

int TimePicker::getMinute()
{
    return minutes.getSelectedIndex();
}

void TimePicker::reset()
{
    hours.reset();
    minutes.reset();
}

void TimePicker::selectedElementChangedHandler(const WheelSelector &wheel, const int &index)
{
}
