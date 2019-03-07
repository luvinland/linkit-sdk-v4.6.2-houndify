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


//#include "bt_notify.h"
#include "stdint.h"
#include <string.h>
#include "stdio.h"

#include <FreeRTOS.h>

#include "queue.h"
#include "task.h"
#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_system.h"
#include "bt_spp.h"
#include "bt_callback_manager.h"
#include "bt_notify.h"
#include "task_def.h"
#include "bt_notify_app_list.h"
#include "stdbool.h"

#include "syslog.h"

log_create_module(NOTIFY_APP, PRINT_LEVEL_INFO);

static bt_notify_app_cntx g_app_cntx;

static bt_notify_app_list_t g_notify_list[BT_NOTIFY_APP_LIST_MAX_NUMBER];
static uint8_t selected_idx = 0;

//static bt_notify_app_list_callback  cb;
static bool update_flag = false;


/*****************************************************************************
 * define
 *****************************************************************************/


/*****************************************************************************
 * typedef
 *****************************************************************************/
static bool bt_notify_app_set_addr(bt_bd_addr_t* bt_addr);

void bt_notify_app_disconnect(void)
{
    memset(&g_app_cntx, 0, sizeof(bt_notify_app_cntx));
    for (uint32_t i = 0; i < BT_NOTIFY_APP_LIST_MAX_NUMBER; i++) {
        memset(&g_notify_list[i], 0, sizeof(bt_notify_app_list_t));
       g_notify_list[i].list_index = -1;
    
    }
    //cb();
    update_flag = true;
}
/**
 * @brief          This function is for recieve the event of the notify service.
 * @param[in]  data      is the data about the event.
 * @return       void.
 */

