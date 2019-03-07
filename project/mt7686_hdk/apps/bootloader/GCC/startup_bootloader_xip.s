;/* Copyright Statement:
; *
; * (C) 2005-2016  MediaTek Inc. All rights reserved.
; *
; * This software/firmware and related documentation ("MediaTek Software") are
; * protected under relevant copyright laws. The information contained herein
; * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
; * Without the prior written permission of MediaTek and/or its licensors,
; * any reproduction, modification, use or disclosure of MediaTek Software,
; * and information contained herein, in whole or in part, shall be strictly prohibited.
; * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
; * if you have agreed to and been bound by the applicable license agreement with
; * MediaTek ("License Agreement") and been granted explicit permission to do so within
; * the License Agreement ("Permitted User").  If you are not a Permitted User,
; * please cease any access or use of MediaTek Software immediately.
; * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
; * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
; * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
; * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
; * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
; * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
; * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
; * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
; * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
; * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
; * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
; * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
; * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
; * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
; * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
; * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
; * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
; */

.file "startup_bootloader.s"
.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb



/* start address for the initialization values of the .data section.
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss


.globl _start
_start:
  b Reset_Handler


/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*/

.section  .reset_handler
.weak  Reset_Handler
.type  Reset_Handler, %function
Reset_Handler:
  ldr  sp, =_estack    		 /* set stack pointer */

/* Copy the data segment initializers from flash to SRAM */
  ldr  r1, =_sidata
  ldr  r2, =_sdata
  ldr  r3, =_edata
  bl  Data_Init

/* Initial BSS section and data section(optional) */
  ldr  r2, =_sbss
  ldr  r3, =_ebss
  bl  Bss_Init

/* ram_text section init */
  ldr  r1, =_ram_code_load
  ldr  r2, =_ram_code_start
  ldr  r3, =_ram_code_end
  bl  Data_Init

/* noncached_data section init */
  ldr  r1, =_ram_noncached_rw_load
  ldr  r2, =_ram_noncached_rw_start
  ldr  r3, =_ram_noncached_rw_end
  bl  Data_Init

  ldr  r2, =_ram_noncached_zi_start
  ldr  r3, =_ram_noncached_zi_end
  bl  Bss_Init

/* tcm section init */
  ldr  r1, =_tcm_text_load
  ldr  r2, =_tcm_text_start
  ldr  r3, =_tcm_text_end
  bl  Data_Init

  ldr  r2, =_tcm_zi_start
  ldr  r3, =_tcm_zi_end
  bl  Bss_Init

/* disable interrupt */
//TODO how to check interrupt disabled , check PRIMASK = 1
//TODO reference : http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/CHDBIBGJ.html
  CPSID  I

/* Call the bootloader main function.*/
  bl  main

end_bootloader:
  //bl bootloader_error_hanbler
  bl end_bootloader

/*utility function*/
.globl JumpCmd
JumpCmd:
  ORR  r0, #0x01
  BX  r0

/**
 * @brief  This is data init sub-function
 * @param  None
 * @retval None
*/
  .section  .text.Data_Init,"ax",%progbits
Data_Init:
CopyDataLoop:
  cmp     r2, r3
  ittt    lo
  ldrlo   r0, [r1], #4
  strlo   r0, [r2], #4
  blo     CopyDataLoop
  bx  lr
  .size  Data_Init, .-Data_Init

/**
 * @brief  This is bss init sub-function
 * @param  None
 * @retval None
*/
  .section  .text.Bss_Init,"ax",%progbits
Bss_Init:
ZeroBssLoop:
  cmp     r2, r3
  ittt    lo
  movlo   r0, #0
  strlo   r0, [r2], #4
  blo     ZeroBssLoop
  bx  lr
  .size  Bss_Init, .-Bss_Init

/* vector table */
.section  .isr_vector,"a",%progbits
.type  g_pfnVectors, %object
.size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
