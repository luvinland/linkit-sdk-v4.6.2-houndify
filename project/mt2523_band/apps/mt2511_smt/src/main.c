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
#include "timers.h"

/* device.h includes */
#include "mt2523.h"

/* hal includes */
#include "hal_cache.h"
#include "hal_mpu.h"
#include "hal_uart.h"
#include "hal_clock.h"
#include "hal_gpio.h"
#include "hal_dvfs.h"
#include "hal_sys_topsm.h"
#include "hal_pdma_internal.h"
#include "hal_gpio.h"
#include "hal_dcxo.h"
#include "bt_log.h"
#include "hal_log.h"
#include "hal_flash.h"
#include "hal_gpt.h"
#ifdef DEVICE_BAND
#include "hal_keypad_table.h"
#include "ept_keypad_drv.h"
#include "keypad_custom.h"
#include "hal_keypad_powerkey_internal.h"
#include "hal_pmu.h"
#include "hal_sleep_manager.h"
#include "hal_charger.h"
#include "hal_clock_internal.h"

#ifdef GT_PROJECT_ENABLE
#include "gt_main.h"
#include "watch.h"
#endif //GT_PROJECT_ENABLE
#endif

#ifdef MTK_SMART_BATTERY_ENABLE
#include "battery_management.h"
#endif

#include "sensor_bt_spp_server.h"

#include "usb.h"
#include "bsp_gpio_ept_config.h"


//extern void GPIO_setting_init(void);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/


#include "sensor_demo.h"

/* Private variables ---------------------------------------------------------*/
//#define DTP_ENABLE

#include "hal_gpio_ept_config.h"
//#include "bsp_gpio_ept_config.h"
#include "memory_map.h"
#include "hal_rtc.h"

//#define USE_MK20D

#include "vsm_driver.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include "hal_pmu_internal.h"
#include "hal_pmu.h"
#endif

#define ATCI_DEMO

#ifdef ATCI_DEMO
#include "atci.h"

#if defined(MTK_ATCI_VIA_PORT_SERVICE) && defined(MTK_PORT_SERVICE_ENABLE)

#ifdef MTK_AUDIO_TUNING_ENABLED
#define ATCI_TASK_STACK_SIZE 5000 + 1024
#else
#define ATCI_TASK_STACK_SIZE 1024 + 1024
#endif

#else

#ifdef MTK_AUDIO_TUNING_ENABLED
#define ATCI_TASK_STACK_SIZE 5000
#else
#define ATCI_TASK_STACK_SIZE 1024
#endif

#endif


#if defined(MTK_PORT_SERVICE_ENABLE)
extern void reg_exception_trigger_at_cmd(void);
#endif

void atci_def_task(void *param)
{
    while (1) {
        atci_processing();
    }
}
#endif

#include "nvdm.h"


/* Private functions ---------------------------------------------------------*/
static void SystemClock_Config(void);
static void prvSetupHardware( void );


void vApplicationTickHook(void)
{

}

int __io_putchar(int ch)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  log_putchar(ch);
  return ch;
}

#ifdef SENSOR_DEMO
#include "sensor_manager.h" /* sensor manager task */
#include "sensor_alg_interface.h"
//static TaskHandle_t sensor_task_handle;

/* 10 second heart rate data*/
static uint8_t heart_rate_10s_data[17];
static uint32_t heart_data_counter;
static uint8_t heart_rate_variability_data[23];

int32_t all_sensor_send_digest_callback(sensor_data_t *const output);

#ifdef FUSION_PEDOMETER_USE
sensor_subscriber_t pedometer_subscriber = {
    "ap12", 0, SENSOR_TYPE_PEDOMETER, 0, all_sensor_send_digest_callback
};
static uint32_t pedo_triggered=0;
#endif

#ifdef MTK_SENSOR_ACCELEROMETER_USE
sensor_subscriber_t acc_subscriber = {
    "ap0", 0, SENSOR_TYPE_ACCELEROMETER, 8, all_sensor_send_digest_callback
};
#endif

#ifdef MTK_SENSOR_BIO_USE
sensor_subscriber_t ekg_subscriber = {
    "ap5", 0, SENSOR_TYPE_BIOSENSOR_EKG, 100, all_sensor_send_digest_callback
};
#endif

#ifdef MTK_SENSOR_BIO_USE
sensor_subscriber_t ppg1_subscriber = {
    "ap9", 0, SENSOR_TYPE_BIOSENSOR_PPG1, 64, all_sensor_send_digest_callback
};
#endif

#ifdef MTK_SENSOR_BIO_USE
sensor_subscriber_t ppg2_subscriber = {
    "ap10", 0, SENSOR_TYPE_BIOSENSOR_PPG2, 64, all_sensor_send_digest_callback
};
#endif

