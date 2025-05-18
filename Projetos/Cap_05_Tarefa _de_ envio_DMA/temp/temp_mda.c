

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

#define NUM_SAMPLES 100
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

uint16_t adc_buffer[NUM_SAMPLES];

// Converte o valor bruto do ADC (12 bits) para temperatura em °C
float convert_to_celsius(uint16_t raw) {
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = raw * conversion_factor;
    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Inicializa I2C para OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa OLED
    ssd1306_init();

    // Define área de renderização
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    // Inicializa ADC e DMA
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Temperatura interna

    int dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_dreq(&cfg, DREQ_ADC);

    while (true) {
        adc_fifo_drain();
        adc_run(false);
        adc_fifo_setup(true, true, 1, false, false);
        adc_run(true);

        dma_channel_configure(
            dma_chan,
            &cfg,
            adc_buffer,
            &adc_hw->fifo,
            NUM_SAMPLES,
            true
        );
        dma_channel_wait_for_finish_blocking(dma_chan);
        adc_run(false);

        // Calcula média
        float sum = 0.0f;
        for (int i = 0; i < NUM_SAMPLES; i++) {
            sum += convert_to_celsius(adc_buffer[i]);
        }
        float avg_temp = sum / NUM_SAMPLES;

        // Mostra no terminal
        printf("Temperatura média: %.2f °C\n", avg_temp);

        // Prepara texto e exibe no OLED
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Temp: %.2f C", avg_temp);

        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);
        ssd1306_draw_string(ssd, 10, 20, buffer);
        render_on_display(ssd, &frame_area);

        sleep_ms(1000);
    }

    return 0;
}
