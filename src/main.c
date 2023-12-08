#include "main.h"

volatile unsigned char led_on;

int main(void) {
    // Enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // reset LD1 pin functions
    GPIOB->MODER &= ~0x3U;
    GPIOB->PUPDR &= ~0x3;
    GPIOB->OSPEEDR &= ~0x3;
    GPIOB->OTYPER &= ~0x3;

    // set LD1 activation to pull up
    GPIOB->MODER |= 0x1U;

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    GPIOC->MODER &= ~(0x3U << 13);
    GPIOC->PUPDR &= ~(0x3 << 13);

    SYSCFG->EXTICR[3] &= ~(SYSCFG_EXTICR4_EXTI13_Msk);
    SYSCFG->EXTICR[3] |=  (SYSCFG_EXTICR4_EXTI13_PC);

    // Setup the button's EXTI line as an interrupt.
    EXTI->IMR  |=  (1 << 13);
    // Disable the 'rising edge' trigger (button release).
    EXTI->FTSR &= ~(1 << 13);
    // Enable the 'falling edge' trigger (button press).
    EXTI->RTSR |=  (1 << 13);

    NVIC_SetPriority(EXTI15_10_IRQn, 0x03);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    // set LD1 on
    led_on = 0x0;
    while (1) {
        if (led_on) {
            GPIOB->ODR |= 0x1U;
        }
        else {
            GPIOB->ODR &= ~(1);
        }
    }
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & (1 << 13)) {
    // Clear the EXTI status flag.
    EXTI->PR |= (1 << 13);
    // Toggle the global 'led on?' variable.
    led_on = !led_on;
    }
}