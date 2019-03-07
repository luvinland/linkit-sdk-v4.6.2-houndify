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
#include "psnetif.h"
#include "gprs_api.h"
#include "sio_gprot.h"
#include "sio_uart_gprot.h"
#include "string.h"
#include "ctype.h"
#include "syslog.h"
#include "nw_gprs.h"
#include "urc_app.h"
#include "queue.h"
#include "hal_sleep_manager.h"
#include "msm.h"
#include "atci.h"
#include "task.h"

#define ATE0_NEED

// TODO:
#ifndef MODEM_ON_HDK_ENABLE
//#define WAIT_CGREG_URC
#endif

#ifdef GPRS_PRINTF
#define LOGE(fmt,arg...)   printf(("[GPRS]: "fmt), ##arg)
#define LOGW(fmt,arg...)   printf(("[GPRS]: "fmt), ##arg)
#define LOGI(fmt,arg...)   printf(("[GPRS]: "fmt), ##arg)
#else
log_create_module(gprs, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(gprs, "[GPRS]: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(gprs, "[GPRS]: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(gprs ,"[GPRS]: "fmt,##arg)
#endif

#define GPRS_SEND_DATA_MAX_LENGTH   (100)
#define GPRS_MAX_IP_ADDR_LEN	(15)
#define GPRS_MIN_IP_ADDR_LEN	(7)
#define GPRS_MIN(a, b)	((a) > (b) ? (b) : (a))

#define GPRS_EVENT_URC  (SIO_UART_EVENT_MAX_NUMBER + 1)

typedef enum {
    GPRS_MODEM_STATE_NOT_READY,
    GPRS_MODEM_STATE_READY,
    GPRS_MODEM_STATE_EXCEPTION,

    GPRS_MODEM_STATE_MAX
} gprs_modem_state_enum;

typedef enum {
    GPRS_NW_MANAGE_STATE_NONE = 0x0,
    GPRS_NW_MANAGE_STATE_INITIALIZED = 0x0001,
    GPRS_NW_MANAGE_STATE_REGISTERED = 0x0002,
    GPRS_NW_MANAGE_STATE_ACTIVATED = 0x0004,
    GPRS_NW_MANAGE_STATE_INFO_SET = 0x0008,

    GPRS_NW_MANAGE_STATE_MAX = 0xFFFF
} gprs_nw_manage_state_enum;

typedef enum {
    GPRS_AT_CMD_TYPE_NONE,
        
    GPRS_AT_CMD_TYPE_ATE0,
    GPRS_AT_CMD_TYPE_ESLP_DISABLE_INIT,
    GPRS_AT_CMD_TYPE_ENABLE_DTR,
    GPRS_AT_CMD_TYPE_ENABLE_RI,
    GPRS_AT_CMD_TYPE_DISABLE_PWM,
    GPRS_AT_CMD_TYPE_SET_GPIO_OUTPUT,
    GPRS_AT_CMD_TYPE_SET_GPIO_HIGH,
    GPRS_AT_CMD_TYPE_SET_CTZR_ON,
    GPRS_AT_CMD_TYPE_CMEE,      
    GPRS_AT_CMD_TYPE_CMUX,    /* 10 */
    GPRS_AT_CMD_TYPE_EIND,
    GPRS_AT_CMD_TYPE_SET_CFUN_ON,
    GPRS_AT_CMD_TYPE_ESLP_ENABLE_INIT,
    
    GPRS_AT_CMD_TYPE_ESLP_DISABLE_REGISTRATION,
    GPRS_AT_CMD_TYPE_ERAT,
    GPRS_AT_CMD_TYPE_CGREG,
    GPRS_AT_CMD_TYPE_ESLP_ENABLE_REGISTRATION,
    
    GPRS_AT_CMD_TYPE_ESLP_DISABLE_ACTIVATE,
    GPRS_AT_CMD_TYPE_CDGCONT,
    GPRS_AT_CMD_TYPE_CDGCONT_UN,  /* 20 */
    GPRS_AT_CMD_TYPE_CGACT,		/* GPRS Active */
    GPRS_AT_CMD_TYPE_CGEREP,    /* Enable unsolicated result codes(URC) or not. */
    GPRS_AT_CMD_TYPE_CGDATA,
    GPRS_AT_CMD_TYPE_CGPADDR,
    GPRS_AT_CMD_TYPE_CGPRCO,
    GPRS_AT_CMD_TYPE_ESLP_ENABLE_ACTIVATE,
    
    GPRS_AT_CMD_TYPE_ESLP_DISABLE_DEACTIVATE,
    GPRS_AT_CMD_TYPE_CGDEACT,	/* GPRS Deactive */
    GPRS_AT_CMD_TYPE_ESLP_ENABLE_DEACTIVATE,    

    GPRS_AT_CMD_TYPE_MAX        /* 30 */
} gprs_at_cmd_type_enum;

typedef enum {
    GPRS_AT_CMD_CLASS_NONE,
    GPRS_AT_CMD_CLASS_INIT,
    GPRS_AT_CMD_CLASS_REGISTRATION,
    GPRS_AT_CMD_CLASS_ACTIVATE,
    GPRS_AT_CMD_CLASS_DEACTIVATE,

    GPRS_AT_CMD_CLASS_MAX
} gprs_at_cmd_class_enum;

typedef enum {
    GPRS_URC_TYPE_NONE,
    GPRS_URC_TYPE_CGEV,
    GPRS_URC_TYPE_CGREG,

    GPRS_URC_TYPE_MAX
} gprs_urc_type_enum;

    
typedef enum {
    CRGRG_STATUS_VALUE_0,  /* not registered, searching */
    CRGRG_STATUS_VALUE_1,  /* registered, home network */
    CRGRG_STATUS_VALUE_2,  /* not registered, trying */
    CRGRG_STATUS_VALUE_3,  /* registration denied */
    CRGRG_STATUS_VALUE_4,  /* Unknown */
    CRGRG_STATUS_VALUE_5,  /* registered, roaming */
    CRGRG_STATUS_VALUE_6,  /* "SMS only", home network */
    CRGRG_STATUS_VALUE_7,  /* "SMS only", roaming */
    CRGRG_STATUS_VALUE_8,  /* emergency bearer services only  */
    CRGRG_STATUS_VALUE_9,  /* registered for "CSFB not preferred" */
    CRGRG_STATUS_VALUE_10  /* registered for "CSFB not preferred" */
} gprs_cgreg_status_value_enum;


typedef enum {
    GPRS_MODEM_RESPONSE_UNKNOW = -2,
    GPRS_MODEM_RESPONSE_ERROR = -1,
    GPRS_MODEM_RESPONSE_ATE0 = 0,
    GPRS_MODEM_RESPONSE_OK,
    GPRS_MODEM_RESPONSE_CONNECT,    
    GPRS_MODEM_RESPONSE_CGPADDR,
    GPRS_MODEM_RESPONSE_CGPRCO,
    GPRS_MODEM_RESPONSE_CGEV_MEPDN,
    GPRS_MODEM_RESPONSE_CGEV_NWDEACT,    
} gprs_modem_response_type_enum;


typedef struct {
    char ipv4_addr[GPRS_MAX_IP_ADDR_LEN + 1];
    char dns_svr1[GPRS_MAX_IP_ADDR_LEN + 1];
    char dns_svr2[GPRS_MAX_IP_ADDR_LEN + 1];
} gprs_nw_info_struct;

typedef struct {
#define GPRS_MSM_DATA_MAX_LEN (60)
    uint8_t payload[GPRS_MSM_DATA_MAX_LEN];
    uint32_t length;
    void *userdata;
} gprs_msm_data_struct;

typedef struct gprs_nw_manage_struct{
    gprs_pdp_context_id_enum cid;
    gprs_nw_manage_state_enum nw_manage_state;
    gprs_nw_info_struct nw_info;
    struct gprs_nw_manage_struct* next;
} gprs_nw_manage_struct;


typedef struct {
    int app_id;
    gprs_modem_state_enum modem_state;
    gprs_nw_manage_struct *nw_manage;	/* Create at each activation, and free at each deactivation. */
    gprs_msm_data_struct noti_data;
    gprs_at_cmd_type_enum current_at_cmd_type;
    gprs_msm_data_struct cb_data;
    int need_send_cmd_after_cgreg;
    int cgreg_succeed;
    int creg_urc_success_received;

#ifdef __CMUX_SUPPORT__
    int need_wait_for_cmux_connect;
    int cmux_connected;
    int cgred_cmux_succeed;
#endif

// sleep mode manager
#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    uint8_t sleep_manager_handler;
#endif

    uint8_t is_running;
    gprs_pdp_context_id_enum current_cid;
    gprs_action_cb callback;
} gprs_context_struct;

typedef struct {
    gprs_pdp_context_id_enum cid;
    uint8_t is_used;
    uint8_t apn[GPRS_SUPPORT_APN_MAX_LENGTH];
} gprs_cid_struct;

gprs_context_struct *gprs_cntx;
gprs_cid_struct gprs_cid_array[GPRS_SUPPORT_APN_NUM];

static gprs_ret_t gprs_send_at_cmd(gprs_at_cmd_type_enum at_cmd_type);
#ifndef WAIT_CGREG_URC
static gprs_ret_t gprs_remove_nw_info_with_cid(gprs_pdp_context_id_enum cid);
#endif
static gprs_ret_t gprs_remove_nw_info(void);
static void gprs_notify_app(gprs_noti_type_enum noti_type);
static gprs_ret_t gprs_find_nw_manage_with_cid(gprs_pdp_context_id_enum cid, gprs_nw_manage_struct** nw_manage);
static gprs_action_type_enum gprs_get_action_by_class(gprs_at_cmd_class_enum cmd_class);

char *gprs_modem_response_ok[] = {
    "OK",
    "CONNECT",
    "+CGPADDR",
    "+CGPRCO",
    "+CGEV: ME PDN",
    "+CGEV: NW DEACT",
    NULL
};

char *gprs_modem_response_error[] = {
    "ERROR",
    NULL
};

char *gprs_modem_urc[] = {
    "+CGEV: NW DEACT",    // deactivate by GPRS
    "+CGREG: ",
    NULL
};


typedef struct {
    char *at_cmd;
    gprs_at_cmd_class_enum cmd_class;
    gprs_at_cmd_type_enum cmd_type;
} gprs_at_cmd_info_struct;

/* How to add new AT CMD
  * 1. Add new enum item in gprs_at_cmd_type_enum
  * 2. Add new item in gprs_at_cmds
  * 3. Add new case in gprs_send_at_cmd
  * 4. If AT CMD RSP for the new AT CMD is different from the existing ones,
  *     4.1 add new rsp in gprs_modem_response_ok or/and gprs_modem_response_err
  *     4.2 add new rsp in msm_uart_msg_handler() in msm_uart.c
  * 5. If MSM_UT is defined,
  *     5.1 add new at cmd in msm_ut_ready_status() in sio_ut.c
  *     5.2 If new AT CMD RSP for the new AT CMD, modify msm_ut_ready_status() in sio_ut.c
  *          to response with correct AT CMD RSP for the new AT CMD.
  */
gprs_at_cmd_info_struct gprs_at_cmds[] = {
    /* Init */
#ifdef ATE0_NEED
    {"ate0\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_ATE0},
#endif
    {"at+eslp=0\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_ESLP_DISABLE_INIT},
#ifdef MODEM_ON_HDK_ENABLE
    {"at+cgfunc=2,1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_ENABLE_DTR},
    {"at+cgfunc=3,1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_ENABLE_RI},
    {"at+cgfunc=6,0\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_DISABLE_PWM},
    {"at+cgdrt=53,1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_SET_GPIO_OUTPUT},
    {"at+cgsetv=53,1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_SET_GPIO_HIGH},
#endif
    {"at+ctzr=1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_SET_CTZR_ON},
    {"at+cmee=0\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_CMEE},
#ifdef __CMUX_SUPPORT__
    {"at+cmux=0\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_CMUX},
#endif
    {"at+eind=255\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_EIND},
    {"at+cfun=1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_SET_CFUN_ON},
    {"at+eslp=1\r\n", GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_ESLP_ENABLE_INIT},

    /* Registration */
    {"at+eslp=0\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_ESLP_DISABLE_REGISTRATION},
    {"at+erat=2,2\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_ERAT}, /* 3G preferred */
    //{"at+erat=1,2\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_ERAT},	/* 3G ONLY */
    {"at+cgreg=2\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_CGREG},
    {"at+cgdcont=%d,\"IP\",\"Internet\"\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_CDGCONT},
    //{"at+cgdcont=%d,\"IPV4V6\",\"internet\",\"\",0,0\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_CDGCONT_UN},
    //{"at+cgdcont=%d,\"IP\",\"3gnet\"\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_CDGCONT},
    {"at+cgact=1, %d\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_CGACT},
    {"at+eslp=1\r\n", GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_ESLP_ENABLE_REGISTRATION},
    
    /* Activate */
    {"at+eslp=0\r\n", GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_ESLP_DISABLE_ACTIVATE},
    {"at+cgerep=1\r\n", GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_CGEREP},
    {"at+cgdata=\"M-L4\",%d\r\n", GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_CGDATA},
    {"at+cgpaddr=%d\r\n", GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_CGPADDR},
    {"at+cgprco?\r\n", GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_CGPRCO},
    {"at+eslp=1\r\n", GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_ESLP_ENABLE_ACTIVATE},

    /* Deactivate */
    {"at+eslp=0\r\n", GPRS_AT_CMD_CLASS_DEACTIVATE, GPRS_AT_CMD_TYPE_ESLP_DISABLE_DEACTIVATE},
    {"at+cgact=0, %d\r\n", GPRS_AT_CMD_CLASS_DEACTIVATE, GPRS_AT_CMD_TYPE_CGDEACT},
    {"at+eslp=1\r\n", GPRS_AT_CMD_CLASS_DEACTIVATE, GPRS_AT_CMD_TYPE_ESLP_ENABLE_DEACTIVATE},

    /* Add new item above */
    {NULL, GPRS_AT_CMD_CLASS_NONE, GPRS_AT_CMD_TYPE_NONE}
};

/**
  * @brief     Find AT CMD by type
  * @param[in] gprs_at_cmd_type_enum at_cmd_type: AT CMD type
  * @retval    AT CMD
  */
static char *gprs_find_at_cmd_by_type(gprs_at_cmd_type_enum at_cmd_type);

/**
  * @brief     Find AT CMD class by type
  * @param[in] gprs_at_cmd_type_enum at_cmd_type: AT CMD type
  * @retval    AT CMD class
  */
static gprs_at_cmd_class_enum gprs_find_cmd_class_by_type(gprs_at_cmd_type_enum at_cmd_type);

static void gprs_action_callback(gprs_ret_t res, gprs_action_type_enum action);


/**
  * @brief     Parse data
  *            If the data matches one of the string in gprs_modem_response_ok, return GPRS_RET_OK;
  * If the data matches one of the string in gprs_modem_response_error, return GPRS_RET_ERROR;
  * @param[in] uint8_t *payload: data
  * @param[in] uint32_t length: data length
  * @retval    result
  */
static gprs_modem_response_type_enum gprs_parse_response_data(uint8_t *payload, uint32_t length)
{
    char res[16] = {0}, *str = NULL;
    int i = 0;

    if (!payload || !length) {
        return GPRS_RET_INVALID_PARAM;
    }

    str = (char *)payload;
    while (*str && i < 15 && i < length) {
        if (*str >= 'a' && *str <= 'z') {
            res[i++] = *str + 'A' - 'a';
        } else if (!('\r' == *str || '\n' == *str)) {
            res[i++] = *str;
        }

        str++;
    }

#ifdef ATE0_NEED
    if (strstr(res, "ATE0")) {
        if (strstr(res, "ATE0") || strstr(res, "OK")) {
            LOGI("ATE0 Parse result is OK\r\n");
            return GPRS_MODEM_RESPONSE_ATE0;   
        } else {
            return GPRS_MODEM_RESPONSE_ERROR;   
        }
    }
#endif

    i = 0;
    while (gprs_modem_response_ok[i]) {
        if (0 == strncmp(res, gprs_modem_response_ok[i], strlen(gprs_modem_response_ok[i]))) {
            LOGI("Parse result is OK\r\n");
            return (gprs_modem_response_type_enum)(i + 1);
        }

        i++;
    }

    i = 0;
    while (gprs_modem_response_error[i]) {
        if (0 == strncmp(res, gprs_modem_response_error[i], strlen(gprs_modem_response_error[i]))) {
            LOGI("Parse result is ERROR\r\n");
            return GPRS_MODEM_RESPONSE_ERROR;
        }

        i++;
    }

    return GPRS_MODEM_RESPONSE_UNKNOW;
}

#ifdef WAIT_CGREG_URC
/**
  * @brief     Parse URC data
  * @param[in] uint8_t *payload: data
  * @param[in] uint32_t length: data length
  * @param[in] uint8_t *urc_data: urc data
  * @retval    urc type
  */
static gprs_urc_type_enum gprs_parse_modem_urc(uint8_t *payload, uint32_t length, uint8_t *urc_data)
{
    char *str = NULL;
    int i = 0;

    LOGI("lenght %d", length);

    if (!payload || !length) {
        return GPRS_URC_TYPE_NONE;
    }

    str = (char*)payload;
    while (*str && i < 15 && i < length) {
        if (*str >= 'a' && *str <= 'z') {
            urc_data[i++] = *str + 'A' - 'a';
        } else if (!('\r' == *str || '\n' == *str)) {
            urc_data[i++] = *str;
        }

        str++;
    }

    LOGI("6280 URC: %s\r\n", urc_data);

    i = 0;
    while (gprs_modem_urc[i]) {
        if (0 == strncmp((const char*)urc_data, gprs_modem_urc[i], strlen((char*)gprs_modem_urc[i]))) {
            LOGI("URC type is %d\r\n", i + 1);
            return (gprs_urc_type_enum)(i + 1);
        }

        i++;
    }

    return GPRS_URC_TYPE_NONE;
}

#endif

/**
  * @brief     Release nw manage
  * @retval    none
  */
static void gprs_nw_manage_deinit(void)
{
    if (NULL == gprs_cntx) {
        return;
    }

    if (NULL == gprs_cntx->nw_manage) {
        return;
    }

    vPortFree(gprs_cntx->nw_manage);
    gprs_cntx->nw_manage = NULL;

    return;
}


/**
  * @brief     event handler, which is send by lower layer
  * @param[in] uint32_t event: event
  * @param[in] void *data: data
  * @retval    0, event will continue to be proccessed by another task.
 *           1, event will not be proccessed by another task afterwards.
 */
static void gprs_event_hdl(uint32_t event, void *data)
{
    LOGI("gprs_event_hdl() event=%d\r\n", event);

    if (NULL == gprs_cntx) {
        LOGI("gprs_event_hdl() return -1\r\n");
        return;
    }

    memset(&gprs_cntx->noti_data, 0, sizeof(gprs_cntx->noti_data));
    if (event == SIO_UART_EVENT_MODEM_DATA_TO_READ) {
        sio_rx_data_to_read_struct *rx_data_ind = (sio_rx_data_to_read_struct *)data;
        gprs_cntx->noti_data.length = GPRS_MIN(rx_data_ind->read_length, GPRS_MSM_DATA_MAX_LEN);
        if (gprs_cntx->noti_data.length == 0) {
            return;
        }
    }

    nw_demo_send_event(MESSAGE_ID_GPRS_NOTI_HDL, event, NULL);

    return;
}

/**
  * @brief     Get first AT CMD by class
  * @param[in] gprs_at_cmd_class_enum cmd_class: AT CMD class
  * @retval    gprs_at_cmd_type_enum
  */
gprs_at_cmd_type_enum gprs_get_1st_cmd_by_class(gprs_at_cmd_class_enum cmd_class)
{
    int i = 0;

    while (gprs_at_cmds[i].at_cmd) {
        if (cmd_class == gprs_at_cmds[i].cmd_class) {
            return gprs_at_cmds[i].cmd_type;
        }

        i++;
    }
    return GPRS_AT_CMD_TYPE_NONE;
}

/**
  * @brief     Get next AT CMD by class
  * @param[in] gprs_at_cmd_class_enum cmd_class: AT CMD class
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    gprs_at_cmd_type_enum
  */
gprs_at_cmd_type_enum gprs_get_next_at_cmd_by_class(gprs_at_cmd_class_enum cmd_class, gprs_at_cmd_type_enum cmd_type)
{
    int i = 0;

    while (gprs_at_cmds[i].at_cmd) {
        if (cmd_type == gprs_at_cmds[i].cmd_type) {
            if (cmd_class == gprs_at_cmds[i + 1].cmd_class) {
                return gprs_at_cmds[i + 1].cmd_type;
            }

            break;
        }

        i++;
    }

    return GPRS_AT_CMD_TYPE_NONE;
}


/**
  * @brief     Send AT CMD
  * @param[in] gprs_at_cmd_class_enum cmd_class: AT CMD class
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    GPRS_RET_OK = 0,
  *               GPRS_RET_ERROR = -1
  */
gprs_ret_t gprs_begin_sent_at_cmd_by_class(gprs_at_cmd_class_enum cmd_class, gprs_at_cmd_type_enum cmd_type_begin)
{
    gprs_at_cmd_type_enum cmd_type = GPRS_AT_CMD_TYPE_NONE;
    gprs_ret_t ret = GPRS_RET_ERROR;

    if (GPRS_AT_CMD_CLASS_NONE < cmd_class && GPRS_AT_CMD_CLASS_MAX > cmd_class) {
        if (GPRS_AT_CMD_TYPE_NONE == cmd_type_begin) {
            cmd_type = gprs_get_1st_cmd_by_class(cmd_class);
        } else {
            if (cmd_class != gprs_find_cmd_class_by_type(cmd_type_begin)) {
                LOGE("Type and class do not match. class:%d, type:%d \r\n", cmd_class, cmd_type_begin);
            } else {
                cmd_type = cmd_type_begin;
            }
        }

        if (GPRS_AT_CMD_TYPE_NONE != cmd_type) {
            ret = gprs_send_at_cmd(cmd_type);
            LOGI("gprs_begin_sent_at_cmd_by_class() class:%d, ret:%d \r\n", cmd_class, ret);
            return ret;
        }
    } 
    
    gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(cmd_class)); 
    return ret;
}


#ifdef WAIT_CGREG_URC

/**
  * @brief     Extract cgregister station
  * @param[in] uint8_t *payload: data
  * @param[in] uint32_t length: data length
  * @retval    Negative is fail.
  */
gprs_cgreg_status_value_enum gprs_extract_cgreg_sta(uint8_t *payload, uint32_t length)
{
    char *sta_start = NULL, *sta_end = NULL;
    char sta_str[4] = {0};	
    gprs_cgreg_status_value_enum sta_int = 0;

    LOGI("gprs_extract_cgreg_sta() payload:%s\r\n", payload);

    if (!payload || length < 9) {
        return -1;
    }

    sta_start = strchr((const char*)payload, ':');
    sta_start += 2;	

    sta_end = strchr(sta_start, ',');

    if (!sta_end) {
        return -2;
    }

    sta_end--; 	
    if (sta_start > sta_end || sta_end - sta_start > 2) {
        return -3;
    }

    strncpy(sta_str, sta_start, GPRS_MIN(3, sta_end - sta_start + 1));

    sta_int = (gprs_cgreg_status_value_enum)atoi(sta_str);

    LOGI("sta_str: %s, sta_int: %d\r\n", sta_str, sta_int);
    return sta_int;
}
#endif


/**
  * @brief     Process notifications coming from sio, such as exception, deactivation, ...
  * @param[in] int32_t event: event
  * @retval    None.
  */
void gprs_noti_hdl(int32_t event)
{
    gprs_nw_manage_struct* nw_manage = NULL;
#ifdef WAIT_CGREG_URC
    char urc_data[16] = {0};
    gprs_cgreg_status_value_enum sta_int = -1;
    gprs_at_cmd_type_enum cmd_type = GPRS_AT_CMD_TYPE_NONE;
#endif

    LOGI("gprs_noti_hdl(), event:%d\r\n", event);

    if (!gprs_cntx) {
        return;
    }

    switch (event) {
        case SIO_UART_EVENT_MODEM_READY: {
            const char *CME = "+CME";
            const char *CGPADDR = "+CGPADDR";
            const char *CGPRCO = "+CGPRCO";
            const char *CGEV = "+CGEV";
            const char *ATE = "ATE";
            
            LOGI("gprs_noti_hdl(), SIO_UART_EVENT_MODEM_READY \r\n");

            sio_add_modem_response(CME, 0);
            sio_add_modem_response(CGPADDR, 0);
            sio_add_modem_response(CGPRCO, 0);
            sio_add_modem_response(CGEV, 0);
            sio_add_modem_response(ATE, 1);
            
            gprs_cntx->modem_state = GPRS_MODEM_STATE_READY;
            gprs_notify_app(GPRS_NOTI_TYPE_MODEM_READY);

            gprs_initialize_modem();
            return;
        }
    #ifdef __CMUX_SUPPORT__
        case CMUX_EVENT_CHANNEL_CONNECTION: {
            LOGI("app get cmux channel connect.\r\n");
            gprs_cntx->cmux_connected = 1;
            if (gprs_cntx->need_wait_for_cmux_connect == 1) {
                gprs_cntx->need_wait_for_cmux_connect = 0;
                if (gprs_cntx->cgreg_succeed == 1) {
                    gprs_cntx->cgred_cmux_succeed = 1;
                    LOGI("cgred_cmux_succeed = 1 1\r\n");
                } else {
                    LOGI("app activate gprs.\r\n");
                    gprs_notify_app(GPRS_NOTI_TYPE_CMUX_CONN);
                }
            }

            return;
        }

        case CMUX_EVENT_CHANNEL_DISCONNECTION: {
            LOGI("app get cmux channel disconnect.\r\n");
            gprs_cntx->cmux_connected = 0;
            if (gprs_cntx->need_wait_for_cmux_connect == 1) {
                gprs_cntx->need_wait_for_cmux_connect = 0;
                if (gprs_cntx->cgreg_succeed == 1) {
                    gprs_cntx->cgred_cmux_succeed = 0;
                } else {
                    LOGI("app deactivate gprs cmux channel disconnect.\r\n");
                    gprs_notify_app(GPRS_NOTI_TYPE_CMUX_DISCONN);
                }
            }

            return;
        }
        case CMUX_EVENT_READY_TO_READ: {
            gprs_cntx->noti_data.length = sio_receive_data(gprs_cntx->app_id,
                                          gprs_cntx->noti_data.payload,
                                          GPRS_MSM_DATA_MAX_LEN);
            if (gprs_cntx->noti_data.length == 0) {
                return;
            }
            gprs_send_callback(gprs_cntx->noti_data.payload, gprs_cntx->noti_data.length, NULL);
            return;
        }
    #endif /* __CMUX_SUPPORT__ */

        case SIO_UART_EVENT_MODEM_ABNORMAL: {
            LOGW("medem exception.\r\n");
            gprs_cntx->modem_state = GPRS_MODEM_STATE_EXCEPTION;
            gprs_cntx->is_running = 0;
            nw_manage = gprs_cntx->nw_manage;
            while (nw_manage) {
                gprs_nw_manage_deinit();
            }

            sio_set_mode(gprs_cntx->app_id, SIO_MODE_TYPE_UART | SIO_DATA_TYPE_COMMAND);

            gprs_notify_app(GPRS_NOTI_TYPE_MODEM_EXCEPTION);
            
            gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(GPRS_AT_CMD_CLASS_DEACTIVATE));
            gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(GPRS_AT_CMD_CLASS_ACTIVATE));
            gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(GPRS_AT_CMD_CLASS_REGISTRATION));
            gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(GPRS_AT_CMD_CLASS_INIT));
            return;
        }
        case SIO_UART_EVENT_MODEM_DATA_TO_READ: {
            gprs_cntx->noti_data.length = sio_receive_data(gprs_cntx->app_id,
                                          gprs_cntx->noti_data.payload,
                                          gprs_cntx->noti_data.length);
            if (gprs_cntx->noti_data.length == 0) {
                return;
            }
            gprs_send_callback(gprs_cntx->noti_data.payload, gprs_cntx->noti_data.length, NULL);
            return;
        }

        case GPRS_EVENT_URC: {
            if ((GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) || (nw_manage == NULL)) {
                LOGW("nw manage not find %d\r\n", gprs_cntx->current_cid);
            }
#ifdef WAIT_CGREG_URC
            switch (gprs_parse_modem_urc(gprs_cntx->noti_data.payload, gprs_cntx->noti_data.length, (uint8_t *)urc_data)) {
                case GPRS_URC_TYPE_CGEV: {
                    if (0 == strncmp(urc_data, gprs_modem_urc[0], strlen(gprs_modem_urc[0]))) {
                        if (gprs_cntx && gprs_cntx->nw_manage && nw_manage) {
                            if (nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_INFO_SET) {
                                gprs_remove_nw_info();
                            }

                            nw_manage->nw_manage_state = GPRS_NW_MANAGE_STATE_NONE;
                        }
                        gprs_notify_app(GPRS_NOTI_TYPE_GPRS_PASSIVE_DEACTIVE);
                    }
                    break;
                }

                case GPRS_URC_TYPE_CGREG: {
                    sta_int = gprs_extract_cgreg_sta((uint8_t *)urc_data, sizeof(gprs_cntx->noti_data.payload));

                    switch (sta_int) {
                        case CRGRG_STATUS_VALUE_0:		/* not registered, searching */
                        case CRGRG_STATUS_VALUE_2: {	/* not registered, trying */
                            return;
                        }

                        case CRGRG_STATUS_VALUE_1:		/* registered, home network */
                        case CRGRG_STATUS_VALUE_5: {	/* registered, roaming */
                            LOGI("**********1*************\r\n");
                            if (gprs_cntx->creg_urc_success_received) {
                                /* Block  CGREG success URC after the first one. */
                                LOGI("Receive CGREG success URC again.\r\n");
                                return;
                            }

                            LOGI("**********2*************\r\n");

                            gprs_cntx->creg_urc_success_received = 1;

                            LOGI("**********3*************\r\n");

                            if (gprs_cntx->need_send_cmd_after_cgreg) {
                                LOGE("Only enter here once on receiving CGREG success URC. flag should be false.\r\n");
                                break;
                            }

                            LOGI("**********4*************\r\n");

                            gprs_cntx->need_send_cmd_after_cgreg = 1;
                            gprs_cntx->cgreg_succeed = 1;
                        
                        #ifdef __CMUX_SUPPORT__
                            if (gprs_cntx->cgreg_succeed == 1 && gprs_cntx->cmux_connected == 1);
                            {
                                gprs_cntx->cgred_cmux_succeed = 1;
                            }
                        #endif

                            cmd_type = gprs_get_next_at_cmd_by_class(GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_CGREG);
                            if (GPRS_RET_OK == gprs_send_at_cmd(cmd_type)) {
                                LOGI("Continue to send at cmd when receiving CGREG URC 1.\r\n");
                                gprs_cntx->need_send_cmd_after_cgreg = 0;
                            }

                            LOGI("**********5*************\r\n");

                            return;
                        }

                        case CRGRG_STATUS_VALUE_3:		/* registration denied */
                        case CRGRG_STATUS_VALUE_6:		/* "SMS only", home network */
                        case CRGRG_STATUS_VALUE_7:		/* "SMS only", roaming */
                        case CRGRG_STATUS_VALUE_8:		/* emergency bearer services only  */
                        case CRGRG_STATUS_VALUE_9:		/* registered for "CSFB not preferred" */
                        case CRGRG_STATUS_VALUE_10: {	/* registered for "CSFB not preferred" */                        
                            LOGI("**********6*************\r\n");
                            LOGI("%d", sta_int);
                            break;
                        }

                        case CRGRG_STATUS_VALUE_4:		/* Unknown */
                            return;
                            
                        default: {
                            LOGI("**********7*************\r\n");
                            LOGI("%d", sta_int);
                            return;
                        }

                    }
                    gprs_action_callback(GPRS_RET_ERROR, GPRS_AT_CMD_CLASS_REGISTRATION); 
                    
                    break;
                }

                default:
                    break;
            }
#else
            if (GPRS_MODEM_RESPONSE_CGEV_NWDEACT == gprs_parse_response_data(gprs_cntx->noti_data.payload, gprs_cntx->noti_data.length)) {
                /* Receive NW Deactivation URC: +CGEV: NW DEACT */
                if (gprs_cntx && gprs_cntx->nw_manage && nw_manage) {
                    if (nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_INFO_SET) {
                        gprs_remove_nw_info_with_cid(nw_manage->cid);
                    }

                    nw_manage->nw_manage_state = GPRS_NW_MANAGE_STATE_NONE;
                }
                gprs_notify_app(GPRS_NOTI_TYPE_GPRS_PASSIVE_DEACTIVE);
            }
#endif

            return;
        }

        default:
            break;
    }
}


/**
  * @brief     Data send callback.
  * @param[in] uint8_t *payload: data
  * @param[in] uint32_t length: data length
  * @param[in] void *user_data: user data
  * @retval    Negative is fail. Zero is success.
  */
int32_t gprs_send_callback(uint8_t *payload, uint32_t length, void *user_data)
{
    LOGI("length = %d\r\n", length);

    if (NULL == payload || 0 == length || NULL == gprs_cntx) {
        return -1;
    }

    memset(&gprs_cntx->cb_data, 0, sizeof(gprs_cntx->cb_data));

    gprs_cntx->cb_data.length = GPRS_MIN(length, GPRS_MSM_DATA_MAX_LEN);
    memcpy(gprs_cntx->cb_data.payload,
              payload,
              gprs_cntx->cb_data.length);

    gprs_cntx->cb_data.userdata = user_data;

    nw_demo_send_event(MESSAGE_ID_GPRS_ATCMD_SENT_CB, 0, NULL);

    return 0;
}

/**
  * @brief     Extract IP address.
  * @param[in] uint8_t *payload: data
  * @param[in/out] char *ip_buf: IP address buffer
  * @param[in/out] int32_t ip_buf_len: IP address buffer length
  * @retval    Negative is fail. Zero is success.
  */
gprs_ret_t gprs_extract_ip_addr(uint8_t *payload, char *ip_buf, int32_t ip_buf_len)
{
    uint8_t *sub_str1 = NULL, *sub_str2 = NULL;
    int32_t addr_len = 0;
    gprs_ret_t ret = GPRS_RET_ERROR;

    LOGI("\r\ngprs_extract_ip_addr() IP: %s\r\n", payload);

    if (NULL == payload || NULL == ip_buf ||
            !(GPRS_MIN_IP_ADDR_LEN <= (ip_buf_len - 1) && GPRS_MAX_IP_ADDR_LEN >= (ip_buf_len - 1))) {
        LOGE("gprs_extract_ip_addr() invalid parameters.\r\n");
        return ret;
    }

    /* Example Response: +CGPADDR: 1, "10.186.49.243"
      *                             +CGPRCO: 1, "183.221.253.100", "223.87.253.100", "", ""
      *					+CGPRCO: 2, "172.21.120.6", "0.0.0.0", "", ""
      */

    /* sub_str1 points to the first byte of the IP, while sub_str2 points to the last byte of the IP. */
    sub_str1 = (uint8_t *)strchr((char *)payload, '\"');
    if (sub_str1)
    {
        ++sub_str1;
        LOGI("payload:%s\r\n*sub_str1:%c, *sub_str1:%d, sub_str1:%s\r\n", payload, *sub_str1, sub_str1);
    
        sub_str2 = (uint8_t *)strchr((char *)sub_str1, '\"');
        if (sub_str2 && (sub_str2 > sub_str1))
        {
            --sub_str2;
        }
        if (sub_str1 && sub_str2) {
            addr_len = sub_str2 - sub_str1 + 1;
        }

        LOGI("sub_str1:%x sub_str2:%x, addr_len:%d\r\n", sub_str1, sub_str2, addr_len);

        if (GPRS_MIN_IP_ADDR_LEN <= addr_len && GPRS_MAX_IP_ADDR_LEN >= addr_len && (ip_buf_len - 1) >= addr_len) {
            strncpy(ip_buf, (char *)sub_str1, addr_len);
            ip_buf[addr_len] = '\0';

            LOGI("Extracted IP Address:%s\r\n", ip_buf);
            ret = GPRS_RET_OK;
        }
    } else {
        LOGI("sub_str1 is NULL");
    }

    return ret;
}

/**
  * @brief     Get AT CMD class by AT CMD type.
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    gprs_at_cmd_class_enum.
  */
gprs_action_type_enum gprs_get_action_by_class(gprs_at_cmd_class_enum cmd_class)
{
    switch(cmd_class) {
        case GPRS_AT_CMD_CLASS_INIT:
            return GPRS_ACTION_TYPE_INITIALIZE;
            
        case GPRS_AT_CMD_CLASS_REGISTRATION:
            return GPRS_ACTION_TYPE_REGISTRATION;
            
        case GPRS_AT_CMD_CLASS_ACTIVATE:
            return GPRS_ACTION_TYPE_ACTIVATE;
            
        case GPRS_AT_CMD_CLASS_DEACTIVATE:
            return GPRS_ACTION_TYPE_DEACTIVATE;
            
        default:
            return GPRS_ACTION_TYPE_NONE;
    }
}


/**
  * @brief     Get AT CMD class by AT CMD type.
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    gprs_at_cmd_class_enum.
  */
gprs_at_cmd_class_enum gprs_get_at_cmd_class_by_type(gprs_at_cmd_type_enum cmd_type)
{
    int i = 0;

    while (gprs_at_cmds[i].at_cmd) {
        if (cmd_type == gprs_at_cmds[i].cmd_type) {
            return gprs_at_cmds[i].cmd_class;
        }
        i++;
    }

    return GPRS_AT_CMD_CLASS_NONE;
}


/**
  * @brief     AT CMD sent callback handler.
  * @retval    none.
  */
void gprs_atcmd_sent_cb_hdl(void)
{
    gprs_ret_t ret = GPRS_RET_ERROR;
    gprs_net_info_struct net_info = {0};
    char *sub_str1 = NULL;
    gprs_at_cmd_class_enum cmd_class = GPRS_AT_CMD_CLASS_NONE;
    gprs_at_cmd_type_enum cmd_type = GPRS_AT_CMD_TYPE_NONE, pre_cmd_type = GPRS_AT_CMD_TYPE_NONE;
    gprs_nw_manage_struct* nw_manage = NULL;

    LOGI("enter \r\n");

    if (NULL == gprs_cntx) {
        configASSERT(0);
        return;
    }

    if ((GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) || (nw_manage == NULL)) {
        LOGW("nw manage not find %d\r\n", gprs_cntx->current_cid);
        configASSERT(0);
        return;
    }

    LOGI("gprs_cntx->current_at_cmd_type = %d\r\n", gprs_cntx->current_at_cmd_type);
    cmd_class = gprs_get_at_cmd_class_by_type(gprs_cntx->current_at_cmd_type);
    pre_cmd_type = gprs_cntx->current_at_cmd_type;
    gprs_cntx->current_at_cmd_type = GPRS_AT_CMD_TYPE_NONE;
    if (gprs_parse_response_data(gprs_cntx->cb_data.payload, gprs_cntx->cb_data.length) > GPRS_MODEM_RESPONSE_ERROR) {
        LOGI("1, cmd_class = %d, pre_cmd_type = %d\r\n", cmd_class, pre_cmd_type);

    #ifdef __CMUX_SUPPORT__
        if (GPRS_AT_CMD_CLASS_INIT == cmd_class && GPRS_AT_CMD_TYPE_CMUX == pre_cmd_type) {
            sio_set_mode(gprs_cntx->app_id, SIO_MODE_TYPE_CMUX);
            gprs_cntx->need_wait_for_cmux_connect = 1;
        }
    #endif

        if (GPRS_AT_CMD_CLASS_REGISTRATION == cmd_class && GPRS_AT_CMD_TYPE_CGREG == pre_cmd_type) {
#ifndef WAIT_CGREG_URC
            vTaskDelay(3000);
            LOGI("Delay 3 seconds after cgreg\r\n");
#else
            if (!gprs_cntx->need_send_cmd_after_cgreg) {
                return;
            }
#endif
        } else if (GPRS_AT_CMD_CLASS_ACTIVATE == cmd_class) {
            LOGI("2, cmd_class = %d, pre_cmd_type = %d\r\n", cmd_class, pre_cmd_type);
            if (GPRS_AT_CMD_TYPE_CGPADDR == pre_cmd_type) {
                /* Extract local IP Addr. */
                ret = gprs_extract_ip_addr(gprs_cntx->cb_data.payload,
                                           nw_manage->nw_info.ipv4_addr,
                                           GPRS_MAX_IP_ADDR_LEN + 1);
                if (GPRS_RET_OK != ret) {
                    goto next;
                }
            } else if (GPRS_AT_CMD_TYPE_CGPRCO == pre_cmd_type) {
                /* Example Response:  +CGPRCO: 1, "183.221.253.100", "223.87.253.100", "", ""
                  *					+CGPRCO: 2, "172.21.120.6", "0.0.0.0", "", ""
                  */

                /* Extract Primary DNS Server */
                if (GPRS_RET_OK != gprs_extract_ip_addr(gprs_cntx->cb_data.payload,
                                                        nw_manage->nw_info.dns_svr1,
                                                        GPRS_MAX_IP_ADDR_LEN + 1)) {
                    LOGW("Extract primary DNS Server failed. Use public DNS Server.\r\n");
                    strcpy(nw_manage->nw_info.dns_svr1, "8.8.8.8");
                    nw_manage->nw_info.dns_svr1[7] = '\0';
                }

                sub_str1 = strchr((char *)gprs_cntx->cb_data.payload, '\"') + 1;
                sub_str1 = (sub_str1 ? strchr(sub_str1, '\"') + 1 : NULL);
                sub_str1 = (sub_str1 ? strchr(sub_str1, '\"') : NULL);
                LOGI("sub_str1:%s\r\n", sub_str1);
                if (NULL == sub_str1 || GPRS_RET_OK != gprs_extract_ip_addr((uint8_t *)sub_str1,
                        nw_manage->nw_info.dns_svr2,
                        GPRS_MAX_IP_ADDR_LEN + 1)) {
                    LOGW("Extract secondary DNS Server failed. Use public DNS Server.\r\n");
                    strcpy(nw_manage->nw_info.dns_svr2, "8.8.4.4");
                    nw_manage->nw_info.dns_svr2[7] = '\0';
                }
            }

            /*if (GPRS_AT_CMD_TYPE_CGPRCO == cmd_type && GPRS_RET_OK != ret) {
                strcpy(nw_manage->nw_info.dns_svr1, "8.8.8.8");
                nw_manage->nw_info.dns_svr1[7] = '\0';
                strcpy(nw_manage->nw_info.dns_svr2, "8.8.4.4");
                nw_manage->nw_info.dns_svr2[7] = '\0';

                ret = GPRS_RET_OK;

                LOGW("Failed to send CGPRCO. Use public DNS servers.\r\n");
            }

            if (GPRS_AT_CMD_TYPE_CGPRCO == pre_cmd_type && parse_ret > GPRS_MODEM_RESPONSE_ERROR) {
                strcpy(nw_manage->nw_info.dns_svr1, "8.8.8.8");
                nw_manage->nw_info.dns_svr1[7] = '\0';
                strcpy(nw_manage->nw_info.dns_svr2, "8.8.4.4");
                nw_manage->nw_info.dns_svr2[7] = '\0';

                ret = GPRS_RET_OK;

                LOGW("parse_ret of CGPRCO rsp is not OK. Use public DNS servers.\r\n");
            } ;*/
        }

        if (GPRS_AT_CMD_TYPE_NONE != (cmd_type = gprs_get_next_at_cmd_by_class(cmd_class, pre_cmd_type))) {
            if (GPRS_RET_OK == (ret = gprs_send_at_cmd(cmd_type))) {
                if (!gprs_cntx->need_send_cmd_after_cgreg) {
                    gprs_cntx->need_send_cmd_after_cgreg = 0;
                }
                ret = GPRS_RET_WOULDBLOCK;
                return;
            }
        } else {
            LOGI("Finished to send at cmd by class. cmd_class:%d\r\n", cmd_class);
            ret = GPRS_RET_OK;
        }
    } else {
        LOGE("parse result: ERROR %d ", pre_cmd_type);
        gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(cmd_class));
        return;
    }
    

next:
    /* 1. parse_ret is not OK
      * 2. No more at cmd needs to be sent,
      * 3. Failed to send next at cmd.
      */
    switch (cmd_class) {
        case GPRS_AT_CMD_CLASS_INIT: {
            if (GPRS_RET_OK == ret) {
                LOGI("Modem initialized success");
                nw_manage->nw_manage_state |= GPRS_NW_MANAGE_STATE_INITIALIZED;
                gprs_cntx->modem_state = GPRS_MODEM_STATE_READY;
                //gprs_notify_app(GPRS_NOTI_TYPE_MODEM_READY);
                gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_INITIALIZE);
            } else {
                LOGI("Modem initialized fail %d", pre_cmd_type);
                gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_INITIALIZE);
            }
            break;
        }

        case GPRS_AT_CMD_CLASS_REGISTRATION: {
            if (GPRS_RET_OK == ret) {
                LOGI("GPRS registration success");
                nw_manage->nw_manage_state |= GPRS_NW_MANAGE_STATE_REGISTERED;
                gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_REGISTRATION);
            } else {
                LOGI("GPRS registration fail %d", pre_cmd_type);
                gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_REGISTRATION);
            }
            break;
        }

        case GPRS_AT_CMD_CLASS_ACTIVATE: {
            if (GPRS_RET_OK == ret) {
                LOGI("Data activate success");
                net_info.ipaddr = (char*)&nw_manage->nw_info.ipv4_addr;
                net_info.dnsservser1 = (char*)&nw_manage->nw_info.dns_svr1;
                net_info.dnsservser2 = (char*)&nw_manage->nw_info.dns_svr2;
                if (1 == ps_netif_set_netinfo(nw_manage->cid, &net_info)) {
                    nw_manage->nw_manage_state |= GPRS_NW_MANAGE_STATE_INFO_SET;
                } else {
                    LOGI("ps_netif_set_netinfo FAIL");
                }

                LOGI("Primary DNS Server:%s\r\n", nw_manage->nw_info.dns_svr1);
                LOGI("Secondary DNS Server:%s\r\n", nw_manage->nw_info.dns_svr2);
                
                nw_manage->nw_manage_state |= GPRS_NW_MANAGE_STATE_ACTIVATED;
                gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_ACTIVATE);
            } else {
                LOGI("Data activate fail %d", pre_cmd_type);
                gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_ACTIVATE);
            }
            break;
        }

        case GPRS_AT_CMD_CLASS_DEACTIVATE: {
            if (GPRS_RET_OK == ret) {
                LOGI("Data deactivate success");
                nw_manage->nw_manage_state &= ~GPRS_NW_MANAGE_STATE_ACTIVATED;
                if (nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_INFO_SET) {
                    gprs_remove_nw_info();
                }
                gprs_nw_manage_deinit();
                gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_DEACTIVATE);
            } else {
                LOGI("Data deactivate fail %d", pre_cmd_type);
                gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_DEACTIVATE);
            }
            break;
        }
        
        default: {
            LOGI("default:%d", cmd_class);
            break;
        }
    }

    return;
}


/**
  * @brief     Find AT CMD class by AT CMD type.
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    gprs_at_cmd_class_enum.
  */
static gprs_at_cmd_class_enum gprs_find_cmd_class_by_type(gprs_at_cmd_type_enum at_cmd_type)
{
    int i = 0;

    while (gprs_at_cmds[i].at_cmd) {
        if (at_cmd_type == gprs_at_cmds[i].cmd_type) {
            return gprs_at_cmds[i].cmd_class;
        }

        i++;
    }

    return GPRS_AT_CMD_CLASS_NONE;
}


/**
  * @brief     Find AT CMD by AT CMD type.
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    AT CMD.
  */
static char *gprs_find_at_cmd_by_type(gprs_at_cmd_type_enum at_cmd_type)
{
    int i = 0;

    while (gprs_at_cmds[i].at_cmd) {
        if (at_cmd_type == gprs_at_cmds[i].cmd_type) {
            return gprs_at_cmds[i].at_cmd;
        }

        i++;
    }

    return NULL;
}


/**
  * @brief     Send AT CMD.
  * @param[in] gprs_at_cmd_type_enum cmd_type: AT CMD type
  * @retval    Negative is fail. Zero is success.
  */
static gprs_ret_t gprs_send_at_cmd(gprs_at_cmd_type_enum at_cmd_type)
{
    uint8_t *data;
    uint32_t len = 0;

    if (GPRS_AT_CMD_TYPE_NONE != gprs_cntx->current_at_cmd_type) {
        LOGW("There's still an AT CMD(%d) waiting for RSP.\r\n", gprs_cntx->current_at_cmd_type);
        return GPRS_RET_ERROR;
    }
    LOGI("gprs_send_at_cmd(), at_cmd_type = %d\r\n", at_cmd_type);

    data = pvPortCalloc(1, GPRS_SEND_DATA_MAX_LENGTH);
    memset(data, 0, GPRS_SEND_DATA_MAX_LENGTH);
    switch (at_cmd_type) {
        case GPRS_AT_CMD_TYPE_CDGCONT_UN:
        case GPRS_AT_CMD_TYPE_CDGCONT:
        case GPRS_AT_CMD_TYPE_CGACT:
        case GPRS_AT_CMD_TYPE_CGDEACT:
        case GPRS_AT_CMD_TYPE_CGDATA:
        case GPRS_AT_CMD_TYPE_CGPADDR: {
            gprs_nw_manage_struct* nw_manage = NULL;
            if ((GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage))
                 && (NULL == nw_manage)) {
                vPortFree(data);
                return GPRS_RET_ERROR;
            }
            sprintf((char*)data, gprs_find_at_cmd_by_type(at_cmd_type), gprs_cntx->current_cid);

            break;
        }
        case GPRS_AT_CMD_TYPE_ENABLE_DTR:
        case GPRS_AT_CMD_TYPE_ENABLE_RI:
        case GPRS_AT_CMD_TYPE_DISABLE_PWM:
        case GPRS_AT_CMD_TYPE_SET_GPIO_OUTPUT:
        case GPRS_AT_CMD_TYPE_SET_GPIO_HIGH:
        case GPRS_AT_CMD_TYPE_CMEE:
        case GPRS_AT_CMD_TYPE_CMUX:
        case GPRS_AT_CMD_TYPE_ESLP_ENABLE_INIT:
        case GPRS_AT_CMD_TYPE_ESLP_ENABLE_REGISTRATION:
        case GPRS_AT_CMD_TYPE_ESLP_ENABLE_ACTIVATE:
        case GPRS_AT_CMD_TYPE_ESLP_ENABLE_DEACTIVATE:
        case GPRS_AT_CMD_TYPE_ESLP_DISABLE_INIT:
        case GPRS_AT_CMD_TYPE_ESLP_DISABLE_REGISTRATION:
        case GPRS_AT_CMD_TYPE_ESLP_DISABLE_ACTIVATE:
        case GPRS_AT_CMD_TYPE_ESLP_DISABLE_DEACTIVATE:
        case GPRS_AT_CMD_TYPE_EIND:
        case GPRS_AT_CMD_TYPE_SET_CFUN_ON:
        case GPRS_AT_CMD_TYPE_ERAT:
        case GPRS_AT_CMD_TYPE_CGREG:
        case GPRS_AT_CMD_TYPE_CGPRCO:
        case GPRS_AT_CMD_TYPE_CGEREP:
        case GPRS_AT_CMD_TYPE_SET_CTZR_ON:
#ifdef ATE0_NEED
        case GPRS_AT_CMD_TYPE_ATE0:
#endif
        {
            sprintf((char *)data, gprs_find_at_cmd_by_type(at_cmd_type));
            break;
        }

        default: {
            LOGE("Didn't find AT CMD 1.\r\n");
            vPortFree(data);
            gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(gprs_get_at_cmd_class_by_type(at_cmd_type))); 
            return GPRS_RET_ERROR;
        }
    }

    len = strlen((char *)data);
    if (len == 0) {
        LOGE("Didn't find AT CMD 2.\r\n");
        vPortFree(data);
        gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(gprs_get_at_cmd_class_by_type(at_cmd_type))); 
        return GPRS_RET_ERROR;
    }

    LOGI("\r\nat_cmd_type: %d\r\n", gprs_cntx->current_at_cmd_type);
    LOGI("\r\nAT CMD SENT: %s\r\n", data);
    if (sio_send_data(gprs_cntx->app_id, data, len) == len) {
        gprs_cntx->current_at_cmd_type = at_cmd_type;
        vPortFree(data);
        return GPRS_RET_OK;
    } else {
        gprs_action_callback(GPRS_RET_ERROR, gprs_get_action_by_class(gprs_get_at_cmd_class_by_type(at_cmd_type))); 
        vPortFree(data);
        return GPRS_RET_ERROR;
    }

}

static gprs_ret_t gprs_find_nw_manage_with_cid(gprs_pdp_context_id_enum cid, gprs_nw_manage_struct** nw_manage)
{
    gprs_nw_manage_struct* last_nw_manage = NULL;
    uint8_t count = 0;

    LOGI("%d", cid);

    if (NULL == gprs_cntx) {
        return GPRS_RET_ERROR;
    }

    if (cid == 0 || cid >= GPRS_PDP_CONTEXT_ID_MAX) {
        return GPRS_RET_INVALID_PARAM;
    }

    (*nw_manage) = gprs_cntx->nw_manage;
    while (*nw_manage)
    {
        count++;
        if (count > GPRS_SUPPORT_APN_NUM) {
            (*nw_manage) = NULL;
            printf("not nw_manage->cid = cid");
            return GPRS_RET_ERROR;
        }

        if ((*nw_manage)->cid == cid) {
            LOGI("gprs_find_nw_manage_with_cid nw_manage->cid = %d. \r\n", (*nw_manage)->cid);
            return GPRS_RET_OK;
        }

        (*nw_manage) = (*nw_manage)->next;
        last_nw_manage = (*nw_manage);
    }

    if (count > GPRS_SUPPORT_APN_NUM)
        return GPRS_RET_ERROR;

    (*nw_manage) = (gprs_nw_manage_struct *)pvPortCalloc(1, sizeof(gprs_nw_manage_struct));
    if (NULL == (*nw_manage)) {
        return GPRS_RET_NOT_ENOUGH_MEMORY;
    }

    memset((*nw_manage), 0, sizeof(gprs_nw_manage_struct));
    (*nw_manage)->cid = cid;
    (*nw_manage)->next = NULL;

    if (last_nw_manage) {
        last_nw_manage->next = (*nw_manage);
    } else {
        gprs_cntx->nw_manage = (*nw_manage);
    }

    return GPRS_RET_OK;
}



#if 0
/**
  * @brief     Initation NW manage.
  *             nw manage is created at each activation, and freeed at each deactivation.
  * @retval    Negative is fail. Zero is success.
  */
static gprs_ret_t gprs_nw_manage_init(void)
{
    if (NULL == gprs_cntx) {
        return GPRS_RET_ERROR;
    }

    if (gprs_cntx->nw_manage) {
        return GPRS_RET_OK;
    }

    gprs_cntx->nw_manage = (gprs_nw_manage_struct *)pvPortCalloc(1, sizeof(gprs_nw_manage_struct));
    if (NULL == gprs_cntx->nw_manage) {
        return GPRS_RET_NOT_ENOUGH_MEMORY;
    }

    gprs_cntx->nw_manage->cid = 1;

    return GPRS_RET_OK;
}
#endif

/**
  * @brief     URC callback.
  * @param[in] uint8_t *payload: data
  * @param[in] uint32_t length: data length
  * @retval    Negative is fail. Zero is success.
  */
urc_cb_ret gprs_urc_callback(uint8_t *payload, uint32_t length)
{
    LOGI("gprs_urc_callback. \r\n");

    gprs_cntx->noti_data.length = GPRS_MIN(length, GPRS_MSM_DATA_MAX_LEN);
    memcpy(gprs_cntx->noti_data.payload, payload, gprs_cntx->noti_data.length);

    nw_demo_send_event(MESSAGE_ID_GPRS_NOTI_HDL, GPRS_EVENT_URC, NULL);

    return RET_OK_CONTINUE;
}

static void gprs_notify_app(gprs_noti_type_enum noti_type)
{
    LOGI("noti_type:%d\r\n", noti_type);

    switch (noti_type) {
        case GPRS_NOTI_TYPE_MODEM_READY: {
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_MODEM_READY_IND, NULL);
            break;
        }
        case GPRS_NOTI_TYPE_CMUX_CONN: {
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_CMUX_CONN_IND, NULL);
            break;
        }
        case GPRS_NOTI_TYPE_CMUX_DISCONN: {
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_CMUX_DISCONN_IND, NULL);
            break;
        }
        case GPRS_NOTI_TYPE_MODEM_EXCEPTION: {
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_MODEM_EXCEPTION_IND, NULL);
            break;
        }

        case GPRS_NOTI_TYPE_GPRS_PASSIVE_DEACTIVE: {
            nw_demo_send_event(MESSAGE_ID_NW_FLOW, NW_FLOW_EVT_TYPE_GPRS_DEACTIVATE_IND, NULL);
            break;
        }

        default: {
            break;
        }
    }
}


