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

#include <gui/home_screen/HomeView.hpp>
#include <BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <gui/common/CommonUI.hpp>
#include <gui/common/CommonService.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>

#ifdef SIMULATOR
#include <ctime>
#ifndef _MSC_VER
#include <sys/time.h>
#endif /* _MSC_VER*/
#else
#include "FreeRTOS.h"
#include "hal_rtc.h"
#include "syslog.h"
#include "bt_notify_app_list.h"
#include "data_transfer.h"

log_create_module(tgfx, PRINT_LEVEL_INFO);
log_create_module(mp3ui, PRINT_LEVEL_INFO);
#endif

int calcWeek(int y, int m, int d)
{
    int c, w;

    if (m == 1 || m == 2) {
        y--;
        m += 12;
    }
    c = y / 100;
    y = y - c * 100;
    w = (c / 4) - 2 * c + (y + y / 4) + (13 * (m + 1) / 5) + d - 1;
    while (w < 0) {
        w += 7;
    }
    w %= 7;

    return w;
}

void ClockWidget::setupClock(BitmapId clockId)
{
    setXY(0, 0);
    setBackground(clockId, HAL::DISPLAY_WIDTH / 2, HAL::DISPLAY_HEIGHT / 2);

#ifdef SIMULATOR
    SYSTEMTIME sm;
    GetLocalTime(&sm);
#else
    hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        LOG_E(tgfx, "hal_rtc_get_time = %d", ret);
        configASSERT(0);
        return;
    }
    LOG_I(tgfx, "rtc_year = %d, rtc_mon = %d, rtc_day = %d\r\n", rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day);
    LOG_I(tgfx, "rtc_hour = %d, rtc_min = %d, rtc_sec = %d\r\n", rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec);
#endif

#ifdef SIMULATOR
    Unicode::snprintf(yearValueBuffer, 5, "%d", sm.wYear);
#else
    Unicode::snprintf(yearValueBuffer, 5, "%d", rtc_time.rtc_year + 2012);
#endif
    year.setWildcard(yearValueBuffer);
    year.setTypedText(TypedText(T_CLOCK_YEARS));
    year.setColor(Color::getColorFrom24BitRGB(0x99, 0x9A, 0x94));
    year.setXY(CommonUI::CLOCK_YEAR_X, CommonUI::CLOCK_YEAR_Y);
    add(year);

    TEXTS monthID[12] = {
        T_CLOCK_MONTHS_1, T_CLOCK_MONTHS_2, T_CLOCK_MONTHS_3, T_CLOCK_MONTHS_4, T_CLOCK_MONTHS_5, T_CLOCK_MONTHS_6,
        T_CLOCK_MONTHS_7, T_CLOCK_MONTHS_8, T_CLOCK_MONTHS_9, T_CLOCK_MONTHS_10, T_CLOCK_MONTHS_11, T_CLOCK_MONTHS_12
    };
#ifdef SIMULATOR
    Unicode::snprintf(monthValueBuffer, 4, "%s", TypedText(monthID[sm.wMonth - 1]).getText());
#else
    Unicode::snprintf(monthValueBuffer, 4, "%s", TypedText(monthID[rtc_time.rtc_mon - 1]).getText());
#endif
    month.setWildcard(monthValueBuffer);
    month.setTypedText(TypedText(T_CLOCK_MONTHS));
    month.setColor(Color::getColorFrom24BitRGB(0xEE, 0xEC, 0xEA));
    month.setXY(CommonUI::CLOCK_MONTH_X, CommonUI::CLOCK_MONTH_Y);
    add(month);

#ifdef SIMULATOR
    Unicode::snprintf(dayValueBuffer, 3, "%02d", sm.wDay);
#else
    Unicode::snprintf(dayValueBuffer, 3, "%02d", rtc_time.rtc_day);
#endif
    day.setWildcard(dayValueBuffer);
    day.setTypedText(TypedText(T_CLOCK_DAYS));
    day.setColor(Color::getColorFrom24BitRGB(0xEE, 0xEC, 0xEA));
    day.setXY(CommonUI::CLOCK_DAY_X, CommonUI::CLOCK_DAY_Y);
    add(day);

    BatteryImage.setBitmap(Bitmap(BITMAP_ICON_BATTERY_INT_ID));
    BatteryImage.setXY(170, 252);
    add(BatteryImage);

    ChargerImage.setBitmap(Bitmap(BITMAP_ICON_BATTERY_CHARGING_INT_ID));
    ChargerImage.setXY(170, 252);
    add(ChargerImage);

    int batteryLevel = CommonService::getCapacityCurrentPercentage();
    Unicode::snprintf(batteryValueBuffer, 5, "%d", batteryLevel);
    battery.setWildcard(batteryValueBuffer);
    battery.setTypedText(TypedText(T_CLOCK_BATTERY));
    battery.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    battery.setPosition(198, 250, 60, 33);
    add(battery);

    if ((1 == CommonService::getChargerStatus()) && (100 != batteryLevel)) {
        BatteryImage.setVisible(false);
        ChargerImage.setVisible(true);
        ChargerImage.invalidate();
    } else {
        BatteryImage.setVisible(true);
        ChargerImage.setVisible(false);
        BatteryImage.invalidate();
    }

    setupHourHand(BITMAP_CLOCK_HOUR_HAND_INT_ID, 12, 103);
    setupMinuteHand(BITMAP_CLOCK_MINUTE_HAND_INT_ID, 12, 179);
    setupSecondHand(BITMAP_CLOCK_SECOND_HAND_INT_ID, 12, 193);
    setHourHandMinuteCorrection(true); // The hour hand will move towards the next hour value as the minute hand moves towards 60.
    setMinuteHandSecondCorrection(false);

#ifdef SIMULATOR
    // Set start time for the analog clock. InitializeTime24Hour is
    // used instead of setTime24 hour to avoid animation of the hands.
    initializeTime24Hour((uint8_t)sm.wHour, (uint8_t)sm.wMinute, (uint8_t)sm.wSecond);
    handleTimeUpdated((uint8_t)sm.wHour, (uint8_t)sm.wMinute, (uint8_t)sm.wSecond);
#else
    // Set start time for the analog clock. InitializeTime24Hour is
    // used instead of setTime24 hour to avoid animation of the hands.
    initializeTime24Hour((uint8_t)rtc_time.rtc_hour, (uint8_t)rtc_time.rtc_min, (uint8_t)rtc_time.rtc_sec);
    handleTimeUpdated((uint8_t)rtc_time.rtc_hour, (uint8_t)rtc_time.rtc_min, (uint8_t)rtc_time.rtc_sec);
#endif
}

void ClockWidget::handleTimeUpdated(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
#ifdef SIMULATOR
    SYSTEMTIME sm;
    GetLocalTime(&sm);
#else
    hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        configASSERT(0);
        return;
    }
