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

/*****************************************************************************
 *
 * Description:
 * ------------
 * The file is used for testing BT notification..
 *
 ****************************************************************************/

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
#include "bt_notify_app.h"
#include "bt_notify_app_list.h"
#include "ble_ancs_app.h"

#include "syslog.h"
#include "sensor_demo.h"

/*****************************************************************************
 * define
 *****************************************************************************/

/*****************************************************************************
 * typedef
 *****************************************************************************/

#define DEVICE_NAME "watch_demo_device"
bt_gap_config_t bt_custom_config;


const bt_gap_config_t* bt_gap_get_local_configuration_test(void)
{


    LOG_I(NOTIFY_APP,"bt_get_local_configuration1\n");
    bt_custom_config.inquiry_mode = 3;
    bt_custom_config.io_capability = BT_GAP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT;
    bt_custom_config.cod = 0x080704;//0x240404
    memcpy(&bt_custom_config.device_name, DEVICE_NAME, sizeof(DEVICE_NAME));


    LOG_I(NOTIFY_APP,"1:%08x\r\n", (unsigned int)DEVICE_NAME);
    return  &bt_custom_config;
}

bt_status_t bt_notify_app_event_callback(
    bt_msg_type_t msg,
    bt_status_t status,
    void *buff)

{

    LOG_I(NOTIFY_APP,"event:0x%x, %x\n", msg, status);
    switch (msg) {
        case BT_GAP_SET_SCAN_MODE_CNF:/*109*/
            break;

#if 0
        case BT_GAP_IO_CAPABILITY_REQ_IND:/*103*/
           bt_gap_reply_io_capability_request(BT_GAP_OOB_DATA_PRESENTED_NONE,
                BT_GAP_SECURITY_AUTH_REQUEST_GENERAL_BONDING_AUTO_ACCEPTED);
            break;
#endif
        case BT_GAP_USER_CONFIRM_REQ_IND:/*104*/
            bt_gap_reply_user_confirm_request(true);
            break;

        case BT_POWER_ON_CNF:  /*24000001*/
            LOG_I(NOTIFY_APP,"bt power on confirm.\n");
            bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
            break;

        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}

/**
 * @brief          This function is for app init implement.
 * @param[in]  void.
 * @return       void.
 */

void bt_notify_app_init(void)
{
    bt_notify_app_list_init();

    bt_notify_init(BT_SPP_SERVER_ID_START);
    ble_ancs_app_init();

    if (BT_NOTIFY_RESULT_REGISTER_OK !=  bt_notify_register_callback(NULL, "app_test", bt_notify_callback_func)) {
        return;
    }
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM, (void*)bt_notify_app_event_callback);
    bt_callback_manager_register_callback(bt_callback_type_gap_get_local_configuration, 0, (void*)bt_gap_get_local_configuration_test);
}


