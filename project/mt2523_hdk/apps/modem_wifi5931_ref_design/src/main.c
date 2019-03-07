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
#include "queue.h"

/* device.h includes */
#include "mt2523.h"

/* hal includes */
#include "hal_cache.h"
#include "hal_mpu.h"
#include "hal_uart.h"
#include "hal_audio.h"
#include "hal_clock.h"
#include "hal_clock_internal.h"
#include "hal_dvfs.h"
#include "hal_rtc.h"
#include "hal_nvic.h"
#include "hal_dcxo.h"

#include "hal_sys_topsm.h"
#include "hal_dsp_topsm.h"

#include "hal_pdma_internal.h"
#include "bsp_gpio_ept_config.h"
#include "hal_flash.h"
#include "hal_mipi_tx_config_internal.h"
#include "hal_display_lcd.h"
#ifdef MTK_USB_DEMO_ENABLED
#include "usb.h"
#endif
#ifdef TCPIP_FOR_MT5931_ENABLE
#include "mtk_wifi_adapter.h"
#include "lwip/tcpip.h"
#include "lwip_network.h"
#endif
#ifdef MODEM_ENABLE
#include "gnss_app.h"
#endif

#include "memory_map.h"
#include "task_def.h"
#include "common.h"
#ifdef __MTK_BT_DONGLE_TEST__
#include "bt_driver_uart.h"
#endif

#ifdef MODEM_ENABLE
#include "sio_gprot.h"
#include "msm.h"
#include "gprs_api.h"
#include "urc_app.h"
#endif

char *btlib_verno(void);
char *btlib_lastest_commit(void);
extern void atci_example_init();
#ifdef MODEM_ENABLE
extern void gnss_demo_main();
extern void ps_netif_init(void);
#endif

#ifdef MTK_SYSTEM_CLOCK_26M
static const uint32_t target_freq = 26000;
#else
#ifdef MTK_SYSTEM_CLOCK_104M
static const uint32_t target_freq = 104000;
#else
static const uint32_t target_freq = 208000;
#endif
#endif

/* Private functions ---------------------------------------------------------*/
static void SystemClock_Config(void);
static void prvSetupHardware( void );
uint8_t *sdps_spp_record[2];
void vApplicationTickHook(void)
{

}

#ifdef __GNUC__
int __io_putchar(int ch)
#else
    int fputc(int ch, FILE *f)
#endif
{
#ifdef MTK_PORT_SERVICE_ENABLE
    log_write((char*)&ch, 1);
    return ch;
#else
    /* Place your implementation of fputc here */
    /* e.g. write a character to the HAL_UART_0 and Loop until the end of transmission */

#ifdef MODEM_ON_HDK_ENABLE
    hal_uart_put_char(HAL_UART_1, ch);
    if (ch == '\n') {
        hal_uart_put_char(HAL_UART_1, '\r');
    }
#else
    hal_uart_put_char( HAL_UART_0, ch);
    if (ch == '\n') {
        hal_uart_put_char(HAL_UART_0, '\r');
    }
#endif
    return ch;
#endif
}

#define ATCI_DEMO
#ifdef __GNUC__
//#define CAMERA_DEMO
#endif

//#define BT_RELAY_MODE
#ifdef __GNUC__
//#define BT_SINK_DEMO
#endif
//#define FOTA_DEMO
#define NVDM_DEMO
//#define BT_HB_DEMO

//#define AUDIO_DEMO

#ifdef BT_HB_DEMO
#include "bt_init.h"
#endif

#ifdef GNSS_TEST
#include "gnss_test_cmd.h"
#endif

#ifdef NVDM_DEMO
#include "nvdm.h"
#endif

#ifdef ATCI_DEMO
#include "atci.h"


void atci_def_task(void *param)
{
    while (1) {
        atci_processing();
    }
}

#endif
#ifdef BLE_THROUGHPUT
extern void ble_gatt_send_data();
QueueHandle_t ble_tp_queue = NULL;
void ble_tp_task(void *param)
{
    LOG_W(common, "enter ble_tp_task");
    while (1) {
        ble_gatt_send_data();
    }
}
#endif

