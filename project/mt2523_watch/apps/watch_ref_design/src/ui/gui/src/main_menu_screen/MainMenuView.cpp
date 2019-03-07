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

#include <gui/main_menu_screen/MainMenuView.hpp>
#include <BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <gui/common/CommonUI.hpp>
#include <gui/common/CommonService.hpp>
#include <gui/database/DynamicBitmapDatabase.hpp>

#ifndef SIMULATOR
#include "ble_find_me_client.h"
#include "syslog.h"
#endif

#include "data_transfer.h"

int MainMenuView::GraphicHeight = 0;
bool MainMenuView::isMainMenuScrn = false;

MainMenuView::MainMenuView() :
    clickAbort(false),
    graphX(0),
    pointCounter(0),
    showFirstTimeFlag(0),
    graphInterval(0),
    averageHeartRate(0),
    totalHeartRate(0),
    maxHeartRate(0),
    minHeartRate(0),
    effectiveHeatrRateNum(0),
    onButtonPressed(this, &MainMenuView::buttonPressedHandler)
{
}

void MainMenuView::buttonPressedHandler(const AbstractButton &button)
{
    int fmpIndex = MainMenuPresenter::FIND_ME;

    if (&button == &start) {
#ifndef SIMULATOR
        ble_fmp_status_t status;
#endif
        isStart = !isStart;
        if (isStart) {
            start.setBitmaps(Bitmap(stopId), Bitmap(stopId));
            fmpStartTxt.setTypedText(TypedText(T_FINDME_STOP_BUTTON));
            start.invalidate();
            fmpStartTxt.invalidate();
#ifndef SIMULATOR
            status = ble_fmp_client_start_alert();
            switch (status) {
                case BLE_FMP_STATUS_SUCCESS:
                    break;
                case BLE_FMP_STATUS_FAIL: {
                    memset(fmpTipsbuf, 0x00, 40);
                    Unicode::snprintf(fmpTipsbuf, 40, "Please try again");
                    fmpTipsTxt.setTypedText(TypedText(T_FINDME_START_TIPS));
                    fmpTipsTxt.setWildcard(fmpTipsbuf);
                    fmpTipsTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
                    fmpTipsTxt.resizeToCurrentText();
                    fmpTipsTxt.setXY(100, 200);
                    demoScreensContainer[fmpIndex].add(fmpTipsTxt);
                    fmpTipsTxt.invalidate();
                }
                break;
                case BLE_FMP_STATUS_SERVICE_DISCOVERY_FAIL: {
                    memset(fmpTipsbuf, 0x00, 40);
                    Unicode::snprintf(fmpTipsbuf, 40, "Not find FMP service");
                    fmpTipsTxt.setTypedText(TypedText(T_FINDME_START_TIPS));
                    fmpTipsTxt.setWildcard(fmpTipsbuf);
                    fmpTipsTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
                    fmpTipsTxt.resizeToCurrentText();
                    fmpTipsTxt.setXY(60, 200);
                    demoScreensContainer[fmpIndex].add(fmpTipsTxt);
                    fmpTipsTxt.invalidate();
                }
                break;
                case BLE_FMP_STATUS_DISCONNECTED: {
                    memset(fmpTipsbuf, 0x00, 40);
                    Unicode::snprintf(fmpTipsbuf, 40, "Please connect BLE firstly");
                    fmpTipsTxt.setTypedText(TypedText(T_FINDME_START_TIPS));
                    fmpTipsTxt.setWildcard(fmpTipsbuf);
                    fmpTipsTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
                    fmpTipsTxt.resizeToCurrentText();
                    fmpTipsTxt.setXY(30, 200);
                    demoScreensContainer[fmpIndex].add(fmpTipsTxt);
                    fmpTipsTxt.invalidate();
                }
                break;
                default :
                    break;

            }
#endif
        } else {
#ifndef SIMULATOR
            demoScreensContainer[fmpIndex].remove(fmpTipsTxt);
            fmpTipsTxt.invalidate();
#endif
            start.setBitmaps(Bitmap(startId), Bitmap(startId));
            fmpStartTxt.setTypedText(TypedText(T_FINDME_START_BUTTON));
            start.invalidate();
            fmpStartTxt.invalidate();
#ifndef SIMULATOR
            status = ble_fmp_client_stop_alert();
#endif
        }
    }
}

