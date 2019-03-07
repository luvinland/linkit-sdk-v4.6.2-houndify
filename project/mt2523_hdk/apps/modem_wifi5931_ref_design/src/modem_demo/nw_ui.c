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
#include "FreeRTOS.h"
#include "gprs_api.h"
#include "ping.h"
#include "main_screen.h"
#include "nw_ui.h"
#include "nw_gprs.h"
#include "nvdm.h"
#include "syslog.h"
#include "sio_gprot.h"
#include "string.h"
#include "stdio.h"
#include "task.h"
#include "fota_demo.h"
#include "task_def.h"
#include "gdi.h"
#include "gdi_font_engine.h"

extern char *gprs_get_local_ip(void);


#ifdef NW_UI_PRINTF
#define LOGE(fmt,arg...)   printf(("[NW]: "fmt), ##arg)
#define LOGW(fmt,arg...)   printf(("[NW]: "fmt), ##arg)
#define LOGI(fmt,arg...)   printf(("[NW]: "fmt), ##arg)
#else
log_create_module(nw_ui, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   //LOG_E(nw_ui, "[NW]: "fmt,##arg)
#define LOGW(fmt,arg...)   //LOG_W(nw_ui, "[NW]: "fmt,##arg)
#define LOGI(fmt,arg...)   //LOG_I(nw_ui ,"[NW]: "fmt,##arg)
#endif

#define NW_UI_PING_COUNT				(4)		// The number of ping requests sent
#define NW_UI_PING_TIMEOUT				(10)	// in second
#define NW_UI_PING_SIZE					(32)
#define NW_UI_PING_REMOTE_IP_DEFAULT	("180.97.33.108")		// www.baidu.com

#define NW_UI_MAX_IP_ADDR_LEN			(15)

#define NW_UI_STR_APP_NAME					("Network Demo")
#ifdef NW_UI_SHOW_EXIT
#define NW_UI_STR_EXIT						("Exit")
#endif
#define NW_UI_STR_TRY_AGAIN					("Try again")

#define NW_UI_STR_MODEM_IS_NOT_READY		("Modem is not ready")				// item index: 0
#define NW_UI_STR_MODEM_IS_NOT_READY_IDX	(0)
#define NW_UI_STR_MODEM_IS_READY			("Modem is ready")					// item index: 0
#define NW_UI_STR_MODEM_IS_READY_IDX		(0)
#define NW_UI_STR_PING_SUCCESS			    ("Ping success")					// item index: 0
#define NW_UI_STR_PING_SUCCESS_IDX	     	(0)
#define NW_UI_STR_PING_FAILED			    ("Ping failed")					// item index: 0
#define NW_UI_STR_PING_FAILED_IDX	     	(0)
#define NW_UI_STR_MODEM_IS_ASLEEP			("Modem is now asleep")	        	// item index: 1
#define NW_UI_STR_MODEM_IS_ASLEEP_IDX		(1)
#define NW_UI_STR_ACTIVATE_GPRS				("Activate GPRS")					// item index: 1
#define NW_UI_STR_ACTIVATE_GPRS_IDX			(1)
#define NW_UI_STR_ACTIVATE_GPRS_S			("GPRS is activated")				// item index: 1
#define NW_UI_STR_ACTIVATE_GPRS_S_IDX		(1)
#define NW_UI_STR_ACTIVATE_GPRS_F			("Failed to activate GPRS")			// item index: 1
#define NW_UI_STR_ACTIVATE_GPRS_F_IDX		(1)
#define NW_UI_STR_DEACTIVATE_GPRS_IND		("GPRS is deactivated")				// item index: 1
#define NW_UI_STR_DEACTIVATE_GPRS_IND_IDX	(1)
#define NW_UI_STR_SEARCHING_NETWORK_IND		("Modem is searching NW")		    // item index: 1
#define NW_UI_STR_SEARCHING_NETWORK_IND_IDX	(1)
#define NW_UI_STR_NW_SETUP_F				("Failed to setup IP stack")		// item index: 2
#define NW_UI_STR_NW_SETUP_F_IDX			(2)
#define NW_UI_STR_LOCAL_IP					("Local IP: %s")					// item index: 2
#define NW_UI_STR_LOCAL_IP_IDX				(2)
#define NW_UI_STR_SEND_PING					("Send ping request")				// item index: 3
#define NW_UI_STR_SEND_PING_IDX				(3)
#define NW_UI_STR_REMOTE_IP					("Remote IP: %s")					// item index: 4
#define NW_UI_STR_REMOTE_IP_IDX				(4)
#define NW_UI_STR_WAIT_PING_RSP				("Wait for ping response")			// item index: 5
#define NW_UI_STR_WAIT_PING_RSP_IDX			(5)
#define NW_UI_STR_SEND_PING_S				("Receive ping response")			// item index: 5
#define NW_UI_STR_SEND_PING_S_IDX			(5)
#define NW_UI_STR_SEND_PING_F				("Failed to Receive ping response")	// item index: 5
#define NW_UI_STR_SEND_PING_F_IDX			(5)
#define NW_UI_STR_RCV_PING_RESULT			("Total/Recv/Lost: %d/%d/%d")
#define NW_UI_STR_RCV_PING_RESULT_IDX		(6)

char *addr_list[] = {
    "180.97.33.108",   // www.baidu.com
    "172.21.100.230",  //www.mediatek.com
    "221.236.31.210",  //www.sina.com.cn
    "74.125.204.94",   // www.google.com.tw
    "116.214.12.74",   //www.yahoo.com.tw
    "103.235.46.39",   //www.a.shifen.com
    NULL
};

nw_ui_cntx_struct nw_ui_cntx;

static void nw_ui_clear_item(int item_idx);
static void nw_ui_clear_try_again(void);
static void nw_ui_draw_title();
static void nw_ui_draw_item(int item_idx, char *str);
static void nw_ui_draw_try_again(void);

void nw_ping_cb(ping_result_t *result)
{
    if (result) {
        if (result->recv_num) {
            nw_ui_cntx.ping_total_num = result->total_num;
            nw_ui_cntx.ping_recv_num = result->recv_num;
            nw_ui_cntx.ping_lost_num = result->lost_num;
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_PING_RECEIVE_S, NULL);
        } else {
            nw_ui_cntx.ping_addr_index++;
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_PING_RECEIVE_F, NULL);
        }
        LOGI("nw_ping_cb() recv_num:%d\r\n", result->recv_num);
    } else {
        LOGI("nw_ping_cb() result null");
    }
    
