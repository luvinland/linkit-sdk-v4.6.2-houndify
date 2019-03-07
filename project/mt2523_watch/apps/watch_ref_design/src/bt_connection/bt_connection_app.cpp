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
#include <string.h>
#include "stdio.h"

#include <FreeRTOS.h>

#include "queue.h"
#include "task.h"
#include "timers.h"
#include "bt_gap.h"
#include "bt_system.h"
#include "bt_callback_manager.h"
#include "bt_notify.h"
#include "task_def.h"
#include "bt_connection_app.h"
#include "bt_source_srv.h"
#include "stdbool.h"

#include "syslog.h"


#include <gui/home_screen/HomeView.hpp>
#include <BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <gui/common/CommonUI.hpp>
#include <gui/common/CommonService.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>


log_create_module(BT_CONNECTION, PRINT_LEVEL_INFO);

bt_connection_app_cntx_t            g_bt_connection_cntx_t;
bt_connection_app_connected_list_t  g_bt_connection_app_connected_list[BT_CONNECTION_APP_CONNECTED_LIST_MAX_NUMBER];
bt_connection_app_search_list_t     g_bt_connection_app_search_list[BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER];
bt_connection_app_pair_list_t       g_bt_connection_app_paired_list[BT_CONNECTION_APP_PAIR_LIST_MAX_NUMBER];

static bt_connection_app_state_t    g_state;
static bool                         update_flag = false;
static bool                         connection_result_flag = false;
static bool                         first_inqure_flag = true;
static bool                         from_pair_to_connect = false;
static bool                         disconnection_result_flag = false;

uint8_t                             g_app_attampts = 5;
uint32_t                            g_app_delay_time = 100;
TimerHandle_t                       g_app_recon_timer_id = 0;


#define CAST_PU8_1(RAW)   ((uint8_t*)(RAW))
#define STR_SIZE_1(len) ((len)<<1)
#define STR_AT_1(RAW, n) ((uint8_t*)(CAST_PU8_1(RAW)+STR_SIZE_1(n)))

#define BT_CONN_APP_REQUEST_DELAY_TIME_INCREASE (4000)


void bt_conection_add_connected_list(bt_source_srv_a2dp_connect_t *p_a2dp_info) {
	uint32_t connected_index = 0; //g_bt_connection_cntx_t.current_connected_device_index;
	bt_connection_app_connected_list_t *connected_dev = &(g_bt_connection_app_connected_list[connected_index]);
    const uint8_t *p_bt_address = (const uint8_t *)p_a2dp_info->address;
    nvdm_status_t status = NVDM_STATUS_OK;
    uint8_t buffer[18] = {0};
    int i = 0;
    
	memcpy(connected_dev->bt_addr, p_a2dp_info->address, sizeof(bt_bd_addr_t));
	connected_dev->dev_handle = p_a2dp_info->handle;
    
    Unicode::snprintf(connected_dev->dev_name,
                BT_CONNECTION_APP_DEVICE_NAME_LENGTH,
                "%2X:%2X:%2X%:%2X:%2X%:%2X",
                *(p_bt_address + 5), *(p_bt_address + 4), *(p_bt_address + 3),
                *(p_bt_address + 2), *(p_bt_address + 1), *p_bt_address);

    // save bt addr to NVRAM
    /* save address to NVDM */
    for (i = 0; i < 6; ++i) {
        sprintf((char *)buffer + 2 * i, "%02X", p_bt_address[i]);
    }
    LOG_I(BT_CONNECTION, "[BT]address to write:%s len:%d \r\n", buffer, strlen((char *)buffer));
    status = nvdm_write_data_item(TOUCHGFX_NVDM_GROUP, 
                                  BLUETOOTH_ADDR_KEY, 
                                  NVDM_DATA_ITEM_TYPE_STRING, 
                                  buffer,
                                  strlen((char *)buffer));
    LOG_I(tgfx, "store bt address: %d\r\n", status);
    if (NVDM_STATUS_OK != status) {
        LOG_I(BT_CONNECTION, "Failed to store address: %d\r\n", status);
    } else {
        LOG_I(BT_CONNECTION, "Successfully store address to NVDM");
    }

	// only one device can connect success, multi-link ignnore here
	g_bt_connection_cntx_t.current_connected_device_index = 0;
    g_bt_connection_cntx_t.connection_list_item_count = 1;

    // need to update paired list if they are same
    if (memcmp(g_bt_connection_app_paired_list[0].bt_addr,
               p_a2dp_info->address, 
               sizeof(bt_bd_addr_t)) == 0) {
        memset(g_bt_connection_app_paired_list, 0 , 
               sizeof(bt_connection_app_pair_list_t) * BT_CONNECTION_APP_PAIR_LIST_MAX_NUMBER);
        g_bt_connection_cntx_t.pair_list_item_count = 0;
        LOG_I(BT_CONNECTION, "clean g_bt_connection_app_paired_list");
    }

    bt_connection_set_status_from_pait_to_connect(true);
}