/**
  * @brief      Cntx initial.
  * @retval    None.
  */
void gprs_cntx_init(void)  
{
    LOGI("~~~~~~~~~~~~~GPRS CNTX INIT~~~~~~~~~~~~~");
    gprs_cntx = (gprs_context_struct *)pvPortCalloc(1, sizeof(gprs_context_struct));

    urc_register_callback(gprs_urc_callback);

    if (gprs_cntx) {
        gprs_cntx->app_id = sio_register_event_notifier((sio_app_type_t)(SIO_APP_TYPE_CMUX_AT_CMD | SIO_APP_TYPE_UART_AT_CMD), gprs_event_hdl);
    }

    if (gprs_cntx->app_id < 0) {
        if (gprs_cntx) {
            vPortFree(gprs_cntx);
            gprs_cntx = NULL;
        }

        LOGW("%s bootup fail\r\n", __FUNCTION__);
        return;
    }

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_cntx->sleep_manager_handler = hal_sleep_manager_set_sleep_handle("gprs_api");
    LOGI("gprs_cntx->sleep_manager_handler = %d.\r\n", gprs_cntx->sleep_manager_handler);

    gprs_set_host_wakeup();
    sio_trigger_modem_wakeup();
#endif

    gprs_cntx->modem_state = GPRS_MODEM_STATE_NOT_READY;
    gprs_cntx->cgreg_succeed = 0;
}


