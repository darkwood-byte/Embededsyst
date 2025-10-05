#include <stdio.h>//debuging
#include <math.h>//pot meter
#include <string.h>//voor display logica

#include "pico/stdlib.h"

#include "hardware/gpio.h"//general
#include "hardware/adc.h"//pot meter
#include "hardware/spi.h"//Pmod
#include "ws2812.pio.h"//leds

#define INPUT_MS 15
#define SECURE_MS 1

#define BUTTON_MS 150

#define ROTARY_PIN_A 0
#define ROTARY_PIN_B 1

#define POT_PIN 26
#define POT_CHANEL 0
#define POT_DIV 100000
#define POT_FACTOR 2.6
#define POT_OFSETT 1910
#define POT_TRY 20
#define POT_INTERVAL 10

#define BUTTON_PIN 2

#define LED_PIN 16

#define PIN_CS_PMOD 17
#define PIN_SCK_PMOD 18
#define PIN_MOSI_PMOD 19
#define PIN_RESET_PMOD 20
#define PIN_DC_PMOD 21

#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_PAGES (OLED_HEIGHT/8)

#define NUM_LEDS 8

#define ANI_COUNT 12
#define ANI_SPEED 15

#define MAX_MODES 9

volatile bool status = 0;
volatile int delta = 0;
volatile int mode = 6;

volatile int green = 0;
volatile int red = 20;
volatile int blue = 0;

int max_delta = 40;

uint64_t start_time;
uint64_t last_rotary_triger;
uint64_t last_button_triger;

PIO pio = pio0;
uint sm = 0;

static spi_inst_t *oled_spi;
static uint cs_pin, dc_pin;
static uint8_t buffer[OLED_WIDTH * OLED_PAGES];

