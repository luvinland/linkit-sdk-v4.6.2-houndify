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
#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include "stdio.h"
#include "stdlib.h"

#endif
#include "gdi.h"
#include "gdi_font_engine.h"
#include "digital_clock.h"
#include "memory_attribute.h"
#include "mt25x3_hdk_lcd.h"
#include "bsp_lcd.h"
#include "mt25x3_hdk_backlight.h"

#ifdef __GNUC__
#include <sys/time.h>
#endif /*__GNUC__*/
#include "FreeRTOS.h"
#include "timers.h"
#include "hal_rtc.h"
#include "syslog.h"

#include "custom_resource_def.h"

#include "main_menu.h"
#include "bt_audio.h"


typedef struct num_info{
	int index;
	int image_id;
}num_info;

#define SPLITNUM 10

num_info num_table[11] = {{0, IMAGE_ID_DIGITAL_CLOCK_DIGIT_0_BMP},
						 {1, IMAGE_ID_DIGITAL_CLOCK_DIGIT_1_BMP},
						 {2, IMAGE_ID_DIGITAL_CLOCK_DIGIT_2_BMP},
						 {3, IMAGE_ID_DIGITAL_CLOCK_DIGIT_3_BMP},
						 {4, IMAGE_ID_DIGITAL_CLOCK_DIGIT_4_BMP},
						 {5, IMAGE_ID_DIGITAL_CLOCK_DIGIT_5_BMP},
						 {6, IMAGE_ID_DIGITAL_CLOCK_DIGIT_6_BMP},
						 {7, IMAGE_ID_DIGITAL_CLOCK_DIGIT_7_BMP},
						 {8, IMAGE_ID_DIGITAL_CLOCK_DIGIT_8_BMP},
						 {9, IMAGE_ID_DIGITAL_CLOCK_DIGIT_9_BMP},
						 {10, IMAGE_ID_DIGITAL_CLOCK_COLON_BMP}};

typedef struct clock_info{
	int first_hour_position_x;
	int first_hour_position_y;
	int second_hour_position_x;
	int second_hour_position_y;
	int spilt_symbol_x;
	int spilt_symbol_y;
	int first_minutes_position_x;
	int first_minutes_position_y;
	int second_minutes_position_x;
	int second_minutes_position_y;	
	int hour_val;
	int minutes_val;
	int seconds_val;
	int bt_audio_icon_position_x;
	int bt_audio_icon_position_y;
	int bt_player_icon_position_x;
	int bt_player_icon_position_y;
	int charger_icon_position_x;
	int charger_icon_position_y;
}clock_info;

bool isCharger = false;
int currentCapacity = 0;
#define LOW_BATTERY_PERCENTAGE 30
#define MIDDLE_BATTERY_PERCENTAGE 50
#define HIGH_BATTERY_PERCENTAGE 80
#define FULL_BATTERY_PERCENTAGE 100

clock_info g_clock_info;

TimerHandle_t clock_timer = NULL;

extern bool isMainMenuScreen;
bool isClockScreen = true;


void draw_charger_icon();
void clock_draw();
void draw_bt_player_icon();


void clock_val_update()
{
	// obtain system closk time here
	hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        GRAPHICLOG("\nhal_rtc_get_time = %d\n", ret);
        configASSERT(0);
        return;
    }
	
	g_clock_info.hour_val = rtc_time.rtc_hour;
	g_clock_info.minutes_val = rtc_time.rtc_min;
	g_clock_info.seconds_val = rtc_time.rtc_sec;
}

