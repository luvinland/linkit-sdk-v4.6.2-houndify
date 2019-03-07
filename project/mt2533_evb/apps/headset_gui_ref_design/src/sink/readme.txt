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
 * @addtogroup mt2533_evb_apps_headset_gui_ref_design_sink sink
 * @{
 
@par Overview
  - Feature description
    - The Bluetooth Sink operates as a Bluetooth headset that
      supports Bluetooth Hands-Free, Advanced Audio Distribution and
      Audio/Video Remote Control Profiles.
  - The sink feature provides:
    - 1. Connection management, such as manage Bluetooth EDR connection, 
         auto-reconnect to a remote device after the Bluetooth is powered
         on or the Bluetooth link is lost. 
    - 2. Call management, such as answer or reject an incoming call, 
         redial on the smartphone, hang up or hold
         an active call, swap duo SCO calls, switch audio path, handle the 
         three way call.
    - 3. Bluetooth music. Streaming music from an audio device, streaming 
         control includes play or pause the music, move to previous or next track,
         Advanced Wireless Stereo (AWS) feature for high quality streaming between
         two headsets. For more details on AWS, please contact MediaTek customer
         support.

@par Hardware and software environment
  - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.

@par Directory contents
  - \b src/sink/bt_sink_app_keypad.c:
                                 Sink keypad file.
  - \b src/sink/bt_sink_app_cmd.c:
                                 Sink command file.
  - \b src/sink/bt_sink_app_event.c:
                                 Sink event file.
  - \b src/sink/bt_sink_app_main.c:
                                 Sink main program file.
  - \b src/sink/bt_sink_app_mapping_table.c:
                                 Sink action mapping file.
  - \b inc/sink/bt_sink_app_keypad.h:
                                 Sink keypad header file.
  - \b inc/sink/bt_sink_app_cmd.h:
                                 Sink command header file.
  - \b inc/sink/bt_sink_app_event.h:
                                 Sink event header file.
  - \b inc/sink/bt_sink_app_main.h:
                                 Sink main program header file.

@par Run the feature
  - Build and Download
    - Please refer to mt2533_evb/apps/headset_gui_ref_design/readme.txt.
  - Power on the MT2533 EVB.
  - Long press the function key to make the MT2533 EVB discoverable.
  - Use smartphone to search, bond and connect to the MT2533 EVB with the name
    "BT_Headset_xxxxxxxxxxxx", where xxxxxxxxxxxx is the MT2533 EVB Bluetooth
    EDR address.
  - After connection is established with the smartphone, the following
    operations will be available:
      \code
      ----------------------------------------------------------------------------------------------------------------
      |                        |                                          State                                      |
      |                        |-------------------------------------------------------------------------------------|
      |    Key Operation       |  Power on  | Connected |Incoming|Outgoing|Active |3-way       |Held and   |Audio    |
      |                        |            |           |call    |call    |       |incoming    |active call|streaming|
      |------------------------|------------|-----------|--------|--------|-------|------------|-----------|---------|
      |            |           |            |           |        |        |       |Hold Active | Hang up   | Pause   |
      |            |Short press|            |Play audio | Answer |Hang up |Hung up|and         | active    | audio   |
      |            |           |            |           |        |        |       |accept other|           |         |
      |  Function  |-----------|------------|-----------|--------|--------|-------|------------|-----------|---------|
      |            |           |            |Redial last|        |        |Hold   |Reject new  |Swap active|         |
      |            |Long press |Discoverable|dialed     | Reject |        |call   |incoming    |and hlod   |         |
      |            |           |            |number     |        |        |       |call        |           |         |
      |------------|-----------|------------|-----------|--------|--------|-------|------------|-----------|---------|
      |            |Short press|            |           |        |        |Volume |            |Volume up  |         |
      |            |           |            |           |        |        |up     |            |           |         |
      |    Next    |-----------|------------|-----------|--------|--------|-------|------------|-----------|---------|
      |            |Long press |            |Next track |        |        |Volume |            |Volume max |Next     |
      |            |           |            |           |        |        |max    |            |           |track    |
      |------------|-----------|------------|-----------|--------|--------|-------|------------|-----------|---------|
      |            |Short press|Inquiry and |           |        |        |Volume |            |Volume down|         |
      |            |           |connect AWS |           |        |        |down   |            |           |         |
      |    Prev    |-----------|------------|-----------|--------|--------|-------|------------|-----------|---------|
      |            |Long press |            |Previous   |        |        |Volume |            |Volume min |Previous |
      |            |           |            |track      |        |        |min    |            |           |track    |
      ----------------------------------------------------------------------------------------------------------------
      \endcode
*/
/**
 * @}
 * @}
 * @}
 * @}
 */
