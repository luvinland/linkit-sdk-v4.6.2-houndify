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

#include "wepodownload.h"

#ifdef __GPS_EPO_DOWNLOAD__
#include "hal_flash.h"
#include "gnss_timer.h"
#include "gnss_app.h"
#include "hal_rtc.h"
#include "md5.h"
#ifdef MODEM_ENABLE
#include "sio_gprot.h"
#include "gprs_api.h"
#endif
#include "md5.h"
#include "epo_demo.h"
#define QEPO_MAX_CHUNK 5

#define EPO_3_DAY_DEFAULT_NAME "epo_3day_temp.DAT"
#define BLOCK_4K (4*1024)
// please help to contact with CPM about the avaliable EPO download URL
#define EPO_DOWNLOAD_SERVER_URL ""


//#define EPO_DEBUG_LOG(MODE, FORMAT_STRING, args...) kal_prompt_trace(MODE, "[EPO]"FORMAT_STRING, args)
//#define EPO_DEBUG_LOG kal_prompt_trace
#define GPS_TRACE(x)
#define EPO_DEBUG_LOG(mod, fmt, args...) GNSSLOGE(fmt, ##args)

#define PRE_FIX "[EPO]"

#define EPO_SEG_BUF_SIZE (EPO_RECORD_SIZE * EPO_SV_NUMBER) >> 2

#define EPO_DOWNLOAD_TIMEOUT_TIMER_DURATION 60*1000

#include "assert.h"
#ifndef ASSERT
   #define ASSERT(x) assert(x)
#endif

struct 
{
    epo_mode_enum epo_mode;
    epo_state_enum state;
    kal_int32 total_chunk;
    kal_int32 curr_download_chunk;


    kal_int8 app_id;

    epo_http_hdlr_struct request_handle[EPO_FILE_TYPE_MAX];
    kal_bool using_qepo_backup_file;

    kal_bool need_aiding;
    kal_uint32 gps_aiding_hours;
    kal_uint32 u4EPOWORD[EPO_SEG_BUF_SIZE];//4032 bytes,  means 72 * (32 + 24) bytes, 32 GPS sat, 24 GLONASS sat
    char* epo_server_name;
    char* query_string;
    kal_int8 is_init;

    kal_int32 gps_epo_time_pool[GPS_EPO_TIME_MAX];
    epo_timer_type timeout_type[GPS_EPO_TIME_MAX];

#ifndef FAT_FS

    uint32_t start_address;
    uint32_t end_address;

    uint32_t block_size;
    uint32_t ubin_pack_count;
    hal_flash_block_t block_type;
#endif

} g_epo_download_cntx;


kal_int8 * epo_name_3_day_array[EPO_MODE_MAX] = 
{
    (kal_int8 *) "EPO_GPS_3_%d.DAT",
    (kal_int8 *) "EPO_GR_3_%d.DAT",
    (kal_int8 *) ""
};

kal_int8 * qepo_name_array[EPO_MODE_MAX] = 
{
    (kal_int8 *) "QGPS_%d.DAT",
    (kal_int8 *) "QG_R_%d.DAT",
    (kal_int8 *) ""
};


void epo_download_3_day_single_chunk();
void epo_qepo_after_download_success_handle(epo_http_hdlr_struct* connection);
void epo_3_day_after_download_success_handle(epo_http_hdlr_struct* connection);
void epo_download_3_day();
kal_uint32 epo_get_default_file_size(epo_file_type file_type);
kal_int8* epo_get_qepo_download_url(kal_int32 index);
void epo_download_timer_callback(int32_t tid);
void epo_download_start_timer(gps_timer_enum timer_id, kal_uint32 period, gnss_timer_callback_f timer_expiry, void *arg);
void epo_download_stop_timer(gps_timer_enum timer_id);
kal_int32 qepo_get_download_index();
kal_bool epo_try_qepo_aiding(kal_uint32 chip_gps_hours);
kal_bool epo_try_qepo_aiding_ext();
void epo_qepo_download();
void epo_create_and_start_connection(epo_file_type file_type);
kal_bool epo_is_3_day_epo_downloading();
kal_int32 epodownload_file_delete(epo_file_type file_type, kal_int32 file_index);



#define APP_LIB_DATA_TIME 
#define EMPTY_MMI
#ifdef APP_LIB_DATA_TIME
typedef struct
{
    kal_uint16 nYear;
    kal_uint8 nMonth;
    kal_uint8 nDay;
    kal_uint8 nHour;
    kal_uint8 nMin;
    kal_uint8 nSec;
    kal_uint8 DayIndex; /* 0=Sunday */
} applib_time_struct;

static void applib_dt_get_rtc_time(applib_time_struct *t)
{
    hal_rtc_time_t rtc_time = {0};
    hal_rtc_get_time(&rtc_time);
    t->nDay = rtc_time.rtc_day;
    t->nHour = rtc_time.rtc_hour;
    t->nMin = rtc_time.rtc_min;
    t->nMonth = rtc_time.rtc_mon;
    t->nSec = rtc_time.rtc_sec;
    t->nYear = rtc_time.rtc_year + 2000;
}

static void applib_dt_init_time(void)
{
}

#endif

void epo_download_finish_connection(epo_http_hdlr_struct* connection)
{
    connection->is_head_recieving = KAL_FALSE;
    connection->recv_buf_len = 0;
    connection->sum = 0;
    httpc_download_cancle_download(connection);
}

void epo_download_error_handle(epo_http_hdlr_struct* connection, epo_download_error_enum error)
{
//    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_download_error_handle");
//    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"state:%d, error:%d, file_type:%d", g_epo_download_cntx.state, error, connection->file_type);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_ERROR_HANDLE, g_epo_download_cntx.state, error, connection->file_type));

    epo_aiding_cancle();

    epo_download_finish_connection(connection);
#ifndef FAT_FS
    epodownload_file_delete(connection->file_type, g_epo_download_cntx.curr_download_chunk);
#endif
    if (connection->file_type == EPO_FILE_TYPE_QEPO)
    {
        if (g_epo_download_cntx.state == STATE_Q_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_EPO_IDLE;
        }
        else if (g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_EPO_3_DAY_DOWNLOAD;
        }
    }
    else if (connection->file_type == EPO_FILE_TYPE_3_DAY_EPO)
    {
        if (g_epo_download_cntx.state == STATE_EPO_3_DAY_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_EPO_IDLE;
        }
        else if (g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_Q_EPO_DOWNLOAD;
        }
        epo_download_stop_timer(GPS_EPO_DOWNLOAD_3DAY_TIMEOUT_TIMER);
    }
    
    switch (error)
    {
        case EPO_DOWNLOAD_ERROR_TYPE_SOCKET_CREATE_FAIL:
        case EPO_DOWNLOAD_ERROR_TYPE_FILE_OPERATE_ERROR:
            // should be ok, cannot handle this case.
            ASSERT(0);
            break;
        case EPO_DOWNLOAD_ERROR_TYPE_GET_HOSTNAME_FAIL:
        case EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_BROKEN:
        case EPO_DOWNLOAD_ERROR_TYPE_FILE_CHECKSUM_ERROR:
        case EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_TIMEOUT:
            // try again. only for 3 day epo
            if (connection->file_type == EPO_FILE_TYPE_3_DAY_EPO)
            {
                kal_uint32 download_time;
                download_time = EPO_3_DAY_RETRY_TIME_BASE(connection->retry_times);
                connection->retry_times++;
                epo_download_start_timer(
                    GPS_EPO_DOWNLOAD_TIMER,
                    download_time, 
                    epo_download_timer_callback, 
                    (void*) EPO_CURR_TIMER_TYPE_NEXT_DOWNLOAD_TIMING);
            }
            else if (connection->file_type == EPO_FILE_TYPE_QEPO)
            {
                //epo_download_3_day();// try to download 3 days
            }
            break;
         case EPO_DOWNLOAD_ERROR_TYPE_NO_ENOUGH_SPACE:
            if (connection->file_type == EPO_FILE_TYPE_QEPO)
            {
                ASSERT(0);
            }
            else
            {
                if (g_epo_download_cntx.curr_download_chunk == 1)
                {
                    ASSERT(0);
                }
                else
                {
                    g_epo_download_cntx.total_chunk = g_epo_download_cntx.curr_download_chunk;
                    epo_download_3_day();
                }
            }
            break;
    }
}