void gprs_set_callback(gprs_action_cb callback)
{
    gprs_cntx->callback = callback;
}


/**
  * @brief     Check modem is ready.
  * @retval    Negative is fail. Zero is success.
  */
gprs_ret_t gprs_is_modem_ready(void)
{
    if (NULL == gprs_cntx) {
        return GPRS_RET_ERROR;
    }

    return GPRS_MODEM_STATE_READY == gprs_cntx->modem_state ? GPRS_RET_OK : GPRS_RET_MODEM_NOT_READY;
}


/**
  * @brief     Check GPRS is activated.
  * @retval    Negative is fail. Zero is success.
  */
gprs_ret_t gprs_is_data_activated(void)
{
    gprs_nw_manage_struct* nw_manage = NULL;
    if (NULL == gprs_cntx ||
        NULL == gprs_cntx->nw_manage ||
        gprs_cntx->current_cid == 0 ||
        GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) {
        return GPRS_RET_ERROR;
    }

    if (GPRS_NW_MANAGE_STATE_ACTIVATED & nw_manage->nw_manage_state) {
        return GPRS_RET_OK;
    }

    return GPRS_RET_ERROR;
}

gprs_ret_t gprs_is_modem_initialized(void)
{
    gprs_nw_manage_struct* nw_manage = NULL;
    if (NULL == gprs_cntx ||
        NULL == gprs_cntx->nw_manage ||
        gprs_cntx->current_cid == 0 ||
        GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) {
        return GPRS_RET_ERROR;
    }

    if (GPRS_NW_MANAGE_STATE_INITIALIZED & nw_manage->nw_manage_state) {
        return GPRS_RET_OK;
    }

    return GPRS_RET_ERROR;
}