#ifdef CAMERA_DEMO
#include "cal_task_msg_if.h"
#include "hal_display_lcd.h"
#include "hal_display_lcd_internal.h"
#include "bsp_lcd.h"
#include "mt25x3_hdk_lcd.h"
#endif

static void cache_init(void)
{
    hal_cache_region_t region, region_number;

    /* Max region number is 16 */
    hal_cache_region_config_t region_cfg_tbl[] = {
        /* cacheable address, cacheable size(both MUST be 4k bytes aligned) */
#ifdef CM4_UBIN_LENGTH
        {BL_BASE, BL_LENGTH + CM4_LENGTH + CM4_UBIN_LENGTH},
#else
        {BL_BASE, BL_LENGTH + CM4_LENGTH},
#endif
        /* virtual memory */
        {VRAM_BASE, VRAM_LENGTH}
    };

    region_number = (hal_cache_region_t) (sizeof(region_cfg_tbl) / sizeof(region_cfg_tbl[0]));

    hal_cache_init();
    hal_cache_set_size(HAL_CACHE_SIZE_32KB);
    for (region = HAL_CACHE_REGION_0; region < region_number; region++) {
        hal_cache_region_config(region, &region_cfg_tbl[region]);
        hal_cache_region_enable(region);
    }
    for ( ; region < HAL_CACHE_REGION_MAX; region++) {
        hal_cache_region_disable(region);
    }
    hal_cache_enable();
}

static uint32_t caculate_mpu_region_size( uint32_t region_size )
{
    uint32_t count;

    if (region_size < 32) {
        return 0;
    }
    for (count = 0; ((region_size  & 0x80000000) == 0); count++, region_size  <<= 1);
    return 30 - count;
}

