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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "httpclient.h"
#include "wepodownload.h"
#include "gnss_log.h"
#define HTTPC_CLIENT_QUEUE_SIZE 5
#define HTTPC_CLIENT_MAX_THREAD_NUM 2
#define HTTPC_CLIENT_STACK_SIZE 1000
#define HTTPC_DOWNLOAD_TASK_PRIORITY 2

typedef enum httpc_download_message_id {
    HTTPC_START_DOWNLOAD,
    HTTPC_CANCLE_DOWNLOAD,
    HTTPC_SEND_REQ,
    HTTPC_SEND_RECV_DATA,
} httpc_download_message_id;

typedef struct gnss_epo_message_struct {
    int message_id;
    int param1;
    void* param2;
} httpc_download_message_struct_t;

struct {
	int32_t thread_index;
    QueueHandle_t httpc_download_queue[HTTPC_CLIENT_MAX_THREAD_NUM];
	httpclient_t client[HTTPC_CLIENT_MAX_THREAD_NUM];
	httpclient_data_t response[HTTPC_CLIENT_MAX_THREAD_NUM];
} http_download_cntx;


void httpc_save_data(epo_http_hdlr_struct *connection)
{
	char *ptr_etag = NULL, *body;
	int32_t written_len;
    
    GNSSLOGE("[EPO] httpc_save_data, sum:%d, len:%d\n", connection->sum, connection->recv_buf_len);
	if (connection->is_head_recieving == pdTRUE)
	{
	    connection->recv_buf[connection->recv_buf_len] = 0;
	    if ((body = strstr((char*) connection->recv_buf, "\r\n\r\n"))) {
            ptr_etag = strstr((char*) connection->recv_buf, "ETag");
            GNSSLOGE("[EPO] header recieved done.");
            if (ptr_etag) {
                GNSSLOGE("[EPO] check sum detected.");
                while (*ptr_etag && *ptr_etag != '\"') ptr_etag++;
                ptr_etag++;
                
                strncpy(
                    (char*) connection->check_sum,
                    (char*) ptr_etag,
                    sizeof(connection->check_sum)-1);
                connection->check_sum[sizeof(connection->check_sum) - 1] = 0;
            }
            
            if (connection->epo_file_handle == -1) {
#ifdef FAT_FS
                connection->epo_file_handle = epo_file_open(connection->file_type, EPO_DEFAULT, FS_CREATE_ALWAYS);
#else
                connection->epo_file_handle = epo_file_open(connection->file_type, epo_downlaod_get_curr_download_index(), FS_CREATE_ALWAYS);
#endif
            }
            body += 4;
            connection->sum = connection->recv_buf_len - (body - (char*)connection->recv_buf);
            FS_Write(connection->epo_file_handle, (kal_int8*) body, connection->sum, (kal_int32*) &written_len);
            connection->recv_buf[0] = 0;
            connection->is_head_recieving = pdFALSE;
        }	else {
            GNSSLOGE("[EPO] curr header:%s\n", connection->recv_buf);
        }
	} else {
        if (connection->epo_file_handle == -1) {
#ifdef FAT_FS
            connection->epo_file_handle = epo_file_open(connection->file_type, EPO_DEFAULT, FS_CREATE_ALWAYS);
#else
            connection->epo_file_handle = epo_file_open(connection->file_type, epo_downlaod_get_curr_download_index(), FS_CREATE_ALWAYS);
#endif
        }
        FS_Write(connection->epo_file_handle, connection->recv_buf, connection->recv_buf_len, (kal_int32*) &written_len);
        connection->sum += connection->recv_buf_len;
        connection->recv_buf[0] = 0;
    }
}