void epo_download_start_timer(gps_timer_enum timer_id, kal_uint32 period, gnss_timer_callback_f timer_expiry, void *arg)
{
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_download_start_timer, arg:%d", (kal_int32) arg);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_START_TIMER, timer_id, period, arg));
    g_epo_download_cntx.gps_epo_time_pool[timer_id] = gnss_start_timer(period, timer_expiry);
    g_epo_download_cntx.timeout_type[timer_id] = (epo_timer_type) arg;
}

void epo_download_stop_timer(gps_timer_enum timer_id)
{
    if (g_epo_download_cntx.gps_epo_time_pool[timer_id] == -1)
        return ;
    gnss_stop_timer(g_epo_download_cntx.gps_epo_time_pool[timer_id]);
    g_epo_download_cntx.gps_epo_time_pool[timer_id] = -1;
}

kal_int8* epo_construct_3_day_epo_file_name(kal_int32 index)
{
    static kal_int8 epo_3_day_name[20];
    GNSSLOGD("construct file name:mode[%d]name[%s]index[%d]\n",g_epo_download_cntx.epo_mode, epo_name_3_day_array[g_epo_download_cntx.epo_mode], index);
    sprintf((char*) &epo_3_day_name[0], (char*) epo_name_3_day_array[g_epo_download_cntx.epo_mode], index);
    return epo_3_day_name;
}

kal_int8* epo_construct_qepo_file_name(kal_int32 index)
{
    static kal_int8 qepo_name[15];
    sprintf((char*) &qepo_name[0], (char*) qepo_name_array[g_epo_download_cntx.epo_mode], index);
    return qepo_name;
}

kal_int8* epo_get_3_day_epo_download_url(kal_int32 index)
{
    static kal_int8 epo_3_day_url[100];
    epo_3_day_url[0] = 0;

    strcat((char*) epo_3_day_url, (char*) "http://");
    strcat((char*) epo_3_day_url, (char*) g_epo_download_cntx.epo_server_name);
    strcat((char*) epo_3_day_url, "/");
    strcat((char*) epo_3_day_url, (char*) epo_construct_3_day_epo_file_name(index));
    if (g_epo_download_cntx.query_string)
    {
        strcat((char*) epo_3_day_url, "?");
        strcat((char*) epo_3_day_url, (char*) g_epo_download_cntx.query_string);
    }

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"3 day url:%s", epo_3_day_url);
    return epo_3_day_url;
}

kal_int8* epo_get_qepo_download_url(kal_int32 index)
{
    static kal_int8 qepo_url[100];
    qepo_url[0] = 0;

    strcat((char*) qepo_url, "http://");
    strcat((char*) qepo_url, (char*) g_epo_download_cntx.epo_server_name);
    strcat((char*) qepo_url, "/");
    strcat((char*) qepo_url, (char*) epo_construct_qepo_file_name(index));
    if (g_epo_download_cntx.query_string)
    {
        strcat((char*) qepo_url, "?");
        strcat((char*) qepo_url, (char*) g_epo_download_cntx.query_string);
    }

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"qepo url:%s", qepo_url);
    return qepo_url;
}


#ifdef FAT_FS
static void convert_string_to_wstring(kal_int8* string, kal_int16* wstring)
{
    kal_int32 index = 0;
    if (!string) {
        return ;
    }
    while (*string) {
        wstring[index] = *string;
        string++;
        index++;
    }
    wstring[index] = 0;
}


kal_int32 epodownload_file_get_drvletter (void)
{
    static kal_int32 iDrvLetter;

    if (iDrvLetter == 0)
    {
        iDrvLetter = FS_GetDrive(FS_DRIVE_I_SYSTEM , 1, FS_DRIVE_V_REMOVABLE | FS_DRIVE_V_NORMAL);
    }
    return iDrvLetter;
}


kal_int8* epo_construct_3_day_epo_file_path(kal_int32 index)
{
    static kal_int8 epo_3_day_path[30];
    epo_3_day_path[0] = epodownload_file_get_drvletter();
    epo_3_day_path[1] = '\0';
    strcat(epo_3_day_path, ":\\@EPO\\");
    sprintf(epo_3_day_path + strlen(epo_3_day_path), epo_name_3_day_array[g_epo_download_cntx.epo_mode], index);
    return epo_3_day_path;
}

kal_int8* epo_construct_qepo_file_path(kal_int32 index)
{
    static kal_int8 qepo_path[30];
    qepo_path[0] = epodownload_file_get_drvletter();
    qepo_path[1] = '\0';
    strcat(qepo_path, ":\\@EPO\\");
    sprintf(qepo_path + strlen(qepo_path), qepo_name_array[g_epo_download_cntx.epo_mode], index);
    return qepo_path;
}

FS_HANDLE epo_file_open(epo_file_type file_type, kal_int32 file_index, kal_uint32 openflag)
{
    FS_HANDLE file_handle;    
    kal_int16 wstring[50];
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_file_open, file_type:%d, file_index:%d", file_type, file_index);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_OPEN, file_type, file_index));
    
    if (file_type == EPO_FILE_TYPE_3_DAY_EPO)
    {
        convert_string_to_wstring(epo_construct_3_day_epo_file_path(file_index), wstring);
        file_handle = FS_Open(wstring, openflag);
    }
    else if (file_type == EPO_FILE_TYPE_QEPO)
    {
        convert_string_to_wstring(epo_construct_qepo_file_path(file_index), wstring);
        file_handle = FS_Open(wstring, openflag);
    }
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"file_handle = %d", file_handle);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_HANDLE, file_handle));

    return file_handle;
}

