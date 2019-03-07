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

#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

typedef enum {
    DT_STATUS_FAILUE = -10,              /**< An error occurred during the function call. */
	DT_STATUS_DISCONNECT,			     /**< An error occurred for no connection. */
	DT_PREVIOUS_DATA_INCOMPLETE,         /**< An error occurred for no connection. */
	DT_BUFFER_FULL,                      /**< An error occurred for no connection. */
	DT_SEND_BUFFER_ALLOCATE_FAILURE,     /**< An error occurred for no connection. */
	DT_FAIL_ERROR_TYPE,                  /**< An error occurred for no connection. */
	DT_CANNOT_TRANSFER,					 /**< An error occurred for no connection. */
    DT_STATUS_OK = 0,                    /**< No error occurred during the function call. */
} dt_status_t;

/** @brief  This defines the data transfer module to register recv callback from BT.
 *  @param [in] data: data module recv data. data_len: receive data length
 *  @return    none.
 */
typedef dt_status_t (* dt_recv_cb)(const char * receiver_id, uint8_t *data, uint16_t data_len);


/** @brief  This defines the data transfer module to send data from BT SPP only.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully send.
			   #DT_STATUS_FAILUE the data send fail. Should retry later\n
 */
dt_status_t dt_send_data(uint8_t* buffer, uint32_t buffer_length);


/** @brief  This defines the data transfer module to send data from BT BLE only.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully send.
			   #DT_STATUS_FAILUE the data send fail. Should retry later\n
 */
dt_status_t dt_update_data(uint8_t* buffer, uint32_t buffer_length);


/** @brief  This defines the data transfer module to register recv callback from BT.
 *  @param [in] dt_recv_cb: it's data transfer reviver callback prototype.
 *  @return    none.
 */
void dt_resister_recv_callback(dt_recv_cb dt_recv_cb);


/** @brief  This defines the data transfer wethercan  start or not.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully set.
			   #DT_STATUS_FAILUE the data send fail
 */
void dt_set_start_transfer_flag(bool startFlag);


/** @brief  This definesreturn  the data transfer wether can start or not.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully set.
			   #DT_STATUS_FAILUE the data send fail
 */
bool dt_get_start_transfer_flag();


/** @brief  data transfer module initialize.
 *  @param none.
 *  @return    none.
 */
void dt_init();

#ifdef __cplusplus
}
#endif


#endif /* DATA_TRANSFER_H */
