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

/**************************************************************************//**
 *
 ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "bsp_lcd.h"
#include "lcd_sw.h"
#include "bw_gdi_api.h"
#include "lcd_manager.h"
//#include "bl_manager.h"
#include "hal_gpio.h"
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
#include "hal_charger.h"

#include "app_task.h"
#include "watch.h"
#include "app_manager.h"
#include "app_status_bar.h"

#define APP_SBAR_LOG(fmt,arg...)   LOG_I(common ,"[SBAR]"fmt,##arg)

#if 1
#define BAT_MAX_LEVEL		4
#define BAT_MAX_VOLT		4200
#define BAT_MIN_VOLT		3000

xQueueHandle queStatusBarTask;
TimerHandle_t vStatusBarTimer;

const uint8_t bat_pic[16] =
{0xFF,0x01,0x01,0x01,0x01,0x01,0x01,0xFF,0x3F,0x20,0xE0,0xE0,0xE0,0xE0,0x20,0x3F};

typedef struct
{
	uint16_t start_x;
	uint16_t start_y;
	uint16_t height;
	uint16_t width;
	uint16_t col;
	const uint8_t *pic;
}bat_image_msg_t;

const bat_image_msg_t bat_image_msg = {
	108,
	1,
	8,
	16,
	12,
	bat_pic
};
 
typedef struct
{
	uint8_t charge;
	uint8_t pre_lv;
	uint8_t cur_lv;
}battery_manager_t;

battery_manager_t bat_msg;
static bool bl_en;

TimerHandle_t vBlTimer;
void vBlTimerCallback( TimerHandle_t xTimer )
{
	bl_TimeStop();
}

void bl_TimeStop(void)
{
	lcd_DisplayOff();
	xTimerStop(vBlTimer, 0);
	bl_en = false;
}

void bl_TimeReset(void)
{
	if (bl_en)
	{
		xTimerReset(vBlTimer, 0);
	}
	else
	{
		lcd_DisplayOn();
		xTimerStart(vBlTimer, 0);
		bl_en = true;
	}
}




bool bl_isOn(void)
{
	return bl_en;
}

void bl_init(uint32_t time)
{
	bl_en = true;
    vBlTimer = xTimerCreate( "vBlTimer",           // Just a text name, not used by the kernel.
                                      ( time*1000 / portTICK_PERIOD_MS), // The timer period in ticks.
                                      pdTRUE,                    // The timer is a one-shot timer.
                                      0,                          // The id is not used by the callback so can take any value.
                                      vBlTimerCallback     // The callback function that switches the LCD back-light off.
                                   );

	xTimerStart(vBlTimer, 0);
}


void battery_display(const bat_image_msg_t *msg,uint8_t level,uint8_t max_level)
{
	uint8_t right;



	disp_draw_pic(msg->start_x,msg->start_y,msg->height,msg->width,msg->pic);
	right = (level*msg->col/max_level);
	disp_draw_rect(msg->start_x,msg->start_y+1,msg->start_x+right,msg->height-1,COLOR_BLACK,1);
	
	/* Initial UI */
	lcd_UpdateRect(msg->start_x,msg->start_y,msg->start_x+msg->width+1,msg->start_y+msg->height+1);



}

uint8_t battery_get_level(int32_t volt,int32_t min_volt,int32_t max_volt,uint8_t max_level)
{
	if (volt < min_volt)
	{
		return 0;
	}
	else if (volt < max_volt)
	{
		return (((volt-min_volt)*max_level)/(max_volt-min_volt));
	}
	else
	{
		return max_level;
	}
}

void vStatusBarCallback( TimerHandle_t xTimer )
{
	    int32_t batt_volt, batt_temp, char_curr, char_volt;
		bool cable_in;
		
	static int rgb_show = 1;
		hal_charger_get_charger_detect_status(&cable_in);
	    hal_charger_meter_get_battery_voltage_sense(&batt_volt);
	    hal_charger_meter_get_battery_temperature(&batt_temp);
	    hal_charger_meter_get_charging_current(&char_curr);
	    hal_charger_meter_get_charger_voltage(&char_volt);

		if (bat_msg.charge != cable_in)
		{
			bl_TimeReset();
			bat_msg.charge = cable_in;


			if (bat_msg.charge == 0) {

				//hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);

			}

		} else if (bat_msg.charge == 1) {

			bl_TimeReset();
		}
		
		if (bat_msg.charge)
		{
			bat_msg.cur_lv++;
			if (bat_msg.cur_lv > BAT_MAX_LEVEL)
				bat_msg.cur_lv = 0;
        	//APP_SBAR_LOG("+ECHAR:%d,%d,%d,%d\r\nOK\r\n", (int)batt_volt, (int)batt_temp, (int)char_curr, (int)char_volt);
		}
		else
		{
			bat_msg.cur_lv = battery_get_level(batt_volt,BAT_MIN_VOLT,BAT_MAX_VOLT,BAT_MAX_LEVEL);
		}
		if (!bl_isOn())
		{
			bat_msg.pre_lv = 0xFF;
		}
		else
		{
			if (bat_msg.cur_lv != bat_msg.pre_lv)
			{
				battery_display(&bat_image_msg,bat_msg.cur_lv,BAT_MAX_LEVEL);
				bat_msg.pre_lv = bat_msg.cur_lv;
			}
		}
	
		if (bat_msg.charge == 1) {
        	if (rgb_show != 0) {
        		//hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
        	} else {

        		//hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
            }

                rgb_show = !rgb_show;

        }

}

void appStatusBar_Task(void *pParameters) 
{
    int32_t batt_volt;
	bool cable_in;
	uint8_t eventGet;
	
	queStatusBarTask = xQueueCreate(2,sizeof(uint8_t));
	bat_msg.charge = 0;

	hal_charger_init();
    hal_charger_meter_get_battery_voltage_sense(&batt_volt);
	hal_charger_get_charger_detect_status(&cable_in);
	bat_msg.cur_lv = battery_get_level(batt_volt,BAT_MIN_VOLT,BAT_MAX_VOLT,BAT_MAX_LEVEL);
	bat_msg.pre_lv = bat_msg.cur_lv;
	battery_display(&bat_image_msg,bat_msg.cur_lv,BAT_MAX_LEVEL);
	if (cable_in)
	{
		bat_msg.charge = 1;
	}
	hal_charger_enable(true);
    vStatusBarTimer = xTimerCreate( "vStatusBarCallback",           // Just a text name, not used by the kernel.
                                      ( 1000 / portTICK_PERIOD_MS), // The timer period in ticks.
                                      pdTRUE,                    // The timer is a one-shot timer.
                                      0,                          // The id is not used by the callback so can take any value.
                                      vStatusBarCallback     // The callback function that switches the LCD back-light off.
                                   );
	xTimerStart(vStatusBarTimer, 0);

	bl_init(15);
	
	for(;;)
	{
		if (pdTRUE == xQueueReceive(queStatusBarTask, &eventGet, portMAX_DELAY))
		{
			switch(eventGet)
			{
				case 0:
					break;
			}
		}
	}
}
#endif


