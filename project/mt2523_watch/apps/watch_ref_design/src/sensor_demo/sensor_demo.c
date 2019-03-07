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

#include "sensor_demo.h"

#ifdef MT2523_WATCH_SENSOR_CALIBRATION
#include "sensor_transfer_info.h"
#endif

#include "nvdm.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include <string.h>

#ifdef SENSOR_DEMO
/* syslog */
#define SENSOR_DEMO_LOGI(fmt,...) LOG_I(sensor,  (fmt), ##__VA_ARGS__)
/* printf*/
//#define SENSOR_DEMO_LOGI(fmt,arg...)   printf("SENSOR_DEMO:INFO: "fmt,##arg)
#endif

#ifdef SENSOR_DEMO

#include "hal_eint.h"
#include "hal_i2c_master.h"
#include "hal_spi_master.h"

/* sensor subsys includes */
#include "mems_bus.h"
#include "sensor_alg_interface.h"

#ifdef MTK_SENSOR_ACCELEROMETER_GYROSCOPE_USE_BMI160
#include "bmi160.h"
#endif

#ifdef MTK_SENSOR_ACCELEROMETER_USE_BMA255
#include "bma255_sensor_adaptor.h"
#endif

#ifdef MTK_SENSOR_PROXIMITY_USE_CM36672
#include "cm36672.h"
#endif

int32_t all_sensor_send_digest_callback(sensor_data_t *const output);
#ifdef FUSION_PEDOMETER_USE
sensor_subscriber_t pedometer_subscriber = {
    "ap12", 0, SENSOR_TYPE_PEDOMETER, 0, all_sensor_send_digest_callback, 0
};
uint32_t pedometer_triggered;
#endif

#ifdef FUSION_HEART_RATE_MONITOR_USE
sensor_subscriber_t heart_rate_monitor_subscriber = {
    "ap21", 0, SENSOR_TYPE_HEART_RATE_MONITOR, 0, all_sensor_send_digest_callback, 0
};
uint32_t hr_triggered;
#endif

#ifdef FUSION_BLOOD_PRESSURE_USE
static sensor_subscriber_t blood_pressure_subscriber = {
    "ap23", 0, SENSOR_TYPE_BLOOD_PRESSURE_MONITOR, 0, all_sensor_send_digest_callback, 0
};
static uint32_t bp_triggered;
/*pwtt transmission start */
static int32_t pwtt_seq;
static int32_t apwtt_data[12];
/*pwtt transmission end */
#endif

static int ppg1_buff_count = 0;
static uint32_t ppg2_buff_data;
static uint32_t ppg2_buff_timestamp;
static int ppg2_buff_count = 0;
static int ekg_buff_count = 0;

static int32_t ppg1_buff_data_array[12];
static int32_t ekg_buff_data_array[12];
static int32_t acc_buff_data_array[12];
static int32_t ppg1_seq = 1;
static int32_t ekg_seq = 1;
static int32_t acc_seq = 1;
static int acc_buff_count = 0;
static int32_t ppg1_amb_flag = 1;


//static uint32_t acc_count;

#include "data_transfer.h"

#define DATA_TRANSFER_LENGTH (16)

