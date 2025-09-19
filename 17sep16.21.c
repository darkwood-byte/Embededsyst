#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ws2812.pio.h"

#define INPUT_MS 15
#define SECURE_MS 8

#define BUTTON_MS 150

#define ROTARY_PIN_A 0
#define ROTARY_PIN_B 1

#define BUTTON_PIN 16

#define LED_PIN 2   // pas aan naar je eigen data pin

#define NUM_LEDS 8
PIO pio = pio0;
uint sm = 0;

void put_pixel(uint32_t pixel_grb) {
    // Zet data in de FIFO van de state machine
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) |
           ((uint32_t)r << 8)  |
           ((uint32_t)b);
}

volatile bool status = 0;
volatile int delta = 0;
uint64_t start_time;
uint64_t last_rotary_triger;
uint64_t last_button_triger;

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

void update_red_leds(void){
    gpio_put(18, (delta == 0 && status));
    gpio_put(19, (delta == 1 && status));
    gpio_put(20, (delta == 2 && status));
    gpio_put(21, (delta == 3 && status));
    return;
}

void rotary_encoder_callback(uint gpio, uint32_t events) {
    bool can_trigger = (INPUT_MS * 1000) < time_us_64() - last_rotary_triger;
    if (gpio == ROTARY_PIN_A && (events & GPIO_IRQ_EDGE_FALL) && can_trigger) {  
        if (!secure_gpio(1)){return;}
        if (secure_gpio(0)){return;}
        delta = (delta + 1) % 4; 
        last_rotary_triger = time_us_64();
        update_red_leds();
    }
    if (gpio == ROTARY_PIN_B && (events & GPIO_IRQ_EDGE_FALL && can_trigger)) {  
        if (!secure_gpio(0)){return;}
        if (secure_gpio(1)){return;}
        delta = (delta + 3) % 4;
        last_rotary_triger = time_us_64();
        update_red_leds();
    }
     if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE && (BUTTON_MS * 1000) < time_us_64() - last_button_triger)){
        if (!secure_gpio(BUTTON_PIN)){return;}
        status = !status;
        last_button_triger = time_us_64();
        update_red_leds();
        gpio_put(17, !status);
     }
    return;
}

void init_all(void){

    stdio_init_all();

    last_rotary_triger = time_us_64();//delay voor de rotary encoder interupts
    last_button_triger = time_us_64();//delay voor de button interupts

    gpio_init(ROTARY_PIN_A); //PIN A van de rotary encoder
    gpio_set_dir(ROTARY_PIN_A, GPIO_IN);
    gpio_pull_up(ROTARY_PIN_A);

    gpio_init(ROTARY_PIN_B); //PIN B van de rotary encoder
    gpio_set_dir(ROTARY_PIN_B, GPIO_IN);
    gpio_pull_up(ROTARY_PIN_B);

    gpio_set_irq_enabled_with_callback(//declaring the interupt
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

    gpio_init(16); //io input for the button of the rotary encoder
    gpio_set_dir(16, GPIO_IN);

    gpio_init(17);// output green led
    gpio_set_dir(17, true);
    gpio_put(17, !status);

    gpio_init(18);// red leds
    gpio_set_dir(18, true);

    gpio_init(19);
    gpio_set_dir(19, true);

    gpio_init(20);
    gpio_set_dir(20, true);

    gpio_init(21);
    gpio_set_dir(21, true);
}

int main() {

    init_all();

    update_red_leds();
    
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);

    while (true) {
        // Zet de LED aan (rood bijvoorbeeld)
        put_pixel(rgb_to_grb(255, 0, 0));  // max rood
        put_pixel(rgb_to_grb(255, 0, 0)); 

        sleep_ms(1000);

        // Zet de LED uit (alle kanalen 0)
        put_pixel(rgb_to_grb(0, 0, 0));
        put_pixel(rgb_to_grb(0, 0, 255)); 

        sleep_ms(1000);
    }
}