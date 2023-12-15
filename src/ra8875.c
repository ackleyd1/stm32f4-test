#include "main.h"
#include "ra8875.h"

void ra8875_init(void) {
	// PLL init
	write_reg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 11);
	delay_ms(1);
	write_reg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
	delay_ms(1);

	write_reg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);
	/* Timing values */
	uint8_t pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
	uint8_t hsync_start = 32;
	uint8_t hsync_pw = 96;
	uint8_t hsync_finetune = 0;
	uint8_t hsync_nondisp = 26;
	uint8_t vsync_pw = 2;
	uint16_t vsync_nondisp = 32;
	uint16_t vsync_start = 23;
	uint16_t _height = 480;
	uint16_t _width = 800;
	uint8_t _voffset = 0;

	write_reg(RA8875_PCSR, pixclk);
	delay_ms(1);

	/* Horizontal settings registers */
	write_reg(RA8875_HDWR, (_width / 8) - 1); // H width: (HDWR + 1) * 8 = 480
	write_reg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
	write_reg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2) /
			    8); // H non-display: HNDR * 8 + HNDFTR + 2 = 10
	write_reg(RA8875_HSTR, hsync_start / 8 - 1); // Hsync start: (HSTR + 1)*8
	write_reg(RA8875_HPWR, RA8875_HPWR_LOW +
	            (hsync_pw / 8 - 1)); // HSync pulse width = (HPWR+1) * 8

	/* Vertical settings registers */
	write_reg(RA8875_VDHR0, (uint16_t)(_height - 1 + _voffset) & 0xFF);
	write_reg(RA8875_VDHR1, (uint16_t)(_height - 1 + _voffset) >> 8);
	write_reg(RA8875_VNDR0, vsync_nondisp - 1); // V non-display period = VNDR + 1
	write_reg(RA8875_VNDR1, vsync_nondisp >> 8);
	write_reg(RA8875_VSTR0, vsync_start - 1); // Vsync start position = VSTR + 1
	write_reg(RA8875_VSTR1, vsync_start >> 8);
	write_reg(RA8875_VPWR,
	   RA8875_VPWR_LOW + vsync_pw - 1); // Vsync pulse width = VPWR + 1

	/* Set active window X */
	write_reg(RA8875_HSAW0, 0); // horizontal start point
	write_reg(RA8875_HSAW1, 0);
	write_reg(RA8875_HEAW0, (uint16_t)(_width - 1) & 0xFF); // horizontal end point
	write_reg(RA8875_HEAW1, (uint16_t)(_width - 1) >> 8);

	/* Set active window Y */
	write_reg(RA8875_VSAW0, 0 + _voffset); // vertical start point
	write_reg(RA8875_VSAW1, 0 + _voffset);
	write_reg(RA8875_VEAW0,
	   (uint16_t)(_height - 1 + _voffset) & 0xFF); // vertical end point
	write_reg(RA8875_VEAW1, (uint16_t)(_height - 1 + _voffset) >> 8);

	/* Clear the entire window */
	write_reg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
	delay_ms(500);

	// turn display on
	write_reg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);

	// enable GPIO
	write_reg(RA8875_GPIOX, 1);

	// configure PWM
	write_reg(RA8875_P1CR, RA8875_P1CR_ENABLE | (RA8875_PWM_CLK_DIV1024 & 0xF));
	write_reg(RA8875_P1DCR, 255);

	// fill screen with white
	fill_screen(_width, _height, RA8875_WHITE);
}

void write_reg(uint8_t reg, uint8_t data) {
	write_command(reg);
	write_data(data);
}

uint8_t read_reg(uint8_t reg) {
	write_command(reg);
	return read_data();
}

void write_command(uint8_t reg) {
	uint16_t msg = (RA8875_CMDWRITE << 8) | reg;
	spi_write_blocking(SPI1, msg);
}

void write_data(uint8_t data) {
	uint16_t msg = (RA8875_DATAWRITE << 8) | data;
	spi_write_blocking(SPI1, msg);
}

uint8_t read_data() {
	uint16_t msg = (RA8875_DATAREAD << 8);
	uint16_t data;
	data = spi_write_blocking(SPI1, msg);
	return (uint8_t)(data & 0x00FF);
}

