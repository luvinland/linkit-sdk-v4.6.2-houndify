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

#ifndef _FONTRES_H_
#define _FONTRES_H_

#if defined(_MSC_VER)
#include "gdi_type_adaptor.h"
#else
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#endif

#define  MAX_FONT_TYPES      6

#define  FONTATTRIB_NORMAL                   0x00000001

typedef struct Range
{
    uint16_t nMin;
    uint16_t nMax;
} RangeData;

typedef struct RangeInfo
{
    uint16_t nNoOfRanges;
    const RangeData *pRangeData;

} RangeDetails;

typedef struct _CustFontData
{
    uint8_t nHeight;
    uint8_t nWidth;
    uint8_t nAscent;
    uint8_t nDescent;
    uint8_t nEquiDistant;
    uint8_t nCharBytes;
    uint16_t nMaxChars;
    uint8_t *pDWidthArray;
    uint8_t *pWidthArray;
    uint32_t *pOffsetArray;
    uint8_t *pDataArray;
    uint16_t *pRange;
    const RangeDetails *pRangeDetails;
    uint32_t language_flag;
} sCustFontData;

typedef struct _FontFamily
{
    uint16_t nTotalFonts;
    sCustFontData *fontData[MAX_FONT_TYPES];
} sFontFamily;

typedef struct 
{
    uint16_t nTotalFonts;
    const sCustFontData *const *fontData;
} font_group_struct;

#define MAX_FONT_SIZE MAX_FONT_TYPES

#endif /* _FONTRES_H_ */ 


