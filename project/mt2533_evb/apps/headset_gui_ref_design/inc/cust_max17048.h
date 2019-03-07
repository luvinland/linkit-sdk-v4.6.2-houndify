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

#ifndef __CUST_MAX17048_H__
#define __CUST_MAX17048_H__

#ifdef __cplusplus
extern "C" {
#endif

const unsigned char max17048_model_data[64] = {

    0x86, 0x30, 0xAD, 0xF0, 0xB4, 0xE0, 0xB6, 0xC0,
    0xBA, 0x30, 0xBB, 0xA0, 0xBC, 0xA0, 0xBD, 0xF0,
    0xBE, 0xE0, 0xBF, 0xD0, 0xC2, 0x00, 0xC3, 0xE0,
    0xC6, 0x20, 0xC8, 0x60, 0xCC, 0x00, 0xD0, 0x10,
    0x00, 0x20, 0x01, 0x10, 0x03, 0x00, 0x10, 0xF0,
    0x16, 0xF0, 0x1A, 0x80, 0x22, 0x30, 0x17, 0x90,
    0x18, 0x40, 0x0D, 0xE0, 0x0A, 0x60, 0x0C, 0x80,
    0x0C, 0x00, 0x0A, 0x10, 0x08, 0xC0, 0x08, 0xC0

};

/* MAXIM MAX17048 INI define */
#define MAX17048_RCOMP_SEG      0x0080
#define MAX17048_OCV_TEST       55824
#define MAX17048_SOC_CAHECK_A   121
#define MAX17048_SOC_CAHECK_B   123
#define MAX17048_RCOMP0         44
#define MAX17048_TEMP_CO_UP     (-0.5f)
#define MAX17048_TEMP_CO_DOWN   (-3.95f)
#define MAX17048_BITS           18

/* User  define */
#define MAX17048_HIB_THR        0x80
#define MAX17048_ACT_THR        0x30
#define MAX17048_MODE           0x20    /* Enable Sleep mode */


#ifdef __cplusplus
}
#endif

#endif /*__CUST_MAX17048_H__*/

