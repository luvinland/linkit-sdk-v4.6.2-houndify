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
#include "nw_atci.h"
#include "nw_gprs.h"
#include "nvdm.h"
#include "syslog.h"
#include "sio_gprot.h"
#include "string.h"
#include "stdio.h"
#include "task.h"
#include "fota_demo.h"
#include "task_def.h"
#include "atci.h"

extern char *gprs_get_local_ip(void);

//#define NW_ATCI_PRINTF

#ifdef NW_ATCI_PRINTF
#define LOGE(fmt,arg...)   printf(("[NW ATCI]: "fmt), ##arg)
#define LOGW(fmt,arg...)   printf(("[NW ATCI]: "fmt), ##arg)
#define LOGI(fmt,arg...)   printf(("[NW ATCI]: "fmt), ##arg)
#else
log_create_module(nw_atci, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(nw_atci, "[NW ATCI]: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(nw_atci, "[NW ATCI]: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(nw_atci ,"[NW ATCI]: "fmt,##arg)
#endif

#define NW_ATCI_PING_COUNT		     		(4)		// The number of ping requests sent
#define NW_ATCI_PING_TIMEOUT				(10)	// in second
#define NW_ATCI_PING_SIZE					(32)
#define NW_ATCI_PING_REMOTE_IP_DEFAULT  	("180.97.33.108")		// www.baidu.com


void nw_atci_print_result(char* result)
{     
    atci_response_t response;
    
    LOGI("enter: %s", result);
    memset(&response, 0, sizeof(atci_response_t));
    strcpy((char *)response.response_buf, result);
    response.response_len = strlen((char *)response.response_buf);
    response.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;   
    atci_send_response(&response);
}

void nw_atci_ping_cb(ping_result_t *result)
{
    char* context = NULL;
    int context_len = 100;

    context = pvPortMalloc(context_len + 1);
        
    if (result) {
        if (result->recv_num) {
            LOGI("ping success: total %d, lost %d, receive %d, avg_time: %d", (int)result->total_num, (int)result->lost_num, (int)result->recv_num, (int)result->avg_time);
        } else {
            LOGI("ping fail");
        }
        sprintf(context, "pind result: total %d, lost %d, receive %d, avg_time: %d \r\n", (int)result->total_num, (int)result->lost_num, (int)result->recv_num, (int)result->avg_time);
    } else {
        sprintf(context, "pind result null \r\n");
        LOGI("pind result null");
    }

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    //vTaskDelay(3000);
    gprs_set_host_sleep();
    sio_trigger_modem_sleep();
#endif

    nw_atci_print_result(context);
    vPortFree(context);
}

void nw_atci_initialize_modem()
{
    gprs_ret_t ret = GPRS_RET_ERROR;
        
    LOGI("enter");
    if (GPRS_RET_OK == gprs_is_modem_ready()) {
        if (GPRS_RET_OK == gprs_is_modem_initialized()) {
            LOGI("modem is initialized!");
        } else {
            if (GPRS_RET_OK != (ret = gprs_initialize_modem()) && GPRS_RET_WOULDBLOCK != ret) {
                LOGW("modem initialize fail %d", ret);
            } else {
                LOGI("modem initialize %d", ret);
            }
        }
    } else {
        LOGW("modem initialize fail modem is not ready!");
    }
}

void nw_atci_registration_network()
{
    gprs_ret_t ret = GPRS_RET_ERROR;
        
    LOGI("enter");
    if (GPRS_RET_OK == gprs_is_modem_initialized()) {
        if (GPRS_RET_OK == gprs_is_network_registered()) {
            LOGI("network is registered");
        } else {
            if (GPRS_RET_OK != (ret = gprs_registration_network()) &&
                GPRS_RET_WOULDBLOCK != ret) {
                LOGW("network registration fail %d", ret);
            } else {
                LOGI("network registration %d", ret);
            }
        }
    } else {
        LOGW("network registration fail modem is not initialized, please initialize modem first!");
    }
}


void nw_atci_gprs_activate()
{
    gprs_ret_t ret = GPRS_RET_ERROR;
        
    LOGI("enter");
    if (GPRS_RET_OK == gprs_is_network_registered()) {
        if (GPRS_RET_OK == gprs_is_data_activated()) {
            LOGI("gprs is activated");
        } else {
            if (GPRS_RET_OK != (ret = gprs_activate_data_with_cid(GPRS_PDP_CONTEXT_ID_1)) &&
                GPRS_RET_WOULDBLOCK != ret) {
                LOGW("gprs activate fail %d", ret);
            } else {
                LOGI("gprs activate %d", ret);
            } 
        }
    } else {
        LOGW("gprs activate fail network is not registered, please register network first!");
    }
}

void nw_atci_gprs_deactivate()
{    
    gprs_ret_t ret = GPRS_RET_ERROR;

    LOGI("enter");
        
    if (GPRS_RET_OK != gprs_is_data_activated()) {
        LOGI("gprs is not activated");
    } else {
        if (GPRS_RET_OK != (ret = gprs_deactivate_data_with_cid(GPRS_PDP_CONTEXT_ID_1)) &&
            GPRS_RET_WOULDBLOCK != ret) {
            LOGW("gprs deactivate fail %d", ret);
        } else {
            LOGI("gprs deactivate %d", ret);
        }
    }
}


void nw_atci_send_ping_req(void)
{
#if PING_NEW_VERSION
    ping_request_t ping_req = {0};
#endif

    sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_set_host_wakeup();
    sio_trigger_modem_wakeup();
#endif

#if PING_NEW_VERSION
    ping_req.addr = NW_ATCI_PING_REMOTE_IP_DEFAULT;
    ping_req.addr_type = PING_IP_ADDR_V4;
    ping_req.count = NW_ATCI_PING_COUNT;
    ping_req.ping_size = NW_ATCI_PING_SIZE;
    ping_req.timeout = NW_ATCI_PING_TIMEOUT;
    ping_req.callback = nw_atci_ping_cb;
    ping_request(&ping_req);
#else
    LOGI("ping_request start, addr = %s.\r\n", NW_ATCI_PING_REMOTE_IP_DEFAULT);
    ping_request(NW_ATCI_PING_COUNT,
                 NW_ATCI_PING_REMOTE_IP_DEFAULT,
                 PING_IP_ADDR_V4,
                 NW_ATCI_PING_SIZE,
                 nw_atci_ping_cb);
#endif
}


void nw_atci_gprs_action_callback(gprs_ret_t res, gprs_action_type_enum action)
{
    switch(action) {
        case GPRS_ACTION_TYPE_INITIALIZE: {
            if (res == GPRS_RET_OK) {
                LOGI("modem initialized success \r\n");
                nw_atci_print_result("MODEM INITIALIZED");
            } else {
                LOGI("modem initialized fail \r\n");
                nw_atci_print_result("MODEM INITIALIZED FAIL");
            }
        }
            break;
            
        case GPRS_ACTION_TYPE_REGISTRATION: {
            if (res == GPRS_RET_OK) {
                LOGI("network registered success \r\n");
                nw_atci_print_result("NETWORK REGISTERED");
            } else {
                LOGI("network registered fail \r\n");
                nw_atci_print_result("NETWORK REGISTERED FAIL");
            }
        }
            break;
            
        case GPRS_ACTION_TYPE_ACTIVATE: {
            if (res == GPRS_RET_OK) {
                LOGI("data activate success \r\n");
                nw_atci_print_result("DATA ACTIVATED");
            } else {
                LOGI("data activate fail \r\n");
                nw_atci_print_result("DATA ACTIVATE FAIL");
            }
        }
            break;
            
        case GPRS_ACTION_TYPE_DEACTIVATE: {
            if (res == GPRS_RET_OK) {
                LOGI("data deactivate success \r\n");
                nw_atci_print_result("DATA DEACTIVATED");
            } else {
                LOGI("data deactivate fail \r\n");
                nw_atci_print_result("DATA DEACTIVATE FAIL");
            }
        }
            break;

        default: 
            break;
    }
}

void nw_atci_cntx_init(void)
{    
    nw_demo_set_event_hander(nw_atci_event_hdl);  

    gprs_set_callback(nw_atci_gprs_action_callback);
}


void nw_atci_event_hdl(message_id_enum event_id, int32_t param1, void *param2)
{    
    LOGI("nw_atci_event_hdl(), event_id=%d, param1=%d\r\n", event_id, param1);

    if (MESSAGE_ID_NW_FLOW == event_id) {
        switch (param1) {
            case NW_FLOW_EVT_TYPE_MODEM_READY_IND: {
                LOGI("Recived MODEM_READY_IND! \r\n");                

                nw_atci_print_result("MODEM READY");
                
                break;
            }


        #ifdef __CMUX_SUPPORT__
            case NW_FLOW_EVT_TYPE_CMUX_CONN_IND: {
                if (GPRS_RET_OK == gprs_is_data_activated()) {
                    LOGW("Recive CMUX_CONN_IND when gprs is activated.\r\n");
                    break;
                }
                break;
            }
        #endif

            case NW_FLOW_EVT_TYPE_MODEM_EXCEPTION_IND: {
                LOGI("modem exception!!!! \r\n");   
                nw_atci_print_result("MODEM EXCEPTION");
                break;
            }

            case NW_FLOW_EVT_TYPE_GPRS_DEACTIVATE_IND: {
                LOGI("gprs deactivate ind \r\n");
                nw_atci_print_result("GPRS DEACTIVATE IND");
                break;
            }

            default: {
                break;
            }
        }
    } else if (MESSAGE_ID_GPRS_ATCMD_SENT_CB == event_id) {
        gprs_atcmd_sent_cb_hdl();
    } else if (MESSAGE_ID_GPRS_NOTI_HDL == event_id) {
        gprs_noti_hdl(param1);
    }
}


