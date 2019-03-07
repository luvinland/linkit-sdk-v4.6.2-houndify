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

#include "atci.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gnss_log.h"
#include "hal_rtc.h"
#include "gnss_app.h"
#include "slp_app.h"
#include "epo_demo.h"
#include "wepodownload.h"
#include "sntp.h"
#include "serial_port.h"
#include "atci_main.h"
#include "gnss_bridge.h"
#include "sio_gprot.h"
#ifdef MODEM_ENABLE
#include "sio_gprot.h"
#include "urc_app.h"
#include "gprs_api.h"
#endif

atci_status_t gnss_test_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_epo_erase_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_epo_set_time_at_handler(atci_parse_cmd_param_t *parse_cmd);

atci_status_t gnss_test_at_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t output = {{0}};

    GNSSLOGD("gnss_test_at_handler:%s\n", parse_cmd->string_ptr);
    
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE:
            //gnss_demo_app_create();
            //gnss_demo_app_config(1, NULL);
            gnss_demo_app_restart();
            strcpy((char*) output.response_buf, "OK");
            output.response_len = strlen((char*) output.response_buf);
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
            atci_send_response(&output);
            break;
        default :
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&output);
            break;
    }
    return ATCI_STATUS_OK;
}

atci_status_t gnss_agps_at_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t output = {{0}};

    GNSSLOGD("gnss_agps_at_handler:%s\n", parse_cmd->string_ptr);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE:
            if (strstr((char *)parse_cmd->string_ptr, "AGPST")) {
                epo_init(2, NULL, NULL);
                sntp_setservername(0, "115.28.122.198");
                sntp_setservername(0, "218.189.210.3");
                sntp_init();
                gnss_location_get_cell_info();
            }else if (strstr((char *)parse_cmd->string_ptr, "AGPSEPO")) {
                epo_init(2, NULL, NULL);
            } else if (strstr((char *)parse_cmd->string_ptr, "AGPSNTP")) {
                sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);
                sntp_setservername(0, "115.28.122.198");
                sntp_setservername(0, "218.189.210.3");
                sntp_init();
            } else if (strstr((char *)parse_cmd->string_ptr, "AGPSSLP")) {
                gnss_location_get_cell_info();
            } else if (strstr((char *)parse_cmd->string_ptr, "POWERGPS")) {
#if defined(MTK_ATCI_VIA_PORT_SERVICE) && defined(MTK_PORT_SERVICE_ENABLE) && defined(GNSS_SUPPORT_TOOL_BRIDGE)
                serial_port_dev_t port;
                serial_port_setting_uart_t uart_setting;
            
                if (serial_port_config_read_dev_number("atci", &port) != SERIAL_PORT_STATUS_OK)
                {
                    port = SERIAL_PORT_DEV_USB_COM1;//SERIAL_PORT_DEV_UART_1;
                    serial_port_config_write_dev_number("atci", port);
                    LOG_W(common, "serial_port_config_write_dev_number setting uart1");
                    uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
                    serial_port_config_write_dev_setting(port, (serial_port_dev_setting_t *)&uart_setting);
                }
                vTaskDelay(500);
                atci_deinit(port);
                vTaskDelay(500);
                gnss_bridge_port_reinit(port);
                vTaskDelay(500);
                return ATCI_STATUS_OK;
#endif
            }
            strcpy((char*) output.response_buf, "OK");
            output.response_len = strlen((char*) output.response_buf);
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
            atci_send_response(&output);
            break;
        default :
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&output);
            break;
    }
    return ATCI_STATUS_OK;
}

atci_status_t gnss_gps_at_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t output = {{0}};
    char* tag;
    GNSSLOGD("gnss_gps_at_handler:%s\n", parse_cmd->string_ptr);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION:
            if ((tag = strstr((char *)parse_cmd->string_ptr, "EGPSC"))) {
                if (tag[11] == '1') {
                    gnss_demo_app_start();
                } else if (tag[11] == '0') {
                    gnss_demo_app_stop();
                }
            }else if ((tag = strstr((char *)parse_cmd->string_ptr, "EGPSS"))) {
                gnss_demo_app_send_cmd((int8_t*) (tag + 11), strlen(tag + 11));
            }
            strcpy((char*) output.response_buf, "OK");
            output.response_len = strlen((char*) output.response_buf);
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
            atci_send_response(&output);
            break;
        default :
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&output);
            break;
    }
    return ATCI_STATUS_OK;
}

