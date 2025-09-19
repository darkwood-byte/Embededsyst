#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdio.h>

int main() {
    stdio_init_all();

    // Init ADC
    adc_init();
    adc_gpio_init(26);        // GPIO26 = pin 31 = ADC0
    adc_select_input(0);      // Kanaal 0 (GPIO26)

    while (true) {
        uint16_t raw = adc_read();   // 12-bit waarde (0–4095)
        printf("ADC raw: %u\n", raw);
        sleep_ms(500);
    }
}
