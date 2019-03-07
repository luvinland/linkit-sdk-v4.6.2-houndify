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

#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <gui/database/heart_rate_db.hpp>
#include <gui/common/CommonService.hpp>

#ifdef SIMULATOR
#include <ctime>
#ifndef _MSC_VER
#include <sys/time.h>
#endif /* _MSC_VER*/
#else
#include "FreeRTOS.h"
#include "hal_rtc.h"
#include "sensor_demo.h"
#endif /* SIMULATOR */

bool Model::powerSaving = false;

Model::Model() :
    modelListener(0),
    selectedHomeIndex(1),
    selectedMenuIndex(0),
    tickCount(10)
{
}

void Model::tick()
{
    Time previousTime = currentTime;
    int heartRateData = 0;
    static int previousStepData = 0;
    static bool duringBPudate = false;
    int stepData = 0;

#ifdef SIMULATOR
#ifdef _MSC_VER
    time_t rawtime;
    struct tm timenow;
    time(&rawtime);
    localtime_s(&timenow, &rawtime);

    currentTime.hours = timenow.tm_hour;
    currentTime.minutes = timenow.tm_min;
    currentTime.seconds = timenow.tm_sec;
    currentTime.milliseconds = 0;
#else
    timeval timenow;
    gettimeofday(&timenow, NULL);

    currentTime.hours = (timenow.tv_sec / 60 / 60) % 24;
    currentTime.minutes = (timenow.tv_sec / 60) % 60;
    currentTime.seconds = timenow.tv_sec % 60;
    currentTime.milliseconds = timenow.tv_usec / 1000;
#endif /*_MSC_VER*/
#else
    hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        configASSERT(0);
        return;
    }
    currentTime.hours = rtc_time.rtc_hour;
    currentTime.minutes = rtc_time.rtc_min;
    currentTime.seconds = rtc_time.rtc_sec;
    currentTime.milliseconds = 0;
#endif /* SIMULATOR */

    if (CommonService::getClockStyle() == CommonService::DIGITAL_CLOCK) {
        if (tickCount == 2 && CommonService::isBacklight() && !powerSaving && !duringBPudate) {
            // to make sure the minute is already updated
            CommonService::setPowerMode(60000);
            powerSaving = true;
        } else if (!CommonService::isBacklight()) {
            powerSaving = false;
        }
    } else {
        if (tickCount == 2 && CommonService::isBacklight() && !powerSaving && !duringBPudate) {
            // to make sure the second is already updated
            CommonService::setPowerMode(1000);
            powerSaving = true;
        } else if (!CommonService::isBacklight()) {
            powerSaving = false;
        }
    }

    if (CommonService::getClockStyle() == CommonService::DIGITAL_CLOCK && currentTime.minutes != previousTime.minutes) {
        tickCount = 1;
        if (modelListener) {
#ifndef SIMULATOR
            LOG_I(tgfx, "timeUpdated %d:%d\r\n", currentTime.minutes, currentTime.seconds);
#endif
            modelListener->timeUpdated(currentTime);
        }
    } else if (CommonService::getClockStyle() != CommonService::DIGITAL_CLOCK && currentTime.seconds != previousTime.seconds) {
        tickCount = 1;
        if (modelListener) {
#ifndef SIMULATOR
            LOG_I(tgfx, "timeUpdated %d:%d\r\n", currentTime.minutes, currentTime.seconds);
#endif
            modelListener->timeUpdated(currentTime);
        }
    } else {
        tickCount++;
    }

    // heart rate update
#ifdef SIMULATOR
    // Simulates the HeartRate data
    heartRateData = heartRateDataSZ[currentHeartDataIndex];
    modelListener->heartRateDataUpdated(heartRateData);
    ++currentHeartDataIndex;
    if (currentHeartDataIndex == FrequenceNum) {
        currentHeartDataIndex = 0;
    }
