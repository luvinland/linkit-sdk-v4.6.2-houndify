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


#ifndef __BT_CONNECTION_APP_H__
#define __BT_CONNECTION_APP_H__
#include "stdbool.h"
#include <stdint.h>


#define BT_CONNECTION_APP_DEVICE_NAME_LENGTH 60
#define BT_CONNECTION_APP_CONNECTED_LIST_MAX_NUMBER 1
#define BT_CONNECTION_APP_PAIR_LIST_MAX_NUMBER 4
#define BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER 10
#define BT_CONNECTION_APP_SEARCH_MAX_TIME 25



typedef struct {
	 uint32_t search_list_item_count;
	 uint32_t pair_list_item_count;
	 uint32_t connection_list_item_count;
	 uint32_t current_connecting_device_index; // for connecting datas
	 uint32_t current_connected_device_index;  // for connected datas
} bt_connection_app_cntx_t;

typedef struct {
    bt_bd_addr_t bt_addr;
    uint16_t dev_name[BT_CONNECTION_APP_DEVICE_NAME_LENGTH];
	uint32_t dev_handle;
} bt_connection_app_connected_list_t;

typedef struct {
    bt_bd_addr_t bt_addr;
    uint16_t dev_name[BT_CONNECTION_APP_DEVICE_NAME_LENGTH];
	uint32_t dev_handle;
} bt_connection_app_pair_list_t;


typedef struct{
    bt_bd_addr_t bt_addr;
    uint16_t dev_name[BT_CONNECTION_APP_DEVICE_NAME_LENGTH];
    bool in_name_request;
} bt_connection_app_search_list_t;

typedef enum {

    BT_CONNECTION_APP_STATE_IDLE,
    BT_CONNECTION_APP_STATE_PAIRING,
    BT_CONNECTION_APP_STATE_CONNECTED,
    BT_CONNECTION_APP_STATE_CONNECTION_FAIL,
    BT_CONNECTION_APP_STATE_DISCONNECTING,
    BT_CONNECTION_APP_STATE_DISCONNECTED,
    BT_CONNECTION_APP_STATE_START_SEARCH,
    BT_CONNECTION_APP_STATE_SEARCHING,
} bt_connection_app_state_t;


#ifdef __cplusplus
extern "C"
{
#endif

#include "bt_gap.h"

extern bt_connection_app_cntx_t g_bt_connection_cntx_t;
extern bt_connection_app_connected_list_t g_bt_connection_app_connected_list[BT_CONNECTION_APP_CONNECTED_LIST_MAX_NUMBER];
extern bt_connection_app_search_list_t g_bt_connection_app_search_list[BT_CONNECTION_APP_SEARCH_LIST_MAX_NUMBER];
extern bt_connection_app_pair_list_t g_bt_connection_app_paired_list[BT_CONNECTION_APP_PAIR_LIST_MAX_NUMBER];

uint32_t bt_connection_app_get_pair_list_item_count(void);

uint32_t bt_connection_app_get_connected_list_item_count(void);

uint32_t bt_connection_app_get_searched_list_item_count(void);
int bt_connection_app_get_arry_size(uint16_t *DD);
bt_connection_app_connected_list_t *bt_connection_app_get_connected_list(uint32_t indx);
bt_connection_app_pair_list_t *bt_connection_app_get_paired_list(uint32_t indx);
bt_connection_app_search_list_t *bt_connection_app_get_search_list(uint32_t indx);
bt_connection_app_state_t bt_connection_app_get_state(void);


void bt_connection_app_set_state(bt_connection_app_state_t state);
void bt_connection_app_scan(void);

void bt_connection_connect_a2dp_source(uint8_t menuIndex);
void bt_connection_connect_a2dp();
void bt_connection_disconnecct_a2dp_source();
void bt_connection_app_cancel_search(void);

bool bt_connection_search_have_update(void);
void bt_connection_search_set_update_flag(bool flag);
bool bt_connection_search_inqured(void);
void bt_connection_search_inqured_flag(bool flag);

bool bt_connection_get_status_from_pait_to_connect(void);
void bt_connection_set_status_from_pait_to_connect(bool flag);


bool bt_connection_app_get_disconnection_result(void);
void bt_connection_app_set_disconnection_result_flag(bool flag);

void bt_connection_app_init();
void bt_connection_data_init();
void bt_connection_search_data_init();

bool bt_connection_app_get_connection_result(void);
void bt_connection_app_set_connection_result_flag(bool flag);


void bt_connection_add_search_list(bt_gap_inquiry_ind_t *gap_info);

#ifdef __cplusplus
}
#endif

#endif/*__BT_CONNECTION_APP_H__*/

