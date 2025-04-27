#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#define LED_PIN 12

void tarefa_core1() {
    while (true) {
        printf("Tarefa no Core 1 ativa\n");
        sleep_ms(1000);
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Inicia a tarefa no Core 1
    multicore_launch_core1(tarefa_core1);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}