kal_int32 epodownload_file_rename(epo_file_type file_type, kal_int32 file_index_old, kal_int32 file_index_new)
{
    kal_int32 ret;
    kal_int16 wfile_index_old[50];
    kal_int16 wfile_index_new[50];
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epodownload_file_rename, file_type:%d, file_index_old:%d, file_index_new:%d", 
        file_type, 
        file_index_old,
        file_index_new));

    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_RENAME,
        file_type, 
        file_index_old,
        file_index_new));

    if (file_type == EPO_FILE_TYPE_3_DAY_EPO)
    {
        convert_string_to_wstring(epo_construct_3_day_epo_file_path(file_index_old), wfile_index_old);
        convert_string_to_wstring(epo_construct_3_day_epo_file_path(file_index_new), wfile_index_new);
        ret = FS_Rename(wfile_index_old,
            wfile_index_new);
    }
    else if (file_type == EPO_FILE_TYPE_QEPO)
    {
        convert_string_to_wstring(epo_construct_qepo_file_path(file_index_old), wfile_index_old);
        convert_string_to_wstring(epo_construct_qepo_file_path(file_index_new), wfile_index_new);
        ret = FS_Rename(wfile_index_old,
            wfile_index_new);
    }
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"ret = %d", ret);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_RENAME_RET, ret));
    
    return ret;
}

kal_int32 epodownload_file_delete(epo_file_type file_type, kal_int32 file_index)
{
    kal_int32 ret;
    kal_int16 wstring[50];
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epodownload_file_delete, file_type:%d, file_index:%d", file_type, file_index);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_DELETE, file_type, file_index));
    
    if (file_type == EPO_FILE_TYPE_3_DAY_EPO)
    {
        convert_string_to_wstring(epo_construct_3_day_epo_file_path(file_index), wstring);
        ret = FS_Delete(wstring);
    }
    else if (file_type == EPO_FILE_TYPE_QEPO)
    {
        convert_string_to_wstring(epo_construct_qepo_file_path(file_index), wstring);
        ret = FS_Delete(wstring);
    }
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"ret = %d", ret);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_DELETE_RET, ret));
    return ret;
}



void epodownload_file_replace(epo_file_type file_type, kal_int32 file_index_old, kal_int32 file_index_new)
{
    kal_int32 ret;
    ret = epodownload_file_delete(file_type, file_index_old);
    ret = epodownload_file_rename(file_type, file_index_new, file_index_old);
}

#else
uint32_t g_3_day_epo_handle[2];
uint32_t g_q_epo_handle;
uint32_t get_file_base_addr(epo_file_type file_type, kal_int32 file_index)
{
    kal_int32 offset = 0;
    uint32_t fHandle;
    if (file_type == EPO_FILE_TYPE_3_DAY_EPO)
    {
        if (file_index >= 0 && file_index <= g_epo_download_cntx.total_chunk) {
            offset = epo_get_default_file_size(file_type) * (file_index - 1);
        } else {
            EPO_DEBUG_LOG(MOD_GPS, "[EPO] index error, file_type:%d, file_index:%d, start_addr:0x%x\n", file_type, file_index, g_epo_download_cntx.start_address);
            return 0;
        }
    }
    else if (file_type == EPO_FILE_TYPE_QEPO)
    {
        file_index = 0;
    	offset = epo_get_default_file_size(EPO_FILE_TYPE_3_DAY_EPO) * (g_epo_download_cntx.total_chunk);
    } else {
        EPO_DEBUG_LOG(MOD_GPS, "[EPO] type error, file_type:%d, file_index:%d, start_addr:0x%x\n", file_type, file_index, g_epo_download_cntx.start_address);
        return 0;
    }
    fHandle = offset + g_epo_download_cntx.start_address;
    if (fHandle > g_epo_download_cntx.end_address)
    {
        printf("file size exceed quota");
        return 0;
    }
    fHandle = (((file_type << 4) + file_index) << 24) + fHandle;
    return fHandle;
}

FS_HANDLE epo_file_open(epo_file_type file_type, kal_int32 file_index, kal_uint32 openflag)
{
	kal_int32 offset = 0;
	uint32_t* fHandle;
    if (file_type == EPO_FILE_TYPE_3_DAY_EPO)
	{
		offset = epo_get_default_file_size(file_type) * (file_index - 1);
        if (file_index >= 0 && file_index <= g_epo_download_cntx.total_chunk) {
            fHandle = &g_3_day_epo_handle[file_index];
        } else {
            EPO_DEBUG_LOG(MOD_GPS, "[EPO] 3d epo open, file_type:%d, file_index:%d, start_addr:0x%x\n", file_type, file_index, g_epo_download_cntx.start_address);
            return 0;
        }
	}
	else if (file_type == EPO_FILE_TYPE_QEPO)
	{
	    file_index = 0;
		offset = epo_get_default_file_size(EPO_FILE_TYPE_3_DAY_EPO) * g_epo_download_cntx.total_chunk;
        fHandle = &g_q_epo_handle;
	} else {
        EPO_DEBUG_LOG(MOD_GPS, "[EPO] file type err, file_type:%d, file_index:%d, start_addr:0x%x\n", file_type, file_index, g_epo_download_cntx.start_address);
        return 0;
    }
	*fHandle = offset + g_epo_download_cntx.start_address;
	if (*fHandle > g_epo_download_cntx.end_address)
	{
		printf("file size exceed quota");
		return 0;
	}
	*fHandle = (((file_type << 4) + file_index) << 24) + *fHandle;
    //EPO_DEBUG_LOG(MOD_GPS, "[EPO]fs_open fHandle:0x%x\n", fHandle);
	return (FS_HANDLE) fHandle;
}
void FS_Close(FS_HANDLE fHandle)
{
}

void
gnss_psdata_hex_dump(unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
#ifdef HEX_DUMP_ENABLE
  unsigned char *pt;
  int x;
  static char gnss_log_buffer[2048 + 2];
  char *str = gnss_log_buffer;
  pt = pSrcBufVA;
  printf("%s: %p, len = %d\n\r", str, pSrcBufVA, SrcBufLen);
  for (x=0; x<SrcBufLen; x++) {
    if (x % 16 == 0)
      printf("0x%04x : ", x);
    printf("%02x ", ((unsigned char)pt[x]));
    if (x%16 == 15) printf("\n\r");
  }
  printf("\n\r");
#endif
}

kal_int32 FS_Write(FS_HANDLE fHandle, kal_int8* buf, kal_int32 buf_len, kal_int32* write_len)
{
    uint32_t file_handle = *((uint32_t*) fHandle);
	epo_file_type file_type = file_handle >> 28;
	kal_int32 file_index = (file_handle >> 24) & 0x0F;
	kal_uint32 addr = file_handle & (0xFFFFFF);
	kal_uint32 file_base;
	
	file_base = get_file_base_addr(file_type, file_index);
	if ( epo_get_default_file_size(file_type) - (file_handle - file_base) < buf_len)
	{
	    EPO_DEBUG_LOG(MOD_GPS,"data exceed buf len\n");
		buf_len = epo_get_default_file_size(file_type) - (file_handle - file_base);
	}
    if (buf_len < 0) {
	    EPO_DEBUG_LOG(MOD_GPS,"file handle is invalid:%d\n", file_handle);
        return 0;
    }
    EPO_DEBUG_LOG(MOD_GPS,"EPO write,addr:0x%x, len:%d\n", addr, buf_len);
    gnss_psdata_hex_dump((unsigned char *) buf, (unsigned int) buf_len);
	hal_flash_write(addr, (uint8_t*) buf, buf_len);
	*write_len = buf_len;
    *((uint32_t*) fHandle) += buf_len;
	return 0;
}

