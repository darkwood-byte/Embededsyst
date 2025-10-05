#include "ssd1306.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <string.h>

#define WIDTH 128
#define HEIGHT 32
static uint cs_pin, dc_pin;
static spi_inst_t *oled_spi;
static uint8_t buffer[WIDTH * HEIGHT / 8];

static void oled_command(uint8_t c) {
    gpio_put(dc_pin, 0);
    gpio_put(cs_pin, 0);
    spi_write_blocking(oled_spi, &c, 1);
    gpio_put(cs_pin, 1);
}

static void oled_data(const uint8_t *data, size_t len) {
    gpio_put(dc_pin, 1);
    gpio_put(cs_pin, 0);
    spi_write_blocking(oled_spi, data, len);
    gpio_put(cs_pin, 1);
}

void ssd1306_init(spi_inst_t *spi, uint cs, uint dc) {
    oled_spi = spi;
    cs_pin = cs;
    dc_pin = dc;

    oled_command(0xAE);
    oled_command(0xD5); oled_command(0x80);
    oled_command(0xA8); oled_command(0x1F);
    oled_command(0xD3); oled_command(0x00);
    oled_command(0x40);
    oled_command(0x8D); oled_command(0x14);
    oled_command(0x20); oled_command(0x00);
    oled_command(0xA1);
    oled_command(0xC8);
    oled_command(0xDA); oled_command(0x02);
    oled_command(0x81); oled_command(0x8F);
    oled_command(0xD9); oled_command(0xF1);
    oled_command(0xDB); oled_command(0x40);
    oled_command(0xA4);
    oled_command(0xA6);
    oled_command(0xAF);
    memset(buffer, 0, sizeof(buffer));
}

void ssd1306_clear(void) {
    memset(buffer, 0, sizeof(buffer));
}

void ssd1306_draw_string(int x, int y, const char *text) {
    int offset = (y / 8) * WIDTH + x;
    for (int i = 0; text[i]; i++) {
        buffer[offset++] = 0x7E;
    }
}

void ssd1306_update(void) {
    oled_command(0x21);
    oled_command(0);
    oled_command(WIDTH - 1);
    oled_command(0x22);
    oled_command(0);
    oled_command((HEIGHT / 8) - 1);
    oled_data(buffer, sizeof(buffer));
}
