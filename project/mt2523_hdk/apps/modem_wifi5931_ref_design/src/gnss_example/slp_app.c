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

#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#ifdef MODEM_ENABLE
#include "sio_gprot.h"
#include "urc_app.h"
#include "gprs_api.h"
#endif
#include "gnss_app.h"
#include "gnss_log.h"
#include "slp_api.h"
#include "gnss_ring_buffer.h"
#include "sclp_main.h"
#include "hal_rtc.h"

#define MAX_AT_RESPONSE_LEN 200
#define GNSS_EVENT_URC SIO_UART_EVENT_MAX_NUMBER + 1000
#define URC_RING_BUFFER_SIZE 2000


typedef void (*sentence_process_f)(char* sentence);

typedef enum {
    GNSS_LOCATION_COPS_INFO,
    GNSS_LOCATION_CGREG_INFO,
    GNSS_LOCATION_ENBR_INFO,
    GNSS_LOCATION_TIME_INFO,
    GNSS_LOCATION_INFO_MAX
} gnss_location_sentence;
struct {
    ring_buf_struct_t ring_buf;
    uint8_t buff[URC_RING_BUFFER_SIZE];
    uint8_t curr_p[MAX_AT_RESPONSE_LEN];
    sclp_cell_info_list cell_info;
    short mnc, mcc;
    uint32_t update_status;
    char supl_server[50];
} at_response_buf;
static sclp_cell_location_list loc_result;

const char     *g_cust_ssl_peer_cn = "supl.nokia.com";
const char     supl_ca_cer[] = 
"-----BEGIN CERTIFICATE-----\r\n"
"MIICPDCCAaUCEHC65B0Q2Sk0tjjKewPMur8wDQYJKoZIhvcNAQECBQAwXzELMAkG\r\n"
"A1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFz\r\n"
"cyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTk2\r\n"
"MDEyOTAwMDAwMFoXDTI4MDgwMTIzNTk1OVowXzELMAkGA1UEBhMCVVMxFzAVBgNV\r\n"
"BAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFzcyAzIFB1YmxpYyBQcmlt\r\n"
"YXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGfMA0GCSqGSIb3DQEBAQUAA4GN\r\n"
"ADCBiQKBgQDJXFme8huKARS0EN8EQNvjV69qRUCPhAwL0TPZ2RHP7gJYHyX3KqhE\r\n"
"BarsAx94f56TuZoAqiN91qyFomNFx3InzPRMxnVx0jnvT0Lwdd8KkMaOIG+YD/is\r\n"
"I19wKTakyYbnsZogy1Olhec9vn2a/iRFM9x2Fe0PonFkTGUugWhFpwIDAQABMA0G\r\n"
"CSqGSIb3DQEBAgUAA4GBALtMEivPLCYATxQT3ab7/AoRhIzzKBxnki98tsX63/Do\r\n"
"lbwdj2wsqFHMc9ikwFPwTtYmwHYBV4GSXiHx0bH/59AhWM1pF+NEHJwZRDmJXNyc\r\n"
"AA9WjQKZ7aKQRUzkuxCkPfAyAw7xzvjoyVGM5mKf5p/AfbdynMk2OmufTqj/ZA1k\r\n"
"-----END CERTIFICATE-----\r\n";

int32_t gnss_set_app_id(int32_t app_id)
{
    static int32_t gnss_app_id;
    int32_t old_app_id = gnss_app_id;
    gnss_app_id = app_id;
    GNSSLOGD("set app id old:%d, new:%d\n", old_app_id, app_id);
    return old_app_id;
}

int32_t gnss_get_app_id()
{
    int32_t app_id;
    app_id = gnss_set_app_id(0);
    gnss_set_app_id(app_id);
    GNSSLOGD("get app id:%d\n", app_id);
    return app_id;
}

int8_t gnss_set_modem_status(int8_t status)
{
    static int8_t modem_status;
    int8_t old_status;
    old_status = modem_status;
    modem_status = status;
    GNSSLOGD("set modem status, old:%d, new:%d\n", old_status, status);
    return old_status;
}

