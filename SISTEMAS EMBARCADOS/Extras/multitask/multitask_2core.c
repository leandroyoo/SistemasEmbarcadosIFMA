#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#define LED_PIN 12

// Função que será executada no Core 1
void tarefa_core1() {
    while (true) {
        printf("Tarefa no Core 1 ativa\n");
        sleep_ms(1000); // Aguarda 1 segundo
    }
}

int main() {
    stdio_init_all(); // Inicializa saída padrão (para printf)

    // Inicializa LED no GPIO 12 (BitDogLab)
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Lança a tarefa_core1 no segundo núcleo
    multicore_launch_core1(tarefa_core1);

    // Core 0 fica piscando o LED
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        printf("Tarefa no Core 0 ativa\n");
        sleep_ms(1000); // Aguarda 1 segundo
    }
}