void MainMenuView::setupScreen()
{
    isMainMenuScrn = true;
    //backgroundLeft.setBitmap(Bitmap(BITMAP_MENU_STAGE_STRETCH_LEFT_SIDE_INT_ID));
    //backgroundLeft.setXY(0, 0);
    //add(backgroundLeft);

    //backgroundRight.setBitmap(Bitmap(BITMAP_MENU_STAGE_STRETCH_RIGHT_SIDE_INT_ID));
    //backgroundRight.setXY(HAL::DISPLAY_WIDTH - backgroundRight.getWidth(), 0);
    //add(backgroundRight);

    menuContainer.setXY(0, 0);
    menuContainer.setEndSwipeElasticWidth(0);
    //menuContainer.setDotIndicatorBitmaps(Bitmap(BITMAP_SCREEN_SWIPE_DOTS_INACTIVE_INT_ID), Bitmap(BITMAP_SCREEN_SWIPE_DOTS_ACTIVE_INT_ID));
    //menuContainer.setDotIndicatorXYWithCenteredX(HAL::DISPLAY_WIDTH / 2 - NUMBER_OF_DEMO_SCREENS * 10 + 5, HAL::DISPLAY_HEIGHT - 30);
    menuContainer.setTouchable(false);

#ifndef SIMULATOR
    LOG_I(tgfx, "start uncompress %d\r\n", CommonService::getLogTimeInMS());
#endif

    for (int i = MainMenuPresenter::HEART_RATE; i < NUMBER_OF_DEMO_SCREENS; i++) {
        demoScreensContainer[i].setPosition(0, 0, HAL::DISPLAY_WIDTH, HAL::DISPLAY_HEIGHT);

        if (i == MainMenuPresenter::HEART_RATE) {
            addHeartRateScreen();
        } else if (i == MainMenuPresenter::BLOOD_PRESURE) {
            addBloodPresureScreen();
        } else if (i == MainMenuPresenter::FIND_ME) {
            addFindMeScreen();
        } else if (i == MainMenuPresenter::GPS) {
            addGPSScreen();
        } else if (i == MainMenuPresenter::SPORT) {
            addSportScreen();
        } else if (i == MainMenuPresenter::SETTINGS) {
            addSettingScreen();
        }

        menuContainer.add(demoScreensContainer[i]);
    }

#ifndef SIMULATOR
    LOG_I(tgfx, "end uncompress %d\r\n", CommonService::getLogTimeInMS());
#endif

    add(menuContainer);

    demoScreenIndex[0] = MainMenuPresenter::HEART_RATE;
    demoScreenIndex[1] = MainMenuPresenter::BLOOD_PRESURE;
    demoScreenIndex[2] = MainMenuPresenter::FIND_ME;
    demoScreenIndex[3] = MainMenuPresenter::GPS;
    demoScreenIndex[4] = MainMenuPresenter::SPORT;
    demoScreenIndex[5] = MainMenuPresenter::SETTINGS;
}

void MainMenuView::tearDownScreen()
{
#ifdef DYNAMIC_BITMAP_LOADER
    //if we have loaded a bitmap already, delete it
    if (commonId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(commonId);
    }
    if (fmpId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(fmpId);
    }
    if (startId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(startId);
    }
    if (stopId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(stopId);
    }
    if (gpsId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(gpsId);
    }
    if (settingId != BITMAP_INVALID) {
        Bitmap::dynamicBitmapDelete(settingId);
    }
#endif
    isMainMenuScrn = false;
}

void MainMenuView::afterTransition()
{
    menuContainer.setTouchable(true);
}

void MainMenuView::handleClickEvent(const ClickEvent &evt)
{
    DemoView::handleClickEvent(evt);

    if (evt.getType() == ClickEvent::RELEASED) {
        if (clickAbort == false) {
            gotoSelectedDemoScreen(menuContainer.getSelectedScreen());
        } else {
            clickAbort = false;
        }
    }
}

void MainMenuView::handleDragEvent(const DragEvent &evt)
{
    DemoView::handleDragEvent(evt);

    if (abs(evt.getDeltaX()) > 0 || abs(evt.getDeltaY()) > 0) {
        clickAbort = true;
    }
}

void MainMenuView::handleTickEvent()
{
    if (CommonService::hasGnssSignal() == true) {
        refreshGPSScreen();
        CommonService::resetGnssSignal();
    }
}

void MainMenuView::handleKeyEvent(uint8_t key)
{
#if 0
    if (key == 1) {
        refreshGPSScreen();
        return;
    }
#endif

    if (CommonService::isBacklight() == true) {
        if (CommonService::BLOODPRESURE_CACULATING != CommonService::GetBloodPresureStatus())
        {
            DemoView::handleKeyEvent(key);
        }
        return;
    }

    DemoView::handleKeyEvent(key);

    presenter->setSelectedMenuIndex(0);
    presenter->backOptionHomeSelected();
}

void MainMenuView::gotoSelectedDemoScreen(uint8_t demoIndex)
{
    presenter->demoSelected(demoScreenIndex[demoIndex], demoIndex);
}

void MainMenuView::setSelectedMenuIndex(uint8_t demoIndex)
{
    menuContainer.setSelectedScreen(demoIndex);
}

