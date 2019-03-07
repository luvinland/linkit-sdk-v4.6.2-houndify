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

#include <new>
#include <gui/common/FrontendApplication.hpp>
#include <mvp/View.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/transitions/NoTransition.hpp>
#include <touchgfx/transitions/SlideTransition.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/home_screen/HomeView.hpp>
#include <gui/home_screen/HomePresenter.hpp>
#include <gui/home_screen/NotificationListView.hpp>
#include <gui/home_screen/NotificationListPresenter.hpp>
#include <gui/home_screen/MassStorageView.hpp>
#include <gui/home_screen/MassStoragePresenter.hpp>
#include <gui/main_menu_screen/MainMenuView.hpp>
#include <gui/main_menu_screen/MainMenuPresenter.hpp>
#include <gui/setting_screen/SettingView.hpp>
#include <gui/setting_screen/SettingPresenter.hpp>
#include <gui/setting_screen/SetClockView.hpp>
#include <gui/setting_screen/SetClockPresenter.hpp>
#include <gui/setting_screen/SetTimeView.hpp>
#include <gui/setting_screen/SetTimePresenter.hpp>
#include <gui/setting_screen/SetDateView.hpp>
#include <gui/setting_screen/SetDatePresenter.hpp>
#include <gui/setting_screen/SetBluetoothView.hpp>
#include <gui/setting_screen/SetBluetoothPresenter.hpp>
#include <gui/setting_screen/SetHeartRateView.hpp>
#include <gui/setting_screen/SetHeartRatePresenter.hpp>
#include <gui/setting_screen/SetGPSView.hpp>
#include <gui/setting_screen/SetGPSPresenter.hpp>
#include <gui/setting_screen/ScreenTimeoutView.hpp>
#include <gui/setting_screen/ScreenTimeoutPresenter.hpp>
#include <gui/setting_screen/SetUSBModeView.hpp>
#include <gui/setting_screen/SetUSBModePresenter.hpp>
#include <gui/setting_screen/BTDeviceListView.hpp>
#include <gui/setting_screen/BTDeviceListPresenter.hpp>
#include <gui/setting_screen/BTDeviceSearchListView.hpp>
#include <gui/setting_screen/BTDeviceSearchListPresenter.hpp>
FrontendApplication::FrontendApplication(Model &m, FrontendHeap &heap)
    : MVPApplication(),
      transitionCallback(),
      frontendHeap(heap),
      model(m)
{
}

void FrontendApplication::gotoHomeScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoHomeScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoHomeScreenImpl()
{
    makeTransition< HomeView, HomePresenter, NoTransition, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::backHomeScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::backHomeScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::backHomeScreenImpl()
{
    makeTransition< HomeView, HomePresenter, SlideTransition<NORTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoNotificationScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoNotificationScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoNotificationScreenImpl()
{
    makeTransition< NotificationListView, NotificationListPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoMassStorageScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoMassStorageScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoMassStorageScreenImpl()
{
    makeTransition< MassStorageView, MassStoragePresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoMainMenuScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoMainMenuScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoMainMenuScreenImpl()
{
    makeTransition< MainMenuView, MainMenuPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::backMainMenuScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::backMainMenuScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::backMainMenuScreenImpl()
{
    makeTransition< MainMenuView, MainMenuPresenter, SlideTransition<NORTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSettingScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSettingScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSettingScreenImpl()
{
    makeTransition< SettingView, SettingPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::backSettingScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::backSettingScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::backSettingScreenImpl()
{
    makeTransition< SettingView, SettingPresenter, SlideTransition<NORTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSetClockScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSetClockScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSetClockScreenImpl()
{
    makeTransition< SetClockView, SetClockPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSetTimeScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSetTimeScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSetTimeScreenImpl()
{
    makeTransition< SetTimeView, SetTimePresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSetDateScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSetDateScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSetDateScreenImpl()
{
    makeTransition< SetDateView, SetDatePresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSetBluetoothScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSetBluetoothScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSetBluetoothScreenImpl()
{
    makeTransition< SetBluetoothView, SetBluetoothPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSetHeartRateScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSetHeartRateScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSetHeartRateScreenImpl()
{
    makeTransition< SetHeartRateView, SetHeartRatePresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoSetGPSScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoSetGPSScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoSetGPSScreenImpl()
{
    makeTransition< SetGPSView, SetGPSPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoScreenTimeoutScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoScreenTimeoutScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoScreenTimeoutScreenImpl()
{
    makeTransition< ScreenTimeoutView, ScreenTimeoutPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoUSBModeScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoUSBModeScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoUSBModeScreenImpl()
{
    makeTransition< SetUSBModeView, SetUSBModePresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
void FrontendApplication::gotoBTConnectionScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoBTConnectionScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoBTConnectionScreenImpl()
{
    makeTransition< BTDeviceListView, BTDeviceListPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::backBTConnectionScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::backBTConnectionScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::backBTConnectionScreenImpl()
{
    makeTransition< BTDeviceListView, BTDeviceListPresenter, SlideTransition<NORTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoBTDeviceOpScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoBTDeviceOpScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoBTDeviceOpScreenImpl()
{
    makeTransition< BTDeviceOpView, BTDeviceOpPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoBTDeviceSearchListScreen()
{
    transitionCallback = Callback< FrontendApplication >(this, &FrontendApplication::gotoBTDeviceSearchListScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoBTDeviceSearchListScreenImpl()
{
    makeTransition< BTDeviceSearchListView, BTDeviceSearchListPresenter, SlideTransition<SOUTH>, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
