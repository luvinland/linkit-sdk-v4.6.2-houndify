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

 
#include "bt_gap_le.h"
#include "bt_callback_manager.h"
#include "ble_find_me_client.h"
#include "bt_gattc.h"
#include "syslog.h"

/* Create the log control block as user wishes. Here we use 'BT_IAS' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(BLE_FMPC, PRINT_LEVEL_INFO);

#define BLE_UUID_IMEDIATELY_ALERT_SERVICE  0x1802     /**< IAS service UUID. */
#define BLE_IAS_MAX_CHAR_NUM               1
#define BLE_IAS_MAX_DESCR_NUM              1
#define BLE_ALERT_LEVEL_NO                 0
#define BLE_ALERT_LEVEL_MILD               1
#define BLE_ALERT_LEVEL_HIGH               2

bt_gattc_discovery_service_t p_ias_srv;
bt_gattc_discovery_descriptor_t g_ias_descr = {0, {0, {0}}};
bt_gattc_discovery_characteristic_t g_ias_char[BLE_IAS_MAX_CHAR_NUM] = {{{0}}};
static ble_fmpc_cntx_t g_fmpc_buffer[BLE_FMPC_CONNECTION_MAX];

static bt_status_t ble_fmp_client_save_connection_info(void *buff);
#if 1
static bt_status_t ble_fmp_client_delete_connection_info(void *buff);
#endif
static void ble_fmp_client_clear_all_connection_info(void);
static void ble_fmp_client_discovery_callback(uint16_t conn_handle, bt_gattc_discovery_service_t *discovered_db);
static bt_status_t ble_fmp_client_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
static bt_status_t ble_fmp_client_event_callback_register(void);
static void bt_fmp_client_evt_handler(bt_gattc_discovery_event_t *event);
static bt_status_t ble_fmp_client_write_command(uint16_t conn_handle, uint16_t attribute_handle, uint8_t len, uint8_t *value);


static bt_status_t ble_fmp_client_write_command(uint16_t conn_handle, uint16_t attribute_handle, uint8_t len, uint8_t *value)
{
    bt_gattc_write_without_rsp_req_t req;
    uint8_t buffer[40] = {0};

    LOG_I(BLE_FMPC, "%s: conn_handle is 0x%04x\r\n", __FUNCTION__, conn_handle);

    req.attribute_value_length = len;
    req.att_req = (bt_att_write_command_t *)buffer;
    req.att_req->opcode = BT_ATT_OPCODE_WRITE_COMMAND;
    req.att_req->attribute_handle = attribute_handle;
    memcpy(req.att_req->attribute_value, value, len);

    return bt_gattc_write_without_rsp(conn_handle, 0, &req);
}

ble_fmp_status_t ble_fmp_client_start_alert(void)
{
    uint8_t i;
    uint8_t dis_num = 0, null_ias = 0;
    ble_fmp_status_t status = BLE_FMP_STATUS_FAIL;
    
    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    
    for (i = 0; i< BLE_FMPC_CONNECTION_MAX; i++) {
        if ((0 != g_fmpc_buffer[i].conn_handle) && (NULL != g_fmpc_buffer[i].ias)) {
            bt_status_t sta = BT_STATUS_FAIL;
            LOG_I(BLE_FMPC, "write_alert_1, conn_handle is 0x%04x, value_handle is 0x%04x\r\n", g_fmpc_buffer[i].conn_handle, 
                  g_fmpc_buffer[i].ias->charateristics->value_handle);
            sta = ble_fmp_client_write_command(g_fmpc_buffer[i].conn_handle, 
                                                  g_fmpc_buffer[i].ias->charateristics->value_handle, 
                                                  1, 
                                                  (uint8_t *)BLE_ALERT_LEVEL_HIGH);
            if (BT_STATUS_SUCCESS == sta) {
                status = BLE_FMP_STATUS_SUCCESS;
            } else {
                status = BLE_FMP_STATUS_FAIL;
            }
            LOG_I(BLE_FMPC, "Find remote device, status is %d\r\n", status);
            break;
        } else if ((0 != g_fmpc_buffer[i].conn_handle) && (NULL == g_fmpc_buffer[i].ias)) {
            null_ias++;
        } else if (0 == g_fmpc_buffer[i].conn_handle) {
            dis_num++;
        }
    }
    if (BLE_FMPC_CONNECTION_MAX == dis_num) {
        status = BLE_FMP_STATUS_DISCONNECTED;
    } else if (BLE_FMPC_CONNECTION_MAX == null_ias) {
        status = BLE_FMP_STATUS_SERVICE_DISCOVERY_FAIL;
    }
    return status;
}