#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    vTaskDelay(3000);
    gprs_set_host_sleep();
    sio_trigger_modem_sleep();
#endif
}

char *nw_ui_get_remote_ip(char *buff, int buff_size)
{
    if (NULL == buff || NW_UI_MAX_IP_ADDR_LEN > buff_size) {
        return NULL;
    }

    memset(buff, 0, buff_size);
    strcpy(buff, addr_list[nw_ui_cntx.ping_addr_index % nw_ui_cntx.ping_addr_number]);

    LOGI("Remote IP is %s\r\n", buff);

    return buff;
}

static uint8_t *nw_ui_str2wstr(uint8_t *string)
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
        index += 2;
    }
    return wstring;
}

void nw_ui_gprs_activate(void)
{
    gprs_ret_t ret = GPRS_RET_ERROR;

    if (GPRS_RET_OK != gprs_is_data_activated() && !nw_ui_cntx.curr_action) {
        gdi_draw_filled_rectangle(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height, nw_ui_cntx.bg_color);
        nw_ui_draw_title();
        nw_ui_draw_item(NW_UI_STR_MODEM_IS_READY_IDX, NW_UI_STR_MODEM_IS_READY);
        nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_IDX, NW_UI_STR_ACTIVATE_GPRS);
        nw_ui_cntx.curr_action = 1;
        /* Flush the screen immediately */
        gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
        if (GPRS_RET_OK != (ret = gprs_activate_with_cid(GPRS_PDP_CONTEXT_ID_1, nw_ui_reset_action)) &&
                GPRS_RET_WOULDBLOCK != ret) {
            nw_ui_cntx.curr_action = 0;
            nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_F_IDX, NW_UI_STR_ACTIVATE_GPRS_F);

            nw_ui_draw_try_again();
            nw_ui_cntx.try_again_state = NW_UI_TA_STATE_ACTIVATE_GPRS_F;
        } else {
            nw_ui_clear_try_again();
            nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NONE;
        }
    }
}


