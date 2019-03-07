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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "syslog.h"
#ifdef __CMUX_SUPPORT__
#include "cmux_porting.h"
#include "cmux.h"
#endif
#include "sio_gprot.h"
#include "urc_app.h"
#include "task_def.h"

#ifdef __CMUX_IT__

log_create_module(sio_test_app, PRINT_LEVEL_INFO);

#define LOGE(fmt,arg...)        LOG_E(sio_test_app, "[######################] "fmt,##arg)
#define LOGW(fmt,arg...)        LOG_W(sio_test_app, "[######################] "fmt,##arg)
#define LOGI(fmt,arg...)        LOG_I(sio_test_app, "[######################] "fmt,##arg)

#define SIO_TEST_APP_QUEUE_SIZE         (10)
#define SIO_TEST_APP_RX_BUFFER_LEN      (1024)

static QueueHandle_t g_sio_test_app_queue;
static TimerHandle_t g_sio_test_app_timer;
static int32_t g_sio_test_app_id;
static uint8_t g_sio_test_rx_buffer[SIO_TEST_APP_RX_BUFFER_LEN];
static bool g_sio_test_is_startup;
static bool g_sio_test_connection_success;
static bool g_sio_test_phonebook_ready;

static kal_bool sio_test_app_send_message(msg_type msg_id, local_para_struct *local_para_ptr)
{
    ilm_struct ilm;

    ilm.msg_id = msg_id;
    ilm.local_para_ptr = local_para_ptr;

    if (xQueueSend(g_sio_test_app_queue, &ilm, portMAX_DELAY) != pdPASS) {
        LOGE("xQueueSend != pdPASS");
        return KAL_FALSE;
    }

    return KAL_TRUE;
}

static void sio_test_app_send_cmux_request(void)
{
    const char *startupCmd = "AT+CMUX=0\r\n";
    uint32_t write_data_len;

    if (g_sio_test_is_startup == false) {
        write_data_len = sio_send_data(g_sio_test_app_id, (const uint8_t *)startupCmd, strlen(startupCmd));
        LOGI("send data (len: %d) to uart: %s", write_data_len, startupCmd);
        g_sio_test_is_startup = true;
    }
}

static void sio_test_app_send_cpbr_request(void)
{
    const char *cpbrReq = "AT+CPBR=?\r\n";
    uint32_t write_data_len;

    if (g_sio_test_phonebook_ready == true && g_sio_test_connection_success == true) {
        write_data_len = cmux_write_data(sio_get_channel_id(g_sio_test_app_id), (const uint8_t *)cpbrReq, strlen(cpbrReq));
        LOGI("send data (len: %d) to uart: %s", write_data_len, cpbrReq);
    }
}

static void sio_test_app_event_hdlr(uint32_t event, void *data)
{
    LOGI("event = %d", event);

    switch (event) {
        case SIO_UART_EVENT_MODEM_READY: {
                sio_test_app_send_message(SIO_UART_EVENT_MODEM_READY, NULL);
            }
            break;
        case SIO_UART_EVENT_MODEM_DATA_TO_READ: {
                sio_rx_data_to_read_struct *rx_data = (sio_rx_data_to_read_struct *)pvPortMalloc(sizeof(sio_rx_data_to_read_struct));
                memcpy(rx_data, (sio_rx_data_to_read_struct *)data, sizeof(sio_rx_data_to_read_struct));
                LOGI("app_id = %d, read_length = %d", rx_data->app_id, rx_data->read_length);
                if (rx_data->app_id != g_sio_test_app_id) {
                    vPortFree(rx_data);
                    break;
                }
                sio_test_app_send_message(SIO_UART_EVENT_MODEM_DATA_TO_READ, (local_para_struct *) rx_data);
            }
            break;
        case CMUX_EVENT_READY_TO_READ: {
                cmux_event_ready_to_read_t *ready_to_read = (cmux_event_ready_to_read_t *)pvPortMalloc(sizeof(cmux_event_ready_to_read_t));
                memcpy(ready_to_read, (cmux_event_ready_to_read_t *)data, sizeof(cmux_event_ready_to_read_t));
                LOGI("channel_id = %d", ready_to_read->channel_id);
                if (ready_to_read->channel_id != sio_get_channel_id(g_sio_test_app_id)) {
                    vPortFree(ready_to_read);
                    break;
                }
                sio_test_app_send_message(CMUX_EVENT_READY_TO_READ, (local_para_struct *) ready_to_read);
            }
            break;
        case CMUX_EVENT_CHANNEL_CONNECTION: {
                cmux_event_channel_connection_t *status_changed = (cmux_event_channel_connection_t *)pvPortMalloc(sizeof(cmux_event_channel_connection_t));
                memcpy(status_changed, (cmux_event_channel_connection_t *)data, sizeof(cmux_event_channel_connection_t));
                LOGI("channel_id = %d", status_changed->channel_id);
                if (status_changed->channel_id != sio_get_channel_id(g_sio_test_app_id)) {
                    vPortFree(status_changed);
                    break;
                }
                sio_test_app_send_message(CMUX_EVENT_CHANNEL_CONNECTION, (local_para_struct *) status_changed);
            }
            break;
        case CMUX_EVENT_CHANNEL_DISCONNECTION: {
                cmux_event_channel_disconnection_t *status_changed = (cmux_event_channel_disconnection_t *)pvPortMalloc(sizeof(cmux_event_channel_disconnection_t));
                memcpy(status_changed, (cmux_event_channel_disconnection_t *)data, sizeof(cmux_event_channel_disconnection_t));
                LOGI("channel_id = %d", status_changed->channel_id);
                if (status_changed->channel_id != sio_get_channel_id(g_sio_test_app_id)) {
                    vPortFree(status_changed);
                    break;
                }
                sio_test_app_send_message(CMUX_EVENT_CHANNEL_DISCONNECTION, (local_para_struct *) status_changed);
            }
            break;
        case CMUX_EVENT_CLOSE_DOWN: {
                sio_test_app_send_message(CMUX_EVENT_CLOSE_DOWN, NULL);
            }
            break;
        case SIO_UART_EVENT_MODEM_EXCEPTION: {
                sio_test_app_send_message(SIO_UART_EVENT_MODEM_EXCEPTION, NULL);
            }
            break;
        default:
            break;
    }
}