void fill_screen(uint16_t width, uint16_t height, uint16_t color) {
	write_reg(0x91, 0);
	write_reg(0x92, 0);
	
	write_reg(0x93, 0);
	write_reg(0x94, 0);

	write_reg(0x95, width-1);
	write_reg(0x96, (width-1)>>8);

	write_reg(0x97, height-1);
	write_reg(0x98, (height-1)>>8);

	write_reg(0x63, (color & 0xf800) >> 11);
	write_reg(0x64, (color & 0x07e0) >> 5);
	write_reg(0x65, (color & 0x001f));

	// draw
	write_reg(RA8875_DCR, 0xb0);
	wait_poll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void wait_poll(uint8_t reg, uint8_t flag) {
	while (1) {
		uint8_t temp = read_reg(reg);
		if(!(temp & flag)) break;
	}
}


void set_mode(enum RA8875modes mode) {
	write_command(RA8875_MWCR0);
	uint8_t temp = read_data();
	
	if (mode == RA8875_gfx_mode){
		temp &= ~RA8875_MWCR0_TXTMODE; // bit 7
		write_data(temp);
	} else if (mode == RA8875_txt_mode) {
		temp |= RA8875_MWCR0_TXTMODE;
		write_data(temp);

		// select internal ROM font
		write_command(0x21);
		temp = read_data();
		temp &= ~((1 << 7) | (1 << 5)); // clear bit 5 and 7
		write_data(temp);
	}
}
void blink_cursor(uint8_t rate) {
	write_command(RA8875_MWCR0);
	uint8_t temp = read_data();
	temp |= RA8875_MWCR0_CURSOR;
	write_data(temp);

	write_command(RA8875_MWCR0);
	temp = read_data();
	temp |= RA8875_MWCR0_BLINK;
	write_data(temp);

	if(rate > 255) {
		rate = 255;	
	}
	write_command(RA8875_BTCR);
	write_data(rate);
}

void set_cursor(uint16_t x, uint16_t y) {
	write_command(0x2a);
	write_data(x & 0xff);
	write_command(0x2b);
	write_data(x >> 8);
	
	write_command(0x2c);
	write_data(y & 0xff);
	write_command(0x2d);
	write_data(y >> 8);
}

void set_text_color(uint16_t fg, uint16_t bg) {
	// set foreground
	write_command(0x63);
	write_data((fg & 0xf800) >> 11);
	write_command(0x64);
	write_data((fg & 0x07e0) >> 5);
	write_command(0x65);
	write_data((fg & 0x001f));
	
	// set background
	write_command(0x60);
	write_data((bg & 0xf800) >> 11);
	write_command(0x61);
	write_data((bg & 0x07e0) >> 5);
	write_command(0x62);
	write_data((bg & 0x001f));

}

void set_text_transparency(uint8_t on) {
	if (on) {
		// clear transparency
		write_command(0x22);
		uint8_t temp = read_data();
		temp |= (1 << 6); // set bit 6
		write_data(temp);

	} else {
		// clear transparency
		write_command(0x22);
		uint8_t temp = read_data();
		temp &= ~(1 << 6); // clear bit 6
		write_data(temp);
	}

}

void write_text(const char *s, uint16_t len) {
	write_command(RA8875_MRWC);
	for(uint16_t i = 0; i < len; ++i) {
		write_data(s[i]);	
	}
}

void set_scroll_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode) {
	// horizontal start point
	write_reg(0x38, x);
	write_reg(0x39, x >> 8);

	// vertical start point
	write_reg(0x3a, y);
	write_reg(0x3b, y >> 8);

	// horizontal end point
	write_reg(0x3c, x+w);
	write_reg(0x3d, (x+w) >> 8);

	// vertical end point
	write_reg(0x3e, y+h);
	write_reg(0x3f, (y+h) >> 8);

	// scroll function setting
	write_reg(0x52, mode);
}

void scroll_x(uint16_t dist) {
	write_reg(0x24, dist);
	write_reg(0x25, dist >> 8);
}

void scroll_y(uint16_t dist) {
	write_reg(0x26, dist);
	write_reg(0x27, dist >> 8);
}