static void nw_ui_set_nw_info(void)
{
    gprs_ret_t ret = GPRS_RET_ERROR;
    LOGI("set nw info: action %d", nw_ui_cntx.curr_action);
    if (!nw_ui_cntx.curr_action) {
        nw_ui_cntx.curr_action = 3;
        if (GPRS_RET_OK != (ret = gprs_set_nw_info_with_cid(GPRS_PDP_CONTEXT_ID_1, nw_ui_reset_action)) &&
                GPRS_RET_WOULDBLOCK != ret) {
            nw_ui_cntx.curr_action = 0;
            nw_ui_draw_item(NW_UI_STR_NW_SETUP_F_IDX, NW_UI_STR_NW_SETUP_F);
            nw_ui_draw_try_again();
            nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NW_SETUP_F;
        } else {
            nw_ui_clear_try_again();
            nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NONE;
        }
    }
}


static void nw_ui_send_ping_req(void)
{
#if PING_NEW_VERSION
    ping_request_t ping_req = {0};
#endif
    char remote_ip[NW_UI_MAX_IP_ADDR_LEN + 1] = {0};

    sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);

    if (NULL == nw_ui_get_remote_ip(remote_ip, NW_UI_MAX_IP_ADDR_LEN)) {
        LOGE("Read remote ip from NVRAM fail.\r\n");
        return;
    }

#if PING_NEW_VERSION
    ping_req.addr = NW_UI_PING_REMOTE_IP_DEFAULT;
    ping_req.addr_type = PING_IP_ADDR_V4;
    ping_req.count = NW_UI_PING_COUNT;
    ping_req.ping_size = NW_UI_PING_SIZE;
    ping_req.timeout = NW_UI_PING_TIMEOUT;
    ping_req.callback = nw_ping_cb;
    ping_request(&ping_req);
#else
    LOGE("ping_request start, addr = %s.\r\n", remote_ip);
    ping_request(NW_UI_PING_COUNT,
                 remote_ip,
                 PING_IP_ADDR_V4,
                 NW_UI_PING_SIZE,
                 nw_ping_cb);
#endif
}


void nw_ui_pen_evt_hdl(touch_event_struct_t *pen_event, void *user_data)
{
#ifdef NW_UI_SHOW_EXIT
    gprs_ret_t ret = GPRS_RET_ERROR;
#endif

    if (pen_event && TOUCH_EVENT_UP == pen_event->type) {
#ifdef NW_UI_SHOW_EXIT
        /* Check if "Exit" is touched. */
        if ((pen_event->position.x >= nw_ui_cntx.exit_x && pen_event->position.x < nw_ui_cntx.lcd_width) &&
                (pen_event->position.y >= nw_ui_cntx.exit_y && pen_event->position.x < nw_ui_cntx.lcd_height)) {
            show_main_screen();

            if (GPRS_RET_OK == gprs_is_data_activated()) {
                gprs_ret_t ret = GPRS_RET_ERROR;
                if (!nw_ui_cntx.curr_action) {
                    nw_ui_cntx.curr_action = 3;
                    if (GPRS_RET_OK != (ret = gprs_deactivate_with_cid(GPRS_PDP_CONTEXT_ID_1)) &&
                            GPRS_RET_WOULDBLOCK != ret) {
                        nw_ui_cntx.curr_action = 0;
                    }
                }
            }
        } else
#endif
            /* Check if "Try again" is touched. */
            if ((pen_event->position.x >= 0 && pen_event->position.x < nw_ui_cntx.try_again_width) &&
                    (pen_event->position.y >= nw_ui_cntx.try_again_y && pen_event->position.x < nw_ui_cntx.lcd_height)) {
                switch (nw_ui_cntx.try_again_state) {
                    case NW_UI_TA_STATE_ACTIVATE_GPRS_F: {
                        nw_ui_gprs_activate();
                        break;
                    }

                    case NW_UI_TA_STATE_NW_SETUP_F: {
                        nw_ui_set_nw_info();
                        break;
                    }

                    case NW_UI_TA_STATE_SEND_PING_F: {
                        nw_ui_clear_item(NW_UI_STR_SEND_PING_F_IDX);
                        nw_ui_clear_try_again();
                        nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NONE;
                        nw_ui_send_ping_req();
                        break;
                    }

                    case NW_UI_TA_STATE_SEND_PING_S: {
                        nw_ui_clear_item(NW_UI_STR_SEND_PING_S_IDX);
                        nw_ui_clear_item(NW_UI_STR_RCV_PING_RESULT_IDX);
                        nw_ui_clear_try_again();
                        nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NONE;

                        nw_ui_send_ping_req();

                        nw_ui_draw_item(NW_UI_STR_WAIT_PING_RSP_IDX, NW_UI_STR_WAIT_PING_RSP);

                        break;
                    }

                    case NW_UI_TA_STATE_SLEEP: {
                        nw_ui_gprs_activate();
                        break;
                    }

                    default: {
                        break;
                    }
                }

                gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
            }
    }
}


