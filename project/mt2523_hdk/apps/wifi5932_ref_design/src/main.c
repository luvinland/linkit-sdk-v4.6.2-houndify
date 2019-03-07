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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt2523.h"

/* hal includes */
#include "hal.h"

#include "sys_init.h"
#include "task_def.h"

#ifdef MTK_ATCI_ENABLE
#include "atci.h"

/**
 * @brief This function is a task main function for processing the data handled by ATCI module.
 * @param[in] param is the task main function paramter.
 * @return    None
 */
static void atci_def_task(void *param)
{

    LOG_I(common, "enter atci_def_task!!\n\r");
    while (1) {
        atci_processing();
    }
}

#endif

/*wifi host includes*/
#ifdef MTK_WIFI_CHIP_USE_MT5932
#include "wifi_host_api.h"
#ifdef MTK_ATCI_ENABLE
#include "at_command_wifi.h"
#endif

void wfcm_stub_init(void);
static void wifi_host_main_init(void);
static void wifi_host_main_task(void *param);
//void wifi_host_config_init(wifi_host_system_config_t *wifi_host_cfg);
#endif


#include "lwip/tcpip.h"
#include "lwip_network.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/*Define this macro indicates The host will send wifi configuration to wifi chip for wifi initialization,  
   Otherwise, wifi chip will do initialization by the default settings in wifi chip's program*/
#define WIFI_CONFIG_BY_HOST 


/* Private variables ---------------------------------------------------------*/
//uint8_t _cli_iperf_remote_client(void); 
//uint8_t _cli_iperf_remote_server(void);


/* Private functions ---------------------------------------------------------*/

/* Create the log control block for freertos module.
 * The initialization of the log is in the sys_init.c.
 * Please refer to the log dev guide under /doc folder for more details.
 */
#ifndef MTK_DEBUG_LEVEL_NONE
log_create_module(main, PRINT_LEVEL_INFO);
#ifdef MTK_WIFI_STUB_CONF_ENABLE
log_create_module(wfcm, PRINT_LEVEL_INFO);
#endif

LOG_CONTROL_BLOCK_DECLARE(common);
LOG_CONTROL_BLOCK_DECLARE(hal);
LOG_CONTROL_BLOCK_DECLARE(main);
LOG_CONTROL_BLOCK_DECLARE(lwip);
#ifdef MTK_WIFI_STUB_CONF_ENABLE
LOG_CONTROL_BLOCK_DECLARE(wfcm);
#endif

log_control_block_t *syslog_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(common),
    &LOG_CONTROL_BLOCK_SYMBOL(hal),
    &LOG_CONTROL_BLOCK_SYMBOL(main),
    &LOG_CONTROL_BLOCK_SYMBOL(lwip),    
#ifdef MTK_WIFI_STUB_CONF_ENABLE
    &LOG_CONTROL_BLOCK_SYMBOL(wfcm),
#endif
    NULL
};
#endif

int main(void)
{ 
    /* Do system initialization, eg: hardware, nvdm. */
    system_init();

    /* Enable I,F bits */
    __enable_irq();
    __enable_fault_irq();

    /* system log initialization.
     * This is the simplest way to initialize system log, that just inputs three NULLs
     * as input arguments. User can use advanced feature of system log along with NVDM.
     * For more details, please refer to the log dev guide under /doc folder or projects
     * under project/mtxxxx_hdk/apps/.
     */
    log_init(NULL, NULL, syslog_control_blocks);

#ifdef MTK_ATCI_ENABLE
    /* init ATCI module and set UART port */
    atci_init(HAL_UART_2);
    /* create task for ATCI */
    //xTaskCreate(atci_def_task, ATCI_TASK_NAME, ATCI_TASK_STACKSIZE /((uint32_t)sizeof(StackType_t)), NULL, ATCI_TASK_PRIO, NULL);
    xTaskCreate(atci_def_task, ATCI_TASK_NAME, ATCI_TASK_STACKSIZE /((uint32_t)sizeof(StackType_t)), NULL, 6, NULL);
#endif

#ifdef MTK_WIFI_CHIP_USE_MT5932
#ifdef MTK_WIFI_STUB_CONF_ENABLE
    wfcm_stub_init();
#endif

    wifi_host_main_init();

#ifdef MTK_ATCI_ENABLE
    /* wifi atci initial in host*/
    wifi_atci_example_init();
#endif
#endif

    //_cli_iperf_remote_client();
    //_cli_iperf_remote_server();
    //spi_master_send_data_over_xboot();
 

    /* Start the scheduler. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

#ifdef MTK_WIFI_CHIP_USE_MT5932
static void wifi_host_main_init(void)
{
    /* create task for WiFi Host Demo*/
    xTaskCreate(wifi_host_main_task, 
                WIFI_HOST_TASK_NAME, 
                WIFI_HOST_TASK_STACKSIZE /((uint32_t)sizeof(StackType_t)),
                NULL, WIFI_HOST_TASK_PRIO, NULL);
}

static void wifi_host_main_task(void *param)
{
    LOG_I(common, "enter wifi_host_main_task!!\n\r");

#ifdef WIFI_CONFIG_BY_HOST
    /*The host will send wifi configuration to wifi chip for wifi initialization*/
    wifi_host_system_config_t wifi_host_sys_cfg;   
    uint8_t sta_mac[WIFI_MAC_ADDRESS_LENGTH] = {0x00,0x0c,0x59,0x32,0x01,0x2};
    uint8_t ap_mac[WIFI_MAC_ADDRESS_LENGTH]  = {0x00,0x0c,0x59,0x32,0x01,0x3};

    memset(&wifi_host_sys_cfg, 0 , sizeof(wifi_host_system_config_t));

    wifi_host_sys_cfg.wifi_config.opmode = WIFI_MODE_STA_ONLY;
    strcpy((char *)wifi_host_sys_cfg.wifi_config.sta_config.ssid,     (const char *)"MTK_STA");
    strcpy((char *)wifi_host_sys_cfg.wifi_config.sta_config.password, (const char *)"12345678");
    wifi_host_sys_cfg.wifi_config.sta_config.ssid_length     =  strlen((const char *)wifi_host_sys_cfg.wifi_config.sta_config.ssid);
    wifi_host_sys_cfg.wifi_config.sta_config.password_length =  strlen((const char *)wifi_host_sys_cfg.wifi_config.sta_config.password);

    memcpy(&wifi_host_sys_cfg.wifi_host_adv_config.sta_mac, sta_mac, WIFI_MAC_ADDRESS_LENGTH);
    memcpy(&wifi_host_sys_cfg.wifi_host_adv_config.ap_mac,  ap_mac,  WIFI_MAC_ADDRESS_LENGTH);

    /*Wait wifi chip's WFC module init done, and set wifi initial setting to wifi chip from Host*/
    wifi_host_config_init(&wifi_host_sys_cfg);
#endif

    /*Do network initial*/
    tcpip_init(NULL, NULL);
    lwip_network_init(WIFI_MODE_STA_ONLY);
    lwip_net_start(WIFI_MODE_STA_ONLY);

    vTaskDelete(NULL);
}
#endif