static void mpu_init(void)
{
    hal_mpu_region_t region, region_number;
    hal_mpu_region_config_t region_config;
    typedef struct {
        uint32_t mpu_region_base_address;/**< MPU region start address */
        uint32_t mpu_region_end_address;/**< MPU region end address */
        hal_mpu_access_permission_t mpu_region_access_permission; /**< MPU region access permission */
        uint8_t mpu_subregion_mask;/**< MPU sub region mask*/
        bool mpu_xn;/**< XN attribute of MPU, if set TRUE, execution of an instruction fetched from the corresponding region is not permitted */
    } mpu_region_information_t;

#if defined (__GNUC__) || defined (__CC_ARM)

    //RAM: VECTOR TABLE+RAM CODE+RO DATA
    extern uint32_t Image$$RAM_TEXT$$Base;
    extern uint32_t Image$$RAM_TEXT$$Limit;
    //TCM: TCM CODE+RO DATA
    extern uint32_t Image$$TCM$$RO$$Base;
    extern uint32_t Image$$TCM$$RO$$Limit;
    //STACK END
    extern unsigned int Image$$STACK$$ZI$$Base[];

    /* MAX region number is 8 */
    mpu_region_information_t region_information[] = {
        /* mpu_region_start_address, mpu_region_end_address, mpu_region_access_permission, mpu_subregion_mask, mpu_xn */
        {(uint32_t) &Image$$RAM_TEXT$$Base, (uint32_t) &Image$$RAM_TEXT$$Limit, HAL_MPU_READONLY, 0x0, FALSE}, //Vector table+RAM code+RAM rodata
        {(uint32_t) &Image$$RAM_TEXT$$Base + VRAM_BASE, (uint32_t) &Image$$RAM_TEXT$$Limit + VRAM_BASE, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //Virtual memory
        {(uint32_t) &Image$$TCM$$RO$$Base, (uint32_t) &Image$$TCM$$RO$$Limit, HAL_MPU_READONLY, 0x0, FALSE},//TCM code+TCM rodata
        {(uint32_t) &Image$$STACK$$ZI$$Base, (uint32_t) &Image$$STACK$$ZI$$Base + 32, HAL_MPU_READONLY, 0x0, TRUE} //Stack end check for stack overflow
    };

#elif defined (__ICCARM__)

#pragma section = ".intvec"
#pragma section = ".ram_rodata"
#pragma section = ".tcm_code"
#pragma section = ".tcm_rwdata"
#pragma section = "CSTACK"

    /* MAX region number is 8, please DO NOT modify memory attribute of this structure! */
    _Pragma("location=\".ram_rodata\"") static mpu_region_information_t region_information[] = {
        /* mpu_region_start_address, mpu_region_end_address, mpu_region_access_permission, mpu_subregion_mask, mpu_xn */
        {(uint32_t)__section_begin(".intvec"), (uint32_t)__section_end(".ram_rodata"), HAL_MPU_READONLY, 0x0, FALSE},//Vector table+RAM code+RAM rodata
        {(uint32_t)__section_begin(".intvec") + VRAM_BASE, (uint32_t)__section_end(".ram_rodata") + VRAM_BASE, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //Virtual memory
        {(uint32_t)__section_begin(".tcm_code"), (uint32_t)__section_begin(".tcm_rwdata"), HAL_MPU_READONLY, 0x0, FALSE},//TCM code+TCM rodata
        {(uint32_t)__section_begin("CSTACK"), (uint32_t)__section_begin("CSTACK") + 32, HAL_MPU_READONLY, 0x0, TRUE} //Stack end check for stack overflow
    };

#endif

    hal_mpu_config_t mpu_config = {
        /* PRIVDEFENA, HFNMIENA */
        TRUE, TRUE
    };

    region_number = (hal_mpu_region_t)(sizeof(region_information) / sizeof(region_information[0]));

    hal_mpu_init(&mpu_config);
    for (region = HAL_MPU_REGION_0; region < region_number; region++) {
        /* Updata region information to be configured */
        region_config.mpu_region_address = region_information[region].mpu_region_base_address;
        region_config.mpu_region_size = (hal_mpu_region_size_t) caculate_mpu_region_size(region_information[region].mpu_region_end_address - region_information[region].mpu_region_base_address);
        region_config.mpu_region_access_permission = region_information[region].mpu_region_access_permission;
        region_config.mpu_subregion_mask = region_information[region].mpu_subregion_mask;
        region_config.mpu_xn = region_information[region].mpu_xn;

        hal_mpu_region_configure(region, &region_config);
        hal_mpu_region_enable(region);
    }
    /* make sure unused regions are disabled */
    for (; region < HAL_MPU_REGION_MAX; region++) {
        hal_mpu_region_disable(region);
    }
    hal_mpu_enable();
}

static void mipi_power_init(void)
{
    MIPITX_CONFIG_REGISTER_T *mipi_tx_config_register_ptr = (MIPITX_CONFIG_REGISTER_T *)(MIPI_TX_CFG_BASE);

    /* MIPI analog module power off */
    mipi_tx_config_register_ptr->mipitx_con6_register.field.RG_LNT_BGR_EN = 0;
}
#ifdef __GNUC__
void __attribute__((weak)) tool_init()
{

}
#endif
#ifdef BT_SINK_DEMO
#include "bt_sink_app_main.h"
#include "bt_sink_srv_am_task.h"
#include "bt_hfp_codec_internal.h"
#include "audio_middleware_api.h"
#endif /* BT_SINK_DEMO */


#ifdef FOTA_DEMO
#include "fota_gprot.h"
#endif

#include "hal_sleep_manager.h"

#ifdef MTK_SMART_BATTERY_ENABLE
#include "hal_charger.h"
#include "hal_gpt.h"
#include "cust_battery_meter.h"
/* battery management includes */
#include "battery_management.h"
#endif


#if !defined (MTK_DEBUG_LEVEL_NONE)
LOG_CONTROL_BLOCK_DECLARE(atci);
LOG_CONTROL_BLOCK_DECLARE(atci_charger);
#ifdef MTK_CTP_ENABLE
LOG_CONTROL_BLOCK_DECLARE(atci_ctp);
#endif
LOG_CONTROL_BLOCK_DECLARE(atci_keypad);
LOG_CONTROL_BLOCK_DECLARE(atci_reg);
LOG_CONTROL_BLOCK_DECLARE(atcmd);
#ifdef AUDIO_DEMO
LOG_CONTROL_BLOCK_DECLARE(audio_middleware_api);
LOG_CONTROL_BLOCK_DECLARE(audio_nvdm);
#endif
#ifdef MTK_SMART_BATTERY_ENABLE
#ifndef MTK_BUILD_SMT_LOAD
LOG_CONTROL_BLOCK_DECLARE(bmt);
#endif
#endif
LOG_CONTROL_BLOCK_DECLARE(common);
LOG_CONTROL_BLOCK_DECLARE(hal);
LOG_CONTROL_BLOCK_DECLARE(RTC_ATCI);
//LOG_CONTROL_BLOCK_DECLARE(sensor);
#ifdef TOOL_LOG_MODULE
LOG_CONTROL_BLOCK_DECLARE(TOOL_LOG_MODULE);
#endif
#ifdef __GNUC__
#if defined(MTK_PORT_SERVICE_ENABLE)
LOG_CONTROL_BLOCK_DECLARE(SPP_PORT);
#endif
#endif
log_control_block_t *syslog_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(atci_charger),
#ifdef MTK_CTP_ENABLE		
    &LOG_CONTROL_BLOCK_SYMBOL(atci_ctp),
#endif    
    &LOG_CONTROL_BLOCK_SYMBOL(atci_keypad),
    &LOG_CONTROL_BLOCK_SYMBOL(atci_reg),
    &LOG_CONTROL_BLOCK_SYMBOL(atcmd),
#ifdef AUDIO_DEMO
    &LOG_CONTROL_BLOCK_SYMBOL(audio_middleware_api),
    &LOG_CONTROL_BLOCK_SYMBOL(audio_nvdm),
#endif
#ifdef MTK_SMART_BATTERY_ENABLE
#ifndef MTK_BUILD_SMT_LOAD
    &LOG_CONTROL_BLOCK_SYMBOL(bmt),
#endif
#endif
    &LOG_CONTROL_BLOCK_SYMBOL(common),
    &LOG_CONTROL_BLOCK_SYMBOL(hal),
    &LOG_CONTROL_BLOCK_SYMBOL(RTC_ATCI),
    //&LOG_CONTROL_BLOCK_SYMBOL(sensor),
#ifdef TOOL_LOG_MODULE
    &LOG_CONTROL_BLOCK_SYMBOL(TOOL_LOG_MODULE),
#endif
#ifdef __GNUC__
#if defined(MTK_PORT_SERVICE_ENABLE)
    &LOG_CONTROL_BLOCK_SYMBOL(SPP_PORT),
#endif
#endif
    NULL
};

static void syslog_config_save(const syslog_config_t *config)
{
    nvdm_status_t status;
    char *syslog_filter_buf;

    syslog_filter_buf = (char*)pvPortMalloc(SYSLOG_FILTER_LEN);
    configASSERT(syslog_filter_buf != NULL);
    syslog_convert_filter_val2str((const log_control_block_t **)config->filters, syslog_filter_buf);
    status = nvdm_write_data_item("common",
                                  "syslog_filters",
                                  NVDM_DATA_ITEM_TYPE_STRING,
                                  (const uint8_t *)syslog_filter_buf,
                                  strlen(syslog_filter_buf));
    vPortFree(syslog_filter_buf);
    LOG_I(common, "syslog config save, status=%d", status);
}

static uint32_t syslog_config_load(syslog_config_t *config)
{
    uint32_t sz = SYSLOG_FILTER_LEN;
    char *syslog_filter_buf;

    syslog_filter_buf = (char*)pvPortMalloc(SYSLOG_FILTER_LEN);
    configASSERT(syslog_filter_buf != NULL);
    if (nvdm_read_data_item("common", "syslog_filters", (uint8_t*)syslog_filter_buf, &sz) == NVDM_STATUS_OK) {
        syslog_convert_filter_str2val(config->filters, syslog_filter_buf);
    } else {
        /* popuplate the syslog nvdm with the image setting */
        syslog_config_save(config);
    }
    vPortFree(syslog_filter_buf);
    return 0;
}

#endif /* MTK_DEBUG_LEVEL_NONE */

#ifdef CAMERA_DEMO
int lcdwidth=0, lcdheight=0;

void Display_LCD_Parameter()
{
    hal_display_lcd_roi_output_t lcd_para;
    bsp_lcd_init(0xF800);

    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_WIDTH, &lcdwidth);
    bsp_lcd_get_parameter(LCM_IOCTRL_QUERY__LCM_HEIGHT, &lcdheight);

    lcd_para.target_start_x = 0;
    lcd_para.target_start_y = 0;
    lcd_para.target_end_x = lcdwidth-1;
    lcd_para.target_end_y = lcdheight-1;
    lcd_para.roi_offset_x = 0;
    lcd_para.roi_offset_y = 0;
    lcd_para.main_lcd_output = LCM_16BIT_16_BPP_RGB565_1;

    bsp_lcd_config_roi(&lcd_para);

}
#endif