static void nw_ui_init(void)
{
    static int32_t is_init;
    gdi_font_engine_string_info_t string_info;

    if (is_init) {
        nw_ui_cntx.curr_action = 0;
        nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NONE;
        return;
    }

    is_init = 1;

    memset(&nw_ui_cntx, 0, sizeof(nw_ui_cntx));

    nw_ui_cntx.lcd_width = 240;
    nw_ui_cntx.lcd_height = 240;
    nw_ui_cntx.left_gap = 10;
    nw_ui_cntx.right_gap = 3;
    nw_ui_cntx.line_gap = 2;
    nw_ui_cntx.bg_color = 0;
    nw_ui_cntx.txt_color = 0xFFFFFFFF;

    string_info.string = nw_ui_str2wstr((uint8_t *)NW_UI_STR_TRY_AGAIN);
    string_info.count = strlen(NW_UI_STR_TRY_AGAIN); 
    gdi_font_engine_get_string_information(&string_info);
    nw_ui_cntx.try_again_width = string_info.width;
    nw_ui_cntx.item_height = string_info.height;
    nw_ui_cntx.top_gap = nw_ui_cntx.item_height + 4 * nw_ui_cntx.line_gap + 10;
    nw_ui_cntx.bottom_gap = nw_ui_cntx.item_height + 3 * nw_ui_cntx.line_gap;
    nw_ui_cntx.try_again_width += nw_ui_cntx.left_gap;
    nw_ui_cntx.try_again_y = nw_ui_cntx.lcd_height - nw_ui_cntx.item_height - nw_ui_cntx.line_gap - 1;
    nw_ui_cntx.item_height += nw_ui_cntx.line_gap;

#ifdef NW_UI_SHOW_EXIT
    string_info.string = nw_ui_str2wstr((uint8_t *)NW_UI_STR_EXIT);
    string_info.count = strlen(NW_UI_STR_EXIT);
    gdi_font_engine_get_string_information(&string_info);
    nw_ui_cntx.exit_x = string_info.width;
    nw_ui_cntx.exit_y = string_info.height; 
    nw_ui_cntx.exit_x = nw_ui_cntx.lcd_width - nw_ui_cntx.right_gap - nw_ui_cntx.exit_x - 1;
    nw_ui_cntx.exit_y = nw_ui_cntx.try_again_y;
#endif

    nw_ui_cntx.screen_max_item = (nw_ui_cntx.lcd_height - nw_ui_cntx.top_gap -
                                  nw_ui_cntx.bottom_gap) / nw_ui_cntx.item_height;


    LOGI("try_again_width:%d, item_height:%d, bottom_gap:%d, try_again_y:%d, screen_max_item:%d \r\n",
         nw_ui_cntx.try_again_width, nw_ui_cntx.item_height, nw_ui_cntx.bottom_gap, nw_ui_cntx.try_again_y,
         nw_ui_cntx.screen_max_item);
    nw_ui_cntx.ping_addr_index = 0;
    nw_ui_cntx.ping_addr_number = 0;
    while(addr_list[nw_ui_cntx.ping_addr_number] != NULL) {
        nw_ui_cntx.ping_addr_number++;
    };
    LOGI("ping_max_number = %d\n", nw_ui_cntx.ping_addr_number);
}

