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

#include "lcd_sw.h"
#include "mt25x3_hdk_lcd.h"
#include "hal_gpt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HEIGHT 128
#define WIDTH 64

extern uint8_t lcd_get_page_count(uint8_t char_height, uint8_t y0);

uint32_t ranesis_delay;

#define LCD_CtrlWrite_SH1107(_data) \
{\
	*((volatile uint16_t *)MAIN_LCD_CMD_ADDR) = ((_data)&(0x00FF));\
for(ranesis_delay = 20; ranesis_delay > 0; ranesis_delay--);\
}

#define LCD_DataWrite_SH1107(_data) \
{\
	*((volatile uint16_t *)MAIN_LCD_DATA_ADDR) = ((_data)&(0x00FF));\
for(ranesis_delay = 20; ranesis_delay > 0; ranesis_delay--);\
}

void Set_Column(uint8_t column)
{
    LCD_CtrlWrite_SH1107((0x00)|((column)&(0x0F)));
    LCD_CtrlWrite_SH1107((0x10)|(column>>4));
}


void Set_Start_Column(void)
{
    LCD_CtrlWrite_SH1107(0x00);
    LCD_CtrlWrite_SH1107(0x10);
}

void Set_Start_Page(unsigned char d)
{
    LCD_CtrlWrite_SH1107((0xB0) | (d));			// Set Page Start Address for Page Addressing Mode
}

void Set_Display_On_Off(unsigned char d)
{
    LCD_CtrlWrite_SH1107((0xAE) | (d));			// Set Display On/Off
}

void Set_Display_Offset(unsigned char d)
{
    LCD_CtrlWrite_SH1107(0xD3);			// Set Display Offset
    LCD_CtrlWrite_SH1107(d);			//   Default => 0x00
}

void LCD_Init_Interface_SH1107(void)
{
    hal_display_lcd_interface_mode_t mode_settings;
    hal_display_lcd_interface_timing_t timing_settings;

	mode_settings.port_number = HAL_DISPLAY_LCD_INTERFACE_SERIAL_0;
	mode_settings.three_wire_mode = 0;
	mode_settings.cs_stay_low_mode = 0;
	mode_settings.driving_current = HAL_DISPLAY_LCD_DRIVING_CURRENT_16MA;
	mode_settings.single_a0_mode = 0;
	mode_settings.read_from_SDI = 0;
	mode_settings.width = HAL_DISPLAY_LCD_INTERFACE_WIDTH_8;
	mode_settings.hw_cs = 1;
	mode_settings.power_domain = HAL_DISPLAY_LCD_POWER_DOMAIN_1V8;
	mode_settings.start_byte_mode = 0;

    hal_display_lcd_set_interface_mode(mode_settings);

	timing_settings.port_number = HAL_DISPLAY_LCD_INTERFACE_SERIAL_0;
	timing_settings.csh = 0;
	timing_settings.css = 0;
	timing_settings.wr_low = 0xF;
	timing_settings.wr_high = 0xF;
	timing_settings.rd_low = 0xF;
	timing_settings.rd_high = 0xF;
	timing_settings.clock_freq = HAL_DISPLAY_LCD_INTERFACE_CLOCK_124MHZ;
    
    hal_display_lcd_set_interface_timing(timing_settings);
		// Do error check
} /* init_lcd_interface() */

/***********************SH1107********************************/

void LCD_blockClear_SH1107(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t data)
{
	unsigned char x, y, send_data;

	if(data == 0)
		send_data = 0;
	else
		send_data = 0xFF;
	
	for(x = 0; x < 16; x++)
	{
		Set_Start_Page(x);
		Set_Start_Column();
		for(y = 0; y < 128; y++)
		{
			LCD_DataWrite_SH1107(send_data);
		}
	}
	hal_gpt_delay_ms(100);
}

void LCD_ClearAll_SH1107(uint16_t data)
{
	LCD_blockClear_SH1107(0,0,HEIGHT-1,WIDTH-1,data);
}

uint32_t device_code;

void LCD_Init_SH1107(uint16_t bkground)
{
	hal_display_lcd_toggle_reset(10, 120);

    LCD_CtrlWrite_SH1107(0xAE);
    LCD_CtrlWrite_SH1107(0xA8);
    LCD_CtrlWrite_SH1107(0x3F);	
    LCD_CtrlWrite_SH1107(0xD5);
    LCD_CtrlWrite_SH1107(0x51);
	LCD_CtrlWrite_SH1107(0xc0); // Set Common scan direction
	LCD_CtrlWrite_SH1107(0xd3); // Set Display Offset
	LCD_CtrlWrite_SH1107(0x60);
	LCD_CtrlWrite_SH1107(0xdc); // Set Display Start Line
	LCD_CtrlWrite_SH1107(0x00);
	LCD_CtrlWrite_SH1107(0x20); // Set Page Addressing Mode
	LCD_CtrlWrite_SH1107(0x81); // Set Contrast Control
	LCD_CtrlWrite_SH1107(0x90);
	LCD_CtrlWrite_SH1107(0xa0); // Set Segment Re-map
	LCD_CtrlWrite_SH1107(0xa4); // Set Entire Display OFF/ON
	LCD_CtrlWrite_SH1107(0xa6); // Set Normal/Reverse Display
	LCD_CtrlWrite_SH1107(0xad); // Set External VPP
	LCD_CtrlWrite_SH1107(0x8a);
	LCD_CtrlWrite_SH1107(0xd9); // Set Phase Leghth
	LCD_CtrlWrite_SH1107(0x22);
	LCD_CtrlWrite_SH1107(0xdb); // Set Vcomh voltage
	LCD_CtrlWrite_SH1107(0x35);
	LCD_ClearAll_SH1107(0);
	LCD_CtrlWrite_SH1107(0xaf); //Display ON

}

