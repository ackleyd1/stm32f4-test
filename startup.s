.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

.global reset_handler

.type  reset_handler, %function
reset_handler: 
  ldr r0, =_estack       /* set stack pointer */
  mov sp, r0

/* Copy the data segment initializers from flash to SRAM */  
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b copy_sidata_loop

copy_sidata:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

copy_sidata_loop:
  adds r4, r0, r3
  cmp r4, r1
  bcc copy_sidata
  
// zero fill the bss segment
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b zero_bss_loop

zero_bss:
  str  r3, [r2]
  adds r2, r2, #4

zero_bss_loop:
  cmp r2, r4
  bcc zero_bss

// branch to the main method
  bl SystemInit
  b  main    
.size  reset_handler, .-reset_handler