kal_int32 FS_Read(FS_HANDLE fHandle, kal_int8* buf, kal_int32 buf_len, kal_int32* read_len)
{
    uint32_t file_handle = *((uint32_t*) fHandle);
	epo_file_type file_type = file_handle >> 28;
	kal_int32 file_index = (file_handle >> 24) & 0x0F;
	kal_uint32 addr = file_handle & (0xFFFFFF);
	kal_uint32 file_base;
    kal_int32 read_idx_left;
	
	file_base = get_file_base_addr(file_type, file_index);
    //EPO_DEBUG_LOG(MOD_GPS, "[EPO]file_base:%d, fHandle:%d, file_type:%d\n", file_base, fHandle, file_type);
    read_idx_left = epo_get_default_file_size(file_type) - (file_handle - file_base);
	if ( read_idx_left < buf_len)
	{
	    EPO_DEBUG_LOG(MOD_GPS, "read_idx_left:%d, buf_len:%d\n", read_idx_left, buf_len);
		buf_len = read_idx_left;
	}
    //EPO_DEBUG_LOG(MOD_GPS, "[EPO]fs_read fHandle:0x%x, addr:0x%x\n", fHandle, addr);
    if (buf_len < 0) {
        EPO_DEBUG_LOG(MOD_GPS, "[EPO]invalid fHandle:0x%x, addr:0x%x\n", file_handle, addr);
        return -1;
    }

	hal_flash_read((uint32_t) addr, (uint8_t*) buf, (uint32_t) buf_len);
    *((uint32_t*) fHandle) += buf_len;
	*read_len = buf_len;
	return 0;
}
kal_int32 FS_Seek(FS_HANDLE fHandle, kal_int32 offset, kal_int32 flag)
{
    uint32_t file_handle = *((uint32_t*) fHandle);
	epo_file_type file_type = file_handle >> 28;
	kal_int32 file_index = (file_handle >> 24) & 0x0F;
    EPO_DEBUG_LOG(MOD_GPS, "[EPO]fs_read fHandle:0x%x\n", file_handle);
	*((uint32_t*) fHandle) = get_file_base_addr(file_type, file_index) + offset;
	return 0;
}
kal_int32 FS_Delete(FS_HANDLE fHandle)
{
    uint32_t file_handle = *((uint32_t*) fHandle);
	epo_file_type file_type = file_handle >> 28;
	kal_int32 file_index = (file_handle >> 24) & 0x0F;
    int32_t filesize = epo_get_default_file_size(file_type);
    EPO_DEBUG_LOG(MOD_GPS,"[EPO]fs_delete file_handle:0x%x\n", file_handle);
	file_handle = get_file_base_addr(file_type, file_index);
    file_handle = file_handle & 0xFFFFFF;
    while (filesize > BLOCK_4K) {
        hal_flash_erase(file_handle, HAL_FLASH_BLOCK_4K);
        file_handle += BLOCK_4K;
        filesize -= BLOCK_4K;
    }
	//FS_Write(fHandle, 0, epo_get_default_file_size(file_type), (kal_int32*) &write_len);
	return 0;
}
kal_int32 epodownload_file_rename(epo_file_type file_type, kal_int32 file_index_old, kal_int32 file_index_new)
{
	return 0;
}
kal_int32 epodownload_file_delete(epo_file_type file_type, kal_int32 file_index)
{
    FS_Delete(epo_file_open(file_type, file_index, 0));
	return 0;
}
void epodownload_file_replace(epo_file_type file_type, kal_int32 file_index_old, kal_int32 file_index_new)
{
	return ;
}
#endif


int32_t epo_downlaod_get_curr_download_index()
{
    return g_epo_download_cntx.curr_download_chunk;
}

static int utc_to_gps_hour (kal_int32 iYr, kal_int32 iMo, kal_int32 iDay, kal_int32 iHr)
{
	kal_int32 iYearsElapsed; // Years since 1980
	kal_int32 iDaysElapsed; // Days elapsed since Jan 6, 1980
	kal_int32 iLeapDays; // Leap days since Jan 6, 1980
	kal_int32 i;
	// Number of days into the year at the start of each month (ignoring leap years)
	const kal_int32 doy[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"utc_to_gps_hour,%d:%d:%d:%d", iYr, iMo, iDay, iHr);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_UTC_TO_GPS_HOUR, iYr, iMo, iDay, iHr));
    
	iYearsElapsed = iYr - 1980;
	i = 0;
	iLeapDays = 0;
	while (i <= iYearsElapsed)
	{
		if ((i % 100) == 20)
		{
			if ((i % 400) == 20)
			{
				iLeapDays++;
			}
		}
		else if ((i % 4) == 0)
		{
			iLeapDays++;
		}
		i++;
	}
	if ((iYearsElapsed % 100) == 20)
	{
		if (((iYearsElapsed % 400) == 20) && (iMo <= 2))
		{
			iLeapDays--;
		}
	}
	else if (((iYearsElapsed % 4) == 0) && (iMo <= 2))
	{
		iLeapDays--;
	}
	iDaysElapsed = iYearsElapsed * 365 + doy[iMo - 1] + iDay + iLeapDays - 6;
	// Convert time to GPS weeks and seconds
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"gps_hour:%d", (iDaysElapsed * 24 + iHr));
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CURR_GPS_HOUR, (iDaysElapsed * 24 + iHr)));
    
	return (iDaysElapsed * 24 + iHr);
}

kal_int32 epo_get_max_gps_hours()
{
    kal_int32 file_index;
    kal_int32 read_len, result;
    FS_HANDLE fHandle;
    kal_int32 gps_hours, max_gps_hours = 0;    
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_get_max_gps_hours");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_GET_MAX_GPS_HOURS));
    
    for (file_index = 1; file_index <= g_epo_download_cntx.total_chunk; file_index++)
    {
        fHandle = epo_file_open(EPO_FILE_TYPE_3_DAY_EPO, file_index, FS_READ_ONLY);
        if (fHandle < 0)
        {
            continue;
        }
        result = FS_Read(fHandle, (kal_int8*) &gps_hours, sizeof(gps_hours), &read_len);

        FS_Close(fHandle);

        if (!(result == 0 && read_len == sizeof(gps_hours))) {
            continue;
        }
#ifndef FAT_FS
        if (gps_hours == 0xFFFFFFFF) {
            gps_hours = 0;
        }
#endif
		gps_hours = gps_hours & 0xFFFFFF;
        if (gps_hours >= max_gps_hours)
        {
            max_gps_hours = gps_hours;
        }
    }

    max_gps_hours += 72;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"max_gps_hours:%d", max_gps_hours);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_MAX_GPS_HOURS, max_gps_hours));
    

    return max_gps_hours;
}