void MainMenuView::checkIfBPscreen()
{
    // if reenter BP screen all thing should be init again
    if (MainMenuPresenter::BLOOD_PRESURE == menuContainer.getSelectedScreen()) {
        int currentStatus = CommonService::GetBloodPresureStatus();
        if (CommonService::BLOODPRESURE_NOT_START != currentStatus) {
            CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
            CommonService::setBPTipsCount(10);
            if (CommonService::BLOODPRESURE_CACULATING == currentStatus) {
                // need stop bp sensor
            }
        }
    }
}

void MainMenuView::addHeartRateScreen()
{
    int heartRateIndex = MainMenuPresenter::HEART_RATE;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[heartRateIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
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
        backgroundImage[heartRateIndex].setBitmap(Bitmap(commonId));
    }
#endif
    backgroundImage[heartRateIndex].setXY(0, 0);
    demoScreensContainer[heartRateIndex].add(backgroundImage[heartRateIndex]);

#ifdef SIMULATOR
    gotoHomeButton.setBitmaps(Bitmap(BITMAP_ICON_BACK_INT_ID), Bitmap(BITMAP_ICON_BACK_INT_ID));
    gotoHomeButton.setXY(CommonUI::BACK_BUTTON_X, CommonUI::BACK_BUTTON_Y);
    demoScreensContainer[heartRateIndex].add(gotoHomeButton);
#endif

    // Heart rate icon
    HeartRateIcon.setBitmap(Bitmap(BITMAP_ICON_LARGE_HEART_INT_ID));
    //HeartRateIcon.setAlpha(0x3B);
    demoScreensContainer[heartRateIndex].add(HeartRateIcon);

    // Average Heart rate
    Unicode::snprintf(averageHRDatabuf, 5, "%d", averageHeartRate);
    averageHRDataTxt.setTypedText(TypedText(T_HEARTRATE_AVERAGE_VALUE));
    averageHRDataTxt.setWildcard(averageHRDatabuf);
    averageHRDataTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    averageHRDataTxt.resizeToCurrentText();
    demoScreensContainer[heartRateIndex].add(averageHRDataTxt);

    // Heart rate title
    strHRTitle.setTypedText(TypedText(T_HEARTRATE_TITLE));
    strHRTitle.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    demoScreensContainer[heartRateIndex].add(strHRTitle);

    // Max Heart rate
    strMaxHeart.setTypedText(TypedText(T_HEARTRATE_MAX_STRING));
    strMaxHeart.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    strMaxHeart.setXY(110, 347);
    strMaxHeart.setAlpha(127);
    demoScreensContainer[heartRateIndex].add(strMaxHeart);

    // Max Heart rate value
    Unicode::snprintf(maxHRDatabuf, 5, "%d", maxHeartRate);
    maxHRDataTxt.setTypedText(TypedText(T_HEARTRATE_MAX_STRING_VALUE));
    maxHRDataTxt.setWildcard(maxHRDatabuf);
    maxHRDataTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    maxHRDataTxt.setPosition(155, 347, 60, 33);
    demoScreensContainer[heartRateIndex].add(maxHRDataTxt);

    // Min Heart rate
    strMinHeart.setTypedText(TypedText(T_HEARTRATE_MIN_STRING));
    strMinHeart.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    strMinHeart.setXY(207, 347);
    strMinHeart.setAlpha(127);
    demoScreensContainer[heartRateIndex].add(strMinHeart);

    // Min Heart rate value
    Unicode::snprintf(minHRDatabuf, 5, "%d", minHeartRate);
    minHRDataTxt.setTypedText(TypedText(T_HEARTRATE_MIN_STRING_VALUE));
    minHRDataTxt.setWildcard(minHRDatabuf);
    minHRDataTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    minHRDataTxt.setPosition(253, 347, 60, 33);
    demoScreensContainer[heartRateIndex].add(minHRDataTxt);

    // Graph
    int graphXOffset = 20;
    int graphYOffset = 10;
    graphArea.setPosition(0, averageHRDataTxt.getRect().bottom(), HAL::DISPLAY_WIDTH - graphXOffset, HAL::DISPLAY_HEIGHT - averageHRDataTxt.getRect().bottom() - 40);

    int graphWidth = graphArea.getRect().width;
    int graphHeight = graphArea.getRect().height;

    GraphicHeight = graphHeight;

    int eachRowData = graphHeight / (NUMBER_OF_GRID_LINES) - 1;
    int logicalREachRowData = (MainMenuPresenter::HeartRateUpper - MainMenuPresenter::HeartRateLower) / (NUMBER_OF_GRID_LINES - 1);

    int graphCircleXoffset[] = {60, 40, 35, 30, 40, 60, 80, 108};
    int graphCircleWidth[] = { graphWidth - 70, graphWidth - 35, graphWidth + 40, graphWidth + 50, graphWidth - 25, graphWidth - 70, graphWidth - 120, graphWidth - 180 };
    for (int i = 0; i < NUMBER_OF_GRID_LINES; i++) {
        // Graph - Grid line
        graphGridLines[i].setColor(Color::getColorFrom24BitRGB(0xE1, 0xE4, 0xE6));
        graphGridLines[i].setPosition(graphCircleXoffset[i], eachRowData * (i + 1), graphCircleWidth[i], 1);
        graphArea.add(graphGridLines[i]);

        // Graph - Column value
        Unicode::snprintf(graphYValuesbuf[i], 5, "%d", MainMenuPresenter::HeartRateUpper -  i * logicalREachRowData);
        graphYValues[i].setTypedText(TypedText(T_HEARTRATE_GRAPH_Y_VALUE));
        graphYValues[i].setWildcard(graphYValuesbuf[i]);
        graphYValues[i].setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
        graphYValues[i].resizeToCurrentText();
        graphYValues[i].setXY(graphGridLines[i].getX() - 25, graphGridLines[i].getY() - 8);
        graphArea.add(graphYValues[i]);
    }

    // Graph - Graph
    primaryGraph.setXY(graphXOffset, graphYOffset);
    primaryGraph.setup(graphWidth, graphHeight, Color::getColorFrom24BitRGB(0xFF, 0xFF, 0x10), graphBackground.getColor());
    primaryGraph.setDotShape(0, 20, 4);
    primaryGraph.setDotBackgroundShape(0, 20, 6);
    graphArea.add(primaryGraph);

    demoScreensContainer[heartRateIndex].add(graphArea);
    graphX = graphXOffset;
    graphInterval = primaryGraph.getWidth() / Data_Save_Num;
    initPrimaryGraph();

    refreshHeartRateChracterastic();
    refreshHeartRateIcon();
}