/*****************************************************************************
 * FUNCTION
 *  main_screen_cntx_init
 * DESCRIPTION
 *  Init main screen context
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
static void clock_cntx_init()
{
	uint32_t width = 0, height = 0;
	uint32_t split_width = 0, split_height = 0;
	g_clock_info.bt_audio_icon_position_x = 35;
	g_clock_info.bt_audio_icon_position_y = 10;	
	g_clock_info.bt_player_icon_position_x = 17;
	g_clock_info.bt_player_icon_position_y = 10;
	g_clock_info.charger_icon_position_x = 53;
	g_clock_info.charger_icon_position_y = 10;
	g_clock_info.first_hour_position_x = 5;
	g_clock_info.first_hour_position_y = 40;	
	gdi_image_get_dimension_by_id(num_table[0].image_id, &width, &height);
	g_clock_info.second_hour_position_x = g_clock_info.first_hour_position_x + width;
	g_clock_info.second_hour_position_y = g_clock_info.first_hour_position_y;
	g_clock_info.spilt_symbol_x = g_clock_info.second_hour_position_x + width;
	g_clock_info.spilt_symbol_y = g_clock_info.first_hour_position_y;
	gdi_image_get_dimension_by_id(num_table[SPLITNUM].image_id, &split_width, &split_height);
	g_clock_info.first_minutes_position_x = g_clock_info.spilt_symbol_x + split_width;
	g_clock_info.first_minutes_position_y =  g_clock_info.first_hour_position_y;
	g_clock_info.second_minutes_position_x = g_clock_info.first_minutes_position_x + width;
	g_clock_info.second_minutes_position_y = g_clock_info.first_hour_position_y;
	
	clock_val_update();
	
	//GRAPHICLOG("\n init current time = %d : %d : %d\n", g_clock_info.hour_val, g_clock_info.minutes_val, g_clock_info.seconds_val);
}


void clock_draw_init()
{
	GRAPHICLOG("clock_draw_init");
	gdi_init(LCD_CURR_WIDTH, LCD_CURR_HEIGHT, GDI_COLOR_FORMAT_16, frame_buffer);
	gdi_draw_filled_rectangle(0,0,LCD_CURR_WIDTH, LCD_CURR_HEIGHT,0);
	GRAPHICLOG("clock_draw_init done");
}


void clock_timer_os_expire(TimerHandle_t timer)
{
    SCREEN_ID_TYPE active_screen_id = get_active_screen_id();
	if( (false == isMainMenuScreen) && (true == isClockScreen) && (WATCH_FACE_SCREEN_ID == active_screen_id)) {
		clock_val_update();
		draw_bt_player_icon();
		clock_draw();
		draw_charger_icon();
		//GRAPHICLOG("clock_timer_os_expire");
	}else{
		//GRAPHICLOG("clock still in mainmenu screen not update");
	}
}

/*****************************************************************************
 * FUNCTION
 *  draw_bt_audio_icon
 * DESCRIPTION
 *  draw_bt_audio_icon
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void draw_bt_audio_icon()
{
	uint8_t conn_num = bt_audio_get_connected_device_number();
	int bt_audio_image_id = IMAGE_ID_BT_STATUS_DISCONNECT_BMP;
	//GRAPHICLOG("draw_bt_audio_icon number: %d\n",conn_num);
	switch (conn_num) {
		case 0:
			bt_audio_image_id = IMAGE_ID_BT_STATUS_DISCONNECT_BMP;
			break;
		case 1:
			bt_audio_image_id = IMAGE_ID_BT_STATUS_SINGLE_LINK_BMP;
			break;
		case 2:
			bt_audio_image_id = IMAGE_ID_BT_STATUS_MULTI_LINK_BMP;
			break;
		default:
			break;
	}
	gdi_image_draw_by_id(g_clock_info.bt_audio_icon_position_x, 
						 g_clock_info.bt_audio_icon_position_y, 
						 bt_audio_image_id);
}


/*****************************************************************************
 * FUNCTION
 *  draw_charger_icon
 * DESCRIPTION
 *  draw_charger_icon
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void draw_charger_icon()
{
	int image_charger_id = IMAGE_ID_BATT_POWER_MIDDLE_BMP;
	if(true == isCharger) {
		image_charger_id= IMAGE_ID_BATT_CHARGING_BMP;
	}else {
		GRAPHICLOG("currentCapacity :%d\n",image_charger_id);
		if (LOW_BATTERY_PERCENTAGE >= currentCapacity) {
			image_charger_id = IMAGE_ID_BATT_POWER_LOW_BMP;
		}else if ( (LOW_BATTERY_PERCENTAGE < currentCapacity) && (HIGH_BATTERY_PERCENTAGE >= currentCapacity) ) {
			image_charger_id = IMAGE_ID_BATT_POWER_MIDDLE_BMP;
		}else if ( (HIGH_BATTERY_PERCENTAGE < currentCapacity) && (FULL_BATTERY_PERCENTAGE > currentCapacity) ) {
			image_charger_id = IMAGE_ID_BATT_POWER_HIGH_BMP;
		}else if (FULL_BATTERY_PERCENTAGE == currentCapacity) {
			image_charger_id = IMAGE_ID_BATT_POWER_FULL_BMP;
		}
	}
	gdi_image_draw_by_id(g_clock_info.charger_icon_position_x, 
						 g_clock_info.charger_icon_position_y, 
						 image_charger_id);
}

void draw_bt_player_icon()
{
	uint8_t bt_palyer_status = audio_player_get_state();
	if(0 != bt_palyer_status) {
		GRAPHICLOG("bt_palyer_status :%d\n",bt_palyer_status);
		gdi_image_draw_by_id(g_clock_info.bt_player_icon_position_x, 
						 g_clock_info.bt_player_icon_position_y, 
						 IMAGE_ID_MUSIC_STATUS_PLAYING_BMP);
	}else {
		// clean backgournd in order to erase bt icon if already show in clock screen
		// so it should be first invoked in watch face show
		gdi_draw_filled_rectangle(0,0,LCD_CURR_WIDTH, LCD_CURR_HEIGHT,0);
		draw_bt_audio_icon();
		draw_charger_icon();
	}
}


/*****************************************************************************
 * FUNCTION
 *  clock_draw
 * DESCRIPTION
 *  Draw clock screen
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
 void clock_draw()
{
	int hour_fir_num = 0, hour_sec_num=0;
	int min_fir_num=0, min_sec_num=0;
	int hour = (g_clock_info.hour_val >= 12) ? (g_clock_info.hour_val - 12) : g_clock_info.hour_val;
	int min = g_clock_info.minutes_val;
		
	hour_fir_num = (hour >= 10) ? (hour / 10) : 0;
	hour_sec_num = hour % 10;
	min_fir_num = (min >= 10) ? (min / 10) : 0;
	min_sec_num = min % 10; 
		
	GRAPHICLOG("\nclock_draw, %d  %d : %d  %d\n", hour_fir_num, hour_sec_num,
											  min_fir_num, min_sec_num);

	gdi_image_draw_by_id(g_clock_info.first_hour_position_x, 
						 g_clock_info.first_hour_position_y, 
						 num_table[hour_fir_num].image_id);
	gdi_image_draw_by_id(g_clock_info.second_hour_position_x, 
						 g_clock_info.second_hour_position_y, 
						 num_table[hour_sec_num].image_id);
	gdi_image_draw_by_id(g_clock_info.spilt_symbol_x, 
						 g_clock_info.spilt_symbol_y, 
						 num_table[SPLITNUM].image_id);
	gdi_image_draw_by_id(g_clock_info.first_minutes_position_x, 
						 g_clock_info.first_minutes_position_y, 
						 num_table[min_fir_num].image_id);
	/*
	GRAPHICLOG("\nclock_draw, second_hour_position_x: %d  second_hour_position_y: %d : spilt_symbol_x: %d  %spilt_symbol_y: d first_minutes_position_x: %d  g_clock_info.first_minutes_position_y: %d\n",
		g_clock_info.second_hour_position_x, g_clock_info.second_hour_position_y, 
		g_clock_info.spilt_symbol_x, g_clock_info.spilt_symbol_y,
		g_clock_info.second_minutes_position_x, g_clock_info.second_minutes_position_y);
       */
	gdi_image_draw_by_id(g_clock_info.second_minutes_position_x, 
						 g_clock_info.second_minutes_position_y, 
						 num_table[min_sec_num].image_id);
		
	gdi_lcd_update_screen(0, 0, LCD_CURR_WIDTH - 1, LCD_CURR_HEIGHT - 1);
	
	GRAPHICLOG("clock_draw done");	

}

