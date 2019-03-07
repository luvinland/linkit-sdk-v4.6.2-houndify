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


#ifndef __BT_NOTIFY_APP_LIST_H__
#define __BT_NOTIFY_APP_LIST_H__
#include "stdbool.h"
#include <stdint.h>

#include <bt_notify.h>
//#include <ble_ancs_gprot.h>

typedef enum {
    BT_NOTIFY_APP_LIST_NONE,
    BT_NOTIFY_APP_LIST_NOTIFICATION,    
    BT_NOTIFY_APP_LIST_SMS,
    BT_NOTIFY_APP_LIST_MISSED_CALL,
    BT_NOTIFY_APP_LIST_ANCS
} bt_notify_app_list_type_t;

typedef struct {
    uint8_t data[1024];  
    uint16_t data_len;
    uint32_t timestamp;
}bt_notify_ancs_t;
typedef enum {
    BT_NOTIFY_APP_REMOTE_SYSTEM_NONE,
    BT_NOTIFY_APP_REMOTE_SYSTEM_ANDROID,
    BT_NOTIFY_APP_REMOTE_SYSTEM_IOS

}bt_notify_app_remote_system_t;

#define BT_NOTIFY_APP_LIST_MAX_NUMBER  10
typedef struct {
    uint16_t sender[BT_NOTIFY_MAX_SENDER_LENGTH];
    uint16_t content[BT_NOTIFY_MAX_PAGE_CONTENT_LENGTH];
    uint16_t title[BT_NOTIFY_MAX_PAGE_TITLE_LENGTH];
    uint32_t action_number;
    bt_notify_action_content_t action_content[BT_NOTIFY_MAX_ACTION_NUM];
    uint32_t timestamp;
    uint32_t app_id;
    bt_notify_action_type_t action;
    uint32_t msg_id;

} bt_notify_list_noti_t;

typedef struct {
    uint16_t sender[BT_NOTIFY_MAX_SENDER_LENGTH];
    uint16_t sender_number[BT_NOTIFY_MAX_PHONE_NUMBER_LENGTH];
    uint16_t content[BT_NOTIFY_MAX_PAGE_CONTENT_LENGTH];
    uint16_t title[BT_NOTIFY_MAX_PAGE_TITLE_LENGTH];
    uint32_t msg_id;
    uint32_t timestamp;
} bt_notify_list_sms_t;

typedef struct {
    int32_t list_index;
    bt_notify_app_remote_system_t remote_system;
    
    uint16_t title[BT_NOTIFY_MAX_PAGE_TITLE_LENGTH];// if notification/sms, show title.
                                          // if missed call/sms, show call number:uint16_t sender_number[BT_NOTIFY_MAX_PHONE_NUMBER_LENGTH];
                                          // if alarm, show time.
    bt_notify_app_list_type_t notification_type;
    union{
        bt_notify_list_noti_t notification;
        bt_notify_list_sms_t sms;
        bt_notify_missed_call_t missed_call;
        bt_notify_ancs_t data;
    };                                      
    
} bt_notify_app_list_t;

typedef void (*bt_notify_app_list_callback)(void);

typedef struct {
    bt_bd_addr_t bt_addr;
    uint32_t list_item_count;
    bt_notify_app_list_t list;
}bt_notify_app_cntx;

#ifdef __cplusplus
extern "C"
{
#endif

bt_notify_app_list_t* bt_notify_app_get_list(void);
void bt_notify_callback_func(void *data);

bool bt_notify_app_set_list(bt_notify_app_list_t *list_item);
void bt_notify_app_list_init(void);
void bt_notify_app_list_set_selected_index(uint8_t menuIndex);

bt_notify_app_list_t*  bt_notify_app_list_get_item(void);
uint32_t bt_notify_app_get_list_item_count(void);
int bt_notify_get_arry_size(uint16_t *DD);

void  bt_notify_app_list_register_callback(bt_notify_app_list_callback cb_ptr);
bool bt_notify_app_list_have_update(void);

void bt_notify_app_list_set_update_flag(bool flag);
void bt_notify_app_disconnect(void);
////

int32_t utf8_to_ucs2_string1(uint8_t *dest, int32_t dest_size, const uint8_t *src);

#ifdef __cplusplus
}
#endif

#endif/*__BT_NOTIFY_APP_LIST_H__*/