void MainMenuView::initPrimaryGraph()
{
    int index = 0;
    showFirstTimeFlag = 0;
    for (index = 0; index < Data_Save_Num; ++index) {
        data[index][0] = graphX;
        data[index][1] = VALIDATE_HR_DATA_Y_VALUE;
        data[index][2] = 0;
        graphX += graphInterval;
    }
    graphX = 20;
    showFirstTimeFlag = 1;
}

void MainMenuView::resetPrimaryGraph()
{
    initPrimaryGraph();
    primaryGraph.clear();
    primaryGraph.invalidate();
    pointCounter = 0;
}

void MainMenuView::refreshGraphs(int y, int physicalData)
{
    int index = 0;
    int firstPhysicalHeartRateData = 0;
    int firstLogicalHeartRateData = 0;
    // move all data from ahead by one element
    if (pointCounter == Data_Save_Num) {
        resetPrimaryGraph();
    }

    data[pointCounter][1] = y;
    data[pointCounter][2] = physicalData;

    realHeartRate = physicalData;
    primaryGraph.addValue(data[pointCounter][0], data[pointCounter][1]);

    index = pointCounter;
    ++pointCounter;

    // caculate charateristic
    firstPhysicalHeartRateData = data[index][2];
    firstLogicalHeartRateData = data[index][1];
    if (VALIDATE_HR_DATA_Y_VALUE != firstLogicalHeartRateData) {
        ++effectiveHeatrRateNum;
        totalHeartRate += firstPhysicalHeartRateData;
        maxHeartRate = maxHeartRate < firstPhysicalHeartRateData ? firstPhysicalHeartRateData : maxHeartRate;
        if (0 != minHeartRate) {
            minHeartRate = minHeartRate > firstPhysicalHeartRateData ? firstPhysicalHeartRateData : minHeartRate;
        } else {
            minHeartRate = firstPhysicalHeartRateData;
        }
    }
}

