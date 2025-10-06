#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include <stdio.h>

#define ADC_PIN 26
#define ADC_CHANNEL 0
#define BUFFER_SIZE 16

uint16_t adc_buffer[BUFFER_SIZE];
int dma_chan;

void dma_handler() {
    // Acknowledge interrupt
    dma_hw->ints0 = 1u << dma_chan;

    // Gemiddelde spanning berekenen
    uint32_t sum = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) sum += adc_buffer[i];
    float avg = (float)sum / BUFFER_SIZE;
    float voltage = (avg * 3.3f) / (1 << 12);
    printf("Gemiddelde spanning: %.2f V\n", avg);

    // DMA opnieuw instellen (volledige restart)
    dma_channel_set_read_addr(dma_chan, &adc_hw->fifo, false);
    dma_channel_set_write_addr(dma_chan, adc_buffer, false);
    dma_channel_set_trans_count(dma_chan, BUFFER_SIZE, true);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Start ADC DMA test (met restart)\n");

    // === ADC instellen ===
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);
    adc_fifo_setup(true, true, 1, false, false);
    adc_set_clkdiv(48000.0f); // ~1kHz
    adc_run(true);

    // === DMA instellen ===
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, DREQ_ADC);

    dma_channel_configure(
        dma_chan, &c,
        adc_buffer,
        &adc_hw->fifo,
        BUFFER_SIZE,
        true
    );

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    while (true) {
        tight_loop_contents();
    }
}