#endif

    if (CommonService::getClockStyle() == CommonService::ANALOG_CLOCK) {
        setTime24Hour(hours, minutes, seconds);

#ifdef SIMULATOR
        Unicode::snprintf(yearValueBuffer, 5, "%d", sm.wYear);
#else
        Unicode::snprintf(yearValueBuffer, 5, "%d", rtc_time.rtc_year + 2012);
#endif
        year.invalidate();

        TEXTS monthID[12] = {
            T_CLOCK_MONTHS_1, T_CLOCK_MONTHS_2, T_CLOCK_MONTHS_3, T_CLOCK_MONTHS_4, T_CLOCK_MONTHS_5, T_CLOCK_MONTHS_6,
            T_CLOCK_MONTHS_7, T_CLOCK_MONTHS_8, T_CLOCK_MONTHS_9, T_CLOCK_MONTHS_10, T_CLOCK_MONTHS_11, T_CLOCK_MONTHS_12
        };
#ifdef SIMULATOR
        Unicode::snprintf(monthValueBuffer, 4, "%s", TypedText(monthID[sm.wMonth - 1]).getText());
#else
        Unicode::snprintf(monthValueBuffer, 4, "%s", TypedText(monthID[rtc_time.rtc_mon - 1]).getText());
#endif
        month.invalidate();

#ifdef SIMULATOR
        Unicode::snprintf(dayValueBuffer, 3, "%02d", sm.wDay);
#else
        Unicode::snprintf(dayValueBuffer, 3, "%02d", rtc_time.rtc_day);
#endif
        day.invalidate();
    }
}

void ClockWidget::handleCapacityUpdated(int capacityData)
{
    int batteryLevel = capacityData;

#ifndef SIMULATOR
    //LOG_I(tgfx, "handleCapacityUpdated = %d", capacityData);
#endif

    if ((1 == CommonService::getChargerStatus()) && (100 != batteryLevel)) {
        BatteryImage.setVisible(false);
        ChargerImage.setVisible(true);
        ChargerImage.invalidate();
    } else {
        BatteryImage.setVisible(true);
        ChargerImage.setVisible(false);
        BatteryImage.invalidate();
    }

    Unicode::snprintf(batteryValueBuffer, 5, "%d", batteryLevel);
    if (batteryLevel == 100) {
        BatteryImage.moveTo(156, 252);
        ChargerImage.moveTo(156, 252);
        battery.moveTo(184, 250);
    } else if (batteryLevel < 100 && batteryLevel >= 10) {
        BatteryImage.moveTo(163, 252);
        ChargerImage.moveTo(163, 252);
        battery.moveTo(191, 250);
    } else if (batteryLevel < 10 && batteryLevel >= 0) {
        BatteryImage.moveTo(170, 252);
        ChargerImage.moveTo(170, 252);
        battery.moveTo(198, 250);
    }
    battery.invalidate();
}

void CompoundClockWidget::setupClock(BitmapId clockId)
{
    setXY(0, 0);
    setBackground(clockId, HAL::DISPLAY_WIDTH / 2, HAL::DISPLAY_HEIGHT / 2);

#ifdef SIMULATOR
    SYSTEMTIME sm;
    GetLocalTime(&sm);
#else
    hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        LOG_E(tgfx, "hal_rtc_get_time = %d", ret);
        configASSERT(0);
        return;
    }
    LOG_I(tgfx, "rtc_year = %d, rtc_mon = %d, rtc_day = %d\r\n", rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day);
    LOG_I(tgfx, "rtc_hour = %d, rtc_min = %d, rtc_sec = %d\r\n", rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec);
#endif

    handColorWhite.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));

    // Month-hand, Jan ~ Dec
    monthHand.setPosition(0, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);
    monthHand.setOrigin(HAL::DISPLAY_WIDTH / 2, HAL::DISPLAY_HEIGHT / 2);
    monthHand.setPainter(handColorWhite);
    add(monthHand);

    centerDot.setBitmap(Bitmap(BITMAP_COMPOUND_CLOCK_CENTER_DOT_INT_ID));
    centerDot.setXY((HAL::DISPLAY_WIDTH - centerDot.getWidth()) / 2, (HAL::DISPLAY_HEIGHT - centerDot.getHeight()) / 2);
    add(centerDot);

    // Week-hand, Sun ~ Sat
    weekHand.setPosition(0, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);
    weekHand.setOrigin(HAL::DISPLAY_WIDTH / 2, 278);
    weekHand.setPainter(handColorWhite);
    add(weekHand);

    centerDot2.setBitmap(Bitmap(BITMAP_COMPOUND_CLOCK_CENTER_DOT_INT_ID));
    centerDot2.setXY((HAL::DISPLAY_WIDTH - centerDot2.getWidth()) / 2, 278 - centerDot2.getHeight() / 2);
    add(centerDot2);

#ifdef SIMULATOR
    Unicode::snprintf(dayValueBuffer, 3, "%02d", sm.wDay);
#else
    Unicode::snprintf(dayValueBuffer, 3, "%02d", rtc_time.rtc_day);
#endif
    day.setWildcard(dayValueBuffer);
    day.setTypedText(TypedText(T_COMPOUNDCLOCK_DAYS));
    day.setColor(Color::getColorFrom24BitRGB(0x55, 0x55, 0x55));
    day.setXY(CommonUI::COMPOUNDCLOCK_DAY_X, CommonUI::COMPOUNDCLOCK_DAY_Y);
    add(day);

    setupHourHand(BITMAP_COMPOUND_CLOCK_HOUR_HAND_INT_ID, 14, 114);
    setupMinuteHand(BITMAP_COMPOUND_CLOCK_MINUTE_HAND_INT_ID, 14, 184);
    setupSecondHand(BITMAP_COMPOUND_CLOCK_SECOND_HAND_INT_ID, 11, 191);
    setHourHandMinuteCorrection(true); // The hour hand will move towards the next hour value as the minute hand moves towards 60.
    setMinuteHandSecondCorrection(false);

#ifdef SIMULATOR
    // Set start time for the analog clock. InitializeTime24Hour is
    // used instead of setTime24 hour to avoid animation of the hands.
    initializeTime24Hour((uint8_t)sm.wHour, (uint8_t)sm.wMinute, (uint8_t)sm.wSecond);
    handleTimeUpdated((uint8_t)sm.wHour, (uint8_t)sm.wMinute, (uint8_t)sm.wSecond);
#else
    // Set start time for the analog clock. InitializeTime24Hour is
    // used instead of setTime24 hour to avoid animation of the hands.
    initializeTime24Hour((uint8_t)rtc_time.rtc_hour, (uint8_t)rtc_time.rtc_min, (uint8_t)rtc_time.rtc_sec);
    handleTimeUpdated((uint8_t)rtc_time.rtc_hour, (uint8_t)rtc_time.rtc_min, (uint8_t)rtc_time.rtc_sec);
#endif
}

void CompoundClockWidget::handleTimeUpdated(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
#ifdef SIMULATOR
    SYSTEMTIME sm;
    GetLocalTime(&sm);
#else
    hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        configASSERT(0);
        return;
    }
    rtc_time.rtc_week = calcWeek(rtc_time.rtc_year + 2012, rtc_time.rtc_mon, rtc_time.rtc_day);
#endif

    if (CommonService::getClockStyle() == CommonService::COMPOUND_CLOCK) {
        setTime24Hour(hours, minutes, seconds);

        int monthAngle[12] = { 250, 270, 290, 310, 330, 350, 10, 30, 50, 70, 90, 110 };
#ifdef SIMULATOR
        monthHand.updateAngle(monthAngle[sm.wMonth - 1]);
#else
        monthHand.updateAngle(monthAngle[rtc_time.rtc_mon - 1]);
#endif

        int weekAngle[7] = { 0, 360 / 7, 360 * 2 / 7, 360 * 3 / 7, 360 * 4 / 7, 360 * 5 / 7, 360 * 6 / 7 };
#ifdef SIMULATOR
        weekHand.updateAngle(weekAngle[sm.wDayOfWeek]);
#else
        weekHand.updateAngle(weekAngle[rtc_time.rtc_week]);
#endif

#ifdef SIMULATOR
        Unicode::snprintf(dayValueBuffer, 3, "%02d", sm.wDay);
#else
        Unicode::snprintf(dayValueBuffer, 3, "%02d", rtc_time.rtc_day);
#endif
        day.invalidate();
    }
}