int send_sensor_data_via_btspp(int32_t magic, int32_t sensor_type, int32_t x, int32_t y, int32_t z, int32_t status, int32_t time_stamp)
{
    int ret = 0;
    uint32_t i = 0;
    int32_t data_transfer_buffer[DATA_TRANSFER_LENGTH];

    data_transfer_buffer[0] = magic;
    data_transfer_buffer[1] = sensor_type;
    data_transfer_buffer[2] = 0;//seq
    data_transfer_buffer[3] = x;
    data_transfer_buffer[4] = y;
    data_transfer_buffer[5] = z;
    data_transfer_buffer[6] = status;
    data_transfer_buffer[7] = time_stamp;
    for (i = 8; i < 16; i++) {
        data_transfer_buffer[i] = 12345;//reserved
    }

    if ((SENSOR_VIRTUAL_TYPE_START<=sensor_type) && (SENSOR_TYPE_ALL>sensor_type)) {
        /* virtual sensor (fusion), send via update dt API for BLE only case.*/
        ret = dt_update_data((uint8_t*)data_transfer_buffer, DATA_TRANSFER_LENGTH*4);
        if ((ret < 0) && (ret != DT_STATUS_DISCONNECT)) {
            SENSOR_DEMO_LOGI("dt_update_data err (%d) \r\n", ret);
        }
    } else {
        ret = dt_send_data((uint8_t*)data_transfer_buffer, DATA_TRANSFER_LENGTH*4);
        if ((ret < 0) && (ret != DT_STATUS_DISCONNECT)) {
            SENSOR_DEMO_LOGI("dt_send_data err (%d) \r\n", ret);
        }
    }

    return ret;
}

int send_sensor_16_datas_via_btspp(int32_t magic, int32_t sensor_type, int32_t seq, int32_t data_buf[], int32_t data_size, int32_t reserve)
{
    int ret = 0;
    uint32_t transfer_length = 0;
    uint32_t i = 0;
    int32_t data_transfer_buffer[DATA_TRANSFER_LENGTH];

    data_transfer_buffer[0] = magic;
    data_transfer_buffer[1] = sensor_type;
    data_transfer_buffer[2] = seq;

    transfer_length = data_size;
    if (transfer_length > 12) {
        transfer_length = 12;
    }

    for (i = 0; i < transfer_length; i++) {
        data_transfer_buffer[3+i] = data_buf[i];
    }
    data_transfer_buffer[15] = reserve;

    ret = dt_send_data((uint8_t*)data_transfer_buffer, DATA_TRANSFER_LENGTH*4);
    if ((ret < 0) && (ret != DT_STATUS_DISCONNECT)) {
        SENSOR_DEMO_LOGI("dt_send_data err (%d) \r\n", ret);
    }

    return ret;
}

