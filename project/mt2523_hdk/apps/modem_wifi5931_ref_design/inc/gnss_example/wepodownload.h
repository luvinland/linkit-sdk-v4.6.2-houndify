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

#ifndef __WEPODOWNLOAD_H__
#define __WEPODOWNLOAD_H__

#include "gnss_app.h"
#ifdef __GPS_EPO_DOWNLOAD__

#include <stdint.h>
#include <stdio.h>
#include <string.h>


/********************START typedef***************************/
typedef char                    kal_char;
/* portable wide character for unicode character set */
typedef unsigned short          kal_wchar;

/* portable 8-bit unsigned integer */
typedef unsigned char           kal_uint8;
/* portable 8-bit signed integer */
typedef signed char             kal_int8;
/* portable 16-bit unsigned integer */
typedef unsigned short int      kal_uint16;
/* portable 16-bit signed integer */
typedef signed short int        kal_int16;
/* portable 32-bit unsigned integer */
typedef unsigned int            kal_uint32;
/* portable 32-bit signed integer */
typedef signed int              kal_int32;

//Rolltech
#if 0
//#if !defined(GEN_FOR_PC) && !defined(__MTK_TARGET__)
/* portable 64-bit unsigned integer */
   typedef unsigned __int64     kal_uint64;
/* portable 64-bit signed integer */
   typedef __int64              kal_int64;
#else
/* portable 64-bit unsigned integer */
   typedef unsigned long long   kal_uint64;
/* portable 64-bit signed integer */
   typedef signed long long     kal_int64;
#endif

/* boolean representation */
typedef enum 
{
    /* FALSE value */
    KAL_FALSE,
    /* TRUE value */
    KAL_TRUE
} kal_bool;

#if !defined(_WINNT_H) && !defined(_WINNT_)
typedef unsigned short WCHAR;
#endif

/*******************************************************************************
 * Constant definition
 *******************************************************************************/
#ifndef NULL
#define NULL               0
#endif


typedef int32_t msg_type;

#define LOCAL_PARA_HDR \
   kal_uint8	ref_count; \
   kal_uint8    lp_reserved; \
   kal_uint16	msg_len;
/*  common local_para header */
typedef struct local_para_struct {
    /* ref_count: reference count; 
     * lp_reserved : reserved for future; 
     * msg_len  : total length including this header.
     */
    LOCAL_PARA_HDR
#ifdef __BUILD_DOM__
    ;
#endif
} local_para_struct;


typedef struct {
    msg_type msg_id; /* Message identifier */
    local_para_struct *local_para_ptr; /* local_para pointer */
} ilm_struct;


typedef enum {
	GPS_EPO_DOWNLOAD_TIMER,
	GPS_EPO_DOWNLOAD_3DAY_TIMEOUT_TIMER,
	GPS_EPO_DOWNLOAD_QEOP_TIMEOUT_TIMER,
	GPS_EPO_TIME_MAX
} gps_timer_enum;

typedef struct
{
    kal_uint32 i[2];        /* number of _bits_ handled mod 2^64 */
    kal_uint32 buf[4];      /* scratch buffer */
    kal_uint8 in[64];       /* input buffer */
    kal_uint8 digest[16];   /* actual digest after MD5Final call */
} applib_md5_ctx;

/*********************END typedef**************************/

typedef enum {
    EPO_MODE_G,
    EPO_MODE_G_G,
    EPO_MODE_G_B,
    EPO_MODE_MAX
} epo_mode_enum;


typedef enum {
    STATE_EPO_3_DAY_DOWNLOAD = 1,
    STATE_Q_EPO_DOWNLOAD = 1 << 1,
    STATE_Q_AND_3_DAY_EPO_DOWNLOAD = STATE_EPO_3_DAY_DOWNLOAD | STATE_Q_EPO_DOWNLOAD,
    STATE_EPO_IDLE
} epo_state_enum;


typedef enum {
    EPO_FILE_TYPE_3_DAY_EPO,
    EPO_FILE_TYPE_QEPO,
    EPO_FILE_TYPE_MAX
} epo_file_type;