HomeView::VolumeLevel HomeView::levelToRestoreAfterMute = (HomeView::VolumeLevel) CommonService::getDefaultVolume();

HomeView::HomeView() :
    clickAbort(false),
    // Notification members
    listElementClickedCallback(this, &HomeView::listElementClicked),
    // Player members
    isPlaying(false),
    mute(false),
    level((VolumeLevel) CommonService::getDefaultVolume()),
    onButtonPressed(this, &HomeView::buttonPressedHandler)
{
#ifndef SIMULATOR
    //log_config_print_switch(hal, DEBUG_LOG_OFF);
    log_config_print_switch(atci, DEBUG_LOG_OFF);
    log_config_print_switch(bmt_demo, DEBUG_LOG_OFF);
    log_config_print_switch(GNSS_TAG, DEBUG_LOG_OFF);
    log_config_print_switch(sensor, DEBUG_LOG_OFF);
    log_config_print_switch(mt2511_spi_driver, DEBUG_LOG_OFF);
    log_config_print_switch(mt2511_driver, DEBUG_LOG_OFF);
    log_config_print_switch(BTMM, DEBUG_LOG_OFF);
    log_config_print_switch(BTHCI, DEBUG_LOG_OFF);
    //log_config_print_switch(BT, DEBUG_LOG_OFF);
    log_config_print_switch(NOTIFY, DEBUG_LOG_OFF);
    log_config_print_switch(NOTIFY_SRV, DEBUG_LOG_OFF);
    log_config_print_switch(BTRFCOMM, DEBUG_LOG_OFF);
    log_config_print_switch(BTSPP, DEBUG_LOG_OFF);
    log_config_print_switch(BTL2CAP, DEBUG_LOG_OFF);
    log_config_print_switch(DOGP_CM, DEBUG_LOG_OFF);
    log_config_print_switch(DOGP_ADP, DEBUG_LOG_OFF);
    log_config_print_switch(DOGP, DEBUG_LOG_OFF);
    //log_config_print_switch(tgfx, DEBUG_LOG_OFF);
    //log_config_print_switch(source_srv, DEBUG_LOG_OFF);
    //log_config_print_switch(mp3ui, DEBUG_LOG_OFF);
#endif
}

void HomeView::setupScreen()
{
    //backgroundLeft.setBitmap(Bitmap(BITMAP_MENU_STAGE_STRETCH_LEFT_SIDE_INT_ID));
    //backgroundLeft.setXY(0, 0);
    //add(backgroundLeft);

    //backgroundRight.setBitmap(Bitmap(BITMAP_MENU_STAGE_STRETCH_RIGHT_SIDE_INT_ID));
    //backgroundRight.setXY(HAL::DISPLAY_WIDTH - backgroundRight.getWidth(), 0);
    //add(backgroundRight);

    menuContainer.setXY(0, 0);
    menuContainer.setEndSwipeElasticWidth(0);
    menuContainer.setTouchable(false);

#ifndef SIMULATOR
    LOG_I(tgfx, "start uncompress %d\r\n", CommonService::getLogTimeInMS());
#endif

    for (int i = HomePresenter::NOTIFICATION; i < NUMBER_OF_DEMO_SCREENS; i++) {
        demoScreensContainer[i].setPosition(0, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);

        if (i == HomePresenter::NOTIFICATION) {
            addNotificationScreen();
        } else if (i == HomePresenter::CLOCK_FACE) {
            addClockFaceScreen();
        } else if (i == HomePresenter::PLAYER) {
            addPlayerScreen();
        }

        menuContainer.add(demoScreensContainer[i]);
    }

#ifndef SIMULATOR
    LOG_I(tgfx, "end uncompress %d\r\n", CommonService::getLogTimeInMS());
#endif

    add(menuContainer);

    demoScreenIndex[0] = HomePresenter::NOTIFICATION;
    demoScreenIndex[1] = HomePresenter::CLOCK_FACE;
    demoScreenIndex[2] = HomePresenter::PLAYER;

    CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
    CommonService::setBPTipsCount(10);
    dt_set_start_transfer_flag(false);
}

void HomeView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (commonId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(commonId);
    }
    if (clockId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(clockId);
    }
    if (clock2Id != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(clock2Id);
    }
    if (menuId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(menuId);
    }
    if (pauseId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(pauseId);
    }
    if (playId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(playId);
    }
    if (previousId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(previousId);
    }
    if (nextId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(nextId);
    }
    for (int i = 0; i < MAX_VOLUME_LEVEL; i++) {
        if (volumeId[i] != BITMAP_INVALID) {
            Bitmap::dynamicBitmapDelete(volumeId[i]);
        }
    }
#endif
    CommonService::stopMP3();
}

void HomeView::afterTransition()
{
    menuContainer.setTouchable(true);
}

void HomeView::handleClickEvent(const ClickEvent &evt)
{
    DemoView::handleClickEvent(evt);

#if SIMULATOR
    if (evt.getType() == ClickEvent::RELEASED) {
        if (clickAbort == false) {
            gotoSelectedDemoScreen(menuContainer.getSelectedScreen());
        } else {
            clickAbort = false;
        }
    }
#endif
}

void HomeView::handleDragEvent(const DragEvent &evt)
{
    DemoView::handleDragEvent(evt);

    if (abs(evt.getDeltaX()) > 0 || abs(evt.getDeltaY()) > 0) {
        clickAbort = true;
    }
}

void HomeView::handleTickEvent()
{
#ifndef SIMULATOR
    if (bt_notify_app_list_have_update() == true) {
        refreshNotificationList();
        bt_notify_app_list_set_update_flag(false);
    } else if (CommonService::hasAvrcpOp() != CommonService::AVRCP_OP_IDLE) {
        refreshPlayerScreen();
        CommonService::resetAvrcpOp();
    } else if (CommonService::hasTrackChange() == true) {
        refreshSong();
        CommonService::resetTrackChange();
    }
#endif
}

void HomeView::handleKeyEvent(uint8_t key)
{
#if 0
    if (key == 2) {
        refreshNotificationList();
        return;
    }
#endif

    if (CommonService::isBacklight() == true) {
        DemoView::handleKeyEvent(key);
        return;
    }

    DemoView::handleKeyEvent(key);

    switch (menuContainer.getSelectedScreen()) {
        case 1:
            presenter->setSelectedHomeIndex(1);
            presenter->gotoOptionMenuSelected();
            break;
        case 0:
        case 2:
            presenter->setSelectedHomeIndex(1);
            presenter->backOptionHomeSelected();
            break;
        default:
            break;
    }
}

