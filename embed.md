# Embedded Programming
use toolchain arm-none-eabi-gcc to cross compile
triplet identifies the platform: arm, none (no operating system), and eabi compliant. use gcc -print-multiarch to view your target platform triplet

specs are the ... you can output the default specs with gcc --dumpspecs

## flags and options seen in the wild
-ffunction-sections
-fdata-sections
-ffreestanding
-std=gnu11
--specs=nano.specs
--specs=nosys.specs

# ARM Cortex M4 (Armv7-m)
Execution begins in the ResetHandler in startup.s, where the stack pointer is set
and the initial data segments () is copied from flash () to SRAM. perhaps we dont want
to burn out our flash if we change that data?
_sidata
_sdata
_edata

Then we zero out the bss segment, which is used for static data that is not initialized
_sbss
_ebss

Further more, if you need more static data memory, you can define it in
DATA_IN_ExtSRAM or DATA_IN_ExtSDRAM

the system branches and links to system Init, which will configure our core clock
at SystemCoreClock 16'000'000 Hz according to the AHB Prescalers?
Default value of the External oscillator in 25000000 Hz
HSI_VALUE  16000000 /*!< Value of the Internal oscillator in Hz*/  
user vector table is kept at the automatic remap of boot address selected but can be offset
from either FLASH or SRAM
AHBPrescTable [16] and APBPrescTable [8] for slower clock cycles

In SystemInit we first check SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

then if data in ExtSRAM or ExtSDRAM we SystemInit_ExtMemCtl();
if we need to offset the vector table we do that SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;
and we return to startup.s

immediately upon return, we branch and link to __libc_init_array , a c library function that
will setup the static constructors for ctors and dtors, among other things. This functionality
is dependent on the c runtime library you link with. with the GNU C Compiler, .ctors and dtors are special table sections made for destrubtors and constructors

upon return, we branch and link to main which should never terminate. if it does, we then branch
to the link register, which returns us back to the reset handler again

Initializes the Flash interface and the Systick
in main, we call first call HAL_Init which will first configure Flash prefetch, Instruction cache, Data cache so

FLASH->ACR |= FLASH_ACR_ICEN
FLASH->ACR |= FLASH_ACR_DCEN
FLASH->ACR |= FLASH_ACR_PRFTEN

and then set the nested vector interrupt group priority
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4)
configure a 1ms tick with HAL_InitTick(TICK_INT_PRIORITY)

__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /* Configure the SysTick to have interrupt in 1ms time basis*/
  if (HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq)) > 0U)
  {
    return HAL_ERROR;
  }

  /* Configure the SysTick IRQ priority */
  if (TickPriority < (1UL << __NVIC_PRIO_BITS))
  {
    HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0U);
    uwTickPrio = TickPriority;
  }
  else
  {
    return HAL_ERROR;
  }

  /* Return function status */
  return HAL_OK;
}

then init the low level hardware in HAL_MspInit {} and return

Now that we have initialized the systick we configure the system clock with
SystemClock_Config();

There is also the DefaultHandler, which is the code that gets called when the processor receives an unexpected interrupt. It will then loop forever, preserving the state of the processor for debugging purposes

The isr_vector or Interrupt Service Routine Vector has a minimal set defined by the
processor. For a Cortex M3 the minimal vector table is

g_pfnVectors:
  .word  _estack
  .word  Reset_Handler

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

Then we have external interrupts defined by the processor vendor
we give them weak aliases to the default handler, such that if a function is implemented with that name, it will replace the default handler for that interrupt

You can write your own drivers for peripherals using the vendor supplied include headers
or you can use the vendors HAL drivers to make the code more portable.

sets up the data structures, addressing, interrupts, and macros for accessing the peripherals
based upon the processor core layout


if you compile without the libraries you will run into symbols being undefined at the linkage step

Lets overview some of the drivers we will need to reimplement
The main ones are cortex, power, flash, rcc (real-time clock control), dma, exti, and gpio.

Clock distribution network. Only peripherals that are needed are powered

For SYSCLK, we can choose between
High Speed Internal (HSI) oscillator clock
High Speed External (HSE) oscillator clock
Phase Locked Loop (PLL) oscillator clock

There are also secondary clock sources for other functionality
such as an independent watchdog and RTC used for auto-wakeup
from Stop/Standby mode

Once a clock source is selected, we must configure the internal system and
peripheral clocks

internal clocks are system clock, advanced high performance bus (AHB)
low speed advanced peripheral bus APB1
and high speed advanced peripheral bus APB2
Each can be configured using prescalers

Use registers to switch clocks for external peripherals
RCC_AHB1ENR and RCC_AHB2ENR
some peripheral clocks may have an independent source



# Sources
- bluetechs (https://bluetechs.wordpress.com/zothers/x/rcc/)[here]
- bare metal stm32 programming (https://vivonomicon.com/2018/04/02/bare-metal-stm32-programming-part-1-hello-arm/)[here]
- embed in c23 (https://thephd.dev/finally-embed-in-c23)[here]
- interrupt vector table (https://charleskorn.com/2016/04/17/a-deeper-look-at-the-stm32f4-project-template-getting-things-started/)[here]