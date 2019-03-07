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
#include "main_menu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "bsp_ctp.h"
#include "task_def.h"

#include "digital_clock.h"

#define UI_TASK_QUEUE_SIZE 20
#define UI_TASK_NAME "UI_DEMO"
#define UI_TASK_STACK_SIZE 4800
#define UI_TASK_PRIORITY 3


typedef struct ui_task_message_struct {
    int message_id;
    int param1;
    void* param2;
} ui_task_message_struct_t;

struct {
    QueueHandle_t event_queue;
    touch_event_proc_func touch_event_callback_f;
    void* user_data;
} ui_task_cntx;


extern bool isMainMenuScreen;
extern bool isClockScreen;
bool isProcessFlag = false;

static int32_t ui_send_event_from_isr(message_id_enum event_id, int32_t param1, void* param2);

log_create_module(GRAPHIC_TAG, PRINT_LEVEL_INFO);


/*****************************************************************************
 * FUNCTION
 *  demo_ui_register_touch_event_callback
 * DESCRIPTION
 *  register touch event callback
 * PARAMETERS
 *  proc_func       [in]
 *  user_data       [in]
 * RETURNS
 *  void
 *****************************************************************************/
void demo_ui_register_touch_event_callback(touch_event_proc_func proc_func, void* user_data)
{
    GRAPHICLOG("demo_ui_register_touch_event_callback, proc_func:%x", proc_func);
    ui_task_cntx.touch_event_callback_f = proc_func;
    ui_task_cntx.user_data = user_data;
}

#ifdef MTK_CTP_ENABLE
/*****************************************************************************
 * FUNCTION
 *  demo_ui_ctp_callback_func
 * DESCRIPTION
 *  CTP callback function
 * PARAMETERS
 *  param       [in]
 * RETURNS
 *  void
 *****************************************************************************/
void demo_ui_ctp_callback_func(void* param)
{
    ui_send_event_from_isr(MESSAGE_ID_PEN_EVENT, 0, NULL);
}

/*****************************************************************************
 * FUNCTION
 *  demo_ui_process_sigle_event
 * DESCRIPTION
 *  Process ctp event, support single event only
 * PARAMETERS
 *  event       [in]
 * RETURNS
 *  void
 *****************************************************************************/
void demo_ui_process_sigle_event(bsp_ctp_multiple_event_t* event)
{
    // support single touch currently.
    static touch_event_struct_t pre_event = {{0,0},TOUCH_EVENT_MAX};

    GRAPHICLOG("process single event, model:%d", event->model);
    if (event->model <= 0) {
        return;
    }

    if (pre_event.type == TOUCH_EVENT_MOVE && event->points[0].event == CTP_PEN_MOVE) {
        return;
    } 

	/*
    while (i < event->model) {
         GRAPHICLOG("[point] event = %d, piont[0].x = %d, piont[0].y = %d\r\n",
                    event->points[i].event, 	\
                    event->points[i].x,		\
                    event->points[i].y);         
         i++;
    }
    */
    

    pre_event.position.x = event->points[0].x;
    pre_event.position.y = event->points[0].y;
    pre_event.type = (touch_event_enum) event->points[0].event;

    if (ui_task_cntx.touch_event_callback_f) {
        GRAPHICLOG("callback app, type:%d,[%d:%d]", pre_event.type, pre_event.position.x, pre_event.position.y);
        ui_task_cntx.touch_event_callback_f(&pre_event, ui_task_cntx.user_data);
    }
}

/*****************************************************************************
 * FUNCTION
 *  pen_event_handle
 * DESCRIPTION
 *  Process ctp event, support single event only
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
static void pen_event_handle()
{
    bsp_ctp_status_t ret;
    bsp_ctp_multiple_event_t ctp_event;

    // get pen event
    GRAPHICLOG("pen_event_handle");

    ret = bsp_ctp_get_event_data(&ctp_event);
    GRAPHICLOG("ctp_get_event_data ret:%d", ret);
    while (ret == BSP_CTP_OK) {
        ret = bsp_ctp_get_event_data(&ctp_event);
        demo_ui_process_sigle_event(&ctp_event);
    }
    if (ret == BSP_CTP_EVENT_EMPTY) {
        //do nothing
    }

}
#endif

TimerHandle_t check_timer = NULL;
#define MAINMENUTIMEREXPIRENUM 20

void clock_check_timer_os_expire(TimerHandle_t timer)
{
	static int check_num = 0;
	SCREEN_ID_TYPE active_screen_id = get_active_screen_id();
	if(true == isMainMenuScreen) {	
		GRAPHICLOG("isMainMenuScreen true");
		if( (false == isProcessFlag) && (BT_AUDIO_SCREEN_ID != active_screen_id) ) {
			GRAPHICLOG("no touch");
			++check_num;
			if(MAINMENUTIMEREXPIRENUM < check_num)
			{
				check_num = 0;
				isProcessFlag = false;
				show_clock_screen();				
			}
		}else {
			isProcessFlag = false;
			check_num = 0;
		}
	}
}


void check_clock_status()
{
	if (NULL == check_timer)
	{
		check_timer = xTimerCreate( "check clock timer", 1000, pdTRUE, NULL, clock_check_timer_os_expire);
	}
	
	if (check_timer)
	{
		xTimerStart(check_timer, 0);
		GRAPHICLOG("check_clock_status");
	}	
}


/*****************************************************************************
 * FUNCTION
 *  ui_task_msg_handler
 * DESCRIPTION
 *  Process message in queue
 * PARAMETERS
 *  message         [in]
 * RETURNS
 *  void
 *****************************************************************************/
