#include "main.h"

int main(void) {
    // Enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    volatile uint32_t tmp __attribute__((unused)) = RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    tmp = RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN;

    // reset and set LD1 Mode to push pull mode
    GPIOB->MODER &= ~0x3U;
    GPIOB->PUPDR &= ~0x3;
    GPIOB->OSPEEDR &= ~0x3;
    GPIOB->OTYPER &= ~0x3;

    // reset and set LD1 activation to pull up
    GPIOB->MODER |= 0x1U;

    // set the pin to on
    GPIOB->ODR |= 0x1U;
    while (1) {

    }
}