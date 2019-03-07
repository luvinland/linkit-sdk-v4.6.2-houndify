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

#ifdef DYNAMIC_BITMAP_LOADER

#include <gui/database/DynamicBitmapDatabase.hpp>
#include "LZMA_decoder.h"
#include "FreeRTOS.h"

extern const unsigned char dynamic_image_common_background[];
extern const unsigned char dynamic_image_home_clock_background[];
extern const unsigned char dynamic_image_home_compound_clock_background[];
extern const unsigned char dynamic_image_icon_usb[];
extern const unsigned char dynamic_image_menu_fmp[];
extern const unsigned char dynamic_image_menu_gps[];
extern const unsigned char dynamic_image_menu_setting[];
extern const unsigned char dynamic_image_datepicker_background[];
extern const unsigned char dynamic_image_list_menu_background[];
extern const unsigned char dynamic_image_switch_off[];
extern const unsigned char dynamic_image_switch_on[];
extern const unsigned char dynamic_image_timepicker_background[];
#ifndef CLOCK_LIST_STYLE
extern const unsigned char dynamic_image_clockstyle_clock1_active[];
extern const unsigned char dynamic_image_clockstyle_clock1_inactive[];
extern const unsigned char dynamic_image_clockstyle_clock2_active[];
extern const unsigned char dynamic_image_clockstyle_clock2_inactive[];
extern const unsigned char dynamic_image_clockstyle_clock3_active[];
extern const unsigned char dynamic_image_clockstyle_clock3_inactive[];
#endif
extern const unsigned char dynamic_image_player_pause[];
extern const unsigned char dynamic_image_player_play[];
extern const unsigned char dynamic_image_player_skip_next[];
extern const unsigned char dynamic_image_player_skip_previous[];
extern const unsigned char dynamic_image_player_volume_1[];
extern const unsigned char dynamic_image_player_volume_2[];
extern const unsigned char dynamic_image_player_volume_3[];
extern const unsigned char dynamic_image_player_volume_4[];
extern const unsigned char dynamic_image_player_volume_5[];
extern const unsigned char dynamic_image_player_volume_6[];
extern const unsigned char dynamic_image_player_volume_7[];
extern const unsigned char dynamic_image_player_volume_background[];
extern const unsigned char dynamic_image_fmp_start_long[];
extern const unsigned char dynamic_image_fmp_stop_long[];
extern const unsigned char dynamic_image_btn_cancel_long[];
extern const unsigned char dynamic_image_btn_cancel_short[];
extern const unsigned char dynamic_image_btn_yes_short[];
const DynamicBitmapDatabase::BitmapInfo dynamic_bitmap_database[] = {
    { dynamic_image_common_background, 49439, 400, 400 },
    { dynamic_image_home_clock_background, 50737, 400, 400 },
    { dynamic_image_home_compound_clock_background, 91560, 400, 400 },
    { dynamic_image_icon_usb, 4206, 120, 120 },
    { dynamic_image_menu_fmp, 9447, 186, 142 },
    { dynamic_image_menu_gps, 6809, 120, 130 },
    { dynamic_image_menu_setting, 23909, 250, 250 },
    { dynamic_image_datepicker_background, 52071, 400, 400 },
    { dynamic_image_list_menu_background, 7726, 300, 80 },
    { dynamic_image_switch_off, 2925, 120, 50 },
    { dynamic_image_switch_on, 2480, 120, 50 },
    { dynamic_image_timepicker_background, 51694, 400, 400 },
#ifndef CLOCK_LIST_STYLE
    { dynamic_image_clockstyle_clock1_active, 27875, 240, 240 },
    { dynamic_image_clockstyle_clock1_inactive, 14733, 160, 160 },
    { dynamic_image_clockstyle_clock2_active, 35516, 240, 240 },
    { dynamic_image_clockstyle_clock2_inactive, 19795, 160, 160 },
    { dynamic_image_clockstyle_clock3_active, 25958, 240, 240 },
    { dynamic_image_clockstyle_clock3_inactive, 12856, 160, 160 },
#endif
    { dynamic_image_player_pause, 2729, 100, 100 },
    { dynamic_image_player_play, 2822, 100, 100 },
    { dynamic_image_player_skip_next, 1267, 60, 60 },
    { dynamic_image_player_skip_previous, 1270, 60, 60 },
    { dynamic_image_player_volume_1, 3326, 200, 40 },
    { dynamic_image_player_volume_2, 3281, 200, 40 },
    { dynamic_image_player_volume_3, 3209, 200, 40 },
    { dynamic_image_player_volume_4, 3107, 200, 40 },
    { dynamic_image_player_volume_5, 2948, 200, 40 },
    { dynamic_image_player_volume_6, 2676, 200, 40 },
    { dynamic_image_player_volume_7, 2099, 200, 40 },
    { dynamic_image_player_volume_background, 2636, 200, 40 },
    { dynamic_image_fmp_start_long, 5724, 250, 60 },
    { dynamic_image_fmp_stop_long, 3972, 250, 60 },
    { dynamic_image_btn_cancel_long, 4988, 250, 60 },
    { dynamic_image_btn_cancel_short, 2719, 120, 60 },
    { dynamic_image_btn_yes_short, 1600, 120, 60 },
};

const DynamicBitmapDatabase::BitmapInfo *DynamicBitmapDatabase::getInstance()
{
    return dynamic_bitmap_database;
}

uint16_t DynamicBitmapDatabase::getInstanceSize()
{
    return (uint16_t)(sizeof(dynamic_bitmap_database) / sizeof(DynamicBitmapDatabase::BitmapInfo));
}

const DynamicBitmapDatabase::BitmapInfo DynamicBitmapDatabase::getInstanceInfo(uint16_t index)
{
    assert(index < getInstanceSize());
    const DynamicBitmapDatabase::BitmapInfo *instance = DynamicBitmapDatabase::getInstance();
    return instance[index];
}

static void *touchgfx_alloc(void *mem_ptr, size_t mem_size)
{
    return pvPortMalloc(mem_size);
}

static void touchgfx_free(void *mem_ptr, void *mem_address)
{
    vPortFree(mem_address);
}

static ISzAlloc touchgfx_uncompress_alloc = { touchgfx_alloc, touchgfx_free };

int DynamicBitmapDatabase::uncompress(unsigned char *dest, uint32_t *destLen, const unsigned char *src, uint32_t *srcLen)
{
    ELzmaStatus status;
    *srcLen -= 13;
    return LzmaDecode((Byte *)dest, (SizeT *)destLen, (const Byte *)src + 13, (SizeT *)srcLen,
                      (const Byte *)src, 5, LZMA_FINISH_ANY, &status, &touchgfx_uncompress_alloc);
}

#endif
