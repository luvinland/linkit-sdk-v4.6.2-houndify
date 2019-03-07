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

#if defined(_MSC_VER)
#else
#include "stdio.h"
#endif
#include "FreeRTOS.h"
#include "timers.h"
#include "gdi_font_engine.h"
#include "gdi.h"
#include "main_menu.h"
#include "digital_clock.h"
#include "bt_audio.h"
#include "syslog.h"
#include "bt_init.h"
#include <string.h>
#include "bt_sink_app_event.h"
#include "custom_resource_def.h"


/*
  * Timer ID
  */
#define BT_AUDIO_TIMER_BASE          0x02000000
#define BT_AUDIO_TIMER_SCREEN_EXIT   (BT_AUDIO_TIMER_BASE + 1)
#define BT_AUDIO_TIMER_SCAN_FLICKER  (BT_AUDIO_TIMER_BASE + 2)

// link lost screen, call hang up scrren, active call screen exit time
#define BT_AUDIO_SCREEN_EXIT_TIME (3000)

bt_audio_context_t g_bt_audio_cntx;
TimerHandle_t g_screen_exit_timer_id;
// scan screen flicker timer
TimerHandle_t g_scan_timer_id;
static bool g_is_show_scan_icon = false;

static uint8_t g_aud_state = 0;
extern bt_sink_srv_status_t bt_sink_srv_action_send(bt_sink_srv_action_t action, void *parameters);

log_create_module(bt_audio_sink, PRINT_LEVEL_INFO);

uint8_t *bt_audio_convert_string_to_wstring(char *string)
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

void bt_audio_post_event_callback(bt_sink_srv_event_t event_id, bt_sink_srv_status_t result, void *parameters)
{
    if (NULL != parameters) {
        vPortFree(parameters);
    }
}

static bool bt_audio_is_point_range(bt_audio_point_t *point, bt_audio_point_t *l_corner, bt_audio_point_t *r_corner)
{
    bool ret = false;   

    if (point->x >= l_corner->x && point->x <= r_corner->x &&
            point->y >= l_corner->y && point->y <= r_corner->y) {
        ret = true;
    }

    LOG_I(bt_audio_sink, "[bt_audio_sink]point_range(end), x1: %d, y1: %d, x2: %d, y2: %d\n",
          l_corner->x, l_corner->y, r_corner->x, r_corner->y);

    return ret;
}

static void bt_audio_screen_exit(void)
{
    g_bt_audio_cntx.entered = false;
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_IDLE;
    LOG_I(bt_audio_sink, "[bt_audio] bt_audio_screen_exit");
    show_clock_screen();
}

// Show this screen after click missed call screen
static void bt_audio_show_missed_call_operation_screen()
{
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_MISSED_CALL_OPERATION;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 4 - 20,
        IMAGE_ID_BT_CALL_CLOSE_BMP);

    // use outgoing image to indicate dial missed call
    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth * 3 / 4 - 20,
        IMAGE_ID_BT_CALL_OUTGOING_BMP);

    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

static void bt_audio_show_idle_screen()
{
    if (NULL != g_scan_timer_id && xTimerIsTimerActive(g_scan_timer_id) != pdFALSE) {
        xTimerStop(g_scan_timer_id, 0);
        xTimerDelete(g_scan_timer_id, 0);
        g_scan_timer_id = NULL;
    }
    bt_audio_screen_exit();
}  