void bt_connection_source_srv_a2dp_cb (bt_source_srv_event_t event_id, void *param) {
	bt_source_srv_a2dp_connect_t *p_a2dp_data = (bt_source_srv_a2dp_connect_t *)param;
    LOG_I(BT_CONNECTION, "bt_connection_source_srv_a2dp_cb event_id: %d\r\n",
					  event_id);
	switch (event_id) {
		case BT_SOURCE_SRV_EVENT_A2DP_CONNECT:
            LOG_I(BT_CONNECTION, "BT_SOURCE_SRV_EVENT_A2DP_CONNECT status\r\n");
			if(BT_STATUS_SUCCESS == p_a2dp_data->result) {
				LOG_I(BT_CONNECTION, "connection success, list handle:%0x\r\n",
					  p_a2dp_data->handle);
				bt_conection_add_connected_list(p_a2dp_data);
                bt_connection_app_set_connection_result_flag(true);
			}else {
				LOG_I(BT_CONNECTION, "connection fail: 0x%x\r\n",p_a2dp_data->result);
				return;
			}
			break;
		case BT_SOURCE_SRV_EVENT_A2DP_DISCONNECT: {
            LOG_I(BT_CONNECTION, "BT_SOURCE_SRV_A2DP_DISCONNECT_EVENT status\r\n");

            if(BT_STATUS_SUCCESS == p_a2dp_data->result) {            
                g_bt_connection_cntx_t.current_connected_device_index = -1;
                g_bt_connection_cntx_t.current_connecting_device_index = -1;
                g_bt_connection_cntx_t.connection_list_item_count = 0;
                memset(g_bt_connection_app_connected_list, 0, 
                  (sizeof(bt_connection_app_connected_list_t) * BT_CONNECTION_APP_CONNECTED_LIST_MAX_NUMBER));
                bt_connection_app_set_disconnection_result_flag(true);
                LOG_I(BT_CONNECTION, "BT_SOURCE_SRV_EVENT_A2DP_DISCONNECT success: %d", p_a2dp_data->result);
            }else {
                LOG_I(BT_CONNECTION, "BT_SOURCE_SRV_EVENT_A2DP_DISCONNECT fail: %d", p_a2dp_data->result);
            }
            }
			break;
		default:
			break;
	}
}


