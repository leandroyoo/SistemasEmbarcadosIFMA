#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <stdbool.h>

#define LED_VERMELHO 13
#define LED_VERDE    11
#define BOTAO_PED    5
#define BUZZER_PED   21

#define TEMPO_VERMELHO 10  // segundos
#define TEMPO_VERDE    10
#define TEMPO_AMARELO  3

typedef enum { VERMELHO, VERDE, AMARELO } EstadoSemaforo;
volatile EstadoSemaforo estado = VERMELHO;

alarm_id_t alarm_id = 0;              // Alarme do semáforo
alarm_id_t alarm_cronometro_id = 0;   // Alarme do cronômetro regressivo
volatile int tempo_restante = 0;

// Callback para aviso sonoro intermitente durante o sinal verde
int64_t buzzer_beep_callback(alarm_id_t id, void *user_data) {
    static bool buzzer_state = false;
    static int beep_count = 0;

    if (estado != VERDE || beep_count >= TEMPO_VERDE * 2) {
        gpio_put(BUZZER_PED, 0);  // Desliga buzzer
        beep_count = 0;
        return 0;
    }

    buzzer_state = !buzzer_state;
    gpio_put(BUZZER_PED, buzzer_state);
    beep_count++;

    return 500000; // alterna a cada 0,5s
}

// Função para atualizar os LEDs e buzzer
void set_leds(EstadoSemaforo e) {
    switch (e) {
        case VERMELHO:
            gpio_put(LED_VERDE, 0);
            gpio_put(LED_VERMELHO, 1);
            gpio_put(BUZZER_PED, 0);
            printf("Sinal: Vermelho\n");
            break;
        case VERDE:
            gpio_put(LED_VERMELHO, 0);
            gpio_put(LED_VERDE, 1);
            printf("Sinal: Verde\n");

            // Inicia bipes sonoros
            add_alarm_in_us(0, buzzer_beep_callback, NULL, false);
            break;
        case AMARELO:
            gpio_put(LED_VERDE, 1);
            gpio_put(LED_VERMELHO, 1);
            gpio_put(BUZZER_PED, 0);
            printf("Sinal: Amarelo\n");
            break;
    }
}

// Callback do cronômetro regressivo
int64_t cronometro_callback(alarm_id_t id, void *user_data) {
    if (tempo_restante > 0) {
        printf("Tempo restante: %d segundos\n", tempo_restante);
        tempo_restante--;
        alarm_cronometro_id = add_alarm_in_us(1000000, cronometro_callback, NULL, false);
        return 0;
    } else {
        alarm_cronometro_id = 0;
        return 0;
    }
}

void iniciar_cronometro(int segundos) {
    tempo_restante = segundos;

    if (alarm_cronometro_id != 0) {
        cancel_alarm(alarm_cronometro_id);
        alarm_cronometro_id = 0;
    }

    printf("Tempo restante: %d segundos\n", tempo_restante);
    tempo_restante--;
    alarm_cronometro_id = add_alarm_in_us(1000000, cronometro_callback, NULL, false);
}

// Callback do semáforo
int64_t proximo_estado_callback(alarm_id_t id, void *user_data) {
    if (estado == VERDE) {
        estado = AMARELO;
        set_leds(estado);
        iniciar_cronometro(TEMPO_AMARELO);
        alarm_id = add_alarm_in_us(TEMPO_AMARELO * 1000000, proximo_estado_callback, NULL, false);
        return 0;
    }

    if (estado == AMARELO) {
        estado = VERMELHO;
        set_leds(estado);
        iniciar_cronometro(TEMPO_VERMELHO);
        alarm_id = add_alarm_in_us(TEMPO_VERMELHO * 1000000, proximo_estado_callback, NULL, false);
        return 0;
    }

    if (estado == VERMELHO) {
        estado = VERDE;
        set_leds(estado);
        iniciar_cronometro(TEMPO_VERDE);
        alarm_id = add_alarm_in_us(TEMPO_VERDE * 1000000, proximo_estado_callback, NULL, false);
        return 0;
    }

    return 0;
}

// Interrupção do botão de pedestre
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO_PED && (events & GPIO_IRQ_EDGE_FALL)) {
        if (estado == VERDE) {
            printf("Pedido de travessia detectado durante o sinal verde\n");
            cancel_alarm(alarm_id);
            estado = AMARELO;
            set_leds(estado);
            iniciar_cronometro(TEMPO_AMARELO);
            alarm_id = add_alarm_in_us(TEMPO_AMARELO * 1000000, proximo_estado_callback, NULL, false);
        } else {
            printf("Botão pressionado fora do tempo verde — ignorado\n");
        }
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    gpio_init(BUZZER_PED);
    gpio_set_dir(BUZZER_PED, GPIO_OUT);
    gpio_put(BUZZER_PED, 0);

    gpio_init(BOTAO_PED);
    gpio_set_dir(BOTAO_PED, GPIO_IN);
    gpio_pull_up(BOTAO_PED);
    gpio_set_irq_enabled_with_callback(BOTAO_PED, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    estado = VERMELHO;
    set_leds(estado);
    iniciar_cronometro(TEMPO_VERMELHO);
    alarm_id = add_alarm_in_us(TEMPO_VERMELHO * 1000000, proximo_estado_callback, NULL, false);

    while (true) {
        tight_loop_contents(); // reduz consumo de CPU
    }

    return 0;
}
