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


#ifdef UI_TOUCHGFX
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/canvas_widget_renderer/CanvasWidgetRenderer.hpp>
#include "sensor_demo.h"
#include <gui/database/heart_rate_db.hpp>
#include <gui/common/CommonService.hpp>
#include <gui/setting_screen/SetBluetoothView.hpp>
#include "MainMenuView.hpp"

using namespace touchgfx;
#endif /* UI_TOUCHGFX */


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "FreeRTOSConfig.h"


/* device.h includes */
#include "mt2523.h"



/* hal includes */
#include "hal.h"
#include "hal_audio.h"
#include "nvdm.h"
#include "bt_log.h"
#include "bt_init.h"

#ifndef WATCH_DEMO
#include "bt_app_common.h"
#endif

#include "bsp_gpio_ept_config.h"
#include "memory_attribute.h"
#include "sys_init.h"
#include "utils.h"

#include "syslog.h"
#include "task_def.h"
#include "gnss_app.h"

#include "mt25x3_hdk_lcd.h"
#include "bt_source_srv.h"

#ifdef UI_TOUCHGFX
/* application includes */
#include "battery_message.h"
#include "CommonService.hpp"
#include "Model.hpp"

#ifdef HAL_SLEEP_MANAGER_ENABLED
const char *touchgfx_name = "TouchGFX";
static uint8_t touchgfx_sleep_handle = 0xFF;
volatile bool touchgfx_lock = false;
#endif
#endif /* UI_TOUCHGFX */


#ifdef __cplusplus
extern "C" {
#endif

#include "audio_player.h"

void bt_spps_atci_main(void);
void am_task_create(void);

#ifdef __cplusplus
}
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Create the log control block for freertos module.
 * The initialization of the log is in the sys_init.c.
 * Please refer to the log dev guide under /doc folder for more details.
 */
//log_create_module(freertos, PRINT_LEVEL_INFO);

#define mainCHECK_DELAY ( ( portTickType ) 1000 / portTICK_RATE_MS )


/**
 * Define the FreeRTOS task priorities and stack sizes
 */
#define configGUI_TASK_PRIORITY                 ( tskIDLE_PRIORITY + 4 )

#define configGUI_TASK_STK_SIZE                 ( 1500 )

#define CANVAS_BUFFER_SIZE (5500)


static void GUITask(void* params)
{
#ifdef UI_TOUCHGFX
    touchgfx::HAL::getInstance()->taskEntry();
#endif /* UI_TOUCHGFX */
}

#ifdef UI_TOUCHGFX
void set_power_mode(PowerMode mode, uint32_t data)
{
    extern uint32_t awakeCount;
    static PowerMode previousMode = PowerMode_Normal;
    static uint32_t previousData = 20;

    switch (mode) {
        case PowerMode_Normal:
#ifdef HAL_SLEEP_MANAGER_ENABLED
            if (!touchgfx_lock) {
                hal_sleep_manager_lock_sleep(touchgfx_sleep_handle);
                touchgfx_lock = true;
            }
#endif
            if (data == 0)
                data = 20;
            awakeCount = 0;
            if (previousMode == PowerMode_Off)
                bsp_lcd_display_on();
            if (previousMode == PowerMode_Standby)
                bsp_lcd_exit_idle();
            break;
        case PowerMode_Standby:
            if (data == 0)
                data = 60000;
            if(data >= 60000)
                awakeCount = 1;
            if (previousMode != PowerMode_Standby)
                bsp_lcd_enter_idle();
#ifdef HAL_SLEEP_MANAGER_ENABLED
            if (touchgfx_lock) {
                hal_sleep_manager_unlock_sleep(touchgfx_sleep_handle);
                touchgfx_lock = false;
            }
#endif
            break;
        case PowerMode_Off:
            data = 0;
            awakeCount = 0;
            if (previousMode != PowerMode_Off)
                bsp_lcd_display_off();
#ifdef HAL_SLEEP_MANAGER_ENABLED
            if (touchgfx_lock) {
                hal_sleep_manager_unlock_sleep(touchgfx_sleep_handle);
                touchgfx_lock = false;
            }
#endif
            break;
        default:
            break;
    }

    if (mode != previousMode) {
        LOG_I(tgfx, "set_power_mode mode = %d\n\r", mode);
        previousMode = mode;
    }

    if (data != previousData) {
        LOG_I(tgfx, "set_power_mode data = %ld\n\r", data);

        if (hal_gpt_stop_timer(HAL_GPT_2) != HAL_GPT_STATUS_OK) {
            LOG_E(tgfx, "ERROR : HAL_GPT_2 stop timer failed\n\r");
        }

        if (data > 0) {
            if (hal_gpt_start_timer_ms(HAL_GPT_2, data, HAL_GPT_TIMER_TYPE_REPEAT) != HAL_GPT_STATUS_OK) {
                LOG_E(tgfx, "ERROR : HAL_GPT_2 start timer failed\n\r");
            }
        }

        previousData = data;
    }
}