void clock_update()
{
	if (NULL == clock_timer)
	{
		clock_timer = xTimerCreate( "clock timer", 1000, pdTRUE, NULL, clock_timer_os_expire);
	}
	
	if (clock_timer)
	{
		xTimerStart(clock_timer, 0);
		GRAPHICLOG("\nstart lock timer\n");
	}	
}


/*****************************************************************************
 * FUNCTION
 *  show_clock_screen
 * DESCRIPTION
 *  show_clock_screen
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void clock_screen_draw()
{
	static bool init_flag = false;
	if(false == init_flag) {
		clock_cntx_init();
		clock_draw_init();    
		clock_update();
		init_flag = true;
	}
	if(false == isMainMenuScreen) {
		draw_bt_player_icon();
		clock_draw();
		draw_bt_audio_icon();
		draw_charger_icon();
	}
}

/*****************************************************************************
 * FUNCTION
 *  main_screen_pen_event_handler
 * DESCRIPTION
 *  Process pen event
 * PARAMETERS
 *  touch_event     [in]
 *  user_data       [in]
 * RETURNS
 *  void
 *****************************************************************************/
void clock_screen_pen_event_handler(touch_event_struct_t* touch_event, void* user_data)
{	
	static bool ctp_done_flag = 0;

	GRAPHICLOG("main_screen_pen_event_handler invoke");
	
    if (touch_event->type == TOUCH_EVENT_DOWN) {
		ctp_done_flag = true;
		GRAPHICLOG("clock_screen_pen_event_handler TOUCH_EVENT_DOWN detect");
        return;
    } else if (touch_event->type == TOUCH_EVENT_UP) {
        if (true == ctp_done_flag) {
			ctp_done_flag = false;			
			GRAPHICLOG("clock prepare to show main menu");
			demo_ui_register_touch_event_callback(NULL, NULL);
			show_main_menu_screen();
		}
    }
}