int8_t gnss_get_modem_status()
{
    int8_t modem_status;
    modem_status = gnss_set_modem_status(0);
    gnss_set_modem_status(modem_status);
    GNSSLOGD("get modem status:%d\n", modem_status);
    return modem_status;
}

char* gnss_get_supl_server_name()
{
    return at_response_buf.supl_server;
}

void gnss_set_supl_server_name(char* host_name)
{
    strncpy((char*) at_response_buf.supl_server, (char*) host_name, sizeof(at_response_buf.supl_server));
}

int8_t gnss_set_channel_status(int8_t status)
{
    static int8_t channel_status;
    int8_t old_status;
    old_status = channel_status;
    channel_status = status;
    GNSSLOGD("set channel status, old:%d, new:%d\n", old_status, status);
    return old_status;
}

int8_t gnss_get_channel_status()
{
    int8_t channel_status;
    channel_status = gnss_set_modem_status(0);
    gnss_set_modem_status(channel_status);
    GNSSLOGD("get channel status:%d\n", channel_status);
    return channel_status;
}

void gnss_update_cell_info_status(gnss_location_sentence updated_sentence)
{
    sclp_config slp_config;
    GNSSLOGD("update_cell_info, [%d]\n", updated_sentence);
    at_response_buf.update_status |= (0x01 << updated_sentence);
    if (at_response_buf.update_status & (0x01 << GNSS_LOCATION_COPS_INFO) 
        && at_response_buf.update_status & (0x01 << GNSS_LOCATION_CGREG_INFO)) {
        if (SCLP_CELL_GSM == at_response_buf.cell_info.cells[0].cell_type) {
            GNSSLOGD("slp start, cell_type[%d], mcc[%d],mnc[%d],lac[%d],cid[%d]\n", 
                at_response_buf.cell_info.cells[0].cell_type,
                at_response_buf.cell_info.cells[0].cell.gsm_cell.mcc,
                at_response_buf.cell_info.cells[0].cell.gsm_cell.mnc,
                at_response_buf.cell_info.cells[0].cell.gsm_cell.lac,
                at_response_buf.cell_info.cells[0].cell.gsm_cell.cid);
            //slp_start2("supl.nokia.com", 7275, 1, &at_response_buf.cell_info);
        } else {
            GNSSLOGD("slp start, cell_type[%d], mcc[%d],mnc[%d],lac[%d],uc[%d]\n", 
                at_response_buf.cell_info.cells[0].cell_type,
                at_response_buf.cell_info.cells[0].cell.wcdma_cell.mcc,
                at_response_buf.cell_info.cells[0].cell.wcdma_cell.mnc,
                at_response_buf.cell_info.cells[0].cell.wcdma_cell.lac,
                at_response_buf.cell_info.cells[0].cell.wcdma_cell.uc);
            //slp_start2("supl.nokia.com", 7275, 1, &at_response_buf.cell_info);
        }
        sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);

        //slp_start2("52.74.215.168", 7275, 1, &at_response_buf.cell_info);
        //slp_start2("supl.nokia.com", 7275, 1, &at_response_buf.cell_info);
        //slp_start2("211.151.53.216", 7275, 1, &at_response_buf.cell_info);
        //slp_start2("60.205.14.168", 7275, 1, &at_response_buf.cell_info);
        slp_config.cert_file = supl_ca_cer;
        slp_config.peer_name = g_cust_ssl_peer_cn;
        slp_config.has_profile = 1;
        strcpy(slp_config.profile.addr, "211.151.53.216");
        slp_config.profile.port = 7275;
        slp_config.profile.tls_enable = 1;
        slp_config.has_tls_version = 0;
        slp_config.tls_version = SCLP_TLS_VER_TLS_1_2;
        slp_config.has_supl_version = 1;
        slp_config.supl_version = 2;
        slp_start3(&slp_config, &at_response_buf.cell_info);
        at_response_buf.update_status = 0;
    }
}

