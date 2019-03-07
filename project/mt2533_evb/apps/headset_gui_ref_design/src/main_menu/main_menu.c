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
#include "main_menu.h"
#include "memory_attribute.h"
#include "mt25x3_hdk_lcd.h"
#include "bsp_lcd.h"
#include "mt25x3_hdk_backlight.h"

#define CONFIG_INCLUDE_HEADER
#include "main_menu_screen_config.h"
#undef CONFIG_INCLUDE_HEADER
#define CONFIG_INCLUD_BODY

#define DEMO_ITEM_NAME_MAX_LEN 50
#define PERVIOUS_PAGE_STRING_NAME "previous page"
#define NEXT_PAGE_STRING_NAME "next page"
#define DEMO_TITLE_STRING_NAME "Demo option:"
typedef struct list_item_struct {
    show_screen_proc_f show_screen_f;
    event_handle_func event_handle_f;
	uint32_t image_id;
} list_item_struct_t;


const list_item_struct_t demo_item[] =
{
#include "main_menu_screen_config.h"
};


struct {
    int32_t total_item_num;
    int32_t top_gap;
    int32_t left_gap;
    int32_t right_gap;
    int32_t bottom_gap;
	int32_t icon_height;
	int32_t icon_width;
    int32_t LCD_WIDTH;
    int32_t LCD_HEIGHT;
	int32_t curr_icon_seq;
    gdi_font_engine_color_t color;
} main_screen_cntx;


//ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN

uint32_t LCD_CURR_HEIGHT = 128, LCD_CURR_WIDTH = 80;

event_handle_func curr_event_handler;

#define RESIZE_RATE LCD_CURR_HEIGHT/240
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN unsigned char frame_buffer[ScrnWidth*ScrnHeight*2];

bool isMainMenuScreen = false;
extern bool isClockScreen;

SCREEN_ID_TYPE g_screen_type_id;

static void main_menu_screen_draw(void);