kal_bool epo_check_file_expired()
{
    kal_int32 current_gps_hour;
    kal_int32 max_gps_hours;    
    applib_time_struct gm_time,rtc_time;
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_check_file_expired");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CHECK_FILE_EXPIRED));

    applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
	applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
	memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif
	current_gps_hour = utc_to_gps_hour(rtc_time.nYear, 
	                                    rtc_time.nMonth, 
	                                    rtc_time.nDay, 
	                                    rtc_time.nHour);
    
    max_gps_hours = epo_get_max_gps_hours();
    max_gps_hours -= EPO_EXPIRED_TIME;
    
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"current_gps_hour:%d, max_gps_hours:%d", current_gps_hour, max_gps_hours);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_GPS_HOURS, current_gps_hour, max_gps_hours));
    if (current_gps_hour >= max_gps_hours)
    {
        while (g_epo_download_cntx.start_address < g_epo_download_cntx.end_address) {
            hal_flash_erase(g_epo_download_cntx.start_address, HAL_FLASH_BLOCK_4K);
            g_epo_download_cntx.start_address += g_epo_download_cntx.block_size;
        }
        g_epo_download_cntx.start_address = 0x003F0000;
        return KAL_TRUE;
    }
    else
    {
        return KAL_FALSE;
    }
}


static kal_bool epo_check_download_finish(epo_http_hdlr_struct* connection)
{
    applib_md5_ctx md5_context;
	kal_uint8 hash_key[16]={0};
    kal_int32 i = 0;
	kal_uint8 hash_string[33]={0};
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_check_download_finish");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CHECK_DOWNLOAD_FINISH));
    EPO_DEBUG_LOG(MOD_GPS, "data len:%d, file size:%d\n", connection->sum, epo_get_default_file_size(connection->file_type));
    if (connection->sum >= epo_get_default_file_size(connection->file_type))
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"start check sum");
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_START_CHECK_SUM));
        if (connection->epo_file_handle < 0)
        {
            ASSERT(0);
        }
        FS_Seek(connection->epo_file_handle, 0, SEEK_SET);
        applib_md5_init(&md5_context);
        do
        {
            FS_Read(
                connection->epo_file_handle, 
                connection->recv_buf, 
                sizeof(connection->recv_buf),
                (kal_int32*) &connection->recv_buf_len);
            applib_md5_update(&md5_context, (const kal_uint8 *)connection->recv_buf, connection->recv_buf_len);
            //EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"md5 buf len:%d", connection->recv_buf_len);
            //vTaskDelay(10);
        } while (connection->recv_buf_len == sizeof(connection->recv_buf));
        applib_md5_final(hash_key, &md5_context);

        FS_Close(connection->epo_file_handle);
        connection->epo_file_handle = -1;

        while (i < 16)
        {
            sprintf((char*) hash_string + i*2,"%02x",hash_key[i]);
            i++;
        }
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"hash_string:%s", hash_string);
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"check_sum:%s", connection->check_sum);

        if (strcmp((char*) hash_string, (char*) connection->check_sum) == 0)
        {
            // download suceed
            EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"check sum OK");
            GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CHECK_SUM_OK));
            
            if (connection->file_type == EPO_FILE_TYPE_3_DAY_EPO)
            {
                epo_3_day_after_download_success_handle(connection);
            }
            else
            {
                epo_qepo_after_download_success_handle(connection);
                //aiding QEPO
            }
        }
        else
        {
            EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"check sum fail");
            GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CHECK_SUM_FAIL));
            epo_download_error_handle(connection, EPO_DOWNLOAD_ERROR_TYPE_FILE_CHECKSUM_ERROR);
        }
		return KAL_TRUE;
    }
	return KAL_FALSE;
}



/********************************send http request end*********************/


kal_uint32 epo_get_default_file_size(epo_file_type file_type)
{
    if (file_type == EPO_FILE_TYPE_3_DAY_EPO)
    {
    	if (g_epo_download_cntx.epo_mode == EPO_MODE_G_G)
		{
			return EPO_G_G_3_DAY_SIZE;
		}
		else if (g_epo_download_cntx.epo_mode == EPO_MODE_G)
		{
			return EPO_G_3_DAY_SIZE;
		}
    }
    else if (file_type == EPO_FILE_TYPE_QEPO)
    {
    	if (g_epo_download_cntx.epo_mode == EPO_MODE_G_G)
		{
			return EPO_G_G_Q_SIZE;
		}
		else if (g_epo_download_cntx.epo_mode == EPO_MODE_G)
		{
			return EPO_G_Q_SIZE;
		}
    }
    else
    {
        return 0xFFFFFFFF;
    }
	return 0xFFFFFFFF;
}


void epo_download_3_day_single_chunk()
{
    sio_set_mode(gprs_get_app_id(), SIO_DATA_TYPE_DATA);

    epo_download_start_timer(
        GPS_EPO_DOWNLOAD_3DAY_TIMEOUT_TIMER, 
        EPO_DOWNLOAD_TIMEOUT_TIMER_DURATION, 
        epo_download_timer_callback, 
        (void*) EPO_CURR_TIMER_TYPE_DOWNLOAD_TIMEOUT);
    httpc_download_start_download(EPO_FILE_TYPE_3_DAY_EPO, 
                        &g_epo_download_cntx.request_handle[EPO_FILE_TYPE_3_DAY_EPO],
                        epo_get_3_day_epo_download_url(g_epo_download_cntx.curr_download_chunk));

}

kal_bool epo_is_3_day_epo_downloading()
{
    return g_epo_download_cntx.state == STATE_EPO_3_DAY_DOWNLOAD || g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD;
}
kal_bool epo_is_download_fail_time_exceed_one_day()
{
    kal_int32 current_gps_hour;
    applib_time_struct gm_time, rtc_time;
    FS_HANDLE fHandle;
    kal_int32 result, gps_hours, read_len;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_is_download_fail_time_exceed_one_day"); 
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_IS_EXCEED_ONE_DAY));

    fHandle = epo_file_open(EPO_FILE_TYPE_3_DAY_EPO, 1, FS_READ_ONLY);
    if (fHandle < 0)
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"file open error:%d", fHandle);
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_OPEN_ERROR, fHandle));
        return KAL_TRUE;
    }
    result = FS_Read(fHandle, (kal_int8*) &gps_hours, sizeof(gps_hours), &read_len);
    FS_Close(fHandle);
    if (!(result == 0 && read_len == sizeof(gps_hours))) 
    {
        ASSERT(0);        
    }

    gps_hours = gps_hours & 0x00FFFFFF;
    
    applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
	applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
	memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif
	current_gps_hour = utc_to_gps_hour(rtc_time.nYear, 
	                                    rtc_time.nMonth, 
	                                    rtc_time.nDay, 
	                                    rtc_time.nHour);
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"current_gps_hour:%d, gps_hours:%d", current_gps_hour, gps_hours);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CURR_MAX_GPS_HOURS, current_gps_hour, gps_hours));
    if (current_gps_hour - gps_hours >= 23)
    {
        if (rtc_time.nMin < 30)
        {
            return KAL_FALSE;
        }
        return KAL_TRUE;
    }
    else
    {
        return KAL_FALSE;
    }

}

