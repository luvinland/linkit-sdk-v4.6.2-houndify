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

#ifndef __GPRS_API_H__
#define __GPRS_API_H__

#include "main_screen.h"

#define GPRS_SUPPORT_SLEEP_MANAGER

#define GPRS_SUPPORT_APN_MAX_LENGTH     10
#define GPRS_SUPPORT_APN_NUM             3

typedef enum {
    GPRS_RET_OK = 0,                 /**< No error occurred during the function call. */        
    GPRS_RET_ERROR = -1,             /**< Error occurred during the function call. */
    GPRS_RET_REGISTER_FAIL = -2,     /**< Fail to register command handler table. */    
    GPRS_RET_UNREGISTER_FAIL = -3,   /**< Fail to unregister command handler table. */
    GPRS_RET_BUZY = -4,              
    GPRS_RET_MODEM_NOT_READY = -5,
    GPRS_RET_NOT_ALLOWED = -6,		/* active/deactive result hasn't been returned. */
    GPRS_RET_INVALID_PARAM = -7,
    GPRS_RET_NOT_ENOUGH_MEMORY = -8,
    GPRS_RET_WOULDBLOCK = -9,		/* Wait for callback */
    GPRS_RET_UNKNOWN = -10,
    
    GPRS_RET_END = -100              /**< The end enum item. */
}gprs_ret_t;

typedef enum
{
	GPRS_NOTI_TYPE_NONE = 0x0,				/* No Noti type is selected. Useful when this enum variables are initialized to zero. */
	GPRS_NOTI_TYPE_MODEM_READY = 0x0001,
	GPRS_NOTI_TYPE_MODEM_EXCEPTION = 0x0002,
	GPRS_NOTI_TYPE_GPRS_PASSIVE_DEACTIVE = 0x0004,	/* Passive deactive */
    GPRS_NOTI_TYPE_CMUX_CONN = 0x0010,
    GPRS_NOTI_TYPE_CMUX_DISCONN = 0x0020,
	GPRS_NOTI_TYPE_ALL = 0xFFFF
}gprs_noti_type_enum;

typedef enum {
    GPRS_PDP_CONTEXT_ID_1 = 1,
    GPRS_PDP_CONTEXT_ID_2 = 2,
    GPRS_PDP_CONTEXT_ID_3 = 3,
    GPRS_PDP_CONTEXT_ID_MAX
}gprs_pdp_context_id_enum;

typedef enum {
    GPRS_ACTION_TYPE_NONE   = 0,
    GPRS_ACTION_TYPE_INITIALIZE   = 1,
    GPRS_ACTION_TYPE_REGISTRATION = 2,
    GPRS_ACTION_TYPE_ACTIVATE   = 3,
    GPRS_ACTION_TYPE_DEACTIVATE = 4,

    GPRS_ACTION_TYPE_MAX
} gprs_action_type_enum;


/* res: GPRS_RET_OK, activate/deactivate GPRS successfully.
  *       Otherwise, failed to activate/deactivate GPRS.
  */
typedef void (* gprs_action_cb)(gprs_ret_t res, gprs_action_type_enum action);

extern void gprs_cntx_init(void);

extern void gprs_set_callback(gprs_action_cb callback);

/* Return GPRS_RET_OK or GPRS_RET_MODEM_NOT_READY */
extern gprs_ret_t gprs_is_modem_ready(void);

/* Return GPRS_RET_OK or GPRS_RET_MODEM_ERROR */
extern gprs_ret_t gprs_is_data_activated(void);

/* Return GPRS_RET_OK or GPRS_RET_MODEM_ERROR */
extern gprs_ret_t gprs_is_modem_initialized(void);

/* Return GPRS_RET_OK or GPRS_RET_MODEM_ERROR */
extern gprs_ret_t gprs_is_network_registered(void);

/* Can't call this API again before the result of previous invokation is returned. 
  * param:
  *    cid [in]  modem pdp context id.
  * Return: OK, done successfully.
  *            WOULDBLOCK, wait for callback to get the result.
  *            Otherwise, failed.
  */	
extern gprs_ret_t gprs_initialize_modem();


/* Can't call this API again before the result of previous invokation is returned. 
  * param:
  *    cid [in]  modem pdp context id.
  * Return: OK, done successfully.
  *            WOULDBLOCK, wait for callback to get the result.
  *            Otherwise, failed.
  */	
extern gprs_ret_t gprs_registration_network();

/* Can't call this API again before the result of previous invokation is returned. 
  * param:
  *    cid [in]  modem pdp context id.
  * Return: OK, done successfully.
  *            WOULDBLOCK, wait for callback to get the result.
  *            Otherwise, failed.
  */	
extern gprs_ret_t gprs_activate_data_with_cid(gprs_pdp_context_id_enum cid);

/* Can't call this API again before the result of previous invokation is returned. 
  * Return: OK, done successfully.
  *            WOULDBLOCK, wait for callback to get the result.
  *            Otherwise, failed.
  */
extern gprs_ret_t gprs_deactivate_data_with_cid(gprs_pdp_context_id_enum cid);

extern void gprs_atcmd_sent_cb_hdl(void);
extern void gprs_noti_hdl(int32_t event);
extern int32_t gprs_send_callback(uint8_t *payload, uint32_t length, void *user_data);
extern uint32_t gprs_get_app_id();
#ifdef GPRS_SUPPORT_SLEEP_MANAGER
extern gprs_ret_t gprs_set_host_wakeup();
extern gprs_ret_t gprs_set_host_sleep();
#endif
 
extern uint8_t gprs_get_cid_by_apn(uint8_t* apn);
#endif /* __GPRS_API_H__ */
