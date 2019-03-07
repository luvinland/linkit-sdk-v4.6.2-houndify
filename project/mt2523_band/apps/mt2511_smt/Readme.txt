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

/**
  * @page iot_ea1 Sensor event data display on smart phone via BT transmission demo project guidelines
  * @{

@par Overview

This application contains an sample which demonstrates how to obtain sensor data from physical sensors and virtual sensors on sensor DTB and sensor algorithm library. The sensor data transfer to Android smart phone via BT connection.

This sample describes how to use sensor subsys SDK API and BT API
  - Subscribe a type of sensor data and wait updated sensor data in registered callback function
  - Unsubscribe a type of sensor data
  - Create a BT SPP connection in server mode
  - Send sensor data to smart phone via BT SPP connection

@par Hardware and Software environment

  - This sample runs on sensor DTB attached to MT7687
  - iot_ea1_demo.apk runs on Android smart phone
  - Must connect the antenna to MT7687 for BT transmission

@par Directory contents

  - apps/iot_ea1/iot_ea1_demo.apk               Demo app to show sensor data on Android smrat phone
  - apps/iot_ea1/inc/FreeRTOSConfig.h           FreeRTOS configuration file
  - apps/iot_ea1/inc/main.h                     Main program header file
  - apps/iot_ea1/inc/bt_log.h                   BT log header file
  - apps/iot_ea1/src/main.c                     Main program
  - apps/iot_ea1/src/system_mt7687.c            MT7687 system clock configuration file
  - apps/iot_ea1/src/syscalls.c                 Syscall funcitons

@par Prepare the demo

  - Prepare an Android smart phone and install iot_ea1_demo.apk on it

@par Run the demo

  - Build source code of this demo application
    - in source tree root folder and enter build command : ./build.sh iot_ea1
  - Download this demo application to MT7687 by IoT_Tool(flash tool)
  - Bluetooth pairing between Android smart phone and MT7687
  - Launch iot_ea1_demo app on Android smart phone
    - iot_ea1_demo app show bluetooth devices list
    - Press MT7687 bluetooth device item (MT7687 bluetooth device name is "MT7687_HDK") on iot_ea1_demo app
    - Receive sensor data from MT7687 after few seconds
  - Observe physical sensor data changing on iot_ea1_demo app
    - Move MT7687 for accelerometer data changing
    - Rotate MT7687 for gyroscope data changing
    - Move MT7687 for magnetic field data changing
    - Upstairs or downstairs with MT7687 for pressure data changing
    - Cover proximity sensor for proximity data changing
  - Observe virtual sensor data changing on iot_ea1_demo app
    - Walk with MT7687 for pedometer data changing
    - Stand, sit or lie with MT7687 for activity data changing
    - Shake MT7687 for gesture data changing

* @}
*/