void HomeView::handleTimeUpdated(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (CommonService::getClockStyle() == CommonService::ANALOG_CLOCK) {
        analogClock.handleTimeUpdated(hours, minutes, seconds);
    } else if (CommonService::getClockStyle() == CommonService::DIGITAL_CLOCK) {
#ifdef SIMULATOR
        SYSTEMTIME sm;
        GetLocalTime(&sm);
#else
        hal_rtc_time_t rtc_time;
        hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
        if (HAL_RTC_STATUS_OK != ret) {
            configASSERT(0);
            return;
        }
        rtc_time.rtc_week = calcWeek(rtc_time.rtc_year + 2012, rtc_time.rtc_mon, rtc_time.rtc_day);
#endif
        TEXTS weekID[7] = {
            T_DIGITALCLOCK_SUNDAY, T_DIGITALCLOCK_MONDAY, T_DIGITALCLOCK_TUESDAY, T_DIGITALCLOCK_WEDNESDAY, T_DIGITALCLOCK_THURSDAY, T_DIGITALCLOCK_FRIDAY,
            T_DIGITALCLOCK_SATURDAY
        };
#ifdef SIMULATOR
        Unicode::snprintf(weekValueBuffer, 10, "%s", TypedText(weekID[sm.wDayOfWeek]).getText());
#else
        Unicode::snprintf(weekValueBuffer, 10, "%s", TypedText(weekID[rtc_time.rtc_week]).getText());
#endif
        week.invalidate();

#ifdef SIMULATOR
        Unicode::snprintf(timeValueBuffer, 6, "%02d:%02d", sm.wHour, sm.wMinute);
#else
        Unicode::snprintf(timeValueBuffer, 6, "%02d:%02d", rtc_time.rtc_hour, rtc_time.rtc_min);
#endif
        time.invalidate();

        TEXTS monthID[12] = {
            T_CLOCK_MONTHS_1, T_CLOCK_MONTHS_2, T_CLOCK_MONTHS_3, T_CLOCK_MONTHS_4, T_CLOCK_MONTHS_5, T_CLOCK_MONTHS_6,
            T_CLOCK_MONTHS_7, T_CLOCK_MONTHS_8, T_CLOCK_MONTHS_9, T_CLOCK_MONTHS_10, T_CLOCK_MONTHS_11, T_CLOCK_MONTHS_12
        };
#ifdef SIMULATOR
        Unicode::snprintf(dateValueBuffer, 13, "%s.%02d, %d", TypedText(monthID[sm.wMonth - 1]).getText(), sm.wDay, sm.wYear);
#else
        Unicode::snprintf(dateValueBuffer, 13, "%s.%02d, %d", TypedText(monthID[rtc_time.rtc_mon - 1]).getText(), rtc_time.rtc_day, rtc_time.rtc_year + 2012);
#endif
        date.invalidate();
    } else if (CommonService::getClockStyle() == CommonService::COMPOUND_CLOCK) {
        analogClock2.handleTimeUpdated(hours, minutes, seconds);
    }
}

void HomeView::handleCapacityUpdated(int capacityData)
{
    if (CommonService::getClockStyle() == CommonService::ANALOG_CLOCK) {
        analogClock.handleCapacityUpdated(capacityData);
    } else if (CommonService::getClockStyle() == CommonService::DIGITAL_CLOCK) {
        int batteryLevel = capacityData;

#ifndef SIMULATOR
        //LOG_I(tgfx, "handleCapacityUpdated = %d, status = %d", capacityData, CommonService::getChargerStatus());
#endif

        if ((1 == CommonService::getChargerStatus()) && (100 != batteryLevel)) {
            batteryIcon.setVisible(false);
            chargerIcon.setVisible(true);
            chargerIcon.invalidate();
        } else {
            batteryIcon.setVisible(true);
            chargerIcon.setVisible(false);
            batteryIcon.invalidate();
        }

        Unicode::snprintf(percentValueBuffer, 5, "%d", batteryLevel);
        percent.invalidate();
    }
}

void HomeView::gotoSelectedDemoScreen(uint8_t demoIndex)
{
    presenter->demoSelected(demoScreenIndex[demoIndex], demoIndex);
}

void HomeView::setSelectedHomeIndex(uint8_t demoIndex)
{
    menuContainer.setSelectedScreen(demoIndex);
}

static uint32_t list_count = 0;
#ifndef SIMULATOR
static bt_notify_app_remote_system_t remote_type[BT_NOTIFY_APP_LIST_MAX_NUMBER];
#endif

void HomeView::refreshNotificationList()
{
#ifndef SIMULATOR
    Unicode::UnicodeChar truncate[30] = { 0 };
    Unicode::UnicodeChar tail[30] = { '.', '.', '.' };

    LOG_I(NOTIFY_APP, "notification_list: %d\r\n", list_count);

    if (list_count > 0) {
        for (uint8_t i = 0; i < list_count; ++i) {
            LOG_I(NOTIFY_APP, "remote_type[i]: %d\r\n", remote_type[i]);
            if (remote_type[i] == 1) {
                listElements[i].removeListElement(i, Bitmap(BITMAP_ICON_ANDROID_INT_ID), &notiTitle[i]);
            } else {
                listElements[i].removeListElement(i, Bitmap(BITMAP_ICON_IOS_INT_ID), &notiTitle[i]);
            }
        }
        for (uint8_t i = 0; i < list_count + 1; ++i) {
            list.remove(listElements[i]);
        }
    }

    list_count = bt_notify_app_get_list_item_count();

    LOG_I(NOTIFY_APP, "list_count: %d\r\n", list_count);

    if (list_count == 0) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 0);
        return;
    }

    bt_notify_app_list_t *noti_list = bt_notify_app_get_list();

    for (uint32_t i = 0; i < list_count; i++) {
        int array_size = bt_notify_get_arry_size(noti_list[i].title);

        LOG_I(NOTIFY_APP, "array_size: %d\r\n", array_size);

        if (array_size > 11) {
            Unicode::strncpy(truncate, noti_list[i].title, 11);
            Unicode::snprintf(notiTitleBuffer[i], 30, "%s%s", truncate, tail);
        } else {
            Unicode::strncpy(notiTitleBuffer[i], noti_list[i].title, 30);

            LOG_I(NOTIFY_APP, "notification_list->title: %s\r\n", noti_list[i].title);
            LOG_I(NOTIFY_APP, "notification_list->notiTitleBuffer[i]: %s\r\n", notiTitleBuffer[i]);
        }

        notiTitle[i].setWildcard(notiTitleBuffer[i]);
        notiTitle[i].setTypedText(TypedText(T_NOTIFICATIONS_LIST));

        LOG_I(NOTIFY_APP, "noti_list[i].remote_system: %d\r\n", noti_list[i].remote_system);
        remote_type[i] = noti_list[i].remote_system;

        if (noti_list[i].remote_system == 1) { //Android
            listElements[i].setupListElement(i, Bitmap(menuId), Bitmap(BITMAP_ICON_ANDROID_INT_ID), &notiTitle[i]);
        } else { //IOS
            listElements[i].setupListElement(i, Bitmap(menuId), Bitmap(BITMAP_ICON_IOS_INT_ID), &notiTitle[i]);
        }
    }
    listElements[list_count].setupLastElement();
#else
    Unicode::UnicodeChar title[30] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'D', 'S', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    Unicode::UnicodeChar truncate[30] = { 0 };
    Unicode::UnicodeChar tail[30] = { '.', '.', '.' };

    if (list_count > 0) {
        for (uint8_t i = 0; i < list_count; ++i) {
            listElements[i].removeListElement(i, Bitmap(BITMAP_ICON_ANDROID_INT_ID), &notiTitle[i]);
        }
        for (uint8_t i = 0; i < list_count + 1; ++i) {
            list.remove(listElements[i]);
        }
    }

    list_count = 5;
    if (list_count == 0) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 0);
        return;
    }

    for (uint32_t i = 0; i < list_count; i++) {
        if (i == 0) {
            Unicode::strncpy(truncate, title, 11);
            Unicode::snprintf(notiTitleBuffer[i], 30, "%s%s", truncate, tail);
        } else {
            Unicode::strncpy(notiTitleBuffer[i], "skhdjksdf", 30);
        }

        notiTitle[i].setWildcard(notiTitleBuffer[i]);
        notiTitle[i].setTypedText(TypedText(T_NOTIFICATIONS_LIST));

        listElements[i].setupListElement(i, Bitmap(menuId), Bitmap(BITMAP_ICON_ANDROID_INT_ID), &notiTitle[i]);
    }
    listElements[list_count].setupLastElement();