#ifdef MTK_SENSOR_BIO_USE
sensor_subscriber_t bisi_subscriber = {
    "ap11", 0, SENSOR_TYPE_BIOSENSOR_BISI, 100, all_sensor_send_digest_callback
};
#endif

#ifdef FUSION_SLEEP_TRACKER_USE
sensor_subscriber_t sleep_subscriber = {
    "ap19", 0, SENSOR_TYPE_SLEEP, 0, all_sensor_send_digest_callback
};
#endif

#ifdef FUSION_HEART_RATE_MONITOR_USE
sensor_subscriber_t heart_rate_monitor_subscriber = {
    "ap21", 0, SENSOR_TYPE_HEART_RATE_MONITOR, 0, all_sensor_send_digest_callback
};
static uint32_t hr_triggered=0;
#endif /*FUSION_HEART_RATE_MONITOR_USE*/

#ifdef FUSION_HEART_RATE_VARIABILITY_USE
sensor_subscriber_t heart_rate_variability_subscriber = {
    "ap22", 0, SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR, 0, all_sensor_send_digest_callback
};

static uint32_t hrv_triggered=0;
#endif

#ifdef FUSION_BLOOD_PRESSURE_USE
sensor_subscriber_t blood_pressure_subscriber = {
    "ap23", 0, SENSOR_TYPE_BLOOD_PRESSURE_MONITOR, 0, all_sensor_send_digest_callback
};

static uint32_t bp_triggered=0;
#endif


#ifdef DTP_ENABLE
bool dtp_enabled;
bool dtp_sent_done;
void sensor_event_callback (dtp_event_t *evt)
{
    //warning: make sure not too much operations in this function, or make bt task stack overflow
    switch(evt->event_id)
    {
        case DTP_EVENT_ENABLED:
            dtp_enabled = true;
            dtp_sent_done = true;
            break;
        case DTP_EVENT_DISABLED:
            dtp_enabled = false;
            break;
        case DTP_EVENT_DATA_SENT:
            if (evt->result == DTP_SUCCESS)
                dtp_sent_done = true;
            break;
#ifdef RECEIVE_DATA_ENABLE
        case DTP_EVENT_DATA_RECEIVED:
            /*LOG_I(DTP, "length = %d, data = %x %x %x %x %x\n", evt->length,
                evt->data[0], evt->data[1], evt->data[2], evt->data[3], evt->data[4]);*/
            break;
#endif
        default:
            break;
    }
}
#endif /*DTP_ENABLE*/

#ifdef RECEIVE_DATA_ENABLE
void spps_receive_data_callback (uint16_t data_len, uint8_t *data)
{
    //LOG_I(SPPS, "data_len = %d, data = %x %x %x %x %x\r\n", data_len, data[0], data[1], data[2], data[3], data[4]);

    /*Don't do too much in this callback. It runs in bt task.*/
    return;
}
#endif

static uint32_t ppg1_buff_data;
static uint32_t ppg1_buff_timestamp;
static int ppg1_buff_count = 0;
static uint32_t ppg2_buff_data;
static uint32_t ppg2_buff_timestamp;
static int ppg2_buff_count = 0;
static uint32_t ekg_buff_data;
static uint32_t ekg_buff_timestamp;
static int ekg_buff_count = 0;

static int32_t ui_main_timer_id;
static TimerHandle_t xtimer_ui_main;
uint32_t key_processing;

/* running at timer task*/
void vuimain_timeout( TimerHandle_t pxTimer )
{
    hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
    key_processing = 0;
}