/* sensor data update */
void update_hr_data(int32_t bpm, int32_t status)
{
    static int hr_value = 0;
    static int hr_decode;

    hr_decode = (status&0xC0000000)>>30;
    SENSOR_DEMO_LOGI("In update_hr_data, value = (%ld)\r\n", bpm);
	//printf("In update_hr_data, value = (%ld)\r\n", hr_value);
    //if(bpm != hr_value) {
    if(hr_decode == 0) {
        hr_value = bpm;
        if( (CommonService::isBacklight())  && (true == MainMenuView::isMainMenuScrn) ) {
            CommonService::setPowerMode(100);
            Model::setPowerSavingStatus(false);
        }
        SENSOR_DEMO_LOGI("report sensor_data val to UI, value = (%ld)\r\n", hr_value);
		//printf("report sensor_data val to UI, value = (%ld)\r\n", hr_value);
		HeartRateCache::getHeartRateCacheInstance()->addHeartRateData(hr_value);
    }

    return;
}

void update_step_count_data(int32_t step_count)
{
    if( (CommonService::isBacklight())  && (true == MainMenuView::isMainMenuScrn) ) {
        CommonService::setPowerMode(100);
        Model::setPowerSavingStatus(false);
    }
    SENSOR_DEMO_LOGI("In update_step_count_data, value = (%ld)\r\n", step_count);
    LOG_I(tgfx, "update_step_count_data step_count: %d", step_count);
    HeartRateCache::getHeartRateCacheInstance()->addGSensorData(step_count);
    return;
}

void update_bp_data(int32_t sbp, int32_t dbp, int32_t status)
{
    SENSOR_DEMO_LOGI("In update_br_data, value = (%ld, %ld, %ld)\r\n", sbp, dbp, status);
    LOG_I(tgfx, "In update_br_data, value = (%ld, %ld, %ld)\r\n", sbp, dbp, status);
    HeartRateCache::getHeartRateCacheInstance()->addHBPData(sbp);
    HeartRateCache::getHeartRateCacheInstance()->addLBPData(dbp);
     // BP caculating done
    CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_DONE);
    return;
}

#include "sensor_alg_interface.h"

void sensor_init()
{
    sensor_manager_init();

    if (CommonService::getHeartRateStatus() == true) {
        enable_hr(); // Heart rate is enabled
    }
	enable_step_counter();
}

/* Create the log control block as user wishes. Here we use 'bmt_demo' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(bmt_demo, PRINT_LEVEL_INFO);

static void get_battery_information(void)
{
    CommonService::setCapacityPreviousPercentage(CommonService::getCapacityCurrentPercentage());
    CommonService::setCapacityCurrentPercentage(battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));
    CommonService::setPrivousChargerStatus(CommonService::getChargerStatus());
    CommonService::setChargerStatus(battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST));
    LOG_I(bmt_demo, "get_battery_information: %d   %d   %d\r\n", CommonService::getChargerStatus(), CommonService::getCapacityPreviousPercentage(), CommonService::getCapacityCurrentPercentage());
}


/**
* @brief       This function is battery management message handler.
* @param[in]   message: The message should be process.
* @return      None.
*/
static void battery_management_message_handler(battery_message_context_t *message)
{
    LOG_I(bmt_demo, "battery_management_message_handler event  = 0x%X", (int) message->event);
	LOG_I(bmt_demo, "battery_management_message_handler event  = 0x%X", (int) message->event);
    switch (message->event) {
        case BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE: {

            get_battery_information();

        }
        break;
        default: {

        }
        break;
    }

    // check status

}


/**
* @brief       Task main function
* @param[in]   pvParameters: Pointer that will be used as the parameter for the task being created.
* @return      None.
*/
void battery_message_task(void *pvParameters)
{
    uint32_t handle;
    battery_message_context_t message;

    battery_message_allocate(&handle);

	LOG_I(bmt_demo, "battery_message_task start\n");

    while (1) {
        if (battery_message_receive(handle, &message)) {

			LOG_I(bmt_demo, "battery_message_task process\n");

            battery_management_message_handler(&message);
        }
    }
}