#endif

    for (uint8_t i = 0; i < list_count + 1; ++i) {
        listElements[i].setAction(listElementClickedCallback);
        list.add(listElements[i]);
    }

    if (list_count == 1) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, CommonUI::CLIENT_ITEM_HEIGHT + CommonUI::CLIENT_ITEM_GAP);
    } else if (list_count == 2) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, 2 * CommonUI::CLIENT_ITEM_HEIGHT + 2 * CommonUI::CLIENT_ITEM_GAP);
    } else if (list_count >= 3) {
        scrollCnt.setPosition(0, CommonUI::TITLE_HEIGHT + CommonUI::CLIENT_Y, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT - 2 * CommonUI::TITLE_HEIGHT - 2 * CommonUI::CLIENT_Y);
    }
    demoScreensContainer[HomePresenter::NOTIFICATION].invalidate();
}

void HomeView::addNotificationScreen()
{
    int notifIndex = HomePresenter::NOTIFICATION;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[notifIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    const DynamicBitmapDatabase::BitmapInfo commonInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_COMMON_BACKGROUND_INT_ID);
    //create dynamic bitmap matching file dimensions
    commonId = Bitmap::dynamicBitmapCreate(commonInfo.width, commonInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", commonId);
    if (commonId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(commonId);
        uint32_t destLen = commonInfo.width * commonInfo.height * 2;
        uint32_t srcLen = commonInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, commonInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        backgroundImage[notifIndex].setBitmap(Bitmap(commonId));
    }
#endif
    backgroundImage[notifIndex].setXY(0, 0);
    demoScreensContainer[notifIndex].add(backgroundImage[notifIndex]);

    // Add title
    titleTxt.setTypedText(TypedText(T_NOTIFICATIONS_TITLE));
    titleTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    titleTxt.setPosition(0, CommonUI::TITLE_Y, HAL::DISPLAY_WIDTH, CommonUI::TITLE_FONT_HEIGHT);
    titleTxt.setAlpha(127);

#ifndef DYNAMIC_BITMAP_LOADER
    menuId = BITMAP_LIST_MENU_BACKGROUND_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo menuInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_LIST_MENU_BACKGROUND_INT_ID);
    //create dynamic bitmap matching file dimensions
    menuId = Bitmap::dynamicBitmapCreate(menuInfo.width, menuInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", menuId);
    if (menuId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(menuId);
        uint32_t destLen = menuInfo.width * menuInfo.height * 2;
        uint32_t srcLen = menuInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, menuInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif

    // Initialize list elements
    refreshNotificationList();

    // Position and set the size of the scrollable container.
    // The width is the area is the list element width plus some extra
    // for space between element and scrollbar
    scrollCnt.add(list);

    // Remember to add widgets to container.
    // They must be added from bottom and out, or else upper layer(s)
    // may block view of elements below.
    demoScreensContainer[notifIndex].add(titleTxt);
    demoScreensContainer[notifIndex].add(scrollCnt);
}

void HomeView::listElementClicked(MenuListElement &element)
{
#ifndef SIMULATOR
    bt_notify_app_list_set_selected_index(element.getMenuIndex());

    LOG_I(NOTIFY_APP, "element.getMenuIndex(): %d\r\n", element.getMenuIndex());
#endif

    presenter->menuSelected(element.getMenuIndex());
}

void HomeView::addClockFaceScreen()
{
    switch (CommonService::getClockStyle()) {
        case CommonService::ANALOG_CLOCK:
            addClockScreen();
            break;
        case CommonService::DIGITAL_CLOCK:
            addDigitalClockScreen();
            break;
        case CommonService::COMPOUND_CLOCK:
            addCompoundClockScreen();
            break;
        default:
            break;
    }
}

void HomeView::addClockScreen()
{
    int clockIndex = HomePresenter::CLOCK_FACE;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    analogClock.setupClock(BITMAP_HOME_CLOCK_BACKGROUND_INT_ID);
#else
    const DynamicBitmapDatabase::BitmapInfo clockInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_HOME_CLOCK_BACKGROUND_INT_ID);
    //create dynamic bitmap matching file dimensions
    clockId = Bitmap::dynamicBitmapCreate(clockInfo.width, clockInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", clockId);
    if (clockId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(clockId);
        uint32_t destLen = clockInfo.width * clockInfo.height * 2;
        uint32_t srcLen = clockInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, clockInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        analogClock.setupClock(clockId);
    }
#endif
    demoScreensContainer[clockIndex].add(analogClock);
}

void HomeView::addDigitalClockScreen()
{
    int clockIndex = HomePresenter::CLOCK_FACE;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[clockIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[clockIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[clockIndex].setXY(0, 0);
    demoScreensContainer[clockIndex].add(backgroundImage[clockIndex]);

#ifdef SIMULATOR
    SYSTEMTIME sm;
    GetLocalTime(&sm);
#else
    hal_rtc_time_t rtc_time;
    hal_rtc_status_t ret = hal_rtc_get_time(&rtc_time);
    if (HAL_RTC_STATUS_OK != ret) {
        LOG_E(tgfx, "hal_rtc_get_time = %d", ret);
        configASSERT(0);
        return;
    }
    rtc_time.rtc_week = calcWeek(rtc_time.rtc_year + 2012, rtc_time.rtc_mon, rtc_time.rtc_day);
    LOG_I(tgfx, "rtc_year = %d, rtc_mon = %d, rtc_day = %d\r\n", rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day);
    LOG_I(tgfx, "rtc_hour = %d, rtc_min = %d, rtc_week = %d\r\n", rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_week);
    LOG_I(tgfx, "calcWeek = %d\r\n", rtc_time.rtc_week);
#endif

    batteryIcon.setBitmap(Bitmap(BITMAP_ICON_BATTERY_SMALL_INT_ID));
    batteryIcon.setXY(CommonUI::DIGITALCLOCK_CHARGER_X, CommonUI::DIGITALCLOCK_CHARGER_Y);
    demoScreensContainer[clockIndex].add(batteryIcon);

    chargerIcon.setBitmap(Bitmap(BITMAP_ICON_BATTERY_CHARGING_SMALL_INT_ID));
    chargerIcon.setXY(CommonUI::DIGITALCLOCK_CHARGER_X, CommonUI::DIGITALCLOCK_CHARGER_Y);
    demoScreensContainer[clockIndex].add(chargerIcon);

    int batteryLevel = CommonService::getCapacityCurrentPercentage();
    Unicode::snprintf(percentValueBuffer, 5, "%d", batteryLevel);
    percent.setWildcard(percentValueBuffer);
    percent.setTypedText(TypedText(T_DIGITALCLOCK_PERCENT));
    percent.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    percent.setPosition(CommonUI::DIGITALCLOCK_PERCENT_X, CommonUI::DIGITALCLOCK_PERCENT_Y, CommonUI::DIGITALCLOCK_PERCENT_WIDTH, CommonUI::DIGITALCLOCK_PERCENT_HEIGHT);
    demoScreensContainer[clockIndex].add(percent);

    if ((1 == CommonService::getChargerStatus()) && (100 != batteryLevel)) {
        batteryIcon.setVisible(false);
        chargerIcon.setVisible(true);
        chargerIcon.invalidate();
    } else {
        batteryIcon.setVisible(true);
        chargerIcon.setVisible(false);
        batteryIcon.invalidate();
    }

    TEXTS weekID[7] = {
        T_DIGITALCLOCK_SUNDAY, T_DIGITALCLOCK_MONDAY, T_DIGITALCLOCK_TUESDAY, T_DIGITALCLOCK_WEDNESDAY, T_DIGITALCLOCK_THURSDAY, T_DIGITALCLOCK_FRIDAY,
        T_DIGITALCLOCK_SATURDAY
    };
#ifdef SIMULATOR
    Unicode::snprintf(weekValueBuffer, 10, "%s", TypedText(weekID[sm.wDayOfWeek]).getText());
#else
    Unicode::snprintf(weekValueBuffer, 10, "%s", TypedText(weekID[rtc_time.rtc_week]).getText());
#endif
    week.setWildcard(weekValueBuffer);
    week.setTypedText(TypedText(T_DIGITALCLOCK_WEEK));
    week.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    week.setXY(CommonUI::DIGITALCLOCK_WEEK_X, CommonUI::DIGITALCLOCK_WEEK_Y);
    demoScreensContainer[clockIndex].add(week);

#ifdef SIMULATOR
    Unicode::snprintf(timeValueBuffer, 6, "%02d:%02d", sm.wHour, sm.wMinute);
#else
    Unicode::snprintf(timeValueBuffer, 6, "%02d:%02d", rtc_time.rtc_hour, rtc_time.rtc_min);
#endif
    time.setWildcard(timeValueBuffer);
    time.setTypedText(TypedText(T_DIGITALCLOCK_TIME));
    time.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    time.setXY(CommonUI::DIGITALCLOCK_TIME_X, CommonUI::DIGITALCLOCK_TIME_Y);
    demoScreensContainer[clockIndex].add(time);

    TEXTS monthID[12] = {
        T_CLOCK_MONTHS_1, T_CLOCK_MONTHS_2, T_CLOCK_MONTHS_3, T_CLOCK_MONTHS_4, T_CLOCK_MONTHS_5, T_CLOCK_MONTHS_6,
        T_CLOCK_MONTHS_7, T_CLOCK_MONTHS_8, T_CLOCK_MONTHS_9, T_CLOCK_MONTHS_10, T_CLOCK_MONTHS_11, T_CLOCK_MONTHS_12
    };
#ifdef SIMULATOR
    Unicode::snprintf(dateValueBuffer, 13, "%s.%02d, %d", TypedText(monthID[sm.wMonth - 1]).getText(), sm.wDay, sm.wYear);
#else
    Unicode::snprintf(dateValueBuffer, 13, "%s.%02d, %d", TypedText(monthID[rtc_time.rtc_mon - 1]).getText(), rtc_time.rtc_day, rtc_time.rtc_year + 2012);
#endif
    date.setWildcard(dateValueBuffer);
    date.setTypedText(TypedText(T_DIGITALCLOCK_DATE));
    date.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    date.setXY(CommonUI::DIGITALCLOCK_DATE_X, CommonUI::DIGITALCLOCK_DATE_Y);
    demoScreensContainer[clockIndex].add(date);
}

void HomeView::addCompoundClockScreen()
{
    int clockIndex = HomePresenter::CLOCK_FACE;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    analogClock2.setupClock(BITMAP_HOME_COMPOUND_CLOCK_BACKGROUND_INT_ID);
#else
    const DynamicBitmapDatabase::BitmapInfo clock2Info = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_HOME_COMPOUND_CLOCK_BACKGROUND_INT_ID);
    //create dynamic bitmap matching file dimensions
    clock2Id = Bitmap::dynamicBitmapCreate(clock2Info.width, clock2Info.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", clock2Id);
    if (clock2Id != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(clock2Id);
        uint32_t destLen = clock2Info.width * clock2Info.height * 2;
        uint32_t srcLen = clock2Info.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, clock2Info.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        analogClock2.setupClock(clock2Id);
    }
#endif
    demoScreensContainer[clockIndex].add(analogClock2);
}

void HomeView::addPlayerScreen()
{
    int playerIndex = HomePresenter::PLAYER;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[playerIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[playerIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[playerIndex].setXY(0, 0);
    demoScreensContainer[playerIndex].add(backgroundImage[playerIndex]);

    Unicode::UnicodeChar truncate1[SONG_MAX_LEN_PER_LINE + 1] = { 0 };
    Unicode::UnicodeChar truncate2[SONG_MAX_LEN_PER_LINE + 1] = { 0 };
    Unicode::UnicodeChar lf[2] = { '\n' };
    Unicode::UnicodeChar tail[4] = { '.', '.', '.' };
    char *name = CommonService::getMP3Song();
    if (strlen(name) > SONG_MAX_LEN_PER_LINE)
    {
        Unicode::strncpy(truncate1, name, SONG_MAX_LEN_PER_LINE);
        Unicode::strncpy(truncate2, name + SONG_MAX_LEN_PER_LINE, SONG_MAX_LEN_PER_LINE);
        Unicode::snprintf(songValueBuffer, 40, "%s%s%s%s", truncate1, lf, truncate2, tail);
    } else {
        Unicode::strncpy(songValueBuffer, name, 40);
    }
    song.setWildcard(songValueBuffer);
    song.setTypedText(TypedText(T_PLAYER_SONG_NAME));
    song.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    song.setPosition(CommonUI::PLAYER_SONG_NAME_X, CommonUI::PLAYER_SONG_NAME_Y, CommonUI::PLAYER_SONG_NAME_WIDTH, 30 + CommonUI::PLAYER_SONG_NAME_HEIGHT);
    demoScreensContainer[playerIndex].add(song);

#ifndef DYNAMIC_BITMAP_LOADER
    pauseId = BITMAP_PLAYER_PAUSE_INT_ID;
#else
    pauseId = BITMAP_INVALID;
#endif

#ifndef DYNAMIC_BITMAP_LOADER
    playId = BITMAP_PLAYER_PLAY_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo playInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_PLAYER_PLAY_INT_ID);
    //create dynamic bitmap matching file dimensions
    playId = Bitmap::dynamicBitmapCreate(playInfo.width, playInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", playId);
    if (playId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(playId);
        uint32_t destLen = playInfo.width * playInfo.height * 2;
        uint32_t srcLen = playInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, playInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif
    play.setBitmaps(Bitmap(playId), Bitmap(playId));
    play.setXY(CommonUI::PLAYER_PLAY_BUTTON_X, CommonUI::PLAYER_PLAY_BUTTON_Y);
    play.setAction(onButtonPressed);
    demoScreensContainer[playerIndex].add(play);

#ifndef DYNAMIC_BITMAP_LOADER
    previousId = BITMAP_PLAYER_SKIP_PREVIOUS_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo previousInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_PLAYER_SKIP_PREVIOUS_INT_ID);
    //create dynamic bitmap matching file dimensions
    previousId = Bitmap::dynamicBitmapCreate(previousInfo.width, previousInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", previousId);
    if (previousId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(previousId);
        uint32_t destLen = previousInfo.width * previousInfo.height * 2;
        uint32_t srcLen = previousInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, previousInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif
    previous.setBitmaps(Bitmap(previousId), Bitmap(previousId));
    previous.setXY(CommonUI::PLAYER_PREVIOUS_BUTTON_X, CommonUI::PLAYER_PREVIOUS_BUTTON_Y);
    previous.setAction(onButtonPressed);
    demoScreensContainer[playerIndex].add(previous);

#ifndef DYNAMIC_BITMAP_LOADER
    nextId = BITMAP_PLAYER_SKIP_NEXT_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo nextInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_PLAYER_SKIP_NEXT_INT_ID);
    //create dynamic bitmap matching file dimensions
    nextId = Bitmap::dynamicBitmapCreate(nextInfo.width, nextInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", nextId);
    if (nextId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(nextId);
        uint32_t destLen = nextInfo.width * nextInfo.height * 2;
        uint32_t srcLen = nextInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, nextInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif
    next.setBitmaps(Bitmap(nextId), Bitmap(nextId));
    next.setXY(CommonUI::PLAYER_NEXT_BUTTON_X, CommonUI::PLAYER_NEXT_BUTTON_Y);
    next.setAction(onButtonPressed);
    demoScreensContainer[playerIndex].add(next);

#ifndef DYNAMIC_BITMAP_LOADER
    uint16_t volumeID[MAX_VOLUME_LEVEL] = { BITMAP_PLAYER_VOLUME_BACKGROUND_INT_ID, BITMAP_PLAYER_VOLUME_1_INT_ID,
                                            BITMAP_PLAYER_VOLUME_2_INT_ID, BITMAP_PLAYER_VOLUME_3_INT_ID, BITMAP_PLAYER_VOLUME_4_INT_ID,
                                            BITMAP_PLAYER_VOLUME_5_INT_ID, BITMAP_PLAYER_VOLUME_6_INT_ID, BITMAP_PLAYER_VOLUME_7_INT_ID
                                          };
    for (int i = 0; i < MAX_VOLUME_LEVEL; i++) {
        volumeId[i] = volumeID[i];
    }
#else
    for (int i = 0; i < level; i++) {
        volumeId[i] = BITMAP_INVALID;
    }
    for (int i = level; i < MAX_VOLUME_LEVEL; i++) {
        volumeId[i] = BITMAP_INVALID;
    }
    uint16_t volumeID[MAX_VOLUME_LEVEL] = { DYNAMIC_BITMAP_PLAYER_VOLUME_BACKGROUND_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_1_INT_ID,
                                            DYNAMIC_BITMAP_PLAYER_VOLUME_2_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_3_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_4_INT_ID,
                                            DYNAMIC_BITMAP_PLAYER_VOLUME_5_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_6_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_7_INT_ID
                                          };
    const DynamicBitmapDatabase::BitmapInfo volumeInfo = DynamicBitmapDatabase::getInstanceInfo(volumeID[level]);
    //create dynamic bitmap matching file dimensions
    volumeId[level] = Bitmap::dynamicBitmapCreate(volumeInfo.width, volumeInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", volumeId[level]);
    if (volumeId[level] != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(volumeId[level]);
        uint32_t destLen = volumeInfo.width * volumeInfo.height * 2;
        uint32_t srcLen = volumeInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, volumeInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif
    volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
    volumeBar.setXY(CommonUI::PLAYER_VOLUME_BAR_X, CommonUI::PLAYER_VOLUME_BAR_Y);
    demoScreensContainer[playerIndex].add(volumeBar);

    if (level == VOLUME_LEVEL7) {
        plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID));
    } else {
        plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
    }
    plus.setXY(CommonUI::PLAYER_PLUS_BUTTON_X, CommonUI::PLAYER_PLUS_BUTTON_Y);
    plus.setAction(onButtonPressed);
    demoScreensContainer[playerIndex].add(plus);

    if (level == VOLUME_LEVEL0) {
        minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID));
    } else {
        minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID));
    }
    minus.setXY(CommonUI::PLAYER_MINUS_BUTTON_X, CommonUI::PLAYER_MINUS_BUTTON_Y);
    minus.setAction(onButtonPressed);
    demoScreensContainer[playerIndex].add(minus);

    if (level == VOLUME_LEVEL0) {
        mute = true;
        volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID));
    }else {
        mute = false;
        volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_INT_ID));
    }
    volume.setXY(CommonUI::PLAYER_VOLUME_BUTTON_X, CommonUI::PLAYER_VOLUME_BUTTON_Y);
    volume.setAction(onButtonPressed);
    demoScreensContainer[playerIndex].add(volume);
}

void HomeView::refreshPlayerScreen()
{
#ifndef SIMULATOR
    CommonService::AVRCPOp avrcpOp = CommonService::hasAvrcpOp();

    switch (avrcpOp) {
        case CommonService::AVRCP_OP_PLAY: {
            isPlaying = true;
            loadPauseId();
            play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
            CommonService::playMP3();
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_A2DP_PLAY: {
            isPlaying = true;
            loadPauseId();
            play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_PAUSE: {
            isPlaying = false;
            play.setBitmaps(Bitmap(playId), Bitmap(playId));
            CommonService::pauseMP3();
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_STOP: {
            isPlaying = false;
            play.setBitmaps(Bitmap(playId), Bitmap(playId));
            CommonService::stopMP3();
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_A2DP_STOP: {
            isPlaying = false;
            play.setBitmaps(Bitmap(playId), Bitmap(playId));
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_NEXT: {
            isPlaying = true;
            loadPauseId();
            play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
            CommonService::nextMP3();
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_PREVIOUS: {
            isPlaying = true;
            loadPauseId();
            play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
            CommonService::previousMP3();
            play.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_VOLUME_UP: {
            if (level < VOLUME_LEVEL7) {
                if (level == VOLUME_LEVEL0) {
                    mute = false;
                    volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_INT_ID));
                    volume.invalidate();
                }

                level = plusVolume(level);
                levelToRestoreAfterMute = level;
                if (level == VOLUME_LEVEL7) {
                    plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID));
                    plus.invalidate();
                }
                minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID));
                minus.invalidate();

                loadVolumeId(level);
                volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
                CommonService::volumeUp(level);
                volumeBar.invalidate();
            }
            break;
        }
        case CommonService::AVRCP_OP_VOLUME_DOWN: {
            if (level > VOLUME_LEVEL0) {
                level = minusVolume(level);
                if (level == VOLUME_LEVEL0) {
                    minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID));
                    minus.invalidate();
                    mute = true;
                    volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID));
                    volume.invalidate();
                } else {
                    levelToRestoreAfterMute = level;
                }
                plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
                plus.invalidate();

                loadVolumeId(level);
                volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
                if (level == VOLUME_LEVEL0) {
                    CommonService::setVolume(0);
                } else {
                    CommonService::volumeDown(level);
                }
                volumeBar.invalidate();
            }
            break;
        }
        case CommonService::AVRCP_OP_MUTE: {
            mute = true;
            volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID));
            level = VOLUME_LEVEL0;
            volume.invalidate();

            minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID));
            minus.invalidate();
            plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
            plus.invalidate();

            loadVolumeId(level);
            volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
            CommonService::setVolume(0);
            volumeBar.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_REMOTE_VOLUME: {
            level = (VolumeLevel) CommonService::getVolume();
            if (level == VOLUME_LEVEL7) {
                plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID));
            } else {
                plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
            }
            plus.invalidate();

            if (level == VOLUME_LEVEL0) {
                minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID));
            } else {
                minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID));
            }
            minus.invalidate();

            if (level == VOLUME_LEVEL0) {
                mute = true;
                volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID));
            } else {
                mute = false;
                volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_INT_ID));
            }
            volume.invalidate();

            loadVolumeId(level);
            volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
            CommonService::setVolume(level);
            volumeBar.invalidate();
            break;
        }
        case CommonService::AVRCP_OP_SONG_UPDATE: {
            refreshSong();
            break;
        }
        default:
            break;
    }