//add to serach list.g_bt_connection_cntx_t. search_list_item_count ++, addrss copy to name. set in_name_request = true.
void bt_connection_add_search_list(bt_gap_inquiry_ind_t *gap_info) {
	const bt_bd_addr_t *p_address = gap_info->address;
	const uint8_t *p_bt_address = (const uint8_t *)p_address;
	int index = g_bt_connection_cntx_t.search_list_item_count;
	bt_connection_app_search_list_t *temp_device = NULL;
	int search_count = index;
	
	
	if(BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER == search_count) {
		// if more than maximum, skip it.
		return;
	}

	temp_device = &g_bt_connection_app_search_list[index];
	temp_device->in_name_request = false;
	memcpy(temp_device->bt_addr, gap_info->address, sizeof(bt_bd_addr_t));

	Unicode::snprintf(temp_device->dev_name,
		    BT_CONNECTION_APP_DEVICE_NAME_LENGTH,
			"%2X:%2X:%2X%:%2X:%2X%:%2X",
    		*(p_bt_address + 5), *(p_bt_address + 4), *(p_bt_address + 3),
    		*(p_bt_address + 2), *(p_bt_address + 1), *p_bt_address);

	LOG_I(BT_CONNECTION, "bt_connection_add_search_list %s\r\n", temp_device->dev_name);

	++search_count;
	g_bt_connection_cntx_t.search_list_item_count = (search_count > BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER)?BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER:search_count;
}


bt_bd_addr_t g_temp_connecting_bt_addr;

void bt_connection_app_timer_callback(TimerHandle_t xTimer)
{
	LOG_I(BT_CONNECTION, "bt_connection_app_timer_callback");

	bt_source_srv_a2dp_connect(&g_temp_connecting_bt_addr);
}


void bt_connection_app_handle_link_recon_done()
{
	LOG_I(BT_CONNECTION, "bt_connection_app_handle_link_recon_done");

    if(g_app_recon_timer_id)
    {
    	xTimerStop(g_app_recon_timer_id, 0);
    	xTimerDelete(g_app_recon_timer_id,0);
    	g_app_recon_timer_id = 0;
    }

    g_app_delay_time = 100;
    
    memset(g_temp_connecting_bt_addr, 0, sizeof(bt_bd_addr_t));
}

void bt_connection_app_handle_link_recon()
{

    LOG_I(BT_CONNECTION, "bt_connection_app_handle_link_recon g_app_attampts: %d", g_app_attampts);

    if(g_app_attampts > 0 ) 
    {
    	g_app_attampts--;
    }
    else
    {
      LOG_I(BT_CONNECTION, "bt_connection_app_handle_link_recon g_app_attampts0: %d", g_app_attampts);
      bt_connection_app_handle_link_recon_done();
      return;
    }
    
    LOG_I(BT_CONNECTION, "bt_connection_app_handle_link_recon g_app_attampts1: %d", g_app_attampts);
    if(!g_app_recon_timer_id )
    {
    	g_app_recon_timer_id = xTimerCreate("BT_CONN_APP_RETRY_TIMER",                /* Just a text name, not used by the kernel. */
                            (g_app_delay_time / portTICK_PERIOD_MS),  /* The timer period in ticks. */
                            pdFALSE,
                            (void *) 0,
                            bt_connection_app_timer_callback);
    	xTimerStart(g_app_recon_timer_id, 0);
    }
    else
    {
        g_app_delay_time += BT_CONN_APP_REQUEST_DELAY_TIME_INCREASE;
    	LOG_I(BT_CONNECTION, "bt_connection_app_handle_link_recon g_app_delay_time: %d", g_app_delay_time);
    	xTimerChangePeriod(g_app_recon_timer_id, g_app_delay_time/ portTICK_PERIOD_MS, 0);
    	xTimerReset(g_app_recon_timer_id, 0);
    }    	

}


