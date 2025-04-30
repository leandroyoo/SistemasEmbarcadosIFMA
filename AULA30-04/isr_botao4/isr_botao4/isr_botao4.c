/*
 * Contador de eventos com “filtro de estado”
 *  – Só soma se houver mudança REAL de estado no LED
 *  – C99 / Raspberry Pi Pico W (BitDogLab)
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BOTAO_A 5
#define BOTAO_B 6
#define LED     11

/* ---------- Variáveis globais compartilhadas ---------- */
volatile uint32_t conta_on  = 0;   // vezes que o LED passou de OFF→ON
volatile uint32_t conta_off = 0;   // vezes que o LED passou de ON →OFF
volatile bool     estado_led = true;   // LED começa aceso (1)
volatile bool     novo_evento = false; // sinal para imprimir

/* ---------- ISR dos botões ---------- */
void isr_botoes(uint gpio, uint32_t events)
{
    /* LED estava APAGADO e pressionou A → liga */
    if (gpio == BOTAO_A && !estado_led) {
        gpio_put(LED, 1);
        estado_led = true;         // atualizado *antes* de sinalizar
        ++conta_on;
        novo_evento = true;

    /* LED estava ACESO e pressionou B → desliga */
    } else if (gpio == BOTAO_B && estado_led) {
        gpio_put(LED, 0);
        estado_led = false;
        ++conta_off;
        novo_evento = true;
    }
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);                // espera USB enumerar

    /* LED */
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 1);              // inicial: aceso

    /* Botões */
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    /* Interrupções nas bordas de descida */
    gpio_set_irq_enabled_with_callback(BOTAO_A,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &isr_botoes);
    gpio_set_irq_enabled_with_callback(BOTAO_B,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &isr_botoes);

    printf("Filtro ativo: só conta transições reais de estado.\n");

    /* ---------- Loop principal ---------- */
    while (true) {
        if (novo_evento) {
            uint32_t on  = conta_on;
            uint32_t off = conta_off;
            novo_evento  = false;

            printf("Transições: ON=%u  OFF=%u  | LED=%s\n",
                   on, off, estado_led ? "Aceso" : "Apagado");
        }

        tight_loop_contents();     // dorme até a próxima IRQ
    }
}
