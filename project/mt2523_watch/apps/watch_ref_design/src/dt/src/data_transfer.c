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

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "data_transfer.h"
#include "nvdm.h"

#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_system.h"
#include "bt_spp.h"
#include "bt_callback_manager.h"
#include "bt_notify.h"
#include "bt_notify_app_list.h"

#include "task_def.h"


#include "stdbool.h"
#include "stdlib.h"


#include "syslog.h"
#include "sensor_transfer_info.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_DT_UNITE_SIZE 10
#define MAX_HEADER_SIZE 80

#define SENDER_HEALTH      			"health_sender"
#define RECEIVER_HEALTH 			"health_receiver"
#define BP_HANDSHAKE_CMD			"bp_handshake"
#define HR_HANDSHAKE_CMD			"hr_handshake"
#define CB_HANDSHAKE_CMD			"calibration"
#define BLOOD_PRESSURE_EXTEND_CMD   "EXCD"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

typedef struct DataTransferUnite {
	uint8_t* dt_buffer;
	int16_t dt_offset_send_size;
	int16_t dt_unite_size;
}DataTransferUnite;

typedef struct DataPacakgeUnite {
	uint8_t* dt_package_buffer;
	int16_t dt_package_size;
}DataPacakgeUnite;

typedef SemaphoreHandle_t dt_mutex_t;

typedef dt_status_t (* dt_send_cb)(uint8_t* buffer, uint32_t buffer_length);


typedef struct DataTransferManagement {
	DataPacakgeUnite g_package[MAX_DT_UNITE_SIZE];
	DataTransferUnite g_dt_data;
	DataTransferUnite g_dt_result_data;
	char notify_header[MAX_HEADER_SIZE];
	char notify_update_header[MAX_HEADER_SIZE];
	int count;
	bool isLeftUniteData;
	dt_recv_cb g_dt_recv_cb;
	dt_send_cb dt_send;
	bt_bd_addr_t g_bt_addr;
	bool isBTConnect;
	bool isBPCapability;
	bool isHRCapability;
	bool isCalibrationCapability;
	bool transferStartFlag;
	bt_notify_data_source_t dt_type;
	dt_mutex_t dt_send_semaphore;
}DataTransferManagement;


DataTransferManagement g_dataTransferNode;


static uint32_t mmi_blood_pressure_little_endian_to_32(const uint8_t *ptr) 
{
	return (uint32_t)(((uint32_t)*(ptr + 3) << 24) |
					((uint32_t)*(ptr + 2) <<16) |
					((uint32_t)*(ptr + 1)) << 8 |
					((uint32_t)*ptr));
}


void dt_clear()
{
	int index = 0;
	
	LOG_I(bmt_demo, "[dt]dt dt_clear data completely");

	if (BT_NOTIFY_DATA_SOURCE_SPP == g_dataTransferNode.dt_type)
	{
		for(index = 0; index < MAX_DT_UNITE_SIZE; ++index)
		{
			if(NULL != g_dataTransferNode.g_package[index].dt_package_buffer)
			{
				vPortFree(g_dataTransferNode.g_package[index].dt_package_buffer);
				g_dataTransferNode.g_package[index].dt_package_buffer = NULL;
				g_dataTransferNode.g_package[index].dt_package_size = 0;
			}		
		}
		g_dataTransferNode.count = 0;
	}
	vPortFree(g_dataTransferNode.g_dt_data.dt_buffer);
	g_dataTransferNode.g_dt_data.dt_buffer = NULL;
	g_dataTransferNode.g_dt_data.dt_offset_send_size = 0;
	g_dataTransferNode.g_dt_data.dt_unite_size = 0;
	g_dataTransferNode.isLeftUniteData = false;	
}


void dt_result_clear()
{
	LOG_I(bmt_demo, "[dt]dt dt_clear data completely");
	if(NULL != g_dataTransferNode.g_dt_result_data.dt_buffer)
	{
		vPortFree(g_dataTransferNode.g_dt_result_data.dt_buffer);
	}
	
	g_dataTransferNode.g_dt_result_data.dt_buffer = NULL;
	g_dataTransferNode.g_dt_result_data.dt_offset_send_size = 0;
	g_dataTransferNode.g_dt_result_data.dt_unite_size = 0;
	//g_dataTransferNode.isLeftUniteData = false;	
}