#endif
}

void HomeView::refreshSong()
{
    Unicode::UnicodeChar truncate1[SONG_MAX_LEN_PER_LINE + 1] = { 0 };
    Unicode::UnicodeChar truncate2[SONG_MAX_LEN_PER_LINE + 1] = { 0 };
    Unicode::UnicodeChar lf[2] = { '\n' };
    Unicode::UnicodeChar tail[4] = { '.', '.', '.' };
    char *name = CommonService::getMP3Song();
    if (strlen(name) > SONG_MAX_LEN_PER_LINE)
    {
        Unicode::strncpy(truncate1, name, SONG_MAX_LEN_PER_LINE);
        Unicode::strncpy(truncate2, name + SONG_MAX_LEN_PER_LINE, SONG_MAX_LEN_PER_LINE);
        Unicode::snprintf(songValueBuffer, 40, "%s%s%s%s", truncate1, lf, truncate2, tail);
    } else {
        Unicode::strncpy(songValueBuffer, name, 40);
    }
    song.invalidate();
}

void HomeView::buttonPressedHandler(const AbstractButton &button)
{
    if (&button == &play) {
        isPlaying = !isPlaying;
        if (isPlaying) {
            loadPauseId();
            play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
            CommonService::playMP3();
        } else {
            play.setBitmaps(Bitmap(playId), Bitmap(playId));
            CommonService::pauseMP3();
        }
        play.invalidate();
    } else if (&button == &plus) {
        if (level < VOLUME_LEVEL7) {
            if (level == VOLUME_LEVEL0) {
                mute = false;
                volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_INT_ID));
                volume.invalidate();
            }

            level = plusVolume(level);
            levelToRestoreAfterMute = level;
            if (level == VOLUME_LEVEL7) {
                plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID));
                plus.invalidate();
            }
            minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID));
            minus.invalidate();

            loadVolumeId(level);
            volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
            CommonService::volumeUp(level);
            volumeBar.invalidate();
        }
    } else if (&button == &minus) {
        if (level > VOLUME_LEVEL0) {
            level = minusVolume(level);
            if (level == VOLUME_LEVEL0) {
                minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID));
                minus.invalidate();
                mute = true;
                volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID));
                volume.invalidate();
            } else {
                levelToRestoreAfterMute = level;
            }
            plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
            plus.invalidate();

            loadVolumeId(level);
            volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
            if (level == VOLUME_LEVEL0) {
                CommonService::setVolume(0);
            } else {
                CommonService::volumeDown(level);
            }
            volumeBar.invalidate();
        }
    } else if (&button == &volume) {
        mute = !mute;
        if (mute) {
            volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_OFF_INT_ID));
            level = VOLUME_LEVEL0;
        } else {
            volume.setBitmaps(Bitmap(BITMAP_PLAYER_VOLUME_INT_ID), Bitmap(BITMAP_PLAYER_VOLUME_INT_ID));
            level = levelToRestoreAfterMute;
        }
        volume.invalidate();

        if (level == VOLUME_LEVEL0) {
            minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_DISABLE_INT_ID));
            minus.invalidate();
            plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
            plus.invalidate();
        } else if (level == VOLUME_LEVEL7) {
            minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID));
            minus.invalidate();
            plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_DISABLE_INT_ID));
            plus.invalidate();
        } else {
            minus.setBitmaps(Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_REMOVE_CIRCLE_INT_ID));
            minus.invalidate();
            plus.setBitmaps(Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID), Bitmap(BITMAP_PLAYER_ADD_CIRCLE_INT_ID));
            plus.invalidate();
        }

        loadVolumeId(level);
        volumeBar.setBitmaps(Bitmap(volumeId[level]), Bitmap(volumeId[level]));
        if (level == VOLUME_LEVEL0) {
            CommonService::setVolume(0);
        } else {
            CommonService::setVolume(level);
        }
        volumeBar.invalidate();
    } else if (&button == &next) {
        isPlaying = true;
        loadPauseId();
        play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
        CommonService::nextMP3();
        play.invalidate();
        refreshSong();
    } else if (&button == &previous) {
        isPlaying = true;
        loadPauseId();
        play.setBitmaps(Bitmap(pauseId), Bitmap(pauseId));
        CommonService::previousMP3();
        play.invalidate();
        refreshSong();
    }
}