void httpc_download_task_msg_handler(httpc_download_message_struct_t *message, int32_t thread_index)
{
    int32_t ret;
	epo_http_hdlr_struct *connection = (epo_http_hdlr_struct*) message->param2;
    GNSSLOGE("[EPO] msg hdlr, message id:%d, connection:0x%x\n", message->message_id, connection);
    vTaskDelay(50);
	switch (message->message_id) {
		case HTTPC_START_DOWNLOAD:
            GNSSLOGE("[EPO] msg hdlr, URL:%s\n", connection->recv_buf);
			if (!(ret = httpclient_connect(&http_download_cntx.client[thread_index], (char*) connection->recv_buf))) {
				message->message_id = HTTPC_SEND_REQ;
				xQueueSend(http_download_cntx.httpc_download_queue[thread_index], message, 0);
			} else {
			    GNSSLOGE("[EPO] connect fail, ret:%d\r\n", ret);
            }
			break;
		case HTTPC_SEND_REQ:
			
			http_download_cntx.response[thread_index].response_buf = (char*) connection->recv_buf;
			http_download_cntx.response[thread_index].response_buf_len = sizeof(connection->recv_buf);
            http_download_cntx.response[thread_index].header_buf = (char*) connection->header_buf;
            http_download_cntx.response[thread_index].header_buf_len = sizeof(connection->header_buf);
            connection->is_head_recieving = pdFALSE;
            connection->sum = 0;
			if (!(ret = httpclient_send_request(&http_download_cntx.client[thread_index], (char*) connection->recv_buf, HTTPCLIENT_GET, &http_download_cntx.response[thread_index]))) {
                http_download_cntx.response[thread_index].response_buf[0] = 0;
                message->message_id = HTTPC_SEND_RECV_DATA;
				xQueueSend(http_download_cntx.httpc_download_queue[thread_index], message, 0);
            } else {
			    GNSSLOGE("[EPO] send request fail, ret:%d\r\n", ret);
            }
			break;
		case HTTPC_SEND_RECV_DATA:
			http_download_cntx.response[thread_index].response_buf = (char*) connection->recv_buf + strlen((char*) connection->recv_buf);
			http_download_cntx.response[thread_index].response_buf_len = sizeof(connection->recv_buf) - strlen((char*) connection->recv_buf);
			if (HTTPCLIENT_RETRIEVE_MORE_DATA == httpclient_recv_response(&http_download_cntx.client[thread_index], &http_download_cntx.response[thread_index])) {
				//process data
				GNSSLOGE("[EPO] recv data, content len:%d, left len:%d\r\n", 
				        http_download_cntx.response[thread_index].response_content_len,
				        http_download_cntx.response[thread_index].retrieve_len);
				connection->recv_buf_len = http_download_cntx.response[thread_index].response_content_len - 
				                            http_download_cntx.response[thread_index].retrieve_len 
				                            - connection->sum;
				httpc_save_data(connection);
				message->message_id = HTTPC_SEND_RECV_DATA;
				xQueueSend(http_download_cntx.httpc_download_queue[thread_index], message, 0);
			} else {
			    int32_t pos, len;
                char* str;
				GNSSLOGE("[EPO] recv data, content len:%d, left len:%d\r\n", 
				        http_download_cntx.response[thread_index].response_content_len,
				        http_download_cntx.response[thread_index].retrieve_len);
				connection->recv_buf_len = http_download_cntx.response[thread_index].response_content_len - 
				                            http_download_cntx.response[thread_index].retrieve_len 
				                            - connection->sum;
				httpc_save_data(connection);
                httpclient_get_response_header_value((char*) connection->header_buf, "ETag", (int*) &pos, (int*) &len);
                str = (char*) connection->header_buf + pos;
                str = strstr((char*) str, "\"") + 1;
                strncpy((char*) connection->check_sum, (char*) str, 32);
                connection->check_sum[32] = 0;
                GNSSLOGE("check sum:%s", connection->check_sum);
				httpclient_close(&http_download_cntx.client[thread_index]);
				// notify gps task
				epo_download_current_trunk_finish_notify(connection);
			}
			break;
		case HTTPC_CANCLE_DOWNLOAD:
			httpclient_close(&http_download_cntx.client[thread_index]);
			break;
	}
}

void httpc_download_task_main(void *args)
{
	httpc_download_message_struct_t queue_item;
	int32_t curr_thread = (int32_t) args;
    while (1) {
        if (xQueueReceive(http_download_cntx.httpc_download_queue[curr_thread], &queue_item, portMAX_DELAY)) {
            httpc_download_task_msg_handler(&queue_item, curr_thread);
        }
    }
}

void httpc_download_init()
{
    TaskHandle_t task_handler;
	int32_t thread_index = 0;
	
	while(thread_index < HTTPC_CLIENT_MAX_THREAD_NUM) {
		http_download_cntx.httpc_download_queue[thread_index] = xQueueCreate(HTTPC_CLIENT_QUEUE_SIZE , sizeof( httpc_download_message_struct_t ) );		
	    xTaskCreate( (TaskFunction_t) httpc_download_task_main, "HTTPCD", HTTPC_CLIENT_STACK_SIZE, (void *) thread_index, HTTPC_DOWNLOAD_TASK_PRIORITY, &task_handler );
        thread_index++;
	}
}

void httpc_download_start_download(int32_t thread_index, epo_http_hdlr_struct *connection, int8_t* URL)
{
	httpc_download_message_struct_t queue_item;
	if (thread_index >= HTTPC_CLIENT_MAX_THREAD_NUM)
	{
		GNSSLOGE("exceed max thread num\n");
		return ;
	}
    strcpy((char*) connection->recv_buf, (char*) URL);
	queue_item.message_id = HTTPC_START_DOWNLOAD;
    queue_item.param2 = (void*) connection;
    xQueueSend(http_download_cntx.httpc_download_queue[thread_index], &queue_item, 0);
}

void httpc_download_cancle_download(epo_http_hdlr_struct *connection)
{
	httpc_download_message_struct_t queue_item;
	if (connection->file_type >= HTTPC_CLIENT_MAX_THREAD_NUM)
	{
		GNSSLOGE("exceed max thread num\n");
		return ;
	}
	queue_item.message_id = HTTPC_CANCLE_DOWNLOAD;
	queue_item.param2 = connection;
    xQueueSend(http_download_cntx.httpc_download_queue[connection->file_type], &queue_item, 0);
}

