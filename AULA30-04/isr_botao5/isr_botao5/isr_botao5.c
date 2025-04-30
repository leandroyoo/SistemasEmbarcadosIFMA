#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

void set_rgb(char cor);
void isr_botoes(uint gpio, uint32_t events);


// Pinos
#define BOTAO_A 5
#define BOTAO_B 6
#define LED_R   13
#define LED_G   11
#define LED_B   12

char apaga_led = 'd';
char acende_led = 'r';

// Interrupção dos botões
void isr_botoes(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A) {
        set_rgb(acende_led);
    } else if (gpio == BOTAO_B) {
        set_rgb(apaga_led);
    }
}

// Controla o LED RGB
void set_rgb(char cor) {
    switch (cor) {
        case 'r':
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 0);
            gpio_put(LED_B, 0);
            break;
        case 'g':
            gpio_put(LED_R, 0);
            gpio_put(LED_G, 1);
            gpio_put(LED_B, 0);
            break;
        case 'b':
            gpio_put(LED_R, 0);
            gpio_put(LED_G, 0);
            gpio_put(LED_B, 1);
            break;
        default:
            gpio_put(LED_R, 0);
            gpio_put(LED_G, 0);
            gpio_put(LED_B, 0);
    }
}

// Código do núcleo 1
void core1_entry() {
    while (true) {
        int ch = getchar();
        if (ch == 'r' || ch == 'g' || ch == 'b') {
            multicore_fifo_push_blocking((uint32_t)ch);
            printf("Core 1: comando enviado [%c] para mudar cor do LED RGB\n", ch);
        } else {
            printf("Core 1: caractere inválido [%c], use apenas r, g ou b\n", ch);
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Iniciar núcleo 1
    multicore_launch_core1(core1_entry);

    // Inicializar LEDs
    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G); gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B); gpio_set_dir(LED_B, GPIO_OUT);

    // Inicializar botões
    gpio_init(BOTAO_A); gpio_set_dir(BOTAO_A, GPIO_IN); gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B); gpio_set_dir(BOTAO_B, GPIO_IN); gpio_pull_up(BOTAO_B);

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &isr_botoes);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);

    // Loop do núcleo 0
    while (true) {
        if (multicore_fifo_rvalid()) {
            char comando = (char)multicore_fifo_pop_blocking();
            //set_rgb(comando);
            acende_led = comando;
        }
        tight_loop_contents();  // Espera passiva
    }
}
