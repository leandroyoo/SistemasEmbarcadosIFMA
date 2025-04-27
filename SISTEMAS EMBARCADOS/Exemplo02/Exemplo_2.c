/*
O que é um temporizador cooperativo baseado em função-de-retorno
Nos microcontroladores da família RP2040 (caso do Raspberry Pi Pico W), 
você possui um bloco de temporizadores de hardware capaz de gerar alarmes (eventos pontuais ou periódicos).

Temporizador cooperativo: em vez de atender o alarme dentro da própria interrupção (estilo “pré-emptivo”), 
a rotina de alarme apenas sinaliza que chegou a hora de fazer algo. A tarefa real é executada depois, no laço principal (while (true)), 
quando o programa “coopera”. Isso mantém o código simples para iniciantes – sem concorrência complicada ou prioridades.

Função-de-retorno (callback): é a função cadastrada junto ao temporizador. 
O RP2040 a chama automaticamente cada vez que o alarme dispara. Dentro dela, colocamos só o mínimo necessário (normalmente, 
setar uma variável volatile) e pedimos ao temporizador que volte a disparar depois de N milissegundos.
*/

/*
Este exemplo mostra o uso de um temporizador cooperativo baseado em callback, utilizando a função add_alarm_in_ms(). 
A versão foi corrigida para garantir que o LED pisque de forma visível ao olho humano com um pequeno atraso após a mudança de estado.
Objetivo
- Alternar o estado de um LED (GPIO 12) a cada 1 segundo
- Usar um callback temporizado com controle de estado
- Tornar a transição visual perceptível com sleep_ms(100)
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define LED_PIN 12

volatile bool alternar_led = false;
bool estado_led = false;


int64_t temporizador_callback(alarm_id_t id, void *user_data) {
    alternar_led = true;
    return 1000;
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    /*
    No trecho apresentado, o temporizador cooperativo serve para gerar, a cada 1 000 ms (1 s), 
    um sinal que avisa ao programa principal que chegou a hora de executar uma ação—inverter o estado do LED.
    
    O RP2040 agenda um alarme para daqui a 1 000 ms.
    
    Quando esse alarme dispara, o próprio hardware chama a função‐de‐retorno temporizador_callback.
    
    O quarto argumento (true) faz com que o alarme seja automático: depois que o callback termina, 
    o temporizador já fica programado para disparar novamente 1 000 ms depois.
    */

    add_alarm_in_ms(1000, temporizador_callback, NULL, true);

    while (true) {
        if (alternar_led) {
            estado_led = !estado_led;
            gpio_put(LED_PIN, estado_led);
            printf("LED = %d\n", estado_led);
            sleep_ms(100);
            alternar_led = false;
        }
/*
tight_loop_contents() é um “marcador” que garante a sanidade e portabilidade de laços apertados: impede otimizações indesejadas, 
força nova leitura da memória e serve como gancho leve para instrumentação ou economia de energia.
*/
        tight_loop_contents();
    }
}

/*
- O uso de gpio_put com sleep_ms(100) garante que a mudança de estado do LED seja visível.
- Esse comportamento melhora a percepção do funcionamento correto do sistema embarcado.
- tight_loop_contents() mantém o sistema responsivo sem bloquear o loop principal.
*/