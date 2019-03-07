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
 * @addtogroup mt7686_hdk mt7686_hdk
 * @{
 * @addtogroup mt7686_hdk_hal_examples hal_examples
 * @{
 * @addtogroup mt7686_hdk_hal_examples_spi_master_polling spi_master_polling
 * @{

@par Overview
  - Example description
    - This example is a reference application to demonstrate how use the SPI
      master APIs to send data with polling mode.
    - This example does not require FreeRTOS.
  - Results
    - The system will log "---spim_send_data_polling_example ends---"
      and the waveform from the SPI pins can be observed on
      an oscilloscope or logical analyzer.

@par Hardware and software environment
  - Supported platform
    - LinkIt 7686 HDK.
  - HDK switches and pin configuration
    - SPI master module pins mapping table are shown as below.
      | SPI Pin | GPIOx     |    PINx     |
      |-------  |---------  |-----------  |
      |  CS     | GPIO_17   | J2101.G17   |
      |  SCK    | GPIO_16   | J2101.G16   |
      |  SIO0   | GPIO_15   | J2101.G15   |
      |  SIO1   | GPIO_14   | J2101.G14   |
      |  SIO2   | GPIO_13   | J2101.G13   |
      |  SIO3   | GPIO_12   | J2101.G12   |
  - Environment configuration
    - The output logs are communicated through a type-A to micro-B USB cable
      to the PC from MK20 USB connector on the HDK.
    - Install the mbed serial driver according to the instructions at
      https://developer.mbed.org/handbook/Windows-serial-configuration. For
      more information, please refer to section "Installing the LinkIt 7686
      HDK drivers on Microsoft Windows" on the "LinkIt 7686 HDK User Guide"
      in [sdk_root]/doc folder.
    - Use a type-A to micro-B USB cable to connect type-A USB of the PC and
      MK20 micro-B USB connector on the LinkIt 7686 HDK. For more information
      about the connector cable, please refer to
      https://en.wikipedia.org/wiki/USB#Mini_and_micro_connectors.
    - Launch a terminal emulator program, such as Tera terminal on your PC
      for data logging through UART. For the installation details, please
      refer to section "Installing Tera terminal on Microsoft Windows" on the
      "LinkIt for RTOS Get Started Guide" in [sdk_root]/doc folder.
    - COM port settings. baudrate: 115200, data: 8 bits, stop bit: 1, parity:
      none and flow control: off.
    - Need an oscilloscope to capture the waveform.

@par Directory contents
  - Source and header files
    - \b src/main.c:            Main program.
    - \b src/system_mt7686.c:   MT7686x system clock configuration file.
    - \b inc/hal_feature_config.h:
                                MT7686x feature configuration file.
    - \b inc/memory_map.h:      MT7686x memory layout symbol file.
    - \b GCC/startup_mt7686.s:  MT7686x startup file of GCC.
    - \b GCC/syscalls.c:        MT7686x syscalls of GCC.
    - \b MDK-ARM/startup_mt7686.s:
                                MT7686x startup file of Keil.
  - Project configuration files using GCC
    - \b GCC/feature.mk:  Feature configuration file.
    - \b GCC/Makefile.:   Makefile.
    - \b GCC/flash.ld:    Linker script.
  - Project configuration files using Keil IDE
    - \b MDK-ARM/spi_master_polling.uvprojx:
                             uVision5 Project File. Contains the project
                             structure in XML format.
    - \b MDK-ARM/spi_master_polling.uvoptx:
                             uVision5 project options. Contains the settings
                             for the debugger, trace configuration,
                             breakpoints, currently open files, etc.
    - \b MDK-ARM/flash.sct:  Linker script.
  - Project configuration files using IAR
    - \b EWARM/spi_master_polling.ewd:
                           IAR project options. Contains the settings for the
                           debugger.
    - \b EWARM/spi_master_polling.ewp:
                           IAR project file. Contains the project structure in
                           XML format.
    - \b EWARM/spi_master_polling.eww:
                           IAR workspace file. Contains project information.
    - \b EWARM/flash.icf:  Linker script.

@par Run the example
  - Build the example project with a command "./build.sh mt7686_hdk
    spi_master_polling" from the SDK root folder and download the binary file
    to LinkIt 7686 HDK.
  - Connect the related pins as shown in the "HDK switches and pin
    configuration" part.
  - Connect J2101.G12, J2101.G13, J2101.G14, J2101.G15, J2101.G16 and J2101.G15
    to oscilloscope or logical analyzer and corresponding GND.
  - Connect the HDK to the PC with a type-A to micro-B USB cable and specify
    the port on Tera terminal corresponding to "mbed Serial Port".
  - Run the example. The system will log "---spim_send_data_polling_example ends---"
    and the waveform corresponding to "0x7E, 0x55" will be captured by the
    oscilloscope or logical analyzer.
*/
/**
 * @}
 * @}
 * @}
 */
