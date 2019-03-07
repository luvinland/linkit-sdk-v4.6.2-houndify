/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
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
 * ("Permitted User"). If you are not a Permitted User, please cease any
 * access or use of MediaTek Software immediately. 
 */

/**
 * @addtogroup mt2533_evb mt2533_evb
 * @{
 * @addtogroup mt2533_evb_apps apps
 * @{
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design headset_ref_design
 * @{
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design_ble_find_me_server ble_find_me_server
 * @{

@par Overview
  - Feature description
    - This feature demonstrates how to find a local device through the Find
      Me Profile using Bluetooth LE.

@par Hardware and software environment
  - Please refer to mt2533_evb/apps/headset_ref_design/readme.txt.

@par Directory contents
  - Source and header files
    - \b inc:                    Common header files.
    - \b inc/ble_find_me_server: Find Me server feature header files.
    - \b src/bt_app_common.c:    Bluetooth support source code.
    - \b src/ble_find_me_server/ble_find_me_server.c:
                                 Find Me server feature source code.

@par Run the feature
  - Build and Download
    - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.
  - Install the application "MediaTek_SmartDevice.apk" located under
    the "sdk_root/tools/smart_device/Android/" folder on an Android smartphone
    and launch the application.
  - Power on the MT2533 EVB.
  - Click "scan" on the smartphone to discover devices with Bluetooth LE
    using GATT profile, then find and connect to the MT2533 EVB.
  - After the Bluetooth LE connection is established, click "find my device"
    on the Smart Device application.
  - The MT2533 EVB displays a log "ble_fmp_enable_alert: enable alert with level
    is %d success!".
  - Click "stop" on the Smart Device application and the MT2533 EVB will
    display "ble_fmp_disable_alert: disable alert success!".
*/
/**
 * @}
 * @}
 * @}
 * @}
 */