static const uint8_t loadingwheel8x8_font[8][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x0f, 0x0f},
    {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},
    {0x0f, 0x0f, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00},
    {0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xf0, 0xf0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00},
    {0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
    {0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xf0, 0xf0}
};
// 8x8 font, complete ASCII 32–127
static const uint8_t font8x8_basic[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00}, // '!'
    {0x00,0x07,0x00,0x07,0x00,0x00,0x00,0x00}, // '"'
    {0x14,0x7F,0x14,0x7F,0x14,0x00,0x00,0x00}, // '#'
    {0x24,0x2A,0x7F,0x2A,0x12,0x00,0x00,0x00}, // '$'
    {0x23,0x13,0x08,0x64,0x62,0x00,0x00,0x00}, // '%'
    {0x36,0x49,0x55,0x22,0x50,0x00,0x00,0x00}, // '&'
    {0x00,0x05,0x03,0x00,0x00,0x00,0x00,0x00}, // '''
    {0x00,0x1C,0x22,0x41,0x00,0x00,0x00,0x00}, // '('
    {0x00,0x41,0x22,0x1C,0x00,0x00,0x00,0x00}, // ')'
    {0x14,0x08,0x3E,0x08,0x14,0x00,0x00,0x00}, // '*'
    {0x08,0x08,0x3E,0x08,0x08,0x00,0x00,0x00}, // '+'
    {0x00,0x50,0x30,0x00,0x00,0x00,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x00}, // '-'
    {0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02,0x00,0x00,0x00}, // '/'
    {0x3E,0x51,0x49,0x45,0x3E,0x00,0x00,0x00}, // '0'
    {0x00,0x42,0x7F,0x40,0x00,0x00,0x00,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46,0x00,0x00,0x00}, // '2'
    {0x21,0x41,0x45,0x4B,0x31,0x00,0x00,0x00}, // '3'
    {0x18,0x14,0x12,0x7F,0x10,0x00,0x00,0x00}, // '4'
    {0x27,0x45,0x45,0x45,0x39,0x00,0x00,0x00}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30,0x00,0x00,0x00}, // '6'
    {0x01,0x71,0x09,0x05,0x03,0x00,0x00,0x00}, // '7'
    {0x36,0x49,0x49,0x49,0x36,0x00,0x00,0x00}, // '8'
    {0x06,0x49,0x49,0x29,0x1E,0x00,0x00,0x00}, // '9'
    {0x00,0x36,0x36,0x00,0x00,0x00,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00,0x00,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00,0x00,0x00,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14,0x00,0x00,0x00}, // '='
    {0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00}, // '>'
    {0x02,0x01,0x51,0x09,0x06,0x00,0x00,0x00}, // '?'
    {0x32,0x49,0x79,0x41,0x3E,0x00,0x00,0x00}, // '@'
    {0x7E,0x11,0x11,0x11,0x7E,0x00,0x00,0x00}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36,0x00,0x00,0x00}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22,0x00,0x00,0x00}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C,0x00,0x00,0x00}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41,0x00,0x00,0x00}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01,0x00,0x00,0x00}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A,0x00,0x00,0x00}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,0x00}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00,0x00,0x00,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01,0x00,0x00,0x00}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41,0x00,0x00,0x00}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40,0x00,0x00,0x00}, // 'L'
    {0x7F,0x02,0x0C,0x02,0x7F,0x00,0x00,0x00}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F,0x00,0x00,0x00}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E,0x00,0x00,0x00}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06,0x00,0x00,0x00}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E,0x00,0x00,0x00}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46,0x00,0x00,0x00}, // 'R'
    {0x46,0x49,0x49,0x49,0x31,0x00,0x00,0x00}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01,0x00,0x00,0x00}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F,0x00,0x00,0x00}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F,0x00,0x00,0x00}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F,0x00,0x00,0x00}, // 'W'
    {0x63,0x14,0x08,0x14,0x63,0x00,0x00,0x00}, // 'X'
    {0x07,0x08,0x70,0x08,0x07,0x00,0x00,0x00}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43,0x00,0x00,0x00}, // 'Z'
    {0x00,0x7F,0x41,0x41,0x00,0x00,0x00,0x00}, // '['
    {0x02,0x04,0x08,0x10,0x20,0x00,0x00,0x00}, // backslash
    {0x00,0x41,0x41,0x7F,0x00,0x00,0x00,0x00}, // ']'
    {0x04,0x02,0x01,0x02,0x04,0x00,0x00,0x00}, // '^'
    {0x40,0x40,0x40,0x40,0x40,0x00,0x00,0x00}, // '_'
    {0x00,0x01,0x02,0x04,0x00,0x00,0x00,0x00}, // '`'
    {0x20,0x54,0x54,0x54,0x78,0x00,0x00,0x00}, // 'a'
    {0x7F,0x48,0x44,0x44,0x38,0x00,0x00,0x00}, // 'b'
    {0x38,0x44,0x44,0x44,0x20,0x00,0x00,0x00}, // 'c'
    {0x38,0x44,0x44,0x48,0x7F,0x00,0x00,0x00}, // 'd'
    {0x38,0x54,0x54,0x54,0x18,0x00,0x00,0x00}, // 'e'
    {0x08,0x7E,0x09,0x01,0x02,0x00,0x00,0x00}, // 'f'
    {0x0C,0x52,0x52,0x52,0x3E,0x00,0x00,0x00}, // 'g'
    {0x7F,0x08,0x04,0x04,0x78,0x00,0x00,0x00}, // 'h'
    {0x00,0x44,0x7D,0x40,0x00,0x00,0x00,0x00}, // 'i'
    {0x20,0x40,0x44,0x3D,0x00,0x00,0x00,0x00}, // 'j'
    {0x7F,0x10,0x28,0x44,0x00,0x00,0x00,0x00}, // 'k'
    {0x00,0x41,0x7F,0x40,0x00,0x00,0x00,0x00}, // 'l'
    {0x7C,0x04,0x18,0x04,0x78,0x00,0x00,0x00}, // 'm'
    {0x7C,0x08,0x04,0x04,0x78,0x00,0x00,0x00}, // 'n'
    {0x38,0x44,0x44,0x44,0x38,0x00,0x00,0x00}, // 'o'
    {0x7C,0x14,0x14,0x14,0x08,0x00,0x00,0x00}, // 'p'
    {0x08,0x14,0x14,0x18,0x7C,0x00,0x00,0x00}, // 'q'
    {0x7C,0x08,0x04,0x04,0x08,0x00,0x00,0x00}, // 'r'
    {0x48,0x54,0x54,0x54,0x20,0x00,0x00,0x00}, // 's'
    {0x04,0x3F,0x44,0x40,0x20,0x00,0x00,0x00}, // 't'
    {0x3C,0x40,0x40,0x20,0x7C,0x00,0x00,0x00}, // 'u'
    {0x1C,0x20,0x40,0x20,0x1C,0x00,0x00,0x00}, // 'v'
    {0x3C,0x40,0x30,0x40,0x3C,0x00,0x00,0x00}, // 'w'
    {0x44,0x28,0x10,0x28,0x44,0x00,0x00,0x00}, // 'x'
    {0x0C,0x50,0x50,0x50,0x3C,0x00,0x00,0x00}, // 'y'
    {0x44,0x64,0x54,0x4C,0x44,0x00,0x00,0x00}, // 'z'
    {0x00,0x08,0x36,0x41,0x00,0x00,0x00,0x00}, // '{'
    {0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00}, // '|'
    {0x00,0x41,0x36,0x08,0x00,0x00,0x00,0x00}, // '}'
    {0x10,0x08,0x08,0x10,0x08,0x00,0x00,0x00} // '~'
};