void epo_download_timer_callback(int32_t tid)
{
    epo_timer_type timer_type;
	int32_t i;
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_download_timer_callback,tid:%d\r\n", tid); 
	for (i = 0; i < GPS_EPO_TIME_MAX; i++)
	{
		if (g_epo_download_cntx.gps_epo_time_pool[i] == tid)
		{
			timer_type = g_epo_download_cntx.timeout_type[i];
			g_epo_download_cntx.gps_epo_time_pool[i] = -1;
            break;
		}
	}

	if (i >= GPS_EPO_TIME_MAX)
	{
		return;
	}
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"type:%d\r\n", timer_type); 
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_TIMER_CALLBACK, timer_type)); 
	
    switch(timer_type)
    {
        case EPO_CURR_TIMER_TYPE_RETRY_DOWNLOAD_TIMING:
            if (!epo_is_download_fail_time_exceed_one_day())
            {
                // if download fail on current day, keep retry download single chunk.
                g_epo_download_cntx.state = STATE_EPO_3_DAY_DOWNLOAD;
                epo_download_3_day_single_chunk();

            }
            else
            {
                epo_download_3_day();
            }
            break;
        case EPO_CURR_TIMER_TYPE_NEXT_DOWNLOAD_TIMING:
        case EPO_CURR_TIMER_TYPE_DELAY_DOWNLOAD_TIMING:
            epo_download_3_day();
            //epo_qepo_download();
            break;
        case EPO_CURR_TIMER_TYPE_CANCLE_AIDING:
            epo_aiding_cancle();
            if (g_epo_download_cntx.state == STATE_Q_EPO_DOWNLOAD
                || g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
            {
                epo_download_error_handle(
                    &g_epo_download_cntx.request_handle[EPO_FILE_TYPE_QEPO], 
                    EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_TIMEOUT);
            }
            break;
        case EPO_CURR_TIMER_TYPE_DOWNLOAD_TIMEOUT:
            if (g_epo_download_cntx.state == STATE_EPO_3_DAY_DOWNLOAD
                || g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
            {
                epo_download_error_handle(
                    &g_epo_download_cntx.request_handle[EPO_FILE_TYPE_3_DAY_EPO], 
                    EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_TIMEOUT);
            }
            break;
        default:
            break;
    }
}


void epo_download_current_trunk_finish_notify(epo_http_hdlr_struct *connection)
{
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_download_current_trunk_finish_notify\n");
	gnss_send_msg(GNSS_ENUM_EPO_DOWNLOAD_FINISH, 0, (void*) connection);
}

void epo_download_current_trunk_finish_handle(void *connection)
{
	if (!connection) {
		return;
	}
	if (!epo_check_download_finish((epo_http_hdlr_struct*) connection)) {
		epo_download_error_handle((epo_http_hdlr_struct*) connection, EPO_DOWNLOAD_ERROR_TYPE_CONNECTION_BROKEN);
	}
}

void epo_start_3_day_download_timer()
{
    kal_int32 current_gps_hour;
    kal_int32 max_gps_hours;   
    kal_uint32 expiry_time;
    applib_time_struct gm_time, rtc_time;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_start_3_day_download_timer");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_START_3_DAY_TIMER));

    applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
	applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
	memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif
	current_gps_hour = utc_to_gps_hour(rtc_time.nYear, 
	                                    rtc_time.nMonth, 
	                                    rtc_time.nDay, 
	                                    rtc_time.nHour);

    max_gps_hours = epo_get_max_gps_hours();

    expiry_time = (max_gps_hours - current_gps_hour) * 3600 - rtc_time.nMin * 60 - rtc_time.nSec;
    expiry_time -= (EPO_EXPIRED_TIME * 3600);

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"expiry_time:%d", expiry_time);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_EXPIRY_TIME, expiry_time));
    if (expiry_time > 0)
    {
    	// add for random download timing... for avoid all device download on same time.
    	expiry_time += rtc_time.nMin * 60 + rtc_time.nSec + ((expiry_time & 0x07) + (max_gps_hours & 0x03)) * 3600;
        expiry_time *= 1000;
        epo_download_start_timer(
            GPS_EPO_DOWNLOAD_TIMER,
            expiry_time, 
            epo_download_timer_callback, 
            (void*) EPO_CURR_TIMER_TYPE_NEXT_DOWNLOAD_TIMING);
    }
    else
    {
        ASSERT(0);
    }
}


void epo_download_3_day()
{
    applib_time_struct gm_time, rtc_time;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_download_3_day, state:%d", g_epo_download_cntx.state); 
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_3_DAY, g_epo_download_cntx.state)); 
	if (!g_epo_download_cntx.is_init)
	{
		return;
	}
    applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
	applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
	memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif
    if (rtc_time.nHour == 23 && rtc_time.nMin > 40)
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"delay download"); 
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_DELAY_DOWNLOAD));

        epo_download_start_timer(
            GPS_EPO_DOWNLOAD_TIMER,
            20*60*1000, 
            epo_download_timer_callback, 
            (void*) EPO_CURR_TIMER_TYPE_DELAY_DOWNLOAD_TIMING);        
    }
    
    if ( epo_is_3_day_epo_downloading() && g_epo_download_cntx.curr_download_chunk > 1)
    {
        epo_download_3_day_single_chunk();
    } 
    else if (epo_check_file_expired())
    {
        // yes
        // start download segment 1
        g_epo_download_cntx.curr_download_chunk = 1;
        if (g_epo_download_cntx.state == STATE_EPO_IDLE)
        {
            g_epo_download_cntx.state = STATE_EPO_3_DAY_DOWNLOAD;
        }
        else if (g_epo_download_cntx.state == STATE_Q_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_Q_AND_3_DAY_EPO_DOWNLOAD;
        }
        epo_download_3_day_single_chunk();
    }
    else
    {
        epo_start_3_day_download_timer();
    }
}

kal_int32 qepo_get_download_index()
{
    kal_int32 index;
    applib_time_struct gm_time, rtc_time;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"qepo_get_download_index");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_QEPO_GET_INDEX));
    applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
	applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
	memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif

    if (rtc_time.nHour >= 0 && rtc_time.nHour < 6)
    {
        index = 1;
    }
    else if (rtc_time.nHour >= 6 && rtc_time.nHour < 12)
    {
        index = 2;
    }
    else if (rtc_time.nHour >= 12 && rtc_time.nHour < 18)
    {
        index = 3;
    }
    else if (rtc_time.nHour >= 18)
    {
        index = 4;
    }

    if ((rtc_time.nHour == 0
        || rtc_time.nHour == 6
        || rtc_time.nHour == 12
        || rtc_time.nHour == 18)
        && rtc_time.nMin >= 0 && rtc_time.nMin < 5)
    {
        index--;
    }
    if (index == 0)
    {
        index = 4;
    }
    if (g_epo_download_cntx.using_qepo_backup_file)
    {
        index = 5;
    }
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"index = %d", index);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_INDEX, index));

    return index;
}

void epo_3_day_after_download_success_handle(epo_http_hdlr_struct* connection)
{
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_3_day_after_download_success_handle");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_DOWNLOAD_SUCCESS_HANDLE));
    epo_download_stop_timer(GPS_EPO_DOWNLOAD_3DAY_TIMEOUT_TIMER);
    epodownload_file_replace(
        connection->file_type, 
        g_epo_download_cntx.curr_download_chunk, 
        EPO_DEFAULT);
    g_epo_download_cntx.curr_download_chunk++;
    if (g_epo_download_cntx.curr_download_chunk <= g_epo_download_cntx.total_chunk)
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"download next chunk");
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_NEXT_CHUNK));
        epo_download_3_day_single_chunk();
    }
    else
    {
        // all chunk is download finish.
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"download finish");
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FINISH));
        connection->retry_times = 0;
        g_epo_download_cntx.curr_download_chunk = -1;
        if (g_epo_download_cntx.state == STATE_EPO_3_DAY_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_EPO_IDLE;
        }
        else if (g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_Q_EPO_DOWNLOAD;
        }
        epo_start_3_day_download_timer();
    }
}