void main_screen_event_handle(message_id_enum event_id, int32_t param1, void* param2)
{
    GRAPHICLOG("main_screen_event_handle");
    for (int32_t i = 0; i < main_screen_cntx.total_item_num; i++) {
        demo_item[i].event_handle_f(event_id, param1, param2);
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
void main_screen_pen_event_handler(touch_event_struct_t* touch_event, void* user_data)
{
	static int32_t ctp_move_flag = 0;
	static int32_t previous_move_x = 0;
	static int32_t previous_move_y = 0;

	GRAPHICLOG("main_screen_pen_event_handler invoke");
	
    if (touch_event->type == TOUCH_EVENT_DOWN) {
        previous_move_x = 0;
		previous_move_y = 0;
		ctp_move_flag = 0;
		GRAPHICLOG("TOUCH_EVENT_DOWN detect");
        return;
    } else if(touch_event->type == TOUCH_EVENT_MOVE){
		ctp_move_flag = 1;
		previous_move_x = touch_event->position.x;
		previous_move_y = touch_event->position.y;
		GRAPHICLOG("TOUCH_EVENT_MOVE detect [%d]  [%d]", previous_move_x, previous_move_y);
	} else if (touch_event->type == TOUCH_EVENT_UP) {
        if (1 == ctp_move_flag) {
			// up moving
			if(previous_move_y > touch_event->position.y) {
				if(0 < main_screen_cntx.total_item_num) {
					if(0 == main_screen_cntx.curr_icon_seq) {
						main_screen_cntx.curr_icon_seq = main_screen_cntx.total_item_num - 1;
					}else {
						main_screen_cntx.curr_icon_seq = (main_screen_cntx.curr_icon_seq - 1) % main_screen_cntx.total_item_num;
					}
				}
            }else if(previous_move_y < touch_event->position.y) {  // down moving  
            	if(0 < main_screen_cntx.total_item_num) {
					main_screen_cntx.curr_icon_seq = (main_screen_cntx.curr_icon_seq + 1) % main_screen_cntx.total_item_num;
            	}
			}
			previous_move_x = 0;
			previous_move_y = 0;
			ctp_move_flag = 0;
			GRAPHICLOG("\nmain_screen_cntx.curr_icon_seq  %d\n", main_screen_cntx.curr_icon_seq);
			main_menu_screen_draw();
        }else{ // enter app screen
        	GRAPHICLOG("TOUCH_EVENT enter app screen  curr_icon_seq: %d", main_screen_cntx.curr_icon_seq);
			curr_event_handler = demo_item[main_screen_cntx.curr_icon_seq].event_handle_f;
            if (demo_item[main_screen_cntx.curr_icon_seq].show_screen_f) {
                demo_item[main_screen_cntx.curr_icon_seq].show_screen_f();
            }
		}
    }    
}

/*****************************************************************************
 * FUNCTION
 *  common_event_handler
 * DESCRIPTION
 *  dispatch event to current screen
 * PARAMETERS
 *  event_id     [in]
 *  param1       [in]
 *  param2       [in]
 * RETURNS
 *  void
 *****************************************************************************/
void common_event_handler(message_id_enum event_id, int32_t param1, void* param2)
{
    if (curr_event_handler) {
        curr_event_handler(event_id, param1, param2);
    }
}

/*****************************************************************************
 * FUNCTION
 *  convert_string_to_wstring
 * DESCRIPTION
 *  convert string to Wstring
 * PARAMETERS
 *  string     [in]
 * RETURNS
 *  uint8_t*
 *****************************************************************************/
uint8_t* convert_string_to_wstring(uint8_t* string)
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
static void main_screen_cntx_init()
{
    static int32_t is_init;
    if (is_init)
        return;
    is_init = 1;
    
    main_screen_cntx.LCD_WIDTH = LCD_CURR_WIDTH;
    main_screen_cntx.LCD_HEIGHT = LCD_CURR_HEIGHT;
	main_screen_cntx.icon_height= 40;
	main_screen_cntx.icon_width = 40;
    main_screen_cntx.top_gap = (main_screen_cntx.LCD_HEIGHT - main_screen_cntx.icon_height) / 2;	
    main_screen_cntx.left_gap = (main_screen_cntx.LCD_WIDTH - main_screen_cntx.icon_width) / 2;
    main_screen_cntx.right_gap = main_screen_cntx.left_gap;
    main_screen_cntx.bottom_gap = main_screen_cntx.top_gap;
    main_screen_cntx.total_item_num = sizeof(demo_item)/sizeof(list_item_struct_t);
    main_screen_cntx.color.alpha = 0xFF;
    main_screen_cntx.color.red = 0xFF;
    main_screen_cntx.color.green = 0xFF;
    main_screen_cntx.color.blue = 0xFF;
	// page has one icon
	main_screen_cntx.curr_icon_seq = 0;
	GRAPHICLOG("\nmain_screen_cntx.total_item_num : %d\n",main_screen_cntx.total_item_num);
    gdi_init(main_screen_cntx.LCD_WIDTH, main_screen_cntx.LCD_HEIGHT, GDI_COLOR_FORMAT_16, frame_buffer);
}

/*****************************************************************************
 * FUNCTION
 *  my_itoa
 * DESCRIPTION
 *  convert int to string
 * PARAMETERS
 *  num     [in]
 *  str     [in]
 *  radix   [in]
 * RETURNS
 *  char*
 *****************************************************************************/
char* my_itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i=0,j,k;
    char temp;

    if(radix==10 && num<0) {
        unum=(unsigned)-num;
        str[i++]='-';
    } else {
        unum=(unsigned)num;
    }

    do {
        str[i++] = index[unum%(unsigned)radix];
        unum /= radix;
    } while(unum);

    str[i]='\0';
    if(str[0]=='-') {
        k=1;
    } else {
        k=0;
    }

    for(j=k;j<=(i-1)/2;j++)
    {
        temp=str[j];
        str[j]=str[i-1+k-j];
        str[i-1+k-j]=temp;
    }
    return str;
}


/*****************************************************************************
 * FUNCTION
 *  main_menu_screen_draw
 * DESCRIPTION
 *  Draw main screen
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
static void main_menu_screen_draw()
{
	GRAPHICLOG("main_menu_screen_draw begin  %d\n", main_screen_cntx.curr_icon_seq);
    
	if(main_screen_cntx.curr_icon_seq >= 0){
		gdi_draw_filled_rectangle(0,0,LCD_CURR_WIDTH, LCD_CURR_HEIGHT,0);
		
		gdi_image_draw_by_id(main_screen_cntx.left_gap, 
							 main_screen_cntx.top_gap, 
							 demo_item[main_screen_cntx.curr_icon_seq].image_id);

		GRAPHICLOG("main_menu_screen_draw middle, %d  %d\n", main_screen_cntx.left_gap, main_screen_cntx.top_gap);
		gdi_lcd_update_screen(0, 0, LCD_CURR_WIDTH - 1, LCD_CURR_HEIGHT - 1);
	}    

	GRAPHICLOG("main_menu_screen done\n");
}

/*****************************************************************************
 * FUNCTION
 *  show_main_menu_screen
 * DESCRIPTION
 *  show main screen
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void show_main_menu_screen()
{
    static int32_t is_init;

	isClockScreen = false;
	isMainMenuScreen = true;
	
	set_active_screen_id(MAIN_MENU_SCREEN_ID);
	
    curr_event_handler = main_screen_event_handle;
    demo_ui_register_touch_event_callback(main_screen_pen_event_handler, NULL);

    if (!is_init) {
        is_init = 1;
        bsp_lcd_init(0xF800);
        bsp_backlight_init();
        bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, &LCD_CURR_HEIGHT);
        bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, &LCD_CURR_WIDTH);

		GRAPHICLOG("show_main_menu_screen  width: %d  height:  %d", LCD_CURR_HEIGHT, LCD_CURR_WIDTH);
		LCD_CURR_HEIGHT = 128;
		LCD_CURR_WIDTH = 80;
    }
    gdi_font_engine_set_font_size(GDI_FONT_ENGINE_FONT_MEDIUM);

    main_screen_cntx_init();
    
    gdi_font_engine_set_text_color(main_screen_cntx.color);
    
#ifndef _MSC_VER
    GRAPHICLOG("show_main_menu_screen");
#endif
    main_menu_screen_draw();
}


void set_active_screen_id(SCREEN_ID_TYPE screen_id)
{
	GRAPHICLOG("\nset_active_screen_id: %d\n",g_screen_type_id);
	g_screen_type_id = screen_id;
}

SCREEN_ID_TYPE get_active_screen_id()
{
	GRAPHICLOG("\nget_active_screen_id: %d\n",g_screen_type_id);
	return g_screen_type_id;
}