typedef enum {
    EPO_CURR_TIMER_TYPE_NEXT_DOWNLOAD_TIMING,
    EPO_CURR_TIMER_TYPE_RETRY_DOWNLOAD_TIMING,
    EPO_CURR_TIMER_TYPE_DELAY_DOWNLOAD_TIMING,
    EPO_CURR_TIMER_TYPE_CANCLE_AIDING,
    EPO_CURR_TIMER_TYPE_DOWNLOAD_TIMEOUT,
    EPO_CURR_TIMER_TYPE_MAX
} epo_timer_type;

typedef enum {
    EPO_DOWNLOAD_ERROR_TYPE_GET_HOSTNAME_FAIL,
    EPO_DOWNLOAD_ERROR_TYPE_SOCKET_CREATE_FAIL,
    EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_BROKEN,
    EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_TIMEOUT,
    EPO_DOWNLOAD_ERROR_TYPE_NO_ENOUGH_SPACE,
    EPO_DOWNLOAD_ERROR_TYPE_FILE_CHECKSUM_ERROR,
    EPO_DOWNLOAD_ERROR_TYPE_FILE_OPERATE_ERROR,
    EPO_DOWNLOAD_ERROR_TYPE_MAX
} epo_download_error_enum;


#define EPO_G_G_3_DAY_SIZE 48384
#define EPO_G_3_DAY_SIZE 27648
#define EPO_G_G_Q_SIZE 4032
#define EPO_G_Q_SIZE 2304

#define EPO_DEFAULT 0
#define EPO_MAX_CHUNK 2
#define EPO_EXPIRED_TIME 23

#define EPO_RECORD_SIZE     72
#define EPO_SV_NUMBER       32

#define EPO_3_DAY_RETRY_TIME_BASE(X) (((1 << (X)) > 8 ? 12 : (1 << (X)))*5*60*1000)


#ifdef FAT_FS
FS_HANDLE epo_file_open(epo_file_type file_type, kal_int32 file_index, kal_uint32 openflag);
#else
typedef kal_int32 FS_HANDLE;
FS_HANDLE epo_file_open(epo_file_type file_type, kal_int32 file_index, kal_uint32 openflag);
void FS_Close(FS_HANDLE fHandle);
kal_int32 FS_Write(FS_HANDLE fHandle, kal_int8* buf, kal_int32 buf_len, kal_int32* write_len);
kal_int32 FS_Read(FS_HANDLE fHandle, kal_int8* buf, kal_int32 buf_len, kal_int32* read_len);
kal_int32 FS_Seek(FS_HANDLE fHandle, kal_int32 offset, kal_int32 flag);
kal_int32 FS_Delete(FS_HANDLE fHandle);

#define FS_READ_ONLY 0
#define FS_CREATE_ALWAYS 0
#define SEEK_SET 0
#endif

typedef struct 
{
    kal_int8 recv_buf[1024 + 1];
    kal_int8 header_buf[1024 + 1];
    kal_uint32 recv_buf_len;
    kal_bool is_head_recieving;
    kal_int32 sum;
    kal_int8 check_sum[32+1];
    epo_file_type file_type;
    FS_HANDLE epo_file_handle;
	kal_int32 retry_times;
} epo_http_hdlr_struct;

extern char* EPO_DOWNLOAD_SERVER_NAME;


extern kal_uint32 app_gettime(kal_uint32 *t_loc);

extern void applib_md5_init(applib_md5_ctx *mdContext);
extern void applib_md5_update(applib_md5_ctx *mdContext, const kal_uint8 *inBuf, kal_uint32 inLen);
extern void applib_md5_final(kal_uint8 hash[], applib_md5_ctx *mdContext);

void epo_connectivty_message_handler(ilm_struct *ilm_ptr);
void epo_init(kal_int32 trunk_num, char* epo_server_name, char* query_string);
kal_bool epo_aiding(kal_uint32 chip_gps_hours);
kal_uint32* epo_get_single_sv_data(kal_int32 num_sv);
void epo_aiding_cancle();
void epo_download_current_trunk_finish_notify(epo_http_hdlr_struct *connection);
void epo_download_current_trunk_finish_handle(void *connection);
int32_t epo_downlaod_get_curr_download_index();
void httpc_download_init();
void httpc_download_cancle_download(epo_http_hdlr_struct *connection);
void httpc_download_start_download(int32_t thread_index, epo_http_hdlr_struct *connection, int8_t* URL);

#endif
#endif