int32_t all_sensor_send_digest_callback(sensor_data_t *const output)
{

#ifdef GT_PROJECT_ENABLE
	tEvent outEvent;
#endif

    switch (output->data[0].sensor_type) {

    case SENSOR_TYPE_ACCELEROMETER:
        #if 0
        SENSOR_DEMO_LOGI("acc sensor  type = %lu , value = ( %ld , %ld , %ld ) (%ld) , timestamp = %lu \r\n",
                        output->data[0].sensor_type,
                        output->data[0].accelerometer_t.x,
                        output->data[0].accelerometer_t.y,
                        output->data[0].accelerometer_t.z,
                        output->data[0].accelerometer_t.status,
                        output->data[0].time_stamp
                      );
        #endif
        send_sensor_data_via_btspp(
                            SENSOR_SPP_DATA_MAGIC,
                            SENSOR_TYPE_ACCELEROMETER,
                            output->data[0].accelerometer_t.x,
                            output->data[0].accelerometer_t.y,
                            output->data[0].accelerometer_t.z,
                            output->data[0].time_stamp,
                            SENSOR_SPP_DATA_RESERVED);
        break;

    case SENSOR_TYPE_BIOSENSOR_PPG1:
        if(ppg1_buff_count==0){
            ppg1_buff_data = output->data[0].bio_data.data;
            ppg1_buff_timestamp = output->data[0].time_stamp;
            ppg1_buff_count++;
        }else{
            send_sensor_data_via_btspp(
                               SENSOR_SPP_DATA_MAGIC,
                               SENSOR_TYPE_BIOSENSOR_PPG1,
                               ppg1_buff_data,
                               ppg1_buff_timestamp,
                               output->data[0].bio_data.data,
                               output->data[0].time_stamp,
                               SENSOR_SPP_DATA_RESERVED);
            ppg1_buff_count = 0;
        }

        break;
    case SENSOR_TYPE_BIOSENSOR_PPG2:
        if(ppg2_buff_count==0){
            ppg2_buff_data = output->data[0].bio_data.data;
            ppg2_buff_timestamp = output->data[0].time_stamp;
            ppg2_buff_count++;
        }else{
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
        if(ekg_buff_count==0){
            ekg_buff_data = output->data[0].bio_data.data;
            ekg_buff_timestamp = output->data[0].time_stamp;
            ekg_buff_count++;
        }else{
            send_sensor_data_via_btspp(
                               SENSOR_SPP_DATA_MAGIC,
                               SENSOR_TYPE_BIOSENSOR_EKG,
                               ekg_buff_data,
                               ekg_buff_timestamp,
                               output->data[0].bio_data.data,
                               output->data[0].time_stamp,
                               SENSOR_SPP_DATA_RESERVED);
            ekg_buff_count = 0;
        }
        break;

    case SENSOR_TYPE_BIOSENSOR_PPG1_512HZ:
        break;

    case SENSOR_TYPE_PEDOMETER:
        SENSOR_DEMO_LOGI("pedometer sensor  type = %lu , value =  %ld , %ld , %ld , %ld , %ld timestamp = %lu \r\n",
                        output->data[0].sensor_type,
                        output->data[0].pedometer_t.accumulated_step_count,
                        output->data[0].pedometer_t.accumulated_step_length,
                        output->data[0].pedometer_t.step_frequency,
                        output->data[0].pedometer_t.step_length,
                        output->data[0].pedometer_t.step_type,
                        output->data[0].time_stamp
                      );
        send_sensor_data_via_btspp(
                           SENSOR_SPP_DATA_MAGIC,
                           SENSOR_TYPE_PEDOMETER,
                           output->data[0].pedometer_t.accumulated_step_count,
                           output->data[0].pedometer_t.step_type,
                           output->data[0].time_stamp,
                           SENSOR_SPP_DATA_RESERVED,
                           SENSOR_SPP_DATA_RESERVED);

#ifdef GT_PROJECT_ENABLE


        if (queAppMgr != NULL) {
            outEvent.event = EVENT_UPDATE_PEDO;
            outEvent.userdata = &output->data[0].pedometer_t;
                    xQueueSend(queAppMgr,&outEvent,0);
        }


#endif


        break;
    case SENSOR_TYPE_SLEEP:
        send_sensor_data_via_btspp(
                           SENSOR_SPP_DATA_MAGIC,
                           SENSOR_TYPE_SLEEP,
                           output->data[0].sleep_data_t.state,
                           output->data[0].time_stamp,
                           SENSOR_SPP_DATA_RESERVED,
                           SENSOR_SPP_DATA_RESERVED,
                           SENSOR_SPP_DATA_RESERVED);
        break;
        case SENSOR_TYPE_HEART_RATE_MONITOR:
            #if 0
            SENSOR_DEMO_LOGI("Heart rate , bpm = %ld , timestampe = %lu \r\n",
               output->data[0].heart_rate_t.bpm,
               output->data[0].time_stamp);
            #endif

#ifdef GT_PROJECT_ENABLE


		if (queAppMgr != NULL) {
			outEvent.event = EVENT_UPDATE_HR;
			outEvent.userdata = &output->data[0].heart_rate_t;
                	xQueueSend(queAppMgr,&outEvent,0);
		}


#endif

            send_sensor_data_via_btspp(
                               SENSOR_SPP_DATA_MAGIC,
                               SENSOR_TYPE_HEART_RATE_MONITOR,
                               output->data[0].heart_rate_t.bpm,
                               output->data[0].heart_rate_t.status,
                               output->data[0].time_stamp,
                               SENSOR_SPP_DATA_RESERVED,
                               SENSOR_SPP_DATA_RESERVED);




            heart_rate_10s_data[7+heart_data_counter] = (uint8_t)(output->data[0].heart_rate_t.bpm);
            heart_data_counter++;
            if(heart_data_counter == 10){
                heart_rate_10s_data[0] = 1;
                heart_rate_10s_data[1] = 17;
                heart_rate_10s_data[6] = 1;
                /* send the 10s data */
#ifdef DTP_ENABLE
                SENSOR_DEMO_LOGI("dtp_enabled = %d, dtp_sent_done = %d\n", dtp_enabled, dtp_sent_done);
                if (dtp_enabled && dtp_sent_done) {
                    dtp_send_data(17, heart_rate_10s_data);
                    dtp_sent_done = false;
                }
#endif /*DTP_ENABLE*/
                SENSOR_DEMO_LOGI("HR: dtp_send_data %ld\r\n", heart_data_counter);


                heart_data_counter = 0;
            }

            break;

        case SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR:
        {


#ifdef GT_PROJECT_ENABLE


		if (queAppMgr != NULL) {
			outEvent.event = EVENT_UPDATE_HRV;
			outEvent.userdata = &output->data[0].heart_rate_variability_t;
                	xQueueSend(queAppMgr,&outEvent,0);
		}


#endif

            SENSOR_DEMO_LOGI("Heart rate variability: T(%ld), SDNN(%ld), LF(%ld), HF(%ld), LF_HF(%ld)\r\n", output->data[0].time_stamp, output->data[0].heart_rate_variability_t.SDNN,
                output->data[0].heart_rate_variability_t.LF, output->data[0].heart_rate_variability_t.HF, output->data[0].heart_rate_variability_t.LF_HF);
            int32_t tLF = output->data[0].heart_rate_variability_t.LF;
            int32_t tHF = output->data[0].heart_rate_variability_t.HF;
            int32_t tLF_HF = output->data[0].heart_rate_variability_t.LF_HF;
            int32_t tSDNN = output->data[0].heart_rate_variability_t.SDNN;
            heart_rate_variability_data[0] = 4;
            heart_rate_variability_data[1] = 23;
            heart_rate_variability_data[6] = 1;
            heart_rate_variability_data[7] = tLF & 0xFF;
            heart_rate_variability_data[8] = (tLF & 0xFF00)>>8;
            heart_rate_variability_data[9] = (tLF & 0xFF0000)>>16;
            heart_rate_variability_data[10] =(tLF & 0xFF000000)>>24;;
            heart_rate_variability_data[11] = tHF & 0xFF;
            heart_rate_variability_data[12] = (tHF & 0xFF00) >>8;
            heart_rate_variability_data[13] = (tHF & 0xFF0000) >>16;
            heart_rate_variability_data[14] = (tHF & 0xFF000000) >>24;
            heart_rate_variability_data[15] = tLF_HF& 0xFF;
            heart_rate_variability_data[16] = (tLF_HF& 0xFF00)>>8;
            heart_rate_variability_data[17] = (tLF_HF& 0xFF0000)>>16;
            heart_rate_variability_data[18] = (tLF_HF& 0xFF000000)>>24;
            heart_rate_variability_data[19] = tSDNN&0xFF;
            heart_rate_variability_data[20] = (tSDNN&0xFF00)>>8;
            heart_rate_variability_data[21] = (tSDNN&0xFF0000)>>16;
            heart_rate_variability_data[22] = (tSDNN&0xFF000000)>>24;
#ifdef DTP_ENABLE
            printf("HRV: dtp_enabled = %d, dtp_sent_done = %d\n", dtp_enabled, dtp_sent_done);
            if (dtp_enabled && dtp_sent_done) {
                dtp_send_data(23, heart_rate_variability_data);
                dtp_sent_done = false;
            }
#endif /*DTP_ENABLE*/

            send_sensor_data_via_btspp(
                               SENSOR_SPP_DATA_MAGIC,
                               SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR,
                               output->data[0].heart_rate_variability_t.SDNN,
                               output->data[0].heart_rate_variability_t.LF,
                               output->data[0].heart_rate_variability_t.HF,
                               output->data[0].heart_rate_variability_t.LF_HF,
                               output->data[0].time_stamp
                               );

            /* To unsubscribe here? */
#ifdef FUSION_HEART_RATE_VARIABILITY_USE
            int32_t unsubscription;
            hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);

            unsubscription = sensor_unsubscribe_sensor(heart_rate_variability_subscriber.handle);
            if (unsubscription < 0) {
                SENSOR_DEMO_LOGI("HRV unsubscription fail\r\n");
            } else {
                hrv_triggered = 0;
                SENSOR_DEMO_LOGI("HRV unsubscribed\r\n");
            }
            xTimerChangePeriod( xtimer_ui_main, 5*1000/portTICK_PERIOD_MS, 1000/portTICK_PERIOD_MS);
            xTimerReset(xtimer_ui_main, 1000 / portTICK_PERIOD_MS);
#endif
            break;
        }

        case SENSOR_TYPE_BLOOD_PRESSURE_MONITOR:
        {
            SENSOR_DEMO_LOGI("Blood pressure sbp(%ld), dbp(%ld), status(%ld) \r\n", output->data[0].blood_pressure_t.sbp,
                output->data[0].blood_pressure_t.dbp, output->data[0].blood_pressure_t.status);

#ifdef GT_PROJECT_ENABLE


                if (queAppMgr != NULL) {
                        outEvent.event = EVENT_UPDATE_BP;
                        outEvent.userdata = &output->data[0].blood_pressure_t;
                        xQueueSend(queAppMgr,&outEvent,0);
                }


#endif
            /* To unsubscribe here? */
#ifdef FUSION_BLOOD_PRESSURE_USE
            int32_t unsubscription;
            hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
            unsubscription = sensor_unsubscribe_sensor(blood_pressure_subscriber.handle);
            if (unsubscription < 0) {
                SENSOR_DEMO_LOGI("B.P unsubscription fail\r\n");
            } else {
                bp_triggered = 0;
                SENSOR_DEMO_LOGI("B.P. unsubscribed\r\n");
            }
            xTimerChangePeriod( xtimer_ui_main, 5*1000/portTICK_PERIOD_MS, 1000/portTICK_PERIOD_MS);
            xTimerReset(xtimer_ui_main, 1000 / portTICK_PERIOD_MS);
#endif
            break;
        }

        default:
            SENSOR_DEMO_LOGI("type = %lu,  timestamp = %lu \r\n",
                   output->data[0].sensor_type,
                   output->data[0].time_stamp);
            break;
    }


    //TODO collect sensor data and send to Android device via BT
    //...

    return 0;
}