static void nw_ui_draw_title()
{
    gdi_font_engine_display_string_info_t info;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};// White color.

    LOGI("nw_ui_draw_title(), x:%d, y:%d\r\n", nw_ui_cntx.left_gap, nw_ui_cntx.line_gap);

    gdi_font_engine_set_text_color(text_color);
    
    info.x = nw_ui_cntx.left_gap;
    info.y = nw_ui_cntx.line_gap;
    info.string = nw_ui_str2wstr((uint8_t *)NW_UI_STR_APP_NAME);
    info.length = strlen(NW_UI_STR_APP_NAME);
    info.baseline_height = -1;
    gdi_font_engine_display_string(&info);
}

static void nw_ui_draw_try_again(void)
{
    gdi_font_engine_display_string_info_t info;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};// White color.
    LOGI("nw_ui_draw_try_again(), x:%d, y:%d\r\n", nw_ui_cntx.left_gap, nw_ui_cntx.try_again_y);

    gdi_font_engine_set_text_color(text_color);

    info.x = nw_ui_cntx.left_gap;
    info.y = nw_ui_cntx.try_again_y;
    info.string = nw_ui_str2wstr((uint8_t *)NW_UI_STR_TRY_AGAIN);
    info.length = strlen(NW_UI_STR_TRY_AGAIN);
    info.baseline_height = -1;
    gdi_font_engine_display_string(&info);
}


static void nw_ui_clear_try_again(void)
{
    LOGI("nw_ui_clear_try_again()\r\n");

    gdi_draw_filled_rectangle(0,
                        nw_ui_cntx.try_again_y,
                        nw_ui_cntx.try_again_width - 1,
                        nw_ui_cntx.lcd_height - 1,
                        nw_ui_cntx.bg_color);
}


/* Remember 4to call gdi_flush_screen() latter. */
static void nw_ui_draw_item(int item_idx, char *str)
{
    gdi_font_engine_display_string_info_t info;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};// White color.
    int x, y;

    LOGI("nw_ui_draw_item() item_idx=%d, str_empty=%d\r\n", item_idx, str ? 0 : 1);

    if (!str || 0 > item_idx || nw_ui_cntx.screen_max_item <= item_idx) {
        return;
    }

    x = nw_ui_cntx.left_gap;
    y = nw_ui_cntx.top_gap + item_idx * nw_ui_cntx.item_height;

    LOGI("x:%d, y:%d\r\n", x, y);

    gdi_draw_filled_rectangle(0, y, nw_ui_cntx.lcd_width, y + nw_ui_cntx.item_height, nw_ui_cntx.bg_color);

    gdi_font_engine_set_text_color(text_color);

    info.x = x;
    info.y = y;
    info.string = nw_ui_str2wstr((uint8_t *)str);
    info.length = strlen(str);
    info.baseline_height = -1;
    gdi_font_engine_display_string(&info);
}


static void nw_ui_clear_item(int item_idx)
{
    int y;

    if (nw_ui_cntx.screen_max_item <= item_idx) {
        return;
    }

    y = nw_ui_cntx.top_gap + item_idx * nw_ui_cntx.item_height;

    gdi_draw_filled_rectangle(0, y, nw_ui_cntx.lcd_width, y + nw_ui_cntx.item_height, nw_ui_cntx.bg_color);
}

