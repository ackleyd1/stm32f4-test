#ifndef _DEA_MAIN_H
#define _DEA_MAIN_H

#include <stdint.h>
#include "stm32f4xx.h"

void systick_init(uint32_t);
void delay_ms(uint32_t);
uint16_t spi_write(SPI_TypeDef, uint16_t);

#endif
