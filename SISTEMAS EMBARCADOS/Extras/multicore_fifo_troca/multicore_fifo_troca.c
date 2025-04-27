#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

void core1_main() {
    while (true) {
        uint32_t recebido = multicore_fifo_pop_blocking();
        printf("Core 1 recebeu: %u\n", recebido);
        multicore_fifo_push_blocking(recebido + 1);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Muito importante!

    multicore_launch_core1(core1_main);

    for (uint32_t i = 0; i < 10; i++) {
        multicore_fifo_push_blocking(i);
        printf("Core 0 enviou: %u\n", i);
        uint32_t resposta = multicore_fifo_pop_blocking();
        printf("Core 0 recebeu resposta: %u\n", resposta);
        sleep_ms(1000);
    }

    return 0;
}