int8_t gnss_is_cell_info_finish_update()
{
    return at_response_buf.update_status == ((0x01 << GNSS_LOCATION_COPS_INFO) | 
                                            (0x01 << GNSS_LOCATION_CGREG_INFO) | 
                                            (0x01 << GNSS_LOCATION_ENBR_INFO));
}

uint8_t *gnss_get_command_string(int32_t index)
{
    const uint8_t *command_string[7] =
        {
            (uint8_t *) "AT+COPS=3,2\r\n",
            (uint8_t *) "AT+COPS?\r\n",
            (uint8_t *) "AT+CGREG=2\r\n",
            (uint8_t *) "AT+ENBR\r\n",
            (uint8_t *) "AT+CTZR=1\r\n",
            (uint8_t *) "AT+CTZR?\r\n",
            NULL
        };
    if (index <= 7) {
        return (uint8_t *) command_string[index];
    } else {
        return NULL;
    }
}

uint8_t *gnss_get_response_string_prefix(int32_t index)
{
    const uint8_t *prefix_string[GNSS_LOCATION_INFO_MAX + 1] =
        {
            (uint8_t *) "+COPS",
            (uint8_t *) "+CGREG",
            (uint8_t *) "+ENBR",
            (uint8_t *) "+CIEV",
            NULL
        };
    if (index <= GNSS_LOCATION_INFO_MAX) {
        return (uint8_t *) prefix_string[index];
    } else {
        return NULL;
    }
}

void gnss_location_send_command()
{
    int32_t i = 0;
    uint8_t *command;
    GNSSLOGD("gnss_location_send_command\n");
    while ((command = gnss_get_command_string(i++))) {
        GNSSLOGD("cmd:%s\n", command);
        sio_send_data(gnss_get_app_id(), command, strlen((char*) command));
    } 
}

void gnss_location_parse_at_response(uint8_t* buff, int32_t len)
{
    buff[len] = 0;
    GNSSLOGD("%s", buff);
    //gnss_location_send_command();
}

void gnss_sio_event_handler(uint32_t event, void *data)
{
    gnss_send_msg(GNSS_ENUM_SIO_EVENT, (int32_t) event, data);
}

