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

#include "hal_sys_topsm.h"
#include "hal_dsp_topsm.h"
#include "hal_pinmux_define.h"
#include "hal_pdma_internal.h"
#include "bsp_gpio_ept_config.h"
#include "hal_flash.h"
#include "hal_mipi_tx_config_internal.h"
#include "hal_gpio.h"

//#define GNSS_COCLOCK_ENABLE
#ifdef GNSS_COCLOCK_ENABLE
#include "gnss_log.h"
bool is_coclock_enable()
{
    static bool is_default_enable;
    static bool is_first_called;
    int *gnss_32k_gpio_setting = 0xa2020c10;
    int *gnss_clock_frequnce = 0xa2020e30;
    GNSSLOGD("is_coclock_enable, default enable:%d, first call:%d", is_default_enable, is_first_called);

    if (is_first_called) {
        return is_default_enable;
    }

    if ((((*gnss_32k_gpio_setting) >> 20) & 0x7) == HAL_GPIO_13_CLKO3
            && ((*gnss_clock_frequnce) & 0xF) == HAL_GPIO_CLOCK_MODE_32K) {
        GNSSLOGD("default enabled");
        is_default_enable = TRUE;
    } else {
        GNSSLOGD("default disabled");
        is_default_enable = FALSE;
    }
    is_first_called = TRUE;
    return is_default_enable;
}

void gnss_driver_init_coclock()
{
    if (!is_coclock_enable()) {
        if (HAL_PINMUX_STATUS_OK != hal_pinmux_set_function(HAL_GPIO_13, HAL_GPIO_13_CLKO3)) {
            GNSSLOGD("set pinmux error");
        }
        if (HAL_GPIO_STATUS_OK != hal_gpio_set_clockout(HAL_GPIO_CLOCK_3, HAL_GPIO_CLOCK_MODE_32K)) {
            GNSSLOGD("set clock error");
        }
    }
}

void gnss_driver_deinit_coclock()
{
    if (!is_coclock_enable()) {
        if (HAL_PINMUX_STATUS_OK != hal_pinmux_set_function(HAL_GPIO_13, HAL_GPIO_13_GPIO13)) {
            GNSSLOGD("set GPIO error");
        }
        if (HAL_GPIO_STATUS_OK != hal_gpio_set_direction(HAL_GPIO_13, HAL_GPIO_DIRECTION_OUTPUT)) {
            GNSSLOGD("set direction error");
        }
        if (HAL_GPIO_STATUS_OK != hal_gpio_set_output(HAL_GPIO_13, HAL_GPIO_DATA_LOW)) {
            GNSSLOGD("pull low error");
        }
    }
    *((volatile uint32_t *)(0xA2030348)) = 0x00000400;
}
#endif