extern event_handle_func curr_event_handler;
extern void main_screen_event_handle(message_id_enum event_id, int32_t param1, void* param2);

void show_clock_screen()
{
    static int32_t is_init;
	gdi_font_engine_color_t color;

	isClockScreen = true;
	isMainMenuScreen = false;

	set_active_screen_id(WATCH_FACE_SCREEN_ID);
	
    curr_event_handler = main_screen_event_handle;
    demo_ui_register_touch_event_callback(clock_screen_pen_event_handler, NULL);

	
    if (!is_init) {
        is_init = 1;
        bsp_lcd_init(0xF800);
        bsp_backlight_init();
        bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, &LCD_CURR_HEIGHT);
        bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, &LCD_CURR_WIDTH);

		GRAPHICLOG("show_clock_screen  width: %d  height:  %d", LCD_CURR_HEIGHT, LCD_CURR_WIDTH);
		LCD_CURR_HEIGHT = 128;
		LCD_CURR_WIDTH = 80;
    }
    gdi_font_engine_set_font_size(GDI_FONT_ENGINE_FONT_MEDIUM);

	color.alpha = 0xff;
	color.blue = 0xff;
	color.green = 0xff;
	color.red = 0xff;
    gdi_font_engine_set_text_color(color);
    
#ifndef _MSC_VER
    GRAPHICLOG("show_clock_screen");
#endif
    clock_screen_draw();
    
}