void MainMenuView::refreshHeartRateChracterastic()
{
    // show title
    if (averageHeartRate >= 100) {
        strHRTitle.moveTo(103, 65);
    } else {
        strHRTitle.moveTo(120, 65);
    }
    strHRTitle.resizeToCurrentText();
    strHRTitle.invalidate();

    // show max data
    Unicode::snprintf(maxHRDatabuf, 5, "%d", maxHeartRate);
    if (100 <= maxHeartRate) {
        maxHRDataTxt.moveTo(155, 347);
    } else if (10 <= maxHeartRate) {
        maxHRDataTxt.moveTo(163, 347);
    } else {
        maxHRDataTxt.moveTo(171, 347);
    }
    maxHRDataTxt.invalidate();

    // show min data
    Unicode::snprintf(minHRDatabuf, 5, "%d", minHeartRate);
    if (100 <= minHeartRate) {
        minHRDataTxt.moveTo(253, 347);
    }  else if (10 <= minHeartRate) {
        minHRDataTxt.moveTo(261, 347);
    }  else {
        minHRDataTxt.moveTo(269, 347);
    }
    minHRDataTxt.invalidate();

    // show real data
    Unicode::snprintf(realHRDatabuf, 5, "%d", realHeartRate);
    if (100 <= realHeartRate) {
        realHRDataTxt.moveTo(238, 115);
    } else if (10 <= realHeartRate) {
        realHRDataTxt.moveTo(246, 115);
    } else {
        realHRDataTxt.moveTo(254, 115);
    }
    realHRDataTxt.invalidate();

    // show average data
    if (0 != effectiveHeatrRateNum) {
        averageHeartRate = totalHeartRate / effectiveHeatrRateNum;
    }
    Unicode::snprintf(averageHRDatabuf, 5, "%d", averageHeartRate);
    if (averageHeartRate >= 100) {
        averageHRDataTxt.moveTo(148, 25);
    } else {
        averageHRDataTxt.moveTo(165, 25);
    }
    averageHRDataTxt.resizeToCurrentText();
    averageHRDataTxt.invalidate();

}

void MainMenuView::refreshHeartRateIcon()
{
    static int hrateFlag = 0;
    //int hrateAlphaVal = HEART_RATE_ICON_SHALLOW_COLOR;
    hrateFlag = (hrateFlag == 0) ? \
                (/* hrateAlphaVal = HEART_RATE_ICON_SHALLOW_COLOR, */HeartRateIcon.setBitmap(Bitmap(BITMAP_ICON_SMALL_HEART_INT_ID)), 1) : \
                (/* hrateAlphaVal = HEART_RATE_ICON_DEEP_COLOR, */HeartRateIcon.setBitmap(Bitmap(BITMAP_ICON_LARGE_HEART_INT_ID)), 0);
    //HeartRateIcon.setAlpha(hrateAlphaVal);
    if (averageHeartRate >= 100) {
        HeartRateIcon.moveTo(257, 65);
    } else {
        HeartRateIcon.moveTo(240, 65);
    }
    HeartRateIcon.invalidate();
}

void MainMenuView::handleGsensorDataUpdated(int stepData)
{
#ifndef SIMULATOR
    LOG_I(tgfx, "handleGsensorDataUpdated() = %d\r\n", stepData);
#endif
    Unicode::snprintf(stepDatabuf, 5, "%d", stepData);
    stepTxt.resizeToCurrentText();
    stepTxt.invalidate();
}

void MainMenuView::handleHeartRateDataUpdated(int physicalData, int logicalRateData)
{
    if (1 == showFirstTimeFlag) {
        refreshGraphs(logicalRateData, physicalData);
        refreshHeartRateChracterastic();

        if ((MainMenuPresenter::HEART_RATE == menuContainer.getSelectedScreen()) && (menuContainer.getCurrentState() == SwipeContainer::NO_ANIMATION)) {
            refreshHeartRateIcon();
            dt_set_start_transfer_flag(true);
            LOG_I(bmt_demo, "[dt_update]dt_set_start_transfer_flag true1");
        }else if ((MainMenuPresenter::BLOOD_PRESURE == menuContainer.getSelectedScreen()) && (menuContainer.getCurrentState() == SwipeContainer::NO_ANIMATION)) {
            dt_set_start_transfer_flag(true);
            LOG_I(bmt_demo, "[dt_update]dt_set_start_transfer_flag true2");
        }else {
            dt_set_start_transfer_flag(false);
            LOG_I(bmt_demo, "[dt_update]dt_set_start_transfer_flag false");
        }
    }
}

void MainMenuView::handleBPResultUpdated(int hbpData, int lbpData)
{
    Unicode::snprintf(bloodPresureTipsbuf, 80, "BloodPresure:\n %d / %d", hbpData, lbpData);
    //bloodPresureTipsTxt.resizeToCurrentText();
    bloodPresureTipsTxt.invalidate();
}

void MainMenuView::handleBPTipsUpdated(int count)
{
    // check if screen is not in bp, should be stopped.
    if (MainMenuPresenter::BLOOD_PRESURE != menuContainer.getSelectedScreen()) {
        CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
        CommonService::setBPTipsCount(10);
    }
    if (10 == CommonService::getBPTipsCount()) {
        Unicode::snprintf(bloodPresureTipsbuf, 80, "Please touch screen,\nthen put your\n finger on device...   %d", CommonService::getBPTipsCount());
    } else {
        Unicode::snprintf(bloodPresureTipsbuf, 80, "Please put your finger\n on device...   %d", CommonService::getBPTipsCount());
    }
    //bloodPresureTipsTxt.resizeToCurrentText();
    bloodPresureTipsTxt.invalidate();
}

