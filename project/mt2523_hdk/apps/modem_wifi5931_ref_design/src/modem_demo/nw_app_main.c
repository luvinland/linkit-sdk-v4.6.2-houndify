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

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_def.h"
#include "nw_app_main.h"
#include "nw_atci.h"
#include "sio_gprot.h"
#include "hal_sleep_manager.h"
#ifdef MTK_CTP_ENABLE
#include "bsp_ctp.h"
#endif

log_create_module(NW_APP, PRINT_LEVEL_INFO);

#define NW_APP_TASK_QUEUE_SIZE 20

static event_handle_func curr_event_handler;
static QueueHandle_t event_queue;

typedef struct nw_app_task_message_struct {
    int message_id;
    int param1;
    void* param2;
} nw_app_task_message_struct_t;


#ifdef LCD_ENABLE
#ifdef MTK_CTP_ENABLE
extern void nw_ui_ctp_callback_func(void* param);
extern void nw_ui_pen_event_handle();
#endif

int32_t nw_demo_send_event_from_isr(message_id_enum event_id, int32_t param1, void* param2)
{
    BaseType_t xHigherPriorityTaskWoken;
    nw_app_task_message_struct_t message;
    message.message_id = event_id;
    message.param1 = param1;
    message.param2 = param2;
    
    return xQueueSendFromISR(event_queue, &message, &xHigherPriorityTaskWoken);
}
#endif

void nw_app_common_event_handler(message_id_enum event_id, int32_t param1, void* param2)
{
    if (curr_event_handler) {
        curr_event_handler(event_id, param1, param2);
    }
}

static void nw_app_task_msg_handler(nw_app_task_message_struct_t *message)
{
    if (!message) {
        return;
    }
    GRAPHICLOG("message_id:%d", message->message_id);
    switch (message->message_id) {
    #ifdef LCD_ENABLE 
    #ifdef MTK_CTP_ENABLE
        case MESSAGE_ID_PEN_EVENT:
            nw_ui_pen_event_handle();
            break;
    #endif
    #endif
        default:
            nw_app_common_event_handler((message_id_enum) message->message_id, (int32_t) message->param1, (void*) message->param2);
            break;
    }
}

int32_t nw_demo_send_event(message_id_enum event_id, int32_t param1, void* param2)
{
    nw_app_task_message_struct_t message;
    message.message_id = event_id;
    message.param1 = param1;
    message.param2 = param2;
    
    return xQueueSend(event_queue, &message, 10);
}

void nw_app_task_main()
{
    nw_app_task_message_struct_t queue_item;
#ifdef MTK_CTP_ENABLE 
#ifdef LCD_ENABLE
    bsp_ctp_status_t ret;
 
    ret = bsp_ctp_init();
    GRAPHICLOG("ctp init, ret:%d", ret);
    ret = bsp_ctp_register_callback(nw_ui_ctp_callback_func, NULL);
    GRAPHICLOG("ctp register callback, ret:%d", ret);
#endif
#endif
    event_queue = xQueueCreate(NW_APP_TASK_QUEUE_SIZE , sizeof(nw_app_task_message_struct_t));
    GRAPHICLOG("ui_task_main");

    nw_atci_cntx_init();

#ifdef LCD_ENABLE
    show_main_screen();
#endif
    while (1) {
        if (xQueueReceive(event_queue, &queue_item, portMAX_DELAY)) {
            nw_app_task_msg_handler(&queue_item);
        }
    }
}

void nw_demo_main()
{
    TaskHandle_t task_handler;
    // TODO:
    //uint8_t sleep_manager_handler = hal_sleep_manager_set_sleep_handle("demp app");

    //hal_sleep_manager_lock_sleep(sleep_manager_handler);
#ifndef MODEM_ON_HDK_ENABLE
    //sio_trigger_modem_wakeup();
#endif
    
    xTaskCreate((TaskFunction_t) nw_app_task_main, DEMO_APP_TASK_NAME, DEMO_APP_TASK_STACK_SIZE/(( uint32_t )sizeof( StackType_t )), NULL, DEMO_APP_TASK_PRIO, &task_handler );
    GRAPHICLOG("nw_demo_main, task_handler:%d", task_handler);
}

void nw_demo_set_event_hander(event_handle_func event_handle)
{
    GRAPHICLOG(" %d", event_handle);
    curr_event_handler = event_handle;
}
