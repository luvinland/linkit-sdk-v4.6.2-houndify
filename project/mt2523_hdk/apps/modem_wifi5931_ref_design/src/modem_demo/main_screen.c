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
#include "stdbool.h"
#include <string.h>
#include "stdio.h"
#include "stdlib.h"

#include "nw_app_main.h"
#include "gdi.h"
#include "gdi_font_engine.h"
#include "main_screen.h"
#include "memory_attribute.h"
#ifdef MTK_CTP_ENABLE
#include "bsp_ctp.h"
#endif

#define CONFIG_INCLUDE_HEADER
#include "screen_config.h"
#undef CONFIG_INCLUDE_HEADER
#define CONFIG_INCLUD_BODY

#define DEMO_ITEM_NAME_MAX_LEN 50
#define PERVIOUS_PAGE_STRING_NAME "previous page"
#define NEXT_PAGE_STRING_NAME "next page"
#define DEMO_TITLE_STRING_NAME "Demo option:"
#define ScrnWidth (240)
#define ScrnHeight (240)

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN unsigned char frame_buffer[ScrnWidth * ScrnHeight * 2];

typedef struct list_item_struct {
    show_screen_proc_f show_screen_f;
    event_handle_func event_handle_f;
    uint8_t name[DEMO_ITEM_NAME_MAX_LEN];
} list_item_struct_t;

const list_item_struct_t demo_item[] =
{
#include "screen_config.h"
};

struct {
    int32_t start_item;
    int32_t curr_item_num;
    int32_t one_screen_item_num;
    int32_t total_item_num;
    int32_t top_gap;
    int32_t left_gap;
    int32_t right_gap;
    int32_t bottom_gap;
    int32_t line_gap;
    int32_t item_height;
    int32_t item_width;
    int32_t LCD_WIDTH;
    int32_t LCD_HEIGHT;
    int32_t has_previous_page;
    int32_t has_next_page;
    gdi_color color;
    touch_event_proc_func touch_event_callback_f;
    void* user_data;
} main_screen_cntx;

static int32_t main_screen_get_index(int32_t x, int32_t y);
static void main_screen_draw(void);
static void main_screen_scroll_to_prevoius_page(void);
static void main_screen_scroll_to_next_page(void);

void nw_ui_register_touch_event_callback(touch_event_proc_func proc_func, void* user_data)
{
    GRAPHICLOG("proc_func:%x", proc_func);
    main_screen_cntx.touch_event_callback_f = proc_func;
    main_screen_cntx.user_data = user_data;
}

#ifdef MTK_CTP_ENABLE
void nw_ui_ctp_callback_func(void* param)
{
    nw_demo_send_event_from_isr(MESSAGE_ID_PEN_EVENT, 0, NULL);
}

void nw_ui_process_sigle_event(bsp_ctp_multiple_event_t* event)
{
    bsp_ctp_event_status_t hit_type;
    int32_t i = 0;
    static touch_event_struct_t pre_event = {{0,0},TOUCH_EVENT_MAX};

    GRAPHICLOG("process single event, model:%d", event->model);
    if (event->model <= 0) {
        return;
    }

    if (pre_event.type == TOUCH_EVENT_MAX || pre_event.type == TOUCH_EVENT_UP || pre_event.type == TOUCH_EVENT_ABORT) {
        // skip all up & abort point
        hit_type = CTP_PEN_DOWN;
    } else {
        // skip all down & abort event
        hit_type = CTP_PEN_UP;
    }

    while (i < event->model) {
         GRAPHICLOG("[point] event = %d, piont[0].x = %d, piont[0].y = %d\r\n",
								event->points[i].event, 	\
								event->points[i].x,		\
								event->points[i].y);
         if (event->points[i].event == hit_type) {
             break;
         }
         i++;
    }
    if (i >= event->model) {
        GRAPHICLOG("no valid point, pre event:%d", pre_event.type);
        return;
    }

    pre_event.position.x = event->points[i].x;
    pre_event.position.y = event->points[i].y;
    pre_event.type = (touch_event_enum) event->points[i].event;

    if (main_screen_cntx.touch_event_callback_f) {
        GRAPHICLOG("callback app, type:%d,[%d:%d]", pre_event.type, pre_event.position.x, pre_event.position.y);
        main_screen_cntx.touch_event_callback_f(&pre_event, main_screen_cntx.user_data);
    }
}

void nw_ui_pen_event_handle()
{
    bsp_ctp_status_t ret;
    bsp_ctp_multiple_event_t ctp_event;

    ret = bsp_ctp_get_event_data(&ctp_event);
    GRAPHICLOG("ctp_get_event_data ret:%d", ret);
    while (ret == BSP_CTP_OK) {
        ret = bsp_ctp_get_event_data(&ctp_event);
        nw_ui_process_sigle_event(&ctp_event);
    }
}
#endif