dt_status_t sensor_bt_recv_cb(const char * receiver_id, uint8_t *data, uint16_t data_len)
{	
	nvram_ef_bp_info_struct bp_info = {0};
	uint8_t tag_len = 0;
	uint8_t parsed_len = 0;

	LOG_I(bmt_demo, "[dt]sensor_bt_recv_cb  %s", data);
	
    if (!strcmp(receiver_id, RECEIVER_HEALTH)) {
		// capability parsing
		if (!strcmp((const char *)data, BP_HANDSHAKE_CMD)) {
			LOG_I(bmt_demo, "[dt]sensor_bt_recv_cb  BP_HANDSHAKE_CMD");
			//handshake
			char rsp[100] = {0};			
			sprintf(rsp, "%s %s 0 %d bp_handshake", SENDER_HEALTH, RECEIVER_HEALTH, strlen("bp_handshake"));
			bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)rsp, strlen(rsp), true); 		
			g_dataTransferNode.isBPCapability = true;
		}else if(!strcmp((const char *)data, HR_HANDSHAKE_CMD)) {
		    LOG_I(bmt_demo, "[dt]sensor_bt_recv_cb  HR_HANDSHAKE_CMD");
			//handshake
			char rsp[100] = {0};			
			sprintf(rsp, "%s %s 0 %d hr_handshake", SENDER_HEALTH, RECEIVER_HEALTH, strlen("hr_handshake"));
            bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)rsp, strlen(rsp), true); 		    
			g_dataTransferNode.isHRCapability = true;
		}else if(!strcmp((const char *)data, CB_HANDSHAKE_CMD)) {
			LOG_I(bmt_demo, "[dt]sensor_bt_recv_cb  CB_HANDSHAKE_CMD");
			//handshake
			char calibration_rsp[100] = {0};			
			sprintf(calibration_rsp, "%s %s 0 %d calibration", SENDER_HEALTH, RECEIVER_HEALTH, strlen("calibration"));
            bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)calibration_rsp, strlen(calibration_rsp), true); 		    
			g_dataTransferNode.isCalibrationCapability = true;
		}else {
		
			uint32_t data_len = 0;
			uint32_t send_len = 0;
			uint32_t nvram_num = sizeof(nvram_ef_bp_info_struct);
			uint32_t bp_info_rsp[16] = {12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345, 12345};
			char notify_rsp[100] = {0};
			
			LOG_I(bmt_demo, "[dt]others");
			
			/*parse the received data*/
			memset(&bp_info, 0, sizeof(nvram_ef_bp_info_struct));

			/* read nvram first*/
			nvdm_status_t ret = nvdm_read_data_item("2511", "bpInfo", (uint8_t *)&bp_info, &nvram_num);

			LOG_I(bmt_demo, "[dt]nvdm_read_data_item  %d   %d", ret, nvram_num);		
			
			parsed_len += 5;

			while (parsed_len < data[0]) {
				
				bp_tag_id_t tag_id = (bp_tag_id_t)(data[parsed_len]);
				
				parsed_len++;
				tag_len = data[parsed_len];
				parsed_len++;

				switch(tag_id) {
					case MMI_BP_TAG_USER_NAME:
						{
							uint8_t max_len = tag_len > NVRAM_EF_BP_USERID_LEN ? NVRAM_EF_BP_USERID_LEN : tag_len;							
							memcpy(bp_info.userid, data + parsed_len, max_len);							
							parsed_len += tag_len;
							LOG_I(bmt_demo, "[dt_BP]userid = %s\n", bp_info.userid);
						}
						break;						
					case MMI_BP_TAG_HEIGHT:
						bp_info.height = mmi_blood_pressure_little_endian_to_32(data + parsed_len);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]height = %d\n", bp_info.height);
						break;
						
					case MMI_BP_TAG_WEIGHT:
						bp_info.weight = mmi_blood_pressure_little_endian_to_32(data + parsed_len);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]weight = %d\n", bp_info.weight);
						break;
						
					case MMI_BP_TAG_GENDER:
						bp_info.gender = mmi_blood_pressure_little_endian_to_32(data + parsed_len);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]gender = %d\n", bp_info.gender);
						break;
						
					case MMI_BP_TAG_AGE:
						bp_info.age = mmi_blood_pressure_little_endian_to_32(data + parsed_len);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]age = %d\n", bp_info.age);
						break;
						
					case MMI_BP_TAG_ARMLEN:
						bp_info.armlen = mmi_blood_pressure_little_endian_to_32(data + parsed_len);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]armlen = %d\n", bp_info.armlen);
						break;

					case MMI_BP_TAG_BP_MODE:
						bp_info.bp_mode = mmi_blood_pressure_little_endian_to_32(data + parsed_len);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]bp_mode = %d\n", bp_info.bp_mode);
						break;

					case MMI_BP_TAG_CALIBRATE_PARA:
						memcpy(bp_info.calibrate_para, data + parsed_len, 18 * 4);
						parsed_len += tag_len;
						LOG_I(bmt_demo, "[dt_BP]calibration data received, tag_len = %d\n", tag_len);
						break;
					}
				}

			
			LOG_I(bmt_demo, "[dt_BP]calibration total_len: %d  parse_len: %d",data[0], parsed_len );

			if (parsed_len == data[0])
			{
				nvdm_status_t ret = NVDM_STATUS_OK;
				//parse success
				bp_info_rsp[3] = 0; //non_zero means fail
				
				//write to NVRAM
				ret = nvdm_write_data_item("2511", "bpInfo", NVDM_DATA_ITEM_TYPE_RAW_DATA, (const uint8_t *)&bp_info, sizeof(bp_info));
					
				LOG_I(bmt_demo, "[dt_BP]calibration data received, ret = %d\n", ret);				

				
				// send response to APK
				bp_info_rsp[0] = 54321;
				bp_info_rsp[1] = 3001;
				sprintf(notify_rsp, "%s %s 0 %d ", SENDER_HEALTH, RECEIVER_HEALTH, 64);
				data_len = strlen(notify_rsp);
				memcpy(notify_rsp + data_len, bp_info_rsp, 64);
				data_len += 64;
				
				send_len = bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)notify_rsp, data_len, true);
				LOG_I(bmt_demo, "[dt_BP]calibration data received, send_len = %d\n", send_len);
			}
			
		}
      }
	return DT_STATUS_OK;
}


