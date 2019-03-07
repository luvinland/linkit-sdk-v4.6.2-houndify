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

#include "FreeRTOS.h"
#include "keypad_custom.h"
#include "hal_keypad.h"
#include "hal_keypad_table.h"
#include "bt_sink_app_keypad.h"
#include "bt_sink_app_main.h"
#ifdef MTK_AUDIO_MP3_ENABLED
#include "audio_player.h"
#endif

hal_keypad_status_t hal_keypad_register_callback(hal_keypad_callback_t callback, void *user_data);
hal_keypad_status_t hal_keypad_enable(void);
bool keypad_custom_init(void);
hal_keypad_status_t hal_keypad_get_key(hal_keypad_event_t *keypad_event);

static const bt_sink_app_keypad_mapping_t g_bt_sink_app_key_mapping[] = {
    {DEVICE_KEY_FUNCTION, BT_SINK_SRV_KEY_FUNC},
    {DEVICE_KEY_VOL_UP, BT_SINK_SRV_KEY_PREV},
    {DEVICE_KEY_VOL_DOWN, BT_SINK_SRV_KEY_NEXT}
};

static const bt_sink_srv_key_action_t g_bt_sink_app_keydown_mapping[] = {
    BT_SINK_SRV_KEY_ACT_NONE,
    BT_SINK_SRV_KEY_ACT_PRESS_DOWN,
    BT_SINK_SRV_KEY_ACT_LONG_PRESS_DOWN
};

static bt_sink_app_keypad_key_t g_bt_sink_app_key[] = {
    {DEVICE_KEY_FUNCTION,  HAL_KEYPAD_KEY_RELEASE},
    {DEVICE_KEY_VOL_UP, HAL_KEYPAD_KEY_RELEASE},
    {DEVICE_KEY_VOL_DOWN, HAL_KEYPAD_KEY_RELEASE}
};

void bt_sink_app_keypad_init(void)
{
    hal_keypad_status_t status;

    keypad_custom_init();

    status = hal_keypad_register_callback(bt_sink_app_keypad_callback, NULL);
    if (status != HAL_KEYPAD_STATUS_OK) {
        bt_sink_app_report("[Sink][Keypad] register callback fail, status:%d", status);
    }

    status = hal_keypad_enable();
    if (status != HAL_KEYPAD_STATUS_OK) {
        bt_sink_app_report("[Sink][Keypad] keypad enable fail, status = %d", status);
    }
}

void bt_sink_app_keypad_callback(void *user_data)
{
    bt_sink_app_keypad_post_event(BT_SINK_EVENT_APP_KEY_INPUT);
}

void bt_sink_app_keypad_post_event(bt_sink_srv_event_t key_event)
{
    BaseType_t xHigherPriorityTaskWoken;
    bt_sink_app_event_t event;

    memset(&event, 0, sizeof(bt_sink_app_event_t));
    event.event_id = key_event;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    if (xQueueSendFromISR(bt_sink_app_context.queue_handle, &event, &xHigherPriorityTaskWoken) == pdTRUE) {
        // Now the buffer is empty we can switch context if necessary.
        if (xHigherPriorityTaskWoken) {
            // Actual macro used here is port specific.
            portYIELD_FROM_ISR(pdTRUE);
        }
    }
}

static bt_sink_app_keypad_key_t *bt_sink_app_keypad_find_key(uint8_t value)
{
    uint16_t i;
    for (i = 0; i < (sizeof(g_bt_sink_app_key) / sizeof(bt_sink_app_keypad_key_t)); i++) {
        if (value == g_bt_sink_app_key[i].value) {
            return &g_bt_sink_app_key[i];
        }
    }
    return NULL;
}

static bt_sink_srv_key_value_t bt_sink_app_keypad_find_sink_key(uint8_t value)
{
    uint16_t i;
    bt_sink_srv_key_value_t sink_key = BT_SINK_SRV_KEY_NONE;

    for (i = 0; i < (sizeof(g_bt_sink_app_key_mapping) / sizeof(bt_sink_app_keypad_mapping_t)); i++) {
        if (value == g_bt_sink_app_key_mapping[i].value) {
            sink_key = g_bt_sink_app_key_mapping[i].sink_key;
            break;
        }
    }
    return sink_key;
}

static void bt_sink_app_keypad_handler(bt_sink_app_keypad_key_t *next)
{
    bt_sink_app_keypad_key_t *current = bt_sink_app_keypad_find_key(next->value);

    bt_sink_app_report("[Sink][KEY] state:%d, value:%d", next->state, next->value);

    if (NULL != current && current->state != next->state) {
        bt_sink_srv_key_value_t sink_key = bt_sink_app_keypad_find_sink_key(next->value);
        bt_sink_srv_key_action_t sink_action = BT_SINK_SRV_KEY_ACT_NONE;

        if (HAL_KEYPAD_KEY_RELEASE == next->state) {
            switch (current->state) {
                case HAL_KEYPAD_KEY_PRESS:
                    sink_action = BT_SINK_SRV_KEY_ACT_PRESS_UP;
                    break;

                case HAL_KEYPAD_KEY_LONG_PRESS:
                    sink_action = BT_SINK_SRV_KEY_ACT_LONG_PRESS_UP;
                    break;

                default:
                    break;
            }
            current->state = next->state;
        } else if (next->state <= HAL_KEYPAD_KEY_LONG_PRESS) {
            sink_action = g_bt_sink_app_keydown_mapping[next->state];
            current->state = next->state;
        } else {
            // HAL_KEYPAD_KEY_REPEAT
            // HAL_KEYPAD_KEY_PMU_LONG_PRESS
        }
        if (BT_SINK_SRV_KEY_ACT_NONE != sink_action) {
            bt_sink_srv_key_action(sink_key, sink_action);
            /* forward key & action */
            #ifdef MTK_AUDIO_MP3_ENABLED
            audio_player_sink_key_action(sink_key, sink_action);
            #endif
        }
        bt_sink_app_report("[Sink][KEY] key:0x%x, action:0x%x", sink_key, sink_action);
    }
}

bt_sink_srv_status_t bt_sink_app_keypad_event_handler(bt_sink_srv_event_t event_id, void *parameters)
{
    bt_sink_app_keypad_key_t sink_key_event;

    if (BT_SINK_EVENT_APP_KEY_INPUT == event_id) {
        hal_keypad_event_t keypad_event;

        if (HAL_KEYPAD_STATUS_OK == hal_keypad_get_key(&keypad_event)) {
            memset(&sink_key_event, 0, sizeof(bt_sink_app_keypad_key_t));
            sink_key_event.state = keypad_event.state;
            sink_key_event.value = keypad_custom_translate_keydata(keypad_event.key_data);

            bt_sink_app_keypad_handler(&sink_key_event);
        }
    }
    return BT_SINK_SRV_STATUS_EVENT_STOP;
}

