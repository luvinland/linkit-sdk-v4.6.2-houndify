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

#ifndef HOME_VIEW_HPP
#define HOME_VIEW_HPP

#include <gui/common/DemoView.hpp>
#include <gui/home_screen/HomePresenter.hpp>
#include <gui/main_menu_screen/SwipeContainer.hpp>
#include <gui/setting_screen/MenuListElement.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/widgets/canvas/Shape.hpp>
#include <touchgfx/widgets/canvas/Circle.hpp>
#include <touchgfx/containers/ScrollableContainer.hpp>
#include <touchgfx/containers/clock/AnalogClock.hpp>

#if !defined(USE_BPP) || USE_BPP==16
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>
#elif USE_BPP==24
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>
#endif

class ClockWidget : public AnalogClock
{
public:
    ClockWidget() {};
    virtual ~ClockWidget() {};

    void setupClock(BitmapId clockId);

    // Updates the hour, minute and second hands
    void handleTimeUpdated(uint8_t hours, uint8_t minutes, uint8_t seconds);
    // Updates capacity info
    void handleCapacityUpdated(int capacityData);

protected:
    Image BatteryImage;
    Image ChargerImage;

    TextAreaWithOneWildcard year;
    Unicode::UnicodeChar yearValueBuffer[5];
    TextAreaWithOneWildcard month;
    Unicode::UnicodeChar monthValueBuffer[4];
    TextAreaWithOneWildcard day;
    Unicode::UnicodeChar dayValueBuffer[3];
    TextAreaWithOneWildcard battery;
    Unicode::UnicodeChar batteryValueBuffer[5];
};

class CompoundClockWidget : public AnalogClock
{
public:
    CompoundClockWidget() {};
    virtual ~CompoundClockWidget() {};

    void setupClock(BitmapId clockId);

    // Updates the hour, minute and second hands
    void handleTimeUpdated(uint8_t hours, uint8_t minutes, uint8_t seconds);

    class MonthClockHand : public Shape<4>
    {
    public:
        MonthClockHand()
        {
            ShapePoint<int> points[4] = { { -2, -80 }, { -2, 15 }, { 2, 15 }, { 2, -80 } };
            setShape(points);
        }
    };

    class WeekClockHand : public Shape<4>
    {
    public:
        WeekClockHand()
        {
            ShapePoint<int> points[4] = { { -2, -45 }, { -2, 15 }, { 2, 15 }, { 2, -45 } };
            setShape(points);
        }
    };

protected:
    Image centerDot;
    Image centerDot2;

    TextAreaWithOneWildcard day;
    Unicode::UnicodeChar dayValueBuffer[3];

    MonthClockHand monthHand;
    WeekClockHand weekHand;

#if !defined(USE_BPP) || USE_BPP==16
    PainterRGB565 handColorWhite;
#elif USE_BPP==24
    PainterRGB888 handColorWhite;
#endif
};

class HomeView : public DemoView<HomePresenter>
{
public:
    HomeView();
    virtual ~HomeView() {};

    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void afterTransition();
    virtual void handleClickEvent(const ClickEvent &evt);
    virtual void handleDragEvent(const DragEvent &evt);
    virtual void handleKeyEvent(uint8_t key);
    virtual void handleTickEvent();

    void gotoSelectedDemoScreen(uint8_t demoIndex);
    void setSelectedHomeIndex(uint8_t demoIndex);

    /**
     * Updates the hour, minute and second hands
     * hours: 0-23
     * minutes: 0-59
     * seconds: 0-59
     */
    void handleTimeUpdated(uint8_t hours, uint8_t minutes, uint8_t seconds);

    // Updates capacity info
    void handleCapacityUpdated(int capacityData);

protected:
    static const int NUMBER_OF_DEMO_SCREENS = HomePresenter::MAX_DEMO_ID;

    HomePresenter::DemoID demoScreenIndex[NUMBER_OF_DEMO_SCREENS];
    bool clickAbort;

    Container demoScreensContainer[NUMBER_OF_DEMO_SCREENS];
    SwipeContainer menuContainer;
    Image backgroundLeft;
    Image backgroundRight;
    Image backgroundImage[NUMBER_OF_DEMO_SCREENS];

    // Notification members
    BitmapId commonId;
    BitmapId menuId;
    TextArea titleTxt;
    ListLayout list;
    ScrollableContainer scrollCnt;

    static const int numberOfListElements = 10;
    MenuListElement listElements[numberOfListElements + 1];
    TextAreaWithOneWildcard notiTitle[numberOfListElements];
    Unicode::UnicodeChar notiTitleBuffer[numberOfListElements][30];
    // Callback that is assigned to each list element
    Callback<HomeView, MenuListElement &> listElementClickedCallback;
    /**
     * Handler of list element clicks.
     */
    void listElementClicked(MenuListElement &element);
    void refreshNotificationList();

    // Clock members
    BitmapId clockId;
    ClockWidget analogClock;

    // DigitalClock members
    Image batteryIcon;
    Image chargerIcon;

    TextAreaWithOneWildcard percent;
    Unicode::UnicodeChar percentValueBuffer[5];
    TextAreaWithOneWildcard week;
    Unicode::UnicodeChar weekValueBuffer[10];
    TextAreaWithOneWildcard time;
    Unicode::UnicodeChar timeValueBuffer[6];
    TextAreaWithOneWildcard date;
    Unicode::UnicodeChar dateValueBuffer[13];

    // CompoundClock members
    BitmapId clock2Id;
    CompoundClockWidget analogClock2;

    // Player members
    TextAreaWithOneWildcard song;
    Unicode::UnicodeChar songValueBuffer[40];
    static const int SONG_MAX_LEN_PER_LINE = 12;
    Button play;
    Button previous;
    Button next;
    Button plus;
    Button minus;
    Button volume;
    Button volumeBar;

    enum VolumeLevel {
        VOLUME_LEVEL0 = 0,
        VOLUME_LEVEL1,
        VOLUME_LEVEL2,
        VOLUME_LEVEL3,
        VOLUME_LEVEL4,
        VOLUME_LEVEL5,
        VOLUME_LEVEL6,
        VOLUME_LEVEL7,
        MAX_VOLUME_LEVEL
    };

    bool isPlaying;
    bool mute;
    VolumeLevel level;
    static VolumeLevel levelToRestoreAfterMute;

    BitmapId pauseId;
    BitmapId playId;
    BitmapId previousId;
    BitmapId nextId;
    BitmapId volumeId[MAX_VOLUME_LEVEL];

    Callback<HomeView, const AbstractButton &> onButtonPressed;
    void buttonPressedHandler(const AbstractButton &button);

    void addNotificationScreen();
    void addClockFaceScreen();
    void addClockScreen();
    void addDigitalClockScreen();
    void addCompoundClockScreen();
    void addPlayerScreen();
    void refreshPlayerScreen();
    void refreshSong();
    void loadPauseId();
    void loadVolumeId(VolumeLevel volume);
    VolumeLevel plusVolume(VolumeLevel volume);
    VolumeLevel minusVolume(VolumeLevel volume);
};

#endif // HOME_VIEW_HPP