static void oled_command(uint8_t c) {
    gpio_put(dc_pin, 0);
    gpio_put(cs_pin, 0);
    spi_write_blocking(oled_spi, &c, 1);
    gpio_put(cs_pin, 1);
}

static void oled_data(const uint8_t *d, size_t len) {
    gpio_put(dc_pin, 1);
    gpio_put(cs_pin, 0);
    spi_write_blocking(oled_spi, d, len);
    gpio_put(cs_pin, 1);
}

static void ssd1306_init() {
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

static void ssd1306_update() {
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        oled_command(0xB0 + page);
        oled_command(0x00);
        oled_command(0x10);
        oled_data(&buffer[OLED_WIDTH * page], OLED_WIDTH);
    }
}

static void clear_display() {
    memset(buffer, 0, sizeof(buffer));
    ssd1306_update();
}

static void draw_char(uint8_t x, uint8_t page, char c) {
    if (c < 32 || c > 127) return;
    const uint8_t *glyph = font8x8_basic[c - 32];
    for (uint8_t i = 0; i < 8; i++) {
        if (x + i < OLED_WIDTH)
            buffer[page * OLED_WIDTH + x + i] = glyph[i];
    }
}

static void draw_wheel(uint8_t x, uint8_t page, int fase) {
    const uint8_t *glyph = loadingwheel8x8_font[fase];
    for (uint8_t i = 0; i < 8; i++) {
        if (x + i < OLED_WIDTH)
            buffer[page * OLED_WIDTH + x + i] = glyph[i];
    }
}

static void draw_string(uint8_t x, uint8_t page, const char *s) {
    while (*s) {
        draw_char(x, page, *s++);
        x += 8;
        if (x > OLED_WIDTH - 8) break;
    }
}

void ssd1306_printf(uint8_t x, uint8_t page, const char *fmt, int value) {
    char text[32];
    snprintf(text, sizeof(text), fmt, value);
    draw_string(x, page, text); // Teken de volledige string
}

void start_animation(void){
    for (int i = 0; i < 128; i++){
        draw_wheel(120, 0, i % 8);
        ssd1306_update();
        sleep_ms(50);
        switch (i)
        {
        case 8:
            draw_string(8 * 1, 0, "s");
            break;
        case 16:
            draw_string(8 * 2, 0, "t");
            break;
        case 24:
            draw_string(8 * 3, 0, "a");
            break;
        case 32:
            draw_string(8 * 4, 0, "r");
            break;
        case 40:
            draw_string(8 * 5, 0, "t");
            break;
        case 48:
            draw_string(8 * 6, 0, "i");
            break;
        case 56:
            draw_string(8 * 7, 0, "n");
            break;
        case 64:
            draw_string(8 * 8, 0, "g");
            break;
        case 72:
            draw_string(8 * 9, 0, ".");
            break;
        case 80:
            draw_string(8 * 10, 0, ".");
            break;
        case 88:
            draw_string(8 * 11, 0, ".");
            break;
        default:
            break;
        }
    }
    clear_display();  
    
    sleep_ms(1000);
}

void put_pixel(uint32_t pixel_grb) {
    // Zet data in de FIFO van de state machine <--- i luv you deepseek =]
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) |
           ((uint32_t)r << 8)  |
           ((uint32_t)b);
}

void soft_sleep_ms(int ms){
    uint64_t clockticks = time_us_64();
    while (time_us_64() - clockticks < ms * 1000);
    return;
}

bool secure_gpio(int pin){
    bool pin_bool = gpio_get(pin);
    soft_sleep_ms(SECURE_MS);
    if (pin_bool != gpio_get(pin)){return secure_gpio(pin);}//can i fix  it? . . . WE CAN!!
    return pin_bool;
}

void update_PMOD_data(void){
    clear_display();  
    ssd1306_printf(0, 0, "Volume: %d", 69);
    ssd1306_printf(0, 1, "Mode: %d", mode);
    ssd1306_printf(0, 2, "Delta: %d", delta);
    ssd1306_printf(0, 3, "WIFI status: %d", 1);
    ssd1306_update(); 
}