int32_t all_sensor_send_digest_callback(sensor_data_t *const output)
{
    switch (output->data[0].sensor_type) {
        case SENSOR_TYPE_ACCELEROMETER:
#if 0
            if ((acc_count++) % 400 == 0) {
                SENSOR_DEMO_LOGI("type = %lu , value = ( %ld , %ld , %ld )(mm/s^2) , timestampe = %lu \r\n",
                                 output->data[0].sensor_type,
                                 output->data[0].accelerometer_t.x,
                                 output->data[0].accelerometer_t.y,
                                 output->data[0].accelerometer_t.z,
                                 output->data[0].time_stamp);
            }
#endif
            if ((acc_buff_count + 4) <= 12) {
                acc_buff_data_array[acc_buff_count]  = output->data[0].accelerometer_t.x;
                acc_buff_data_array[acc_buff_count + 1]  = output->data[0].accelerometer_t.y;
                acc_buff_data_array[acc_buff_count + 2]  = output->data[0].accelerometer_t.z;
                acc_buff_data_array[acc_buff_count + 3]  = output->data[0].time_stamp;
                acc_buff_count += 4;
            } else {
                send_sensor_16_datas_via_btspp(
                    SENSOR_SPP_DATA_MAGIC,
                    SENSOR_TYPE_ACCELEROMETER,
                    acc_seq,
                    acc_buff_data_array,
                    12,
                    SENSOR_SPP_DATA_RESERVED);
                acc_seq++;
                acc_buff_count = 0;
            }
            break;

        case SENSOR_TYPE_BIOSENSOR_PPG1_512HZ:
            if (ppg1_amb_flag == 1) {
                // SENSOR_TYPE_BIOSENSOR_PPG1_512HZ only sample ppg , no amb
                if (ppg1_buff_count < 12) {
                    ppg1_buff_data_array[ppg1_buff_count] = output->data[0].bio_data.data;
                    ppg1_buff_count++;
                }
                if (ppg1_buff_count == 12) {
                    send_sensor_16_datas_via_btspp(
                        SENSOR_SPP_DATA_MAGIC,
                        SENSOR_TYPE_BIOSENSOR_PPG1_512HZ,
                        ppg1_seq,
                        ppg1_buff_data_array,
                        12,
                        SENSOR_SPP_DATA_RESERVED);
                    ppg1_seq++;
                    ppg1_buff_count = 0;
                }
                ppg1_amb_flag = 0;
            } else {
                ppg1_amb_flag = 1;
            }

            break;

        case SENSOR_TYPE_BIOSENSOR_PPG1:
            if (ppg1_buff_count < 12) {
                ppg1_buff_data_array[ppg1_buff_count] = output->data[0].bio_data.data;
                ppg1_buff_count++;
            }
            if (ppg1_buff_count == 12) {
                send_sensor_16_datas_via_btspp(
                    SENSOR_SPP_DATA_MAGIC,
                    SENSOR_TYPE_BIOSENSOR_PPG1,
                    ppg1_seq,
                    ppg1_buff_data_array,
                    12,
                    SENSOR_SPP_DATA_RESERVED);
                ppg1_seq++;
                ppg1_buff_count = 0;
            }
            break;
        case SENSOR_TYPE_BIOSENSOR_PPG2:
            if (ppg2_buff_count == 0) {
                ppg2_buff_data = output->data[0].bio_data.data;
                ppg2_buff_timestamp = output->data[0].time_stamp;
                ppg2_buff_count++;
            } else {
                send_sensor_data_via_btspp(
                    SENSOR_SPP_DATA_MAGIC,
                    SENSOR_TYPE_BIOSENSOR_PPG2,
                    ppg2_buff_data,
                    ppg2_buff_timestamp,
                    output->data[0].bio_data.data,
                    output->data[0].time_stamp,
                    SENSOR_SPP_DATA_RESERVED);
                ppg2_buff_count = 0;
            }
            break;

        case SENSOR_TYPE_BIOSENSOR_BISI:
            send_sensor_data_via_btspp(
                SENSOR_SPP_DATA_MAGIC,
                SENSOR_TYPE_BIOSENSOR_BISI,
                output->data[0].bio_data.data,
                output->data[0].time_stamp,
                SENSOR_SPP_DATA_RESERVED,
                SENSOR_SPP_DATA_RESERVED,
                SENSOR_SPP_DATA_RESERVED);
            break;
        case SENSOR_TYPE_BIOSENSOR_EKG:
            if (ekg_buff_count < 12) {
                ekg_buff_data_array[ekg_buff_count] = output->data[0].bio_data.data;
                ekg_buff_count++;
            }
            if (ekg_buff_count == 12) {
                send_sensor_16_datas_via_btspp(
                    SENSOR_SPP_DATA_MAGIC,
                    SENSOR_TYPE_BIOSENSOR_EKG,
                    ekg_seq,
                    ekg_buff_data_array,
                    12,
                    SENSOR_SPP_DATA_RESERVED);
                ekg_seq++;
                ekg_buff_count = 0;
            }
            break;

        case SENSOR_TYPE_PEDOMETER:
            SENSOR_DEMO_LOGI("Step count = %ld , timestampe = %lu \r\n",
                             output->data[0].pedometer_t.accumulated_step_count,
                             output->data[0].time_stamp);

            update_step_count_data(output->data[0].pedometer_t.accumulated_step_count);
            break;

        case SENSOR_TYPE_HEART_RATE_MONITOR:
#if 1
            SENSOR_DEMO_LOGI("Heart rate , bpm = %ld , timestampe = %lu \r\n",
                             output->data[0].heart_rate_t.bpm,
                             output->data[0].time_stamp);

            send_sensor_data_via_btspp(
                SENSOR_SPP_DATA_MAGIC,
                SENSOR_TYPE_HEART_RATE_MONITOR,
                output->data[0].heart_rate_t.bpm,
                output->data[0].heart_rate_t.status,
                output->data[0].time_stamp,
                SENSOR_SPP_DATA_RESERVED,
                SENSOR_SPP_DATA_RESERVED);

            update_hr_data(output->data[0].heart_rate_t.bpm, output->data[0].heart_rate_t.status);

#endif
            break;

        case SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR: {
            SENSOR_DEMO_LOGI("Heart rate variability: T(%ld), SDNN(%ld), LF(%ld), HF(%ld), LF_HF(%ld)\r\n", output->data[0].time_stamp, output->data[0].heart_rate_variability_t.SDNN,
                             output->data[0].heart_rate_variability_t.LF, output->data[0].heart_rate_variability_t.HF, output->data[0].heart_rate_variability_t.LF_HF);

            send_sensor_data_via_btspp(
                SENSOR_SPP_DATA_MAGIC,
                SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR-1,
                output->data[0].heart_rate_variability_t.SDNN,
                output->data[0].heart_rate_variability_t.LF,
                output->data[0].heart_rate_variability_t.HF,
                output->data[0].heart_rate_variability_t.LF_HF,
                output->data[0].time_stamp
            );

            /* To unsubscribe here? */
#ifdef FUSION_HEART_RATE_VARIABILITY_USE
            int32_t unsubscription;

            unsubscription = sensor_unsubscribe_sensor(heart_rate_variability_subscriber.handle);
            if (unsubscription < 0) {
                SENSOR_DEMO_LOGI("HRV unsubscription fail\r\n");
            } else {
                hrv_triggered = 0;
                SENSOR_DEMO_LOGI("HRV unsubscribed\r\n");
            }
#endif
            break;
        }

        case SENSOR_TYPE_BLOOD_PRESSURE_MONITOR: {
            if (output->data[0].blood_pressure_t.pwtt == NULL) {
                SENSOR_DEMO_LOGI("B.P: T(%ld), sdp(%ld), dbp(%ld), status(%ld) \r\n", output->data[0].time_stamp, output->data[0].blood_pressure_t.sbp,
                                 output->data[0].blood_pressure_t.dbp, output->data[0].blood_pressure_t.status);

                send_sensor_data_via_btspp(
                    SENSOR_SPP_DATA_MAGIC,
                    SENSOR_TYPE_BLOOD_PRESSURE_MONITOR,
                    output->data[0].blood_pressure_t.sbp,
                    output->data[0].blood_pressure_t.dbp,
                    output->data[0].blood_pressure_t.bpm,
                    output->data[0].blood_pressure_t.status,
                    output->data[0].time_stamp
                );
                /* To unsubscribe here? */
#ifdef FUSION_BLOOD_PRESSURE_USE
#ifdef  MT2523_WATCH_SENSOR_CALIBRATION
#define BP_CALIBRATION_MODE 1023
                nvdm_status_t nvdm_status;
                uint32_t nvram_num = sizeof(nvram_ef_bp_info_struct);
                nvram_ef_bp_info_struct bp_info;
                nvdm_status = nvdm_read_data_item("2511","bpInfo",(uint8_t *)&bp_info, &nvram_num);
                if(nvdm_status != NVDM_STATUS_OK) {
                    LOG_I(hal,"read nvdm mode fail\n");
                } else {
                    if (bp_info.bp_mode == BP_CALIBRATION_MODE) {
                        bp_info.bp_mode = 0;
                        nvdm_write_data_item("2511","bpInfo",NVDM_DATA_ITEM_TYPE_RAW_DATA,(const uint8_t *)&bp_info, nvram_num);
                    }
                }
#endif
                int32_t unsubscription;

                unsubscription = sensor_unsubscribe_sensor(blood_pressure_subscriber.handle);
                if (unsubscription < 0) {
                    SENSOR_DEMO_LOGI("B.P. unsubscription fail\r\n");
                } else {
                    bp_triggered = 0;
                    SENSOR_DEMO_LOGI("B.P. unsubscribed\r\n");
                }
#endif

#ifdef FUSION_HEART_RATE_MONITOR_USE
                int32_t subscription;

                if (hr_triggered == 2) {
                    subscription = sensor_subscribe_sensor(&heart_rate_monitor_subscriber);
                    if (subscription < 0) {
                        SENSOR_DEMO_LOGI("HR subscription fail\r\n");
                    } else {
                        hr_triggered = 1;
                        SENSOR_DEMO_LOGI("HR subscribed\r\n");
                    }
                }
#endif
                update_bp_data(output->data[0].blood_pressure_t.sbp, output->data[0].blood_pressure_t.dbp, output->data[0].blood_pressure_t.status);
            } else { /* pwtt data*/
                uint32_t bunch_count = 0;
                uint32_t i = 0;

                memset(apwtt_data, 0, sizeof(int32_t));
                //apwtt_data[0] = 0; /* feature type*/
                apwtt_data[0] = output->data[0].blood_pressure_t.feature_type;
                apwtt_data[1] = output->data[0].blood_pressure_t.status; /* status*/
                apwtt_data[2] = output->data[0].time_stamp; /* timestamp */
                bunch_count = output->data[0].blood_pressure_t.numPwtt;
                if (bunch_count > 9) {
                    bunch_count = 9; /* maximum data  to transmit is 9 */
                }
                i = 0;
                while (i < bunch_count) {
                    apwtt_data[3 + i] = output->data[0].blood_pressure_t.pwtt[i];
                    i++;
                }
                SENSOR_DEMO_LOGI("pwtt (%d) in B.P. %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", output->data[0].blood_pressure_t.numPwtt, apwtt_data[0],
                                 apwtt_data[1], apwtt_data[2], apwtt_data[3], apwtt_data[4], apwtt_data[5], apwtt_data[6],
                                 apwtt_data[7], apwtt_data[8], apwtt_data[9], apwtt_data[10], apwtt_data[11]);
                send_sensor_16_datas_via_btspp(
                    SENSOR_SPP_DATA_MAGIC,
                    SENSOR_TYPE_INTERNAL_PWTT,
                    pwtt_seq,
                    apwtt_data,
                    12,
                    SENSOR_SPP_DATA_RESERVED);
                pwtt_seq++;
            }

            break;
        }

        default:
            SENSOR_DEMO_LOGI("type = %lu , value = ( %ld %ld %ld ) , timestamp = %lu \r\n",
                             output->data[0].sensor_type,
                             output->data[0].value[0],
                             output->data[0].value[1],
                             output->data[0].value[2],
                             output->data[0].time_stamp);
            break;
    }

    return 0;
}

