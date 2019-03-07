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

 
#include "ble_find_me_client_screen.h"
#include "ble_find_me_client.h"

log_create_module(FMPC_SCR, PRINT_LEVEL_INFO);

#define FMPC_TITLE_X  1
#define FMPC_TITLE_Y  1
#define FMPC_CONTENT_X 5
#define FMPC_ITEM_HEIGHT 10
#define FMPC_ITEM_WIDTH 10

//static uint8_t lcd_size = 0;
static ble_fmpc_screen_cntx g_ble_fmpc_screen_cntx;
static uint32_t LCD_HEIGHT = 128, LCD_WIDTH = 80;
//static bool g_fmpc_init = false;

static uint8_t* ble_fmpc_convert_string_to_wstring(uint8_t* string);
static void ble_fmpc_font_engine_show_string(int32_t x, int32_t y, uint8_t *string,
    int32_t len, gdi_font_engine_color_t font_color,
    gdi_font_engine_size_t font_size);
#if 0
static int32_t bt_fmpc_get_convert_coordinate(uint8_t lcd_size, int32_t pos);
static void bt_fmpc_font_engine_get_string_width_height(uint8_t *string, int32_t len,
    int32_t *width, int32_t *height);
#endif
static void ble_fmpc_pen_event_handler(touch_event_struct_t* pen_event, void* user_data);
static void ble_fmp_client_show_error_screen(void);
static void ble_fmp_client_show_alert_screen(void);

void ble_fmp_client_screen_event_handler(message_id_enum event_id, int32_t param1, void* param2)
{
    LOG_I(FMPC_SCR, "%s \r\n", __FUNCTION__);
}

void ble_fmp_client_screen_init(void)
{
    LOG_I(FMPC_SCR, "%s \r\n", __FUNCTION__);
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};//white color
    
    memset(&g_ble_fmpc_screen_cntx, 0, sizeof(ble_fmpc_screen_cntx));

    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, (void *)&LCD_HEIGHT);
    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, (void *)&LCD_WIDTH);

    LOG_I(FMPC_SCR, "%s: height is %d, width is %d!\r\n", __FUNCTION__, LCD_HEIGHT, LCD_WIDTH);

    LCD_HEIGHT = 128;
    LCD_WIDTH = 80;
    g_ble_fmpc_screen_cntx.bg_color = 0;
    g_ble_fmpc_screen_cntx.font_color = text_color;
    g_ble_fmpc_screen_cntx.font = GDI_FONT_ENGINE_FONT_MEDIUM;
    //g_fmpc_init = true;
}

static uint8_t* ble_fmpc_convert_string_to_wstring(uint8_t* string)
{
    static uint8_t wstring[50];
    int32_t index = 0;
    if (!string) {
        return NULL;
    }

    while (*string) {
        wstring[index] = *string;
        wstring[index + 1] = 0;
        string++;
        index+=2;
    }
    return wstring;
}

static void ble_fmpc_font_engine_show_string(int32_t x, int32_t y, uint8_t *string,
    int32_t len, gdi_font_engine_color_t font_color,
    gdi_font_engine_size_t font_size)
{

    gdi_font_engine_display_string_info_t string_info = {0};

    gdi_font_engine_set_font_size(font_size);
    gdi_font_engine_set_text_color(font_color);

    string_info.x = x;
    string_info.y = y;
    string_info.baseline_height = -1;
    string_info.string = string;
    string_info.length = len;
    gdi_font_engine_display_string(&string_info);
}

#if 0
/* base size 240 * 240 */
static int32_t bt_fmpc_get_convert_coordinate(uint8_t lcd_size, int32_t pos)
{
    double tmp = 0.0;
    double factor = 1.0;
    double skip = 0.5;
    int32_t ret = 0;

    if (lcd_size == 1) {
        factor = 320.0 / 240;
        tmp = floor(factor * pos + skip);
        ret = (int32_t) tmp;
    } else if (lcd_size == 1) {
        ret = pos;
    }

    return ret;
}

static void bt_fmpc_font_engine_get_string_width_height(uint8_t *string, int32_t len,
    int32_t *width, int32_t *height)
{
    *width = 1;
    *height = 1;
}
#endif

