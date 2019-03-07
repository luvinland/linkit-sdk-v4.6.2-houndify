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

#include "ble_ancs_app.h"
#include <string.h>
#include "bt_debug.h"
#include "bt_ancs_common.h"
#include "gdi_font_engine.h"
#include "gdi.h"
#include "bsp_lcd.h"
#include "custom_resource_def.h"
#include "mt25x3_hdk_lcd.h"

ble_ancs_app_cntx_t ancs_app_cntx;
ble_ancs_app_cntx_t *p_ancs_app = &ancs_app_cntx;
extern bool isMainMenuScreen;
extern bool isClockScreen ;


static void ble_ancs_convert_string_to_wstring(uint8_t* input, uint8_t *output)
{
    int32_t index = 0;
    if (!input || !output) {
        return;
    }

    while (*input) {
        output[index] = *input;
        output[index + 1] = 0;
        input++;
        index+=2;
    }
}

static void ble_ancs_pen_event_handler(touch_event_struct_t* pen_event, void* user_data)
{
    static int32_t back_hit;
    
    LOG_I(ANCS, "%s \r\n", __FUNCTION__);

    if (pen_event->position.x <= (p_ancs_app->lcd_w - 1) && pen_event->position.x >= 1) {
        if (pen_event->position.y <= (p_ancs_app->lcd_h - 1) && pen_event->position.y >= 1) {
            if (pen_event->type == TOUCH_EVENT_DOWN) {
                back_hit = 1;
            } else if (back_hit == 1 && pen_event->type == TOUCH_EVENT_UP) {
                back_hit = 0;  
                show_main_menu_screen();
            }
        }
    } else {
        back_hit = 0;
    }
}


void ble_ancs_show_notificaiton(void)
{
    gdi_font_engine_string_info_t query = {0};
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};//white color
    gdi_font_engine_display_string_info_t string_info = {0};
    uint8_t show_str[30]; 
    char buff[30];
    
    isClockScreen = false;
    isMainMenuScreen = true;
    
    LOG_I(ANCS, "[ANCSapp]ble_ancs_show_notificaiton, counter = %d\r\n", p_ancs_app->notif_counter);
    
    demo_ui_register_touch_event_callback(ble_ancs_pen_event_handler, NULL);

    gdi_draw_filled_rectangle(0, 0, p_ancs_app->lcd_w - 1, p_ancs_app->lcd_h -1, 0);
    
    /*show image*/
    gdi_image_draw_by_id(p_ancs_app->lcd_w / 2 - 20, p_ancs_app->lcd_h / 2 - 40, IMAGE_ID_ANCS_NOTIFY_BMP);
    
    /*show string*/
    gdi_font_engine_set_font_size(GDI_FONT_ENGINE_FONT_MEDIUM);
    gdi_font_engine_set_text_color(text_color);

    sprintf(buff, "%d msg", (int)p_ancs_app->notif_counter);
    
    ble_ancs_convert_string_to_wstring((uint8_t *)buff, show_str);

    query.string = show_str;
    query.count = strlen(buff);
    gdi_font_engine_get_string_information(&query);

    string_info.x = (p_ancs_app->lcd_w - (query.width > p_ancs_app->lcd_w ? p_ancs_app->lcd_w : query.width)) / 2;
    string_info.y = p_ancs_app->lcd_h / 2 + 20;
    string_info.baseline_height = -1;
    string_info.string = show_str;
    string_info.length = strlen(buff);
    gdi_font_engine_display_string(&string_info);

    gdi_lcd_update_screen(0, 0, p_ancs_app->lcd_w - 1, p_ancs_app->lcd_h -1);
}


/*****************************************************************************
* FUNCTION
*  ble_ancs_deal_with_new_notification
* DESCRIPTION
* Deal with the newly received notificaitons
* PARAMETERS
*  notif            [IN]        Pointer to notificaiton source
* RETURNS
*  void
*****************************************************************************/
void ble_ancs_deal_with_new_notification(ble_ancs_event_notification_t *notif)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    LOG_I(ANCS, "[ANCSapp]notification type = %d\r\n", notif->event_id);

    if (notif->event_id == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED) {
        p_ancs_app->notif_counter--;
    } else if (notif->event_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED) {
        p_ancs_app->notif_counter++;
    }
}


/*****************************************************************************
* FUNCTION
*  ble_ancs_app_event_callback
* DESCRIPTION
* Send events from ancs service to ancs task
* PARAMETERS
*  ancs_evt            [IN]        Pointer to event
* RETURNS
*  void
*****************************************************************************/
void ble_ancs_app_event_callback(ble_ancs_event_t *ancs_evt)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
        
    LOG_I(ANCS, "[ANCSapp]event_id = %d\r\n", ancs_evt->evt_type);
    
    switch (ancs_evt->evt_type) {
        case BLE_ANCS_EVENT_CONNECTED: 
            if (ancs_evt->result == BT_STATUS_SUCCESS) {            
                p_ancs_app->connection_handle = ancs_evt->connection_handle;
                ble_ancs_enable_data_source(ancs_evt->connection_handle);
                p_ancs_app->status = ANCS_APP_STATUS_ENABLE_DATA_SOURCE;
                
            } else {
                LOG_I(ANCS, "[ANCSapp]Discover service failed!\r\n");
            }
            break;
            
        case BLE_ANCS_EVENT_REQUEST_COMPLETED: 
        {
            if (ancs_evt->result == BT_STATUS_SUCCESS) {
                if (p_ancs_app->status == ANCS_APP_STATUS_ENABLE_DATA_SOURCE) {
                    /* if notification source is not enabled, enable it */

                    ble_ancs_enable_notification_source(ancs_evt->connection_handle);
                    p_ancs_app->status = ANCS_APP_STATUS_ENABLE_NOTIF_SOURCE;
                } else {
                    p_ancs_app->status = ANCS_APP_STATUS_NONE;
                }
            } else {
                LOG_I(ANCS, "[ANCSapp]Enable data source/notification source failed! result = %x\r\n", ancs_evt->result);
                p_ancs_app->status = ANCS_APP_STATUS_NONE;
            }
        }
            break;
            
        case BLE_ANCS_EVENT_IOS_NOTIFICATION: 
            /*If you need ,get notification attribute*/
            ble_ancs_deal_with_new_notification(&ancs_evt->data.notification);

            if (ancs_evt->data.notification.event_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED ||
                ancs_evt->data.notification.event_id == BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED) {
                ble_ancs_show_notificaiton();
            }
            break;

        case BLE_ANCS_EVENT_DISCONNECTED: 
            p_ancs_app->notif_counter = 0;
            LOG_I(ANCS, "[ANCSapp]ANCS disconnected!\r\n");
            break;
        default:
            break;
    }
}


/*****************************************************************************
* FUNCTION
*  ble_ancs_app_init
* DESCRIPTION
* Initialize the context p_ancs_app and ancs service
* PARAMETERS
*
* RETURNS
*  ObStatus
*****************************************************************************/
void ble_ancs_app_init(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    LOG_I(ANCS, "[ANCSapp]ble_ancs_app_init\r\n");
    
    memset(p_ancs_app, 0, sizeof(ble_ancs_app_cntx_t));

    ble_ancs_start(ble_ancs_app_event_callback);

    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, (void *)&p_ancs_app->lcd_h);
    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, (void *)&p_ancs_app->lcd_w);

    LOG_I(ANCS, "%s: height is %d, width is %d!\r\n", __FUNCTION__, p_ancs_app->lcd_h, p_ancs_app->lcd_w);

    p_ancs_app->lcd_h = 128;
    p_ancs_app->lcd_w = 80;
}