static void ui_task_msg_handler(ui_task_message_struct_t *message)
{
    if (!message) {
        return;
    }
    GRAPHICLOG("ui_task_msg_handler, message_id:%d", message->message_id);
    switch (message->message_id) {
#ifdef MTK_CTP_ENABLE
        case MESSAGE_ID_PEN_EVENT:
			isProcessFlag= true;
            pen_event_handle();
            break;
#endif
        default:
        	GRAPHICLOG("ui_task_msg_handler default case");
            common_event_handler((message_id_enum) message->message_id, (int32_t) message->param1, (void*) message->param2);
            break;
                
    }
}

/*****************************************************************************
 * FUNCTION
 *  ui_send_event
 * DESCRIPTION
 *  Send message to UI task
 * PARAMETERS
 *  event_id        [in]
 *  param1          [in]
 *  param2          [in]
 * RETURNS
 *  int32_t
 *****************************************************************************/
int32_t ui_send_event(message_id_enum event_id, int32_t param1, void* param2)
{
    ui_task_message_struct_t message;
    message.message_id = event_id;
    message.param1 = param1;
    message.param2 = param2;
    
    return xQueueSend(ui_task_cntx.event_queue, &message, 10);
}

/*****************************************************************************
 * FUNCTION
 *  ui_send_event_from_isr
 * DESCRIPTION
 *  Send message to UI task
 * PARAMETERS
 *  event_id        [in]
 *  param1          [in]
 *  param2          [in]
 * RETURNS
 *  int32_t
 *****************************************************************************/
int32_t ui_send_event_from_isr(message_id_enum event_id, int32_t param1, void* param2)
{
    BaseType_t xHigherPriorityTaskWoken;
    ui_task_message_struct_t message;
    message.message_id = event_id;
    message.param1 = param1;
    message.param2 = param2;
    
    return xQueueSendFromISR(ui_task_cntx.event_queue, &message, &xHigherPriorityTaskWoken);
}

/*****************************************************************************
 * FUNCTION
 *  ui_task_main
 * DESCRIPTION
 *  Task mail loop
 * PARAMETERS
 *  arg        [in]
 * RETURNS
 *  void
 *****************************************************************************/
void ui_task_main(void*arg)
{
    ui_task_message_struct_t queue_item;
#ifdef MTK_CTP_ENABLE
    bsp_ctp_status_t ret;
 
    ret = bsp_ctp_init();
    GRAPHICLOG("ctp init, ret:%d", ret);
    ret = bsp_ctp_register_callback(demo_ui_ctp_callback_func, NULL);
    GRAPHICLOG("ctp register callback, ret:%d", ret);
#endif
    arg = arg;
    ui_task_cntx.event_queue = xQueueCreate(UI_TASK_QUEUE_SIZE , sizeof( ui_task_message_struct_t ) );
    GRAPHICLOG("ui_task_main");
    //show_main_menu_screen();	
    show_clock_screen();
	check_clock_status();
    while (1) {
        if (xQueueReceive(ui_task_cntx.event_queue, &queue_item, portMAX_DELAY)) {			
            ui_task_msg_handler(&queue_item);
        }        
    }    
}

/*****************************************************************************
 * FUNCTION
 *  demo_app_start
 * DESCRIPTION
 *  Start UI app
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
TaskHandle_t demo_app_start()
{
    TaskHandle_t task_handler;
    xTaskCreate((TaskFunction_t) ui_task_main, DEMO_UI_TASK_NAME, DEMO_UI_TASK_STACK_SIZE/(( uint32_t )sizeof( StackType_t )), NULL, DEMO_UI_TASK_PRIO, &task_handler );
    GRAPHICLOG("demo_app_start, task_handler:%d", task_handler);
    return task_handler;
}