void enable_hr(void)
{
    int32_t subscription = 0;

    (void)subscription;

#ifdef FUSION_HEART_RATE_MONITOR_USE
    if (hr_triggered == 0) {
        subscription = sensor_subscribe_sensor(&heart_rate_monitor_subscriber);
        if (subscription < 0) {
            SENSOR_DEMO_LOGI("HR subscription fail\r\n");
        } else {
            /* reset bt buffer index of input start */
            acc_buff_count = 0;
            ppg1_buff_count = 0;
            /* reset bt buffer index of input end */
            SENSOR_DEMO_LOGI("HR triggered\r\n");
            hr_triggered = 1;
        }
    }
#endif


}

void disable_hr(void)
{
    int32_t unsubscription = 0;

    (void)unsubscription;

#ifdef FUSION_HEART_RATE_MONITOR_USE
    if (hr_triggered == 1) {
        unsubscription = sensor_unsubscribe_sensor(heart_rate_monitor_subscriber.handle);
        if (unsubscription < 0) {
            SENSOR_DEMO_LOGI("HR unsubscription fail\r\b");
        } else {
            hr_triggered = 0;
            SENSOR_DEMO_LOGI("HR unsubscribed\r\n");
        }
    }
#endif
}

void enable_step_counter(void)
{
    int32_t subscription = 0;

    (void)subscription;

#ifdef FUSION_PEDOMETER_USE
    if (pedometer_triggered == 0) {
        subscription = sensor_subscribe_sensor(&pedometer_subscriber);
        if (subscription < 0) {
            SENSOR_DEMO_LOGI("Pedometer subscription fail\r\n");
        } else {
            /* reset bt buffer index of input start */
            acc_buff_count = 0;
            /* reset bt buffer index of input end */
            SENSOR_DEMO_LOGI("Pedometer triggered\r\n");
            pedometer_triggered = 1;
        }
    }
#endif

}

