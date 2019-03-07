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

#ifndef __NW_UI_H__
#define __NW_UI_H__

typedef uint32_t gdi_color;

/* Try again state: Currently which failure leads to try again. */
typedef enum
{
	NW_UI_TA_STATE_NONE,		
	NW_UI_TA_STATE_ACTIVATE_GPRS_F,
	NW_UI_TA_STATE_NW_SETUP_F,
	NW_UI_TA_STATE_SEND_PING_F,
	NW_UI_TA_STATE_SEND_PING_S,
	NW_UI_TA_STATE_SLEEP,
	
	NW_UI_TA_STATE_MAX	
} nw_ui_ta_state_enum;

typedef enum
{
	NW_PING_RESULT_FAILED,
	NW_PING_RESULT_SUCCESS,
	NW_PING_RESULT_MAX	
} nw_ping_result_enum;


typedef struct {
    int32_t lcd_width;
    int32_t lcd_height;
    int32_t top_gap;	// The gap between the top of the screen to the top of the first item
    int32_t bottom_gap;	// The gap between the bottom of last item to the bottom of the screen
    int32_t left_gap;
    int32_t right_gap;
    int32_t line_gap;	// The gap between two items
    int32_t item_height;
#ifdef NW_UI_SHOW_EXIT
    int32_t exit_x;
    int32_t exit_y;
#endif
    int32_t try_again_width;
    int32_t try_again_y;	
    int32_t screen_max_item;	
    gdi_color bg_color;		
    gdi_color txt_color;	
    int32_t curr_action;	/* 1: activation; 2: deactivation; 3: set nw info */
    nw_ui_ta_state_enum try_again_state;
    uint32_t ping_addr_index;
    uint32_t ping_addr_number;
    nw_ping_result_enum ping_result;
    uint32_t ping_total_num;
    uint32_t ping_lost_num;
    uint32_t ping_recv_num;
} nw_ui_cntx_struct;

extern void nw_show_screen(void);
extern void nw_event_hdl(message_id_enum event_id, int32_t param1, void *param2);
extern void nw_ui_reset_action(void);

extern void fota_event_hdl(message_id_enum event_id, int32_t param1, void *param2);
extern void gnss_show_screen(void);
extern void gnss_nw_event_hdl(message_id_enum event_id, int32_t param1, void *param2);

#endif /* __NW_UI_H__ */