void epo_qepo_after_download_success_handle(epo_http_hdlr_struct* connection)
{
    kal_bool is_aiding_success;
    // aiding EPO
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_qepo_after_download_success_handle");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_QEPO_DOWNLOAD_SUCESS_HANDLE));
    is_aiding_success = epo_try_qepo_aiding_ext();

    // check if need download again. if yes
    if (qepo_get_download_index() == 4 && !is_aiding_success)
    {
        g_epo_download_cntx.using_qepo_backup_file = KAL_TRUE;
        httpc_download_start_download(EPO_FILE_TYPE_QEPO, 
                            &g_epo_download_cntx.request_handle[EPO_FILE_TYPE_QEPO],
                            epo_get_qepo_download_url(qepo_get_download_index()));
    }
    else
    {
        //time is error, cannot get right EPO.
        if (g_epo_download_cntx.state == STATE_Q_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_EPO_IDLE;
        }
        else if (g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
        {
            g_epo_download_cntx.state = STATE_EPO_3_DAY_DOWNLOAD;
        }
        epo_download_stop_timer(GPS_EPO_DOWNLOAD_QEOP_TIMEOUT_TIMER);
        g_epo_download_cntx.using_qepo_backup_file = KAL_FALSE;
        return;
    }
}

void epo_qepo_download()
{
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_qepo_download, state:%d", g_epo_download_cntx.state);
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_QEPO_DOWNLOAD, g_epo_download_cntx.state));
	
	if (!g_epo_download_cntx.is_init)
	{
		return;
	}

    if (g_epo_download_cntx.state == STATE_Q_EPO_DOWNLOAD
        || g_epo_download_cntx.state == STATE_Q_AND_3_DAY_EPO_DOWNLOAD)
    {
        return;
    }

    if (g_epo_download_cntx.state == STATE_EPO_IDLE)
    {
        g_epo_download_cntx.state = STATE_Q_EPO_DOWNLOAD;
    }
    else if (g_epo_download_cntx.state == STATE_EPO_3_DAY_DOWNLOAD)
    {
        g_epo_download_cntx.state = STATE_Q_AND_3_DAY_EPO_DOWNLOAD;
    }
    httpc_download_start_download(EPO_FILE_TYPE_QEPO, 
                        &g_epo_download_cntx.request_handle[EPO_FILE_TYPE_QEPO],
                        epo_get_qepo_download_url(qepo_get_download_index()));
}

void epo_delete_all()
{
    kal_int32 file_index;
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_delete_all");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_DELETE_ALL));
    for (file_index = 1; file_index <= EPO_MAX_CHUNK; file_index++)
    {
        epodownload_file_delete(EPO_FILE_TYPE_3_DAY_EPO, file_index);
    }
    epodownload_file_delete(EPO_FILE_TYPE_QEPO, EPO_DEFAULT);
}


void epo_aiding_internal(FS_HANDLE file_handle)
{
    kal_int32 result, read_len;
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_EPO_AIDING_INTERNAL));
    result = FS_Read(file_handle, 
        (kal_int8*) g_epo_download_cntx.u4EPOWORD, 
        sizeof(g_epo_download_cntx.u4EPOWORD),
        &read_len);
    if (result == 0 && read_len == sizeof(g_epo_download_cntx.u4EPOWORD))
    {
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_CURR_SEG, g_epo_download_cntx.u4EPOWORD[0] & 0x00FFFFFF));
        gps_timer_epo_send_assistance_data();
        g_epo_download_cntx.need_aiding = KAL_FALSE;
    }
    else
    {
        
    }
}

void epo_aiding_cancle()
{
    g_epo_download_cntx.need_aiding = KAL_FALSE;
    g_epo_download_cntx.gps_aiding_hours = 0;
    memset(g_epo_download_cntx.u4EPOWORD, 0, sizeof(g_epo_download_cntx.u4EPOWORD));
}

kal_uint32* epo_get_single_sv_data(kal_int32 num_sv)
{
    return g_epo_download_cntx.u4EPOWORD + (num_sv * 18);
}

kal_bool epo_try_3day_epo_aiding(kal_uint32 chip_gps_hours)
{
    kal_int32 current_gps_hour;
    applib_time_struct gm_time, rtc_time;
    FS_HANDLE fHandle;
    kal_int32 result, gps_hours, read_len, segment;
    kal_int32 file_index = 1;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_try_3day_epo_aiding");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_TRY_3_DAY_EPO_AIDING));
    
    if (chip_gps_hours == 0)
    {
        applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
		applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
		memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif
        current_gps_hour = utc_to_gps_hour(rtc_time.nYear, 
                                            rtc_time.nMonth, 
                                            rtc_time.nDay, 
                                            rtc_time.nHour);
    }
    else
    {
        current_gps_hour = chip_gps_hours; 
    }

    do {
        fHandle = epo_file_open(EPO_FILE_TYPE_3_DAY_EPO, file_index, FS_READ_ONLY);

        if (fHandle < 0)
        {
            return KAL_FALSE;        
        }

        result = FS_Read(fHandle, (kal_int8*) &gps_hours, sizeof(gps_hours), &read_len);
        if (!(result == 0 && read_len == sizeof(gps_hours))) 
        {
            ASSERT(0);        
        }

        gps_hours = gps_hours & 0x00FFFFFF;
        if (current_gps_hour - gps_hours >= 72)
        {
            file_index += (current_gps_hour - gps_hours)/72;
            FS_Close(fHandle);
        }
        else
        {
            break;
        }
    }while ( file_index <= EPO_MAX_CHUNK);

    if (file_index > EPO_MAX_CHUNK || current_gps_hour < gps_hours)
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"aiding fail");
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_AIDING_FAIL));
 		if (fHandle >= 0)
		{
			FS_Close(fHandle);
		}
       return KAL_FALSE;
    }

    segment = (current_gps_hour - gps_hours) / 6;
	FS_Seek(fHandle, segment*(EPO_RECORD_SIZE)*(EPO_SV_NUMBER), SEEK_SET);
    epo_aiding_internal(fHandle);
    FS_Close(fHandle);
    
    return KAL_TRUE;
}

kal_bool epo_try_qepo_aiding_ext()
{
    if (g_epo_download_cntx.need_aiding)
    {
        g_epo_download_cntx.need_aiding = KAL_FALSE;
        return epo_try_qepo_aiding(g_epo_download_cntx.gps_aiding_hours);
    }
    return KAL_TRUE;
}