static void bt_audio_pen_event_handler(touch_event_struct_t *pen_event, void *user_data)
{
    bt_audio_point_t point, l_corner, r_corner;
    int32_t touch = -1;

    LOG_I(bt_audio_sink, "[bt_audio_sink]pen_hdr(s)--type: %d, x: %d, y: %d, touch: %d\n",
          pen_event->type, pen_event->position.x, pen_event->position.y, touch);

    if (g_bt_audio_cntx.entered) {
        if (pen_event->type == TOUCH_EVENT_UP) {
            bt_sink_app_ext_cmd_t *ext_cmd_p = (bt_sink_app_ext_cmd_t *)pvPortMalloc(sizeof(*ext_cmd_p));

            point.x = pen_event->position.x;
            point.y = pen_event->position.y;
            switch (g_bt_audio_cntx.scr) {
                case BT_AUDIO_SCR_MISSED_CALL: {
                    bt_audio_show_missed_call_operation_screen();
                    break;
                }
                                
                case BT_AUDIO_SCR_MISSED_CALL_OPERATION: {
                    l_corner.x = BT_AUDIO_CALL_DROP_X;
                    l_corner.y = BT_AUDIO_CALL_DROP_Y;
                    r_corner.x = g_bt_audio_cntx.lcd_width - BT_AUDIO_CALL_DROP_X;
                    r_corner.y = g_bt_audio_cntx.lcd_heigth / 2 - BT_AUDIO_CALL_DROP_Y;
                    if (bt_audio_is_point_range(&point, &l_corner, &r_corner)) {
                        bt_audio_screen_exit();
                        break;
                    }

                    l_corner.x = BT_AUDIO_CALL_DROP_X;
                    l_corner.y = BT_AUDIO_CALL_DROP_Y + g_bt_audio_cntx.lcd_heigth / 2;
                    r_corner.x = g_bt_audio_cntx.lcd_width - BT_AUDIO_CALL_DROP_X;
                    r_corner.y = g_bt_audio_cntx.lcd_heigth - BT_AUDIO_CALL_DROP_Y;

                    if (bt_audio_is_point_range(&point, &l_corner, &r_corner)) {
                        /* handle hang up touch */
                        //bt_sink_event_send(BT_SINK_EVENT_HF_SWITCH_AUDIO_PATH, NULL);
                        ext_cmd_p->key_value = BT_SINK_SRV_KEY_FUNC;
                        ext_cmd_p->key_action = BT_SINK_SRV_KEY_ACT_DOUBLE_CLICK;
                        touch = 1;
                        break;
                    }

                    break;
                }

                case BT_AUDIO_SCR_SCANNING: {
                    l_corner.x = BT_AUDIO_CALL_DROP_X;
                    l_corner.y = BT_AUDIO_CALL_DROP_Y;
                    r_corner.x = g_bt_audio_cntx.lcd_width - BT_AUDIO_CALL_DROP_X;
                    r_corner.y = g_bt_audio_cntx.lcd_heigth - BT_AUDIO_CALL_DROP_Y;

                    if (bt_audio_is_point_range(&point, &l_corner, &r_corner)) {
                        /* exit to idle screen and close scan mode */
                        bt_audio_show_idle_screen();
                        bt_sink_srv_action_send(BT_SINK_SRV_ACTION_DISCOVERABLE, NULL);
                        break;
                    }
                    
                    break;
                }
                
                default:
                    break;
            }

            if (touch > 0) {
                bt_sink_app_event_post((bt_sink_srv_event_t)BT_SINK_EVENT_APP_EXT_COMMAND,
                                       (void *)ext_cmd_p, bt_audio_post_event_callback);
            } else {
                vPortFree(ext_cmd_p);
            }
        }
    }
    LOG_I(bt_audio_sink, "[bt_audio_sink]pen_hdr(e)--type: %d, x: %d, y: %d, touch: %d\n",
          pen_event->type, pen_event->position.x, pen_event->position.y, touch);
}

static void bt_audio_scan_timer_hdr(bool show_scan_icon) {
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);
    if (show_scan_icon) {
        gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
                             g_bt_audio_cntx.lcd_heigth / 2 - 20,
                             IMAGE_ID_MAINMENU_BT_SCAN_BMP);
    }
    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

void bt_audio_timer_callback(TimerHandle_t xTimer)
{
    int32_t timerID;
    timerID = ( int32_t ) pvTimerGetTimerID( xTimer );

    if (timerID == BT_AUDIO_TIMER_SCREEN_EXIT) {
        bt_audio_screen_exit();
    } else if (timerID == BT_AUDIO_TIMER_SCAN_FLICKER) {
        if (g_is_show_scan_icon) {
            bt_audio_scan_timer_hdr(g_is_show_scan_icon);
            g_is_show_scan_icon = false;
        } else {
            bt_audio_scan_timer_hdr(g_is_show_scan_icon);
            g_is_show_scan_icon = true;
        }
    }
}