/**
 * @brief          This function is for recieve the event of the notify service.
 * @param[in]  data      is the data about the event.
 * @return       void.
 */
void bt_notify_dt_callback_func(void *data)
{    
    bt_notify_callback_data_t *p_data = (bt_notify_callback_data_t *)data;
    
    LOG_I(bmt_demo, "[dt]callback evt_id = %d!\r\n", p_data->evt_id);
    switch (p_data->evt_id) {
        case BT_NOTIFY_EVENT_CONNECTION: {         
			memcpy(&g_dataTransferNode.g_bt_addr, &p_data->bt_addr, sizeof(bt_bd_addr_t));
			g_dataTransferNode.dt_type = bt_notify_get_data_source(&g_dataTransferNode.g_bt_addr);
			//g_dataTransferNode.dt_type = BT_NOTIFY_DATA_SOURCE_SPP;
			LOG_I(bmt_demo, "dt conntection recvd  %d", g_dataTransferNode.dt_type);
			g_dataTransferNode.isBTConnect = true;			
            dt_clear();
        }
        break;
        case BT_NOTIFY_EVENT_DISCONNECTION: {            
            /*disconnected with the remote device*/
            //bt_notify_app_disconnect();
			memset(&g_dataTransferNode.g_bt_addr, 0, sizeof(bt_bd_addr_t));
			g_dataTransferNode.isBTConnect = false;
			g_dataTransferNode.isBPCapability = false;
			g_dataTransferNode.isHRCapability = false;
			g_dataTransferNode.transferStartFlag = false;
			g_dataTransferNode.dt_type = BT_NOTIFY_DATA_SOURCE_INVALID;
			dt_clear();
        }
        break;           
		case BT_NOTIFY_EVENT_SEND_IND: {
			/*send  new/the rest data flow start*/            
            int32_t send_size;
			bool new_pacakge = false;
			int16_t left_size = g_dataTransferNode.g_dt_data.dt_unite_size - g_dataTransferNode.g_dt_data.dt_offset_send_size;
			LOG_I(bmt_demo, "[dt]2dt_send BT_NOTIFY_EVENT_SEND_IND size: %d", left_size);
			if(0 >= left_size)
			{
				LOG_I(bmt_demo, "[dt]2dt_send BT_NOTIFY_EVENT_SEND_IND abnormal");
			}
			if(0 == g_dataTransferNode.g_dt_data.dt_offset_send_size)
			{
				LOG_I(bmt_demo, "[dt]2dt_send BT_NOTIFY_EVENT_SEND_IND new package");
				new_pacakge = true;
			}
			send_size = bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)(g_dataTransferNode.g_dt_data.dt_buffer + g_dataTransferNode.g_dt_data.dt_offset_send_size), left_size, new_pacakge);           
			// left size can't send this time.
			if(send_size < left_size)
			{
				g_dataTransferNode.g_dt_data.dt_offset_send_size += send_size;
				g_dataTransferNode.isLeftUniteData = true;
				LOG_I(bmt_demo, "[dt]2dt_send continue success but size incomplete  %d  %d", left_size, send_size);
			}else {				
				LOG_I(bmt_demo, "[dt]2dt_send continue completely");
				dt_clear();				
			}
        }
        break;
        case BT_NOTIFY_EVENT_DATA_RECEIVED: {
            bt_notify_event_data_t *p_notify_data = (bt_notify_event_data_t *)&(p_data->event_data);
            /*receive data*/
            LOG_I(bmt_demo, "receiverName = %s, data = %s, len = %d\r\n", p_notify_data->receiver_id, p_notify_data->data, p_notify_data->length);
            g_dataTransferNode.g_dt_recv_cb(p_notify_data->receiver_id, p_notify_data->data, p_notify_data->length);
        }
        break;
        case BT_NOTIFY_EVENT_NOTIFICATION: {            
           
        }
        break;
    	}
}


