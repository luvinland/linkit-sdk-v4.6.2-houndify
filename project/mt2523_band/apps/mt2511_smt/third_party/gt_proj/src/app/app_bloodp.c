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
#if 1
/* Includes ------------------------------------------------------------------*/
#include "bsp_lcd.h"
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

/*******************************************************************************************************************
 *				Application Task 
 *******************************************************************************************************************/


extern xQueueHandle queAppTask;
extern xSemaphoreHandle semModeChange;
volatile int isBPFnTrigger = 0;
uint32_t bp1_init_val =0, bp2_init_val = 0 ;

//extern int disp_set_font(int FONT);
static void appBP_Upate(uint32_t bp1,uint32_t bp2)
{
	char buffer[20]={0};

	disp_set_font(FONT_16x16);
	printf("appBP_Upate %lu %lu\n\r", bp1, bp2);
	
	sprintf(buffer,"%lu%s%lu",bp1, " / "  ,bp2);

	printf("appBP_Upate %s\n\r", buffer);

	disp_draw_string(6*8,30,buffer);
	
	/* Initial UI */
	lcd_UpdateRect(6*8,30,128,46);
}

static void appBP_Init(void)
{
	char buffer[20];
	
	disp_clear_screen();

	disp_set_font(FONT_16x16);
	disp_draw_string(0,15,"BloodPressure:");
	sprintf(buffer,"%lu%s%lu",bp1_init_val, " / ",bp2_init_val);
	disp_draw_string(6*8,30,buffer);

	/* Initial UI */
	lcd_UpdateRect(0,10,128,64);
}

void appBP_Task(void *pParameters)
{
		tEvent inEvent = {EVENT_NO, NULL} ;
		uint32_t sbp = 0 , dbp = 0 ;
		sensor_blood_pressure_event_t *data;

		ui_task_message_struct_t ui_msg = {0};


		printf("appBP_Task\r\n");

		appBP_Init();
		
		for (;;)
		{		
				if (pdTRUE == xQueueReceive(queAppTask, &inEvent,portMAX_DELAY /*60000/portTICK_RATE_MS*/)) 
				{
						switch (inEvent.event)
						{
												

							case EVENT_UPDATE_BP:

								if (isBPFnTrigger != 0) {

									printf("======appBP_Task====== ev=%d, data=%p\r\n",inEvent.event,inEvent.userdata);
									data = (sensor_blood_pressure_event_t*)inEvent.userdata;
									sbp = data->sbp;
									dbp = data->dbp;
									appBP_Upate(sbp,dbp);
									isBPFnTrigger = 0;
								}

								break;
						
							case EVENT_BUTTON_LEFT_LONG_PRESS:
							


								printf(" =============== B.P. EVENT_BUTTON_LEFT_LONG_PRESS \n\r");
		
								if (isBPFnTrigger == 0) {
									
									ui_msg.message_id = UI_MSG_SENSOR;
                                                                        ui_msg.param1 = SENSOR_TYPE_BLOOD_PRESSURE_MONITOR;
									isBPFnTrigger = 1;
									xQueueSend(ui_main_event_queue, &ui_msg, 0);
									appBP_Upate(0,0);	

								} else {

									/*
									ui_msg.message_id = UI_MSG_SENSOR;
                                                                        ui_msg.param1 = SENSOR_TYPE_BLOOD_PRESSURE_MONITOR;
									isBPFnTrigger = 0;
									appBP_Init();
									xQueueSend(ui_main_event_queue, &ui_msg, 0);
									*/
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

