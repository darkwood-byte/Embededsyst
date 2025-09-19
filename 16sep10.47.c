#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define INPUT_MS 15
#define SECURE_MS 8

#define ROTARY_PIN_A 0
#define ROTARY_PIN_B 1

volatile bool status = 0;
volatile int delta = 0;
uint64_t start_time;
uint64_t last_rotary_triger;

bool secure_gpio(int pin){
    uint64_t clockticks = time_us_64();
    bool pin_bool = gpio_get(pin);
    while (time_us_64() - clockticks < SECURE_MS * 1000);
    if (pin_bool != gpio_get(pin)){return secure_gpio(pin);}//can we fix  it? . . . YES WE CAN!!
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
    return;
}

void init_all(void){

    stdio_init_all();

    last_rotary_triger = time_us_64();//delay voor de rotary encoder interupts

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

    gpio_init(16); //io input for the button of the rotary encoder
    gpio_set_dir(16, GPIO_IN);

    gpio_init(17);// output green led
    gpio_set_dir(17, true);

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
    
    bool setingpin = 0;
    bool last_setingpin = 0;
    while (true) {
        last_setingpin = setingpin;
        setingpin = gpio_get(16);
        if (setingpin && last_setingpin == 0){
            status = !status;
            printf("%d %d %d\n", status, setingpin, last_setingpin);
            update_red_leds();
            sleep_ms(150);
        }
        gpio_put(17, !status);
    }
}