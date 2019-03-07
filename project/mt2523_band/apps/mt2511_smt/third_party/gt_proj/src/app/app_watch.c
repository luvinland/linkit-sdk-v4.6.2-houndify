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
#include "hal_rtc.h"

#include "watch.h"
//#include "clock.h"
#include "app_manager.h"
#if 1

#define APP_WATCH_LOG(fmt,arg...)   //LOG_I(common ,"[WATCH]"fmt,##arg)

/*******************************************************************************************************************
 *				Application Task 
 *******************************************************************************************************************/
 const char day_str[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const hal_rtc_time_t base_time = {
    0,                                  /**< Seconds after the minute   - [0,59]  */
    0,                                 /**< Minutes after the hour     - [0,59]  */
    0,                                /**< Hours after the midnight   - [0,23]  */
    1,                                  /**< Day of the month           - [1,31]  */
    1,                                  /**< Months                     - [1,12]  */
    5,                                 /**< Days in a week             - [1,7]   */
    16                                /**< Years                      - [0,127] */
};

extern xQueueHandle queAppTask;
extern xSemaphoreHandle semModeChange;

hal_rtc_status_t set_current_time(hal_rtc_time_t time)
{
	hal_rtc_status_t ret;

	APP_WATCH_LOG("set_current_time\n");

	ret = hal_rtc_init();
	if (HAL_RTC_STATUS_OK != ret){
		//error handling
		APP_WATCH_LOG("hal_rtc_init fail--ret->%d\n",ret);
		return ret;
	}

	//Set the RTC current time
	ret = hal_rtc_set_time(&time);
	if (HAL_RTC_STATUS_OK != ret){
		//error handling
		APP_WATCH_LOG("hal_rtc_set_time fail--ret->%d\n",ret);
		return ret;
	}


	return ret;

}

hal_rtc_status_t watch_get_current_time(hal_rtc_time_t *time)
{
	hal_rtc_status_t ret;

	ret = hal_rtc_init();
	if (HAL_RTC_STATUS_OK != ret){
		//error handling
		APP_WATCH_LOG("hal_rtc_init fail--ret->%d\n",ret);
		return ret;
	}

	//Set the RTC current time
	ret = hal_rtc_get_time(time);
	if (HAL_RTC_STATUS_OK != ret){
		//error handling
		APP_WATCH_LOG("hal_rtc_set_time fail--ret->%d\n",ret);
		return ret;
	}

	return ret;
}
//extern int disp_set_font(int FONT);

static void appWatch_UpateTime(void)
{
	char buffer[20];
	hal_rtc_time_t time;

	watch_get_current_time(&time);
	disp_set_font(FONT_16x16);
	sprintf(buffer,"20%02d-%02d-%02d",time.rtc_year,time.rtc_mon,time.rtc_day);
	disp_draw_string(24,11,buffer);
	sprintf(buffer,"%02d:%02d  %s",time.rtc_hour,time.rtc_min,day_str[time.rtc_week%7]);
	disp_draw_string(24,40,buffer);
//	sprintf(buffer,"%s",day_str[time.DayIndex]);
//	disp_draw_string(52,50,buffer);

	APP_WATCH_LOG("RTC-> (20%02d-%02d-%02d) (%02d:%02d:%02d) (%d)\n",time.rtc_year,time.rtc_mon,time.rtc_day
														,time.rtc_hour,time.rtc_min,time.rtc_sec
														,time.rtc_week);

	/* Initial UI */
	lcd_UpdateRect(24,10,108,56);
}

void appWatch_rtc_time_callback(void *user_data)
{
			 tEvent outEvent;
			// event = WF_EVENT_RTC;
			 BaseType_t xHigherPriorityTaskWoken;
			LOG_I(common,"rtc init failed ");
			outEvent.event = EVEINT_1MIN;
			//xQueueSend(queAppMgr,&outEvent,0);
			 xQueueSendFromISR(queAppMgr, (void *) &outEvent , &xHigherPriorityTaskWoken);
			 
			 // Now the buffer is empty we can switch context if necessary.
			 if( xHigherPriorityTaskWoken ) {
			     // Actual macro used here is port specific.
			     portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
 }

}

static void appWatch_Init(void)
{
	char buffer[20];
	//applib_time_struct time;
	hal_rtc_time_t time;
	
	disp_clear_screen();

	watch_get_current_time(&time);
	if (time.rtc_year == 0)
	{
		set_current_time(base_time);
	}
	watch_get_current_time(&time);
	disp_set_font(FONT_16x16);
	sprintf(buffer,"20%02d-%02d-%02d",time.rtc_year,time.rtc_mon,time.rtc_day);
	disp_draw_string(24,11,buffer);
	sprintf(buffer,"%02d:%02d  %s",time.rtc_hour,time.rtc_min,day_str[time.rtc_week%7]);
	disp_draw_string(24,40,buffer);
//	sprintf(buffer,"%s",day_str[time.DayIndex]);
//	disp_draw_string(52,50,buffer);
	APP_WATCH_LOG("RTC-> (20%02d-%02d-%02d) (%02d:%02d:%02d) (%d)\n",time.rtc_year,time.rtc_mon,time.rtc_day
														,time.rtc_hour,time.rtc_min,time.rtc_sec
														,time.rtc_week);

	/* Initial UI */
	lcd_UpdateRect(0,10,128,64);
}

void appWatch_Task(void *pParameters)
{
		tEvent inEvent = {EVENT_NO, NULL} ;

		APP_WATCH_LOG("appWatch_Task\r\n");

		appWatch_Init();

     //	hal_rtc_set_time_callback(appWatch_rtc_time_callback, NULL);
		
		for (;;)
		{		
				if (pdTRUE == xQueueReceive(queAppTask, &inEvent,portMAX_DELAY /*60000/portTICK_RATE_MS*/)) 
				{
						switch (inEvent.event)
						{
							case EVEINT_1MIN :
								appWatch_UpateTime();
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