void MainMenuView::handleBPTipsUpdate()
{
    Unicode::snprintf(bloodPresureTipsbuf, 80, "Blood Pressure\n measuring.\n Please wait...");
    //bloodPresureTipsTxt.resizeToCurrentText();
    bloodPresureTipsTxt.invalidate();
}

void MainMenuView::addFindMeScreen()
{
    int fmpIndex = MainMenuPresenter::FIND_ME;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[fmpIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[fmpIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[fmpIndex].setXY(0, 0);
    demoScreensContainer[fmpIndex].add(backgroundImage[fmpIndex]);

#ifndef DYNAMIC_BITMAP_LOADER
    fmpBG.setBitmap(Bitmap(BITMAP_MENU_FMP_INT_ID));
#else
    const DynamicBitmapDatabase::BitmapInfo fmpInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_MENU_FMP_INT_ID);
    //create dynamic bitmap matching file dimensions
    fmpId = Bitmap::dynamicBitmapCreate(fmpInfo.width, fmpInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", fmpId);
    if (fmpId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(fmpId);
        uint32_t destLen = fmpInfo.width * fmpInfo.height * 2;
        uint32_t srcLen = fmpInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, fmpInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        fmpBG.setBitmap(Bitmap(fmpId));
    }
#endif
    fmpBG.setXY(CommonUI::MENU_FMP_X, CommonUI::MENU_FMP_Y);
    demoScreensContainer[fmpIndex].add(fmpBG);

#ifndef DYNAMIC_BITMAP_LOADER
    stopId = BITMAP_FMP_STOP_LONG_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo stopInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_FMP_STOP_LONG_INT_ID);
    //create dynamic bitmap matching file dimensions
    stopId = Bitmap::dynamicBitmapCreate(stopInfo.width, stopInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", stopId);
    if (stopId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(stopId);
        uint32_t destLen = stopInfo.width * stopInfo.height * 2;
        uint32_t srcLen = stopInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, stopInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif

#ifndef DYNAMIC_BITMAP_LOADER
    startId = BITMAP_FMP_START_LONG_INT_ID;
#else
    const DynamicBitmapDatabase::BitmapInfo startInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_FMP_START_LONG_INT_ID);
    //create dynamic bitmap matching file dimensions
    startId = Bitmap::dynamicBitmapCreate(startInfo.width, startInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", startId);
    if (startId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(startId);
        uint32_t destLen = startInfo.width * startInfo.height * 2;
        uint32_t srcLen = startInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, startInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
    }
#endif

    //startId = BITMAP_BTN_START_LONG_INT_ID;
    //stopId = BITMAP_BTN_STOP_LONG_INT_ID;

    start.setBitmaps(Bitmap(startId), Bitmap(startId));
    start.setXY(CommonUI::FINDME_START_BUTTON_X, CommonUI::FINDME_START_BUTTON_Y);
    start.setAction(onButtonPressed);
    demoScreensContainer[fmpIndex].add(start);

    fmpStartTxt.setTypedText(TypedText(T_FINDME_START_BUTTON));
    fmpStartTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    fmpStartTxt.setXY(CommonUI::FINDME_START_BUTTON_X + 65, CommonUI::FINDME_START_BUTTON_Y + 10);
    demoScreensContainer[fmpIndex].add(fmpStartTxt);
}

void MainMenuView::addGPSScreen()
{
    int gpsIndex = MainMenuPresenter::GPS;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[gpsIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[gpsIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[gpsIndex].setXY(0, 0);
    demoScreensContainer[gpsIndex].add(backgroundImage[gpsIndex]);

#ifndef DYNAMIC_BITMAP_LOADER
    gpsBG.setBitmap(Bitmap(BITMAP_MENU_GPS_INT_ID));
#else
    const DynamicBitmapDatabase::BitmapInfo gpsInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_MENU_GPS_INT_ID);
    //create dynamic bitmap matching file dimensions
    gpsId = Bitmap::dynamicBitmapCreate(gpsInfo.width, gpsInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", gpsId);
    if (gpsId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(gpsId);
        uint32_t destLen = gpsInfo.width * gpsInfo.height * 2;
        uint32_t srcLen = gpsInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, gpsInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        gpsBG.setBitmap(Bitmap(gpsId));
    }
#endif
    gpsBG.setXY(CommonUI::MENU_GPS_X, CommonUI::MENU_GPS_Y);
    demoScreensContainer[gpsIndex].add(gpsBG);

    Geography geo = CommonService::getCurrentGeography();

    bool gpsStatus = CommonService::getGPSStatus();

    if (gpsStatus == false) {
        Unicode::snprintf(longitudeValueBuffer, 20, "--\xB0--'--\" N");
    } else if (geo.longitude.degree > 0 || geo.longitude.minute > 0 || geo.longitude.second > 0) {
        Unicode::snprintf(longitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" N", geo.longitude.degree, geo.longitude.minute, geo.longitude.second);
    } else {
        Unicode::snprintf(longitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" S", geo.longitude.degree, geo.longitude.minute, geo.longitude.second);
    }
    longitude.setWildcard(longitudeValueBuffer);
    longitude.setTypedText(TypedText(T_GPS_LONGITUDE));
    longitude.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    longitude.setPosition(CommonUI::LONGITUDE_X, CommonUI::LONGITUDE_Y, CommonUI::LONGITUDE_WIDTH, CommonUI::LONGITUDE_HEIGHT);
    demoScreensContainer[gpsIndex].add(longitude);

    if (gpsStatus == false) {
        Unicode::snprintf(latitudeValueBuffer, 20, "--\xB0--'--\" E");
    } else if (geo.latitude.degree > 0 || geo.latitude.minute > 0 || geo.latitude.second > 0) {
        Unicode::snprintf(latitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" E", geo.latitude.degree, geo.latitude.minute, geo.latitude.second);
    } else {
        Unicode::snprintf(latitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" W", geo.latitude.degree, geo.latitude.minute, geo.latitude.second);
    }
    latitude.setWildcard(latitudeValueBuffer);
    latitude.setTypedText(TypedText(T_GPS_LATITUDE));
    latitude.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    latitude.setPosition(CommonUI::LATITUDE_X, CommonUI::LATITUDE_Y, CommonUI::LATITUDE_WIDTH, CommonUI::LATITUDE_HEIGHT);
    demoScreensContainer[gpsIndex].add(latitude);

    if (gpsStatus == false) {
        Unicode::snprintf(altitudeValueBuffer, 20, "--");
    } else {
        Unicode::snprintf(altitudeValueBuffer, 20, "%02d", geo.altitude);
    }
    altitude.setWildcard(altitudeValueBuffer);
    altitude.setTypedText(TypedText(T_GPS_ALTITUDE));
    altitude.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    altitude.setPosition(0, CommonUI::ALTITUDE_Y, HAL::DISPLAY_WIDTH, CommonUI::ALTITUDE_HEIGHT);
    demoScreensContainer[gpsIndex].add(altitude);
}

void MainMenuView::refreshGPSScreen()
{
    Geography geo = CommonService::getCurrentGeography();

    if (geo.longitude.degree > 0 || geo.longitude.minute > 0 || geo.longitude.second > 0) {
        Unicode::snprintf(longitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" N", geo.longitude.degree, geo.longitude.minute, geo.longitude.second);
    } else {
        Unicode::snprintf(longitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" S", geo.longitude.degree, geo.longitude.minute, geo.longitude.second);
    }
    longitude.invalidate();

    if (geo.latitude.degree > 0 || geo.latitude.minute > 0 || geo.latitude.second > 0) {
        Unicode::snprintf(latitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" E", geo.latitude.degree, geo.latitude.minute, geo.latitude.second);
    } else {
        Unicode::snprintf(latitudeValueBuffer, 20, "%02d\xB0%02d'%02d\" W", geo.latitude.degree, geo.latitude.minute, geo.latitude.second);
    }
    latitude.invalidate();

    Unicode::snprintf(altitudeValueBuffer, 20, "%02d", geo.altitude);
    altitude.invalidate();
}

void MainMenuView::addSportScreen()
{
    int sportIndex = MainMenuPresenter::SPORT;
    bool heartrateStatus = CommonService::getHeartRateStatus();
    bool gpsStatus = CommonService::getGPSStatus();

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[sportIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[sportIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[sportIndex].setXY(0, 0);
    demoScreensContainer[sportIndex].add(backgroundImage[sportIndex]);

    // Heart rate string
    strHeartRate.setTypedText(TypedText(T_HEARTRATE_TEXT));
    strHeartRate.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    strHeartRate.setXY(69, 116);
    demoScreensContainer[sportIndex].add(strHeartRate);

    // Heart rate value
    if (heartrateStatus == false) {
        Unicode::snprintf(realHRDatabuf, 5, "----");
    } else {
        Unicode::snprintf(realHRDatabuf, 5, "%d", realHeartRate);
    }
    realHRDataTxt.setTypedText(TypedText(T_HEARTRATE_REAL_STRING_VALUE));
    realHRDataTxt.setWildcard(realHRDatabuf);
    realHRDataTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    realHRDataTxt.setPosition(238, 115, 60, 33);
    demoScreensContainer[sportIndex].add(realHRDataTxt);

    // GPS string
    strGps.setTypedText(TypedText(T_GPS_STATUS_TEXT));
    strGps.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    strGps.setXY(60, 178);
    demoScreensContainer[sportIndex].add(strGps);

    // GPS status string
    if (gpsStatus == false) {
        realGPSDataTxt.setTypedText(TypedText(T_OFF_TEXT));
    } else {
        realGPSDataTxt.setTypedText(TypedText(T_ON_TEXT));
    }
    realGPSDataTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    realGPSDataTxt.resizeToCurrentText();
    realGPSDataTxt.setXY(238, 178);
    demoScreensContainer[sportIndex].add(realGPSDataTxt);

    // G-sensor string
    strStep.setTypedText(TypedText(T_STEP_TEXT));
    strStep.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    strStep.setXY(145, 240);
    demoScreensContainer[sportIndex].add(strStep);

    // G-sensor value
    int gSensorData = HeartRateCache::getHeartRateCacheInstance()->getGSensorData();
    Unicode::snprintf(stepDatabuf, 5, "%d", gSensorData);
    stepTxt.setTypedText(TypedText(T_STEP_STRING_VALUE));
    stepTxt.setWildcard(stepDatabuf);
    stepTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    stepTxt.resizeToCurrentText();
    stepTxt.setXY(238, 240);
    demoScreensContainer[sportIndex].add(stepTxt);
}

void MainMenuView::addBloodPresureScreen()
{
    int bloodPresureIndex = MainMenuPresenter::BLOOD_PRESURE;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[bloodPresureIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[bloodPresureIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[bloodPresureIndex].setXY(0, 0);
    demoScreensContainer[bloodPresureIndex].add(backgroundImage[bloodPresureIndex]);

    // blood presure title
    strBloodpresureTitle.setTypedText(TypedText(T_BLOODPRESURE_TITLE));
    strBloodpresureTitle.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    strBloodpresureTitle.setXY(120, 55);
    demoScreensContainer[bloodPresureIndex].add(strBloodpresureTitle);

    // blood presure start tips
    Unicode::snprintf(bloodPresureTipsbuf, 80, "Please touch screen,\nthen put your\n finger on device...   %d", 10);
    bloodPresureTipsTxt.setTypedText(TypedText(T_BLOODPRESURE_START_TIPS));
    bloodPresureTipsTxt.setWildcard(bloodPresureTipsbuf);
    bloodPresureTipsTxt.setColor(Color::getColorFrom24BitRGB(0xFF, 0xFF, 0xFF));
    bloodPresureTipsTxt.resizeToCurrentText();
    bloodPresureTipsTxt.setXY(50, 150);
    demoScreensContainer[bloodPresureIndex].add(bloodPresureTipsTxt);

    CommonService::SetBloodPresureStatus(CommonService::BLOODPRESURE_NOT_START);
    CommonService::setBPTipsCount(10);
}

void MainMenuView::addSettingScreen()
{
    int settingIndex = MainMenuPresenter::SETTINGS;

    // Setup background
#ifndef DYNAMIC_BITMAP_LOADER
    backgroundImage[settingIndex].setBitmap(Bitmap(BITMAP_COMMON_BACKGROUND_INT_ID));
#else
    backgroundImage[settingIndex].setBitmap(Bitmap(commonId));
#endif
    backgroundImage[settingIndex].setXY(0, 0);
    demoScreensContainer[settingIndex].add(backgroundImage[settingIndex]);

#ifndef DYNAMIC_BITMAP_LOADER
    settingBG.setBitmap(Bitmap(BITMAP_MENU_SETTING_INT_ID));
#else
    const DynamicBitmapDatabase::BitmapInfo settingInfo = DynamicBitmapDatabase::getInstanceInfo(DYNAMIC_BITMAP_MENU_SETTING_INT_ID);
    //create dynamic bitmap matching file dimensions
    settingId = Bitmap::dynamicBitmapCreate(settingInfo.width, settingInfo.height, Bitmap::RGB565);
    LOG_I(tgfx, "dynamicBitmapCreate() = %d\r\n", settingId);
    if (settingId != BITMAP_INVALID) {
        //read the bitmap file into the dynamic bitmap
        uint8_t *const buffer8 = Bitmap::dynamicBitmapGetAddress(settingId);
        uint32_t destLen = settingInfo.width * settingInfo.height * 2;
        uint32_t srcLen = settingInfo.data_length;
        int result = DynamicBitmapDatabase::uncompress(buffer8, &destLen, settingInfo.data, &srcLen);
        LOG_I(tgfx, "uncompress() = %d, srcLen = %d, destLen = %d\r\n", result, srcLen, destLen);
        settingBG.setBitmap(Bitmap(settingId));
    }
#endif
    settingBG.setXY(CommonUI::MENU_SETTING_X, CommonUI::MENU_SETTING_Y);
    demoScreensContainer[settingIndex].add(settingBG);
}
