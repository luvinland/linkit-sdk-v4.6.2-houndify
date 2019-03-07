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
// For Register AT command handler
#include "atci.h"
// System head file
#include <string.h>
#include <stdio.h>
#include "syslog.h"
#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "nvdm.h"
#include "task_def.h"

#include "httpclient.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "queue.h"
#ifdef MODEM_ENABLE
#include "gprs_api.h"
#include "main_screen.h"
#include "sio_gprot.h"
#include "msm.h"
#include "urc_app.h"
#include "MQTTClient.h"
#include "nw_atci.h"
#include "hal_uart.h"
#endif
#include "iperf_task.h"
#ifdef TCPIP_FOR_MT5931_ENABLE
#include "mtk_wifi_adapter.h"
#include "kal_public_api.h"
#include "wifi_atci.h"
#endif
#include "ping.h"

#ifdef MTK_FOTA_ENABLE
#include "fota_demo.h"
#endif

#ifdef LOGI
#undef LOGI
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif

log_create_module(atprj, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(atprj, "ATCI project: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atprj, "ATCI project: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atprj, "ATCI project: "fmt,##arg)


#define DEMO_APP_SUPPORT_IPERF

/*
 * sample code
*/    
#ifdef MODEM_ENABLE
extern void fota_event_hdl(message_id_enum event_id, int32_t param1, void *param2);
extern void gnss_show_screen(void);
extern void gnss_nw_event_hdl(message_id_enum event_id, int32_t param1, void *param2);
extern sio_uart_ret_t sio_uart_set_status(sio_uart_status_t status);

extern hal_uart_status_t hal_uart_set_dte_dtr_active(hal_uart_port_t uart_port);
extern hal_uart_status_t hal_uart_set_dte_dtr_deactive(hal_uart_port_t uart_port);
#endif

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_testcmd(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_mrip(atci_parse_cmd_param_t *parse_cmd);
#ifdef DEMO_APP_SUPPORT_IPERF
atci_status_t atci_cmd_hdlr_iperf(atci_parse_cmd_param_t *parse_cmd);
static uint8_t atci_cmd_iperf_client(char *param);
static uint8_t atci_cmd_iperf_server(char *param);
#endif

#ifdef MTK_FOTA_ENABLE
static atci_status_t atci_cmd_hdlr_fota_cmd(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef MODEM_ENABLE
static void atci_cmd_iperf_cb(iperf_result_t* iperf_result);
atci_status_t gnss_test_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_epo_erase_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_epo_set_time_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_loc_set_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_epo_erase_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_agps_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t gnss_gps_at_handler(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_https(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_mqtt(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_modem(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_raw(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_raws(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_rawss(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_tcp(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_udp(atci_parse_cmd_param_t *parse_cmd);
#endif
#ifdef TCPIP_FOR_MT5931_ENABLE
extern uint8_t wndrv_test_ehpi_loopback_test(char *inject_string);
atci_status_t atci_cmd_hdlr_ping(atci_parse_cmd_param_t *parse_cmd);
atci_status_t atci_cmd_hdlr_loopback_cmd(atci_parse_cmd_param_t *parse_cmd);
void wifi_loopback_test(char * inject_string);
void wifi_loopback_task(void *arg);
#endif

/*---  Variant ---*/
atci_cmd_hdlr_item_t item_table[] = {
    {"AT+TESTCMD",         atci_cmd_hdlr_testcmd,         0, 0},
    {"AT+MRIP",         atci_cmd_hdlr_mrip,         0, 0},
#ifdef DEMO_APP_SUPPORT_IPERF
    {"AT+IPERF",         atci_cmd_hdlr_iperf,         0, 0},
#endif
#ifdef MODEM_ENABLE
#ifdef MTK_FOTA_ENABLE
    {"AT+FOTA",         atci_cmd_hdlr_fota_cmd,         0, 0},
#endif
    {"AT+EGPST", 		gnss_test_at_handler, 		0, 0},
    {"AT+EGPSEPOE",     gnss_epo_erase_at_handler, 	0, 0},
    {"AT+EGPSEPOS",     gnss_epo_set_time_at_handler,   0, 0},
    {"AT+EGPSLOC",     gnss_loc_set_at_handler,   0, 0},
    {"AT+AGPSEPO",     gnss_agps_at_handler,   0, 0},
    {"AT+AGPSNTP",     gnss_agps_at_handler,   0, 0},
    {"AT+AGPSSLP",     gnss_agps_at_handler,   0, 0},
    {"AT+AGPST",     gnss_agps_at_handler,   0, 0},
    {"AT+EGPSC_DEMO",     gnss_gps_at_handler,   0, 0},
    {"AT+EGPSS_DEMO",     gnss_gps_at_handler,   0, 0},
    {"AT+POWERGPS",     gnss_agps_at_handler,   0, 0},
    {"AT+HTTPS",        atci_cmd_hdlr_https,        0, 0},
    {"AT+MQTT",         atci_cmd_hdlr_mqtt,         0, 0},
    {"AT+MD",     atci_cmd_hdlr_modem, 	0, 0},
    {"AT+RAW",          atci_cmd_hdlr_raw,            0, 0},
    {"AT+RAWS",          atci_cmd_hdlr_raws,            0, 0},
    {"AT+RAWSS",          atci_cmd_hdlr_rawss,            0, 0},

    {"AT+TCP",          atci_cmd_hdlr_tcp,            0, 0},
    {"AT+UDP",          atci_cmd_hdlr_udp,            0, 0},
#endif

#ifdef TCPIP_FOR_MT5931_ENABLE
    {"AT+PING",          atci_cmd_hdlr_ping,         0, 0},
#if CFG_SUPPORT_LOOPBACK
    {"AT+LB",           atci_cmd_hdlr_loopback_cmd, 0, 0},
#endif
    {"AT+WIFI",         atci_cmd_hdlr_wifi_cmd,     0, 0},
    {"AT+WIFIEM",       atci_cmd_hdlr_wifi_em_cmd,  0, 0},
#endif
    };


void atci_example_init()
{
    atci_status_t ret = ATCI_STATUS_REGISTRATION_FAILURE;

    LOGI("atci_example_init\r\n");

    // --------------------------------------------------------- //
    // ------- Test Scenario: register AT handler in CM4 ------- //
    // --------------------------------------------------------- //
    ret = atci_register_handler(item_table, sizeof(item_table) / sizeof(atci_cmd_hdlr_item_t));
    if (ret == ATCI_STATUS_OK) {
		LOGI("at_example_init register success\r\n");
	} else {
		LOGW("at_example_init register fail\r\n");
	}

}


/* AT command handler */
atci_status_t atci_cmd_hdlr_mrip(atci_parse_cmd_param_t *parse_cmd)
{
	atci_response_t resonse;
	char *param = NULL;
	struct ip4_addr ip4;
	nvdm_status_t nv_ret = NVDM_STATUS_ERROR;

	LOGW("atci_cmd_hdlr_mrip\n");

	memset(&resonse, 0, sizeof(atci_response_t));
	resonse.response_flag = 0; // Command Execute Finish.

	switch (parse_cmd->mode) {
		case ATCI_CMD_MODE_TESTING:    // rec: AT+MRIP=?
			LOGI("AT Test OK.\n");
			strcpy((char *)resonse.response_buf, "\r\n+MRIP:(0,1)\r\nOK\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);
			break;

		case ATCI_CMD_MODE_READ:	// rec: AT+MRIP?
			LOGI("AT Read done.\n");
			sprintf((char *)resonse.response_buf,"\r\n+MRIP:<ip>\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;	// ATCI will help append OK at the end of resonse buffer
			atci_send_response(&resonse);
			break;

		case ATCI_CMD_MODE_ACTIVE:	// rec: AT+MRIP
			LOGI("AT Active OK.\n");
			// assume the active mode is invalid and we will return "ERROR"
			strcpy((char *)resonse.response_buf, "\r\nERROR\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);
			break;

		case ATCI_CMD_MODE_EXECUTION: // rec: AT+MRIP=<p1>  the handler need to parse the parameters
			LOGI("AT Executing...\r\n");
			//parsing the parameter
			/*param = strtok(parse_cmd->string_ptr, ",\r\n");
			param = strtok(parse_cmd->string_ptr, "AT+MRIP=");*/
			LOGI("AT CMD received:%s", parse_cmd->string_ptr);
			param = strchr(parse_cmd->string_ptr, '=');

			if (param && 0 != inet_aton(++param, &ip4))
			{
				LOGI("param2:%s", param);
				
				nv_ret = nvdm_write_data_item("common", "RemoteIp", 
											  NVDM_DATA_ITEM_TYPE_STRING, 
											  (uint8_t *)param, strlen(param));				
				if (NVDM_STATUS_OK == nv_ret)
				{
					// valid parameter, update the data and return "OK"
					/*resonse.response_len = 0;
					resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK; // ATCI will help append "OK" at the end of resonse buffer */
					strcpy((char *)resonse.response_buf, "\r\nOK\r\n");
					resonse.response_len = strlen((char *)resonse.response_buf);
					atci_send_response(&resonse);
					break;
				}
			}
		
			// invalide parameter, return "ERROR"
			/*resonse.response_len = 0;
			resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer*/
			strcpy((char *)resonse.response_buf, "\r\nERROR\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);			
			break;
			
		default :
			strcpy((char *)resonse.response_buf, "ERROR\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);
			break;
	}
	return ATCI_STATUS_OK;
}



atci_status_t atci_cmd_hdlr_testcmd(atci_parse_cmd_param_t *parse_cmd)
{
    static int test_param1 = 0;
    atci_response_t resonse;
    char *param = NULL;
    int  param1_val = -1;

    LOGI("atci_cmd_hdlr_testcmd\n");
    memset(&resonse, 0, sizeof(atci_response_t));
    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+TESTCMD=?
            LOGI("AT Test OK.\n");
            strcpy((char *)resonse.response_buf, "+TESTCMD:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_READ:    // rec: AT+TESTCMD?
            LOGI("AT Read done.\n");
            sprintf((char *)resonse.response_buf,"+TESTCMD:%d\r\n", test_param1);
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
            atci_send_response(&resonse);
            break;
    
        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+TESTCMD
            LOGW("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+TESTCMD=<p1>  the handler need to parse the parameters
            LOGW("AT Executing...\r\n");
            //parsing the parameter
            param = strtok(parse_cmd->string_ptr, ",\n\r");
	        param = strtok(parse_cmd->string_ptr, "AT+TESTCMD=");
            param1_val = atoi(param);
		
	    if (param != NULL && (param1_val == 0 || param1_val == 1)){
                
                // valid parameter, update the data and return "OK"
                resonse.response_len = 0;
                resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK; // ATCI will help append "OK" at the end of resonse buffer 
            } else {
                // invalide parameter, return "ERROR"
                resonse.response_len = 0;
                resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
            };
            atci_send_response(&resonse);
			param = NULL;
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}


#ifdef MODEM_ENABLE
void httpclient_test_stability(void)
{
    #define HTTPS_MTK_CLOUD_GET_URL     "https://api.mediatek.com/mcs/v2/devices/D0n2yhrl/datachannels/1/datapoints"
    #define HTTPS_MTK_CLOUD_POST_URL    "https://api.mediatek.com/mcs/v2/devices/D0n2yhrl/datapoints.csv"
    #define HTTPS_MTK_CLOUD_HEADER      "deviceKey:FZoo0S07CpwUHcrt\r\n"
    #define BUF_SIZE        (1024 * 1)
    
    char *get_url = HTTPS_MTK_CLOUD_GET_URL;
    char *post_url = HTTPS_MTK_CLOUD_POST_URL;
    char *header = HTTPS_MTK_CLOUD_HEADER;
    char *content_type = "text/csv";
    int ret = HTTPCLIENT_ERROR_CONN;
    httpclient_t client1 = {0}, client2 = {0};
    char *buf1 = NULL, *buf2 = NULL;
    httpclient_data_t client_data = {0};
    char post_data[32];
    int count = 0;

    buf1 = pvPortMalloc(BUF_SIZE);
    if (buf1 == NULL) {
        LOGI("[HTTPS CLIENT]: httpclient_test_keepalive buf1 malloc failed.\r\n");
        return;
    }

    buf2 = pvPortMalloc(BUF_SIZE);
    if (buf2 == NULL) {
        LOGI("[HTTPS CLIENT]: httpclient_test_keepalive buf2 malloc failed.\r\n");
        vPortFree(buf1);
        buf1 = NULL;
        return;
    }
    
    sio_trigger_modem_wakeup();

    ret = httpclient_connect(&client1, get_url);
    if (ret != HTTPCLIENT_OK) {
        LOGI("[HTTPS CLIENT]: httpclient_connect failed 1 %d.\r\n", ret);        
        httpclient_close(&client1);
        vPortFree(buf1);
        buf1 = NULL;
        
        vPortFree(buf2);
        buf2 = NULL;
        return;
    }
    ret = httpclient_connect(&client2, post_url);
    if (ret != HTTPCLIENT_OK) {
        LOGI("[HTTPS CLIENT]: httpclient_connect failed 2 %d.\r\n", ret);
        
        httpclient_close(&client1);
        vPortFree(buf1);
        buf1 = NULL;
        
        httpclient_close(&client2);
        vPortFree(buf2);
        buf2 = NULL;
        return;
    }
    
    buf1[0] = '\0';
    buf2[0] = '\0';

    while(1) {
        memset(buf1, 0, BUF_SIZE);
        client_data.response_buf = buf1;
        client_data.response_buf_len = BUF_SIZE;

        ret = httpclient_send_request(&client1, get_url, HTTPCLIENT_GET, &client_data);
        if (ret < 0) {
            LOGI("[HTTPS CLIENT]: get send %d. \r\n", ret);
            break;
        }
        ret = httpclient_recv_response(&client1, &client_data);
        if (ret < 0) {
            LOGI("[HTTPS CLIENT]: get recv %d. \r\n", ret);
            break;
        }

        memset(buf2, 0, BUF_SIZE);
        client_data.response_buf = buf2;
        client_data.response_buf_len = BUF_SIZE;
        
        client_data.post_content_type = content_type;
        memset(post_data, 0, 32);
        sprintf((char *)post_data, "1,temperature:%d", (10 + count));
        client_data.post_buf = post_data;
        client_data.post_buf_len = strlen((char *)post_data);
        httpclient_set_custom_header(&client2, header);

        ret = httpclient_send_request(&client2, post_url, HTTPCLIENT_POST, &client_data);
        if (ret < 0) {
            LOGI("[HTTPS CLIENT]: post send %d. \r\n", ret);
            break;
        }
        ret = httpclient_recv_response(&client2, &client_data);
        if (ret < 0) {
            LOGI("[HTTPS CLIENT]: post recv %d. \r\n", ret);
            break;
        }

        count++;
        LOGI("[HTTPS CLIENT]: count %d. \r\n", count);

        if (count == 1000) {
            break;
        }
    }
    
    LOGI("[HTTPS CLIENT]: count %d. \r\n", count);

    LOGI("[HTTPS CLIENT]: get close. END\r\n");
    httpclient_close(&client1);
    sio_trigger_modem_sleep();
    vPortFree(buf1);
    buf1 = NULL;

    LOGI("[HTTPS CLIENT]: post close. END\r\n");
    httpclient_close(&client2);
    vPortFree(buf2);
    buf2 = NULL;
}

void httpsclient_task(void *parameter)
{
    httpclient_test_stability();

    for (;;) {
        ;
    }
}

void httpsclient_callback()
{
    xTaskCreate(httpsclient_task, HTTPS_CLIENT_TASK_NAME, HTTPS_CLIENT_TASK_STACKSIZE/(( uint32_t )sizeof( StackType_t )), NULL, IPERF_TASK_PRIO, NULL);
}

static int arrivedcount = 0;

/*MQTT message RX handler*/
static void messageArrived(MessageData *md)
{
    MQTTMessage *message = md->message;

    LOGI("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", message->qos, message->retained, message->dup, message->id);
    LOGI("Payload %d.%s\n", (size_t)(message->payloadlen), (char *)(message->payload));
    ++arrivedcount;
    LOGI("arrivedcount %d\n", arrivedcount);
}

/*MQTT client example*/
void mqtt_client_run()
{
    #define MQTT_SERVER		"iot.eclipse.org"//"test.mosquitto.org"
    #define MQTT_PORT		"1883"
    #define MQTT_TOPIC		"7687test"
    #define MQTT_CLIENT_ID	"mqtt-7687-client"
    #define MQTT_MSG_VER	"0.50"

    int rc = 0;
    unsigned char msg_buf[100];     //Buffer for outgoing messages, such as unsubscribe.
    unsigned char msg_readbuf[100]; //Buffer for incoming messages, such as unsubscribe ACK.
    char buf[100];                  //Buffer for application payload.

    Network n;  //TCP network
    Client c;   //MQTT client
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    char *topic = MQTT_TOPIC;
    MQTTMessage message;
    
    LOGI("mqtt_client_example running! \n");

    sio_trigger_modem_wakeup();

    //Initialize MQTT network structure
    NewNetwork(&n);

    //Connect to remote server
    LOGI("Connect to %s:%s\n", MQTT_SERVER, MQTT_PORT);
    rc = ConnectNetwork(&n, MQTT_SERVER, MQTT_PORT);    
    
    LOGI("TCP connectstatus %d\n", rc);

    if (rc != 0) {
        LOGI("TCP connect failed,status -%4X\n", -rc);
        vTaskDelete(NULL);
    }

    //Initialize MQTT client structure
    MQTTClient(&c, &n, 12000, msg_buf, 100, msg_readbuf, 100);

    //The packet header of MQTT connection request
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = MQTT_CLIENT_ID;
    data.username.cstring = NULL;
    data.password.cstring = NULL;
    data.keepAliveInterval = 10;
    data.cleansession = 1;

    //Send MQTT connection request to the remote MQTT server
    rc = MQTTConnect(&c, &data);

    if (rc != 0) {
        LOGI("MQTT connect failed,status%d\n", rc);
    }

    LOGI("Subscribing to %s\n", topic);
    rc = MQTTSubscribe(&c, topic, QOS1, messageArrived);
    LOGI("Client Subscribed %d\n", rc);


    // QoS 0
    sprintf(buf, "Hello World! QoS 0 message from app version %s\n", MQTT_MSG_VER);
    message.qos = QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void *)buf;
    message.payloadlen = strlen((char *)buf) + 1;
    rc = MQTTPublish(&c, topic, &message);

    while (arrivedcount < 1) {
        MQTTYield(&c, 1000);
    }

    // QoS 1
    sprintf(buf, "Hello World! QoS 1 message from app version %s\n", MQTT_MSG_VER);
    message.qos = QOS1; 
    message.payloadlen = strlen((char *)buf) + 1;
    rc = MQTTPublish(&c, topic, &message);
    while (arrivedcount < 2) {
        MQTTYield(&c, 1000);
    }

    // QoS 2
    sprintf(buf, "Hello World! QoS 2 message from app version %s\n", MQTT_MSG_VER);
    message.qos = QOS2;
    message.payloadlen = strlen((char *)buf) + 1;
    rc = MQTTPublish(&c, topic, &message);
    while (arrivedcount < 3) {
        MQTTYield(&c, 1000);
    }

    if ((rc = MQTTUnsubscribe(&c, topic)) != 0) {
        LOGI("The return from unsubscribe was %d\n", rc);
    }
    LOGI("MQTT unsubscribe done\n");

    if ((rc = MQTTDisconnect(&c)) != 0) {
        LOGI("The return from disconnect was %d\n", rc);
    }
    LOGI("MQTT disconnect done\n");

    n.disconnect(&n);
    LOGI("Network disconnect done\n");
    
    sio_trigger_modem_sleep();

    vTaskDelay(300);
    
    vTaskDelete(NULL);
}

void mqtt_client_example()
{
    LOGI("mqtt_client_example, create task.\n");
    xTaskCreate((TaskFunction_t)mqtt_client_run, MQTT_TASK_NAME, MQTT_TASK_STACKSIZE / (( uint32_t )sizeof( StackType_t )), NULL, MQTT_TASK_PRIO, NULL);
}

atci_status_t atci_cmd_hdlr_mqtt(atci_parse_cmd_param_t *parse_cmd)
{
    static int test_param1 = 0;
    atci_response_t resonse;

    LOGI("atci_cmd_hdlr_mqtt\n");
    memset(&resonse, 0, sizeof(atci_response_t));
   
    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+MQTT=?
            LOGI("AT Test OK.\n");
            strcpy((char *)resonse.response_buf, "+MQTTCMD:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_READ:    // rec: AT+MQTT?
            LOGI("AT Read done.\n");
            sprintf((char *)resonse.response_buf,"+MQTTCMD:%d\r\n", test_param1);
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+MQTT
            LOGI("AT Active OK.\n");
            strcpy((char *)resonse.response_buf, "OK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);
            mqtt_client_example();
            atci_send_response(&resonse);
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}

atci_status_t atci_cmd_hdlr_https(atci_parse_cmd_param_t *parse_cmd)
{
    static int test_param1 = 0;
    atci_response_t resonse;

    LOGI("atci_cmd_hdlr_https\n");

    memset(&resonse, 0, sizeof(atci_response_t));
    
    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+HTTPS=?
            LOGI("AT Test OK.\n");
            strcpy((char *)resonse.response_buf, "+HTTPSCMD:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_READ:    // rec: AT+HTTPS?
            LOGI("AT Read done.\n");
            sprintf((char *)resonse.response_buf,"+HTTPSCMD:%d\r\n", test_param1);
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+HTTPS
            LOGI("AT Active OK.\n");
            strcpy((char *)resonse.response_buf, "OK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);
            httpsclient_callback();
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}

void atci_sio_callback(uint32_t event, void *data)
{
    if (event == SIO_UART_EVENT_MODEM_DATA_TO_READ) {
        atci_response_t resonse;
        uint8_t *payload;
        uint8_t length;
        
        sio_rx_data_to_read_struct *rx_data = (sio_rx_data_to_read_struct *)pvPortCalloc(1, sizeof(sio_rx_data_to_read_struct));
        memcpy(rx_data, (sio_rx_data_to_read_struct *)data, sizeof(sio_rx_data_to_read_struct));
        payload = pvPortCalloc(1, rx_data->read_length);
        memset(&payload, 0, rx_data->read_length);
        
        length = sio_receive_data(rx_data->app_id, payload, rx_data->read_length);

        if (length) {
            LOGI("AT response: %s", payload);
            
            memset(&resonse, 0, sizeof(atci_response_t));
            strcpy((char *)resonse.response_buf, (const char *)payload);
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;
            atci_send_response(&resonse);
        }
        
        vPortFree(payload);
        vPortFree(rx_data);
    }
}


urc_cb_ret atci_urc_callback(uint8_t *payload, uint32_t length)
{    
    atci_response_t resonse;
    
    LOGI("URC %s", payload);
    memset(&resonse, 0, sizeof(atci_response_t));
    strncpy((char *)resonse.response_buf, (const char *)payload, length);
    resonse.response_len = strlen((char *)resonse.response_buf);
    resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;   
    atci_send_response(&resonse);

    return RET_OK_CONTINUE;
}

atci_status_t atci_cmd_hdlr_modem(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse ;

    LOGI("atci_cmd_hdlr_modem\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+MD=?
            LOGI("AT Modem OK.\n");
            strcpy((char *)resonse.response_buf, "+MODEM:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;            

       case ATCI_CMD_MODE_READ:    // rec: AT+MD?
            LOGI("AT Read done.\n");
            strcpy((char *)resonse.response_buf, "+MODEM:ok \r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+MD
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+MD=<p1>  the handler need to parse the parameters
            LOGI("AT Executing... AT+MD \r\n");
            if (strstr((char *) parse_cmd->string_ptr, "AT+MD=0") != NULL) {
                // POWER OFF MODEM
                if (MSM_RET_OK == msm_trigger_to_modem(MSM_TRIGGER_TYPE_POWEROFF)) {
                    strcpy((char *)resonse.response_buf, "POWER OFF MODEM SUCCESS!\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                } else {
                    strcpy((char *)resonse.response_buf, "POWER OFF MODEM FAIL!\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                }
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+MD=1") != NULL) {
                // POWER ON MODEM
                if (MSM_RET_OK == msm_trigger_to_modem(MSM_TRIGGER_TYPE_POWERON)) {
                    strcpy((char *)resonse.response_buf, "POWER ON MODEM SUCCESS!\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                } else {
                    strcpy((char *)resonse.response_buf, "POWER ON MODEM FAIL!\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                }
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+MD=2") != NULL) {
                sio_ret_t ret = SIO_RET_ERROR;
                
                // ACTIVATE MODEM 
                while(1) {
                    ret = sio_trigger_modem_ready();

                    LOGI("ret %d", ret);
                
                    if (SIO_RET_BUSY == ret) {  
                        vTaskDelay(200);
                        continue;
                    }
                    else if(SIO_RET_OK == ret) {
                        nw_atci_registration_network();
                    }
                    break;
                }
                
                strcpy((char *)resonse.response_buf, "START REGISTRATION NETWORK!!\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+MD=3") != NULL) {
                sio_ret_t ret = SIO_RET_ERROR;
                
                // ACTIVATE MODEM 
                while(1) {
                    ret = sio_trigger_modem_ready();

                    LOGI("ret %d", ret);
                
                    if (SIO_RET_BUSY == ret) {  
                        vTaskDelay(200);
                        continue;
                    }
                    else if(SIO_RET_OK == ret) {
                        nw_atci_gprs_activate();
                    }
                    break;
                }
                
                strcpy((char *)resonse.response_buf, "START ACTIVATE MODEM!\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
            } else if (strstr((char *)parse_cmd->string_ptr, "AT+MD=4") != NULL) {
                sio_ret_t ret = SIO_RET_ERROR;
                
                // DEACTIVATE MODEM
                while(1) {
                    ret = sio_trigger_modem_ready();

                    LOGI("ret %d", ret);
                    
                    if (SIO_RET_BUSY == ret) {  
                        vTaskDelay(200);
                        continue;
                    }
                    else if(SIO_RET_OK == ret) {
                        nw_atci_send_ping_req();
                    }
                    
                    break;
                }               
                strcpy((char *)resonse.response_buf, "START SEND PING!\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+MD=5") != NULL) {
                
                sio_ret_t ret = SIO_RET_ERROR;
                
                // DEACTIVATE MODEM
                while(1) {
                    ret = sio_trigger_modem_ready();

                    LOGI("ret %d", ret);
                    
                    if (SIO_RET_BUSY == ret) {  
                        vTaskDelay(200);
                        continue;
                    }
                    else if(SIO_RET_OK == ret) {
                        nw_atci_gprs_deactivate();
                    }
                    
                    break;
                }               
                strcpy((char *)resonse.response_buf, "START DEACTIVATE MODEM!\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
            }else if (strstr((char *) parse_cmd->string_ptr, "AT+MD=6") != NULL) {
                // QUERY MODEM STATUS
                /*if (MSM_STATUS_ACTIVE == msm_query_status_from_modem()) {
                    strcpy((char *)resonse.response_buf, "MODEM PWOER ON!\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                } else {
                    strcpy((char *)resonse.response_buf, "MODEM PWOER OFF!\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                }*/
                strcpy((char *)resonse.response_buf, "MODEM PWOER Not support!\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
                resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;
            }
            atci_send_response(&resonse);
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
       }
    return ATCI_STATUS_OK;
}

atci_status_t atci_cmd_hdlr_raw(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse ;

    LOGI("atci_cmd_hdlr_modem\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+RAW=?
            LOGI("AT Modem OK.\n");
            strcpy((char *)resonse.response_buf, "+RAW:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;            

       case ATCI_CMD_MODE_READ:    // rec: AT+RAW?
            LOGI("AT Read done.\n");
            strcpy((char *)resonse.response_buf, "+RAW:ok \r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+RAW
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+RAW:<p1>  the handler need to parse the parameters        
        {
            char* context = NULL;
            int context_len = 0;
            int header_len = 0;
            hal_uart_status_t uart_ret;
            
            LOGI("AT Executing... AT+RAW \r\n");

            header_len = strlen((char *)"AT+RAW:");
            context_len = strlen((char *)parse_cmd->string_ptr) - header_len;
            context = pvPortMalloc(context_len + 1);

            strcpy((char *)context, parse_cmd->string_ptr + header_len);

            LOGI("context %s", context);
            sio_uart_set_status(SIO_UART_STATUS_READY);
            // TODO:
            //sio_uart_set_notification_callback(1, gprs_get_app_id(), SIO_UART_EVENT_MAX_NUMBER, atci_sio_callback);
            urc_register_callback(atci_urc_callback);
            uart_ret = hal_uart_set_dte_dtr_active(HAL_UART_2);
            vTaskDelay(500);
            LOGI("hal_uart_set_dte_dtr_active %d", uart_ret);
            sio_send_data(gprs_get_app_id(), (const uint8_t *)context, strlen((char *)context));
            vPortFree(context);
            strcpy((char *)resonse.response_buf, parse_cmd->string_ptr);
            resonse.response_len = strlen((char *)resonse.response_buf);            
            resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;           
            atci_send_response(&resonse);
        }
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
       }
    return ATCI_STATUS_OK;
}

atci_status_t atci_cmd_hdlr_raws(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse ;

    LOGI("atci_cmd_hdlr_modem\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+RAW=?
            LOGI("AT Modem OK.\n");
            strcpy((char *)resonse.response_buf, "+RAW:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;            

       case ATCI_CMD_MODE_READ:    // rec: AT+RAW?
            LOGI("AT Read done.\n");
            strcpy((char *)resonse.response_buf, "+RAW:ok \r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+RAW
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+RAW:<p1>  the handler need to parse the parameters        
        {
            char* context = NULL;
            int context_len = 0;
            int header_len = 0;
            
            LOGI("AT Executing... AT+RAW \r\n");

            header_len = strlen((char *)"AT+RAW:");
            context_len = strlen((char *)parse_cmd->string_ptr) - header_len;
            context = pvPortMalloc(context_len + 1);

            strcpy((char *)context, parse_cmd->string_ptr + header_len);

            LOGI("context %s", context);
            sio_uart_set_status(SIO_UART_STATUS_READY);
            // TODO:
            //sio_uart_set_notification_callback(1, gprs_get_app_id(), SIO_UART_EVENT_MAX_NUMBER, atci_sio_callback);
            urc_register_callback(atci_urc_callback);            
            sio_send_data(gprs_get_app_id(), (const uint8_t *)context, strlen((char *)context));
            vPortFree(context);
            strcpy((char *)resonse.response_buf, parse_cmd->string_ptr);
            resonse.response_len = strlen((char *)resonse.response_buf);            
            resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;           
            atci_send_response(&resonse);
        }
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
       }
    return ATCI_STATUS_OK;
}

atci_status_t atci_cmd_hdlr_rawss(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse ;

    LOGI("atci_cmd_hdlr_modem\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+RAW=?
            LOGI("AT Modem OK.\n");
            strcpy((char *)resonse.response_buf, "+RAW:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;            

       case ATCI_CMD_MODE_READ:    // rec: AT+RAW?
            LOGI("AT Read done.\n");
            strcpy((char *)resonse.response_buf, "+RAW:ok \r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+RAW
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+RAW:<p1>  the handler need to parse the parameters        
        {            
            hal_uart_status_t uart_ret;
            
            LOGI("AT Executing... AT+RAW \r\n");
            uart_ret = hal_uart_set_dte_dtr_deactive(HAL_UART_2);
            LOGI("hal_uart_set_dte_dtr_deactive %d", uart_ret);
            strcpy((char *)resonse.response_buf, parse_cmd->string_ptr);
            resonse.response_len = strlen((char *)resonse.response_buf);            
            resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;           
            atci_send_response(&resonse);
        }
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
       }
    return ATCI_STATUS_OK;
}


atci_status_t atci_cmd_hdlr_udp(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse ;

    LOGI("atci_cmd_hdlr_udp\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+RAW=?
            LOGI("AT UDP OK.\n");
            strcpy((char *)resonse.response_buf, "+UDP:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;            

       case ATCI_CMD_MODE_READ:    // rec: AT+RAW?
            LOGI("AT Read done.\n");
            strcpy((char *)resonse.response_buf, "+UDP:ok \r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+RAW
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+RAW:<p1>  the handler need to parse the parameters        
        {                 
            char* context = NULL;
            int context_len = 0;
            int header_len = 0;
            hal_uart_status_t uart_ret;
            int sockfd;
            struct sockaddr_in servaddr;
            char *str = NULL;
            ip_addr_t address;
            
            LOGI("AT Executing... AT+UDP \r\n");

            header_len = strlen((char *)"AT+UDP=");
            context_len = strlen((char *)parse_cmd->string_ptr) - header_len;
            context = pvPortMalloc(context_len + 1);

            strcpy((char *)context, parse_cmd->string_ptr + header_len);

            LOGI("UDP ipaddr %s", context);
            uart_ret = hal_uart_set_dte_dtr_active(HAL_UART_2);
            vTaskDelay(500);
            LOGI("hal_uart_set_dte_dtr_active %d", uart_ret);

            if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                LOGI("socket fail %d", sockfd);
                vPortFree(context);
                goto exit;
            }
            
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = 80;      
            ipaddr_aton(context, &address);            
            inet_addr_from_ipaddr(&servaddr.sin_addr, &address);
            
            LOGI("address %d", servaddr.sin_addr.s_addr);
            
            if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
                LOGI("Connect failed\n");
                close(sockfd);
                vPortFree(context);
                goto exit;
            }
            
            str = pvPortCalloc(1, 100);
            if (str == NULL) {
                LOGI("not enough buffer to send data!\n");
                close(sockfd);
                vPortFree(context);
                goto exit;
            }
            memset(str, '2', 100);  // TODO: send data

            if (send(sockfd, str, strlen(str), 0)){
                LOGI("not enough buffer to send data!\n");
                close(sockfd);
                vPortFree(str);
                vPortFree(context);
                goto exit;
            }
            
            vPortFree(context);
            vPortFree(str);
            exit:
            strcpy((char *)resonse.response_buf, parse_cmd->string_ptr);
            resonse.response_len = strlen((char *)resonse.response_buf);            
            resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;           
            atci_send_response(&resonse);
        }
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
       }
    return ATCI_STATUS_OK;
}


atci_status_t atci_cmd_hdlr_tcp(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse ;

    LOGI("atci_cmd_hdlr_tcp\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+RAW=?
            LOGI("AT TCP OK.\n");
            strcpy((char *)resonse.response_buf, "+RAW:(0,1)\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;            

       case ATCI_CMD_MODE_READ:    // rec: AT+RAW?
            LOGI("AT Read done.\n");
            strcpy((char *)resonse.response_buf, "+RAW:ok \r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+RAW
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+RAW:<p1>  the handler need to parse the parameters        
        {                             
            char* context = NULL;
            int context_len = 0;
            int header_len = 0;
            hal_uart_status_t uart_ret;
            int sockfd;
            struct sockaddr_in servaddr;
            char *str = NULL;
            ip_addr_t address;
            
            LOGI("AT Executing... AT+UDP \r\n");

            header_len = strlen((char *)"AT+UDP=");
            context_len = strlen((char *)parse_cmd->string_ptr) - header_len;
            context = pvPortMalloc(context_len + 1);

            strcpy((char *)context, parse_cmd->string_ptr + header_len);

            LOGI("UDP ipaddr %s", context);
            uart_ret = hal_uart_set_dte_dtr_active(HAL_UART_2);
            vTaskDelay(500);
            LOGI("hal_uart_set_dte_dtr_active %d", uart_ret);

            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                LOGI("socket fail %d", sockfd);
                vPortFree(context);
                goto exit;
            }
            
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = 80;            
            ipaddr_aton(context, &address);      
            inet_addr_from_ipaddr(&servaddr.sin_addr, &address);            
            LOGI("address %d", servaddr.sin_addr.s_addr);
            
            if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
                LOGI("Connect failed\n");
                close(sockfd);
                vPortFree(context);
                goto exit;
            }
            
            str = pvPortCalloc(1, 100);
            if (str == NULL) {
                LOGI("not enough buffer to send data!\n");
                close(sockfd);
                vPortFree(context);
                goto exit;
            }
            memset(str, '2', 100);  // TODO: send data

            if (send(sockfd, str, strlen(str), 0)){
                LOGI("not enough buffer to send data!\n");
                close(sockfd);
                vPortFree(context);
                vPortFree(str);
                goto exit;
            }
            
            vPortFree(context);
            vPortFree(str);
            exit:
            strcpy((char *)resonse.response_buf, parse_cmd->string_ptr);
            resonse.response_len = strlen((char *)resonse.response_buf);            
            resonse.response_flag |= ATCI_RESPONSE_FLAG_QUOTED_WITH_LF_CR;           
            atci_send_response(&resonse);
        }
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
       }
    return ATCI_STATUS_OK;
}
#endif

void ping_request_result(ping_result_t *result)
{
    atci_response_t resonse = {{0}};
    int len = 0;

    resonse.response_flag = 0; // Command Execute Finish.
    resonse.response_len  = 0;
    sprintf((char *)resonse.response_buf, "Packets: Sent = %d, Received =%d, Lost = %d (%d%% loss)\n\r", (int)result->total_num, (int)result->recv_num, (int)result->lost_num, (int)((result->lost_num * 100)/result->total_num));
    len = strlen((char *)resonse.response_buf);
    sprintf((char *)(resonse.response_buf + len), "Packets: min = %d, max =%d, avg = %d\n", (int)result->min_time, (int)result->max_time, (int)result->avg_time);
    resonse.response_len = strlen((char *)resonse.response_buf);
    atci_send_response(&resonse);
}

#ifdef TCPIP_FOR_MT5931_ENABLE
atci_status_t atci_cmd_hdlr_ping(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse;
    char *param = NULL;
    int count = 3;
    int pktsz = 64;

    LOGI("atci_cmd_hdlr_ping\n");
    memset(&resonse, 0, sizeof(atci_response_t));

    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+PING=?
            LOGI("AT TEST OK.\n");
            strcpy((char *)resonse.response_buf, "\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_READ:    // rec: AT+PING?
            LOGI("AT Read done.\n");
            sprintf((char *)resonse.response_buf,"\r\n+PING:<ip>\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+PING
            LOGI("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "\r\nERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_EXECUTION: // rec: AT+PING=<p1>  the handler need to parse the parameters
            LOGI("AT Executing...\r\n");
            LOGI("AT CMD received:%s ", parse_cmd->string_ptr);
            param = strtok((char *)parse_cmd->string_ptr, ",");
            LOGI("AT CMD received IP:%s", param);
            if (param != NULL) {
                char *p;
                p = strtok(NULL, ",");

                if (p != NULL) {
                    count = atoi(p);
                    p = strtok(NULL, ",");
                    if (p != NULL) {
                        pktsz = atoi(p);
                    }
                }
            }
            param = strtok((char *)parse_cmd->string_ptr, "AT+PING=");
            ping_request(count, param, PING_IP_ADDR_V4, pktsz, ping_request_result);

            strcpy((char *)resonse.response_buf, "\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}
#endif


#ifdef DEMO_APP_SUPPORT_IPERF
atci_status_t atci_cmd_hdlr_iperf(atci_parse_cmd_param_t *parse_cmd)
{
	atci_response_t resonse;
	char *param = NULL;

	LOGI("atci_cmd_hdlr_iperf\n");
        memset(&resonse, 0, sizeof(atci_response_t));
	
	resonse.response_flag = 0; // Command Execute Finish.

	switch (parse_cmd->mode) {
		case ATCI_CMD_MODE_TESTING:    // rec: AT+IPERF=?
			LOGI("AT TEST OK.\n");
			strcpy((char *)resonse.response_buf, "\r\n+IPERF:(0,1)\r\nOK\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);
			break;

		case ATCI_CMD_MODE_READ:	// rec: AT+IPERF?
			LOGI("AT Read done.\n");
			sprintf((char *)resonse.response_buf,"\r\n+IPERF:<ip>\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;	// ATCI will help append OK at the end of resonse buffer
			atci_send_response(&resonse);
			break;

		case ATCI_CMD_MODE_ACTIVE:	// rec: AT+IPERF
			LOGI("AT Active OK.\n");
			// assume the active mode is invalid and we will return "ERROR"
			strcpy((char *)resonse.response_buf, "\r\nERROR\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);
			break;

		case ATCI_CMD_MODE_EXECUTION: // rec: AT+IPERF=<p1>  the handler need to parse the parameters
            LOGI("AT Executing...\r\n");
            LOGI("AT CMD received: %s", parse_cmd->string_ptr);

            param = strtok((char *)parse_cmd->string_ptr, "AT+IPERF=");
            if (param != NULL) {
                int len = strlen((char *)(param + 3));
                if (param[0] == '-' && param[1] == 'c') {
                    char *str = (char *)pvPortMalloc(len + 1);
                    str[len] = '\0';
                    memcpy(str, param + 3, len);
                    atci_cmd_iperf_client(str);
                    vPortFree(str);
                } else if (param[0] == '-' && param[1] == 's') {
                    char *str = (char *)pvPortMalloc(len + 1);
                    str[len] = '\0';
                    memcpy(str, param + 3, len);
                    atci_cmd_iperf_server(str);
                    vPortFree(str);
                } else {
                    strcpy((char *)resonse.response_buf, "\r\nERROR\r\n");
                    resonse.response_len = strlen((char *)resonse.response_buf);
                    atci_send_response(&resonse);
                    break;
                }
                strcpy((char *)resonse.response_buf, "\r\nOK\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
                atci_send_response(&resonse);
                break;
            }
            strcpy((char *)resonse.response_buf, "\r\nERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
		default :
			strcpy((char *)resonse.response_buf, "ERROR\r\n");
			resonse.response_len = strlen((char *)resonse.response_buf);
			atci_send_response(&resonse);
			break;
	}
	return ATCI_STATUS_OK;

}

static uint8_t atci_cmd_iperf_server(char *param)
{
    int i;
    char **g_iperf_param = NULL;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    char *p = NULL;
    int is_create_task = 0;
    LOGI("atci_cmd_iperf_server.\n");

    if (param == NULL) {
        return 0;
    }
    g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
    if (g_iperf_param == NULL) {
        return 0;
    }

#ifdef MODEM_ENABLE
    sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);
    iperf_register_callback(atci_cmd_iperf_cb);
#endif

    i = 0;
    p = strtok(param, " ");
    while(p != NULL && i < 13) {
        strcpy((char *)&g_iperf_param[i * offset], p);
        i++;
        p = strtok(NULL, " ");
    }

    for (i = 0; i < 13; i++) {
        if (strncmp((char *)&g_iperf_param[i * offset], "-u", 2) == 0) {
            //printf("Iperf UDP Server: Start!\n");
            //printf("Iperf UDP Server Receive Timeout = 20 (secs)\n");
            xTaskCreate((TaskFunction_t)iperf_udp_run_server, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / (( uint32_t )sizeof( StackType_t )), g_iperf_param, IPERF_TASK_PRIO, NULL);
            is_create_task = 1;
            break;
        }
    }

    if (strncmp((char *)&g_iperf_param[i * offset], "-u", 2) != 0) {
        printf("Iperf TCP Server: Start!");
        printf("Iperf TCP Server Receive Timeout = 20 (secs)");
        xTaskCreate((TaskFunction_t)iperf_tcp_run_server, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
        is_create_task = 1;
    }

    if (is_create_task == 0) {
        vPortFree(g_iperf_param);
    }
    return 0;
}

static uint8_t atci_cmd_iperf_client(char *param)
{
    int i;
    char **g_iperf_param = NULL;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    int is_create_task = 0;
    char *p = NULL;

    if (param == NULL) {
        return 0;
    }

    g_iperf_param = pvPortCalloc(1, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
    if (g_iperf_param == NULL) {
        //printf("Warning: No enough memory to running iperf.\n");
        return 0;
    }

#ifdef MODEM_ENABLE
    sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);
    iperf_register_callback(atci_cmd_iperf_cb);
#endif

    i = 0;
    p = strtok(param, " ");
    while(p != NULL && i < 18) {
        strcpy((char *)&g_iperf_param[i * offset], p);
        i++;
        p = strtok(NULL, " ");
    }

    for (i = 0; i < 18; i++) {
        if (strncmp((char *)&g_iperf_param[i * offset], "-u", 2) == 0) {
            printf("iperf run udp client\n");
            xTaskCreate((TaskFunction_t)iperf_udp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
            is_create_task = 1;
            break;
        }
    }

    if (strncmp((char *)&g_iperf_param[i * offset], "-u", 2) != 0) {
        printf("Iperf TCP Client: Start!");
#if (CFG_CONNSYS_TRX_BALANCE_EN == 1)
        xTaskCreate((TaskFunction_t)iperf_tcp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , &g_balance_ctr.tx_handle);
#else
        xTaskCreate((TaskFunction_t)iperf_tcp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
#endif
        is_create_task = 1;
    }

    if (is_create_task == 0) {
        vPortFree(g_iperf_param);
    }

    return 0;
}


#ifdef MODEM_ENABLE
static void atci_cmd_iperf_cb(iperf_result_t* iperf_result)
{
    atci_response_t resonse;

    vTaskDelay(3000);

    sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_COMMAND);

    // TODO: handle the result
    resonse.response_flag = 0; // Command Execute Finish.
    if (iperf_result)
        sprintf((char *)resonse.response_buf, "\r\n iperf finish, %s, data_size = %d, total = %s, result = %s \r\n",
            iperf_result->report_title, (int)iperf_result->data_size, iperf_result->total_len, iperf_result->result);
    else
	    strcpy((char *)resonse.response_buf, "\r\n iperf finish, no result!\r\n");
    resonse.response_len = strlen((char *)resonse.response_buf);
    atci_send_response(&resonse);
}
#endif
#endif

#ifdef MTK_FOTA_ENABLE
atci_status_t atci_cmd_hdlr_fota_cmd(atci_parse_cmd_param_t *parse_cmd)
{
    static int test_param1 = 0;
    atci_response_t resonse;
    char *param = NULL;
    int  param1_val = -1;

    LOGW("atci_cmd_hdlr_fota_cmd\n");
    memset(&resonse, 0, sizeof(atci_response_t));
    
    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+FOTA=?
            LOGW("AT FOTA OK.\n");
            strcpy((char *)resonse.response_buf, "+FOTA:(0,1)\r\nOK\r\n");
            break;

       case ATCI_CMD_MODE_READ:    // rec: AT+FOTA?
            LOGW("AT Read done.\n");
            sprintf((char *)resonse.response_buf, "+FOTA:%d\r\n", test_param1);
            resonse.response_len = strlen((char *)resonse.response_buf);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+FOTA
            LOGW("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
            
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+FOTA=<p1>  the handler need to parse the parameters
            LOGW("AT Executing...\r\n");
            //parsing the parameter
            param = strtok(parse_cmd->string_ptr, ",\n\r");
            param = strtok(parse_cmd->string_ptr, "AT+FOTA=");
            param1_val = atoi(param);

            if (param != NULL && (param1_val == 0 || param1_val == 1)){
                
                // valid parameter, update the data and return "OK"
                char *url = NULL;
                url = pvPortMalloc(FOTA_URL_BUF_LEN);
                if (url != NULL) {
                    strncpy(url, param, FOTA_URL_BUF_LEN);
                    LOG_I(fota_atci, "[FOTA AT] url = %s\r\n", url);
                    xTaskCreate(fota_download_task, FOTA_DOWNLOAD_TASK_NAME, FOTA_DOWNLOAD_TASK_STACKSIZE / (( uint32_t )sizeof( StackType_t )), url, FOTA_DOWNLOAD_TASK_PRIO, NULL);
                    LOG_I(fota_atci, "[FOTA AT] create fota download task \r\n");

                    resonse.response_len = 0;
                    resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK; // ATCI will help append "OK" at the end of resonse buffer
                } else {
                    LOG_I(fota_atci, "[FOTA AT] alloc buffer failed.\r\n");
                    resonse.response_len = 0;
                    resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
                }
            } else {
                // invalide parameter, return "ERROR"
                resonse.response_len = 0;
                resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
            };
            atci_send_response(&resonse);
            param = NULL;
            break;
            
        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}
#endif

#ifdef TCPIP_FOR_MT5931_ENABLE
#if CFG_SUPPORT_LOOPBACK
atci_status_t atci_cmd_hdlr_loopback_cmd(atci_parse_cmd_param_t *parse_cmd)
{
    static int test_param1 = 0;
    atci_response_t resonse;
    char *param = NULL;

    LOGW("atci_cmd_hdlr_loopback_cmd\n");
    
    memset(&resonse, 0, sizeof(atci_response_t));
    resonse.response_flag = 0; // Command Execute Finish.

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+LB=?
            LOGW("AT Test OK.\n");
            sprintf((char *)resonse.response_buf, "+LB:<loopback_times>,<loopback_mode>,<loopback_length>\r\nOK\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

       case ATCI_CMD_MODE_READ:    // rec: AT+LB?
            LOGW("AT Read done.\n");
            sprintf((char *)resonse.response_buf,"+LB:%d\r\n", test_param1);
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+LB
            LOGW("AT Active OK.\n");
            // assume the active mode is invalid and we will return "ACTIVE"
            sprintf((char *)resonse.response_buf, "ACTIVE\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_EXECUTION: // rec: AT+LB=<p1>,<p2>,<p3>  the handler need to parse the parameters
            LOGW("AT Executing...\r\n");
            param = strtok(parse_cmd->string_ptr, "\n\r");
            param = strtok(parse_cmd->string_ptr, "AT+LB=");
            LOGW("input_buf: %s\n", param);  //1,0,2

            if (param != NULL) {
                LOGW("[AT+LB] test Begin\n");
                wifi_loopback_test(param);
                //ret = wndrv_test_ehpi_loopback_test(param);
            } else {
                // invalide parameter
                sprintf((char *)resonse.response_buf, "FAILED:Inbalide Input\n\r");
                resonse.response_len = strlen((char *)resonse.response_buf);
                //resonse.response_len = 0;
                resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
            };

            atci_send_response(&resonse);
            param = NULL;
            break;

        default :
            sprintf((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}

void wifi_loopback_test(char * inject_string)
{
    xTaskCreate(wifi_loopback_task,
            WIFI_LB_TASK_NAME,
            WIFI_LB_TASK_STACKSIZE / (( uint32_t )sizeof( StackType_t )),
            inject_string,
            WIFI_LB_TASK_PRIO,
            NULL);
}

void wifi_loopback_task(void *arg)
{
    uint8_t ret = 0;
    atci_response_t resonse;
    memset(&resonse, 0, sizeof(atci_response_t));

    ret = wndrv_test_ehpi_loopback_test(arg);
    if (0 == ret) {
        sprintf((char *)resonse.response_buf, "SUCCEED:Loop Back Test PASS\n\r");
        resonse.response_len = strlen((char *)resonse.response_buf);
        //resonse.response_len = 0;
        resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK; // ATCI will help append "OK" at the end of resonse buffer 
    } else {
        sprintf((char *)resonse.response_buf, "FAILED:Loop Back Test FAIL\n\r");
        resonse.response_len = strlen((char *)resonse.response_buf);
        //resonse.response_len = 0;
        resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
    }
    atci_send_response(&resonse);
    vTaskDelete(NULL);
}
#endif
#endif