bool ble_fmp_client_is_connected(void)
{
    uint8_t i, null_connection = 0;
    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    for (i = 0; i< BLE_FMPC_CONNECTION_MAX; i++) {
        if (0 == g_fmpc_buffer[i].conn_handle) {
            null_connection++;
        }
    }
    if (BLE_FMPC_CONNECTION_MAX == null_connection) {
        return false;
    }
    return true;
}

ble_fmp_status_t ble_fmp_client_stop_alert(void)
{
    uint8_t i;
    uint8_t dis_num = 0, null_ias = 0;
    ble_fmp_status_t status = BLE_FMP_STATUS_FAIL;
    
    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    
    for (i = 0; i< BLE_FMPC_CONNECTION_MAX; i++) {
        if ((0 != g_fmpc_buffer[i].conn_handle) && (NULL != g_fmpc_buffer[i].ias)) {
            bt_status_t sta = BT_STATUS_FAIL;
            LOG_I(BLE_FMPC, "write_alert_0, conn_handle is 0x%04x, value_handle is 0x%04x\r\n", g_fmpc_buffer[i].conn_handle, 
                  g_fmpc_buffer[i].ias->charateristics->value_handle);
            sta = ble_fmp_client_write_command(g_fmpc_buffer[i].conn_handle, 
                                                  g_fmpc_buffer[i].ias->charateristics->value_handle, 
                                                  1, 
                                                  (uint8_t *)BLE_ALERT_LEVEL_NO);
            if (BT_STATUS_SUCCESS == sta) {
                status = BLE_FMP_STATUS_SUCCESS;
            } else {
                status = BLE_FMP_STATUS_FAIL;
            }
            LOG_I(BLE_FMPC, "%s: enter status is %d\r\n", __FUNCTION__, status);
            break;
        } else if ((0 != g_fmpc_buffer[i].conn_handle) && (NULL == g_fmpc_buffer[i].ias)) {
            null_ias++;
        } else if (0 == g_fmpc_buffer[i].conn_handle) {
            dis_num++;
        }
    }
    if (BLE_FMPC_CONNECTION_MAX == dis_num) {
        status = BLE_FMP_STATUS_DISCONNECTED;
    } else if (BLE_FMPC_CONNECTION_MAX == null_ias) {
        status = BLE_FMP_STATUS_SERVICE_DISCOVERY_FAIL;
    }
    return status;
}

static bt_status_t ble_fmp_client_save_connection_info(void *buff)
{
    uint8_t i;
    bt_status_t status = BT_STATUS_SUCCESS;
    
    bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buff;
	  LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    for (i = 0; i< BLE_FMPC_CONNECTION_MAX; i++) {
        /**< first connect, to save connection info. */
        if ((0 == g_fmpc_buffer[i].conn_handle) || (conn_ind->connection_handle != g_fmpc_buffer[i].conn_handle)) {
            g_fmpc_buffer[i].ias = NULL;
            g_fmpc_buffer[i].conn_handle = conn_ind->connection_handle;  
            LOG_I(BLE_FMPC, "%s: conn_handle is 0x%04x\r\n", __FUNCTION__, g_fmpc_buffer[i].conn_handle);
            break;
        /**< Reconnect. */
        } else if (conn_ind->connection_handle == g_fmpc_buffer[i].conn_handle) {
            LOG_I(BLE_FMPC, "%s: reconnect, conn_handle is 0x%04x\r\n", __FUNCTION__, g_fmpc_buffer[i].conn_handle);
            break;
        }
    }

    if (i == BLE_FMPC_CONNECTION_MAX) {  
        LOG_I(BLE_FMPC, "Reach maximum connection, no empty buffer!\r\n"); 
        status = BT_STATUS_OUT_OF_MEMORY;
    }
    return status;
}

