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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "sys_init.h"
#include "task_def.h"

#include "syslog.h"


/****************************************************************************
 *
 * CLI Example Code
 *
 ****************************************************************************/


/* Include this file for CLI engine prototypes definition. */
#include "cli.h"

/* Include this file for I/O interface API default implementation */
#include "io_def.h"

/* Include this file for integer parsing */
#include "toi.h"


/*	getchar, putchar declaration  */
//GETCHAR_PROTOTYPE;
//PUTCHAR_PROTOTYPE;


#define HISTORY_LINE_MAX    (128)
#define HISTORY_LINES       (20)

static char s_history_lines[ HISTORY_LINES ][ HISTORY_LINE_MAX ];
static char *s_history_ptrs[ HISTORY_LINES ];

static char s_history_input[ HISTORY_LINE_MAX ];
static char s_history_parse_token[ HISTORY_LINE_MAX ];


/* Example of user custom CLI command, note that the prototype matches cli_cmd_handler_t */
static uint8_t _example_cli_command_hello(uint8_t argc, char *argv[])
{
    printf("Hello world\n");
    return 0;
}


static uint8_t _example_cli_command_echo(uint8_t argc, char *argv[])
{
    int i;

    for (i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }

    printf("\n");

    return 0;
}


static uint8_t _example_cli_command_int(uint8_t argc, char *argv[])
{
    int         i;
    uint8_t     type;
    uint32_t    value;

    for (i = 0; i < argc; i++) {

        value = toi(argv[i], &type);
        switch (type) {
            case TOI_ERR:
                printf("'%s' is not a valid integer\n", argv[i]);
                break;
            case TOI_BIN:
                printf("'%s' is a binary number, value is %u in decimal\n", argv[i], (unsigned int)value);
                break;
            case TOI_OCT:
                printf("'%s' is an octal number, value is %u in decimal\n", argv[i], (unsigned int)value);
                break;
            case TOI_DEC:
                printf("'%s' is a decimal number, value is %u\n", argv[i], (unsigned int)value);
                break;
            case TOI_HEX:
                printf("'%s' is a hexadecimal number, value is %u in decimal\n", argv[i], (unsigned int)value);
            default:
                printf("this never happens!\n");
                break;
        }
    }

    return 0;
}


/* CLI Command list
   Format:
   {<Command name>, <Command help message>, <Command function>, <Sub command (cmd_t)>}\

   NOTE:
   The last one must be {NULL, NULL, NULL, NULL}
*/
static cmd_t  _cmds_normal[] = {
    { "hello", "hello world",               _example_cli_command_hello, NULL },
    { "echo",  "echo input 'parameters'",   _example_cli_command_echo,  NULL },
    { "parse", "parse input 'parameters'",  _example_cli_command_int,   NULL },
    /*	Add your custom command here */
    { NULL, NULL, NULL, NULL }
};


/* CLI control block */
static cli_t _cli_cb = {
    .state  = 1,
    .echo   = 0,
    .get    = __io_getchar,
    .put    = __io_putchar,
    .cmd	= &_cmds_normal[0]
};


/* FreeRTOS task of CLI */
static void _example_cli_task(void *param)
{
    int i;

    /**
     *  Prepare CLI history buffer
     */
    cli_history_t *hist = &_cli_cb.history;

    for (i = 0; i < HISTORY_LINES; i++) {
        s_history_ptrs[i] = s_history_lines[i];
    }

    hist->history           = &s_history_ptrs[0];
    hist->input             = s_history_input;
    hist->parse_token       = s_history_parse_token;
    hist->history_max       = HISTORY_LINES;
    hist->line_max          = HISTORY_LINE_MAX;
    hist->index             = 0;
    hist->position          = 0;
    hist->full              = 0;

    /**
     *  Init CLI control block
     */
    cli_init(&_cli_cb);

    for (;;) {
        cli_task();
    }
}


/****************************************************************************
 *
 * C Langauge Entry Point
 *
 ****************************************************************************/


/* Create the log control block as user wishes. Here we use 'template' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(minicli_proj, PRINT_LEVEL_INFO);


int main(void)
{
    /*
     * Do system initialization, eg: hardware, nvdm and random seed.
     */

    system_init();

    /*
     * system log initialization.
     * This is the simplest way to initialize system log, that just inputs three NULLs
     * as input arguments. User can use advanved feature of system log along with NVDM.
     * For more details, please refer to the log dev guide under /doc folder or projects
     * under project/mtxxxx_hdk/apps/.
     */

    log_init(NULL, NULL, NULL);

    LOG_I(minicli_proj, "start to create task");

    /*
     * Create a user task for demo when and how to use wifi config API to change WiFI settings,
     * Most WiFi APIs must be called in task scheduler, the system will work wrong if called in main(),
     * For which API must be called in task, please refer to wifi_api.h or WiFi API reference.
     * xTaskCreate(user_wifi_app_entry,
     *             UNIFY_USR_DEMO_TASK_NAME,
     *             UNIFY_USR_DEMO_TASK_STACKSIZE / 4,
     *             NULL, UNIFY_USR_DEMO_TASK_PRIO, NULL);
     * user_wifi_app_entry is user's task entry function, which may be defined in another C file to do application job.
     * UNIFY_USR_DEMO_TASK_NAME, UNIFY_USR_DEMO_TASK_STACKSIZE and UNIFY_USR_DEMO_TASK_PRIO should be defined
     * in task_def.h. User needs to refer to example in task_def.h, then makes own task MACROs defined.
     */

    xTaskCreate(_example_cli_task,
                APP_TASK_NAME,
                APP_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                NULL,
                APP_TASK_PRIO,
                NULL);

    /*
     * Call this function to indicate the system initialize done.
     */

    SysInitStatus_Set();

    /*
     * Start the scheduler.
     */

    vTaskStartScheduler();

    /*
     * If all is well, the scheduler will now be running, and the following line
     * will never be reached.  If the following line does execute, then there was
     * insufficient FreeRTOS heap memory available for the idle and/or timer tasks
     * to be created.  See the memory management section on the FreeRTOS web site
     * for more details.
     */

    for( ;; );
}
