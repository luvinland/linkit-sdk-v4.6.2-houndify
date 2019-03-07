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

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Hal includes. */
#include "hal.h"
#include "hal_dcxo.h"

#include "syslog.h"
#include "io_def.h"
#include "bsp_gpio_ept_config.h"

#include "memory_map.h"
#include "memory_attribute.h"

#include <connsys_driver.h>
#include "connsys_profile.h"

#include <connsys_log.h>

#if defined(MTK_MINICLI_ENABLE)
#include "cli_def.h"
#endif

#ifdef MTK_WIFI_ROM_ENABLE
void move_iot_rom_data_to_ram();
#endif

extern uint32_t chip_hardware_code(void);
extern uint32_t chip_eco_version(void);

/* Create the log control block for different modules.
 * Please refer to the log dev guide under /doc folder for more details.
 */
#ifndef MTK_DEBUG_LEVEL_NONE
log_create_module(main, PRINT_LEVEL_INFO);


LOG_CONTROL_BLOCK_DECLARE(common);
LOG_CONTROL_BLOCK_DECLARE(hal);
LOG_CONTROL_BLOCK_DECLARE(main);
LOG_CONTROL_BLOCK_DECLARE(lwip);
LOG_CONTROL_BLOCK_DECLARE(fw_interface);
LOG_CONTROL_BLOCK_DECLARE(inband);
LOG_CONTROL_BLOCK_DECLARE(wifi);
LOG_CONTROL_BLOCK_DECLARE(wfc);


log_control_block_t *syslog_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(common),
    &LOG_CONTROL_BLOCK_SYMBOL(hal),
    &LOG_CONTROL_BLOCK_SYMBOL(main),
    &LOG_CONTROL_BLOCK_SYMBOL(lwip),
    &LOG_CONTROL_BLOCK_SYMBOL(fw_interface),
    &LOG_CONTROL_BLOCK_SYMBOL(inband),
    &LOG_CONTROL_BLOCK_SYMBOL(wifi),
    &LOG_CONTROL_BLOCK_SYMBOL(wfc),
    NULL
};
#endif

static void SystemClock_Config(void)
{
    hal_clock_init();
}

/**
* @brief       This function is to initialize cache controller.
* @param[in]   None.
* @return      None.
*/
static void cache_init(void)
{
    hal_cache_region_t region, region_number;

    /* Max region number is 16 */
    hal_cache_region_config_t region_cfg_tbl[] = {
        /* cacheable address, cacheable size(both MUST be 4k bytes aligned) */
        /* UBIN length */
#if ((PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7682))
#ifdef CM4_UBIN_LENGTH
        {CM4_BASE, CM4_LENGTH + CM4_UBIN_LENGTH},
#else
        {CM4_BASE, CM4_LENGTH},
#endif
#endif
        /* virtual sysram */
        {VSYSRAM_BASE, VSYSRAM_LENGTH},
#if (PRODUCT_VERSION == 7686)
        /* virtual memory */
        {VRAM_BASE, VRAM_LENGTH}
#endif
    };

    region_number = (hal_cache_region_t)(sizeof(region_cfg_tbl) / sizeof(region_cfg_tbl[0]));

    hal_cache_init();
    hal_cache_set_size(HAL_CACHE_SIZE_16KB);
    for (region = HAL_CACHE_REGION_0; region < region_number; region++) {
        hal_cache_region_config(region, &region_cfg_tbl[region]);
        hal_cache_region_enable(region);
    }
    for (; region < HAL_CACHE_REGION_MAX; region++) {
        hal_cache_region_disable(region);
    }
    hal_cache_enable();
}


static void prvSetupHardware(void)
{
    bsp_io_def_uart_init();

    /* sleep manager init*/
    hal_sleep_manager_init();

#ifdef HAL_FLASH_MODULE_ENABLED
    hal_flash_init();
#endif
    cache_init();
    hal_nvic_init();
#ifdef HAL_PMU_MODULE_ENABLED
    hal_pmu_init();
#endif
    hal_dcxo_init();
    hal_rtc_init();
}

void system_init(void)
{
    /* SystemClock Config */
    SystemClock_Config();
    SystemCoreClockUpdate();

    /* bsp_ept_gpio_setting_init() under driver/board/mt76x7_hdk/ept will initialize the GPIO settings
     * generated by easy pinmux tool (ept). ept_*.c and ept*.h are the ept files and will be used by
     * bsp_ept_gpio_setting_init() for GPIO pinumux setup.
     */
    bsp_ept_gpio_setting_init();

    /* Configure the hardware ready to run the test. */
    prvSetupHardware();

#ifndef MTK_WIFI_SLIM_ENABLE
    /* Connsys init will download FW, MT5932 need download N9 FW before any task created, to init ucHeap early */
    connsys_init(NULL);
#endif

#ifndef MTK_DEBUG_LEVEL_NONE
    log_init(NULL, NULL, syslog_control_blocks);
#endif
    LOG_I(common, "platform:MT%04x-E%d.\r\n", chip_hardware_code(), chip_eco_version());
    LOG_I(common, "system initialize done.\r\n");
}


