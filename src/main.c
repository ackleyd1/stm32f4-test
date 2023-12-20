#include "main.h"

volatile unsigned char led_on;

int main(void) {
    // Enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // turn LED1 on
    GPIOB->MODER |= 0x1U;
    GPIOB->ODR |= 0x1U;

    // turn LED2 on
    GPIOB->MODER |= (0x1U << 14);
    GPIOB->ODR |= (0x1U << 7);

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

    // set GPIOs as AF in open-drain
    GPIOF->MODER |= 0x2;
    GPIOF->MODER |= (0x2 << 2);
    GPIOF->OTYPER |= 0x2;

    // select AF to use
    GPIOF->AFR[0] = 0x44;

    // set input clock
    I2C2->CR2 |= 0x2;
    // config clock rate
    I2C2->CCR |= 0xa;
    // config rise time
    I2C2->TRISE |= 0x3;
    // enable i2c interface
    I2C2->CR1 |= 0x1;
    // generate start
    I2C2->CR1 |= (0x1 << 8);

    // set LD1 on
    led_on = 0;
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