#endif /*SENSOR_DEMO*/

#define UI_MSG_SENSOR 1
typedef struct ui_task_message_struct {
    int32_t message_id;
    int32_t param1;
    void* param2;
} ui_task_message_struct_t;

static TaskHandle_t ui_main_task_handler;
QueueHandle_t ui_main_event_queue;

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

/**
* @brief       caculate actual bit value of region size.
* @param[in]   region_size: actual region size.
* @return      corresponding bit value of region size for MPU setting.
*/
static uint32_t caculate_mpu_region_size( uint32_t region_size )
{
    uint32_t count;

    if (region_size < 32) {
        return 0;
    }
    for (count = 0; ((region_size  & 0x80000000) == 0); count++, region_size  <<= 1);
    return 30 - count;
}

/**
* @brief       This function is to initialize MPU.
* @param[in]   None.
* @return      None.
*/
static void mpu_init(void)
{
    hal_mpu_region_t region, region_number;
    hal_mpu_region_config_t region_config;
    typedef struct {
        uint32_t mpu_region_base_address;/**< MPU region start address */
        uint32_t mpu_region_end_address;/**< MPU region end address */
        hal_mpu_access_permission_t mpu_region_access_permission;/**< MPU region access permission */
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

#ifdef DEVICE_BAND
/* keypad handler */
void keypad_user_powerkey_handler(void)
{
        hal_keypad_status_t             ret;
        hal_keypad_powerkey_event_t powekey_event;
    	char *string[5] = {"release", "press", "longpress", "repeat","pmu_longpress"};

        ret = hal_keypad_powerkey_get_key(&powekey_event);

        if (ret == HAL_KEYPAD_STATUS_ERROR) {
                log_hal_info("[keypad][test]powerkey no key in buffer\r\n\r\n");
                return;
        }

        log_hal_info("[keypad][test]powerkey data:[%d], state:[%s]\r\n", powekey_event.key_data, string[powekey_event.state]);

	if (powekey_event.state == 2) {

               	log_hal_info("[keypad][test]start to power down\r\n");
		hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
                hal_gpio_set_output(HAL_GPIO_5, HAL_GPIO_DATA_LOW);
                hal_gpio_set_output(HAL_GPIO_6, HAL_GPIO_DATA_HIGH);


                hal_gpt_delay_ms(3 * 1000);
		hal_sleep_manager_enter_power_off_mode();
	}

}

void keypad_user_key_handler(void)
{
    hal_keypad_status_t             ret;
    hal_keypad_event_t key_event;
    char *string[3] = {"release", "press", "longpress"};

    ret = hal_keypad_get_key(&key_event);

    if (ret == HAL_KEYPAD_STATUS_ERROR) {
        log_hal_info("[keypad][test]key no key in buffer\r\n\r\n");
        return;
    }

    log_hal_info("[keypad][test]key data:[%d], state:[%s]\r\n", key_event.key_data, string[key_event.state]);

    if (key_event.state == 1) { /* key release */

        if (key_processing  == 0) {
            key_processing = 1;

            //add trigger HRV/BP here

            BaseType_t xHigherPriorityTaskWoken;
            ui_task_message_struct_t message;
            message.message_id = UI_MSG_SENSOR;

            #ifdef FUSION_HEART_RATE_VARIABILITY_USE
            log_hal_info("[keypad][test] Trigger HRV \r\n");
            message.param1 = SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR;
            #endif

            #ifdef FUSION_BLOOD_PRESSURE_USE
            log_hal_info("[keypad][test] Trigger B.P. \r\n");
            message.param1 = SENSOR_TYPE_BLOOD_PRESSURE_MONITOR;
            #endif
            message.param2 = NULL;

            xQueueSendFromISR(ui_main_event_queue, &message, &xHigherPriorityTaskWoken);
        }else {
            log_hal_info("ker processing\r\n");
        }
    }
}

/*use powerkey*/
void hal_key_enable(void)
{
        bool                            ret_bool;
        hal_keypad_status_t ret_state;


        /*init keypad and powerkey*/
        ret_bool = keypad_custom_powerkey_init();
        if (ret_bool == false) {
                log_hal_info("[keypad][test]keypad_custom_init init fail\r\n");
        }

        ret_state = hal_keypad_powerkey_register_callback((hal_keypad_callback_t)keypad_user_powerkey_handler, NULL);
        if (ret_state != HAL_KEYPAD_STATUS_OK) {
                log_hal_info("[keypad][test]hal_keypad_powerkey_register_callback fail, state = %d\r\n", ret_state);

		hal_sleep_manager_enter_power_off_mode();

        }

        ret_bool = keypad_custom_normal_init();
        if (ret_bool == false) {
                log_hal_info("[keypad][test]keypad_custom_init init fail\r\n");
        }


        ret_state = hal_keypad_register_callback((hal_keypad_callback_t)keypad_user_key_handler, NULL);
        if (ret_state != HAL_KEYPAD_STATUS_OK) {
                log_hal_info("[keypad][test]hal_keypad_key_register_callback fail, state = %d\r\n", ret_state);
        }


	ret_state = hal_keypad_enable();
	if (ret_state != HAL_KEYPAD_STATUS_OK) {
		log_hal_info("[keypad][test]hal_keypad_enable fail, state = %d\r\n", ret_state);
	}



}
#endif

void ui_task_main()
{
    ui_task_message_struct_t queue_item;

    ui_main_timer_id = 0;
    xtimer_ui_main = xTimerCreate("uimain",       /* Just a text name, not used by the kernel. */
                                ( 10*1000 / portTICK_PERIOD_MS ),  /* The timer period in ticks. */
                                pdFALSE,        /* The timers will auto-reload themselves when they expire. */
                                ( void *) ui_main_timer_id,   /* Assign each timer a unique id equal to its array index. */
                                vuimain_timeout /* Each timer calls the same callback when it expires. */
                               );

    if ( xtimer_ui_main == NULL ) {
        SENSOR_DEMO_LOGI("xTimerofMems create fail \r\n");
    }

    int32_t subscription = -1 ;
    int32_t unsubscription = -1;

    while (1) {
        if (xQueueReceive(ui_main_event_queue, &queue_item, 10)) {
            switch (queue_item.message_id) {
                case UI_MSG_SENSOR:


                    #ifdef FUSION_HEART_RATE_VARIABILITY_USE
                    if (queue_item.param1 == SENSOR_TYPE_HEART_RATE_VARIABILITY_MONITOR) {
                        if (hrv_triggered == 0) {
				
                    		hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
                    		vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
				
                    		hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
                            	subscription = sensor_subscribe_sensor(&heart_rate_variability_subscriber);
                            if (subscription < 0) {
                                SENSOR_DEMO_LOGI("Heart rate variability subscription fail\r\n");
                            }
                            else {
                                SENSOR_DEMO_LOGI("Heart rate variability triggered\r\n");
                                hrv_triggered = 1;
                            }
                        }
                    }
                    #endif

                    #ifdef FUSION_BLOOD_PRESSURE_USE
                    if (queue_item.param1 == SENSOR_TYPE_BLOOD_PRESSURE_MONITOR) {
                        if (bp_triggered == 0) {
			
                    		hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
                    		vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
				 
                   		 hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
                            subscription = sensor_subscribe_sensor(&blood_pressure_subscriber);
                            if (subscription < 0) {
                                SENSOR_DEMO_LOGI("B.P. subscription fail\r\n");
                            }
                            else {
                                SENSOR_DEMO_LOGI("B.P. triggered\r\n");
                                bp_triggered = 1;
                            }
                        }
                    }
                    #endif

                    #ifdef FUSION_HEART_RATE_MONITOR_USE
                    if (queue_item.param1 == SENSOR_TYPE_HEART_RATE_MONITOR) {
			
                    	hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
                    	vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);
			
                    	hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
                        if (hr_triggered == 0) {
                            subscription = sensor_subscribe_sensor(&heart_rate_monitor_subscriber);
                            if (subscription < 0) {
                                SENSOR_DEMO_LOGI("Heart rate monitor subscription fail\r\n");
                            }
                            else {
                                SENSOR_DEMO_LOGI("Heart rate monitor triggered\r\n");
                                hr_triggered = 1;
                            }
                        } else {

                        	unsubscription = sensor_unsubscribe_sensor(heart_rate_monitor_subscriber.handle);
							if (unsubscription < 0) {
                				SENSOR_DEMO_LOGI("HR unsubscription fail\r\n");
            				} else {
                				hr_triggered = 0;
                				SENSOR_DEMO_LOGI("HR unsubscribed\r\n");
            				}
						}
                    }
                    #endif


                    #ifdef FUSION_PEDOMETER_USE
                    if (queue_item.param1 == SENSOR_TYPE_PEDOMETER) {
			
                    	hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
                    	vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);
			
                    	hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_LOW);
                        if (pedo_triggered == 0) {
                            subscription = sensor_subscribe_sensor(&pedometer_subscriber);
                            if (subscription < 0) {
                                SENSOR_DEMO_LOGI("PEDO subscribe fail\r\n");
                            }
                            else {
                                SENSOR_DEMO_LOGI("pedo triggered\r\n");
                                pedo_triggered = 1;
                            }
                        } else {

                        	unsubscription = sensor_unsubscribe_sensor(heart_rate_monitor_subscriber.handle);
							if (unsubscription < 0) {
                				SENSOR_DEMO_LOGI("PEDO unsubscription fail\r\n");
            				} else {
                				pedo_triggered = 0;
                				SENSOR_DEMO_LOGI("PEDO unsubscribed\r\n");
            				}
						}
                    }
                    #endif



                    break;


                default:
                    break;
            }
        }
    }
}

