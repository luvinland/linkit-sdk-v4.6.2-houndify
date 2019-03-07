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

#ifndef DYNAMICBITMAPDATABASE_HPP
#define DYNAMICBITMAPDATABASE_HPP

#ifdef DYNAMIC_BITMAP_LOADER

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/Bitmap.hpp>

using namespace touchgfx;

const uint16_t DYNAMIC_BITMAP_COMMON_BACKGROUND_INT_ID = 0; // Size: 400x400 pixels
const uint16_t DYNAMIC_BITMAP_HOME_CLOCK_BACKGROUND_INT_ID = 1; // Size: 400x400 pixels
const uint16_t DYNAMIC_BITMAP_HOME_COMPOUND_CLOCK_BACKGROUND_INT_ID = 2; // Size: 400x400 pixels
const uint16_t DYNAMIC_BITMAP_ICON_USB_INT_ID = 3; // Size: 120x120 pixels
const uint16_t DYNAMIC_BITMAP_MENU_FMP_INT_ID = 4; // Size: 186x142 pixels
const uint16_t DYNAMIC_BITMAP_MENU_GPS_INT_ID = 5; // Size: 120x130 pixels
const uint16_t DYNAMIC_BITMAP_MENU_SETTING_INT_ID = 6; // Size: 250x250 pixels
const uint16_t DYNAMIC_BITMAP_DATEPICKER_BACKGROUND_INT_ID = 7; // Size: 400x400 pixels
const uint16_t DYNAMIC_BITMAP_LIST_MENU_BACKGROUND_INT_ID = 8; // Size: 300x80 pixels
const uint16_t DYNAMIC_BITMAP_SWITCH_OFF_INT_ID = 9; // Size: 120x50 pixels
const uint16_t DYNAMIC_BITMAP_SWITCH_ON_INT_ID = 10; // Size: 120x50 pixels
const uint16_t DYNAMIC_BITMAP_TIMEPICKER_BACKGROUND_INT_ID = 11; // Size: 400x400 pixels
#ifndef CLOCK_LIST_STYLE
const uint16_t DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK1_ACTIVE_INT_ID = 12; // Size: 240x240 pixels
const uint16_t DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK1_INACTIVE_INT_ID = 13; // Size: 160x160 pixels
const uint16_t DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK2_ACTIVE_INT_ID = 14; // Size: 240x240 pixels
const uint16_t DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK2_INACTIVE_INT_ID = 15; // Size: 160x160 pixels
const uint16_t DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK3_ACTIVE_INT_ID = 16; // Size: 240x240 pixels
const uint16_t DYNAMIC_BITMAP_CLOCKSTYLE_CLOCK3_INACTIVE_INT_ID = 17; // Size: 160x160 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_PAUSE_INT_ID = 18; // Size: 100x100 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_PLAY_INT_ID = 19; // Size: 100x100 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_SKIP_NEXT_INT_ID = 20; // Size: 60x60 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_SKIP_PREVIOUS_INT_ID = 21; // Size: 60x60 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_1_INT_ID = 22; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_2_INT_ID = 23; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_3_INT_ID = 24; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_4_INT_ID = 25; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_5_INT_ID = 26; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_6_INT_ID = 27; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_7_INT_ID = 28; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_BACKGROUND_INT_ID = 29; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_FMP_START_LONG_INT_ID = 30; // Size: 250x60 pixels
const uint16_t DYNAMIC_BITMAP_FMP_STOP_LONG_INT_ID = 31; // Size: 250x60 pixels
const uint16_t DYNAMIC_BITMAP_BTN_CANCEL_LONG_INT_ID = 32; // Size: 250x60 pixels
const uint16_t DYNAMIC_BITMAP_BTN_CANCEL_SHORT_INT_ID = 33; // Size: 120x60 pixels
const uint16_t DYNAMIC_BITMAP_BTN_YES_SHORT_INT_ID = 34; // Size: 120x60 pixels
#else
const uint16_t DYNAMIC_BITMAP_PLAYER_PAUSE_INT_ID = 12; // Size: 100x100 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_PLAY_INT_ID = 13; // Size: 100x100 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_SKIP_NEXT_INT_ID = 14; // Size: 60x60 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_SKIP_PREVIOUS_INT_ID = 15; // Size: 60x60 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_1_INT_ID = 16; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_2_INT_ID = 17; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_3_INT_ID = 18; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_4_INT_ID = 19; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_5_INT_ID = 20; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_6_INT_ID = 21; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_7_INT_ID = 22; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_PLAYER_VOLUME_BACKGROUND_INT_ID = 23; // Size: 200x40 pixels
const uint16_t DYNAMIC_BITMAP_FMP_START_LONG_INT_ID = 24; // Size: 250x60 pixels
const uint16_t DYNAMIC_BITMAP_FMP_STOP_LONG_INT_ID = 25; // Size: 250x60 pixels
const uint16_t DYNAMIC_BITMAP_BTN_CANCEL_LONG_INT_ID = 26; // Size: 250x60 pixels
const uint16_t DYNAMIC_BITMAP_BTN_CANCEL_SHORT_INT_ID = 27; // Size: 120x60 pixels
const uint16_t DYNAMIC_BITMAP_BTN_YES_SHORT_INT_ID = 28; // Size: 120x60 pixels
#endif

class DynamicBitmapDatabase
{
public:
    struct BitmapInfo {
        const uint8_t *const data;                  ///< The data of this bitmap
        const uint32_t       data_length;           ///< The data length of this bitmap
        const uint16_t       width;                 ///< The width of the bitmap
        const uint16_t       height;                ///< The height of the bitmap
    };

    static const BitmapInfo *getInstance();

    static uint16_t getInstanceSize();

    static const BitmapInfo getInstanceInfo(uint16_t index);

    static int uncompress(unsigned char *dest, uint32_t *destLen, const unsigned char *src, uint32_t *srcLen);
};

#endif

#endif // DYNAMICBITMAPDATABASE_HPP