char* gnss_get_param(char* sentence, int32_t i)
{
    char* param;
    if ((param = strstr(sentence, ":"))) {
        param++;
        while (--i && (param = strstr(param, ","))) {
            param++;
        }
        if (i == 0) {
            return param;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

int32_t gnss_atoi_16(char* str)
{
    int32_t ret = 0;
    GNSSLOGD("to16:%s\n", str);
    if (str) {
        while ((*str && *str == ' ') || (*str <= '\t') || (*str <= '\"')) {
            //do nothing
            str++;
        }
        while(*str) {
            if (*str >= '0' && *str <= '9') {
                ret = ret << 4;
                ret += (*str - '0');
            } else if (*str >= 'A' && *str <= 'F') {
                ret = ret << 4;
                ret += (*str - 'A') + 10;
            } else if (*str >= 'a' && *str <= 'f') {
                ret = ret << 4;
                ret += (*str - 'a') + 10;
            } else {
                break;
            }
            str++;
        }
    }
    return ret;
}

void gnss_location_urc_cops_process(char* sentence)
{
    int32_t mode, fomat, cpos = 0;
    int32_t idx = 0;
    GNSSLOGD("gnss_location_urc_cops_process\n");
    mode = atoi(gnss_get_param(sentence, 1));
    fomat = atoi(gnss_get_param(sentence, 2));
    if (fomat == 2) {
        cpos = atoi(strstr(gnss_get_param(sentence, 3), "\"") + 1);
    } else if (fomat == 1 || fomat == 2) {
        GNSSLOGD("fomat is wrong\n");
    }
    GNSSLOGD("mode[%d] fomat[%d] cpos[%d]", mode, fomat, cpos);
    at_response_buf.mcc = cpos / 100;
    at_response_buf.mnc = cpos - (at_response_buf.mcc * 100);
    while (idx < at_response_buf.cell_info.num) {
        at_response_buf.cell_info.cells[idx].cell.gsm_cell.mcc = at_response_buf.mcc;
        at_response_buf.cell_info.cells[idx].cell.gsm_cell.mnc = at_response_buf.mnc;
        idx++;
    }
    gnss_update_cell_info_status(GNSS_LOCATION_COPS_INFO);
}

void gnss_location_urc_cgreg_process(char* sentence)
{
    int32_t status, act;
    sclp_cell_info *cell;

    GNSSLOGD("gnss_location_urc_cgreg_process\n");
    at_response_buf.cell_info.num = 0;
    cell = &at_response_buf.cell_info.cells[at_response_buf.cell_info.num];
    status = atoi(gnss_get_param(sentence, 1));

    GNSSLOGD("status:%d\n", status);

    act = atoi(gnss_get_param(sentence, 4));

    if ( act == 3 || act == 1) {
        //means GSM cell info
        cell->cell_type = SCLP_CELL_GSM;
        cell->cell.gsm_cell.lac = gnss_atoi_16(gnss_get_param(sentence, 2));
        cell->cell.gsm_cell.cid = gnss_atoi_16(gnss_get_param(sentence, 3));
        cell->cell.gsm_cell.mcc = at_response_buf.mcc;
        cell->cell.gsm_cell.mnc = at_response_buf.mnc;
        GNSSLOGD("GSM: mcc:%d, mnc:%d, lac:%d, cid%d\n", 
            cell->cell.gsm_cell.mcc, 
            cell->cell.gsm_cell.mnc,
            cell->cell.gsm_cell.lac,
            cell->cell.gsm_cell.cid);
    } else if (act == 2 || act == 4 || act == 5 || act == 6) {
        cell->cell_type = SCLP_CELL_WCDMA;
        cell->cell.wcdma_cell.lac = gnss_atoi_16(gnss_get_param(sentence, 2));
        cell->cell.wcdma_cell.uc = gnss_atoi_16(gnss_get_param(sentence, 3));
        cell->cell.wcdma_cell.mcc = at_response_buf.mcc;
        cell->cell.wcdma_cell.mnc = at_response_buf.mnc;
        GNSSLOGD("WCDMA: mcc:%d, mnc:%d, lac:%d, uc%d\n", 
            cell->cell.wcdma_cell.mcc, 
            cell->cell.wcdma_cell.mnc,
            cell->cell.wcdma_cell.lac,
            cell->cell.wcdma_cell.uc);
    }

    at_response_buf.cell_info.num++;
    gnss_update_cell_info_status(GNSS_LOCATION_CGREG_INFO);
}
void gnss_location_urc_enbr_process(char* sentence)
{
    int32_t type, rssi = -1, psc = -1;
    short lac = -1, cid = -1;

    GNSSLOGD("gnss_location_urc_cgreg_process\n");

    type = atoi(gnss_get_param(sentence, 1));
    if (type == 1) {
        rssi = atoi(gnss_get_param(sentence, 2));
        cid = atoi(gnss_get_param(sentence, 3));
        lac = atoi(gnss_get_param(sentence, 4));
    } else if (type == 2) {
        rssi = atoi(gnss_get_param(sentence, 2));
        psc = atoi(gnss_get_param(sentence, 3));
    }
    GNSSLOGD("rssi:%d, cid:%d, lac:%d, psc:%d\n", rssi, cid, lac, psc);
    gnss_update_cell_info_status(GNSS_LOCATION_ENBR_INFO);
}

void gnss_location_urc_ciev_process(char* sentence)
{
#if 1
    int32_t ind;
    char* datatime;
    int32_t year, mon, day, hour, min, sec;
    hal_rtc_time_t rtc_time = {0};
    GNSSLOGD("gnss_location_urc_ciev_process\n");
    ind = atoi(gnss_get_param(sentence, 1));
    if (ind == 9) {
        datatime = gnss_get_param(sentence, 2);
        datatime = strstr(datatime, "\"") + 1;
        GNSSLOGD("datatime:%s\n", datatime);
        if (datatime) {
            sscanf(datatime, "%d/%d/%d,%d:%d:%d",
                (int*) &year, (int*) &mon, (int*) &day, (int*) &hour, (int*) &min, (int*) &sec);
            GNSSLOGD("NITZ time:%d/%d/%d,%d:%d:%d\n", (int) year, (int) mon, (int) day, (int) hour, (int) min, (int) sec);
            rtc_time.rtc_year = year - (year/100*100);
            rtc_time.rtc_mon = mon;
            rtc_time.rtc_day = day;
            rtc_time.rtc_hour = hour;
            rtc_time.rtc_min = min;
            rtc_time.rtc_sec = sec;
            //rtc_time.rtc_week = ?
            hal_rtc_set_time(&rtc_time);
        } else {
            GNSSLOGD("no time content\n");
        }
    } else {
        GNSSLOGD("wrong urc\n");
    }
#endif
}

sentence_process_f gnss_get_process_func(int32_t index)
{
    const sentence_process_f process_func[GNSS_LOCATION_INFO_MAX + 1] =
        {
            gnss_location_urc_cops_process,
            gnss_location_urc_cgreg_process,
            gnss_location_urc_enbr_process,
            gnss_location_urc_ciev_process,
            NULL
        };
    if (index <= GNSS_LOCATION_INFO_MAX) {
        return process_func[index];
    } else {
        return NULL;
    }
}

void gnss_location_urc_sentence_process(char* sentence)
{
    int32_t i = 0;
    char* prefix;
    sentence_process_f process_func;
    GNSSLOGD("gnss_location_urc_sentence_process\n");
    GNSSLOGD("%s\n", sentence);
    while((prefix = (char*)gnss_get_response_string_prefix(i))) {
        if ((strstr(sentence, prefix))) {
            process_func = gnss_get_process_func(i);
            if (process_func) {
                GNSSLOGD("processing\n");
                process_func(sentence);
            }
        }
        i++;
    }
}

void gnss_location_urc_process()
{
#define SENTENCE_NOT_START 0
#define SENTENCE_START 1
#define SENTENCE_END 2

    uint8_t tempBuff[30 + 1] = {0};
    int32_t read_ret = 0;
    uint8_t *pbuf = tempBuff;
    static uint32_t status;
    uint8_t *sentence_buf = at_response_buf.curr_p;
    uint8_t *sentence_buf_end = at_response_buf.curr_p + MAX_AT_RESPONSE_LEN - 1;
    
    GNSSLOGD("gnss_location_urc_process\n");
    
    do {
        if (*pbuf == 0) {
            read_ret = consume_data(&at_response_buf.ring_buf, (int8_t*)tempBuff, 30);
            tempBuff[read_ret] = 0;
            pbuf = tempBuff;
            //GNSSLOGD("tempBuff:%s\n",tempBuff);
        }

        switch (status) {
            case SENTENCE_NOT_START:
                while(*pbuf && *pbuf != '+') {
                    pbuf++;
                }
                if (!(*pbuf)) {
                    continue;
                }
                status = SENTENCE_START;
            case SENTENCE_START:
                while(*pbuf && *pbuf != '\n') {
                    if (sentence_buf < sentence_buf_end - 1) {
                        *sentence_buf = *pbuf;
                        sentence_buf++;
                    }
                    pbuf++;
                }
                if (!(*pbuf)) {
                    //GNSSLOGD("sentence fregment, need more data, continue\n");
                    continue;
                } else {
                    *sentence_buf = 0;
                    gnss_location_urc_sentence_process((char*) at_response_buf.curr_p);
                    sentence_buf = at_response_buf.curr_p;
                    status = SENTENCE_NOT_START;
                }
                break;
            default:
                //GNSSLOGD("parse status error:%d\n",status);
                break;
        }
    } while(read_ret >= 30);
}
void gnss_sio_event_handler_int(uint32_t event, void *data)
{
    GNSSLOGD("gnss_sio_event_handler_int, event:%d\n", event);
    switch (event) {
        case SIO_UART_EVENT_MODEM_READY:
            gnss_set_modem_status(1);
            break;
#ifdef __CMUX_SUPPORT__
        case CMUX_EVENT_CHANNEL_CONNECTION:
            gnss_set_channel_status(1);
            break;
        case CMUX_EVENT_CHANNEL_DISCONNECTION:
            gnss_set_channel_status(0);
            break;
        case CMUX_EVENT_READY_TO_READ:
            break;
#endif /* __CMUX_SUPPORT__ */
        case SIO_UART_EVENT_MODEM_DATA_TO_READ:
            break;
        case GNSS_EVENT_URC:
            gnss_location_urc_process();
            break;
    }
}

urc_cb_ret  gnss_urc_callback(uint8_t *payload, uint32_t length)
{
    //GNSSLOGD("gnss_urc_callback:%s\n",payload);
    put_data(&at_response_buf.ring_buf, (int8_t*) payload, length);
    put_data(&at_response_buf.ring_buf, (int8_t*) "\r\n", strlen("\r\n") );
    gnss_send_msg(GNSS_ENUM_SIO_EVENT, GNSS_EVENT_URC, NULL);
    return RET_OK_CONTINUE;
}

void gnss_location_get_cell_info_result(sclp_cell_location_list* result)
{
    int32_t i = 0;
    const char* ref_loc_cmd = "PMTK713,%f,%f,%d,%d,%d,0,1200";
    char cmd_buf[100];
    if (!result) {
        GNSSLOGE("no result");
        return;
    }
    if (result->num <= 0) {
        GNSSLOGE("result num 0");
    }
    loc_result = *result;
    while (i < result->num) {
        if (result->locations[i].result == SCLP_RESULT_SUCCESS) {
            if (!gnss_get_aiding_flag(GNSS_LOC_AIDING_FLAG)) {
                GNSSLOGE("aiding flag not be set");
                return;
            }
            gnss_set_aiding_flag(GNSS_LOC_AIDING_FLAG, 0);
            sprintf(cmd_buf, ref_loc_cmd, 
                    (double) result->locations[i].lat,
                    (double) result->locations[i].lng,
                    result->locations[i].alt,
                    result->locations[i].acc,
                    result->locations[i].acc);
            GNSSLOGD("loc aiding:%s", cmd_buf);
            gnss_app_send_cmd((int8_t*) cmd_buf, strlen((char*) cmd_buf));
            return;
        }
        i++;
    }
}


void gnss_location_aiding()
{
    gnss_location_get_cell_info_result(&loc_result);
}

void gnss_location_init()
{
    int32_t app_id;
    GNSSLOGD("gnss_location_init\n");
    memset(&at_response_buf, 0, sizeof (at_response_buf));
    ring_buf_init(&at_response_buf.ring_buf, (int8_t*) &at_response_buf.buff, sizeof(at_response_buf.buff));
    //app_id = sio_register_event_notifier(SIO_APP_TYPE_CMUX_AT_CMD, gnss_sio_event_handler);
    app_id = sio_register_event_notifier(SIO_APP_TYPE_UART_AT_CMD, gnss_sio_event_handler);
    urc_register_callback(gnss_urc_callback);
    gnss_set_app_id(app_id);
    sclp_init(gnss_location_get_cell_info_result);
    //sio_set_mode(app_id, SIO_DATA_TYPE_COMMAND);
}

void gnss_location_get_cell_info()
{
    GNSSLOGD("gnss_location_get_cell_info\n");
    if (gnss_get_app_id() == 0) {
        gnss_location_init();
    }
    if (gnss_get_modem_status()) {
        gnss_location_send_command();
    } else {
        //error: modem is not ready
    }
}