void dt_port_mutex_take(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        if (xSemaphoreTake(g_dataTransferNode.dt_send_semaphore, portMAX_DELAY) == pdFALSE) {
             LOG_I(bmt_demo, "dt_port_mutex_take error\r\n");
        }        
    }
}

void dt_port_mutex_give(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        if (xSemaphoreGive(g_dataTransferNode.dt_send_semaphore) == pdFALSE) {
            LOG_I(bmt_demo, "dt_port_mutex_give error\r\n");
        }        
    }
}


/** @brief  data transfer module initialize.
 *  @param none.
 *  @return    none.
 */
void dt_init()
{
	g_dataTransferNode.isBTConnect = false;
	g_dataTransferNode.isBPCapability = false;
	g_dataTransferNode.isHRCapability = false;
	g_dataTransferNode.transferStartFlag = false;
	g_dataTransferNode.dt_send_semaphore = xSemaphoreCreateMutex();

    if (g_dataTransferNode.dt_send_semaphore == NULL) {
        LOG_I(bmt_demo, "[dt]dt_init_port_mutex_creat error\r\n");
		return;
    }

	dt_clear();
	
	dt_resister_recv_callback(sensor_bt_recv_cb);
	
	if (BT_NOTIFY_RESULT_REGISTER_OK !=  bt_notify_register_callback(NULL, SENDER_HEALTH, bt_notify_dt_callback_func)) {
        return;
    }
}



/** @brief  This defines the data transfer module to send data from BT.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully send.
			   #DT_STATUS_FAILUE the data send fail. Should retry later\n
 */