#if 1
static bt_status_t ble_fmp_client_delete_connection_info(void *buff)
{
    uint8_t i; 
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_hci_evt_disconnect_complete_t *disconnect_complete;

    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    disconnect_complete = (bt_hci_evt_disconnect_complete_t*) buff;
    for (i = 0; i< BLE_FMPC_CONNECTION_MAX ; i++) {
        if (disconnect_complete->connection_handle == g_fmpc_buffer[i].conn_handle) {
            g_fmpc_buffer[i].conn_handle = 0;
            g_fmpc_buffer[i].ias = NULL;
            break;
        }
    }
    if (i == BLE_FMPC_CONNECTION_MAX) {
        LOG_I(BLE_FMPC, "Don't know connection info for deleting!\r\n"); 
        status = BT_STATUS_FAIL;
    }
    
    return status;
}
#endif

static void ble_fmp_client_clear_all_connection_info(void)
{
    uint8_t i;
    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);

    for (i = 0; i< BLE_FMPC_CONNECTION_MAX ; i++) {   
        g_fmpc_buffer[i].conn_handle = 0;
        g_fmpc_buffer[i].ias = NULL;
    }
}

static void ble_fmp_client_discovery_callback(uint16_t conn_handle, bt_gattc_discovery_service_t *discovered_db)
{
    uint8_t i; 

    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    for (i = 0; i< BLE_FMPC_CONNECTION_MAX ; i++) {
        if (conn_handle == g_fmpc_buffer[i].conn_handle) {
            g_fmpc_buffer[i].ias = discovered_db;       
            LOG_I(BLE_FMPC, "%s: conn_handle is 0x%04x, index is %d\r\n", __FUNCTION__, conn_handle, i);
            break;
        }
    }
    if (i == BLE_FMPC_CONNECTION_MAX) {
        LOG_I(BLE_FMPC, "Unknown connetion handle!\r\n");
    }
}

static bt_status_t ble_fmp_client_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    LOG_I(BLE_FMPC, "%s: status(0x%04x), msg(0x%04x)\r\n", __FUNCTION__, status, msg);

     if(status != BT_STATUS_SUCCESS) {
        return BT_STATUS_SUCCESS;
     }

     switch (msg)
     {
         case BT_GAP_LE_CONNECT_IND: {
             //bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buff;
             ble_fmp_client_save_connection_info(buff);
             //status = bt_gattc_discovery_start(conn_ind->connection_handle);
             //LOG_I(BLE_FMPC, "FMP Client start discovery, status is %d\r\n", status);
         }
             break;  

         case BT_GAP_LE_BONDING_COMPLETE_IND:   
            break;
         case BT_GAP_LE_DISCONNECT_IND: {
            ble_fmp_client_delete_connection_info(buff);
         }
            break;
            
         default:
             break;
     }
     return BT_STATUS_SUCCESS;
}

static bt_status_t ble_fmp_client_event_callback_register(void)
{
    LOG_I(BLE_FMPC, "%s: enter\r\n", __FUNCTION__);
    return bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM, (void *)ble_fmp_client_event_callback);    
}

bt_status_t ble_fmp_client_init(void)
{
    bt_status_t err_code;
    //ble_uuid_t ias_uuid;

#if 1
    ble_uuid_t ias_uuid = {
        .type = BLE_UUID_TYPE_128BIT,
        .uuid.uuid = {
        //     8     7     6     5     4     3     2     1
            0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
            0x00, 0x10, 0x00, 0x00, 0x02, 0x18, 0x00, 0x00
        }
    };
#endif
    ble_fmp_client_clear_all_connection_info();

    memset(&p_ias_srv, 0x00, sizeof(bt_gattc_discovery_service_t));
    //ias_uuid.type = BLE_UUID_TYPE_16BIT;
    //ias_uuid.uuid.uuid16 = BLE_UUID_IMEDIATELY_ALERT_SERVICE;
    p_ias_srv.characteristic_count = BLE_IAS_MAX_CHAR_NUM;
    for (int i = 0; i < BLE_IAS_MAX_CHAR_NUM; i++) {
        g_ias_char[i].descriptor_count = BLE_IAS_MAX_DESCR_NUM;
        g_ias_char[i].descriptor = &g_ias_descr;
    }
    p_ias_srv.charateristics = g_ias_char;
    //LOG_I(BLE_FMPC, "IAS srv_uuid = 0x%04x\r\n", ias_uuid.uuid.uuid16);

    err_code = bt_gattc_discovery_service_register(&ias_uuid, &p_ias_srv, bt_fmp_client_evt_handler);
    if (0 != err_code) {
        LOG_I(BLE_FMPC, "IAS init fail, status is %d\r\n", err_code);
        return BT_STATUS_FAIL;
    }
    err_code = ble_fmp_client_event_callback_register();
    return err_code;
}