void disable_step_counter(void)
{
    int32_t unsubscription = 0;

    (void)unsubscription;

#ifdef FUSION_PEDOMETER_USE
    if (pedometer_triggered == 1) {
        unsubscription = sensor_unsubscribe_sensor(pedometer_subscriber.handle);
        if (unsubscription < 0) {
            SENSOR_DEMO_LOGI("Pedometer subscription fail\r\n");
        } else {
            /* reset bt buffer index of input start */
            pedometer_triggered = 0;
            /* reset bt buffer index of input end */
            SENSOR_DEMO_LOGI("Pedometer unsubscribed\r\n");
        }
    }
#endif

}

#ifdef FUSION_BLOOD_PRESSURE_USE
void enable_bp(void)
{
    int32_t subscription = 0;

    (void)subscription;

    /* B.P. usage is exclusive with Heart rate*/

#ifdef FUSION_HEART_RATE_MONITOR_USE
    if (hr_triggered == 1) {
        subscription = sensor_unsubscribe_sensor(heart_rate_monitor_subscriber.handle);
        if (subscription < 0) {
            SENSOR_DEMO_LOGI("HR unsubscription fail\r\b");
        } else {
            hr_triggered = 2;
            SENSOR_DEMO_LOGI("HR unsubscribed\r\n");
        }
    }
#endif

    if (bp_triggered == 0) {
        subscription = sensor_subscribe_sensor(&blood_pressure_subscriber);
        if (subscription < 0) {
            SENSOR_DEMO_LOGI("B.P. subscription fail\r\n");
        } else {
            /* reset bt buffer index of input start */
            ekg_buff_count = 0;
            ppg1_buff_count = 0;
            /* reset bt buffer index of input end */
            SENSOR_DEMO_LOGI("B.P. triggered\r\n");
            bp_triggered = 1;
        }
    }
}

void disable_bp(void)
{
    int32_t unsubscription;

    if ( bp_triggered == 1) {
        unsubscription = sensor_unsubscribe_sensor(blood_pressure_subscriber.handle);
        if (unsubscription < 0) {
            SENSOR_DEMO_LOGI("B.P. unsubscription fail\r\n");
        } else {
            bp_triggered = 0;
            SENSOR_DEMO_LOGI("B.P. unsubscribed\r\n");
        }
    }

#ifdef FUSION_HEART_RATE_MONITOR_USE
    int32_t subscription;

    if (hr_triggered == 2) {
        subscription = sensor_subscribe_sensor(&heart_rate_monitor_subscriber);
        if (subscription < 0) {
            SENSOR_DEMO_LOGI("HR subscription fail\r\n");
        } else {
            hr_triggered = 1;
            SENSOR_DEMO_LOGI("HR subscribed\r\n");
        }
    }
#endif
}
#endif /* FUSION_BLOOD_PRESSURE_USE */

#else
void enable_hr(void)
{

}
void disable_hr(void)
{

}

void enable_step_counter(void)
{
}

void disable_step_counter(void)
{
}
#endif /*SENSOR_DEMO*/

