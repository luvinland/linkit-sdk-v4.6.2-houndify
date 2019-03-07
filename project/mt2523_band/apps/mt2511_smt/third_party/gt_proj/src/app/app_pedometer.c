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

/* Includes ------------------------------------------------------------------*/
#include "bsp_lcd.h"
#include "lcd_sw.h"
#include "bw_gdi_api.h"
#include "lcd_manager.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* device.h includes */
#include "mt2523.h"

/* hal includes */
#include "hal_cache.h"
#include "hal_mpu.h"
#include "hal_uart.h"
#include "hal_clock.h"

#include "watch.h"
#include "app_manager.h"
#if 1
/*******************************************************************************************************************
 *				Application Task 
 *******************************************************************************************************************/

extern xQueueHandle queAppTask;
extern xSemaphoreHandle semModeChange;
volatile int isPEDOFnTrigger = 0;
uint32_t steps_init_val = 0xFFFFFFFF;

//extern int disp_set_font(int FONT);
static void appPedo_Upate(uint32_t steps)
{
	char buffer[20];

	disp_set_font(FONT_16x16);
	sprintf(buffer,"%lu ",steps);
	disp_draw_string(6*8,30,buffer);
	
	/* Initial UI */
	lcd_UpdateRect(6*8,30,128,46);
}

static void appPedo_Init(void)
{
	char buffer[20];
	
	disp_clear_screen();

	disp_set_font(FONT_16x16);
	disp_draw_string(0,10,"Steps:");

	if (0xFFFFFFFF == steps_init_val) 
		disp_draw_string(6*8,30 , "NA                   ");
	else {
		sprintf(buffer,"%lu ",steps_init_val);
		disp_draw_string(6*8,30,buffer);
	}

	/* Initial UI */
	lcd_UpdateRect(0,10,128,64);
}

void appPedo_Task(void *pParameters)
{
		tEvent inEvent = {EVENT_NO, NULL} ;
		uint32_t steps=0;
		ui_task_message_struct_t ui_msg = {0};
		sensor_pedometer_event_t *data;

		printf("appPedo_Task\r\n");

		appPedo_Init();
		
		for (;;)
		{		
				if (pdTRUE == xQueueReceive(queAppTask, &inEvent,portMAX_DELAY /*60000/portTICK_RATE_MS*/)) 
				{
						switch (inEvent.event)
						{
							case EVENT_UPDATE_PEDO :

								if (isPEDOFnTrigger != 0) {
									data = (sensor_pedometer_event_t *)inEvent.userdata;
									steps = data->accumulated_step_count;
									appPedo_Upate(steps);
								}
								break;
						
							case EVENT_BUTTON_LEFT_LONG_PRESS:


                                                                printf(" =============== PEDO EVENT_BUTTON_LEFT_LONG_PRESS \n\r");

                                                                if (isPEDOFnTrigger == 0) {

                                                                        ui_msg.message_id = UI_MSG_SENSOR;
                                                                        ui_msg.param1 = SENSOR_TYPE_PEDOMETER;
                                                                        isPEDOFnTrigger = 1;
									steps_init_val = 0;
									appPedo_Upate(0);
                                                                        xQueueSend(ui_main_event_queue, &ui_msg, 0);

                                                                } else {
                                                                        ui_msg.message_id = UI_MSG_SENSOR;
                                                                        ui_msg.param1 = SENSOR_TYPE_PEDOMETER;
                                                                        isPEDOFnTrigger = 0;
									steps_init_val = 0xFFFFFFFF;
                                                                        appPedo_Init();
                                                                        xQueueSend(ui_main_event_queue, &ui_msg, 0);
                                                                }


                                                                break;
	
							case EVENT_BUTTON_3:
								break;
							case EVENT_BUTTON_4:
								break;
							case EVENT_BUTTON_5:
								break;
							case EVENT_BUTTON_6:
								break;
							case EVENT_INIT_APP:
								break;
							case EVENT_TASK_END:
								xSemaphoreGive(semModeChange);
								vTaskSuspend(NULL);
								break;
							default:
								break;
						} 
				}
		}
}
#endif