#else
    heartRateData = HeartRateCache::getHeartRateCacheInstance()->getHeartRateData();
    if (-1 != heartRateData) {
        // new data coming, need refresh UI
        modelListener->heartRateDataUpdated(heartRateData);
    }
#endif

    // update battery
    int currentCapatcityData = CommonService::getCapacityCurrentPercentage();
    int previousCapatcityData = CommonService::getCapacityPreviousPercentage();
    int chargerStatus = CommonService::getChargerStatus();
    int previousChargerStatus = CommonService::getPreviousChargerStatus();
    if (100 != previousCapatcityData) {
        if ( (currentCapatcityData != previousCapatcityData) ||
                (previousChargerStatus != chargerStatus) ) {
#ifndef SIMULATOR
            //LOG_I(bmt_demo, "currentCapatcityData: %d, previousCapatcityData: %d\r\n", currentCapatcityData, previousCapatcityData);
            //LOG_I(bmt_demo, "chargerStatus: %d, previousChargerStatus: %d\r\n", chargerStatus, previousChargerStatus);
#endif
            modelListener->capacityUpdated(currentCapatcityData);

        }
    }

    // update Gsensor data
    stepData = HeartRateCache::getHeartRateCacheInstance()->getGSensorData();
    if (previousStepData != stepData) {
#ifndef SIMULATOR
        LOG_I(tgfx, "getGSensorData previousStepData: %d, stepData: %d", previousStepData, stepData);
#endif
        modelListener->gSensorUpdated(stepData);
        previousStepData = stepData;
    }

    // update BP flow
#ifndef SIMULATOR
    if (CommonService::BLOODPRESURE_NOT_START == CommonService::GetBloodPresureStatus()) {
        if (CommonService::BLOODPRESURE_CACULATING == CommonService::GetBloodPresurePreviousStatus()) {
            disable_bp();
            CommonService::setBPTipsCount(10);
            CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
        } else if (CommonService::BLOODPRESURE_USER_TIPS == CommonService::GetBloodPresurePreviousStatus()) {
            //bsp_ctp_power_on(true);
            CommonService::setBPTipsCount(10);
            CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
        }
    }
#endif
    if (true == modelListener->bloodPresureTipsStartCheck()) {
        //bsp_ctp_power_on(false);
        if (currentTime.seconds != previousTime.seconds) {
            if (modelListener) {
                modelListener->bloodPresureTipsUpdated(CommonService::getBPTipsCount());
                if (0 == CommonService::reduceBPTipsCount()) {
                    CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_CACULATING);
                    CommonService::setBPTipsCount(10);
                    duringBPudate = true;
                }
            }
        }
    } else if (CommonService::BLOODPRESURE_CACULATING == CommonService::GetBloodPresureStatus()) {
        if (CommonService::BLOODPRESURE_CACULATING != CommonService::GetBloodPresurePreviousStatus()) {
#ifndef SIMULATOR
#include "syslog.h"
            CommonService::gotoBacklightCallback(NULL);
            LOG_I(tgfx, "BLOODPRESURE_CACULATING");
            enable_bp();
#endif
            modelListener->bloodPresureCaculating();
            CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_CACULATING);
        }
    } else if (CommonService::BLOODPRESURE_DONE == CommonService::GetBloodPresureStatus()) {
        // close bp sensor
        CommonService::turnOnBacklight();
#ifndef SIMULATOR
        disable_bp();
#endif
        // update sensor data
        int gbpData = HeartRateCache::getHeartRateCacheInstance()->getGHBPData();
        int lbpData = HeartRateCache::getHeartRateCacheInstance()->getLHBPData();
#ifndef SIMULATOR
        LOG_I(tgfx, "BLOODPRESURE_DONE gbpData: %d, lbpData: %d", gbpData, lbpData);
#endif
        modelListener->bloodPresureShowResult(gbpData, lbpData);
        duringBPudate = false;

        CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
    }
}
