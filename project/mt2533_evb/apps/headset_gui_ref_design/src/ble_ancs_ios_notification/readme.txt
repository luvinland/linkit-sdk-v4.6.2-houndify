/* Copyright Statement:
 *
 * (C) 2005-2017  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its
 * licensors. Without the prior written permission of MediaTek and/or its
 * licensors, any reproduction, modification, use or disclosure of MediaTek
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute
 * (as applicable) MediaTek Software if you have agreed to and been bound by
 * the applicable license agreement with MediaTek ("License Agreement") and
 * been granted explicit permission to do so within the License Agreement
 * ("Permitted User").  If you are not a Permitted User, please cease any
 * access or use of MediaTek Software immediately.
 */

/**
 * @addtogroup mt2533_evb mt2533_evb
 * @{
 * @addtogroup mt2533_evb_apps apps
 * @{
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design headset_gui_ref_design
 * @{
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design_ble_ancs_ios_notification ble_ancs_ios_notification
 * @{

@par Overview
  - Feature description
    - This feature demonstrates a convenient approach to access notifications
      generated on iOS devices through Bluetooth LE link.
      The received notificaitons are shown in a pop up message.

@par Hardware and software environment
  - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.

@par Directory contents
    - \b src/ble_ancs_ios_notification/ble_ancs_app.c:  Notifications and 
                                                        attributes file.
    - \b src/ble_ancs_ios_notification/bt_ancs_common.c:Bluetooth stack 
                                                        callback file.
    - \b inc/ble_ancs_ios_notification/ble_ancs_app.h:  Main header file.
    - \b inc/ble_ancs_ios_notification/bt_ancs_common.h:Common header file.

@par Run the feature
  - Build and Download
    - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.
  - Power on the MT2533 EVB.
  - Download and install an app, such as "LightBlue", to connect to iOS
    devices that support Bluetooth LE.
  - Scan and find MT2533 on LightBlue, tap to start Bluetooth LE connection.
    On a pairing pop up window on iPhone, select "pair". A message, such as
    "5 msg" on MT2533 EVB indicates a successful operation.
*/
/**
 * @}
 * @}
 * @}
 * @}
 */