static void bt_audio_font_engine_show_string(int32_t x, int32_t y, uint8_t *string,
                                      int32_t len, gdi_font_engine_color_t font_color)
{ 
    gdi_font_engine_display_string_info_t string_info = {0};

    LOG_I(bt_audio_sink, "[bt_audio_sink]show_string(enter), x: %d, y: %d\n", x, y);

    gdi_font_engine_set_font_size(g_bt_audio_cntx.font);
    gdi_font_engine_set_text_color(font_color);

    string_info.x = x;
    string_info.y = y;
    string_info.baseline_height = -1;
    string_info.string = string;
    string_info.length = len;
    gdi_font_engine_display_string(&string_info);
}

static void bt_audio_show_incoming_screen(bt_audio_caller_t *num)
{
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_INCOMING;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 2 - 40,
        IMAGE_ID_BT_CALL_INCOMING_BMP);

    if (num->num_len > 0 || num->name_len > 0) {
        uint8_t *display_name;
        uint8_t length;

        if (num->name_len > 0) {
            display_name = num->name;
            length = num->name_len;
        } else {
            display_name = num->num;
            length = num->num_len;
        }

        bt_audio_font_engine_show_string(BT_AUDIO_CALL_DROP_X,
                                         g_bt_audio_cntx.lcd_heigth / 2 + 20,
                                         bt_audio_convert_string_to_wstring((char *)display_name),
                                         length,
                                         g_bt_audio_cntx.font_color);
    }
 
    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}


static void bt_audio_show_calling_screen()
{
    // active call screen do not show name and duration
    // use incoming image to show active call screen
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_CALLING;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 2 - 20,
        IMAGE_ID_BT_CALL_INCOMING_BMP);

    g_screen_exit_timer_id = xTimerCreate("BT_AUDIO_TIMER",                /* Just a text name, not used by the kernel. */
                        (BT_AUDIO_SCREEN_EXIT_TIME / portTICK_PERIOD_MS),  /* The timer period in ticks. */
                        pdFALSE,
                        (void *) BT_AUDIO_TIMER_SCREEN_EXIT,
                        bt_audio_timer_callback);
    xTimerStart(g_screen_exit_timer_id, 1);

    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

static void bt_audio_show_outgoing_screen()
{
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_OUTGOING;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 2 - 20,
        IMAGE_ID_BT_CALL_OUTGOING_BMP);

    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

static void bt_audio_show_hang_up_screen()
{
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_HANG_UP;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 2 - 20,
        IMAGE_ID_BT_CALL_HANG_UP_BMP);

    g_screen_exit_timer_id = xTimerCreate("BT_AUDIO_TIMER",                /* Just a text name, not used by the kernel. */
                        (BT_AUDIO_SCREEN_EXIT_TIME / portTICK_PERIOD_MS),  /* The timer period in ticks. */
                        pdFALSE,
                        (void *) BT_AUDIO_TIMER_SCREEN_EXIT,
                        bt_audio_timer_callback);
    xTimerStart(g_screen_exit_timer_id, 1);

    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

void bt_audio_show_link_lost_screen(void)
{
    LOG_I(bt_audio_sink, "[bt_audio_sink]link lost screen\n");

    g_bt_audio_cntx.scr = BT_AUDIO_SCR_LINK_LOST;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 2 - 20,
        IMAGE_ID_BT_DISCONNECT_BMP);
    g_screen_exit_timer_id = xTimerCreate("BT_AUDIO_TIMER",                /* Just a text name, not used by the kernel. */
                        (BT_AUDIO_SCREEN_EXIT_TIME / portTICK_PERIOD_MS),  /* The timer period in ticks. */
                        pdFALSE,
                        (void *) BT_AUDIO_TIMER_SCREEN_EXIT,
                        bt_audio_timer_callback);
    xTimerStart(g_screen_exit_timer_id, 1);

    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