void bt_notify_callback_func(void *data)
{    
    bt_notify_callback_data_t *p_data = (bt_notify_callback_data_t *)data;

    
    LOG_I(NOTIFY_APP, "[App test]callback evt_id = %d!\r\n", p_data->evt_id);
    switch (p_data->evt_id) {
        case BT_NOTIFY_EVENT_CONNECTION: {
            bt_notify_app_set_addr(&p_data->bt_addr);
            /*connected with the remote device*/
        }
        break;
        case BT_NOTIFY_EVENT_DISCONNECTION: {
            
            /*disconnected with the remote device*/
            bt_notify_app_disconnect();       
        }
        break;
        case BT_NOTIFY_EVENT_NOTIFICATION: {

            
            /*receive a notification*/
            uint32_t i = 0, j = 0;
            bt_notify_notification_t *noti = (bt_notify_notification_t *)&p_data->notification;
            bt_notify_app_list_t list_item;
            memset(&list_item, 0, sizeof(bt_notify_app_list_t));
    
            LOG_I(NOTIFY_APP, "********Noti_S,UCS2******************\r\n");
            LOG_I(NOTIFY_APP, "notification id: %d\r\n", noti->msg_id);
            list_item.notification.msg_id = noti->msg_id;
            LOG_I(NOTIFY_APP, "action: %d\r\n", noti->action);
            
            list_item.notification.action = noti->action;
            if (sizeof(noti->sender)/2 > BT_NOTIFY_MAX_PAGE_TITLE_LENGTH) {
                memcpy(list_item.title, noti->sender, BT_NOTIFY_MAX_PAGE_TITLE_LENGTH);

            } else {
                memcpy(list_item.title, noti->sender, sizeof(noti->sender));
            }
            
            LOG_I(NOTIFY_APP, "title: %s\r\n", list_item.title);
            if (noti->action != BT_NOTIFY_ACTION_TYPE_DELETE) {
                LOG_I(NOTIFY_APP, "sender: %s\r\n", noti->sender);
                
                memcpy(list_item.notification.sender, noti->sender, sizeof(noti->sender));
                LOG_I(NOTIFY_APP, "timestamp: %d\r\n", noti->timestamp);
                
                list_item.notification.timestamp = noti->timestamp;
                LOG_I(NOTIFY_APP, "app id: %d\r\n", noti->app_id);
                
                list_item.notification.app_id = noti->app_id;

                LOG_I(NOTIFY_APP, "page num: %d\r\n", noti->page_num);
             
                {
                    bt_notify_page_content_list_t *page_content = NULL;

                    for (i = noti->page_num; i > 0; i--) {
                        page_content = noti->page_content;
                        for (j = 0; j < i - 1; j++) {
                            page_content = page_content->next;
                        }

                        LOG_I(NOTIFY_APP, "content: %s\r\n", page_content->content);
                        LOG_I(NOTIFY_APP, "title: %s\r\n", page_content->title);

                    }
                     memcpy(list_item.notification.content, page_content->content, sizeof(page_content->content));
                     memcpy(list_item.notification.title, page_content->title, sizeof(page_content->title));
                }

                   
                LOG_I(NOTIFY_APP, "action number: %d\r\n", noti->action_number);
                
                list_item.notification.action_number = noti->action_number;
                for (i = 0; i < noti->action_number; i++) {
                    LOG_I(NOTIFY_APP, "action name: %s\r\n", noti->action_content[i].action_name);
                    
                    memcpy(list_item.notification.action_content[i].action_name, noti->action_content[i].action_name, sizeof(noti->action_content[i].action_name));

                    LOG_I(NOTIFY_APP, "action id: %s\r\n", noti->action_content[i].action_id);
                                            
                    memcpy(list_item.notification.action_content[i].action_id, noti->action_content[i].action_id, sizeof(noti->action_content[i].action_id));
                }

            }
            
            list_item.notification_type = BT_NOTIFY_APP_LIST_NOTIFICATION;
            list_item.remote_system = BT_NOTIFY_APP_REMOTE_SYSTEM_ANDROID;
            
            LOG_I(NOTIFY_APP, "@@@notification_type: %s\r\n", list_item.notification_type);
            bt_notify_app_set_list(&list_item);
            
            LOG_I(NOTIFY_APP, "2222@@@notification_type: %s\r\n", list_item.notification_type);
            LOG_I(NOTIFY_APP, "********Noti_E******************\r\n");
        }
        break;
        case BT_NOTIFY_EVENT_MISSED_CALL: {

            /*receive a missed call*/
            bt_notify_missed_call_t *call = (bt_notify_missed_call_t *)&p_data->missed_call;
            bt_notify_app_list_t list_item;
            memset(&list_item, 0, sizeof(bt_notify_app_list_t));
    
            list_item.notification_type = BT_NOTIFY_APP_LIST_MISSED_CALL;
            list_item.remote_system = BT_NOTIFY_APP_REMOTE_SYSTEM_ANDROID;
            memcpy(list_item.title, call->sender_number, sizeof(call->sender_number));
            
            LOG_I(NOTIFY_APP, "title: %s\r\n", list_item.title);
            LOG_I(NOTIFY_APP, "********MissedCall_S******************\r\n");
            LOG_I(NOTIFY_APP, "sender: %s\r\n", call->sender);
            memcpy(list_item.missed_call.sender, call->sender, sizeof(call->sender));

            LOG_I(NOTIFY_APP, "send number: %s\r\n", call->sender_number);
            
            memcpy(list_item.missed_call.sender_number, call->sender_number, sizeof(call->sender_number));

            LOG_I(NOTIFY_APP, "missed call count: %d\r\n", call->missed_call_count);
            
            list_item.missed_call.missed_call_count = call->missed_call_count;
            LOG_I(NOTIFY_APP, "notification id: %d\r\n", call->msg_id);
            
            list_item.missed_call.msg_id = call->msg_id;
            LOG_I(NOTIFY_APP, "timestamp: %d\r\n", call->timestamp);
            
            list_item.missed_call.timestamp = call->timestamp;
            LOG_I(NOTIFY_APP, "********MissedCall_E******************\r\n");
            
            list_item.remote_system = BT_NOTIFY_APP_REMOTE_SYSTEM_ANDROID;
            bt_notify_app_set_list(&list_item);

        }
        break;
        case BT_NOTIFY_EVENT_SMS: {
            /*receive a SMS*/
            bt_notify_sms_t *sms = (bt_notify_sms_t *)&p_data->sms;
            bt_notify_app_list_t list_item;
            memset(&list_item, 0, sizeof(bt_notify_app_list_t));
    
            list_item.notification_type = BT_NOTIFY_APP_LIST_SMS;
            list_item.remote_system = BT_NOTIFY_APP_REMOTE_SYSTEM_ANDROID;
            memcpy(list_item.title, sms->sender_number, sizeof(sms->sender_number));
            LOG_I(NOTIFY_APP, "title: %s\r\n", list_item.title);

            LOG_I(NOTIFY_APP, "********SMS_S UCS2******************\r\n");
            LOG_I(NOTIFY_APP, "sender: %s\r\n", sms->sender);

            memcpy(list_item.sms.sender, sms->sender, sizeof(sms->sender));
            LOG_I(NOTIFY_APP, "send number: %s\r\n", sms->sender_number);
            
            memcpy(list_item.sms.sender_number, sms->sender_number, sizeof(sms->sender_number));

            LOG_I(NOTIFY_APP, "page content: %s\r\n", sms->page_content->content);
            

            memcpy(list_item.sms.content, sms->page_content->content, sizeof(sms->page_content->content));
            LOG_I(NOTIFY_APP, "page title: %s\r\n", sms->page_content->title);
            
            LOG_I(NOTIFY_APP, "notification id: %d\r\n", sms->msg_id);
            
            list_item.sms.msg_id = sms->msg_id;
            LOG_I(NOTIFY_APP, "timestamp: %d\r\n", sms->timestamp);
            
            list_item.sms.timestamp = sms->timestamp;
            LOG_I(NOTIFY_APP, "********SMS_E******************\r\n");

            list_item.remote_system = BT_NOTIFY_APP_REMOTE_SYSTEM_ANDROID;
            bt_notify_app_set_list(&list_item);
        }
        break;
        default:
            break;
    }

}
//regist commpn serivice functionto ui.
void  bt_notify_app_list_register_callback(bt_notify_app_list_callback cb_ptr)
{
    LOG_I(NOTIFY_APP, "bt_notify_app_list_register_callback\r\n");

    //cb = cb_ptr;
    

}

