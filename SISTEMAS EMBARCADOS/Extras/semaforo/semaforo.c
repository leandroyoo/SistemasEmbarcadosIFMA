#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "minhas_funcoes.c"

volatile bool botao_A_ativo = false; //variável bool botão ativo A, com modificador de tipo volatile.

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A && (events & GPIO_IRQ_EDGE_FALL)) {
        botao_A_ativo= true;
    }
}

int main()
{
    stdio_init_all();

    // Inicializa LEDs como saída
    inicializar_pino(LED_VERMELHO, GPIO_OUT);
    inicializar_pino(LED_VERDE, GPIO_OUT);
    inicializar_pino(LED_AZUL, GPIO_OUT);

    // Inicializar Botão A e B
    // Inicializa botão como entrada
    inicializar_pino(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A); // Ativa pull-up interno.


    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while (true) {
        // Inicializar o PWM no pino do buzzer
            pwm_init_buzzer(BUZZER_PIN,FREQ_VERMELHO);
            defina_cor(1, 0, 0); // Vermelho
            beep(BUZZER_PIN, TEMPO_VERMELHO); // 

            if (!gpio_get(BOTAO_A)){                    
            pwm_init_buzzer(BUZZER_PIN,FREQ_VERDE);
            defina_cor(0, 1, 0); // Vermelho
            beep(BUZZER_PIN, TEMPO_VERDE); //

            pwm_init_buzzer(BUZZER_PIN,FREQ_AMARELO);
            defina_cor(1, 1, 0); // Vermelho
            beep(BUZZER_PIN, TEMPO_AMARELO); //

            botao_A_ativo = false;
            }

            tight_loop_contents();
        }

}