static void bt_audio_show_missed_call_screen(void *param) 
{
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_MISSED_CALL;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    gdi_image_draw_by_id(g_bt_audio_cntx.lcd_width / 2 - 20, 
        g_bt_audio_cntx.lcd_heigth / 2 - 40,
        IMAGE_ID_BT_CALL_MISSED_BMP);

    char *string = (char *)param;
    if (string != NULL && strlen(string) > 0) {
        bt_audio_font_engine_show_string(BT_AUDIO_CALL_DROP_X,
                                         g_bt_audio_cntx.lcd_heigth / 2 + 20,
                                         bt_audio_convert_string_to_wstring(string),
                                         strlen(string),
                                         g_bt_audio_cntx.font_color);
    }
 
    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

static void bt_audio_show_screen(bt_audio_screen_t scr, void *param)
{
    g_bt_audio_cntx.bg_screen = scr;
    g_bt_audio_cntx.entered = true;

    demo_ui_register_touch_event_callback(bt_audio_pen_event_handler, NULL);

    LOG_I(bt_audio_sink, "[bt_audio_sink]show screen:%d\n", scr);
    
    switch (scr) {
        case BT_AUDIO_SCR_IDLE:
            bt_audio_show_idle_screen();
            break;

        case BT_AUDIO_SCR_INCOMING:
            set_active_screen_id(BT_AUDIO_SCREEN_ID);
            bt_audio_show_incoming_screen((bt_audio_caller_t *)param);
            break;

        case BT_AUDIO_SCR_CALLING:
            set_active_screen_id(BT_AUDIO_SCREEN_ID);
            bt_audio_show_calling_screen();
            break;
            
        case BT_AUDIO_SCR_OUTGOING:
            set_active_screen_id(BT_AUDIO_SCREEN_ID);
            bt_audio_show_outgoing_screen();
            break;

        case BT_AUDIO_SCR_HANG_UP:
            set_active_screen_id(BT_AUDIO_SCREEN_ID);
            bt_audio_show_hang_up_screen();
            break;
            
        case BT_AUDIO_SCR_LINK_LOST:
            set_active_screen_id(BT_AUDIO_SCREEN_ID);
            bt_audio_show_link_lost_screen();
            break;

        case BT_AUDIO_SCR_MISSED_CALL:
            set_active_screen_id(BT_AUDIO_SCREEN_ID);
            bt_audio_show_missed_call_screen(param);
            break;
            
        default:
            break;
    }
}

void bt_audio_event_handler(message_id_enum event_id, int32_t param1, void *param2)
{
    LOG_I(bt_audio_sink, "[bt_audio_sink]event_handler--bt: %d, eid: %d, p1: %d, p2: 0x%x\n",
          MESSAGE_ID_BT_AUDIO, event_id, param1, param2);

    if (event_id == MESSAGE_ID_BT_AUDIO) {
        bt_audio_show_screen((bt_audio_screen_t)param1, param2);
    }
}

bt_sink_srv_status_t bt_audio_sink_event_hdr(bt_sink_srv_event_t event_id, void *parameters)
{
    bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)parameters;

    switch (event_id) {
        case BT_SINK_SRV_EVENT_STATE_CHANGE: {
            LOG_I(bt_audio_sink, "[bt_audio_sink] state change, prev state:0x%x, now:0x%x\n", 
                                 event->state_change.previous, event->state_change.now);
            
            if (event->state_change.now & BT_SINK_SRV_STATE_INCOMING) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_INCOMING, (void *)(&g_bt_audio_cntx.caller));
            } else if (event->state_change.now & BT_SINK_SRV_STATE_OUTGOING) {
                LOG_I(bt_audio_sink, "[bt_audio_sink] state change, outgoing\n");    
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_OUTGOING, NULL);
            } else if (event->state_change.now & BT_SINK_SRV_STATE_ACTIVE) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_CALLING, NULL);
            } else if (BT_SINK_SRV_STATE_DISCOVERABLE & event->state_change.previous &&
                    ~event->state_change.now & BT_SINK_SRV_STATE_DISCOVERABLE) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_IDLE, NULL);
            } else if ((BT_SINK_SRV_STATE_ACTIVE & event->state_change.previous &&
                    ~event->state_change.now & BT_SINK_SRV_STATE_ACTIVE) ||
                    (BT_SINK_SRV_STATE_HELD_ACTIVE & event->state_change.previous &&
                    event->state_change.now & BT_SINK_SRV_STATE_ACTIVE) ||
                    (BT_SINK_SRV_STATE_HELD_ACTIVE & event->state_change.previous &&
                    event->state_change.now & BT_SINK_SRV_STATE_HELD_REMAINING)) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_HANG_UP, NULL);
            } else if (BT_SINK_SRV_STATE_OUTGOING & event->state_change.previous &&
                    ~event->state_change.now & BT_SINK_SRV_STATE_OUTGOING) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_IDLE, NULL);
            }

            if (event->state_change.now & BT_SINK_SRV_STATE_STREAMING) {
                audio_player_set_state(AUDIO_PLAYER_STATE_BT_PLAYING);
            } else if (event->state_change.now & BT_SINK_SRV_STATE_CONNECTED) {
                audio_player_reset_state(AUDIO_PLAYER_STATE_BT_PLAYING);
            } else if ((event->state_change.previous & BT_SINK_SRV_STATE_CONNECTED) && (!(event->state_change.now & BT_SINK_SRV_STATE_CONNECTED))) {
                audio_player_reset_state(AUDIO_PLAYER_STATE_BT_PLAYING);
            }
            break;
        }
        case BT_SINK_SRV_EVENT_CONNECTION_INFO_UPDATE: {
            LOG_I(bt_audio_sink, "[bt_audio_sink] conn info update, profile type:0x%02x\n", 
                                 event->connection_info.profile_type);
            
            if (event->connection_info.profile_type == BT_SINK_SRV_TYPE_NONE) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_LINK_LOST, NULL);
            }
            g_bt_audio_cntx.profile_type = event->connection_info.profile_type;
            break;
        }

        case BT_SINK_SRV_EVENT_HF_CALLER_INFORMATION: {
            LOG_I(bt_audio_sink, "[bt_audio_sink] caller information\n");
            
            if (0 == g_bt_audio_cntx.caller.num_len
                    || (0 == g_bt_audio_cntx.caller.name_len && 0 != strlen((char *)event->caller_info.name))) {
                g_bt_audio_cntx.caller.num_len = event->caller_info.num_size;
                memcpy(g_bt_audio_cntx.caller.num, event->caller_info.number, BT_AUDIO_MAX_NUM_LEN);
                g_bt_audio_cntx.caller.name_len = strlen((char *)event->caller_info.name);
                memcpy(g_bt_audio_cntx.caller.name, event->caller_info.name, BT_AUDIO_MAX_NUM_LEN);
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_INCOMING, (void *)(&g_bt_audio_cntx.caller));
            }
            break;
        }

        case BT_SINK_SRV_EVENT_HF_MISSED_CALL: {
            LOG_I(bt_audio_sink, "[bt_audio_sink] missed call\n");
            
            if (strlen((char *)event->caller_info.name) > 0) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_MISSED_CALL, (void *)event->caller_info.name);
            } else if (strlen((char *)event->caller_info.number) > 0) {
                ui_send_event(MESSAGE_ID_BT_AUDIO, BT_AUDIO_SCR_MISSED_CALL, (void *)event->caller_info.number);
            }
            break;
        }
        
        default:
            break;
    }
    return BT_SINK_SRV_STATUS_SUCCESS;
}

