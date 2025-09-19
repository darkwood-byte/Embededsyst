#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define INPUT_MS 15
#define MS 8
#define GPIO0_PIN 0
#define GPIO1_PIN 1

volatile int delta = 0;
uint64_t start_time;
uint64_t last_rotary_triger;

bool secure_gpio(int pin){
    uint64_t clockticks = time_us_64();
    bool pin_bool = gpio_get(pin);
    while (time_us_64() - clockticks < SECURE_MS * 1000);
    if (pin_bool != gpio_get(pin)){return secure_gpio(pin);}
    return pin_bool;
}

void rotary_encoder_callback(uint gpio, uint32_t events) {
    bool can_trigger = (INPUT_MS * 1000) < time_us_64() - last_rotary_triger;
    if (gpio == GPIO0_PIN && (events & GPIO_IRQ_EDGE_FALL) && can_trigger) {  
        if (!secure_gpio(1)){return;}
        if (secure_gpio(0)){return;}
        delta = (delta + 1) % 4; 
        printf("Adebug\n");
        last_rotary_triger = time_us_64();
    }
    if (gpio == GPIO1_PIN && (events & GPIO_IRQ_EDGE_FALL && can_trigger)) {  
        if (!secure_gpio(0)){return;}
        if (secure_gpio(1)){return;}
        delta = (delta + 3) % 4;
        printf("Bdebug2\n");
        last_rotary_triger = time_us_64();
    }
    //printf("delta %d\n", delta);
    return;
}


int main() {
    // Initialiseer de standaard I/O (voor printf naar USB)
    stdio_init_all();

    last_rotary_triger = time_us_64();
    // Zet de pinnen als input
    gpio_init(GPIO0_PIN);
    gpio_set_dir(GPIO0_PIN, GPIO_IN);
    gpio_pull_up(GPIO0_PIN);
    //gpio_pull_up(GPIO0_PIN); // optioneel, afhankelijk van je schakeling

    gpio_init(GPIO1_PIN);
    gpio_set_dir(GPIO1_PIN, GPIO_IN);
    gpio_pull_up(GPIO1_PIN);
    //gpio_pull_up(GPIO1_PIN); // optioneel

    gpio_set_irq_enabled_with_callback(
        GPIO0_PIN,
        GPIO_IRQ_EDGE_FALL,
        true,
        &rotary_encoder_callback
    );
    gpio_set_irq_enabled(
    GPIO1_PIN,
    GPIO_IRQ_EDGE_FALL,
    true
    );

    gpio_init(16);
    gpio_set_dir(16, GPIO_IN);

    gpio_init(17);
    gpio_set_dir(17, true);

    gpio_init(18);
    gpio_set_dir(18, true);

    gpio_init(19);
    gpio_set_dir(19, true);

    gpio_init(20);
    gpio_set_dir(20, true);

    gpio_init(21);
    gpio_set_dir(21, true);
    bool l18 = 0;
    bool l19 = 0;
    bool l20 = 0;
    bool l21 = 0;
    // Zet ze allemaal hoog (aan)
    gpio_put(18, l18);
    gpio_put(19, l19);
    gpio_put(20, l20);
    gpio_put(21, l21);

    printf("GPIO monitor\n");
    bool old_val0 = 0;
    bool old_val1 = 0;
    bool list1[4];
    bool list2[4];
    int i = 0;
    int t = 0;
    bool leftlist1[4] = {1,1,1,1};
    bool leftlist2[4] = {0,1,0,1};
    bool rightlist1[4] = {0,1,0,1};
    bool rightlist2[4] = {1,1,1,1};
    bool first = 0;
    
    bool status = 0;
    bool setingpin = 0;
    bool last_setingpin = 0;
    while (true) {
        t++;
        // Lees de pinwaarden
        bool val0 = gpio_get(GPIO0_PIN);
        bool val1 = gpio_get(GPIO1_PIN);
        // Print de waarden naar de terminal
        if (old_val0 != val0 || old_val1 != val1){
            list1[i] = val0;
            list2[i] = val1;
            i++;
            bool changed = 1;
            if (i == 4){
                bool left = 1;
                bool right = 1;
                while (i != 0){
                    i--;
                    if (leftlist1[i] != list1[i] || leftlist2[i] != list2[i]){left = 0;}
                    if (rightlist1[i] != list1[i] || rightlist2[i] != list2[i]){right = 0;}
                    printf("%d %d, %d %d %d\n", list1[i], list2[i], leftlist1[i], leftlist2[i], i);
                }
                printf("-------------\n");
                //if (left && first){printf("left\n-------------\n"); changed = 1; delta++;}
                //if (right && first){printf("right\n-------------\n"); changed = 1; delta--;}
                first = 0;
            }
            if (changed){
                if (delta < 0){delta += 4;}
                if (delta > 3){delta -= 4;}
                l18 = (delta == 0 && status);
                l19 = (delta == 1 && status);
                l20 = (delta == 2 && status);
                l21 = (delta == 3 && status);
                gpio_put(18, l18);
                gpio_put(19, l19);
                gpio_put(20, l20);
                gpio_put(21, l21);
            }
            
            //printf("GPIO0: %d, GPIO1: %d\n", val0, val1);
        }
        old_val0 = val0;
        old_val1 = val1;
        if (t > 100000){t = 0; i = 0; first = 1;}
        last_setingpin = setingpin;
        setingpin = gpio_get(16);
        if (setingpin && last_setingpin == 0){
            status = !status;
            printf("%d %d %d\n", status, setingpin, last_setingpin);
            l18 = (delta == 0 && status);
            l19 = (delta == 1 && status);
            l20 = (delta == 2 && status);
            l21 = (delta == 3 && status);
            gpio_put(18, l18);
            gpio_put(19, l19);
            gpio_put(20, l20);
            gpio_put(21, l21);
            sleep_ms(150);
        }
        gpio_put(17, !status);
        printf("%llu\n", (unsigned long long)(time_us_64() - last_rotary_triger));
    }
}