TaskHandle_t battery_app_create()
{
    TaskHandle_t task_handler;
    BaseType_t ret;
   ret = xTaskCreate((TaskFunction_t) battery_message_task,
        BMT_APP_TASK_NAME,
        BMT_APP_TASK_STACKSIZE/(( uint32_t )sizeof( StackType_t )),
        NULL,
        BMT_APP_TASK_PRIO,
        &task_handler );
    if (ret != pdPASS) {
        assert(0);
    }
    return task_handler;
}


TimerHandle_t xTimerofTest;


void log_cup_resource_callback(TimerHandle_t pxTimer)
{
	char list_buf[512],i;
	memset(list_buf,0,512);
	vTaskList(list_buf);
	printf("\r\nTask");
	for(i = 0; i < configMAX_TASK_NAME_LEN; i++)
	{
		printf(" ");
	}
	printf("State  Priority  Stack   Num\r\n");
	printf("************************************************\r\n");
	printf(list_buf);

	memset(list_buf,0,512);
	vTaskGetRunTimeStats(list_buf);

	printf("\r\nTask");
	for(i = 0; i < configMAX_TASK_NAME_LEN; i++)
	{
		printf(" ");
	}
	printf("Abs Time      Percentage\r\n");
	printf("************************************************\r\n");
	printf(list_buf);
}


void perfomance_task()
{
	//Add Code
    xTimerofTest = xTimerCreate("TimerofTest",       /* Just a text name, not used by the kernel. */
                                   (1 * 1000 / portTICK_PERIOD_MS),    /* The timer period in ticks. */
                                   pdTRUE,        /* The timers will auto-reload themselves when they expire. */
                                   NULL,   /* Assign each timer a unique id equal to its array index. */
                                   log_cup_resource_callback /* Each timer calls the same callback when it expires. */
                                  );

	xTimerStart(xTimerofTest, 0);
}
/*bt merge:*/
int BT_XFile_EncryptionCommand()
{
    return 0;
}

#define VFIFO_SIZE_RX_BT (1024)
#define VFIFO_SIZE_TX_BT (1024)
#define VFIFO_ALERT_LENGTH_BT (20)

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t g_bt_cmd_tx_vfifo[VFIFO_SIZE_TX_BT];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t g_bt_cmd_rx_vfifo[VFIFO_SIZE_RX_BT];


void bt_io_uart_irq_ex(hal_uart_callback_event_t event, void *parameter)
{
    // BaseType_t  x_higher_priority_task_woken = pdFALSE;
    // if (HAL_UART_EVENT_READY_TO_READ == event)
    {
        // xSemaphoreGiveFromISR(g_bt_io_semaphore, &x_higher_priority_task_woken);
        // portYIELD_FROM_ISR( x_higher_priority_task_woken );
    }
}

hal_uart_status_t bt_io_uart_init_ex(hal_uart_port_t port)
{
    hal_uart_status_t ret;
    /* Configure UART PORT */
    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.parity = HAL_UART_PARITY_NONE;


    ret = hal_uart_init(port, &uart_config);

    if (HAL_UART_STATUS_OK == ret) {

        dma_config.send_vfifo_buffer              = g_bt_cmd_tx_vfifo;
        dma_config.send_vfifo_buffer_size         = VFIFO_SIZE_TX_BT;
        dma_config.send_vfifo_threshold_size      = 128;
        dma_config.receive_vfifo_buffer           = g_bt_cmd_rx_vfifo;
        dma_config.receive_vfifo_buffer_size      = VFIFO_SIZE_RX_BT;
        dma_config.receive_vfifo_threshold_size   = 128;
        dma_config.receive_vfifo_alert_size       = VFIFO_ALERT_LENGTH_BT;

        ret = hal_uart_set_dma(port, &dma_config);

        ret = hal_uart_register_callback(port, bt_io_uart_irq_ex, NULL);

    }
    return ret;
}

void vApplicationTickHook(void)
{

}

