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

#include "hal_gpio_ept_config.h"
#include "mt2523.h"
#include "hal_feature_config.h"

#include "ept_gpio_drv.h"

#ifdef HAL_GPIO_MODULE_ENABLED
#ifdef HAL_GPIO_EPT_USED



extern GPIO_REGISTER_T *gpio_register_base;

#define GPIO_HWORD_REG_VAL(name, port0, port1, port2, port3, port4, port5, port6, port7, port8, port9, port10, \
	                            port11, port12, port13, port14, port15, port16, port17, port18, port19, port20, port21, \
                               port22, port23, port24, port25, port26, port27, port28, port29, port30, port31)      \
((GPIO_PORT##port0##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*0) & 0x1f)) | \
(GPIO_PORT##port1##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*1) & 0x1f)) |	\
(GPIO_PORT##port2##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*2) & 0x1f)) |   \
(GPIO_PORT##port3##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*3) & 0x1f)) |	\
(GPIO_PORT##port4##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*4) & 0x1f)) | \
(GPIO_PORT##port5##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*5) & 0x1f)) | 	\
(GPIO_PORT##port6##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*6) & 0x1f)) | \
(GPIO_PORT##port7##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*7) & 0x1f)) | 	\
(GPIO_PORT##port8##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*8) & 0x1f)) | \
(GPIO_PORT##port9##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*9) & 0x1f)) |	\
(GPIO_PORT##port10##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*10) & 0x1f)) |\
(GPIO_PORT##port11##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*11) & 0x1f)) |	\
(GPIO_PORT##port12##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*12) & 0x1f)) | \
(GPIO_PORT##port13##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*13) & 0x1f)) | 	\
(GPIO_PORT##port14##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*14) & 0x1f)) | \
(GPIO_PORT##port15##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*15) & 0x1f)) | 	\
(GPIO_PORT##port16##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*16) & 0x1f)) | \
(GPIO_PORT##port17##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*17) & 0x1f)) |	\
(GPIO_PORT##port18##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*18) & 0x1f)) |	\
(GPIO_PORT##port19##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*19) & 0x1f)) |	\
(GPIO_PORT##port20##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*20) & 0x1f)) | \
(GPIO_PORT##port21##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*21) & 0x1f)) | 	\
(GPIO_PORT##port22##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*22) & 0x1f)) | \
(GPIO_PORT##port23##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*23) & 0x1f)) | 	\
(GPIO_PORT##port24##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*24) & 0x1f)) | \
(GPIO_PORT##port25##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*25) & 0x1f)) |	\
(GPIO_PORT##port26##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*26) & 0x1f)) |	\
(GPIO_PORT##port27##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*27) & 0x1f)) |	\
(GPIO_PORT##port28##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*28) & 0x1f)) | \
(GPIO_PORT##port29##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*29) & 0x1f)) | 	\
(GPIO_PORT##port30##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*30) & 0x1f)) | \
(GPIO_PORT##port31##_##name<<((GPIO_##name##_##ONE_CONTROL_BITS*31) & 0x1f) ))



#define GPIO_WORD_VAR_FOR_1BIT(name, port0, port1, port2, port3, port4, port5, port6, port7, port8, port9, port10, \
	                            port11, port12, port13, port14, port15, port16, port17, port18, port19, port20, port21, \
                               port22, port23, port24, port25, port26, port27, port28, port29, port30, port31) \
    ((name##_GPIO##port0) | (name##_GPIO##port1<<1) | (name##_GPIO##port2<<2) | (name##_GPIO##port3<<3) | (name##_GPIO##port4<<4) | \
    (name##_GPIO##port5<<5) | (name##_GPIO##port6<<6) | (name##_GPIO##port7<<7) | (name##_GPIO##port8<<8) | (name##_GPIO##port9<<9) | \
    (name##_GPIO##port10<<10) | (name##_GPIO##port11<<11) | (name##_GPIO##port12<<12) | (name##_GPIO##port13<<13) | (name##_GPIO##port14<<14) | \
    (name##_GPIO##port15<<15) | (name##_GPIO##port16<<16) | (name##_GPIO##port17<<17) | (name##_GPIO##port18<<18) | (name##_GPIO##port19<<19) | \
    (name##_GPIO##port20<<20) | (name##_GPIO##port21<<21) | (name##_GPIO##port22<<22) | (name##_GPIO##port23<<23) | (name##_GPIO##port24<<24) | \
    (name##_GPIO##port25<<25) | (name##_GPIO##port26<<26) | (name##_GPIO##port27<<27) | (name##_GPIO##port28<<28) | (name##_GPIO##port29<<29) | \
    (name##_GPIO##port30<<30) | (name##_GPIO##port31<<31))




void GPIO_MODE_Init(void)
{
    uint8_t i;

#ifdef GPIO_MODE_REG_MAX_NUM
    uint32_t mode_temp[GPIO_MODE_REG_MAX_NUM + 1] = { GPIO_MODE_ALL_VALUE };

    for (i = 0; i <= GPIO_MODE_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_MODE_REGISTER[i].GPIO_MODE = mode_temp[i];
    }
#endif
}


void GPIO_DIR_Init(void)
{
    uint8_t i;

#ifdef GPIO_DIR_REG_MAX_NUM
    uint32_t dir_temp[GPIO_DIR_REG_MAX_NUM + 1] = { GPIO_DIR_ALL_VALUE };

    for (i = 0; i <= GPIO_DIR_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_DIR_REGISTER[i].GPIO_DIR = dir_temp[i];
    }
#endif
}


void GPIO_PULL_SEL_Init(void)
{
    uint8_t i;

#ifdef GPIO_PULL_SEL_REG_MAX_NUM
    uint32_t pullsel_temp[GPIO_PULL_SEL_REG_MAX_NUM + 1] = { GPIO_PULL_SEL_ALL_VALUE };

    for (i = 0; i <= GPIO_PULL_SEL_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_PULLSEL_REGISTER[i].GPIO_PULLSEL = pullsel_temp[i];
    }
#endif
}


void GPIO_PULLEN_Init(void)
{
    uint8_t i;

#ifdef GPIO_PULL_REG_MAX_NUM
    uint32_t pullen_temp[GPIO_PULL_REG_MAX_NUM + 1] = { GPIO_PULL_ALL_VALUE };

    for (i = 0; i <= GPIO_PULL_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_PULLEN_REGISTER[i].GPIO_PULLEN = pullen_temp[i];
    }
#endif
}




void GPIO_OUTPUT_Init(void)
{
    uint8_t i;

#ifdef GPIO_OUTPUT_LEVEL_REG_MAX_NUM
    uint32_t dout_temp[GPIO_OUTPUT_LEVEL_REG_MAX_NUM + 1] = { GPIO_OUTPUT_LEVEL_ALL_VALUE };

    for (i = 0; i <= GPIO_OUTPUT_LEVEL_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_DOUT_REGISTER[i].GPIO_DOUT = dout_temp[i];
    }
#endif
}

void GPIO_PUPD_Init(void)
{
    uint8_t i;

#ifdef GPIO_PUPD_REG_MAX_NUM
    uint32_t pupd_temp[GPIO_PUPD_REG_MAX_NUM + 1] = { GPIO_PUPD_ALL_VALUE };

    for (i = 0; i <= GPIO_PUPD_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_PUPD_REGISTER[i].GPIO_PUPD = pupd_temp[i];
    }
#endif
}



void GPIO_R0_Init(void)
{
    uint8_t i;

#ifdef GPIO_R0_REG_MAX_NUM
    uint32_t r0_temp[GPIO_R0_REG_MAX_NUM + 1] = { GPIO_R0_ALL_VALUE };

    for (i = 0; i <= GPIO_R0_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_RESEN0_REGISTER[i].GPIO_RESEN0 = r0_temp[i];
    }
#endif
}


void GPIO_R1_Init(void)
{
    uint8_t i;

#ifdef GPIO_R1_REG_MAX_NUM
    uint32_t r1_temp[GPIO_R1_REG_MAX_NUM + 1] = { GPIO_R1_ALL_VALUE };


    for (i = 0; i <= GPIO_R1_REG_MAX_NUM; i++) {
        gpio_register_base->GPIO_RESEN1_REGISTER[i].GPIO_RESEN1 = r1_temp[i];
    }
#endif
}


void GPIO_setting_init(void)
{
    GPIO_MODE_Init();
    GPIO_DIR_Init();
    GPIO_PULL_SEL_Init();
    GPIO_PULLEN_Init();
    GPIO_OUTPUT_Init();
    GPIO_PUPD_Init();
    GPIO_R0_Init();
    GPIO_R1_Init();

    /* GPIO49 must work in mode of D2D_I2C_CLK and GPIO50 must work in mode of D2D_I2C_DAT,
       GPIO51 must work in mode of D2D_I2C_TRPI1 and GPIO50 must work in mode of D2D_I2C_TRPI0 */

    *(volatile uint32_t *)(0xA2020C68) = 0x77770;
    *(volatile uint32_t *)(0xA2020C64) = 0x11110;
}

#else

void GPIO_setting_init(void)
{

}




#endif
#endif