atci_status_t gnss_epo_erase_at_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t output = {{0}};

    GNSSLOGD("gnss_epo_erase_at_handler:%s\n", parse_cmd->string_ptr);
    
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE:
            epo_demo_init_mem_info();
            epo_demo_flash_erase();
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&output);
            break;
        default :
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&output);
            break;
    }
    return ATCI_STATUS_OK;
}

atci_status_t gnss_epo_set_time_at_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t output = {{0}};

    GNSSLOGD("gnss_epo_set_time_at_handler:%s\n", parse_cmd->string_ptr);
    
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION:
        {
            uint8_t year_p[5] = {0};
            uint8_t month_p[3] = {0};
            uint8_t day_p[3] = {0};
            uint8_t hour_p[3] = {0};
            uint8_t min_p[3] = {0};
            uint8_t sec_p[3] = {0};
            uint8_t week_p[2] = {0};

            hal_rtc_time_t rtc_time = {0};
            hal_rtc_time_t rtc_time_org = {0};

            memcpy(year_p, parse_cmd->string_ptr + 12, 4);
            memcpy(month_p, parse_cmd->string_ptr + 17, 2);
            memcpy(day_p, parse_cmd->string_ptr + 20, 2);
            memcpy(hour_p, parse_cmd->string_ptr + 23, 2);
            memcpy(min_p, parse_cmd->string_ptr + 26, 2);
            memcpy(sec_p, parse_cmd->string_ptr + 29, 2);
            memcpy(week_p, parse_cmd->string_ptr + 32, 1);

            rtc_time.rtc_year = (year_p[2] - '0') * 10 + (year_p[3] - '0');
            rtc_time.rtc_mon = (month_p[0] - '0') * 10 + (month_p[1] - '0');
            rtc_time.rtc_day = (day_p[0] - '0') * 10 + (day_p[1] - '0');
            rtc_time.rtc_hour = (hour_p[0] - '0') * 10 + (hour_p[1] - '0');
            rtc_time.rtc_min = (min_p[0] - '0') * 10 + (min_p[1] - '0');
            rtc_time.rtc_sec = (sec_p[0] - '0') * 10 + (sec_p[1] - '0');
            rtc_time.rtc_week = (week_p[0] - '0');

            GNSSLOGD("set data: 20%d,%d,%d,%d,%d,%d,%d\n",rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day, rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec, rtc_time.rtc_week);
            hal_rtc_get_time(&rtc_time_org);
            GNSSLOGD("get data: %d,%d,%d,%d,%d,%d,%d\n",rtc_time_org.rtc_year, rtc_time_org.rtc_mon, rtc_time_org.rtc_day, rtc_time_org.rtc_hour, rtc_time_org.rtc_min, rtc_time_org.rtc_sec, rtc_time_org.rtc_week);
            hal_rtc_set_time(&rtc_time);
            hal_rtc_get_time(&rtc_time_org);
            GNSSLOGD("get data: %d,%d,%d,%d,%d,%d,%d\n",rtc_time_org.rtc_year, rtc_time_org.rtc_mon, rtc_time_org.rtc_day, rtc_time_org.rtc_hour, rtc_time_org.rtc_min, rtc_time_org.rtc_sec, rtc_time_org.rtc_week);
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&output);
            break;
        }
        default :
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&output);
            break;
    }
    return ATCI_STATUS_OK;
}


static sclp_cell_location_list location;

atci_status_t gnss_loc_set_at_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t output = {{0}};

    GNSSLOGD("gnss_loc_set_at_handler:%s\n", parse_cmd->string_ptr);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION:
        {
            char lat[10], lng[10], alt[10], acc[10];
            location.num = 1;
            location.locations[0].result = SCLP_RESULT_SUCCESS;
            sscanf(parse_cmd->string_ptr + 11,"%[^,],%[^,],%[^,],%[^\r\n]", 
                lat,
                lng,
                alt,
                acc);
            GNSSLOGD("re AT+EGPSLOC=%s,%s,%s,%s",
                lat,
                lng,
                alt,
                acc);
            
            location.locations[0].lat = atof(lat);
            location.locations[0].lng = atof(lng);
            location.locations[0].alt = atoi(alt);
            location.locations[0].acc = atoi(acc);
            GNSSLOGD("re 2 AT+EGPSLOC=%f,%f,%d,%d",
                (double) location.locations[0].lat,
                (double) location.locations[0].lng,
                location.locations[0].alt,
                location.locations[0].acc);

            gnss_location_get_cell_info_result(&location);
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&output);
            break;
        }
        default :
            output.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&output);
            break;
    }
    return ATCI_STATUS_OK;
}