void HomeView::loadPauseId()
{
#ifdef DYNAMIC_BITMAP_LOADER
    if (pauseId == BITMAP_INVALID) {
        const DynamicBitmapDatabase::BitmapInfo pauseInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_PLAYER_PAUSE_INT_ID);
        //create dynamic bitmap matching file dimensions
        pauseId = Bitmap::dynamicBitmapCreate(pauseInfo.width, pauseInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", pauseId);
        if (pauseId != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(pauseId);
            uint32_t destLen = pauseInfo.width * pauseInfo.height * 2;
            uint32_t srcLen = pauseInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, pauseInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        }
    }
#endif
}

void HomeView::loadVolumeId(VolumeLevel volume)
{
#ifdef DYNAMIC_BITMAP_LOADER
    if (volumeId[level] == BITMAP_INVALID) {
        uint16_t volumeID[MAX_VOLUME_LEVEL] = { DYNAMIC_BITMAP_PLAYER_VOLUME_BACKGROUND_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_1_INT_ID,
                                                DYNAMIC_BITMAP_PLAYER_VOLUME_2_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_3_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_4_INT_ID,
                                                DYNAMIC_BITMAP_PLAYER_VOLUME_5_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_6_INT_ID, DYNAMIC_BITMAP_PLAYER_VOLUME_7_INT_ID
                                              };
        const DynamicBitmapDatabase::BitmapInfo volumeInfo = DynamicBitmapDatabase::getInstanceInfo(volumeID[level]);
        //create dynamic bitmap matching file dimensions
        volumeId[level] = Bitmap::dynamicBitmapCreate(volumeInfo.width, volumeInfo.height, Bitmap::RGB565);
        LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", volumeId[level]);
        if (volumeId[level] != BITMAP_INVALID) {
            //read the bitmap file into the dynamic bitmap
            uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(volumeId[level]);
            uint32_t destLen = volumeInfo.width * volumeInfo.height * 2;
            uint32_t srcLen = volumeInfo.data_length;
            int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, volumeInfo.data, &srcLen);
            LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        }
    }