/**
 * @brief          This function is for app init implement.
 * @param[in]  void.
 * @return       void.
 */

void bt_notify_app_list_init(void)
{

    memset(&g_app_cntx, 0, sizeof(bt_notify_app_cntx));
   
    for (uint32_t i = 0; i < BT_NOTIFY_APP_LIST_MAX_NUMBER; i++) {
         memset(&g_notify_list[i], 0, sizeof(bt_notify_app_list_t));
        g_notify_list[i].list_index = -1;
     
    }

}

bt_notify_app_list_t* bt_notify_app_get_list(void)
{
    return g_notify_list;

}
int32_t bt_notify_app_list_get_free_index(void)
{
    uint32_t i = 0;

    for (i = 0; i < BT_NOTIFY_APP_LIST_MAX_NUMBER; i++) {
        if (g_notify_list[i].list_index == -1) {
            return i;
        }
    }
    return i;

}
//shft the list.
bool bt_notify_app_list_item_is_avaliable(uint32_t idx)
{
    for (uint32_t i = 0; i < BT_NOTIFY_APP_LIST_MAX_NUMBER; i++) {
        if (g_notify_list[i].list_index == idx) {
           return true;
        }
    }
    return false;
}

void bt_notify_app_list_del_item(uint32_t idx)
{
    uint32_t j;

        if (bt_notify_app_list_item_is_avaliable(idx)) {
            for (j = 0; j < BT_NOTIFY_APP_LIST_MAX_NUMBER - 1; j++) {
                //shift array
                memcpy(&g_notify_list[j], &g_notify_list[j + 1], sizeof(bt_notify_app_list_t));
                g_notify_list[j].list_index-=1;
            }
            memset(&g_notify_list[BT_NOTIFY_APP_LIST_MAX_NUMBER - 1], 0, sizeof(bt_notify_app_list_t));
            g_notify_list[BT_NOTIFY_APP_LIST_MAX_NUMBER - 1].list_index = -1;
        }
        g_app_cntx.list_item_count--;
}
bt_notify_app_cntx* bt_notify_app_get_cntx(void)
{
    uint32_t i;
    for (i = 0; i < 6; i++) {
        if (0 != g_app_cntx.bt_addr[i]) {
            return &g_app_cntx;
        }
    }
 
    return NULL;

}

static bool bt_notify_app_set_addr(bt_bd_addr_t* bt_addr)
{
    if (bt_notify_app_get_cntx() == NULL) {
        memcpy(&g_app_cntx.bt_addr, bt_addr, sizeof(bt_bd_addr_t)); 
        return true;
    }
    return false;
}


