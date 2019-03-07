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
     
#include <string.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "sio_gprot.h"
#include "task.h"
#include "msm.h"
#include "timers.h"
#include "hal_sleep_manager.h"
#include "hal_uart.h"

#define SIO_SUPPORT_SLEEP_MANAGER

typedef struct sio_context_struct {
#ifdef __CMUX_SUPPORT__
    uint32_t                 data_type;
    uint32_t                 uart_mode;
#endif
#ifdef SIO_SUPPORT_SLEEP_MANAGER
    uint32_t                 sleep_manager_handler;
    uint8_t                  is_exception;
    uint8_t                  is_wakeup;
    TimerHandle_t            sleep_lock_time_handle;
    uint8_t                  timer_is_running;
    uint8_t                  set_modem_wakeup;
#endif
} sio_context_struct;

static sio_context_struct sio_cnt = {0};

#ifdef __CMUX_SUPPORT__
#define  sio_is_uart_type(type)       ((type & 0xF0) ? 1 : 0)
#define  sio_is_uart_app(app_id)      ((app_id & 0xF0) ? 1 : 0)
#endif

#ifdef SIO_PRINTF
#define LOGE(fmt,arg...)   printf(("[SIO MAIN]: "fmt), ##arg)
#define LOGW(fmt,arg...)   printf(("[SIO MAIN]: "fmt), ##arg)
#define LOGI(fmt,arg...)   printf(("[SIO MAIN]: "fmt), ##arg)
#else
log_create_module(sio, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(sio, "[SIO MAIN]: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(sio, "[SIO MAIN]: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(sio, "[SIO MAIN]: "fmt,##arg)
#endif


extern uint32_t sio_uart_port_write_data(hal_uart_port_t uart_index, uint8_t *buf, uint32_t buf_len);
extern hal_uart_status_t hal_uart_set_dte_dtr_active(hal_uart_port_t uart_port);
extern hal_uart_status_t hal_uart_set_dte_dtr_deactive(hal_uart_port_t uart_port);

#ifdef __CMUX_SUPPORT__
static cmux_channel_type_t sio_app_type_to_cmux_type(sio_app_type_t type)
{
    uint32_t app_type = type & 0x0F;

    if (app_type == SIO_APP_TYPE_CMUX_AT_CMD) {
        return CMUX_CHANNEL_TYPE_COMMAND;
    } else if (app_type == SIO_APP_TYPE_CMUX_IP_DATA) {
        return CMUX_CHANNEL_TYPE_IP_DATA;
    } else {
        return CMUX_CHANNEL_TYPE_MAX;
    }
}
#endif /* __CMUX_SUPPORT__ */

#ifdef __CMUX_SUPPORT__
int32_t sio_get_current_mode(void)
{
    return sio_cnt.uart_mode;
}

sio_ret_t sio_set_current_mode(int32_t mode)
{
    sio_cnt.uart_mode = mode;
    return SIO_RET_OK;
}
#endif

#ifdef SIO_SUPPORT_SLEEP_MANAGER
static void
sio_sleep_lock_timer_timeout_callback(TimerHandle_t tmr)
{
    uint32_t sleep_lock_status;

    LOGI("sio_sleep_lock_timer_timeout_callback\n");

    sleep_lock_status = hal_sleep_manager_get_lock_status();
    LOGI("sio_sleep_lock_timer_timeout_callback() begin %d\r\n", sleep_lock_status);
    if (sleep_lock_status & (1 << sio_cnt.sleep_manager_handler)) {
        LOGI("sleep is lock, sleep unlock.\r\n");
        hal_sleep_manager_unlock_sleep(sio_cnt.sleep_manager_handler);
        sleep_lock_status = hal_sleep_manager_get_lock_status();
        LOGI("sio_sleep_lock_timer_timeout_callback() end %d\r\n", sleep_lock_status);
        msm_notify_host_status_to_modem(MSM_STATUS_INACTIVE);
    } else {
        LOGI("sleep is unlocked.\r\n");
    }  

    sio_cnt.is_exception = 0;
    sio_cnt.is_wakeup = 0;
                    
    // stop timer
    if (sio_cnt.sleep_lock_time_handle && sio_cnt.timer_is_running) {
        xTimerStop(sio_cnt.sleep_lock_time_handle, 0);
        sio_cnt.timer_is_running = 0;
    }    
}
#endif

void sio_from_uart_event_handler(uint32_t event, void *data)
{
#ifdef SIO_SUPPORT_SLEEP_MANAGER
    uint32_t sleep_lock_status;

    LOGI("sio_from_uart_event_handler() event = %d handler = %d. \r\n", event, sio_cnt.sleep_manager_handler);
    
    switch(event)
    {
        case SIO_UART_EVENT_MODEM_ABNORMAL:        
            msm_trigger_to_modem(MSM_TRIGGER_TYPE_RESET);
        
            sio_cnt.is_exception = 1;
            sleep_lock_status = hal_sleep_manager_get_lock_status();
            LOGI("SIO_UART_EVENT_MODEM_ABNORMAL begin %d\r\n", sleep_lock_status);
            if (!(sleep_lock_status & (1 << sio_cnt.sleep_manager_handler))) {
                LOGI("sleep is not lock, sleep lock.\r\n");
                hal_sleep_manager_lock_sleep(sio_cnt.sleep_manager_handler);
                msm_notify_host_status_to_modem(MSM_STATUS_ACTIVE);
                sleep_lock_status = hal_sleep_manager_get_lock_status();
                LOGI("SIO_UART_EVENT_MODEM_ABNORMAL end %d\r\n", sleep_lock_status);

                // start timer
                if (sio_cnt.sleep_lock_time_handle == NULL) {
                    sio_cnt.sleep_lock_time_handle = xTimerCreate("sio sleep lock timer",
                       10 * 1000 / portTICK_PERIOD_MS, // 10 seconds
                       pdFALSE,
                       NULL,
                       sio_sleep_lock_timer_timeout_callback);
                }
                if (sio_cnt.sleep_lock_time_handle) {
                    if (sio_cnt.timer_is_running == 0) {
                        xTimerStart(sio_cnt.sleep_lock_time_handle, 0);
                        sio_cnt.timer_is_running = 1;
                    } else {
                        xTimerReset(sio_cnt.sleep_lock_time_handle, 0);
                        sio_cnt.timer_is_running = 1;
                    }
                }
            } else {
                LOGI("sleep is locked.\r\n");
            }    
            break;

        case SIO_UART_EVENT_MODEM_READY:
            if (sio_cnt.is_exception == 1) {
                sleep_lock_status = hal_sleep_manager_get_lock_status();
                LOGI("SIO_UART_EVENT_MODEM_READY begin %d\r\n", sleep_lock_status);
                if (sleep_lock_status & (1 << sio_cnt.sleep_manager_handler)) {
                    LOGI("sleep is lock, sleep unlock.\r\n");
                    hal_sleep_manager_unlock_sleep(sio_cnt.sleep_manager_handler);
                    msm_notify_host_status_to_modem(MSM_STATUS_INACTIVE);
                    sleep_lock_status = hal_sleep_manager_get_lock_status();
                    LOGI("SIO_UART_EVENT_MODEM_READY end %d\r\n", sleep_lock_status);
                    
                    // stop timer
                    if (sio_cnt.sleep_lock_time_handle && sio_cnt.timer_is_running) {
                        xTimerStop(sio_cnt.sleep_lock_time_handle, 0);
                        sio_cnt.is_exception = 0;
                        sio_cnt.is_wakeup = 0;
                        sio_cnt.timer_is_running = 0;
                    }
                } else {
                    LOGI("sleep is unlocked.\r\n");
                }  
            }
            break;

        case SIO_UART_EVENT_MODEM_WAKEUP:
            sio_cnt.is_wakeup = 1;
            sleep_lock_status = hal_sleep_manager_get_lock_status();
            LOGI("SIO_UART_EVENT_MODEM_WAKEUP begin %d\r\n", sleep_lock_status);
            if (!(sleep_lock_status & (1 << sio_cnt.sleep_manager_handler))) {
                LOGI("sleep is not lock, sleep lock.\r\n");
                hal_sleep_manager_lock_sleep(sio_cnt.sleep_manager_handler);
                msm_notify_host_status_to_modem(MSM_STATUS_ACTIVE);
                sleep_lock_status = hal_sleep_manager_get_lock_status();
                LOGI("SIO_UART_EVENT_MODEM_WAKEUP end %d\r\n", sleep_lock_status);
                
                // start timer
                if (sio_cnt.sleep_lock_time_handle == NULL) {
                    sio_cnt.sleep_lock_time_handle = xTimerCreate("sio sleep lock timer",
                       10 * 1000 / portTICK_PERIOD_MS, // 10 seconds
                       pdFALSE,
                       NULL,
                       sio_sleep_lock_timer_timeout_callback);
                }
                if (sio_cnt.sleep_lock_time_handle) {
                    if (sio_cnt.timer_is_running == 0) {
                        xTimerStart(sio_cnt.sleep_lock_time_handle, 0);
                        sio_cnt.timer_is_running = 1;
                    } else {
                        xTimerReset(sio_cnt.sleep_lock_time_handle, 0);
                        sio_cnt.timer_is_running = 1;
                    }
                }
            } else {
                LOGI("sleep is locked.\r\n");
            }    
            break;

        case SIO_UART_EVENT_MODEM_URC:
            if (sio_cnt.is_wakeup == 1) {
                sleep_lock_status = hal_sleep_manager_get_lock_status();
                LOGI("SIO_UART_EVENT_MODEM_URC begin %d\r\n", sleep_lock_status);
                if (sleep_lock_status & (1 << sio_cnt.sleep_manager_handler)) {
                    LOGI("sleep is lock, sleep unlock.\r\n");
                    hal_sleep_manager_unlock_sleep(sio_cnt.sleep_manager_handler);
                    msm_notify_host_status_to_modem(MSM_STATUS_INACTIVE);
                    sleep_lock_status = hal_sleep_manager_get_lock_status();
                    LOGI("SIO_UART_EVENT_MODEM_URC end %d\r\n", sleep_lock_status);
                    
                    // stop timer
                    if (sio_cnt.sleep_lock_time_handle && sio_cnt.timer_is_running) {
                        xTimerStop(sio_cnt.sleep_lock_time_handle, 0);
                        sio_cnt.is_exception = 0;
                        sio_cnt.is_wakeup = 0;
                        sio_cnt.timer_is_running = 0;
                    }
                } else {
                    LOGI("sleep is unlocked.\r\n");
                }  
            }
            break;

        default:
            break;
    }
#else
    if (SIO_UART_EVENT_MODEM_ABNORMAL == event) {
        msm_trigger_to_modem(MSM_TRIGGER_TYPE_RESET);
    }
#endif
}


sio_ret_t sio_init(hal_uart_port_t uart_port)
{
    LOGI("~~~~~~~~~~~~~SIO INIT~~~~~~~~~~~~~");
#ifdef __CMUX_SUPPORT__
    cmux_create_task(HAL_UART_1, CMUX_ROLE_CLIENT);
#endif /* __CMUX_SUPPORT__ */
    
    if (SIO_UART_RET_OK != sio_uart_init(uart_port)) {
    return SIO_RET_ERROR;
}

#ifdef __CMUX_SUPPORT__
    sio_set_current_mode(SIO_MODE_TYPE_UART | SIO_DATA_TYPE_COMMAND);  
#endif

#ifdef SIO_SUPPORT_SLEEP_MANAGER
    sio_cnt.sleep_manager_handler = hal_sleep_manager_set_sleep_handle("sio_api");
    LOGI("sio_cnt.sleep_manager_handler = %d.\r\n", sio_cnt.sleep_manager_handler);
#endif
    sio_uart_set_notification_callback(1, 0, SIO_UART_EVENT_MAX_NUMBER, sio_from_uart_event_handler);
    return SIO_RET_OK;
} 

sio_ret_t sio_set_mode(int32_t app_id, int32_t flag)
{
#ifdef __CMUX_SUPPORT__
    int32_t mode_type, data_type;
    int32_t current_type;
    int32_t current_mode, current_data;
    sio_uart_ret_t ret = SIO_UART_RET_ERROR;
    LOGI("sio_cnt.sleep_manager_handler = %d.\r\n", sio_cnt.sleep_manager_handler);

    LOGI("sio_set_mode() app_id = %d,  flag = %d. \r\n", app_id, flag);
    if (sio_is_uart_app(app_id) == 0) {
        return SIO_RET_ERROR;
    }

    mode_type = flag & 0x0F;
    data_type = flag & 0xF0;
    current_type = sio_get_current_mode();
    current_mode = current_type & 0x0F;
    current_data = current_type & 0xF0;

    if (mode_type != current_mode) {
        cmux_status_t cmux_status = CMUX_STATUS_UART_OPEN_ERROR;
        if (mode_type == SIO_MODE_TYPE_CMUX) {
            ret = sio_uart_port_deinit();
            if (ret == SIO_UART_RET_OK) {
                cmux_status = cmux_enable();
            }
        } else if (mode_type == SIO_MODE_TYPE_UART) {
            cmux_status = cmux_disable();
            if (cmux_status == CMUX_STATUS_OK) {
                ret = sio_uart_port_init();
            }
        } else {
            ret = SIO_UART_RET_ERROR;
        }

        if (cmux_status == CMUX_STATUS_OK && ret == SIO_UART_RET_OK) {
            current_type = (current_type & 0xF0) | mode_type;
            ret = SIO_UART_RET_OK;
        }
    }

    if (data_type != current_data) {
        if (data_type == SIO_DATA_TYPE_COMMAND || data_type == SIO_DATA_TYPE_DATA) {
            current_type = (current_type & 0x0F) | data_type;
            ret = SIO_UART_RET_OK;
        }
    }
    sio_set_current_mode(current_type);

    if (ret == SIO_UART_RET_ERROR)
        return SIO_RET_ERROR;
    else 
        return SIO_RET_OK;
#else /* __CMUX_SUPPORT__ */

    return SIO_RET_OK;
#endif /* __CMUX_SUPPORT__ */
}


uint32_t sio_send_data(int32_t app_id, const uint8_t *data, uint32_t size)
{
    uint32_t sent_length = 0;
#ifdef __CMUX_SUPPORT__
    int32_t current_type;
    int32_t mode_type, data_type;

    current_type = sio_get_current_mode();
    mode_type = current_type & 0x0F;
    data_type = current_type & 0xF0;
    if (mode_type == SIO_MODE_TYPE_UART) {
        if ((data_type == SIO_DATA_TYPE_COMMAND && sio_is_uart_app(app_id) == 1) ||
                (data_type == SIO_DATA_TYPE_DATA)) {
            sent_length = sio_uart_send_data(app_id, data, size);
        } else {
            LOGE("app_id = %d Can't send data. \r\n", app_id);
        }
    }
    else if (mode_type == SIO_MODE_TYPE_CMUX) {
        sent_length = cmux_write_data(sio_get_channel_id(app_id), data, size);
    }
#else
    sent_length = sio_uart_send_data(app_id, data, size);
#endif /* __CMUX_SUPPORT__ */
    return sent_length;
}


uint32_t sio_receive_data(int32_t app_id, uint8_t *buffer, uint32_t size)
{
    uint32_t read_size = 0;
#ifdef __CMUX_SUPPORT__
    int32_t mode;
    int32_t mode_type, data_type;
    uint32_t total_size = 0;
    cmux_channel_id_t channel_id;

    mode = sio_get_current_mode();
    LOGI("sio_receive_data() app_id = %d,  mode = %d, size = %d. \r\n", app_id, mode, size);
    mode_type = mode & 0x0F;
    data_type = mode & 0xF0;
    if (mode_type == SIO_MODE_TYPE_UART) {
        if((data_type == SIO_DATA_TYPE_COMMAND && sio_is_uart_app(app_id) == 1) ||
            (data_type == SIO_DATA_TYPE_DATA)) {
        read_size = sio_uart_receive_data(app_id, buffer, size);
        }
    } 
    else if (mode_type == SIO_MODE_TYPE_CMUX) {
        channel_id = sio_get_channel_id(app_id);
        total_size = cmux_get_available_read_length(channel_id);
        size = size < total_size ? size : total_size;
        read_size = cmux_read_data(channel_id, buffer, size);
    }
#else
    read_size = sio_uart_receive_data(app_id, buffer, size);
#endif /* __CMUX_SUPPORT__ */
    LOGI("sio_receive_data() read_size = %d, buffer = %s. \r\n", read_size, buffer);
    return read_size;
}


int32_t sio_register_event_notifier(sio_app_type_t type, sio_event_handler notifier)
{
    int32_t app_id = 0;
    int32_t channel_id = 0;
#ifdef __CMUX_SUPPORT__
    cmux_channel_type_t channel_type;
#endif /* __CMUX_SUPPORT__ */

    LOGI("sio_register_app_id. type = %d notifier=%p\r\n", type, notifier);

    if ((type & SIO_APP_TYPE_UART_AT_CMD) == SIO_APP_TYPE_UART_AT_CMD) {
        app_id = 1;
    }
    
#ifdef __CMUX_SUPPORT__
    channel_type = sio_app_type_to_cmux_type(type);        
    channel_id = cmux_register_callback(channel_type, (cmux_callback_t)notifier);
#endif /* __CMUX_SUPPORT__ */

    // app_id = type(4) | channel(4);    0X1a mean: app is uart type and channel id = a
    app_id = (app_id << 4) | channel_id;

    sio_uart_set_notification_callback(1, app_id, SIO_UART_EVENT_MAX_NUMBER, notifier);
    LOGI("sio_register_app_id. app_id = %d \r\n", app_id);
    return app_id;
}

void sio_add_modem_response(const char* modem_response, uint8_t is_end)
{
    sio_uart_add_modem_response(modem_response, is_end);
}


uint8_t sio_modem_is_wakeup()
{    
    return sio_cnt.set_modem_wakeup;
}


void sio_trigger_modem_wakeup()
{    
    LOGI("sio_trigger_modem_wakeup %d", sio_cnt.sleep_manager_handler);
#ifdef MODEM_SUPPORT_DTE  
    hal_uart_set_dte_dtr_active(sio_uart_get_modem_port());
    vTaskDelay(10);
#endif
    sio_cnt.set_modem_wakeup = 1;
}

void sio_trigger_modem_sleep()
{    
    LOGI("sio_trigger_modem_sleep %d", sio_cnt.sleep_manager_handler);
#ifdef MODEM_SUPPORT_DTE  
    hal_uart_set_dte_dtr_deactive(sio_uart_get_modem_port());
    vTaskDelay(10);
#endif
    sio_cnt.set_modem_wakeup = 0;
}


sio_ret_t sio_trigger_modem_ready()
{
    const char *data = "at\r\n";
    int32_t size = strlen((char *)data);

    LOGI("enter");

    if (sio_uart_get_status() == SIO_UART_STATUS_READY) {
        return SIO_RET_OK;
    }
    else if (sio_uart_get_status() == SIO_UART_STATUS_INIT) {
        sio_uart_set_trigger_modem_ready();
        sio_uart_port_write_data(sio_uart_get_modem_port(), (uint8_t *)data, size);
        return SIO_RET_BUSY;
    }
    else {
        return SIO_RET_ERROR;
    }
}