dt_status_t dt_send_data(uint8_t* buffer, uint32_t buffer_length)
{
	int32_t send_size = -1;
	int32_t temp_size = 0;
	uint8_t* ptr_buffer;
	int index = 0;
	uint8_t *temp_buffer = NULL;	

	if( (false == g_dataTransferNode.isBTConnect) || (false == g_dataTransferNode.isBPCapability) || (false == g_dataTransferNode.isHRCapability) )
	{
		//LOG_I(bmt_demo, "dt_send_data has no connection");
		return DT_STATUS_DISCONNECT;
	}

	if(BT_NOTIFY_DATA_SOURCE_SPP != g_dataTransferNode.dt_type)
	{
		return DT_FAIL_ERROR_TYPE;
	}

	if(false == g_dataTransferNode.transferStartFlag)
	{
		return DT_CANNOT_TRANSFER;
	}

	dt_port_mutex_take();
	
	// can't send data because previous hasn't send completely.
	if(true == g_dataTransferNode.isLeftUniteData)
	{
		LOG_I(bmt_demo, "[dt]can't send data because previous hasn't send completely");
		dt_port_mutex_give();
		return DT_PREVIOUS_DATA_INCOMPLETE;
 	}

	ptr_buffer = (uint8_t *)pvPortMalloc(buffer_length);
	if(NULL == ptr_buffer)
	{
		LOG_I(bmt_demo, "[dt]dt_send_allocate failure");
		dt_port_mutex_give();
		return DT_SEND_BUFFER_ALLOCATE_FAILURE;
	}

	memcpy(ptr_buffer, buffer, buffer_length);
	
	g_dataTransferNode.g_package[g_dataTransferNode.count].dt_package_buffer = ptr_buffer;
	g_dataTransferNode.g_package[g_dataTransferNode.count].dt_package_size = buffer_length;	
	
	g_dataTransferNode.g_dt_data.dt_unite_size += g_dataTransferNode.g_package[g_dataTransferNode.count].dt_package_size;	
	
	++g_dataTransferNode.count;
	
	if( MAX_DT_UNITE_SIZE == g_dataTransferNode.count )
	{
		
		
		LOG_I(bmt_demo, "[dt]dt_send ll size %d", g_dataTransferNode.g_dt_data.dt_unite_size);
		
		for(index = 0; index < MAX_DT_UNITE_SIZE; ++index)
		{
			if(0 == index)
			{				
				snprintf(g_dataTransferNode.notify_header, MAX_HEADER_SIZE, "%s %s 0 %d ", SENDER_HEALTH, RECEIVER_HEALTH, g_dataTransferNode.g_dt_data.dt_unite_size);
				temp_size = strlen((char*)g_dataTransferNode.notify_header);
				g_dataTransferNode.g_dt_data.dt_unite_size += temp_size;
				g_dataTransferNode.g_dt_data.dt_buffer = (uint8_t *)pvPortMalloc(g_dataTransferNode.g_dt_data.dt_unite_size);
				if(NULL == g_dataTransferNode.g_dt_data.dt_buffer)
				{
					LOG_I(bmt_demo, "[dt]dt_send_g_dt_data allocate failure");
					dt_port_mutex_give();
					return DT_SEND_BUFFER_ALLOCATE_FAILURE;
				}
				temp_buffer = g_dataTransferNode.g_dt_data.dt_buffer;
				memcpy(temp_buffer, g_dataTransferNode.notify_header, temp_size);
				temp_buffer += temp_size;
	 		}					
			memcpy(temp_buffer, g_dataTransferNode.g_package[index].dt_package_buffer, g_dataTransferNode.g_package[index].dt_package_size);
			temp_buffer += g_dataTransferNode.g_package[index].dt_package_size;
		}

		LOG_I(bmt_demo, "[dt]dt_send dt_unite_size %d - %d", g_dataTransferNode.g_dt_data.dt_unite_size, temp_size);

		send_size = bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)(g_dataTransferNode.g_dt_data.dt_buffer), (uint16_t)g_dataTransferNode.g_dt_data.dt_unite_size, true);           


		LOG_I(bmt_demo, "[dt]dddddddddd dt_send dt_unite_size %d - %d", g_dataTransferNode.g_dt_data.dt_unite_size, temp_size);
		 
		if(0 > send_size)
		{
			LOG_I(bmt_demo, "[dt]dt_send fail: %d", send_size);
			dt_port_mutex_give();
			return DT_STATUS_FAILUE;
		}
		// left size can't send this time.
		if(send_size < g_dataTransferNode.g_dt_data.dt_unite_size)
		{
			g_dataTransferNode.g_dt_data.dt_offset_send_size += send_size;
			g_dataTransferNode.isLeftUniteData = true;
			LOG_I(bmt_demo, "[dt]dt_send success but size incomplete main directly %d  %d", g_dataTransferNode.g_dt_data.dt_unite_size, send_size);
		}else {
			g_dataTransferNode.isLeftUniteData = false;
			dt_clear();
		}	
	}

	dt_port_mutex_give();
	
	return DT_STATUS_OK;
}