bool bt_notify_app_set_list(bt_notify_app_list_t *list_item)
{
    if (list_item != NULL) {
        int32_t idx = bt_notify_app_list_get_free_index();
        if (idx == BT_NOTIFY_APP_LIST_MAX_NUMBER) {
           bt_notify_app_list_del_item(0);
           idx = BT_NOTIFY_APP_LIST_MAX_NUMBER-1;
        }
        LOG_I(NOTIFY_APP, "idx:%d\n", idx);
        g_notify_list[idx].remote_system = list_item->remote_system;
        g_notify_list[idx].list_index = idx;       
        g_notify_list[idx].notification_type = list_item->notification_type;
        LOG_I(NOTIFY_APP, "type:%d\n", list_item->notification_type);
        switch (g_notify_list[idx].notification_type) {
            case BT_NOTIFY_APP_LIST_NOTIFICATION:
            {
                memcpy(&g_notify_list[idx].notification, &list_item->notification, sizeof(bt_notify_list_noti_t));
                //UT log
                LOG_I(NOTIFY_APP, "notification list&&&&&&&&&&&&&&&&&:\n");

                LOG_I(NOTIFY_APP, "notification id: %d\r\n", g_notify_list[idx].notification.msg_id);
                LOG_I(NOTIFY_APP, "action: %d\r\n", g_notify_list[idx].notification.action);

                LOG_I(NOTIFY_APP, "sender: %s\r\n", g_notify_list[idx].notification.sender);

                LOG_I(NOTIFY_APP, "timestamp: %d\r\n", g_notify_list[idx].notification.timestamp);

                LOG_I(NOTIFY_APP, "app id: %d\r\n", g_notify_list[idx].notification.app_id);        

                LOG_I(NOTIFY_APP, "content: %s\r\n", g_notify_list[idx].notification.content);
                LOG_I(NOTIFY_APP, "title: %s\r\n", g_notify_list[idx].notification.title);


                LOG_I(NOTIFY_APP, "action number: %d\r\n", g_notify_list[idx].notification.action_number);

                for (uint32_t i = 0; i < g_notify_list[idx].notification.action_number; i++) {
                    LOG_I(NOTIFY_APP, "action name: %s\r\n", g_notify_list[idx].notification.action_content[i].action_name);

                    LOG_I(NOTIFY_APP, "action id: %s\r\n", g_notify_list[idx].notification.action_content[i].action_id);
                }
                LOG_I(NOTIFY_APP, "notification list&&&&&&&&&&&&&&&&&end\n");
                //debug: check list .
                 for (uint32_t i = 0; i < (idx +1); i++) {
                     LOG_I(NOTIFY_APP, "i:%d, id: %d\r\n",i, g_notify_list[i].notification.msg_id);

                }
            }
            break;
            case BT_NOTIFY_APP_LIST_SMS:
            {
                memcpy(&g_notify_list[idx].sms, &list_item->sms, sizeof(bt_notify_list_sms_t));
                //UT log
                LOG_I(NOTIFY_APP, "sms list&&&&&&&&&&&&&&&&&:\n");
                LOG_I(NOTIFY_APP, "sender: %s\r\n", g_notify_list[idx].sms.sender);
 
                LOG_I(NOTIFY_APP, "send number: %s\r\n", g_notify_list[idx].sms.sender_number);
                

                LOG_I(NOTIFY_APP, "page content: %s\r\n", g_notify_list[idx].sms.content);
                
 
                LOG_I(NOTIFY_APP, "page title: %s\r\n", g_notify_list[idx].sms.title);

                LOG_I(NOTIFY_APP, "notification id: %d\r\n", g_notify_list[idx].sms.msg_id);
                
                LOG_I(NOTIFY_APP, "timestamp: %d\r\n", g_notify_list[idx].sms.timestamp);
                
                
                LOG_I(NOTIFY_APP, "sms list&&&&&&&&&&&&&&&&&end\n");
            }
            break;
            case BT_NOTIFY_APP_LIST_MISSED_CALL:
            {
                memcpy(&g_notify_list[idx].missed_call, &list_item->missed_call, sizeof(bt_notify_missed_call_t));
                //UT log
                LOG_I(NOTIFY_APP, "MISSED_CALL list&&&&&&&&&&&&&&&&&:\n");
                LOG_I(NOTIFY_APP, "sender: %s\r\n", g_notify_list[idx].missed_call.sender);

                LOG_I(NOTIFY_APP, "send number: %s\r\n", g_notify_list[idx].missed_call.sender_number);

                LOG_I(NOTIFY_APP, "missed call count: %d\r\n", g_notify_list[idx].missed_call.missed_call_count);
                
                LOG_I(NOTIFY_APP, "notification id: %d\r\n", g_notify_list[idx].missed_call.msg_id);
                
                LOG_I(NOTIFY_APP, "timestamp: %d\r\n", g_notify_list[idx].missed_call.timestamp);
                
                LOG_I(NOTIFY_APP, "MISSED_CALL list&&&&&&&&&&&&&&&&&end\n");
            }
            break;
            case BT_NOTIFY_APP_LIST_ANCS: 
            {
                //memcpy(g_notify_list[idx].data.data, list_item->data.data, list_item->data.data_len);   
                memcpy(g_notify_list[idx].data.data, list_item->data.data, sizeof(list_item->data.data));  
                g_notify_list[idx].data.timestamp = list_item->data.timestamp;
                //UT log
                
                LOG_I(NOTIFY_APP, "timestamp: %d\r\n", g_notify_list[idx].data.timestamp);               
                LOG_I(NOTIFY_APP, "data: %s\r\n", g_notify_list[idx].data.data);
            }
            break;
            default:
            break;
        }
        memcpy(g_notify_list[idx].title,list_item->title, sizeof(list_item->title));
        
        g_app_cntx.list_item_count++;
        LOG_I(NOTIFY_APP, "title: %s, count:%d\r\n", g_notify_list[idx].title, g_app_cntx.list_item_count);
        
        LOG_I(NOTIFY_APP, "callback UI\r\n");
        //cb();
        update_flag = true;
    }
    return true;
}

