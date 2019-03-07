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

#ifndef NOTIFICATION_LIST_VIEW_HPP
#define NOTIFICATION_LIST_VIEW_HPP

#include <gui/common/DemoView.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/widgets/canvas/Circle.hpp>
#include <touchgfx/containers/ScrollableContainer.hpp>
#include <touchgfx/containers/ListLayout.hpp>

#if !defined(USE_BPP) || USE_BPP==16
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>
#elif USE_BPP==24
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>
#endif

#include <gui/home_screen/NotificationListPresenter.hpp>
#include <gui/setting_screen/MenuListElement.hpp>

/**
* The View for a template screen. In the MVP paradigm, the view is a
* passive interface that only displays the user interface and routes
* any events from the user interface to the presenter. Normally it is the
* responsibility of the presenter to act upon those events.
*
* By deriving from View, this class has a presenter pointer, which is configured
* automatically.
*/
class NotificationListView : public DemoView<NotificationListPresenter>
{
public:
    /**
    * The view constructor. Note that the constructor runs before the new screen is
    * completely initialized, so do not place code here that causes anything to be drawn.
    * Use the setupScreen function instead.
    */
    NotificationListView();

    virtual ~NotificationListView() {}

    /**
    * This function is called automatically when the view is activated. This function
    * should add widgets to the root container, configure widget sizes and event
    * callbacks, et cetera.
    */
    virtual void setupScreen();

    /**
    * This function is called automatically when transitioning to a different screen
    * than this one. Can optionally be used to clean up.
    */
    virtual void tearDownScreen();

    virtual void handleKeyEvent(uint8_t key);

private:
    TextArea headlineTxt;
    TextArea mainTxt;

    TextAreaWithOneWildcard notiListContent;
    Unicode::UnicodeChar notiListContentBuffer[300];
    Unicode::UnicodeChar notiListContentBuffer1[350];

    Image backgroundImage;
    BitmapId backgroundId;
    TextArea titleTxt;
    ScrollableContainer scrollCnt;
    ListLayout list;
};

#endif // NOTIFICATION_LIST_VIEW_HPP
