#include "main.h"

#define RA8875_DATAREAD 0x40  ///< See datasheet
#define RA8875_CMDWRITE 0x80  ///< See datasheet

volatile uint32_t tick;
volatile unsigned char led_on;

int main(void) {
    systick_init(8000);
    // enable the GPIOB peripheral in 'RCC_AHB1ENR'.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // enable LED1
    GPIOB->MODER |= 0x1U;
    GPIOB->ODR |= 0x1U;

    // same thing for LED2
    GPIOB->MODER |= (0x1U << 14);
    GPIOB->ODR |= (0x1U << 7);

    delay_ms(1000);

    // enable GPIOC peripheral
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

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
    GPIOB->MODER |= (0x1U << 16);

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
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

    // enable SPI interface on APB2
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 = (SPI_CR1_DFF | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_BR_0 | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA);
    SPI1->CR2 |= SPI_CR2_SSOE;

    // set reset pin low
    GPIOB->ODR &= ~(0x1 << 8);
    // wait 100us
    delay_ms(100);
    // set reset pin high
    GPIOB->ODR |= (0x1 << 8);
    // wait 100us
    delay_ms(100);
    
    SPI1->CR1 |= SPI_CR1_SPE;
    // dummy read
    uint16_t cmd_write = (RA8875_CMDWRITE << 8);
    uint16_t data_read = (RA8875_DATAREAD << 8);

    SPI1->CR1 &= ~(0x1 << 8);
    while(!(SPI1->SR & SPI_SR_TXE)){}
    SPI1->DR = cmd_write;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    uint16_t data = SPI1->DR;
    SPI1->CR1 |= (0x1 << 8);

    SPI1->CR1 &= ~(0x1 << 8);
    SPI1->DR = data_read;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    data = SPI1->DR;
    SPI1->CR1 |= (0x1 << 8);

    // real read
    SPI1->CR1 &= ~(0x1 << 8);
    SPI1->DR = cmd_write;
    while(!(SPI1->SR & SPI_SR_TXE)){}
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    data = SPI1->DR;
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

void systick_init(uint32_t reload) {
	SysTick->LOAD = (uint32_t)(reload - 1UL);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

void delay_ms(uint32_t t) {
    __IO uint32_t tmp = SysTick->CTRL;
    ((void)tmp);

    while(t) {
        if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) t--;    
    }
}
