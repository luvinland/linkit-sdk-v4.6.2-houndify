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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "audio_player_main.h"
#include "syslog.h"
#include "audio_player.h"
#include "bt_sink_srv.h"
#include "hal_audio.h"
#include "atci.h"
#include "bt_sink_srv_utils.h"

#ifdef __LOCAL_PLAYER_ENABLE__


/*************************/

log_create_module(AudPlyATCI, PRINT_LEVEL_INFO);

//#define DEBUG_USE_PRINT
//#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define DEBUG_PRINT(fmt, arg...)    printf(fmt, ##arg)
#else
#define DEBUG_PRINT(fmt, arg...)
#endif

#ifndef DEBUG_USE_PRINT
#define LOGI(fmt, arg...)          LOG_I(AudPlyATCI, fmt, ##arg);
#define LOGW(fmt, arg...)          LOG_W(AudPlyATCI, fmt, ##arg);
#define LOGE(fmt, arg...)          LOG_E(AudPlyATCI, fmt, ##arg);
#else
#define LOGI(fmt, arg...)          DEBUG_PRINT(fmt, ##arg);
#define LOGW(fmt, arg...)          DEBUG_PRINT(fmt, ##arg);
#define LOGE(fmt, arg...)          DEBUG_PRINT(fmt, ##arg);
#endif


#define CMD_PARAM(s) s, strlen(s)

static atci_status_t audio_player_atci_handler(atci_parse_cmd_param_t *parse_cmd);

static atci_cmd_hdlr_item_t bt_sink_srv_atci_cmd[] = {
    {
        .command_head = "AT+EAUD",    /**< AT command string. */
        .command_hdlr = audio_player_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
};


static int32_t audio_player_get_hex(const char *ch)
{
    int32_t ret = -1;

    if (*ch >= '0' && *ch <= '9') {
        ret = *ch - '0';
    } else if (*ch >= 'a' && *ch <= 'f') {
        ret = *ch - 'a' + 0x0a;
    } else if (*ch >= 'A' && *ch <= 'F') {
        ret = *ch - 'A' + 0x0a;
    }

    return ret;
}


static void audio_player_handle_volume_setting(const char *string)
{
    uint32_t digital_index = 0, analog_index = 0;    
    int32_t ret = 0;
    int32_t i = 0;

    /* get digital_index */
    digital_index = 0;
    for (i = 0; i < 8; i++) {
        ret = audio_player_get_hex(string + 2 + i);
        if (ret < 0) {
            LOGI("handle_volume_setting--d_index error: %s\n", string);
            return;
        }
        digital_index = ((digital_index << 4) | ret);
    }

    analog_index = 0;
    for (i = 0; i < 4; i++) {
        ret = audio_player_get_hex(string + 13 + i);
        if (ret < 0) {
            LOGI("handle_volume_setting--a_index error: %s\n", string);
            return;
        }
        analog_index = ((analog_index << 4) | ret);
    }

    LOGI("[AudPly]volume_setting--di: 0x%x, ai: 0x%x\n", digital_index, analog_index);

    hal_audio_set_stream_out_volume(digital_index, analog_index);
}


static int16_t audio_player_cmd_entry(const char *string)
{
    LOGI("[AudPly]cmd_entry--string:%s", string);

   if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-PLAY"))) {
        audio_player_sink_key_action(BT_SINK_SRV_KEY_FUNC, BT_SINK_SRV_KEY_ACT_PRESS_UP);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-PAUSE"))) {
        audio_player_sink_key_action(BT_SINK_SRV_KEY_FUNC, BT_SINK_SRV_KEY_ACT_PRESS_UP);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-NEXT"))) {
        audio_player_sink_key_action(BT_SINK_SRV_KEY_NEXT, BT_SINK_SRV_KEY_ACT_LONG_PRESS_UP);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-PREV"))) {
        audio_player_sink_key_action(BT_SINK_SRV_KEY_PREV, BT_SINK_SRV_KEY_ACT_LONG_PRESS_UP);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-VOLUP"))) {
        audio_player_sink_key_action(BT_SINK_SRV_KEY_NEXT, BT_SINK_SRV_KEY_ACT_PRESS_UP);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-VOLDOWN"))) {
        audio_player_sink_key_action(BT_SINK_SRV_KEY_PREV, BT_SINK_SRV_KEY_ACT_PRESS_UP);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LOCAL-VOLUME"))) {
        /*****************
        AT+EAUD=LOCAL-VOLUME 0xFFFFFC18 0x044C
        *****************/
        audio_player_handle_volume_setting(string + (strlen("LOCAL-VOLUME") + 1));
    }

    return 0;
}


static atci_status_t audio_player_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    //bt_bd_addr_t local_address;
    int16_t result;

    response.response_flag = 0; /* Command Execute Finish. */

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+BTADDR=<BT address> */
            if (0 == strncmp(parse_cmd->string_ptr + 3, "EAUD", 4)) {
                result = audio_player_cmd_entry(parse_cmd->string_ptr + 8);
                if (result == 0) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    snprintf((char *)response.response_buf,
                             ATCI_UART_TX_FIFO_BUFFER_SIZE,
                             "command error:%d\r\n",
                             result);
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
            break;

        default :
            /* others are invalid command format */
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
            break;
    }
    return ATCI_STATUS_OK;
}


void audio_player_atci_init(void)
{
    atci_status_t ret = ATCI_STATUS_OK;

    ret = atci_register_handler(bt_sink_srv_atci_cmd, sizeof(bt_sink_srv_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));

    if (ret != ATCI_STATUS_OK) {
        LOGI("[AudPly]at_cmd_init register fail\r\n");
    }

    LOGI("[AudPly]atci_init--ret: %d\n", ret);
}

#endif /* __LOCAL_PLAYER_ENABLE__ */