static void nw_ui_draw_main_screen(void)
{
    gdi_font_engine_display_string_info_t info;
    gdi_font_engine_color_t text_color = {0, 255, 255, 255};// White color.
    
    LOGI("nw_ui_draw_main_screen()\r\n");

    gdi_draw_filled_rectangle(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height, nw_ui_cntx.bg_color);

    gdi_font_engine_set_text_color(text_color);

    info.x = nw_ui_cntx.left_gap;
    info.y = nw_ui_cntx.line_gap;
    info.string = nw_ui_str2wstr((uint8_t *)NW_UI_STR_APP_NAME);
    info.length = strlen(NW_UI_STR_APP_NAME);
    info.baseline_height = -1;
    gdi_font_engine_display_string(&info);

#ifdef NW_UI_SHOW_EXIT
    info.x = nw_ui_cntx.left_gap;
    info.y = nw_ui_cntx.line_gap;
    info.string = nw_ui_str2wstr((uint8_t *)NW_UI_STR_EXIT);
    info.length = strlen(NW_UI_STR_EXIT);
    info.baseline_height = -1;
    gdi_font_engine_set_text_color(text_color);
    gdi_font_engine_display_string(&info);
#endif

    if (GPRS_RET_OK == gprs_is_modem_ready()) {
        nw_ui_draw_item(NW_UI_STR_MODEM_IS_READY_IDX, NW_UI_STR_MODEM_IS_READY);
        if (GPRS_RET_OK == gprs_is_data_activated()) {
            nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_S_IDX, NW_UI_STR_ACTIVATE_GPRS_S);

            nw_ui_set_nw_info();
        } else {
            nw_ui_gprs_activate();
        }
    } else {
        nw_ui_draw_item(NW_UI_STR_MODEM_IS_NOT_READY_IDX, NW_UI_STR_MODEM_IS_NOT_READY);
    }

    gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
}


void nw_sleep_cb(gprs_ret_t res, void *user_data)
{
    LOGI("nw_sleep_cb(), res:%d", res);

    if (GPRS_RET_OK == res) {
        gdi_draw_filled_rectangle(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height, nw_ui_cntx.bg_color);
        nw_ui_draw_title();
        if (nw_ui_cntx.ping_result == NW_PING_RESULT_SUCCESS) {
            nw_ui_draw_item(NW_UI_STR_PING_SUCCESS_IDX, NW_UI_STR_PING_SUCCESS);
        } else {
            nw_ui_draw_item(NW_UI_STR_PING_FAILED_IDX, NW_UI_STR_PING_FAILED);
        }
        nw_ui_draw_item(NW_UI_STR_MODEM_IS_ASLEEP_IDX, NW_UI_STR_MODEM_IS_ASLEEP);
        nw_ui_draw_try_again();
        nw_ui_cntx.try_again_state = NW_UI_TA_STATE_SLEEP;
        gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
    } else {
        LOGE("Failed to set modem sleep. ");
    }

    nw_ui_reset_action();
}


