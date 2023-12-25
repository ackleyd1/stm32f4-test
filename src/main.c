#include "main.h"

volatile unsigned char led_on;

int main(void) {
    systick_init(8000);
    // enable peripherals
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
    GPIOF->OTYPER |= 0x3;
    GPIOF->PUPDR |= 0x1;
    GPIOF->PUPDR |= (0x1 << 2);
    GPIOF->OSPEEDR |= 0x2;
    GPIOF->OSPEEDR |= (0x2 << 2);

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

    SSD1306_init();

     // Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)
    struct render_area frame_area = {
        start_col: 0,
        end_col : SSD1306_WIDTH - 1,
        start_page : 0,
        end_page : SSD1306_NUM_PAGES - 1
        };

    calc_render_area_buflen(&frame_area);

    // zero the entire display
    uint8_t buf[SSD1306_BUF_LEN];
    for (int i = 0; i < SSD1306_BUF_LEN; i++) {
        buf[i] = '\0';
    }
    render(buf, &frame_area);

    // intro sequence: flash the screen 3 times
    for (int i = 0; i < 3; i++) {
        SSD1306_send_cmd(SSD1306_SET_ALL_ON);    // Set all pixels on
        delay_ms(500);
        SSD1306_send_cmd(SSD1306_SET_ENTIRE_ON); // go back to following RAM for pixel state
        delay_ms(500);
    }

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

void i2c_write_blocking(uint8_t addr, uint8_t *buf, uint32_t buflen) {
    // prepare for acknowledgement
    I2C2->CR1 |= (0x1 << 10);
    // generate start
    I2C2->CR1 |= (0x1 << 8);
    // wait for SB set
    while(!(I2C2->SR1 & I2C_SR1_SB)) {}
    // write slave address
    *((__IO uint8_t *)&I2C2->DR) = addr;
    // wait for addr ack set and read SR2
    while(!(I2C2->SR1 & I2C_SR1_ADDR)) {}
    while(!(I2C2->SR2 & I2C_SR2_MSL)) {}
    // write first byte, wait for TxE and continue
    for (int i = 0; i < buflen; i++) {
       I2C2->DR = buf[i];
       while(!(I2C2->SR1 & I2C_SR1_TXE)) {}
    }
    // set stop bit
    I2C2->CR1 |= (0x1 << 9);

}
