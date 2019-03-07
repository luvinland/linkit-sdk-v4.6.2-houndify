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

/*****************************************************************************
 *
 * Description:
 * ------------
 * The file is used for testing BT notification..
 *
 ****************************************************************************/

#include "stdint.h"
#include <string.h>
#include "stdio.h"

#include <FreeRTOS.h>

#include "queue.h"
#include "task.h"
#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_system.h"
#include "bt_spp.h"
#include "bt_callback_manager.h"
#include "bt_notify.h"
#include "task_def.h"
#include "bt_notify_app.h"

#include "syslog.h"

#include "fota_bt_common.h"
#include "gdi_font_engine.h"
#include "gdi.h"
#include "custom_resource_def.h"
#include "mt25x3_hdk_lcd.h"
#include "main_menu.h"

log_create_module(NOTIFY_APP, PRINT_LEVEL_INFO);
typedef struct {
    uint32_t lcd_heigth;
    uint32_t lcd_width;
    bool entered;
    gdi_color_t bg_color;
    gdi_font_engine_color_t font_color;
    gdi_font_engine_size_t font;
    uint32_t notification_num;
} bt_notify_ui_context_t;

bt_notify_ui_context_t g_bt_notify_ui_cntx;
#define STR_BT_NOTIFY_UI_ALERT     ("Notifications.")

static void bt_notify_app_ui_init(void);
static void bt_notify_app_show_alert_screen(uint32_t count);
/*****************************************************************************
 * define
 *****************************************************************************/

/*****************************************************************************
 * typedef
 *****************************************************************************/

void bt_notify_app_callback(void *data)
{
    LOG_I(NOTIFY_APP, "\r\n[App test]callback task\r\n");
    
    bt_notify_callback_data_t *p_data = (bt_notify_callback_data_t *)data;

    switch (p_data->evt_id) {
        case BT_NOTIFY_EVENT_CONNECTION: {
           
            break;
        }
        case BT_NOTIFY_EVENT_DISCONNECTION: {
            
            g_bt_notify_ui_cntx.notification_num = 0;
            if (get_active_screen_id() == BT_NOTIFICATION_SCREEN_ID) {
                
                LOG_I(NOTIFY_APP, "\r\n[App test]back to main.\r\n");
                show_main_menu_screen();
            }
            break;
        }
        case BT_NOTIFY_EVENT_SEND_IND: {
           
            break;
        }
        case BT_NOTIFY_EVENT_DATA_RECEIVED: {
            
            break;
        }
        case BT_NOTIFY_EVENT_NOTIFICATION: {
            
            if (p_data->notification.action == BT_NOTIFY_ACTION_TYPE_NEW) {
                LOG_I(NOTIFY_APP, "\r\n[Notification]Incoming notification!!\r\n");
                g_bt_notify_ui_cntx.notification_num ++;
                bt_notify_app_show_alert_screen(g_bt_notify_ui_cntx.notification_num);
            }
            break;
        }
        case BT_NOTIFY_EVENT_MISSED_CALL: {
            LOG_I(NOTIFY_APP, "\r\n[Notification]Incoming notification!!\r\n");

            
            g_bt_notify_ui_cntx.notification_num ++;
            bt_notify_app_show_alert_screen(g_bt_notify_ui_cntx.notification_num);
            break;
        }
        case BT_NOTIFY_EVENT_SMS: {
            LOG_I(NOTIFY_APP, "\r\n[Notification]Incoming notification!!\r\n");

            
            g_bt_notify_ui_cntx.notification_num ++;
            bt_notify_app_show_alert_screen(g_bt_notify_ui_cntx.notification_num );
            break;
        }
        default: {
            break;
        }
    }  
}
void bt_notify_app_init(void)
{
    bt_notify_init((BT_SPP_SERVER_ID_START + 2));
    bt_notify_app_ui_init();
    bt_notify_register_callback(NULL, "app_test", bt_notify_app_callback);
}
static void bt_notify_app_ui_init(void)
{
    uint32_t lcd_h = 0, lcd_w = 0;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};//white color

    memset(&g_bt_notify_ui_cntx, 0, sizeof(bt_notify_ui_context_t));
    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, (void *)&lcd_h);
    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, (void *)&lcd_w);
    g_bt_notify_ui_cntx.lcd_heigth = 128;
    g_bt_notify_ui_cntx.lcd_width = 80;
    

    g_bt_notify_ui_cntx.bg_color = 0;
    g_bt_notify_ui_cntx.font_color = text_color;
    g_bt_notify_ui_cntx.font = GDI_FONT_ENGINE_FONT_SMALL;
    g_bt_notify_ui_cntx.entered = true;
   
    LOG_I(NOTIFY_APP, "[notify UI]bt_notify_app_ui_init\n");

}