void rotary_encoder_callback(uint gpio, uint32_t events) {
    bool can_trigger = (INPUT_MS * 1000) < time_us_64() - last_rotary_triger;
    if (gpio == ROTARY_PIN_A && (events & GPIO_IRQ_EDGE_FALL) && can_trigger) {  
        if (!secure_gpio(1)){return;}
        if (secure_gpio(0)){return;}
        delta = (delta + 1) % max_delta; 
        last_rotary_triger = time_us_64();
        update_PMOD_data();
    }
    if (gpio == ROTARY_PIN_B && (events & GPIO_IRQ_EDGE_FALL && can_trigger)) {  
        if (!secure_gpio(0)){return;}
        if (secure_gpio(1)){return;}
        delta = (delta + max_delta - 1) % max_delta;
        last_rotary_triger = time_us_64();
        update_PMOD_data();
    }
     if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE && (BUTTON_MS * 1000) < time_us_64() - last_button_triger)){
        if (!secure_gpio(BUTTON_PIN)){return;}
        status = !status;
        last_button_triger = time_us_64();
        mode = (mode + 1) % MAX_MODES;
        update_PMOD_data();
     }
    return;
}

void init_all(void){

    stdio_init_all();

    spi_init(spi0, 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_SCK_PMOD, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_PMOD, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_PMOD); gpio_set_dir(PIN_CS_PMOD, GPIO_OUT); gpio_put(PIN_CS_PMOD, 1);
    gpio_init(PIN_DC_PMOD); gpio_set_dir(PIN_DC_PMOD, GPIO_OUT);
    gpio_init(PIN_RESET_PMOD); gpio_set_dir(PIN_RESET_PMOD, GPIO_OUT);

    gpio_put(PIN_RESET_PMOD, 0); sleep_ms(50);
    gpio_put(PIN_RESET_PMOD, 1); sleep_ms(50);

    oled_spi = spi0;
    cs_pin = PIN_CS_PMOD;
    dc_pin = PIN_DC_PMOD;

    ssd1306_init();

    adc_init();
    adc_gpio_init(POT_PIN);        // GPIO26 = pin 31 = ADC0
    adc_select_input(POT_CHANEL);  

    last_rotary_triger = time_us_64();//delay voor de rotary encoder interupts
    last_button_triger = time_us_64();//delay voor de button interupts

    uint offset = pio_add_program(pio, &ws2812_program);//pio maddnes, do not touch or i will kill u and marry your daugters son. . .
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);

    gpio_init(ROTARY_PIN_A); //PIN A van de rotary encoder
    gpio_set_dir(ROTARY_PIN_A, GPIO_IN);
    gpio_pull_up(ROTARY_PIN_A);

    gpio_init(ROTARY_PIN_B); //PIN B van de rotary encoder
    gpio_set_dir(ROTARY_PIN_B, GPIO_IN);
    gpio_pull_up(ROTARY_PIN_B);

    gpio_set_irq_enabled_with_callback(//declaring the interupts
        ROTARY_PIN_A,
        GPIO_IRQ_EDGE_FALL,
        true,
        &rotary_encoder_callback
    );
    gpio_set_irq_enabled(
    ROTARY_PIN_B,
    GPIO_IRQ_EDGE_FALL,
    true
    );
    gpio_set_irq_enabled(
    BUTTON_PIN,
    GPIO_IRQ_EDGE_RISE,
    true
    );

    gpio_init(BUTTON_PIN); //io input for the button of the rotary encoder
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
}

void single_led(int index){
    if (index >= NUM_LEDS){printf("somthing went wrong check void_single_led. . .\n"); return;}
    for(int i = 0; i < NUM_LEDS; i++){
        if (i == index){put_pixel(rgb_to_grb(red, green, blue));}
        else{put_pixel(rgb_to_grb(0, 0, 0)); }     
    }
    sleep_ms(30);
    printf("%d",index);
    return;
}

void led_run(void){
    for (int i = 0; i < NUM_LEDS; i++, single_led(i), sleep_ms(15 + delta));
    return;
}

void led_jump(void){
    led_run();
    for (int i = NUM_LEDS - 1; i >= 0; i--, single_led(i), sleep_ms(15 + delta));
    return;
}

void led_rainbow_init(void){
    for (int i = 0; i < NUM_LEDS; i++){
        for (int j = 0; j < NUM_LEDS; j++){
            if (j <= i){put_pixel(rgb_to_grb(255, 255, 255));}
            else{put_pixel(rgb_to_grb(0, 0, 0)); }
        }
        sleep_ms(40 + delta * 2);
    }
    return;
}

