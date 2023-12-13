#include "main.h"

#define RA8875_DATAREAD 0x40  ///< See datasheet
#define RA8875_CMDWRITE 0x80  ///< See datasheet

volatile uint32_t tick;
volatile unsigned char led_on;

int main(void) {
    tick = 0;
    SysTick_Config(2000);
    // Enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // reset LD1 pin functions
    GPIOB->MODER &= ~0x3U;
    GPIOB->PUPDR &= ~0x3;
    GPIOB->OSPEEDR &= ~0x3;
    GPIOB->OTYPER &= ~0x1;

    // set LD1 activation to pull up
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
    
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

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

    // set GPIOB for RST pin
    GPIOB->MODER &= ~(0x3U << 16);
    GPIOB->PUPDR &= ~(0x3U << 16);
    GPIOB->OSPEEDR &= ~(0x3U << 16);
    GPIOB->OTYPER &= ~(0x1U << 8);
    GPIOB->MODER |= (0x1U << 16);

    // set SPI GPIOA pins to alternate mode
    GPIOA->MODER |= (0x2 << 8);
    GPIOA->PUPDR |= (0x2 << 8);

    GPIOA->MODER |= (0x2 << 10);
    GPIOA->PUPDR |= (0x2 << 10);

    GPIOA->MODER |= (0x2 << 12);
    GPIOA->PUPDR |= (0x2 << 12);

    GPIOA->MODER |= (0x2 << 14);
    GPIOA->PUPDR |= (0x2 << 14);

    // select SPI in AFRL
    GPIOA->AFR[0] = 0x55550000;

    //NVIC_SetPriority(SPI1_IRQn, 0x03);
    //NVIC_EnableIRQ(SPI1_IRQn);

    // enable SPI interface on APB2
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // select SPI baud rate
    // set CPOL and CPHA
    SPI1->CR1 |= 0x7;
    // set 16bit format
    SPI1->CR1 |= (0x1 << 11);
    // configure LSB or MSB first
    // setup SSM pin
    SPI1->CR1 |= (0x1 << 9);
    // set SPE
    SPI1->CR1 |= (0x1 << 6);

    // set chip select
    SPI1->CR1 |= (0x1 << 8);

    // set pin low
    GPIOB->ODR |= (0x0 << 8);
    // wait 100us
    delay(100);
    // set high
    GPIOB->ODR |= (0x1 << 8);
    // wait 100us
    delay(100);
    // dummy read
    // write command
    uint16_t cmd_write = (RA8875_CMDWRITE << 8);
    uint16_t data_read = (RA8875_DATAREAD << 8);

    SPI1->CR1 &= ~(0x1 << 8);
    SPI1->DR = cmd_write;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    SPI1->CR1 |= (0x1 << 8);

    SPI1->CR1 &= ~(0x1 << 8);
    SPI1->DR = data_read;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    uint16_t data = SPI1->DR;
    SPI1->CR1 |= (0x1 << 8);

    // real read
    SPI1->CR1 &= ~(0x1 << 8);
    SPI1->DR = cmd_write;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    SPI1->CR1 |= (0x1 << 8);

    SPI1->CR1 &= ~(0x1 << 8);
    SPI1->DR = data_read;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    data = SPI1->DR;
    SPI1->CR1 |= (0x1 << 8);

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

void SysTick_Handler(void) {
    tick++;	
}

void delay(uint32_t d) {
    volatile uint32_t start = tick;
    while((tick - start) < d) {}
}