void nw_event_hdl(message_id_enum event_id, int32_t param1, void *param2)
{
    LOGI("nw_event_hdl(), event_id=%d, param1=%d\r\n", event_id, param1);

    if (MESSAGE_ID_NW_FLOW == event_id) {
        switch (param1) {
            case NW_FLOW_EVT_TYPE_MODEM_READY_IND: {
                if (GPRS_RET_OK == gprs_is_data_activated()) {
                    LOGW("Recive MODEM_READY_IND when gprs is activated.\r\n");
                    break;
                }

                nw_ui_draw_item(NW_UI_STR_MODEM_IS_READY_IDX, NW_UI_STR_MODEM_IS_READY);

                nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_IDX, NW_UI_STR_ACTIVATE_GPRS);

                nw_ui_gprs_activate();

                break;
            }

            case NW_FLOW_EVT_TYPE_SEARCHING_NW_IND: {
                nw_ui_draw_item(NW_UI_STR_MODEM_IS_READY_IDX, NW_UI_STR_MODEM_IS_READY);

                nw_ui_draw_item(NW_UI_STR_SEARCHING_NETWORK_IND_IDX, NW_UI_STR_SEARCHING_NETWORK_IND);

                break;
            }

            case NW_FLOW_EVT_TYPE_CMUX_CONN_IND: {
                if (GPRS_RET_OK == gprs_is_data_activated()) {
                    LOGW("Recive MODEM_READY_IND when gprs is activated.\r\n");
                    break;
                }

                nw_ui_draw_item(NW_UI_STR_MODEM_IS_READY_IDX, NW_UI_STR_MODEM_IS_READY);

                nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_IDX, NW_UI_STR_ACTIVATE_GPRS);

                //nw_ui_gprs_activate();

                break;
            }

            case NW_FLOW_EVT_TYPE_MODEM_EXCEPTION_IND: {
                gdi_draw_filled_rectangle(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height, nw_ui_cntx.bg_color);
                nw_ui_draw_item(NW_UI_STR_MODEM_IS_NOT_READY_IDX, NW_UI_STR_MODEM_IS_NOT_READY);
                break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_ACTIVATE_S: {
                nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_S_IDX, NW_UI_STR_ACTIVATE_GPRS_S);

                nw_ui_set_nw_info();

                break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_ACTIVATE_F: {
                nw_ui_draw_item(NW_UI_STR_ACTIVATE_GPRS_F_IDX, NW_UI_STR_ACTIVATE_GPRS_F);
                nw_ui_draw_try_again();
                nw_ui_cntx.try_again_state = NW_UI_TA_STATE_ACTIVATE_GPRS_F;
                break;
            }

            case NW_FLOW_EVT_TYPE_SET_NW_INFO_S: {
                char str[30] = {0};
                char remote_ip[NW_UI_MAX_IP_ADDR_LEN + 1] = {0};

                sprintf(str, NW_UI_STR_LOCAL_IP, gprs_get_local_ip());
                nw_ui_draw_item(NW_UI_STR_LOCAL_IP_IDX, str);

                nw_ui_draw_item(NW_UI_STR_SEND_PING_IDX, NW_UI_STR_SEND_PING);

                memset(str, 0, 30);
                sprintf(str, NW_UI_STR_REMOTE_IP, nw_ui_get_remote_ip(remote_ip, NW_UI_MAX_IP_ADDR_LEN));
                nw_ui_draw_item(NW_UI_STR_REMOTE_IP_IDX, str);

                nw_ui_send_ping_req();

                nw_ui_draw_item(NW_UI_STR_WAIT_PING_RSP_IDX, NW_UI_STR_WAIT_PING_RSP);

                break;
            }

            case NW_FLOW_EVT_TYPE_SET_NW_INFO_F: {
                nw_ui_draw_item(NW_UI_STR_NW_SETUP_F_IDX, NW_UI_STR_NW_SETUP_F);

                nw_ui_draw_try_again();
                nw_ui_cntx.try_again_state = NW_UI_TA_STATE_NW_SETUP_F;
                break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_DEACTIVATE_S:
            {
            	break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_DEACTIVATE_F:
            {
            	break;
            }

            case NW_FLOW_EVT_TYPE_PING_RECEIVE_S: {
                char str[30] = {0};

                nw_ui_draw_item(NW_UI_STR_SEND_PING_S_IDX, NW_UI_STR_SEND_PING_S);

                sprintf(str,
                        NW_UI_STR_RCV_PING_RESULT,
                        (int)nw_ui_cntx.ping_total_num,
                        (int)nw_ui_cntx.ping_recv_num,
                        (int)nw_ui_cntx.ping_lost_num);

                nw_ui_draw_item(NW_UI_STR_RCV_PING_RESULT_IDX, str);

                nw_ui_cntx.try_again_state = NW_UI_TA_STATE_SEND_PING_S;
                nw_ui_cntx.ping_result = NW_PING_RESULT_SUCCESS;
                gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);

                // TODO:
                /* if (fota_is_entered()) {
                    //xTaskCreate(fota_download_task, FOTA_DOWNLOAD_TASK_NAME, FOTA_DOWNLOAD_TASK_STACKSIZE / (( uint32_t )sizeof( StackType_t )), NULL, FOTA_DOWNLOAD_TASK_PRIO, NULL);
                } else {
                    //gprs_set_modem_sleep(nw_sleep_cb, NULL);
                }*/

                break;
            }

            case NW_FLOW_EVT_TYPE_PING_RECEIVE_F: {
                nw_ui_draw_item(NW_UI_STR_SEND_PING_F_IDX, NW_UI_STR_SEND_PING_F);

                nw_ui_cntx.try_again_state = NW_UI_TA_STATE_SEND_PING_F;
                nw_ui_cntx.ping_result = NW_PING_RESULT_FAILED;
                gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
                break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_DEACTIVATE_IND: {
                nw_ui_cntx.curr_action = 0;
                gdi_draw_filled_rectangle(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height, nw_ui_cntx.bg_color);
                nw_ui_draw_item(NW_UI_STR_MODEM_IS_READY_IDX, NW_UI_STR_MODEM_IS_READY);
                nw_ui_draw_item(NW_UI_STR_DEACTIVATE_GPRS_IND_IDX, NW_UI_STR_DEACTIVATE_GPRS_IND);

                nw_ui_draw_try_again();
                nw_ui_cntx.try_again_state = NW_UI_TA_STATE_ACTIVATE_GPRS_F;
                break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_DEACTIVATED_BY_SLEEP: {
                nw_ui_draw_try_again();
                nw_ui_cntx.try_again_state = NW_UI_TA_STATE_ACTIVATE_GPRS_F;
                break;
            }

            default: {
                break;
            }
        }

        gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
    } else if (MESSAGE_ID_GPRS_ATCMD_SENT_CB == event_id) {
        gprs_atcmd_sent_cb_hdl();
    } else if (MESSAGE_ID_GPRS_NOTI_HDL == event_id) {
        gprs_noti_hdl(param1);
    }
}


void nw_show_screen(void)
{
    LOGI("enter\r\n");

    nw_ui_register_touch_event_callback(nw_ui_pen_evt_hdl, NULL);
    nw_ui_init();
    nw_ui_draw_main_screen();
}

void nw_ui_reset_action()
{
    nw_ui_cntx.curr_action = 0;
    LOGI("%d", nw_ui_cntx.curr_action);
}

#include "gnss_app.h"
extern void gnss_location_get_cell_info();
extern void epo_init(int32_t trunk_num, char* epo_server_name, char* query_string);
void gnss_nw_event_hdl(message_id_enum event_id, int32_t param1, void *param2)
{
    LOGI("gnss_nw_event_hdl(), event_id=%d, param1=%d\r\n", event_id, param1);
    if(MESSAGE_ID_NW_FLOW == event_id && param1 ==  NW_FLOW_EVT_TYPE_PING_RECEIVE_S) {
        char str[30] = {0};

        nw_ui_draw_item(NW_UI_STR_SEND_PING_S_IDX, NW_UI_STR_SEND_PING_S);

        sprintf(str,
                NW_UI_STR_RCV_PING_RESULT,
                (int)nw_ui_cntx.ping_total_num,
                (int)nw_ui_cntx.ping_recv_num,
                (int)nw_ui_cntx.ping_lost_num);

        nw_ui_draw_item(NW_UI_STR_RCV_PING_RESULT_IDX, str);

        //nw_ui_draw_try_again();

        nw_ui_cntx.try_again_state = NW_UI_TA_STATE_SEND_PING_S;
        nw_ui_cntx.ping_result = NW_PING_RESULT_SUCCESS;
        /* Flush the screen immediately */
        gdi_lcd_update_screen(0, 0, nw_ui_cntx.lcd_width, nw_ui_cntx.lcd_height);
        gnss_send_msg(GNSS_ENUM_NW_READY , 0, NULL);
        return;
    }

    nw_event_hdl(event_id, param1, param2);
}

void gnss_show_screen(void)
{
    LOGI("gnss_show_screen()\r\n");
    nw_show_screen();
}