void bt_audio_init(void)
{
    uint32_t height = 0, width = 0;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};//white color

    memset(&g_bt_audio_cntx, 0, sizeof(g_bt_audio_cntx));

    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, (void *)&height);
    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, (void *)&width);
    //LCM is 240*240, so lcd_height and lcd_width will be 240
    //g_bt_audio_cntx.lcd_heigth = height;
    //g_bt_audio_cntx.lcd_width = width;
    g_bt_audio_cntx.lcd_heigth = 128;
    g_bt_audio_cntx.lcd_width = 80;
    g_bt_audio_cntx.bg_color = 0;
    g_bt_audio_cntx.font_color = text_color;
    g_bt_audio_cntx.font = GDI_FONT_ENGINE_FONT_MEDIUM;
    bt_sink_app_event_register_callback(BT_SINK_SRV_EVENT_ALL, bt_audio_sink_event_hdr);
}

void bt_audio_show_scan_screen(void)
{
    LOG_I(bt_audio_sink, "[bt_audio_sink]scan screen\n");

    g_bt_audio_cntx.entered = true;
    g_bt_audio_cntx.scr = BT_AUDIO_SCR_SCANNING;
    gdi_draw_filled_rectangle(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth, g_bt_audio_cntx.bg_color);

    set_active_screen_id(BT_AUDIO_SCREEN_ID);

    demo_ui_register_touch_event_callback(bt_audio_pen_event_handler, NULL);
    
    // 2 is max device number
    if (bt_audio_get_connected_device_number() >= 2) {
        bt_audio_font_engine_show_string(5,
                             g_bt_audio_cntx.lcd_heigth / 2,
                             bt_audio_convert_string_to_wstring(STR_BT_AUDIO_LINK_IS_TWO),
                             strlen(STR_BT_AUDIO_LINK_IS_TWO),
                             g_bt_audio_cntx.font_color);
        g_screen_exit_timer_id = xTimerCreate("BT_AUDIO_TIMER",                /* Just a text name, not used by the kernel. */
                            (BT_AUDIO_SCREEN_EXIT_TIME / portTICK_PERIOD_MS),  /* The timer period in ticks. */
                            pdFALSE,
                            (void *) BT_AUDIO_TIMER_SCREEN_EXIT,
                            bt_audio_timer_callback);
        xTimerStart(g_screen_exit_timer_id, 1);
    } else {
       bt_sink_srv_action_send(BT_SINK_SRV_ACTION_DISCOVERABLE, NULL);
#if 0    
        // if one link exist, long press func will redial last dialed number, so can't use EXT_COMMAND
        bt_sink_app_ext_cmd_t *ext_cmd_p = (bt_sink_app_ext_cmd_t *)pvPortMalloc(sizeof(*ext_cmd_p));
        ext_cmd_p->key_value = BT_SINK_SRV_KEY_FUNC;
        ext_cmd_p->key_action = BT_SINK_SRV_KEY_ACT_LONG_PRESS_UP;
        bt_sink_app_event_post((bt_sink_srv_event_t)BT_SINK_EVENT_APP_EXT_COMMAND,
                          (void *)ext_cmd_p, bt_audio_post_event_callback);
#endif

        g_scan_timer_id = xTimerCreate("BT_SCAN_TIMER",                /* Just a text name, not used by the kernel. */
                            (500 / portTICK_PERIOD_MS),  /* The timer period in ticks. */
                            pdTRUE,
                            (void *) BT_AUDIO_TIMER_SCAN_FLICKER,
                            bt_audio_timer_callback);
        xTimerStart(g_scan_timer_id, 1);
    }

    gdi_lcd_update_screen(0, 0, g_bt_audio_cntx.lcd_width, g_bt_audio_cntx.lcd_heigth);
}

uint8_t bt_audio_get_connected_device_number(void)
{
    uint8_t number = 0;

    number = bt_sink_srv_get_connected_device_number();
    return number;
}


uint8_t audio_player_get_state(void)
{
    //LOG_I(bt_audio_sink, "[AudPly]get_state--state: 0x%x\n", g_aud_state);
    return g_aud_state;
}


void audio_player_set_state(uint8_t state)
{
    //LOG_I(bt_audio_sink, "[AudPly]set_state--ori: 0x%x, set: 0x%x\n", g_aud_state, state);
    g_aud_state |= state;
}


void audio_player_reset_state(uint8_t state)
{
    //LOG_I(bt_audio_sink, "[AudPly]reset_state--ori: 0x%x, reset: 0x%x\n", g_aud_state, state);
    g_aud_state &= ~state;
}

