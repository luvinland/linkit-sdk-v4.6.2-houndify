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
 * @addtogroup mt2523_hdk mt2523_hdk
 * @{
 * @addtogroup mt2523_hdk_apps apps
 * @{
 * @addtogroup mt2523_hdk_apps_modem_wifi5931_ref_design modem_wifi5931_ref_design
 * @{

@par Overview
  - Description
    - This application demonstrates how to use LinkIt 2523 HDK with a UMTS
      modem and Wi-Fi SOC.
    - There are three build command options for this application - modem
      only, Wi-Fi only, modem and Wi-Fi.
  - Feature list
    - GNSS. Measures and displays location information continuously.
    - Wi-Fi. Provides network access with MediaTek MT5931 Wi-Fi SOC.
      Please refer to the document \<sdk_root\>/driver/board/mt25x3_hdk/wifi/mt5931/doc
    - UMTS cellular modem. Provides fast cellular data connectivity with
      MediaTek MT6280.
  - Architecture
    - @image html modem_wifi5931_architecture.png.

@par Hardware and software environment
  - Supported HW platform
    - LinkIt 2523 HDK.
    - MediaTek MT6280 UMTS modem.
    - MediaTek MT5931 Wi-Fi SOC.
  - HDK switches and pin configuration
    - Pin mapping for the modules. Modem only:
      |         PIN Name             |GPIOx    |EINTx   |
      |------------------------------|---------|--------|
      |  NOTITF_MODEM_WAKEUP_PIN     |  1      |  1     |
      |  NOTITF_MODEM_EXCEPTION_PIN  |  13     |  11    |
      |  TRIGGER_MODEM_RESET_PIN     |  14     |        |
      |  UPDATE_HOST_STATUS_PIN      |  11     |        |
      |  QUERY_MODEM_STATUS_PIN      |  12     |        |
      |  TRIGGER_MODEM_WAKEUP_PIN    |  0      |        |
      Wi-Fi only and modem and Wi-Fi:
      |         PIN Name             |GPIOx    |EINTx   |
      |------------------------------|---------|--------|
      |  NOTITF_MODEM_WAKEUP_PIN     |  24     |  9     |
      |  NOTITF_MODEM_EXCEPTION_PIN  |  8      |  7     |
      |  TRIGGER_MODEM_RESET_PIN     |  9      |        |
      |  UPDATE_HOST_STATUS_PIN      |  4      |        |
      |  QUERY_MODEM_STATUS_PIN      |  5      |        |
      |  TRIGGER_MODEM_WAKEUP_PIN    |  29     |        |
      |  POWERON_LED_PIN             |  44     |        |
      |  TRIGGER_MODEM_POWERON_PIN   |  25     |        |
  - Environment configuration
    - Modem only:
      - The output logs are communicated through a type-A to micro-B USB
        cable to the PC from MK20 USB connector on the HDK.
      - Install the mbed serial driver according to the instructions at
        https://developer.mbed.org/handbook/Windows-serial-configuration. For
        more information, please refer to section "Installing the LinkIt 2523
        HDK drivers on Microsoft Windows" on the "LinkIt 2523 HDK User Guide"
        in [sdk_root]/doc folder.
      - Use a type-A to micro-B USB cable to connect type-A USB of the PC and
        MK20 micro-B USB connector on the LinkIt 2523 HDK. For more
        information about the connector cable, please refer to
        https://en.wikipedia.org/wiki/USB#Mini_and_micro_connectors.
      - Launch a terminal emulator program, such as Tera terminal on your PC
        for data logging through UART. For the installation details, please
        refer to section "Installing Tera terminal on Microsoft Windows" on
        the "LinkIt for RTOS Get Started Guide" in [sdk_root]/doc folder.
      - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1,
        parity: none and flow control: off.
      - LinkIt 2523 HDK uses UART0 communication with the modem (MT6280), at
        the baudrate of 921600.
    - Wi-Fi only and modem and Wi-Fi:
      - The output logs are communicated through a type-A to micro-B USB
        cable to the PC from USB PIN on the device.
      - There are two USB ports under Device Manager on your PC: debug port
        for logs, modem port for AT COMMAND.
      - Launch a terminal emulator program, such as Tera terminal on your PC
        for data logging through UART. For the installation details, please
        refer to section "Installing Tera terminal on Microsoft Windows" on
        the "LinkIt for RTOS Get Started Guide" in [sdk_root]/doc folder.
      - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1,
        parity: none and flow control: off.
      - Device uses UART2 to communicate with the modem (MT6280), at the
        baudrate of 921600.

@par Directory contents
  - Source and header files
    - \b inc:                  Common header files.
    - \b src/ept_dte:          The GPIO and EINT configuration file generated
                               by the Easy Pinmux Tool (EPT) for modem only.
    - \b src/ept_common:       The GPIO and EINT configuration file generated
                               by the EPT for Wi-Fi only and modem and Wi-Fi.
    - \b src/gnss_example:     GNSS demo source code.
    - \b src/fato_demo:        Fota demo source code.
    - \b src/resource:         Resource code for GDI display.
    - \b src/main.c:           Main program.
    - \b src/system_mt2523.c:  The configuration file of the Cortex-M4 with
                               floating point core registers and system clock.
    - \b src/task_def.c:       Task definition file.
    - \b src/at_command_serial_port.c:
                               Port service code.
    - \b src/atci_cmd.c:       AT COMMAND.
    - \b src/bt_common.c:      Bluetooth common configuration file.
    - \b src/bt_init.c:        Bluetooth initialization file.
    - \b src/gnss_cust.c:      Custom settings for GNSS operation file.
    - \b src/hci_log.c:        HCI log file.
    - \b src/io_def.c:         IO definition file.
    - \b src/lwip_network.c:   Wi-Fi initialization file.
    - \b src/wifi_custom_config.c:
                               Wi-Fi for customer file.
  - Project configuration files using GCC
    - \b GCC/feature_6280.mk:  Feature configuration file for modem only.
    - \b GCC/feature_5931.mk:  Feature configuration file for Wi-Fi only.
    - \b GCC/feature.mk:       Feature configuration file for modem and Wi-Fi.
    - \b Makefile:             Makefile.
    - \b GCC/flash.ld:         Linker script.
    - \b GCC/module_gnss.mk:   GNSS application makefile.
    - \b GCC/modem_demo.mk:    Modem application makefile.

@par Run the application
  - How to build the modem_wifi5931_ref_design application
    - Modem only
      - make command "./build.sh mt2523_hdk modem_wifi5931_ref_design
        -f=feature_6280.mk" under the SDK root folder.
    - Wi-Fi only
      - make command "./build.sh mt2523_hdk modem_wifi5931_ref_design
        -f=feature_5931.mk" under the SDK root folder.
    - Modem and Wi-Fi
      - make command "./build.sh mt2523_hdk modem_wifi5931_ref_design" under
        the SDK root folder.
*/
/**
 * @}
 * @}
 * @}
 */