#ifdef MTK_SMART_BATTERY_ENABLE
#ifndef MTK_BUILD_SMT_LOAD
static void check_battery_voltage(void)
{
    /* Check power on battery voltage and pre-charging battery sample code */
    hal_charger_init();
    while (1) {
        int32_t battery_voltage = 0;
        bool charger_status;

        hal_charger_meter_get_battery_voltage_sense(&battery_voltage);
        LOG_I(common, "Check battery_voltage = %d mV", battery_voltage);
        /* Check battery voltage  > SHUTDOWN_SYSTEM_VOLTAGE (3400mV)  + 100mV */
        if (battery_voltage >= SHUTDOWN_SYSTEM_VOLTAGE + 100) {
            break;
        } else {
            hal_charger_get_charger_detect_status(&charger_status);
            if (charger_status == true) {
                LOG_I(common, "SW charging battery_voltage = %d mV", battery_voltage);
                hal_charger_init();
                /* Setup pre-charging current. It depends on the battery specifications */
                hal_charger_set_charging_current(HAL_CHARGE_CURRENT_70_MA);
                hal_charger_enable(true);
                /* Reset watchdog timer */
                hal_charger_reset_watchdog_timer();
                hal_gpt_delay_ms(1 * 1000);
            } else {
                LOG_I(common, "Low battery power off !! battery_voltage = %d mV", battery_voltage);
                hal_sleep_manager_enter_power_off_mode();
            }

        }
    }

}
#endif
#endif