void main_screen_pen_event_handler(touch_event_struct_t* touch_event, void* user_data)
{
    static int32_t item_down_index = -1;
	int32_t temp_index;
    
    temp_index = main_screen_get_index(touch_event->position.x, touch_event->position.y);
	if (touch_event->type == TOUCH_EVENT_DOWN) {
		item_down_index = temp_index;
		return;
	} else if (touch_event->type == TOUCH_EVENT_UP) {
		if (item_down_index == -1) {
			return;
		}
	}

	if (item_down_index != temp_index) {
		item_down_index = -1;
		return;
	}

	item_down_index = -1;

    switch (temp_index) {
        case -1:
            return;
        case -2:
            main_screen_scroll_to_prevoius_page();
            break;
        case -3:
            main_screen_scroll_to_next_page();
            break;
        default:
             nw_demo_set_event_hander(demo_item[temp_index].event_handle_f);
            if (demo_item[temp_index].show_screen_f) {
                demo_item[temp_index].show_screen_f();
            }
            return;
    }
    main_screen_draw();
}

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

static void main_screen_cntx_init()
{
    static int32_t is_init;
    gdi_font_engine_string_info_t query_info;
    if (is_init)
        return;
    is_init = 1;
    
    main_screen_cntx.LCD_WIDTH = 240;
    main_screen_cntx.LCD_HEIGHT = 240;
    main_screen_cntx.top_gap = 50;
    main_screen_cntx.left_gap = 40;
    main_screen_cntx.right_gap = 3;
    main_screen_cntx.bottom_gap = 3;
    main_screen_cntx.line_gap = 10;
    main_screen_cntx.total_item_num = sizeof(demo_item)/sizeof(list_item_struct_t);
    main_screen_cntx.start_item = 0;
    main_screen_cntx.color = 0xFFFFFFFF;

    query_info.string = convert_string_to_wstring((uint8_t*) demo_item[0].name);
    query_info.count = strlen((char*) demo_item[0].name);
    gdi_font_engine_get_string_information(&query_info);
    main_screen_cntx.item_height = query_info.height;
    main_screen_cntx.item_width = query_info.width;
    if ((main_screen_cntx.LCD_WIDTH - main_screen_cntx.left_gap - main_screen_cntx.right_gap) < main_screen_cntx.item_width) {
        main_screen_cntx.item_width = main_screen_cntx.LCD_WIDTH - main_screen_cntx.left_gap - main_screen_cntx.right_gap;
    }
    main_screen_cntx.item_height += main_screen_cntx.line_gap;
    main_screen_cntx.one_screen_item_num = (main_screen_cntx.LCD_HEIGHT - main_screen_cntx.top_gap - main_screen_cntx.bottom_gap)/main_screen_cntx.item_height;
    if (main_screen_cntx.one_screen_item_num < main_screen_cntx.total_item_num) {
        main_screen_cntx.curr_item_num = main_screen_cntx.one_screen_item_num - 1;
        main_screen_cntx.has_next_page = 1;
    } else {
        main_screen_cntx.curr_item_num = main_screen_cntx.total_item_num;
        main_screen_cntx.has_next_page = 0;
    }
    main_screen_cntx.has_previous_page = 0;
    gdi_init(main_screen_cntx.LCD_WIDTH, main_screen_cntx.LCD_HEIGHT, GDI_COLOR_FORMAT_16, frame_buffer);

    main_screen_cntx.touch_event_callback_f = NULL;
    main_screen_cntx.user_data = NULL;
}

static void main_screen_scroll_to_next_page()
{
    int32_t left_item;
    if (main_screen_cntx.has_next_page) {
        left_item = main_screen_cntx.total_item_num - (main_screen_cntx.start_item + main_screen_cntx.curr_item_num);
        if (left_item > 0) {
            main_screen_cntx.has_previous_page = 1;
            if (left_item <= main_screen_cntx.one_screen_item_num - main_screen_cntx.has_previous_page) {
                main_screen_cntx.has_next_page = 0;
            } else {
                main_screen_cntx.has_next_page = 1;
            }
            main_screen_cntx.start_item += main_screen_cntx.curr_item_num;
            main_screen_cntx.curr_item_num = main_screen_cntx.one_screen_item_num 
                                            - main_screen_cntx.has_previous_page - main_screen_cntx.has_next_page;
        } else {
            //error case, logic wrong, should not be hit
        }
    } else {
        return;
    }
}

static void main_screen_scroll_to_prevoius_page()
{
    if (main_screen_cntx.has_previous_page) {
        if (main_screen_cntx.start_item == main_screen_cntx.one_screen_item_num -1) {
            main_screen_cntx.has_previous_page = 0;
        } else {
            main_screen_cntx.has_previous_page = 1;
        }
        
        main_screen_cntx.has_next_page = 1;
		main_screen_cntx.curr_item_num =  main_screen_cntx.one_screen_item_num 
                                            - main_screen_cntx.has_previous_page - main_screen_cntx.has_next_page;
        main_screen_cntx.start_item -= main_screen_cntx.curr_item_num;
    } else {
    }
}

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