TaskHandle_t battery_task_create()
{
    TaskHandle_t task_handler;
    BaseType_t ret;
    ret = xTaskCreate((TaskFunction_t) battery_message_task,
        BMT_APP_TASK_NAME,
        BMT_APP_TASK_STACKSIZE/(( uint32_t )sizeof( StackType_t )),
        NULL,
        BMT_APP_TASK_PRIO,
        &task_handler );
    if (ret != pdPASS) {
        assert(0);
    }

    return task_handler;
}


#include "atci.h"

#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"
#endif

#ifdef MTK_USB_DEMO_ENABLED
#include "usb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void atcmd_serial_port_init(void);
#ifdef __cplusplus
}
#endif
static void atci_def_task(void *param)
{
    while (1) {
        atci_processing();
    }
}

#if defined(MTK_PORT_SERVICE_ENABLE)
static void syslog_port_service_init(void)
{
    serial_port_dev_t syslog_port;
    serial_port_setting_uart_t uart_setting;

    if (serial_port_config_read_dev_number((char *)"syslog", &syslog_port) != SERIAL_PORT_STATUS_OK) {
        syslog_port = SERIAL_PORT_DEV_USB_COM2;
        serial_port_config_write_dev_number((char *)"syslog", syslog_port);
        uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
        serial_port_config_write_dev_setting(syslog_port, (serial_port_dev_setting_t *)&uart_setting);
    }
}
#endif

#endif /* UI_TOUCHGFX */

/*bt merge done.*/
int main(void)
{
#ifdef UI_TOUCHGFX
    static uint8_t canvasBuffer[CANVAS_BUFFER_SIZE];
#endif /* UI_TOUCHGFX */

    system_init();
    nvdm_init();
#ifdef __GNUC__
    section_init_func();
#endif

#ifdef UI_TOUCHGFX
#ifdef HAL_SLEEP_MANAGER_ENABLED
    touchgfx_sleep_handle = hal_sleep_manager_set_sleep_handle(touchgfx_name);
    if (!touchgfx_lock) {
        hal_sleep_manager_lock_sleep(touchgfx_sleep_handle);
        touchgfx_lock = true;
    }
#endif

    hw_init();
    LOG_I(tgfx, "hw_init finish\n\r");
    touchgfx_init();
    hal_audio_init();
#if 1
#ifdef MTK_USB_DEMO_ENABLED
    usb_boot_init();
#endif

#if defined(MTK_PORT_SERVICE_ENABLE)
    syslog_port_service_init();
#endif

#if defined(MTK_ATCI_VIA_PORT_SERVICE) && defined(MTK_PORT_SERVICE_ENABLE)
{
    serial_port_dev_t port;
    serial_port_setting_uart_t uart_setting;

    if(serial_port_config_read_dev_number((char *)"atci", &port) != SERIAL_PORT_STATUS_OK)
    {
        port = SERIAL_PORT_DEV_USB_COM1; //SERIAL_PORT_DEV_USB_COM1;//SERIAL_PORT_DEV_UART_1;
        serial_port_config_write_dev_number("atci", port);
        LOG_W(common, "serial_port_config_write_dev_number setting uart1");
        uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
        serial_port_config_write_dev_setting(port, (serial_port_dev_setting_t *)&uart_setting);
    }

    atci_init(port);

}
#else
    atci_init(HAL_UART_1);
#endif


    xTaskCreate(atci_def_task,
                 ATCI_TASK_NAME,
                 ATCI_TASK_STACKSIZE/(( uint32_t )sizeof( StackType_t )),
                 NULL,
                 ATCI_TASK_PRIO,
                 NULL);
#endif
    gnss_demo_app_create();
    battery_task_create();

    //perfomance_task();
    CommonService::readNVDM();
    bt_create_task(CommonService::getBluetoothStatus());
    bt_app_common_init();
    bt_source_srv_init();
    am_task_create();
    audio_player_init();

#ifdef SENSOR_DEMO
    sensor_init();
#endif

    CommonService::createTask();
    CommonService::registerPowerkey();
    CommonService::registerGnss();
    CommonService::registerNotification();
    CommonService::registerBacklight();
    CommonService::registerUSBPlug();
    CommonService::registerA2dpSource();
    SetBluetoothView::init();

    CanvasWidgetRenderer::setupBuffer(canvasBuffer, CANVAS_BUFFER_SIZE);

#endif /* UI_TOUCHGFX */
    
    xTaskCreate(GUITask, (const char*)"GUITask",
                configGUI_TASK_STK_SIZE,
                NULL,
                configGUI_TASK_PRIORITY,
                NULL);

    /* Start the scheduler. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for( ;; );
}