static urc_cb_ret sio_test_app_urc_callback(uint8_t *payload, uint32_t length)
{
    const char *PhonebookNotReady = "\r\n+EIND: 32\r\n";
    const char *PhonebookReady = "\r\n+EIND: 2\r\n";
    uint32_t read_data_len;

    read_data_len = (length < SIO_TEST_APP_RX_BUFFER_LEN) ? (length) : (SIO_TEST_APP_RX_BUFFER_LEN - 1);
    memcpy(g_sio_test_rx_buffer, payload, read_data_len);
    g_sio_test_rx_buffer[read_data_len] = '\0';
    LOGI("g_sio_test_rx_buffer = @@@@@@@@@@%s@@@@@@@@@@, read_data_len = %d", g_sio_test_rx_buffer, read_data_len);

    if (strncasecmp((const char *)g_sio_test_rx_buffer, PhonebookNotReady, strlen(PhonebookNotReady)) == 0) {
        g_sio_test_phonebook_ready = false;
    } else if (strncasecmp((const char *)g_sio_test_rx_buffer, PhonebookReady, strlen(PhonebookReady)) == 0) {
        g_sio_test_phonebook_ready = true;
        sio_test_app_send_cpbr_request();
    }

    return RET_OK_CONTINUE;
}

static void sio_test_app_init(void)
{
    g_sio_test_app_id = sio_register_event_notifier(SIO_APP_TYPE_CMUX_AT_CMD | SIO_APP_TYPE_UART_AT_CMD, sio_test_app_event_hdlr);
    LOGI("g_sio_test_app_id: %d", g_sio_test_app_id);

    urc_register_callback(sio_test_app_urc_callback);

    log_config_print_switch(msm, DEBUG_LOG_OFF);
    log_config_print_switch(gprs, DEBUG_LOG_OFF);
    log_config_print_switch(nw, DEBUG_LOG_OFF);
    log_config_print_switch(GRAPHIC_TAG, DEBUG_LOG_OFF);
}

