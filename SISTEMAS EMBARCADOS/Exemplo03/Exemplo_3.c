/*
Comunicação entre Interrupção e Laço Principal com Variável Global (variavel_global_entre_ISR_loop)
*/

/*
Descrição
Este experimento demonstra o uso de uma variável global como meio de comunicação entre uma rotina de interrupção (ISR) e o laço principal do programa. Um botão conectado ao GPIO 5 aciona uma interrupção de borda de descida, que define uma flag. O laço principal detecta essa flag e executa uma ação.
Objetivo
- Acionar uma interrupção por botão
- Modificar uma variável global na ISR
- Detectar e reagir a essa modificação no laço principal
- Usar LED no GPIO 12 como resposta ao evento
*/

/*
Observações Práticas
- Esse modelo representa um exemplo de acoplamento forte entre interrupção e laço principal.
- A variável global deve ser declarada como 'volatile' para evitar otimizações indesejadas.
- É uma técnica útil, mas pode gerar condições de corrida se o acesso à variável não for controlado.
- Ideal para eventos simples e não críticos.
*/

#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN    12
#define BUTTON_PIN 5

volatile bool flag_evento = false;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        flag_evento = true;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while (true) {
        if (flag_evento) {
            printf("Interrupção detectada! Acionando LED...\n");
            gpio_put(LED_PIN, 1);
            sleep_ms(250);
            gpio_put(LED_PIN, 0);
            flag_evento = false;
        }
        tight_loop_contents();
    }
}
/*
Sobre tight_loop_contents()
Essa chamada (macro) informa ao compilador e à biblioteca que este é um laço apertado (“busy-wait”).
No SDK do Pico, ela pode:

Inserir instrução de espera (__wfe) em builds com multicore ou freertos, reduzindo consumo.

Manter compatibilidade com futuras otimizações.

Não altera a lógica; é apenas uma boa prática para loops intermináveis.
*/
