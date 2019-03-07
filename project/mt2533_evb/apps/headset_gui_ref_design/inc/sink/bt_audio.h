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

#ifndef __BT_AUDIO_H__
#define __BT_AUDIO_H__

#if defined(_MSC_VER)
#else
#include "stdio.h"
#endif

#include "stdbool.h"
#include "gdi_font_engine.h"
#include "gdi.h"
#include "main_menu.h"
#include "bt_type.h"
#include "bt_sink_srv.h"
#include <syslog.h>
#include <math.h>
#include "mt25x3_hdk_lcd.h"


#define BT_AUDIO_ITEM_H             (40)
#define BT_AUDIO_ITEM_W             (40)

#define BT_AUDIO_CALL_DROP_X        (5)
#define BT_AUDIO_CALL_DROP_Y        (5)

#define BT_AUDIO_MAX_NUM_LEN        (45)
#define BT_AUDIO_MAX_NAME_LEN       (45)
 
#define STR_BT_AUDIO_LINK_IS_TWO       ("Link is 2")

typedef enum {
    BT_AUDIO_SCR_IDLE,
    BT_AUDIO_SCR_SCANNING,
    BT_AUDIO_SCR_INCOMING,
    BT_AUDIO_SCR_CALLING,
    BT_AUDIO_SCR_OUTGOING,
    BT_AUDIO_SCR_HANG_UP,
    BT_AUDIO_SCR_LINK_LOST,
    BT_AUDIO_SCR_MISSED_CALL,
    BT_AUDIO_SCR_MISSED_CALL_OPERATION,  // screen after click missed call screen
    
    BT_AUDIO_SCR_TOAL
} bt_audio_screen_t;

typedef struct {
    uint8_t num[BT_AUDIO_MAX_NUM_LEN + 1];
    uint8_t name[BT_AUDIO_MAX_NAME_LEN + 1];
    uint8_t num_len;
    uint8_t name_len;
} bt_audio_caller_t;

typedef struct {
    int16_t x;
    int16_t y;
} bt_audio_point_t;

typedef struct {
    uint32_t lcd_heigth;
    uint32_t lcd_width;
    bool entered;
    gdi_color_t bg_color;
    gdi_font_engine_color_t font_color;
    gdi_font_engine_size_t font;
    bt_audio_screen_t scr;
    bt_audio_screen_t bg_screen;
    bt_sink_srv_profile_type_t   profile_type;
    bt_audio_caller_t            caller;
} bt_audio_context_t;

void bt_audio_init(void);

void bt_audio_show_scan_screen(void);

void bt_audio_event_handler(message_id_enum event_id, int32_t param1, void *param2);

uint8_t bt_audio_get_connected_device_number(void);

#define AUDIO_PLAYER_STATE_BT_PLAYING               (0x01)
#define AUDIO_PLAYER_STATE_LOCAL_PLAYING            (0x02)

uint8_t audio_player_get_state(void);

void audio_player_set_state(uint8_t state);

void audio_player_reset_state(uint8_t state);
#endif /* __BT_AUDIO_H__ */