static void sio_test_app_dispatch_event(ilm_struct *ilm_ptr)
{
    LOGI("msg_id: %d", ilm_ptr->msg_id);

    switch (ilm_ptr->msg_id) {
        case SIO_UART_EVENT_MODEM_READY: {
                sio_test_app_send_cmux_request();
            }
            break;
        case SIO_UART_EVENT_MODEM_DATA_TO_READ: {
                sio_rx_data_to_read_struct *rx_data = (sio_rx_data_to_read_struct *)ilm_ptr->local_para_ptr;
                const char *startupRsp = "\r\nOK\r\n";
                uint32_t read_data_len;

                read_data_len = (rx_data->read_length < SIO_TEST_APP_RX_BUFFER_LEN) ? (rx_data->read_length) : (SIO_TEST_APP_RX_BUFFER_LEN - 1);
                configASSERT(rx_data->app_id == g_sio_test_app_id);
                read_data_len = sio_receive_data(rx_data->app_id, g_sio_test_rx_buffer, read_data_len);
                g_sio_test_rx_buffer[read_data_len] = '\0';
                LOGI("g_sio_test_rx_buffer = @@@@@@@@@@%s@@@@@@@@@@, read_data_len = %d", g_sio_test_rx_buffer, read_data_len);

                if (strncasecmp((const char *)g_sio_test_rx_buffer, startupRsp, strlen(startupRsp)) == 0) {
                    sio_uart_ret_t ret;
                    ret = sio_set_mode(rx_data->app_id, SIO_MODE_TYPE_CMUX);
                    LOGI("sio_set_mode: %d", ret);
                }
            }
            break;
        case CMUX_EVENT_READY_TO_READ: {
                cmux_event_ready_to_read_t *ready_to_read = (cmux_event_ready_to_read_t *)ilm_ptr->local_para_ptr;
                const char *cpbrRsp = "\r\n+CPBR:";
                uint32_t read_data_len;

                configASSERT(ready_to_read->channel_id == sio_get_channel_id(g_sio_test_app_id));
                read_data_len = cmux_get_available_read_length(ready_to_read->channel_id);
                if (read_data_len >= SIO_TEST_APP_RX_BUFFER_LEN) {
                    read_data_len = SIO_TEST_APP_RX_BUFFER_LEN - 1;
                }

                read_data_len = cmux_read_data(ready_to_read->channel_id, g_sio_test_rx_buffer, read_data_len);
                g_sio_test_rx_buffer[read_data_len] = '\0';
                LOGI("g_sio_test_rx_buffer = @@@@@@@@@@%s@@@@@@@@@@, read_data_len = %d", g_sio_test_rx_buffer, read_data_len);

                if (strncasecmp((const char *)g_sio_test_rx_buffer, cpbrRsp, strlen(cpbrRsp)) == 0) {
                    sio_uart_ret_t ret;
                    ret = sio_set_mode(g_sio_test_app_id, SIO_MODE_TYPE_UART);
                    LOGI("sio_set_mode: %d", ret);
                }
            }
            break;
        case CMUX_EVENT_CHANNEL_CONNECTION: {
                cmux_event_channel_connection_t *status_changed = (cmux_event_channel_connection_t *)ilm_ptr->local_para_ptr;

                configASSERT(status_changed->channel_id == sio_get_channel_id(g_sio_test_app_id));
                g_sio_test_connection_success = true;
                sio_test_app_send_cpbr_request();
            }
            break;
        case CMUX_EVENT_CHANNEL_DISCONNECTION: {
                cmux_event_channel_disconnection_t *status_changed = (cmux_event_channel_disconnection_t *)ilm_ptr->local_para_ptr;

                configASSERT(status_changed->channel_id == sio_get_channel_id(g_sio_test_app_id));
                g_sio_test_connection_success = false;
            }
            break;
        case CMUX_EVENT_CLOSE_DOWN: {
                if (g_sio_test_is_startup == true) {
                    g_sio_test_is_startup = false;
                    xTimerChangePeriod(g_sio_test_app_timer, 10000 / portTICK_PERIOD_MS, 0);
                    xTimerStart(g_sio_test_app_timer, 0);
                }
            }
            break;
        case SIO_UART_EVENT_MODEM_EXCEPTION: {
                sio_uart_ret_t ret;
                ret = sio_set_mode(g_sio_test_app_id, SIO_MODE_TYPE_UART);
                LOGI("sio_set_mode: %d", ret);
            }
            break;
        default:
            break;
    }
}

static void sio_test_app_timeout_hdlr(TimerHandle_t xTimer)
{
    sio_test_app_send_cmux_request();
}

static void sio_test_app_main(void *arg)
{
    ilm_struct current_ilm;

    while (1) {
        if (xQueueReceive(g_sio_test_app_queue, (void *)&current_ilm, portMAX_DELAY)) {

            sio_test_app_dispatch_event(&current_ilm);

            if (current_ilm.local_para_ptr != NULL) {
                vPortFree(current_ilm.local_para_ptr);
            }
        }
    }
}

#endif /* __CMUX_IT__ */

void sio_test_app_create(void)
{
#ifdef __CMUX_IT__
    sio_test_app_init();

    g_sio_test_app_timer = xTimerCreate("Restart CMUX", 0xffff, pdFALSE, NULL, sio_test_app_timeout_hdlr);

    g_sio_test_app_queue = xQueueCreate(SIO_TEST_APP_QUEUE_SIZE, sizeof(ilm_struct));

    xTaskCreate(sio_test_app_main,
                SIO_TEST_APP_TASK_NAME,
                SIO_TEST_APP_TASK_STACK_SIZE / sizeof(StackType_t),
                NULL,
                SIO_TEST_APP_TASK_PRIORITY,
                NULL);
#endif
}

