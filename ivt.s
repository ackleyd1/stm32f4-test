.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

.global ivt
.global default_handler

.type ivt %object
.section .isr_vector,"a",%progbits
ivt:
  .word  _estack
  .word  reset_handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler

.weak      NMI_Handler
.thumb_set NMI_Handler,default_handler

.weak      HardFault_Handler
.thumb_set HardFault_Handler,default_handler

.weak      MemManage_Handler
.thumb_set MemManage_Handler,default_handler

.weak      BusFault_Handler
.thumb_set BusFault_Handler,default_handler

.weak      UsageFault_Handler
.thumb_set UsageFault_Handler,default_handler

.weak      SVC_Handler
.thumb_set SVC_Handler,default_handler

.weak      DebugMon_Handler
.thumb_set DebugMon_Handler,default_handler

.weak      PendSV_Handler
.thumb_set PendSV_Handler,default_handler

.weak      SysTick_Handler
.thumb_set SysTick_Handler,default_handler
.size ivt, .-ivt

.section .text.default_handler,"ax",%progbits
default_handler:
  default_loop:
    b default_loop
.size default_handler, .-default_handler