bt_status_t bt_connection_app_event_callback(
    bt_msg_type_t msg,
    bt_status_t status,
    void *buff)
{
    //LOG_I(BT_CONNECTION,"bt_connection_app_event_callback event:0x%x, %d\n", msg, status);
	printf("[BT_CM] bt_connection_app_event_callback event:0x%x, %d\n", (unsigned int)msg, (unsigned int)status);
	switch (msg) {
        case BT_GAP_INQUIRY_CNF:
        break;
        case BT_GAP_INQUIRY_IND: {
         	// Find a near//by device.
            bt_gap_inquiry_ind_t* device = (bt_gap_inquiry_ind_t*) buff;
			bt_connection_add_search_list(device);
			bt_connection_search_set_update_flag(true);
        }
        break;
        case BT_GAP_INQUIRY_COMPLETE_IND: { // The inquiry is complete.        
		   LOG_I(BT_CONNECTION, "search completed BT_GAP_INQUIRY_COMPLETE_IND\r\n");
           bt_connection_app_set_state(BT_CONNECTION_APP_STATE_IDLE);           
        }
	    break;
        case BT_GAP_LINK_STATUS_UPDATED_IND:{
		    bt_gap_link_status_updated_ind_t *param = (bt_gap_link_status_updated_ind_t *)buff;
			LOG_I(BT_CONNECTION, "BT_GAP_LINK_STATUS_UPDATED_IND param->link_status: %d,status:%d\r\n",param->link_status,status);
			if (BT_GAP_LINK_STATUS_DISCONNECTED == param->link_status) {
				if ( 0x08 == (status & 0xff) ){
                        uint8_t *p_bt_address = NULL;
                        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_NOT_ACCESSIBLE);
                        memcpy(g_temp_connecting_bt_addr, param->address, sizeof(bt_bd_addr_t));
                        p_bt_address = (uint8_t *)g_temp_connecting_bt_addr;
                        LOG_I(BT_CONNECTION, "BT_GAP_LINK_STATUS_UPDATED_IND disconnect confirm,addr %2X:%2X:%2X%:%2X:%2X%:%2X",
                              *(p_bt_address + 5), *(p_bt_address + 4), *(p_bt_address + 3),
                              *(p_bt_address + 2), *(p_bt_address + 1), *p_bt_address);
        			    bt_connection_app_handle_link_recon();
                    }
				}else{
                    bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
                }
			}				
		break;
        default:
        break;
        }
    return BT_STATUS_SUCCESS;
}

#include "bt_device_manager.h"

void bt_connection_search_data_init() {

    uint32_t i = 0;

    g_bt_connection_cntx_t.search_list_item_count = 0;

    for (i = 0; i < BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER; i++) {
         memset(&g_bt_connection_app_search_list[i], 0, sizeof(bt_connection_app_search_list_t));
    }	
}

void bt_connection_data_init() {
	uint32_t i = 0;
	
	memset(&g_bt_connection_cntx_t, 0, sizeof(bt_connection_app_cntx_t));

    for (i = 0; i < BT_CONNECTION_APP_CONNECTED_LIST_MAX_NUMBER; i++) {
         memset(&g_bt_connection_app_connected_list[i], 0, sizeof(bt_connection_app_connected_list_t));
    }
   
    bt_connection_search_data_init();
}

void bt_connection_app_init()
{ 
    bt_connection_data_init();
	// GAP
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_A2DP | MODULE_MASK_AVRCP, (void*)bt_connection_app_event_callback);
	// A2DP
	bt_source_srv_register_callback(bt_connection_source_srv_a2dp_cb);
	// BT connection manager device
	bt_device_manager_init();	
}


uint32_t bt_connection_app_get_pair_list_item_count()
{
    LOG_I(BT_CONNECTION,"pair_list_item_count:%d\n", g_bt_connection_cntx_t.pair_list_item_count);
    return g_bt_connection_cntx_t.pair_list_item_count;

}

uint32_t bt_connection_app_get_connected_list_item_count()
{
    LOG_I(BT_CONNECTION,"connection_list_item_count:%d\n", g_bt_connection_cntx_t.connection_list_item_count);
    return g_bt_connection_cntx_t.connection_list_item_count;

}

uint32_t bt_connection_app_get_searched_list_item_count()
{
    LOG_I(BT_CONNECTION,"search_list_item_count:%d\n", g_bt_connection_cntx_t.search_list_item_count);
    return g_bt_connection_cntx_t.search_list_item_count;

}

