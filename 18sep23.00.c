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

#define LED_PIN 2   

#define NUM_LEDS 8

#define ANI_COUNT 12
#define ANI_SPEED 15

#define MAX_MODES 9

volatile bool status = 0;
volatile int delta = 0;
volatile int mode = 6;

volatile int green = 0;
volatile int red = 100;
volatile int blue = 0;

int max_delta = 40;

uint64_t start_time;
uint64_t last_rotary_triger;
uint64_t last_button_triger;

PIO pio = pio0;
uint sm = 0;

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

void rotary_encoder_callback(uint gpio, uint32_t events) {
    bool can_trigger = (INPUT_MS * 1000) < time_us_64() - last_rotary_triger;
    if (gpio == ROTARY_PIN_A && (events & GPIO_IRQ_EDGE_FALL) && can_trigger) {  
        if (!secure_gpio(1)){return;}
        if (secure_gpio(0)){return;}
        delta = (delta + 1) % max_delta; 
        last_rotary_triger = time_us_64();
    }
    if (gpio == ROTARY_PIN_B && (events & GPIO_IRQ_EDGE_FALL && can_trigger)) {  
        if (!secure_gpio(0)){return;}
        if (secure_gpio(1)){return;}
        delta = (delta + max_delta - 1) % max_delta;
        last_rotary_triger = time_us_64();
    }
     if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE && (BUTTON_MS * 1000) < time_us_64() - last_button_triger)){
        if (!secure_gpio(BUTTON_PIN)){return;}
        status = !status;
        last_button_triger = time_us_64();
        mode = (mode + 1) % MAX_MODES;
     }
    return;
}

void init_all(void){

    stdio_init_all();

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
        if (mode != 5){return;}
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
int main() {

    init_all();
    
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