uint32_t bt_notify_app_get_list_item_count(void)
{
    LOG_I(NOTIFY_APP, "count:%d\r\n", g_app_cntx.list_item_count);
    return g_app_cntx.list_item_count;

}
bool bt_notify_app_list_have_update(void)
{   
    if (update_flag == 1) {
        LOG_I(NOTIFY_APP, "bt_notify_app_list_have_update:%d\r\n", update_flag);
    }
    return update_flag;
}
void bt_notify_app_list_set_update_flag(bool flag)
{   
    
    LOG_I(NOTIFY_APP, "bt_notify_app_list_set_update_flag\r\n");
    update_flag = false;
}

bool bt_notify_app_reset_list(bt_bd_addr_t* bt_addr)
{
    if (&g_app_cntx.bt_addr == bt_addr) {
        memset(&g_app_cntx, 0, sizeof(bt_notify_app_cntx));
        memset(g_notify_list, 0, sizeof(bt_notify_app_list_t)*BT_NOTIFY_APP_LIST_MAX_NUMBER);
        return true;

    }
    return false;
}
void bt_notify_app_list_set_selected_index(uint8_t menuIndex)
{
    selected_idx = menuIndex;

}

bt_notify_app_list_t*  bt_notify_app_list_get_item(void)
{
    // (selected_idx != -1) {
        return &g_notify_list[selected_idx];

    //else {
     // LOG_I(NOTIFY_APP, "not select list item.\n");
     // return NULL;

    //}
   

}
int bt_notify_get_arry_size(uint16_t *DD)
{
	int size = 0;
	for (int i = 0;; i++)
	{
		if (DD[i] != '\0') {

			size++;
		}
		else
		{
			break;
		}

	}
	return size;
}
/////



/*****************************************************************************
 * define
 *****************************************************************************/


#define CAST_PU8_1(RAW)   ((uint8_t*)(RAW))
#define STR_SIZE_1(len) ((len)<<1)
#define STR_AT_1(RAW, n) ((void*)(CAST_PU8_1(RAW)+STR_SIZE_1(n)))

/*****************************************************************************
 * function
 *****************************************************************************/
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

int32_t utf8_to_ucs2_string_ex1(
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

int32_t utf8_to_ucs2_string1(uint8_t *dest, int32_t dest_size, const uint8_t *src)
{
    uint32_t src_end_pos = (uint32_t) src;

    if (NULL == src || NULL == dest) {
        return 0;
    }
    return utf8_to_ucs2_string_ex1(dest, dest_size, src, &src_end_pos);
}