#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"

static void syslog_port_service_init(void)
{
    serial_port_dev_t syslog_port;
    serial_port_setting_uart_t uart_setting;

    if (serial_port_config_read_dev_number("syslog", &syslog_port) != SERIAL_PORT_STATUS_OK) {
    #ifdef MODEM_ON_HDK_ENABLE
        syslog_port = SERIAL_PORT_DEV_UART_2;
    #else
    #ifdef __MODEM_LOWER_POWER__
        syslog_port = SERIAL_PORT_DEV_UART_0;
    #else
        syslog_port = SERIAL_PORT_DEV_USB_COM2;
    #endif
    #endif
        serial_port_config_write_dev_number("syslog", syslog_port);
        uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
		serial_port_config_write_dev_setting(syslog_port, (serial_port_dev_setting_t *)&uart_setting);
    }
}
#endif

#ifdef __MODEM_LOWER_POWER__

#include "timers.h"
void my_callback(TimerHandle_t pxTimer)
{
    LOG_I(common, "==============timer callback \r\n");
}

#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{
    /* SystemClock Config */
    SystemClock_Config();

    SystemCoreClockUpdate();

    /* Configure the hardware ready to run the test. */
    prvSetupHardware();

    nvdm_init();

    /* init UART earlier */
#ifdef MODEM_ON_HDK_ENABLE
#ifdef MTK_PORT_SERVICE_ENABLE
    log_uart_init(HAL_UART_2);
#else
    log_uart_init(HAL_UART_1);
    log_init(NULL, NULL, NULL);
#endif
#else
    log_uart_init(HAL_UART_0);
#endif
    
