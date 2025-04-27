#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#define FLAG_VALUE 0xCAFEBABE     // valor-chave para sincronizar núcleos

/* ---------- Núcleo 1 -------------------------------------------------- */
void core1_entry(void) {
    /* Aguarda sinal de “pronto” vindo do core0 */
    uint32_t sync = multicore_fifo_pop_blocking();
    if (sync == FLAG_VALUE) {
        printf("[CORE1] Sinal recebido, iniciando tarefas…\n");
        multicore_fifo_push_blocking(FLAG_VALUE);   // confirma
    }

    /* Loop principal do core1 */
    while (true) {
        printf("[CORE1] Ainda vivo!\n");
        sleep_ms(1000);             // 1 s para não inundar o terminal
    }
}

/* ---------- Núcleo 0 -------------------------------------------------- */
int main(void) {
    stdio_init_all();               // habilita CDC-ACM (USB) e buffers
    sleep_ms(2000);                 // 2 s: tempo para o PC enumerar a porta

    printf("\n=== Exemplo Multicore + USB CDC ===\n");
    printf("[CORE0] Inicializando…\n");

    multicore_launch_core1(core1_entry);       // dispara o core1

    /* Sincronização simples com FIFO de hardware */
    multicore_fifo_push_blocking(FLAG_VALUE);  // avisa core1
    uint32_t ack = multicore_fifo_pop_blocking();
    if (ack == FLAG_VALUE)
        printf("[CORE0] core1 pronto!\n");
    else
        printf("[CORE0] Falha na sincronização!\n");

    /* Loop principal do core0 */
    uint32_t contador = 0;
    while (true) {
        printf("[CORE0] contador = %lu\n", (unsigned long)contador++);
        sleep_ms(1000);
    }
}