bt_connection_app_connected_list_t *bt_connection_app_get_connected_list(uint32_t indx)
{
    return &g_bt_connection_app_connected_list[indx];

}


bt_connection_app_pair_list_t *bt_connection_app_get_paired_list(uint32_t indx)
{
    return &g_bt_connection_app_paired_list[indx];

}

bt_connection_app_search_list_t *bt_connection_app_get_search_list(uint32_t indx)
{
    return &g_bt_connection_app_search_list[indx];
}

bt_connection_app_state_t bt_connection_app_get_state(void)
{
    return g_state;
}

void bt_connection_app_set_state(bt_connection_app_state_t state)
{
    g_state = state;
}

void bt_connection_app_scan(void)
{
    // every scan, need clear the last info.
    uint32_t i = 0;	

    for (i = 0; i < BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER; i++) {
        memset(&g_bt_connection_app_search_list[i], 0, sizeof(bt_connection_app_search_list_t));
    }

    if(true == CommonService::getBluetoothStatus()) {
        bt_gap_inquiry(BT_CONNECTION_APP_SEARCH_MAX_TIME, BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER);// could change the define..
    }
    return;
}


void bt_connection_app_cancel_search(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    if(true == CommonService::getBluetoothStatus()) {
        ret = bt_gap_cancel_inquiry();
        LOG_I(BT_CONNECTION, "bt_gap_cancel_inquiry ret %d", ret);
     }
    return;

}

// only can support one connected device, multi-link can't support here
void bt_connection_disconnecct_a2dp_source()
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    ret = bt_source_srv_a2dp_disconnect(g_bt_connection_app_connected_list[0].dev_handle);

    LOG_I(BT_CONNECTION, "bt_source_srv_a2dp_disconnect ret: %d  handle: %0x\r\n",
                        ret,g_bt_connection_app_connected_list[0].dev_handle);

    return;
}

void bt_connection_connect_a2dp_source(uint8_t menuIndex) {
    g_bt_connection_cntx_t.current_connecting_device_index = menuIndex;
	bt_source_srv_a2dp_connect(&(g_bt_connection_app_search_list[menuIndex].bt_addr));
}


void bt_connection_connect_a2dp() {
	bt_source_srv_a2dp_connect(&(g_bt_connection_app_paired_list[0].bt_addr));
}


bool bt_connection_search_have_update(void)
{	
    if (update_flag == 1) {
        LOG_I(BT_CONNECTION, "bt_connection_app_have_update:%d\r\n", update_flag);
    }
    return update_flag;
}

bool bt_connection_search_inqured(void)
{
    if (first_inqure_flag == true) {
        LOG_I(BT_CONNECTION, "bt_connection_app_have_first_inqured:%d\r\n", first_inqure_flag);
    }
    return first_inqure_flag;
}



bool bt_connection_get_status_from_pait_to_connect(void)
{
    return from_pair_to_connect;
}


void bt_connection_set_status_from_pait_to_connect(bool flag)
{
    from_pair_to_connect = flag;
}


void bt_connection_search_inqured_flag(bool flag)
{
    LOG_I(BT_CONNECTION, "bt_connection_app_set_first_inqured_flag\r\n");
    first_inqure_flag = flag;
}

bool bt_connection_app_get_connection_result(void)
{
    LOG_I(BT_CONNECTION, "bt_connection_app_get_connection_result:%d\r\n", connection_result_flag);
    return connection_result_flag;
}

void bt_connection_app_set_connection_result_flag(bool flag)
{

    LOG_I(BT_CONNECTION, "bt_connection_app_set_connection_result_flag  %d\r\n",flag);
    connection_result_flag = flag;
}


bool bt_connection_app_get_disconnection_result(void)
{
    return disconnection_result_flag;
}

void bt_connection_app_set_disconnection_result_flag(bool flag)
{

    LOG_I(BT_CONNECTION, "bt_connection_app_set_disconnection_result_flag  %d\r\n",flag);
    disconnection_result_flag = flag;
}