#ifdef MTK_PORT_SERVICE_ENABLE
    syslog_port_service_init();
#endif
    
    log_init(syslog_config_save, syslog_config_load, syslog_control_blocks);

    LOG_I(common, "enter main!!\n\r");

    hal_sleep_manager_init();

    clock_dump_info();
 
#ifndef __MODEM_LOWER_POWER__
    tcpip_init(NULL, NULL);
#endif

#ifdef MODEM_ENABLE
{
    uint32_t pin_config = 0;
#ifdef MODEM_ON_HDK_ENABLE
    sio_init(HAL_UART_0);
    pin_config = MSM_NOTITF_MODEM_WAKEUP_PIN_SUPPORTED | 
        MSM_NOTITF_MODEM_EXCEPTION_PIN_SUPPORTED | 
        MSM_TRIGGER_MODEM_RESET_PIN_SUPPORTED |
        MSM_UPDATE_HOST_STATUS_PIN_SUPPORTED | 
        MSM_QUERY_MODEM_STATUS_PIN_SUPPORTED | 
        MSM_TRIGGER_MODEM_WAKEUP_PIN_SUPPORTED;
    msm_init(pin_config, 0);
#else
    sio_init(HAL_UART_2);
// TODO:
    /*pin_config = MSM_NOTITF_MODEM_WAKEUP_PIN_SUPPORTED | 
        MSM_UPDATE_HOST_STATUS_PIN_SUPPORTED | 
        MSM_TRIGGER_MODEM_WAKEUP_PIN_SUPPORTED;*/
    pin_config = 0;
    msm_init(pin_config, MSM_EXCEPTION_PIN_EDGE_TRIGGER);
#endif
    urc_app_init();
    gprs_cntx_init();
#ifdef MTK_TCPIP_FOR_EXTERNAL_MODULE_ENABLE
    ps_netif_init();
#endif
#ifndef MODEM_ON_HDK_ENABLE
    // TODO:
    LOG_I(common, "LED!!\n\r");
    poweron_led();
#endif
}
#endif

#ifdef MTK_SMART_BATTERY_ENABLE
#ifndef MTK_BUILD_SMT_LOAD
    /* Check battery voltage */
    check_battery_voltage();
//#ifndef __MODEM_LOWER_POWER__
    battery_management_init();
//#endif
#endif
#endif

#ifdef AUDIO_DEMO
    hal_audio_init();
#endif


#ifdef MTK_USB_DEMO_ENABLED
    usb_boot_init();
#endif
#ifdef __GNUC__
#if defined (MTK_PORT_SERVICE_ENABLE)
    /* Reduce bt-spp log to make syslog over bt-spp work. */
    //log_config_print_level(bt_spp, PRINT_LEVEL_WARNING);
    log_config_print_level(SPP_PORT, PRINT_LEVEL_INFO);          // TODO: ?????
#endif
#endif

    /* init ATCI module and set UART port */
#if defined(MTK_ATCI_VIA_PORT_SERVICE) && defined(MTK_PORT_SERVICE_ENABLE)
{
	serial_port_dev_t port;
    serial_port_setting_uart_t uart_setting;

	if (serial_port_config_read_dev_number("atci", &port) != SERIAL_PORT_STATUS_OK)
	{
    /*#ifdef __MODEM_LOWER_POWER__
        port = SERIAL_PORT_DEV_UART_0;
    #else*/
        port = SERIAL_PORT_DEV_USB_COM1;
    //#endif
		serial_port_config_write_dev_number("atci", port);
		LOG_W(common, "serial_port_config_write_dev_number setting uart1");
		uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
		serial_port_config_write_dev_setting(port, (serial_port_dev_setting_t *)&uart_setting);
	}
    atci_init(port);

}
#else
#ifdef MODEM_ON_HDK_ENABLE
    atci_init(HAL_UART_2);
#else
    atci_init(HAL_UART_1);
#endif
#endif

    /* create task for ATCI */
    atci_example_init();
    xTaskCreate( atci_def_task, ATCI_TASK_NAME, ATCI_TASK_STACKSIZE /((uint32_t)sizeof(StackType_t)), NULL, ATCI_TASK_PRIO, NULL);

