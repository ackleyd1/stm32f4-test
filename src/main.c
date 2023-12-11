#include "main.h"

int main(void) {
    // Enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // reset and set LD1 Mode to push pull mode
    GPIOB->MODER &= ~0x3U;
    GPIOB->PUPDR &= ~0x3;
    GPIOB->OSPEEDR &= ~0x3;
    GPIOB->OTYPER &= ~0x1;

    // reset and set LD1 activation to pull up
    GPIOB->MODER |= 0x1U;

    // set the pin to on
    GPIOB->ODR |= 0x1U;

    // enable GPIOC peripheral
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // same thing for LD2
    GPIOB->MODER &= ~(0x3U << 14);
    GPIOB->PUPDR &= ~(0x3U << 14);
    GPIOB->OSPEEDR &= ~(0x3U << 14);
    GPIOB->OTYPER &= ~(0x1U << 7);
    GPIOB->MODER |= (0x1U << 14);
    GPIOB->ODR |= (0x1U << 7);

    // configure button
    GPIOC->MODER &= ~(0x3U << 26);
    GPIOC->PUPDR &= ~(0x3U << 26);
    
    uint8_t pressed = 0;
    volatile uint16_t gpioc_input;
while (1) {
	gpioc_input = ~GPIOC->IDR;
	if(gpioc_input & (0x1U << 13)) {
		if(!pressed) {
			GPIOB->ODR ^= (1 << 7);		
		}	
		pressed = 1;
	}
	else { pressed = 0; }

    }
}
