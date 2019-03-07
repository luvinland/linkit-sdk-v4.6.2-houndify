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

#ifndef COMMON_UI_HPP
#define COMMON_UI_HPP

#include <touchgfx/hal/HAL.hpp>

#ifdef SIMULATOR
#include <stdlib.h>
#endif

class CommonUI
{
public:
    static const int TITLE_Y = 25;
    static const int TITLE_HEIGHT = 65;
    static const int TITLE_FONT_HEIGHT = 38;

    static const int DATETIME_DAYS_X = 32;
    static const int DATETIME_DAYS_Y = 0;
    static const int DATETIME_DAYS_WIDTH = 40;
    static const int DATETIME_DAYS_HEIGHT = 200;

    static const int DATETIME_MONTHS_X = 113;
    static const int DATETIME_MONTHS_Y = 0;
    static const int DATETIME_MONTHS_WIDTH = 69;
    static const int DATETIME_MONTHS_HEIGHT = 200;

    static const int DATETIME_YEARS_X = 222;
    static const int DATETIME_YEARS_Y = 0;
    static const int DATETIME_YEARS_WIDTH = 79;
    static const int DATETIME_YEARS_HEIGHT = 200;

    static const int DATETIME_HOURS_X = 68;
    static const int DATETIME_HOURS_Y = 0;
    static const int DATETIME_HOURS_WIDTH = 40;
    static const int DATETIME_HOURS_HEIGHT = 200;

    static const int DATETIME_MINUTES_X = 220;
    static const int DATETIME_MINUTES_Y = 0;
    static const int DATETIME_MINUTES_WIDTH = 40;
    static const int DATETIME_MINUTES_HEIGHT = 200;

    static const int DATETIME_CLIENT_X = 40;
    static const int DATETIME_CLIENT_Y = 110;
    static const int DATETIME_CLIENT_WIDTH = 320;
    static const int DATETIME_CLIENT_HEIGHT = 200;

    static const int DATETIME_OVERLAY_X = 0;
    static const int DATETIME_OVERLAY_Y = 60;

    static const int CLOCK_YEAR_X = 170;
    static const int CLOCK_YEAR_Y = 110;
    static const int CLOCK_MONTH_X = 92;
    static const int CLOCK_MONTH_Y = 176;
    static const int CLOCK_DAY_X = 273;
    static const int CLOCK_DAY_Y = 178;

    static const int CLOCK_CENTERDOT_BG_X = 191;
    static const int CLOCK_CENTERDOT_BG_Y = 191;
    static const int CLOCK_CENTERDOT_X = 196;
    static const int CLOCK_CENTERDOT_Y = 196;

    static const int DIGITALCLOCK_PERCENT_X = 266;
    static const int DIGITALCLOCK_PERCENT_Y = 90;
    static const int DIGITALCLOCK_PERCENT_WIDTH = 51;
    static const int DIGITALCLOCK_PERCENT_HEIGHT = 28;

    static const int DIGITALCLOCK_CHARGER_X = 240;
    static const int DIGITALCLOCK_CHARGER_Y = 91;

    static const int DIGITALCLOCK_WEEK_X = 90;
    static const int DIGITALCLOCK_WEEK_Y = 90;
    static const int DIGITALCLOCK_TIME_X = 68;
    static const int DIGITALCLOCK_TIME_Y = 130;
    static const int DIGITALCLOCK_DATE_X = 118;
    static const int DIGITALCLOCK_DATE_Y = 270;
    static const int DIGITALCLOCK_BATTERY_X = 282;
    static const int DIGITALCLOCK_BATTERY_Y = 91;

    static const int COMPOUNDCLOCK_DAY_X = 56;
    static const int COMPOUNDCLOCK_DAY_Y = 257;

    static const int PLAYER_SONG_NAME_X = 60;
    static const int PLAYER_SONG_NAME_Y = 80;
    static const int PLAYER_SONG_NAME_WIDTH = 280;
    static const int PLAYER_SONG_NAME_HEIGHT = 76;

    static const int PLAYER_PLAY_BUTTON_X = 150;
    static const int PLAYER_PLAY_BUTTON_Y = 166;
    static const int PLAYER_PREVIOUS_BUTTON_X = 60;
    static const int PLAYER_PREVIOUS_BUTTON_Y = 186;
    static const int PLAYER_NEXT_BUTTON_X = 280;
    static const int PLAYER_NEXT_BUTTON_Y = 186;