#ifdef BT_RELAY_MODE
    bt_enable_relay();
#else
#ifdef BT_SINK_DEMO
#ifndef MTK_AUDIO_TUNING_ENABLED
    audio_middleware_init();
#endif /* MTK_AUDIO_TUNING_ENABLED */
    bt_codec_task_create();
    am_task_create();
    bt_sink_app_task_create();
#endif /* BT_SINK_DEMO */
#endif

#ifdef CAMERA_DEMO
    Display_LCD_Parameter();
    LOG_I(common, "camera CAL init\n\r");
    CalInit();
    LOG_I(common, "mdp init\n\r");
    idp_init();

    xTaskCreate( CalTaskMain, CAMERA_TASK_NAME, CAMERA_TASK_STACKSIZE/sizeof(StackType_t), NULL, CAMERA_TASK_PRIORITY, NULL);
#else
#ifdef LCD_ENABLE
    bsp_lcd_init(0);
#endif
#endif

#ifdef FOTA_DEMO
    LOG_I(common, "[FOTA] create fota task. \r\n");
    xTaskCreate(fota_task, FOTA_TASK_NAME, FOTA_TASK_STACKSIZE/((uint32_t)sizeof(StackType_t)), NULL, TASK_PRIORITY_NORMAL, NULL);
#endif

#ifdef BLE_THROUGHPUT
    ble_tp_queue = xQueueCreate(100, 8);
    xTaskCreate(ble_tp_task, BLE_TP_TASK_NAME, BLE_TP_TASK_STACKSIZE/sizeof(StackType_t), NULL, BLE_TP_TASK_PRIORITY, NULL);
#endif
#ifdef __GNUC__
#ifdef BT_HB_DEMO
    bt_create_task();
#endif
#endif


#if defined(MTK_WIFI_CHIP_USE_MT5931)
    LOG_I(common, "wifi init start!!\n\r");
    wifi_config_set_wifi_start();
    LOG_I(common, "wifi init 1\n\r");
    lwip_network_init(WIFI_MODE_STA_ONLY);
    LOG_I(common, "wifi init 2\n\r");
    lwip_net_start(WIFI_MODE_STA_ONLY);
    LOG_I(common, "wifi init end!!\n\r");
#endif //defined(MTK_WIFI_CHIP_USE_MT5931)

#ifdef MODEM_ENABLE
    LOG_I(common, "~~~~~~~~modem demo app init start!!~~~~~~~~");
    nw_demo_main();
//#ifndef __MODEM_LOWER_POWER__
    gnss_demo_main();
//#endif
    LOG_I(common, "~~~~~~~~modem demo app init end!!~~~~~~~~");
#endif /* MODEM_ENABLE */
 
#ifdef __MODEM_LOWER_POWER__
    TimerHandle_t xTimerofTest = NULL;
    xTimerofTest = xTimerCreate("TimerofTest",       /* Just a text name, not used by the kernel. */
                               (5 *1000 / portTICK_PERIOD_MS),    /* The timer period in ticks. */
                               pdTRUE,        /* The timers will auto-reload themselves when they expire. */
                               NULL,   /* Assign each timer a unique id equal to its array index. */
                               my_callback /* Each timer calls the same callback when it expires. */
                              );
    xTimerStart(xTimerofTest, 0);
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
    for ( ;; );
}

static void prvSetupHardware( void )
{
    /* system HW init */
    cache_init();
    mpu_init();

    /* peripherals init */
    hal_flash_init();

    hal_nvic_init();

    bsp_ept_gpio_setting_init();

    hal_rtc_init();
    hal_dcxo_init();

    mipi_power_init();
}

static void SystemClock_Config(void)
{
    hal_clock_init();

    hal_display_lcd_set_clock(HAL_DISPLAY_LCD_INTERFACE_CLOCK_124MHZ);
    hal_dvfs_init();
    hal_dvfs_target_cpu_frequency(target_freq, HAL_DVFS_FREQ_RELATION_H);
}

