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

#ifndef SET_CLOCK_VIEW_HPP
#define SET_CLOCK_VIEW_HPP

#include <gui/common/DemoView.hpp>
#include <gui/setting_screen/SetClockPresenter.hpp>
#include <gui/setting_screen/HorizontalSlideMenu.hpp>
#include <gui/setting_screen/MenuListElement.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/widgets/Button.hpp>
#include <touchgfx/widgets/canvas/Circle.hpp>
#include <touchgfx/containers/ScrollableContainer.hpp>
#include <touchgfx/containers/ListLayout.hpp>

#if !defined(USE_BPP) || USE_BPP==16
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>
#elif USE_BPP==24
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>
#endif

#ifndef CLOCK_LIST_STYLE

class SetClockView : public DemoView<SetClockPresenter>
{
public:
    SetClockView();
    virtual ~SetClockView();

    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void handleKeyEvent(uint8_t key);

private:
    static const int numberOfElements = 3;

    Image backgroundImage;
    BitmapId backgroundId;
    BitmapId clockActiveId[numberOfElements];
    BitmapId clockInactiveId[numberOfElements];
    TextArea titleTxt;

    HorizontalSlideMenu slideMenu;
    Callback<SetClockView, const HorizontalSlideMenu &> slideMenuElementSelectedCallback;
    void slideMenuElementSelectedHandler(const HorizontalSlideMenu &menu);

    Button menuLeft;
    Button menuRight;
    Callback<SetClockView, const AbstractButton &> onButtonPressed;
    void buttonPressedHandler(const AbstractButton &button);
};

#else

class SetClockView : public DemoView<SetClockPresenter>
{
public:
    SetClockView();
    virtual ~SetClockView();

    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void handleKeyEvent(uint8_t key);

    /**
    * Handler of list element clicks.
    */
    void listElementClicked(MenuListElement &element);

private:
    static const int numberOfListElements = 3;

    Image backgroundImage;
    BitmapId backgroundId;
    BitmapId menuId;
    TextArea titleTxt;
    ScrollableContainer scrollCnt;
    ListLayout list;

    MenuListElement listElements[numberOfListElements + 1];
    Image overlay;

    // Callback that is assigned to each list element
    Callback<SetClockView, MenuListElement &> listElementClickedCallback;
};

#endif

#endif // SET_CLOCK_VIEW_HPP