static void main_screen_draw()
{
    int32_t index = main_screen_cntx.start_item;
    int32_t num = main_screen_cntx.curr_item_num;
    int32_t x,y;
    gdi_font_engine_display_string_info_t info;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};// White color.
    
    x = main_screen_cntx.left_gap;
    y = main_screen_cntx.top_gap;
    gdi_draw_filled_rectangle(0, 0, main_screen_cntx.LCD_WIDTH, main_screen_cntx.LCD_HEIGHT, (gdi_color_t)0);

    gdi_font_engine_set_font_size(GDI_FONT_ENGINE_FONT_MEDIUM);
     
    gdi_draw_filled_rectangle(10, 10, main_screen_cntx.LCD_WIDTH, main_screen_cntx.item_height, (gdi_color_t)0);

    info.x = 10;
    info.y = 10;
    info.string = convert_string_to_wstring((uint8_t *)DEMO_TITLE_STRING_NAME);
    info.length = strlen((char*)DEMO_TITLE_STRING_NAME);
    info.baseline_height = -1;
    gdi_font_engine_set_text_color(text_color);
    gdi_font_engine_display_string(&info);

    if (main_screen_cntx.has_previous_page) { 
        gdi_draw_filled_rectangle(x, y, main_screen_cntx.LCD_WIDTH, main_screen_cntx.item_height, (gdi_color_t)0);

        info.x = x;
        info.y = y;
        info.string = convert_string_to_wstring((uint8_t *)PERVIOUS_PAGE_STRING_NAME);
        info.length = strlen((char*)PERVIOUS_PAGE_STRING_NAME);
        info.baseline_height = -1;
        gdi_font_engine_set_text_color(text_color);
        gdi_font_engine_display_string(&info);
                                
        y += main_screen_cntx.item_height;
    }
    while (num) {
        uint8_t pre_index[10];
		int32_t str_len;
        
        gdi_draw_filled_rectangle(x, y, main_screen_cntx.LCD_WIDTH, main_screen_cntx.item_height, (gdi_color_t)0);
  
        info.x = x - 30;
        info.y = y;
        my_itoa((int) index, (char*) pre_index,10);
		str_len = strlen((char*) pre_index);
		pre_index[str_len] = '.';
		pre_index[str_len + 1] = 0;
        info.string = convert_string_to_wstring((uint8_t *)pre_index);
        info.length = strlen((char*)pre_index);
        info.baseline_height = -1;
        gdi_font_engine_set_text_color(text_color);
        gdi_font_engine_display_string(&info);
        
        info.x = x;
        info.y = y;
        info.string = convert_string_to_wstring((uint8_t *)demo_item[index].name);
        info.length = strlen((char*)demo_item[index].name);
        info.baseline_height = -1;
        gdi_font_engine_set_text_color(text_color);
        gdi_font_engine_display_string(&info);
                                
        y += main_screen_cntx.item_height;
        index++;
        num--;
    }
    
    if (main_screen_cntx.has_next_page) {          
        gdi_draw_filled_rectangle(x, y, main_screen_cntx.LCD_WIDTH, main_screen_cntx.item_height, (gdi_color_t)0);

        info.x = x;
        info.y = y;
        info.string = convert_string_to_wstring((uint8_t *)NEXT_PAGE_STRING_NAME);
        info.length = strlen((char*)NEXT_PAGE_STRING_NAME);
        info.baseline_height = -1;
        gdi_font_engine_set_text_color(text_color);
        gdi_font_engine_display_string(&info);
                                
        y += main_screen_cntx.item_height;        
    }
    
    gdi_lcd_update_screen(0, 0, main_screen_cntx.LCD_WIDTH, main_screen_cntx.LCD_HEIGHT);
}

// -1 means not hit, -2 means prevoius page, -3 means next page
static int32_t main_screen_get_index(int32_t x, int32_t y)
{
    int32_t ui_index = -1;
    if (x > main_screen_cntx.left_gap && x < main_screen_cntx.LCD_WIDTH - main_screen_cntx.right_gap) {
        if (y > main_screen_cntx.top_gap + 1) {
            ui_index = (y - main_screen_cntx.top_gap)/main_screen_cntx.item_height;
        }
    }

    if (ui_index >= main_screen_cntx.curr_item_num + main_screen_cntx.has_previous_page + main_screen_cntx.has_next_page) {
        return -1;
    } 

    if (ui_index == 0 && main_screen_cntx.has_previous_page) {
        return -2;
    }

    if (ui_index == main_screen_cntx.one_screen_item_num - 1 && main_screen_cntx.has_next_page) {
        return -3;
    }

    ui_index -= main_screen_cntx.has_previous_page;

    return main_screen_cntx.start_item + ui_index;
}

void show_main_screen()
{
    nw_demo_set_event_hander(nw_event_hdl);
    main_screen_cntx_init();
    nw_ui_register_touch_event_callback(main_screen_pen_event_handler, NULL);
    GRAPHICLOG("show_main_screen");
    main_screen_draw();
}