#endif
}

HomeView::VolumeLevel HomeView::plusVolume(VolumeLevel volume)
{
    switch (volume) {
        case VOLUME_LEVEL0:
            return VOLUME_LEVEL1;
        case VOLUME_LEVEL1:
            return VOLUME_LEVEL2;
        case VOLUME_LEVEL2:
            return VOLUME_LEVEL3;
        case VOLUME_LEVEL3:
            return VOLUME_LEVEL4;
        case VOLUME_LEVEL4:
            return VOLUME_LEVEL5;
        case VOLUME_LEVEL5:
            return VOLUME_LEVEL6;
        case VOLUME_LEVEL6:
            return VOLUME_LEVEL7;
        default:
            return VOLUME_LEVEL0;
    }
}

HomeView::VolumeLevel HomeView::minusVolume(VolumeLevel volume)
{
    switch (volume) {
        case VOLUME_LEVEL1:
            return VOLUME_LEVEL0;
        case VOLUME_LEVEL2:
            return VOLUME_LEVEL1;
        case VOLUME_LEVEL3:
            return VOLUME_LEVEL2;
        case VOLUME_LEVEL4:
            return VOLUME_LEVEL3;
        case VOLUME_LEVEL5:
            return VOLUME_LEVEL4;
        case VOLUME_LEVEL6:
            return VOLUME_LEVEL5;
        case VOLUME_LEVEL7:
            return VOLUME_LEVEL6;
        default:
            return VOLUME_LEVEL7;
    }
}