gprs_ret_t gprs_is_network_registered(void)
{
    gprs_nw_manage_struct* nw_manage = NULL;
    if (NULL == gprs_cntx ||
        NULL == gprs_cntx->nw_manage ||
        gprs_cntx->current_cid == 0 ||
        GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) {
        return GPRS_RET_ERROR;
    }

    if (GPRS_NW_MANAGE_STATE_REGISTERED & nw_manage->nw_manage_state) {
        return GPRS_RET_OK;
    }

    return GPRS_RET_ERROR;
}

 
 /**
  * @brief     Check GPRS is activated.
  *            When is the action not allowed?
  * 1. bootup fail
  * 2. modem is not ready
  * 3. gprs nw manage is not initialized
  * 4. The flow of other API(activate, deactivate, set nw info) hasn't been completed yet.
  *     (Only  when the response of last AT CMD is returned, can the new AT CMD be sent.)
  * @retval    Negative is fail. Zero is success.
  */
gprs_ret_t gprs_is_action_allowed(void)
{
    gprs_nw_manage_struct* nw_manage = NULL;

    if (GPRS_RET_OK != gprs_is_modem_ready()) {
        LOGE("modem is not ready.\r\n");
        return GPRS_RET_MODEM_NOT_READY;
    }

    if (NULL == gprs_cntx ||
        NULL == gprs_cntx->nw_manage ||
        gprs_cntx->current_cid == 0 ||
        GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) {
        LOGE("can not find nw manager.\r\n");
        return GPRS_RET_ERROR;
    }

    if (nw_manage == NULL) {
        LOGE("nw_manage null.\r\n");
        return GPRS_RET_ERROR;
    }
    
    return GPRS_RET_OK;
}
 
