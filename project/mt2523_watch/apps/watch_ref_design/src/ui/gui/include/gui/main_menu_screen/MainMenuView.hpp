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

#ifndef MAIN_MENU_VIEW_HPP
#define MAIN_MENU_VIEW_HPP

#include <gui/common/DemoView.hpp>
#include <gui/main_menu_screen/MainMenuPresenter.hpp>
#include <gui/main_menu_screen/SwipeContainer.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/widgets/canvas/Circle.hpp>
#include <gui/graph_screen/Graph.hpp>

#if !defined(USE_BPP) || USE_BPP==16
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>
#elif USE_BPP==24
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>
#endif

class MainMenuView : public DemoView<MainMenuPresenter>
{
public:
    MainMenuView();
    virtual ~MainMenuView() {};

    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void afterTransition();
    virtual void handleClickEvent(const ClickEvent &evt);
    virtual void handleDragEvent(const DragEvent &evt);
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

    void gotoSelectedDemoScreen(uint8_t demoIndex);
    void setSelectedMenuIndex(uint8_t demoIndex);

    // Updates the HeatrRate data
    void handleHeartRateDataUpdated(int physicalData, int logicalShowData);

    void handleGsensorDataUpdated(int stepData);

    void handleBPTipsUpdated(int count);

    void handleBPTipsUpdate();

    void handleBPResultUpdated(int hbpData, int lbpData);

    int getGraphRowNum()
    {
        return NUMBER_OF_GRID_LINES;
    }

    static int GraphicHeight;

    static bool isMainMenuScrn;

protected:
    static const int NUMBER_OF_DEMO_SCREENS = MainMenuPresenter::MAX_DEMO_ID;

    MainMenuPresenter::DemoID demoScreenIndex[NUMBER_OF_DEMO_SCREENS];
    bool clickAbort;

    Container demoScreensContainer[NUMBER_OF_DEMO_SCREENS];
    SwipeContainer menuContainer;
    Image backgroundLeft;
    Image backgroundRight;
    Image backgroundImage[NUMBER_OF_DEMO_SCREENS];

    // HeartRate members
    static const int NUMBER_OF_GRID_LINES = 8;
    BitmapId commonId;
    Image HeartRateIcon;
    Container graphArea;
    Box graphBackground;
    Box graphGridLines[NUMBER_OF_GRID_LINES];
    TextAreaWithOneWildcard graphYValues[NUMBER_OF_GRID_LINES];
    Unicode::UnicodeChar graphYValuesbuf[NUMBER_OF_GRID_LINES][5];
    Graph primaryGraph;
    int tickCount;
    int graphX;
    int graphType;
    int leftX;
    int pointCounter;
    int showFirstTimeFlag;
    int graphInterval;
    static const int VALIDATE_HR_DATA_Y_VALUE = 25;
    static const int Data_Save_Num = 60;
    static const int HEART_RATE_ICON_SHALLOW_COLOR = 0xAB;
    static const int HEART_RATE_ICON_DEEP_COLOR = 0xFF;
    int data[Data_Save_Num][3];
    int averageHeartRate;
    int totalHeartRate;
    int maxHeartRate;
    int minHeartRate;
    int effectiveHeatrRateNum;

    TextAreaWithOneWildcard averageHRDataTxt;
    Unicode::UnicodeChar averageHRDatabuf[5];
    TextArea strHRTitle;
    TextArea strMaxHeart;
    TextArea strMinHeart;
    TextAreaWithOneWildcard maxHRDataTxt;
    Unicode::UnicodeChar maxHRDatabuf[5];
    TextAreaWithOneWildcard minHRDataTxt;
    Unicode::UnicodeChar minHRDatabuf[5];

    // Blood presure members
    TextArea strBloodpresureTitle;
    TextAreaWithOneWildcard bloodPresureTipsTxt;
    Unicode::UnicodeChar bloodPresureTipsbuf[80];

    // FMP members
    Image fmpBG;
    BitmapId fmpId;
    TextAreaWithOneWildcard fmpTipsTxt;
    Unicode::UnicodeChar fmpTipsbuf[40];
    Button start;
    BitmapId startId;
    BitmapId stopId;
    TextArea fmpStartTxt;
    bool isStart;

    // GPS members
    Image gpsBG;
    BitmapId gpsId;

    TextAreaWithOneWildcard longitude;
    Unicode::UnicodeChar longitudeValueBuffer[20];
    TextAreaWithOneWildcard latitude;
    Unicode::UnicodeChar latitudeValueBuffer[20];
    TextAreaWithOneWildcard altitude;
    Unicode::UnicodeChar altitudeValueBuffer[20];

    // Sport members
    TextArea strHeartRate;
    TextAreaWithOneWildcard realHRDataTxt;
    Unicode::UnicodeChar realHRDatabuf[5];
    int realHeartRate;

    TextArea strGps;
    TextArea realGPSDataTxt;
    int realGPS;

    TextArea strStep;
    TextAreaWithOneWildcard stepTxt;
    Unicode::UnicodeChar stepDatabuf[5];

    // Setting members
    Image settingBG;
    BitmapId settingId;

    Callback<MainMenuView, const AbstractButton &> onButtonPressed;
    void buttonPressedHandler(const AbstractButton &button);

    void addHeartRateScreen();
    void initPrimaryGraph();
    void resetPrimaryGraph();
    void refreshGraphs(int y, int physicalData);
    void refreshHeartRateChracterastic();
    void refreshHeartRateIcon();

    void addFindMeScreen();
    void addGPSScreen();
    void refreshGPSScreen();
    void addSportScreen();
    void addSettingScreen();
    void addBloodPresureScreen();
    void checkIfBPscreen();
};

#endif // MAIN_MENU_VIEW_HPP