#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"

static void syslog_port_service_init(void)
{
    serial_port_dev_t syslog_port;
    serial_port_setting_uart_t uart_setting;

    if (serial_port_config_read_dev_number("syslog", &syslog_port) != SERIAL_PORT_STATUS_OK) {
        syslog_port = SERIAL_PORT_DEV_UART_0;
        serial_port_config_write_dev_number("syslog", syslog_port);
        uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
        serial_port_config_write_dev_setting(syslog_port, (serial_port_dev_setting_t *)&uart_setting);
    }
}
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	//uint8_t buf[11];
    //uint32_t buf_size;
	//nvdm_status_t nvdm_init_status;
    //nvdm_status_t nvdm_status;

    /* SystemClock Config */
    SystemClock_Config();

    SystemCoreClockUpdate();

    /* Configure the hardware ready to run the test. */
    prvSetupHardware();

    /* init sys log */
    log_uart_init(HAL_UART_0);


	hal_sleep_manager_init();

    hal_dcxo_init();

    clock_dump_info();

	//nvdm_init_status = nvdm_init();
    nvdm_init();

#if defined(MTK_PORT_SERVICE_ENABLE)
    syslog_port_service_init();
#endif

#ifdef DEVICE_BAND
    hal_gpio_set_direction(HAL_GPIO_4, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_direction(HAL_GPIO_5, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_direction(HAL_GPIO_6, HAL_GPIO_DIRECTION_OUTPUT);
	hal_pinmux_set_function(HAL_GPIO_7, 0);
    hal_gpio_set_direction(HAL_GPIO_7, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_7, HAL_GPIO_DATA_HIGH);


    while(1)
	{
		int32_t battery_voltage;
		bool charger_status;

		hal_charger_meter_get_battery_voltage_sense(&battery_voltage);
		LOG_I(hal, "Check battery_voltage = %d mV \n\r",battery_voltage);

		if ( battery_voltage >= 3400 ) {
			hal_gpio_set_output(HAL_GPIO_6, HAL_GPIO_DATA_LOW);
			break;
		} else {
			hal_charger_get_charger_detect_status(&charger_status);

			if (charger_status == true)
			{

			    hal_gpio_set_output(HAL_GPIO_6, HAL_GPIO_DATA_HIGH);

				LOG_I(hal, "SW charging battery_voltage = %d mV \n\r",battery_voltage);
				hal_charger_init();
				hal_charger_set_charging_current(HAL_CHARGE_CURRENT_120_MA);
				hal_charger_enable(true);
				hal_charger_reset_watchdog_timer();
				hal_gpt_delay_ms(10 * 1000);

			} else {
				LOG_I(hal, "Low battery power off battery_voltage = %d mV \n\r",battery_voltage);

				hal_gpt_delay_ms(1000);
				hal_sleep_manager_enter_power_off_mode();
			}

		}
	}

    //ret = hal_gpio_set_output(HAL_GPIO_4, HAL_GPIO_DATA_HIGH);
    hal_gpio_set_output(HAL_GPIO_5, HAL_GPIO_DATA_HIGH);
    //ret = hal_gpio_set_output(HAL_GPIO_6, HAL_GPIO_DATA_HIGH);
#endif

#if 0
	if(nvdm_init_status != NVDM_STATUS_OK) {
				

	} else {

		
		nvdm_status = nvdm_read_data_item("2511","userid",(uint8_t *)buf, &buf_size);
		if(nvdm_status == NVDM_STATUS_OK) nvdm_write_data_item("2511","userid",NVDM_DATA_ITEM_TYPE_STRING, "0000000000", 11);
		nvdm_status = nvdm_read_data_item("2511","height",(uint8_t *)buf, &buf_size); 
		if(nvdm_status == NVDM_STATUS_OK) nvdm_write_data_item("2511","userid",NVDM_DATA_ITEM_TYPE_STRING, "173", 11);
		nvdm_status = nvdm_read_data_item("2511","weight",(uint8_t *)buf, &buf_size); 
		if(nvdm_status == NVDM_STATUS_OK) nvdm_write_data_item("2511","userid",NVDM_DATA_ITEM_TYPE_STRING, "65", 11);
		nvdm_status = nvdm_read_data_item("2511","gender",(uint8_t *)buf, &buf_size); 
		if(nvdm_status == NVDM_STATUS_OK) nvdm_write_data_item("2511","userid",NVDM_DATA_ITEM_TYPE_STRING, "1", 11);
		nvdm_status = nvdm_read_data_item("2511","old",(uint8_t *)buf, &buf_size); 
		if(nvdm_status == NVDM_STATUS_OK) nvdm_write_data_item("2511","userid",NVDM_DATA_ITEM_TYPE_STRING, "35", 11);
		nvdm_status = nvdm_read_data_item("2511","handlen",(uint8_t *)buf, &buf_size); 
		if(nvdm_status == NVDM_STATUS_OK) nvdm_write_data_item("2511","handlen",NVDM_DATA_ITEM_TYPE_STRING, "80", 11);

	}
#endif
	

    log_init(NULL, NULL, NULL);


#ifdef MTK_SMART_BATTERY_ENABLE
    battery_management_init();
#endif

//#ifdef DTP_ENABLE
//    bt_gap_power_on();
//    dtp_init();
//#endif /*DTP_ENABLE*/

  #ifdef SENSOR_DEMO

//#ifdef DTP_ENABLE
//    dtp_register(sensor_event_callback);
//#endif

#ifdef MTK_USB_DEMO_ENABLED
	usb_boot_init();
#endif

#ifdef RECEIVE_DATA_ENABLE
    spps_register(spps_receive_data_callback);
#endif

#ifdef USE_MK20D
    pmu_set_register_value(PMU_RG_PWRHOLD_ADDR          ,PMU_RG_PWRHOLD_MASK            ,PMU_RG_PWRHOLD_SHIFT           ,0);
#endif
    
#ifdef ATCI_DEMO
	/* init ATCI module and set UART port */
#if defined(MTK_ATCI_VIA_PORT_SERVICE) && defined(MTK_PORT_SERVICE_ENABLE)
{
    serial_port_dev_t port;
    serial_port_setting_uart_t uart_setting;

    if (serial_port_config_read_dev_number("atci", &port) != SERIAL_PORT_STATUS_OK)
    {
        //port = SERIAL_PORT_DEV_UART_1;
        port = SERIAL_PORT_DEV_USB_COM1;
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
	
	/* create task for ATCI */
	xTaskCreate( atci_def_task, "ATCI", ATCI_TASK_STACK_SIZE, NULL, 3, NULL );
#endif


    //sensor_peripheral_init();
    sensor_manager_init();

    //xTaskCreate( sensor_mgr_task, "SensorMgr", 2048/(( uint32_t )sizeof( StackType_t )), (void *)idx, 3, &sensor_task_handle);
    //xTaskCreate( sensor_fusion_task, "SensorFusion", 6144/(( uint32_t )sizeof( StackType_t )), (void *)idx, 2, &sensor_task_handle);
  #endif /*SENSOR_DEMO*/


    /* Main UI task */
    ui_main_event_queue = xQueueCreate(10 , sizeof( ui_task_message_struct_t ) );
    xTaskCreate( ui_task_main, "UI_MAIN", 4096, NULL, 3, &ui_main_task_handler );

#ifdef DEVICE_BAND

#ifdef GT_PROJECT_ENABLE
	xTaskCreate( vGTMain_Task, "vGTMain_Task", 4096, NULL, 2, NULL);
#else
    	hal_key_enable();

#endif //GT_PROJECT_ENABLE
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
    for( ;; );
}

static void prvSetupHardware( void )
{


    

    /* system HW init */
    cache_init();
    
    mpu_init();

    /* peripherals init */
    hal_flash_init(); /* flash init */

    hal_nvic_init();  /* nvic init */

    bsp_ept_gpio_setting_init();


    hal_rtc_init();


}

static void SystemClock_Config(void)
{

	hal_clock_init();

    hal_dvfs_init();
    hal_dvfs_target_cpu_frequency(104000, HAL_DVFS_FREQ_RELATION_H);
}