static void ble_fmpc_pen_event_handler(touch_event_struct_t* pen_event, void* user_data)
{
    static int32_t back_hit;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};//white color
    bt_status_t status;
    
    LOG_I(FMPC_SCR, "%s \r\n", __FUNCTION__);
    //if (pen_event->position.x <= g_ble_fmpc_screen_cntx.back_x2 && pen_event->position.x >= g_ble_fmpc_screen_cntx.back_x1) {
        //if (pen_event->position.y <= g_ble_fmpc_screen_cntx.back_y2 && pen_event->position.y >= g_ble_fmpc_screen_cntx.back_y1) {
    if (pen_event->position.x <= (LCD_WIDTH - 1) && pen_event->position.x >= 1) {
        if (pen_event->position.y <= (LCD_HEIGHT - 1) && pen_event->position.y >= 1) {
            if (pen_event->type == TOUCH_EVENT_DOWN) {
                back_hit = 1;
            } else if (back_hit == 1 && pen_event->type == TOUCH_EVENT_UP) {
                back_hit = 0;  
                if (g_ble_fmpc_screen_cntx.entered) {
                    status = ble_fmp_client_stop_alert();
                    if (BT_STATUS_SUCCESS != status) {  
                        LOG_I(FMPC_SCR, "%s start error code is %d\r\n", __FUNCTION__, status);
                    }
                }
                memset(&g_ble_fmpc_screen_cntx, 0, sizeof(ble_fmpc_screen_cntx));
                g_ble_fmpc_screen_cntx.bg_color = 0;
                g_ble_fmpc_screen_cntx.font_color = text_color;
                g_ble_fmpc_screen_cntx.font = GDI_FONT_ENGINE_FONT_MEDIUM;
                LOG_I(FMPC_SCR, "%s, Back to mainscreen Success!\r\n", __FUNCTION__);
                show_main_menu_screen();
            }
        }
    } else {
        back_hit = 0;
    }
}

void ble_fmp_client_show_main_screen(void)
{
    bt_status_t status;
    
    LOG_I(FMPC_SCR, "%s \r\n", __FUNCTION__);

    set_active_screen_id(BLE_FMP_SCREEN_ID);
    demo_ui_register_touch_event_callback(ble_fmpc_pen_event_handler, NULL);

    g_ble_fmpc_screen_cntx.title_x = FMPC_TITLE_X;
    g_ble_fmpc_screen_cntx.title_y = FMPC_TITLE_Y;

    g_ble_fmpc_screen_cntx.content_x = FMPC_CONTENT_X;
    g_ble_fmpc_screen_cntx.content_y = (LCD_HEIGHT - FMPC_ITEM_HEIGHT) / 2;

    g_ble_fmpc_screen_cntx.back_x2 = LCD_WIDTH - 30;
    g_ble_fmpc_screen_cntx.back_y2 = LCD_HEIGHT - 20;
    g_ble_fmpc_screen_cntx.back_x1 = g_ble_fmpc_screen_cntx.back_x2 - FMPC_ITEM_WIDTH;
    g_ble_fmpc_screen_cntx.back_y1 = g_ble_fmpc_screen_cntx.back_y2 - FMPC_ITEM_HEIGHT;

    status = ble_fmp_client_start_alert();
    if (BT_STATUS_SUCCESS != status) {  
        LOG_I(FMPC_SCR, "%s start error code is (0x%04x)\r\n", __FUNCTION__, status);
        ble_fmp_client_show_error_screen();
    } else {
        ble_fmp_client_show_alert_screen();
    }
}
    
