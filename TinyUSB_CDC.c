#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"

// Definição dos LEDs (BitDogLab)
#define LED_VERDE     11
#define LED_AZUL      12
#define LED_VERMELHO  13
#define BUZZER        10  // GPIO do Buzzer

// Inicializa os LEDs e o Buzzer
void leds_buzzer_init() {
    // Configura LEDs
    gpio_init(LED_VERDE);
    gpio_init(LED_AZUL);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERDE, 0);
    gpio_put(LED_AZUL, 0);
    gpio_put(LED_VERMELHO, 0);

    // Configura Buzzer
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);
}

// Aciona o Buzzer por um tempo (ms)
void acionar_buzzer(int tempo_ms) {
    gpio_put(BUZZER, 1);  // Liga o Buzzer
    sleep_ms(tempo_ms);
    gpio_put(BUZZER, 0);  // Desliga o Buzzer
}

// Controla os LEDs (verde, azul, vermelho) por um tempo específico
void acender_leds(int verde, int azul, int vermelho, int tempo_ms) {
    gpio_put(LED_VERDE, verde);
    gpio_put(LED_AZUL, azul);
    gpio_put(LED_VERMELHO, vermelho);
    if (tempo_ms > 0) {
        sleep_ms(tempo_ms);
        gpio_put(LED_VERDE, 0);
        gpio_put(LED_AZUL, 0);
        gpio_put(LED_VERMELHO, 0);
    }
}

int main() {
    stdio_init_all();
    leds_buzzer_init();  // Inicializa LEDs e Buzzer

    while (!tud_cdc_connected()) {
        sleep_ms(100);
    }
    printf("USB conectado!\n");

    while (true) {
        if (tud_cdc_available()) {
            uint8_t buf[64];
            uint32_t count = tud_cdc_read(buf, sizeof(buf));
            
            // Remove \r e \n do final do buffer
            while (count > 0 && (buf[count - 1] == '\n' || buf[count - 1] == '\r')) {
                count--;
            }
            buf[count] = '\0';  // Termina a string

            // Compara o texto recebido
            if (strcmp((char*)buf, "vermelho") == 0) {
                printf("Recebido: vermelho\n");
                tud_cdc_write_str("vermelho\n");
                acender_leds(0, 0, 1, 1000);
            }
            else if (strcmp((char*)buf, "verde") == 0) {
                printf("Recebido: verde\n");
                tud_cdc_write_str("verde\n");
                acender_leds(1, 0, 0, 1000);
            }
            else if (strcmp((char*)buf, "azul") == 0) {
                printf("Recebido: azul\n");
                tud_cdc_write_str("azul\n");
                acender_leds(0, 1, 0, 1000);
            }
            else if (strcmp((char*)buf, "amarelo") == 0) {
                printf("Recebido: amarelo\n");
                tud_cdc_write_str("amarelo\n");
                acender_leds(1, 0, 1, 1000);
            }
            else if (strcmp((char*)buf, "roxo") == 0) {
                printf("Recebido: roxo\n");
                tud_cdc_write_str("roxo\n");
                acender_leds(0, 1, 1, 1000);
            }
            else if (strcmp((char*)buf, "ciano") == 0) {
                printf("Recebido: ciano\n");
                tud_cdc_write_str("ciano\n");
                acender_leds(1, 1, 0, 1000);
            }
            else if (strcmp((char*)buf, "apaga") == 0) {
                printf("Recebido: apaga\n");
                tud_cdc_write_str("apaga\n");
                acender_leds(0, 0, 0, 0);
            }
            else if (strcmp((char*)buf, "som") == 0) {
                printf("Recebido: som\n");
                tud_cdc_write_str("som\n");
                acionar_buzzer(1000);  // Buzzer por 1 segundo
            }
            else {
                printf("Comando desconhecido: %s\n", buf);
                tud_cdc_write_str("Comando inválido. Use: vermelho, verde, azul, amarelo, roxo, ciano, apaga, som\n");
            }
            tud_cdc_write_flush();
        }
        tud_task();
    }
    return 0;
}