gprs_ret_t gprs_initialize_modem()
{
    gprs_ret_t ret;
    gprs_nw_manage_struct* nw_manage = NULL;

    LOGI("enter");

    if (gprs_cntx->is_running) {
        LOGE("Busy!\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_INITIALIZE);
        return GPRS_RET_BUZY;
    }

    gprs_cntx->current_cid = GPRS_PDP_CONTEXT_ID_1;

    if (GPRS_RET_OK != (ret = gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage))) {
        LOGE("find nw manage fail.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_INITIALIZE);
        return ret;
    }

    if (nw_manage == NULL) {
        LOGE("nw manage is null.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_INITIALIZE);
        return GPRS_RET_ERROR;
    }

    if (GPRS_RET_OK != gprs_is_action_allowed()) {
        LOGE("initialized is not allowed.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_INITIALIZE);
        return GPRS_RET_ERROR;
    }

    if (nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_INITIALIZED) {
        LOGI("modem is initialized.\r\n");
        gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_INITIALIZE);
        return GPRS_RET_OK;
    }

    gprs_cntx->is_running = 1;

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_set_host_wakeup();
    sio_trigger_modem_wakeup();
#endif

    ret = gprs_begin_sent_at_cmd_by_class(GPRS_AT_CMD_CLASS_INIT, GPRS_AT_CMD_TYPE_NONE);

    if (GPRS_RET_OK == ret) {
        return GPRS_RET_WOULDBLOCK;
    } 
    
    gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_INITIALIZE);

    return ret;
}

gprs_ret_t gprs_registration_network()
{
    gprs_ret_t ret;
    gprs_nw_manage_struct* nw_manage = NULL;

    LOGI("enter");

    if (gprs_cntx->is_running) {
        LOGE("Busy!\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_REGISTRATION);
        return GPRS_RET_BUZY;
    }

    gprs_cntx->current_cid = GPRS_PDP_CONTEXT_ID_1;

    if (GPRS_RET_OK != (ret = gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage))) {
        LOGE("find nw manage fail.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_REGISTRATION);
        return ret;
    }

    if (nw_manage == NULL) {
        LOGE("nw manage is null.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_REGISTRATION);
        return GPRS_RET_ERROR;
    }

    if (GPRS_RET_OK != gprs_is_action_allowed()) {
        LOGE("initialized is not allowed.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_REGISTRATION);
        return GPRS_RET_ERROR;
    }

    if (nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_REGISTERED) {
        LOGI("modem is initialized.\r\n");
        gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_REGISTRATION);
        return GPRS_RET_OK;
    }

    gprs_cntx->is_running = 1;

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_set_host_wakeup();
    sio_trigger_modem_wakeup();
#endif

    ret = gprs_begin_sent_at_cmd_by_class(GPRS_AT_CMD_CLASS_REGISTRATION, GPRS_AT_CMD_TYPE_NONE);

    if (GPRS_RET_OK == ret) {
        return GPRS_RET_WOULDBLOCK;
    } 
    
    gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_REGISTRATION);

    return ret;
}

gprs_ret_t gprs_activate_data_with_cid(gprs_pdp_context_id_enum cid)
{
    gprs_ret_t ret;
    gprs_nw_manage_struct* nw_manage = NULL;

    LOGI("%d", cid);

    if (cid >= GPRS_PDP_CONTEXT_ID_MAX) {
        LOGI("cid error!");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_ACTIVATE);
        return GPRS_RET_INVALID_PARAM;
    }

    if (gprs_cntx->is_running) {
        LOGI("busy!");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_ACTIVATE);
        return GPRS_RET_BUZY;
    }

    gprs_cntx->current_cid = cid;

    if (GPRS_RET_OK != (ret = gprs_find_nw_manage_with_cid(cid, &nw_manage))) {
        LOGE("find nw manage fail.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_ACTIVATE);
        return ret;
    }

    if (nw_manage == NULL) {
        LOGE("nw manage is null.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_ACTIVATE);
        return GPRS_RET_ERROR;
    }

    if (GPRS_RET_OK != gprs_is_action_allowed()) {
        LOGE("Activate is not allowed.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_ACTIVATE);
        return GPRS_RET_NOT_ALLOWED;
    }

    if (nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_ACTIVATED) {
        LOGI("GPRS has been activated.\r\n");
        gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_ACTIVATE);
        return GPRS_RET_OK;
    }
#ifdef __CMUX_SUPPORT__
    gprs_cntx->need_send_cmd_after_cgreg = 0; 

    LOGI("need_wait_for_cmux_connect = %d, cgred_cmux_succeed = %d. \r\n", gprs_cntx->need_wait_for_cmux_connect, gprs_cntx->cgred_cmux_succeed);
    if (gprs_cntx->need_wait_for_cmux_connect == 1 && gprs_cntx->cgred_cmux_succeed == 0) {
        LOGI("GPRS activated but cmux is not connect. \r\n");
        return GPRS_RET_WOULDBLOCK;
    }
#endif

    gprs_cntx->is_running = 1;

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_set_host_wakeup();
    sio_trigger_modem_wakeup();
#endif

    ret = gprs_begin_sent_at_cmd_by_class(GPRS_AT_CMD_CLASS_ACTIVATE, GPRS_AT_CMD_TYPE_ESLP_DISABLE_ACTIVATE);

    if (GPRS_RET_OK == ret) {
        return GPRS_RET_WOULDBLOCK;
    }
    
    gprs_action_callback(GPRS_RET_OK, GPRS_ACTION_TYPE_ACTIVATE);
    
    return ret;
}


gprs_ret_t gprs_deactivate_data_with_cid(gprs_pdp_context_id_enum cid)
{
    gprs_ret_t ret = GPRS_RET_ERROR;
    gprs_nw_manage_struct* nw_manage = NULL;

    LOGI("%d", cid);

    if (cid >= GPRS_PDP_CONTEXT_ID_MAX) {
        LOGI("ERROR!");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_DEACTIVATE);
        return GPRS_RET_INVALID_PARAM;
    }

    if (GPRS_RET_OK != gprs_is_action_allowed()) {
        LOGE("Deactivate is not allowed.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_DEACTIVATE);
        return GPRS_RET_NOT_ALLOWED;
    }

    if ((GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) || (nw_manage == NULL)) {
        LOGW("NW information not find.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_DEACTIVATE);
        return GPRS_RET_ERROR;
    }

    if (!(nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_ACTIVATED)) {
        LOGI("GPRS hasn't been activated.\r\n");
        gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_DEACTIVATE);
        return GPRS_RET_OK;
    }
    
    gprs_cntx->is_running = 1;

#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_set_host_wakeup();
    sio_trigger_modem_wakeup();
#endif

    ret = gprs_begin_sent_at_cmd_by_class(GPRS_AT_CMD_CLASS_DEACTIVATE, GPRS_AT_CMD_TYPE_NONE);

    if (GPRS_RET_OK == ret) {
        return GPRS_RET_WOULDBLOCK;
    }
    
    gprs_action_callback(GPRS_RET_ERROR, GPRS_ACTION_TYPE_DEACTIVATE);

    return ret;
}


#ifndef WAIT_CGREG_URC
static gprs_ret_t gprs_remove_nw_info_with_cid(gprs_pdp_context_id_enum cid)
{
    char ret;
    gprs_nw_manage_struct* nw_manage = NULL;

    LOGI("%d", cid);

    if (cid >= GPRS_PDP_CONTEXT_ID_MAX) {
        return GPRS_RET_INVALID_PARAM;
    }

    if (NULL == gprs_cntx || NULL == gprs_cntx->nw_manage) {
        return GPRS_RET_ERROR;
    }

    if ((GPRS_RET_OK != gprs_find_nw_manage_with_cid(cid, &nw_manage)) || (nw_manage == NULL)) {
        LOGW("NW information not find.\r\n");
    }

    if (!(nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_INFO_SET)) {
        LOGW("NW information hasn't been set\r\n");
        return GPRS_RET_OK;
    }

    ret = ps_netif_remove_netinfo(nw_manage->cid);

    if (GPRS_RET_OK == ret) {
        nw_manage->nw_manage_state &= ~GPRS_NW_MANAGE_STATE_INFO_SET;
    }

    return ret == 0 ? GPRS_RET_OK : GPRS_RET_ERROR;
}
#endif
/*
static gprs_ret_t gprs_remove_all_nw_info(uint8_t cid)
{
    return GPRS_RET_OK;
}*/
    

/**
  * @brief     Remove NW information.
  * @retval    Negative is fail. Zero is success.
  */
static gprs_ret_t gprs_remove_nw_info(void)
{
    char ret;

    LOGI("begin");

    if (NULL == gprs_cntx || NULL == gprs_cntx->nw_manage) {
        return GPRS_RET_ERROR;
    }

    if (!(gprs_cntx->nw_manage->nw_manage_state & GPRS_NW_MANAGE_STATE_INFO_SET)) {
        LOGW("NW information hasn't been set\r\n");
        return GPRS_RET_OK;
    }

    ret = ps_netif_remove_netinfo(gprs_cntx->nw_manage->cid);

    if (GPRS_RET_OK == ret) {
        gprs_cntx->nw_manage->nw_manage_state &= ~GPRS_NW_MANAGE_STATE_INFO_SET;
    }

    return ret == 0 ? GPRS_RET_OK : GPRS_RET_ERROR;
}

/**
  * @brief     Get local IP address.
  * @retval    IP address.
  */
char *gprs_get_local_ip(void)
{
    if (NULL == gprs_cntx ||
            NULL == gprs_cntx->nw_manage ||
            '\0' == gprs_cntx->nw_manage->nw_info.ipv4_addr[0]) {
        return NULL;
    }

    return gprs_cntx->nw_manage->nw_info.ipv4_addr;
}


               
/* Callback for activation, deactivation and nw info set. */
static void gprs_action_callback(gprs_ret_t res, gprs_action_type_enum action)
{
    gprs_nw_manage_struct* nw_manage = NULL;
    
    LOGI("res:%d, action_type:%d\r\n", res, (int)action ? (int)action : -1);
    
    if ((GPRS_RET_OK != gprs_find_nw_manage_with_cid(gprs_cntx->current_cid, &nw_manage)) || (nw_manage == NULL)) {
        LOGE("NW information not find.\r\n");
        configASSERT(0);
    }
    
    gprs_cntx->is_running = 0;

    if (gprs_cntx->callback) {
        gprs_cntx->callback(res, action);
    }
    
#ifdef GPRS_SUPPORT_SLEEP_MANAGER
    gprs_set_host_sleep();
    sio_trigger_modem_sleep();
#endif
}


/**
  * @brief     Get app id register form SIO API.
  * @retval    app id
  */
uint32_t gprs_get_app_id()
{
    return gprs_cntx->app_id;
}

uint8_t gprs_get_cid_by_apn(uint8_t* apn)
{
    uint8_t i = 0;
    for(i = 0; i < GPRS_SUPPORT_APN_NUM; i++)
    {
        if (strcmp((const char*)gprs_cid_array[i].apn, (const char*)apn) && gprs_cid_array[i].is_used == 1)
        {
            return gprs_cid_array[i].cid;
        }
    }

    for (i = 0; i < GPRS_SUPPORT_APN_NUM; i++)
    {
        if (gprs_cid_array[i].is_used == 0) {
            memcpy(gprs_cid_array[i].apn, apn, strlen((const char*)apn));
            gprs_cid_array[i].is_used = 1;
            gprs_cid_array[i].cid = i+1;
            return gprs_cid_array[i].cid;
        }
    }

    return 0;
}


gprs_ret_t gprs_free_cid_by_apn(uint8_t* apn)
{
    uint8_t i = 0;
    for(i = 0; i < GPRS_SUPPORT_APN_NUM; i++)
    {
        if (strcmp((const char*)gprs_cid_array[i].apn, (const char*)apn) && gprs_cid_array[i].is_used == 1)
        {
            memset(gprs_cid_array[i].apn, 0, GPRS_SUPPORT_APN_MAX_LENGTH);
            gprs_cid_array[i].is_used = 1;
            gprs_cid_array[i].cid = 0;
            return GPRS_RET_OK;
        }
    }

    return GPRS_RET_ERROR;
}


gprs_ret_t gprs_free_cid_by_cid(uint8_t cid)
{
    uint8_t i = 0;

    if (cid > GPRS_SUPPORT_APN_NUM || cid < 1)
        return GPRS_RET_ERROR;

    for(i = 0; i < GPRS_SUPPORT_APN_NUM; i++)
    {
        if (gprs_cid_array[i].cid == cid && gprs_cid_array[i].is_used == 1)
        {
            memset(gprs_cid_array[i].apn, 0, GPRS_SUPPORT_APN_MAX_LENGTH);
            gprs_cid_array[i].is_used = 1;
            gprs_cid_array[i].cid = 0;
            return GPRS_RET_OK;
        }
    }

    return GPRS_RET_ERROR;
}



#ifdef GPRS_SUPPORT_SLEEP_MANAGER
gprs_ret_t gprs_set_host_wakeup()
{
    uint32_t sleep_lock_status;
    atci_response_t resonse;
    memset(&resonse, 0, sizeof(atci_response_t));

    sleep_lock_status = hal_sleep_manager_get_lock_status();
    LOGI("gprs_set_host_wakeup() begin %d\r\n", sleep_lock_status);
    if (!(sleep_lock_status & (1 << gprs_cntx->sleep_manager_handler))) {
        LOGI("sleep is not lock, sleep lock.\r\n");
        hal_sleep_manager_lock_sleep(gprs_cntx->sleep_manager_handler);
        sleep_lock_status = hal_sleep_manager_get_lock_status();
        LOGI("gprs_set_host_wakeup() end %d\r\n", sleep_lock_status);     

        sprintf((char *)resonse.response_buf, "%d sleep manager = %d", (int)gprs_cntx->sleep_manager_handler, (int)sleep_lock_status);
        resonse.response_len = strlen((char *)resonse.response_buf);
        resonse.response_flag |= ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        atci_send_response(&resonse);

        msm_notify_host_status_to_modem(MSM_STATUS_ACTIVE);
    } else {
        LOGI("sleep is locked.\r\n");
    }

    return GPRS_RET_OK;
}

gprs_ret_t gprs_set_host_sleep()
{
    uint32_t sleep_lock_status;
    atci_response_t resonse;

    LOGI("gprs_set_host_sleep()\r\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    sleep_lock_status = hal_sleep_manager_get_lock_status();
    LOGI("gprs_set_host_sleep() begin %d %d\r\n", sleep_lock_status, gprs_cntx->sleep_manager_handler);
    if (sleep_lock_status & (1 << gprs_cntx->sleep_manager_handler)) {
        LOGI("sleep is lock, sleep unlock.\r\n");
        hal_sleep_manager_unlock_sleep(gprs_cntx->sleep_manager_handler);
        sleep_lock_status = hal_sleep_manager_get_lock_status();
        LOGI("gprs_set_host_sleep() end %d\r\n", sleep_lock_status);

        sprintf((char *)resonse.response_buf, "%d sleep manager = %d", (int)gprs_cntx->sleep_manager_handler, (int)sleep_lock_status);
        resonse.response_len = strlen((char *)resonse.response_buf);
        resonse.response_flag |= ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        atci_send_response(&resonse);
        
        msm_notify_host_status_to_modem(MSM_STATUS_INACTIVE);
    } else {
        LOGI("sleep is unlocked.\r\n");
    }

    return GPRS_RET_OK;
}

#endif