void LCD_EnterSleep_SH1107(void)
{
	Set_Display_On_Off(0x00);
}

void LCD_ExitSleep_SH1107(void)
{
	LCD_Init_SH1107(0);
}
 

void LCD_EnterIdle_SH1107(void)
{
	LCD_CtrlWrite_SH1107(0x39);
}

void LCD_ExitIdle_SH1107(void)
{
	LCD_CtrlWrite_SH1107(0x38);
}

//static int b =0;
extern uint8_t lcd_GetPageCount(uint8_t charHeight, uint8_t y0_Coordinate);  
void LCD_BlockWrite_SH1107(uint16_t startx,uint16_t starty,uint16_t endx,uint16_t endy)
{
	uint8_t *buffer_addr;


	uint16_t        i, j,k;
	uint16_t        start_page_num, page_num;
	uint16_t        height;
	uint8_t*        p_frame;

	buffer_addr = (uint8_t *)hal_display_lcd_get_layer_address(HAL_DISPLAY_LCD_LAYER0);

	height = endy - starty + 1;
	page_num = lcd_get_page_count((uint8_t) height, (uint8_t)starty);
	start_page_num = starty >> 3;
	p_frame = buffer_addr + start_page_num * WIDTH;
	startx = WIDTH-startx-1;
	endx = WIDTH-endx-1;
	for(j = start_page_num; j < start_page_num+page_num; j++) {
		Set_Start_Page(j);
		Set_Column(endx);
		k = WIDTH-endx-1;
		for(i=endx;i<=startx;i++)
		{
			LCD_DataWrite_SH1107(*(p_frame+k));
			k--;
		}
		p_frame += WIDTH;
	}	


	hal_gpt_delay_ms(1);
}


/***************************** Take Notice *******************************
*************************************************************************/ 
 
void LCD_IOCTRL_SH1107(LCM_IOCTRL_ID_ENUM ID, void* Parameters)
{
	switch (ID)
	{
		case LCM_IOCTRL_QUERY__FRAME_RATE:
			*((uint32_t*)(Parameters)) = 60;
			break;

		case LCM_IOCTRL_QUERY__BACK_PORCH:
			*(uint32_t *)(Parameters) = 0;
			break;
		    
		case LCM_IOCTRL_QUERY__FRONT_PORCH:
			*(uint32_t *)(Parameters) = 0;
			break;

		case LCM_IOCTRL_QUERY__IF_CS_NUMBER:
			*(uint32_t *)(Parameters) = HAL_DISPLAY_LCD_INTERFACE_SERIAL_0;
			break;

		case LCM_IOCTRL_QUERY__LCM_WIDTH: 
			*((unsigned long*)(Parameters)) = 128;// physical, not use app's LCD_WIDTH
			break;

		case LCM_IOCTRL_QUERY__LCM_HEIGHT: 
			*((unsigned long*)(Parameters)) = 64;//320;// physical, not use app's LCD_HEIGHT
			break;

#if 0
		//#ifdef __LCD_ESD_RECOVERY__
		/** LCM ESD feature control, Check LCD ESD is happend or not **/
		case LCM_IOCTL_ESD_CHECK:
		{
		LCM_ESD_INFO_STRUCT* pESDInfo = (LCM_ESD_INFO_STRUCT *)Parameters;
		kal_bool lcm_esd_test = KAL_FALSE;

		lcm_esd_test = LCD_Read_ESD_SH1107();

		if(pESDInfo)
		{
		 //TODO: add ESD real check function here
		 // If happend, must take lcm_esd_test = KAL_TURE;
		 if(lcm_esd_test)
		 {
		  lcm_esd_test = KAL_FALSE;
		  pESDInfo->bEsdHappened = KAL_TRUE;
		 }
		 else
		  pESDInfo->bEsdHappened = KAL_FALSE;
		}

		return LCM_IOCTRL_OK;
		break;
		}
			   
		/** LCM ESD feature control, If LCD ESD is happend, We should do something here like resetLCM. **/
		case LCM_IOCTL_ESD_RESET:
		//TODO: add LCM reset function here
		//Example
		LCD_Init_SH1107(0,0);

		break;  
#endif

		default:
		    break;
	}
}

void LCD_ClearScreen_SH1107(uint16_t color)
{
	LCD_ClearAll_SH1107(color);
}

void LCD_ClearScreenBW_SH1107(void)
{
	unsigned char x, y, send_data;

	for(x = 0; x < 16; x++)
	{
		if(x < 8)
			send_data = 0xFF;
		else
			send_data = 0;
		Set_Start_Page(x);
		Set_Start_Column();
		for(y = 0; y < 128; y++)
		{
			LCD_DataWrite_SH1107(send_data);
		}
	}
	hal_gpt_delay_ms(100);
}

bool LCD_CheckID_SH1107(void)
{
    return true;
}

LCD_Funcs LCD_func_SH1107 = {
	LCD_Init_SH1107,
	LCD_Init_Interface_SH1107,
	LCD_BlockWrite_SH1107,
	LCD_EnterSleep_SH1107,
	LCD_ExitSleep_SH1107,
	LCD_EnterIdle_SH1107,
	LCD_ExitIdle_SH1107,
	LCD_ClearScreen_SH1107,
	LCD_ClearScreenBW_SH1107,
	LCD_IOCTRL_SH1107,
    LCD_CheckID_SH1107
};

#ifdef __cplusplus
}
#endif