kal_bool epo_try_qepo_aiding(kal_uint32 chip_gps_hours)
{
    FS_HANDLE fHandle;
    kal_int32 result, gps_hours, read_len;
    kal_int32 current_gps_hour;
    applib_time_struct gm_time, rtc_time;
    kal_bool aiding_ret;

    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_try_qepo_aiding");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_TRY_QEPO_AIDING));

    if (chip_gps_hours == 0)
    {
        applib_dt_get_rtc_time(&gm_time);
#if !defined(EXTERNAL_MMI) && !defined(EMPTY_MMI) && !defined(__IOT__)
        applib_dt_rtc_to_utc_with_default_tz(&gm_time, &rtc_time);
#else
        memcpy(&rtc_time, &gm_time, sizeof(rtc_time));
#endif
        current_gps_hour = utc_to_gps_hour(rtc_time.nYear, 
                                            rtc_time.nMonth, 
                                            rtc_time.nDay, 
                                            rtc_time.nHour);
    }
    else
    {
        current_gps_hour = chip_gps_hours; 
    }

    fHandle = epo_file_open(EPO_FILE_TYPE_QEPO, EPO_DEFAULT, FS_READ_ONLY);
    
    if (fHandle < 0)
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"file open fail");
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_FILE_OPEN_FAIL));
        return KAL_FALSE;        
    }
    
    result = FS_Read(fHandle, (kal_int8*) &gps_hours, sizeof(gps_hours), &read_len);

    if (!(result == 0 && read_len == sizeof(gps_hours))) 
    {
        ASSERT(0);        
    }

    gps_hours = gps_hours & 0x00FFFFFF;

    if (gps_hours/6 == current_gps_hour/6)
    {
        FS_Seek(fHandle, 0, SEEK_SET);
        epo_aiding_internal(fHandle);
        aiding_ret = KAL_TRUE;
    }
    else
    {
        EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"aiding fail");
        GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_AIDING_FAIL));
        aiding_ret = KAL_FALSE;        
    }
    FS_Close(fHandle);
    return aiding_ret;
}


kal_bool epo_aiding(kal_uint32 chip_gps_hours)
{
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_aiding");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_EPO_AIDING));

	if (!g_epo_download_cntx.is_init)
	{
		return KAL_FALSE;
	}
    g_epo_download_cntx.gps_aiding_hours = chip_gps_hours;
    g_epo_download_cntx.need_aiding = KAL_TRUE;

    if (g_epo_download_cntx.state == STATE_EPO_IDLE && epo_try_3day_epo_aiding(chip_gps_hours))
    {
        //we have available epo. do nothing. 
        return KAL_TRUE;
    }
    else if (!epo_try_qepo_aiding(chip_gps_hours))
    {
        epo_download_start_timer(
            GPS_EPO_DOWNLOAD_QEOP_TIMEOUT_TIMER,
            60000, 
            epo_download_timer_callback, 
            (void*) EPO_CURR_TIMER_TYPE_CANCLE_AIDING);
        //epo_delete_all();
        epo_qepo_download();
        return KAL_FALSE;
    }
    return KAL_TRUE;
}


void epo_init(kal_int32 trunk_num, char* epo_server_name, char* query_string)
{
    EPO_DEBUG_LOG(MOD_GPS, PRE_FIX"epo_init");
    GPS_TRACE((TRACE_GROUP_6, GPS_TRC_EPO_DOWNLOAD_EPO_INIT));
	
	if (g_epo_download_cntx.is_init)
	{
		return;
	}

    g_epo_download_cntx.is_init = KAL_TRUE;
    applib_dt_init_time();
	
	g_epo_download_cntx.epo_mode = EPO_MODE_G;
    g_epo_download_cntx.curr_download_chunk = -1;
    g_epo_download_cntx.state = STATE_EPO_IDLE;
    g_epo_download_cntx.total_chunk = trunk_num;
    g_epo_download_cntx.request_handle[EPO_FILE_TYPE_3_DAY_EPO].file_type = EPO_FILE_TYPE_3_DAY_EPO;
    g_epo_download_cntx.request_handle[EPO_FILE_TYPE_QEPO].file_type = EPO_FILE_TYPE_QEPO;
    g_epo_download_cntx.request_handle[EPO_FILE_TYPE_3_DAY_EPO].epo_file_handle = -1;
    g_epo_download_cntx.request_handle[EPO_FILE_TYPE_QEPO].epo_file_handle = -1;
    g_epo_download_cntx.request_handle[EPO_FILE_TYPE_3_DAY_EPO].retry_times = 0;
    g_epo_download_cntx.using_qepo_backup_file = KAL_FALSE;
    g_epo_download_cntx.need_aiding = KAL_FALSE;
	if (epo_server_name)
	{
		g_epo_download_cntx.epo_server_name = epo_server_name;
		g_epo_download_cntx.query_string = query_string;
	}
	else
	{
		g_epo_download_cntx.epo_server_name = EPO_DOWNLOAD_SERVER_NAME;
	}

#ifndef FAT_FS
	
		g_epo_download_cntx.start_address = 0x003F0000;
		g_epo_download_cntx.end_address = 0x00400000;
	
		g_epo_download_cntx.block_size = 4*1024;
		g_epo_download_cntx.ubin_pack_count = HAL_FLASH_BLOCK_4K;
		g_epo_download_cntx.block_type = 16;
#endif
	memset(g_epo_download_cntx.gps_epo_time_pool, 0xff, sizeof(g_epo_download_cntx.gps_epo_time_pool));
    httpc_download_init();
    #if 1
    epo_download_start_timer(
        GPS_EPO_DOWNLOAD_TIMER,
        2000, 
        epo_download_timer_callback, 
        (void*) EPO_CURR_TIMER_TYPE_NEXT_DOWNLOAD_TIMING);
    #endif
	//FS_CreateDir(L"Z:\\@EPO");
}

void epo_deinit()
{
	EPO_DEBUG_LOG(MOD_GPS, "[EPO]epo_deinit");
	if (!g_epo_download_cntx.is_init)
	{
		return;
	}
	epo_download_finish_connection(&g_epo_download_cntx.request_handle[EPO_FILE_TYPE_QEPO]);
	epo_download_finish_connection(&g_epo_download_cntx.request_handle[EPO_FILE_TYPE_3_DAY_EPO]);
	epo_aiding_cancle();
	epo_download_stop_timer(GPS_EPO_DOWNLOAD_QEOP_TIMEOUT_TIMER);
	epo_download_stop_timer(GPS_EPO_DOWNLOAD_TIMER);
	epo_download_stop_timer(GPS_EPO_DOWNLOAD_3DAY_TIMEOUT_TIMER);
	memset(&g_epo_download_cntx, 0, sizeof(g_epo_download_cntx));
}



void applib_md5_init(applib_md5_ctx *mdContext)
{
    mbedtls_md5_init((mbedtls_md5_context*)mdContext);
    mbedtls_md5_starts((mbedtls_md5_context*)mdContext);
}
void applib_md5_update(applib_md5_ctx *mdContext, const kal_uint8 *inBuf, kal_uint32 inLen)
{
    mbedtls_md5_update((mbedtls_md5_context*)mdContext, inBuf, inLen);
}
void applib_md5_final(kal_uint8 hash[], applib_md5_ctx *mdContext)
{
    mbedtls_md5_finish((mbedtls_md5_context*)mdContext, hash);
    mbedtls_md5_free((mbedtls_md5_context*)mdContext);
}

#endif

