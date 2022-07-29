#include "main.h"

int main(void) {
    // Enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // reset and set LD2 Mode to output mode
    GPIOB->MODER &= ~(0x3 << (LD2*2));
    GPIOB->MODER |= (0x1 << (LD2*2));

    // reset and set LD2 activation to pull up
    // GPIOB->PUPDR &= ~(0x3 << (LD2*2));
    // GPIOB->PUPDR |= (0x1 << (LD2*2));

    GPIOB->IDR |= (1<< LD2);
    //uint32_t gpiob_input = ;
    while (1) {

    }
}