    static const int PLAYER_PLUS_BUTTON_X = 260;
    static const int PLAYER_PLUS_BUTTON_Y = 290;
    static const int PLAYER_MINUS_BUTTON_X = 100;
    static const int PLAYER_MINUS_BUTTON_Y = 290;
    static const int PLAYER_VOLUME_BUTTON_X = 180;
    static const int PLAYER_VOLUME_BUTTON_Y = 290;
    static const int PLAYER_VOLUME_BAR_X = 100;
    static const int PLAYER_VOLUME_BAR_Y = 290;

    static const int CLIENT_X = 50;
    static const int CLIENT_Y = 10;
    static const int CLIENT_ITEM_WIDTH = 300;
    static const int CLIENT_ITEM_HEIGHT = 80;
    static const int CLIENT_ITEM_GAP = 5;
    static const int CLIENT_ITEM_LAST_HEIGHT = 30;
    static const int CLIENT_ITEM_TEXT_X = 130;
    static const int CLIENT_ITEM_TEXT_Y = 21;

    static const int BACK_BUTTON_X = 10;
    static const int BACK_BUTTON_Y = 5;
    static const int SAVE_BUTTON_X = 360;
    static const int SAVE_BUTTON_Y = 5;

    static const int TOGGLE_BUTTON_TEXT_X = 70;
    static const int TOGGLE_BUTTON_TEXT_Y = 120;
    static const int TOGGLE_BUTTON_TEXT_WIDTH = 40;
    static const int TOGGLE_BUTTON_TEXT_HEIGHT = 40;

    static const int TOGGLE_BUTTON_X = 210;
    static const int TOGGLE_BUTTON_Y = 115;
    static const int TOGGLE_BUTTON_WIDTH = 120;
    static const int TOGGLE_BUTTON_HEIGHT = 50;

    static const int SLIDE_MENU_LARGE_IMAGE_Y = 100;
    static const int SLIDE_MENU_LARGE_IMAGE_WIDTH = 240;
    static const int SLIDE_MENU_LARGE_IMAGE_HEIGHT = 240;
    static const int SLIDE_MENU_SMALL_IMAGE_Y = 120;
    static const int SLIDE_MENU_SMALL_IMAGE_WIDTH = 160;
    static const int SLIDE_MENU_SMALL_IMAGE_HEIGHT = 160;
    static const int SLIDE_MENU_IMGAE_X_DELTA = 20;

    static const int SLIDE_MENU_LEFT_BUTTON_X = 0;
    static const int SLIDE_MENU_LEFT_BUTTON_Y = 100;
    static const int SLIDE_MENU_LEFT_BUTTON_WIDTH = 80;
    static const int SLIDE_MENU_LEFT_BUTTON_HEIGHT = 200;

    static const int SLIDE_MENU_RIGHT_BUTTON_X = 320;
    static const int SLIDE_MENU_RIGHT_BUTTON_Y = 100;
    static const int SLIDE_MENU_RIGHT_BUTTON_WIDTH = 80;
    static const int SLIDE_MENU_RIGHT_BUTTON_HEIGHT = 200;

    static const int LONGITUDE_X = 25;
    static const int LONGITUDE_Y = 200;
    static const int LONGITUDE_WIDTH = 170;
    static const int LONGITUDE_HEIGHT = 38;

    static const int LATITUDE_X = 205;
    static const int LATITUDE_Y = 200;
    static const int LATITUDE_WIDTH = 170;
    static const int LATITUDE_HEIGHT = 38;

    static const int ALTITUDE_Y = 258;
    static const int ALTITUDE_HEIGHT = 38;

    static const int MENU_FMP_X = 105;
    static const int MENU_FMP_Y = 47;
    static const int MENU_GPS_X = 140;
    static const int MENU_GPS_Y = 50;
    static const int MENU_SETTING_X = 75;
    static const int MENU_SETTING_Y = 75;

    static const int USB_MASS_X = 140;
    static const int USB_MASS_Y = 140;

    static const int FINDME_START_BUTTON_X = 75;
    static const int FINDME_START_BUTTON_Y = 260;
};

#endif /* COMMON_UI_HPP */