/**@brief Heart Rate Collector Handler.
 */
static void bt_fmp_client_evt_handler(bt_gattc_discovery_event_t *event)
{
    int32_t err_code;

    switch (event->event_type)
    {
        case BT_GATTC_DISCOVERY_EVENT_COMPLETE: {
            LOG_I(BLE_FMPC, "IAS discover success, start_handle is 0x%08x, end_handle is 0x%08x\r\n", p_ias_srv.start_handle, p_ias_srv.end_handle);
            #if 1
            for (int i = 0; i < p_ias_srv.char_count_found; i++) {  
                LOG_I(BLE_FMPC, "IAS char[%d].handle = 0x%04x, value_handle = 0x%04x, property = %d \r\n", i, 
                    (p_ias_srv.charateristics + i)->handle, (p_ias_srv.charateristics + i)->value_handle, (p_ias_srv.charateristics + i)->property);
                if (BLE_UUID_TYPE_16BIT == (p_ias_srv.charateristics + i)->char_uuid.type) {
                    LOG_I(BLE_FMPC, "IAS char[%d].uuid16 = 0x%04x\r\n", i, (p_ias_srv.charateristics + i)->char_uuid.uuid.uuid16);
                } else if (BLE_UUID_TYPE_32BIT == (p_ias_srv.charateristics + i)->char_uuid.type) {
                    LOG_I(BLE_FMPC, "IAS char[%d].uuid32 = 0x%04x\r\n", i, (p_ias_srv.charateristics + i)->char_uuid.uuid.uuid32);
                } else if (BLE_UUID_TYPE_128BIT == (p_ias_srv.charateristics + i)->char_uuid.type){
                    for (int j = 15; j >= 0; j--) {
                        printf("0x%02x ", (p_ias_srv.charateristics + i)->char_uuid.uuid.uuid[j]);
                    }
                    printf("\n");
                } else {
                    LOG_I(BLE_FMPC, "IAS uuid error!\r\n");
                }
                for (int k = 0; k < (p_ias_srv.charateristics + i)->descr_count_found; k++) {
                    LOG_I(BLE_FMPC, "IAS char[%d].descriptor[%d].handle = 0x%04x\r\n", i, k, ((p_ias_srv.charateristics + i)->descriptor + k)->handle);
                    if (BLE_UUID_TYPE_16BIT == ((p_ias_srv.charateristics + i)->descriptor + k)->descriptor_uuid.type) {
                        LOG_I(BLE_FMPC, "IAS char[%d].descriptor[%d].uuid16 = 0x%04x\r\n", i, k, ((p_ias_srv.charateristics + i)->descriptor + k)->descriptor_uuid.uuid.uuid16);
                    } else if (BLE_UUID_TYPE_32BIT == ((p_ias_srv.charateristics + i)->descriptor + k)->descriptor_uuid.type) {
                        LOG_I(BLE_FMPC, "IAS char[%d].descriptor[%d].uuid32 = 0x%04x\r\n", i, k, ((p_ias_srv.charateristics + i)->descriptor + k)->descriptor_uuid.uuid.uuid32);
                    } else if (BLE_UUID_TYPE_32BIT == ((p_ias_srv.charateristics + i)->descriptor + k)->descriptor_uuid.type) {
                        for (int m = 15; m >= 0; m--) {
                            printf("0x%02x ", ((p_ias_srv.charateristics + i)->descriptor + k)->descriptor_uuid.uuid.uuid[m]);
                        }
                        printf("\n");
                    } else {
                        LOG_I(BLE_FMPC, "IAS uuid error!\r\n");
                    }
                }
            }
            #endif
            ble_fmp_client_discovery_callback(event->conn_handle, event->params.discovered_db);
            break;
        }
        case BT_GATTC_DISCOVERY_EVENT_FAIL:
        {
            err_code = event->params.error_code;
            ble_fmp_client_discovery_callback(event->conn_handle, NULL);
            LOG_I(BLE_FMPC, "IAS discover fail, error_code is %d\r\n", err_code);
            break;
        }

        default:
            break;
    }
}