void bt_connection_search_set_update_flag(bool flag)
{
    LOG_I(BT_CONNECTION, "bt_connection_app_set_update_flag\r\n");
    update_flag = flag;
}


/*****************************************************************************
 * Tool function
 *****************************************************************************/
int bt_connection_app_get_arry_size(uint16_t *DD)
{
    int size = 0;
    int i = 0;
    for (i = 0;; i++) {
        if (DD[i] != '\0') {
            size++;
        } else {
            break;
        }

    }
    return size;
}


static void SET_CHR_AT_1(uint8_t *RAW, int32_t n, uint16_t value)
{
    do {
        uint8_t *_p = STR_AT_1(RAW, n);
        uint16_t v = (uint16_t) (value);
        _p[0] = (uint8_t) (v & 0xff);
        _p[1] = (uint8_t) (v >> 8);
    } while (0);

}

static const uint8_t g_cheset_utf8_bytes_per[16] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 2, 2, 3, 4
};

static int32_t utf8_to_ucs21(uint8_t *dest, const uint8_t *utf8)
{
    uint8_t c = utf8[0];
    int count = g_cheset_utf8_bytes_per[c >> 4];
    uint16_t ucs2 = 0xFFFF;   /* invalid character */

    switch (count) {
        case 1:
            ucs2 = (uint16_t)c;
            break;
        case 2:
            if (utf8[1]) {
                ucs2 = ((uint16_t)(c & 0x1f) << 6) | (uint16_t)(utf8[1] ^ 0x80);
            }
            break;
        case 3:
            if (utf8[1] && utf8[2]) {
                ucs2 = ((uint16_t)(c & 0x0f) << 12)
                       | ((uint16_t)(utf8[1] ^ 0x80) << 6) | (uint16_t)(utf8[2] ^ 0x80);
            }
            break;
        case 4:
            break;
        default:
            count = 1;   /* the character cannot be converted, return 1 means always consume 1 byte */
            break;
    }

    SET_CHR_AT_1(dest, 0, ucs2);

    return count;
}

int32_t bt_utf8_to_ucs2_string_ex1(
    uint8_t *dest,
    int32_t dest_size,
    const uint8_t *src,
    uint32_t *src_end_pos)
{
    int pos = 0;
    int cnt;
    int src_len = strlen((const char *)src);

    dest_size -= (dest_size % 2);
    *src_end_pos = (uint32_t)src; /* set src_end_pos first */

    if (dest_size < 2) {
        return 0;
    }

    while (*src && pos < dest_size - 2) {
        cnt = utf8_to_ucs21(dest + pos, src);
        if (((uint32_t)src - (*src_end_pos)) >= (uint32_t)(src_len - cnt) &&
                (*(dest + pos) == 0xFF && *(dest + pos + 1) == 0xFF) &&
                !(*src == 0xEF && *(src + 1) == 0xBF && *(src + 2) == 0xBF)) {
            /*
             * If src isn't 0xEF, 0xBF, 0xBF and its remain length is not enough but dest is 0xFFFF, we will abort the process.
             * dest data is invalid character because src data is not enough to convert
             */
            break;
        }
        if (cnt == 0) {  /* abnormal case */
            break;
        } else {   /* normal case */
            src += cnt;
            pos += 2;
        }
    }
    *src_end_pos = (uint32_t)src;
    dest[pos] = 0;
    dest[pos + 1] = 0;
    return pos + 2;
}

// call this API to convert string.
int32_t bt_utf8_to_ucs2_string(uint8_t *dest, int32_t dest_size, const uint8_t *src)
{
    uint32_t src_end_pos = (uint32_t) src;

    if (NULL == src || NULL == dest) {
        return 0;
    }
    return bt_utf8_to_ucs2_string_ex1(dest, dest_size, src, &src_end_pos);
}


