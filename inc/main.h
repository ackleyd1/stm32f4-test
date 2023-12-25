#ifndef _DEA_MAIN_H
#define _DEA_MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "ssd1306.h"

void systick_init(uint32_t);
void delay_ms(uint32_t);
void i2c_write_blocking(uint8_t addr, uint8_t *buf, uint32_t buflen);

#endif