/** @brief  This defines the data transfer module to send data from BT.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully send.
			   #DT_STATUS_FAILUE the data send fail. Should retry later\n
 */
dt_status_t dt_update_data(uint8_t* buffer, uint32_t buffer_length)
{
	int32_t send_size = -1;
	int32_t temp_size = 0;
	uint8_t *temp_buffer = NULL;

	if( (false == g_dataTransferNode.isBTConnect) || (false == g_dataTransferNode.isBPCapability) || (false == g_dataTransferNode.isHRCapability) )
	{
		//LOG_I(bmt_demo, "dt_send_data has no connection");
		return DT_STATUS_DISCONNECT;
	}

	if(false == g_dataTransferNode.transferStartFlag)
	{
		return DT_CANNOT_TRANSFER;
	}

	
	dt_port_mutex_take();
	
	// can't send data because previous hasn't send completely.
	if(true == g_dataTransferNode.isLeftUniteData)
	{
		LOG_I(bmt_demo, "[dt_update]can't send data because previous hasn't send completely");
		dt_port_mutex_give();
		return DT_PREVIOUS_DATA_INCOMPLETE;
 	}

	snprintf(g_dataTransferNode.notify_update_header, MAX_HEADER_SIZE, "%s %s 0 %u ", SENDER_HEALTH, RECEIVER_HEALTH, (unsigned int)buffer_length);
	temp_size = strlen(g_dataTransferNode.notify_update_header);
	g_dataTransferNode.g_dt_result_data.dt_unite_size = temp_size + buffer_length;
	g_dataTransferNode.g_dt_result_data.dt_buffer = (uint8_t *)pvPortMalloc(g_dataTransferNode.g_dt_result_data.dt_unite_size);
	if(NULL == g_dataTransferNode.g_dt_result_data.dt_buffer)
	{
			LOG_I(bmt_demo, "[dt]dt_send_g_dt_data allocate failure");
			dt_port_mutex_give();
			return DT_SEND_BUFFER_ALLOCATE_FAILURE;
	}
	temp_buffer = g_dataTransferNode.g_dt_result_data.dt_buffer;
	memcpy(temp_buffer, g_dataTransferNode.notify_update_header, temp_size);
	temp_buffer += temp_size;				
	memcpy(temp_buffer, buffer, buffer_length);
	
	
	LOG_I(bmt_demo, "[dt_update]dt_send dt_unite_size %d", g_dataTransferNode.g_dt_result_data.dt_unite_size);

	send_size = bt_notify_send_data(&g_dataTransferNode.g_bt_addr, (const char *)(g_dataTransferNode.g_dt_result_data.dt_buffer), (uint16_t)g_dataTransferNode.g_dt_result_data.dt_unite_size, true);           

	if(0 > send_size)
	{
		LOG_I(bmt_demo, "[dt_update]dt_send fail: %d", send_size);
		dt_port_mutex_give();
		return DT_STATUS_FAILUE;
	}
	
	dt_result_clear();
	
	dt_port_mutex_give();	
	
	return DT_STATUS_OK;
}


/** @brief  This defines the data transfer module to register recv callback from BT.
 *  @param [in] dt_recv_cb: it's data transfer reviver callback prototype.
 *  @return    none.
 */
void dt_resister_recv_callback(dt_recv_cb dt_recv_cb)
{
	g_dataTransferNode.g_dt_recv_cb = dt_recv_cb;
}

/** @brief  This defines the data transfer wethercan  start or not.
 *  @param [in] buffer: send buffer size.
 *         buffer_length: buffer length.
 *  @return    #DT_STATUS_OK the data successfully set.
			   #DT_STATUS_FAILUE the data send fail
 */
void dt_set_start_transfer_flag(bool startFlag)
{
	g_dataTransferNode.transferStartFlag = startFlag;
}

