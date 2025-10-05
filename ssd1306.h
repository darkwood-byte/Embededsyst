#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/spi.h"

void ssd1306_init(spi_inst_t *spi, uint cs, uint dc);
void ssd1306_clear(void);
void ssd1306_draw_string(int x, int y, const char *text);
void ssd1306_update(void);

#include "ssd1306.c"

#endif
