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
#include "mt7686.h"

/* hal includes */
#include "hal.h"
#include "bsp_gpio_ept_config.h"
#include "hal_sleep_manager.h"

/* system includes */
#include "sys_init.h"
#include "task_def.h"

/* wifi includes */
#include "connsys_profile.h"
#include "wifi_api.h"
#include "wifi_lwip_helper.h"
#include "wifi_os_api.h"
#include "wifi_default_config.h"

#ifdef MTK_WIFI_STUB_CONF_ENABLE
#include "wfc_stub.h"
#include "wfc_at_api.h"
#endif

/* cli/atci includes */
#ifdef MTK_MINICLI_ENABLE
#include "cli_def.h"
#endif

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
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define MTK_WIFI_MAIN_DEMO_TASK_ENABLE

/*Define this macro indicates The host will send wifi configuration to wifi chip for wifi initialization,  
   Otherwise, wifi chip will do initialization by the default settings in wifi chip's program*/
#define WIFI_CONFIG_BY_HOST 


/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
#ifdef MTK_WIFI_MAIN_DEMO_TASK_ENABLE
static void wifi_main_init(void);
static void wifi_main_task(void *param);
#endif

#ifndef MTK_WIFI_SLIM_ENABLE
void hal_clock_set_pll_dcm_init(void);
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
#ifndef MTK_WIFI_SLIM_ENABLE
    //Enable PLL
    hal_clock_set_pll_dcm_init();
#endif

    /* Do system initialization, eg: hardware, nvdm. */
    system_init();

#if defined(MTK_MINICLI_ENABLE)
    /* Initialize cli task to enable user input cli command from uart port.*/
    cli_def_create();
    cli_task_create();
#endif

#ifdef MTK_ATCI_ENABLE
    /* init ATCI module and set UART port */
    atci_init(HAL_UART_2);
    /* create task for ATCI */
    xTaskCreate(atci_def_task, ATCI_TASK_NAME, ATCI_TASK_STACKSIZE /((uint32_t)sizeof(StackType_t)), NULL, ATCI_TASK_PRIO, NULL);
#endif

#ifdef MTK_WIFI_STUB_CONF_ENABLE
    wfc_stub_init();
#endif

#ifdef MTK_WIFI_MAIN_DEMO_TASK_ENABLE
    wifi_main_init();
#endif

    /* Call this function to indicate the system initialize done. */
    SysInitStatus_Set();

    /* Start the scheduler. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}



#ifdef MTK_WIFI_MAIN_DEMO_TASK_ENABLE
static void wifi_main_init(void)
{
    /* create task for wifi demo */
    xTaskCreate(wifi_main_task, WIFI_MAIN_DEMO_TASK_NAME, WIFI_MAIN_DEMO_TASK_STACKSIZE /((uint32_t)sizeof(StackType_t)), NULL, WIFI_MAIN_DEMO_TASK_PRIO, NULL);
}

/**
  * @brief  wifi_init_done_handler callback function
  * @param  None
  * @retval 1 success,   0 fail
  */
int32_t wifi_init_done_handler(wifi_event_t event,
                                      uint8_t *payload,
                                      uint32_t length)
{
    LOG_I(common, "WiFi Init Done: port = %d", payload[6]);
    return 1;
}


/**
  * @brief  wifi main initial program
  * @param  None
  * @retval None
  */
static void wifi_main_task(void *param)
{
    LOG_I(common, "enter wifi_main_task!!\n\r");

    /* User initial the parameters for wifi initial process,  system will determin which wifi operation mode
     * will be started , and adopt which settings for the specific mode while wifi initial process is running*/
    wifi_config_t     config     = {0}, *pconfig     = NULL;
    wifi_config_ext_t config_ext = {0}, *pconfig_ext = NULL;
    uint8_t config_valid         = FALSE;
    uint8_t config_ext_valid     = FALSE;
    
#ifdef WIFI_CONFIG_BY_HOST
    /*Wait and get the wifi config settings from the host*/
    wfc_get_wifi_host_setting(&config, &config_valid, &config_ext, &config_ext_valid);
    if(config_valid == FALSE){
       /*set default opmode here in order to initial the lwip in the following step.*/
       config.opmode = (uint8_t)atoi(WIFI_DEFAULT_OPMODE);
    }
#else
    /*Wifi chip will do initialization by blow settings in wifi chip's program*/
    config.opmode = WIFI_MODE_STA_ONLY;
    strcpy((char *)config.sta_config.ssid,     (const char *)"MTK_STA");
    strcpy((char *)config.sta_config.password, (const char *)"12345678");
    config.sta_config.ssid_length       = strlen((const char *)config.sta_config.ssid);
    config.sta_config.password_length   = strlen((const char *)config.sta_config.password);
    config_ext.sta_auto_connect_present = 1;
    config_ext.sta_auto_connect         = 1;
    config_valid                        = TRUE;
    config_ext_valid                    = TRUE;
#endif

#if !defined(MTK_WFC_WITH_LWIP_NO_WIFI_ENABLE)
    /*if config, config_ext be set by Host or be set by the device itself with hard code*/
    if(config_valid == TRUE){
        pconfig = &config;
    }
    if(config_ext_valid == TRUE){
        pconfig_ext = &config_ext;
    }
    
    /* Initialize wifi stack and register wifi init complete event handler,
     * notes:  the wifi initial process will be implemented and finished while system task scheduler is running.
     */
    wifi_init(pconfig, pconfig_ext);
    wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE, wifi_init_done_handler);
#endif

    /* Tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization*/
    lwip_network_init(config.opmode);
    lwip_net_start(config.opmode);

    vTaskDelete(NULL);
}
#endif

