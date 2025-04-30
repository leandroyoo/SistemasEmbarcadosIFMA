/*
 * Exemplo: dois botões controlam um LED
 * - BOTAO_A acende  (nível baixo → borda de descida)
 * - BOTAO_B apaga   (nível baixo → borda de descida)
 * Agora com debounce via software (25 ms)
 *
 * Hardware: Raspberry Pi Pico W – kit BitDogLab
 * Compilador: arm-none-eabi-gcc -std=c99
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"   // time_us_64()

#define BOTAO_A        5
#define BOTAO_B        6
#define LED            11
#define DEBOUNCE_US    25000    // 25 ms

// Armazena o instante da última interrupção válida de cada botão
static volatile uint64_t ultima_A = 0;
static volatile uint64_t ultima_B = 0;

void isr_botoes(uint gpio, uint32_t events)
{
    uint64_t agora = time_us_64();

    if (gpio == BOTAO_A) {
        if (agora - ultima_A < DEBOUNCE_US) return;   // ignora bouncing
        ultima_A = agora;
        gpio_put(LED, 1);                             // acende
    }
    else if (gpio == BOTAO_B) {
        if (agora - ultima_B < DEBOUNCE_US) return;   // ignora bouncing
        ultima_B = agora;
        gpio_put(LED, 0);                             // apaga
    }
}

int main(void)
{
    stdio_init_all();

    // LED
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 1);        // parte aceso

    // Botão A
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    // Botão B
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    // Mesma rotina para ambos, disparo na borda de descida (premido)
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true,  &isr_botoes);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true,  &isr_botoes);

    while (true) {
        tight_loop_contents();   // aguarda interrupções (consumo mínimo de CPU)
    }
}