void led_fill(void){
    for (int i = 0; i < NUM_LEDS; i++){
        for (int j = 0; j < NUM_LEDS; j++){
            if (j <= i){put_pixel(rgb_to_grb(red, green, blue));}
            else{put_pixel(rgb_to_grb(0, 0, 0)); }
        }
        sleep_ms(40 + delta * 2);
    }
    for (int i = NUM_LEDS - 1; i >= 0; i--){
        for (int j = 0; j < NUM_LEDS; j++){
            if (j <= i){put_pixel(rgb_to_grb(red, green, blue));}
            else{put_pixel(rgb_to_grb(0, 0, 0)); }
        }
        sleep_ms(20 + delta * 2);
    }
    if (mode != 4){led_rainbow_init();}
    return;
}

void all_leds(int g, int r, int b){
    for (int i = 0; i < NUM_LEDS; i++){
        put_pixel(rgb_to_grb(b, g, r));
    }
    return;
}

void led_rainbow(void){
    int r, g, b;
    for (int h = 0; h < 360; h++) {
        int region = h / 60;       // bepaalt welk deel van de regenboog
        int step   = (h % 60) * 255 / 60;  // 0..255 fade in/out

        switch (region) {
            case 0: r = 255;       g = step;       b = 0;   break; // rood -> geel
            case 1: r = 255-step;  g = 255;        b = 0;   break; // geel -> groen
            case 2: r = 0;         g = 255;        b = step;break; // groen -> cyaan
            case 3: r = 0;         g = 255-step;  b = 255; break; // cyaan -> blauw
            case 4: r = step;      g = 0;          b = 255; break; // blauw -> magenta
            case 5: r = 255;       g = 0;          b = 255-step; break; // magenta -> rood
        }
        if (mode != 5){delta = blue; return;}
        all_leds(r, g, b);
        sleep_ms(5 + delta); 
    }
    if (mode != 5){delta = blue;}
    return;
}

void select_led_color_blue(void){
    blue = delta;
    for (int i = 0; i < NUM_LEDS; i++){
        if(i == 0 || i == NUM_LEDS - 1){put_pixel(rgb_to_grb(0, 0, blue));}
        else{put_pixel(rgb_to_grb(red, green, blue));}
        
    }
    sleep_ms(30);
    if (mode != 6){delta = green;}
    return;
}
void select_led_color_green(void){
    green = delta;
    for (int i = 0; i < NUM_LEDS; i++){
        if(i == 0 || i == NUM_LEDS - 1){put_pixel(rgb_to_grb(0, green, 0));}
        else{put_pixel(rgb_to_grb(red, green, blue));}
    }
    sleep_ms(30);
    if (mode != 7){delta = red;}
    return;
}
void select_led_color_red(void){
    red = delta;
    for (int i = 0; i < NUM_LEDS; i++){
        if(i == 0 || i == NUM_LEDS - 1){put_pixel(rgb_to_grb(red, 0, 0));}
        else{put_pixel(rgb_to_grb(red, green, blue));}
    }
    sleep_ms(30);
    return;
}
void led_chess(void){
    for (int i = 0; i < NUM_LEDS; i++){
        if(i % 2 == 0){put_pixel(rgb_to_grb(red, green, blue));}
        else{put_pixel(rgb_to_grb(0, 0, 0));}
    }
    sleep_ms(50 + delta);
    for (int i = 0; i < NUM_LEDS; i++){
        if((i + 1) % 2 == 0){put_pixel(rgb_to_grb(red, green, blue));}
        else{put_pixel(rgb_to_grb(0, 0, 0));}
    }
    sleep_ms(50 + delta);
    return;
}

int read_pot(void){
    int avg = 0;
    for (int i = 0; i < POT_TRY; i++){
        avg += pow((double)adc_read(), POT_FACTOR) / POT_DIV - POT_OFSETT;
        sleep_ms(POT_INTERVAL);
    }
    return avg / POT_TRY;
}



int main() {

    init_all();
    
    start_animation();

    update_PMOD_data();

    while (true) {
        switch (mode)
        {
        case 0:
            max_delta = NUM_LEDS;
            delta = delta % max_delta;//this is why they are ranked from least to most delta
            single_led(delta);
            break;
        case 1:
            max_delta = 40;
            led_chess();
            break;
        case 2:
            max_delta = 40;
            led_run();
            break;
        case 3:
            max_delta = 40;
            led_jump();
            break;
        case 4:
            max_delta = 40;
            led_fill();
            break;
        case 5:
            max_delta = 40;
            led_rainbow();
            break;
        case 6:
            max_delta = 255;
            select_led_color_blue();
            break;
        case 7:
            max_delta = 255;
            select_led_color_green();
            break;
        case 8:
            max_delta = 255;
            select_led_color_red();
            break;
        default:
            break;
        }
         
    }
}