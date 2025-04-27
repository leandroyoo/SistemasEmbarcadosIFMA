/*
Controle de Tempo com Botão (led_dinamico_btn)
*/

/*
Descrição
Este exemplo amplia o modelo cooperativo com callback, permitindo alterar o tempo de piscar do LED usando um botão. A frequência de alternância é controlada dinamicamente e o tempo de visibilidade do LED é proporcional ao novo período.
Objetivo
- Utilizar add_alarm_in_ms() com callback periódico
- Alternar entre períodos de 1000 ms e 500 ms ao pressionar um botão
- Tornar o piscar do LED visível ao ajustar o tempo de acionamento
*/

/*
Observações Práticas
- O botão altera entre dois períodos de temporização (500 ms e 1000 ms).
- O tempo de visibilidade do LED é proporcional ao novo período.
- A borda de descida é usada para evitar múltiplas trocas por ruído.
- O terminal exibe a frequência e o estado do LED em tempo real.
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define LED_PIN     12
#define BUTTON_PIN  5

volatile bool alternar_led = false;
bool estado_led = false;
uint32_t periodo_ms = 1000;
bool botao_anterior = true;

int64_t temporizador_callback(alarm_id_t id, void *user_data) {
    alternar_led = true;
    return periodo_ms;
}

/*
O programa configura o Raspberry Pi Pico W para piscar o LED ligado ao pino 12 usando um temporizador de hardware que dispara a cada período variável (1 s ou 0,5 s). 
Esse temporizador, definido por add_alarm_in_ms, executa um callback rápido que apenas levanta a bandeira alternar_led; o laço principal percebe essa bandeira, 
inverte o estado do LED, imprime o novo valor e faz uma breve pausa para que o piscar seja perceptível. Ao mesmo tempo, o pino 5 é lido como botão com pull-up interno: 
cada vez que detecta a transição de pressionado (alta → baixa), o programa alterna o período do temporizador entre 1000 ms e 500 ms e informa a mudança pelo console USB. 
Assim, com código simples e não-bloqueante, o usuário pode alterar em tempo real a frequência do piscar apenas apertando o botão, 
ilustrando o uso integrado de GPIO, temporizador periódico e comunicação USB para depuração.
*/

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    add_alarm_in_ms(periodo_ms, temporizador_callback, NULL, true);

    while (true) {
        bool botao_atual = gpio_get(BUTTON_PIN);

        if (botao_anterior && !botao_atual) {
            periodo_ms = (periodo_ms == 1000) ? 500 : 1000;
            printf("Novo período: %u ms\n", periodo_ms);
        }
        botao_anterior = botao_atual;

        if (alternar_led) {
            estado_led = !estado_led;
            gpio_put(LED_PIN, estado_led);
            printf("LED = %d\n", estado_led);
            sleep_ms(periodo_ms / 4);
            alternar_led = false;
        }

        tight_loop_contents();
    }
}

/*
Por que essa estrutura é pedagógica?
Separa responsabilidades

Callback muito curto (só sinaliza) → ISR rápido, sem prints.

Mostra uso de volatile para comunicação entre interrupção e laço principal.

Exibe o mecanismo de alarme periódico do RP2040 sem precisar de RTOS.

Demonstra tratamento de botão com pull-up interno e detecção de borda em software.

Permite alteração de parâmetro em tempo de execução (troca de período) mostrando integração entre lógica de aplicação e serviço de temporização.
*/