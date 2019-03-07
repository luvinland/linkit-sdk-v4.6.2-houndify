/* Copyright Statement:
 *
 * (C) 2005-2016 MediaTek Inc. All rights reserved.
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
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design headset_gui_ref_design
 * @{
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design_ble_find_me_client ble_find_me_client
 * @{

@par Overview
  - Feature description
    - This feature demonstrates how to use the Find Me Profile through Bluetooth
      LE to find a remote device.

@par Hardware and software environment
  - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.

@par Directory contents
  - Source and header files
    - \b inc:                     Common header files.
    - \b inc/ble_find_me_client:  Find Me client feature related header
                                  files.
    - \b src/bt_app_common.c:     Bluetooth common source code.
    - \b src/bt_gattc_discovery.c:
                                  GATT client discovery service source code.
    - \b src/ble_find_me_client/ble_find_me_client.c:
                                  Find Me client feature source code.
    - \b src/ble_find_me_client/ble_find_me_client_screen.c:
                                  Find Me client feature UI source code.

@par Run the feature
  - Build and Download
    - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.
  - Install the application "MediaTek_SmartDevice.apk" located under
    the "sdk_root/tools/smart_device/Android/" folder on Android smartphone
    and launch the application.
  - Power on the MT2533 EVB.
  - Click "scan" on the smartphone to discover devices with Bluetooth LE
    using GATT profile, find and connect to the MT2533 EVB.
  - After the Bluetooth LE connection is established, click "FMP" to display
    Find Me client feature on the MT2533 EVB. Then the smartphone will alert
    or vibrate as an indication.
  - Exit Find Me client feature to stop alert or vibration on the smartphone.
*/
/**
 * @}
 * @}
 * @}
 * @}
 */