static void ble_fmp_client_show_alert_screen(void)
{    
    uint32_t rect_x1 = 0, rect_y1 = 0, rect_x2 = 0, rect_y2 = 0;

    LOG_I(FMPC_SCR, "%s \r\n", __FUNCTION__);
    gdi_draw_filled_rectangle(0, 0, LCD_WIDTH - 1, LCD_HEIGHT -1, g_ble_fmpc_screen_cntx.bg_color);
    rect_x1 = 0;
    rect_y1 = 0;
    rect_x2 = LCD_WIDTH - 1;
    rect_y2 = LCD_HEIGHT -1;
#if 0
    bt_fmpc_font_engine_get_string_width_height(
                                ble_fmpc_convert_string_to_wstring((uint8_t *)"FMPC"), 
                                strlen("FMPC"),
                                &string_width,
                                &string_height);
#endif
    ble_fmpc_font_engine_show_string(g_ble_fmpc_screen_cntx.title_x, 
                            g_ble_fmpc_screen_cntx.title_y, 
                            ble_fmpc_convert_string_to_wstring((uint8_t *)"FMPC"), 
                            strlen("FMPC"), 
                            g_ble_fmpc_screen_cntx.font_color, g_ble_fmpc_screen_cntx.font);
#if 0
    bt_fmpc_font_engine_get_string_width_height(
                                ble_fmpc_convert_string_to_wstring((uint8_t *)"ALERT"), 
                                strlen("ALERT"),
                                &string_width,
                                &string_height);
#endif
    ble_fmpc_font_engine_show_string(g_ble_fmpc_screen_cntx.content_x, 
                            g_ble_fmpc_screen_cntx.content_y, 
                            ble_fmpc_convert_string_to_wstring((uint8_t *)"ALERT"), 
                            strlen("ALERT"), 
                            g_ble_fmpc_screen_cntx.font_color, g_ble_fmpc_screen_cntx.font);
 
#if 0
    bt_fmpc_font_engine_get_string_width_height(
                                ble_fmpc_convert_string_to_wstring((uint8_t *)"Exit"), 
                                strlen("Exit"),
                                &string_width,
                                &string_height);
#endif 
    ble_fmpc_font_engine_show_string(g_ble_fmpc_screen_cntx.back_x1, 
                            g_ble_fmpc_screen_cntx.back_y1, 
                            ble_fmpc_convert_string_to_wstring((uint8_t *)"Exit"), 
                            strlen("Exit"), 
                            g_ble_fmpc_screen_cntx.font_color, g_ble_fmpc_screen_cntx.font);

    g_ble_fmpc_screen_cntx.entered = true;
    gdi_lcd_update_screen(rect_x1, rect_y1, rect_x2, rect_y2);
}


static void ble_fmp_client_show_error_screen(void)
{
    LOG_I(FMPC_SCR, "%s \r\n", __FUNCTION__);
    //int32_t string_width, string_height;
    uint32_t rect_x1 = 0, rect_y1 = 0, rect_x2 = 0, rect_y2 = 0;

    gdi_draw_filled_rectangle(0, 0, LCD_WIDTH - 1, LCD_HEIGHT -1, g_ble_fmpc_screen_cntx.bg_color);
    rect_x1 = 0;
    rect_y1 = 0;
    rect_x2 = LCD_WIDTH - 1;
    rect_y2 = LCD_HEIGHT -1;
#if 0
    bt_fmpc_font_engine_get_string_width_height(
                                ble_fmpc_convert_string_to_wstring((uint8_t *)"FMPC"), 
                                strlen("FMPC"),
                                &string_width,
                                &string_height);
#endif
    ble_fmpc_font_engine_show_string(g_ble_fmpc_screen_cntx.title_x, 
                            g_ble_fmpc_screen_cntx.title_y, 
                            ble_fmpc_convert_string_to_wstring((uint8_t *)"FMPC"), 
                            strlen("FMPC"), 
                            g_ble_fmpc_screen_cntx.font_color, g_ble_fmpc_screen_cntx.font);
#if 0
    bt_fmpc_font_engine_get_string_width_height(
                                ble_fmpc_convert_string_to_wstring((uint8_t *)"ERROR"), 
                                strlen("ERROR"),
                                &string_width,
                                &string_height);
#endif
    ble_fmpc_font_engine_show_string(g_ble_fmpc_screen_cntx.content_x, 
                            g_ble_fmpc_screen_cntx.content_y, 
                            ble_fmpc_convert_string_to_wstring((uint8_t *)"ERROR"), 
                            strlen("ERROR"), 
                            g_ble_fmpc_screen_cntx.font_color, g_ble_fmpc_screen_cntx.font);
 
#if 0
    bt_fmpc_font_engine_get_string_width_height(
                                ble_fmpc_convert_string_to_wstring((uint8_t *)"Exit"), 
                                strlen("Exit"),
                                &string_width,
                                &string_height);
#endif
    ble_fmpc_font_engine_show_string(g_ble_fmpc_screen_cntx.back_x1, 
                            g_ble_fmpc_screen_cntx.back_y1, 
                            ble_fmpc_convert_string_to_wstring((uint8_t *)"Exit"), 
                            strlen("Exit"), 
                            g_ble_fmpc_screen_cntx.font_color, g_ble_fmpc_screen_cntx.font);
    gdi_lcd_update_screen(rect_x1, rect_y1, rect_x2, rect_y2);
}