static void bt_notify_app_font_engine_show_string(int32_t x, int32_t y, uint8_t *string,
                                      int32_t len, gdi_font_engine_color_t font_color)
{ 
    gdi_font_engine_display_string_info_t string_info = {0};

    LOG_I(NOTIFY_APP, "[notify UI]show_string(enter), x: %d, y: %d\n", x, y);

    gdi_font_engine_set_font_size(g_bt_notify_ui_cntx.font);
    gdi_font_engine_set_text_color(font_color);

    string_info.x = x;
    string_info.y = y;
    string_info.baseline_height = -1;
    string_info.string = string;
    string_info.length = len;
    gdi_font_engine_display_string(&string_info);
}
uint8_t *bt_notify_app_convert_string_to_wstring(uint8_t *string)
{
    static uint8_t wstring[128];
    int32_t index = 0;
    if (!string) {
        return NULL;
    }

    while (*string) {
        wstring[index] = *string;
        wstring[index + 1] = 0;
        string++;
        index += 2;
    }
    return wstring;
}

static void bt_notify_pen_event_handler(touch_event_struct_t *pen_event, void *user_data)
{
// exit screen
    show_main_menu_screen();
    g_bt_notify_ui_cntx.notification_num = 0;
    LOG_I(NOTIFY_APP, "[notify UI]exit notify ui.\n");

}
static int itoa(int n, char *s, int len)
{
    int i, j, sign;
    char c[32];
    i = j = sign = 0;
    memset(c, '\0', 32);
    if ((sign = n) < 0) {
        n = (-n);
    }

    do {
        if (i >= len) {
            return (-1);
        }
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (i >= len) {
        return (-1);
    }
    if (sign < 0) {
        s[i++] = '-';
    }

    if (i >= len) {
        return (-1);
    }

    s[i] = '\0';
    for (; i > 0 ; i--) {
        c[j] = s[i - 1];
        j++;
    }
    strncpy(s, c, strlen(c) + 1);
    return 0;
}


extern bool isMainMenuScreen;
extern bool isClockScreen;

static void bt_notify_app_show_alert_screen(uint32_t count)
{
    char string[30];
    LOG_I(NOTIFY_APP, "[notify UI]bt_notify_app_show_alert_screen: %d\n", count);
    isClockScreen = false;
    isMainMenuScreen = true;
    set_active_screen_id(BT_NOTIFICATION_SCREEN_ID);
    demo_ui_register_touch_event_callback(bt_notify_pen_event_handler, NULL);
    gdi_draw_filled_rectangle(0, 0, g_bt_notify_ui_cntx.lcd_width, g_bt_notify_ui_cntx.lcd_heigth, g_bt_notify_ui_cntx.bg_color);
   // gdi_draw_rectangle

    gdi_image_draw_by_id(g_bt_notify_ui_cntx.lcd_width / 2 - 20, 
        g_bt_notify_ui_cntx.lcd_heigth / 2 - 40,
        IMAGE_ID_ANDROID_NOTIFY_BMP);

    itoa((int)count, string,10);
    strcat(string," msg");
    bt_notify_app_font_engine_show_string(15,
                                         g_bt_notify_ui_cntx.lcd_heigth / 2 + 20,
                                         bt_notify_app_convert_string_to_wstring((uint8_t*)string),
                                         strlen(string),
                                         g_bt_notify_ui_cntx.font_color); 
    gdi_lcd_update_screen(0, 0, g_bt_notify_ui_cntx.lcd_width, g_bt_notify_ui_cntx.lcd_heigth);

}


