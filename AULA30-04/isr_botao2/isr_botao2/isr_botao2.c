#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"          // <- para irq_set_enabled()
#include "hardware/sync.h"   // para __wfi()

#define BOTAO_A 5
#define BOTAO_B 6
#define LED     11

// ---- Callback único para todos os GPIOs deste core ----
void isr_botoes(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A) {
        gpio_put(LED, 1);          // Acende LED
    } else if (gpio == BOTAO_B) {
        gpio_put(LED, 0);          // Apaga LED
    }
}

int main() {
    stdio_init_all();

    // --- LED como saída ---
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    // --- Botões como entrada (pull-up interno) ---
    gpio_init(BOTAO_A);  gpio_set_dir(BOTAO_A, GPIO_IN);  gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);  gpio_set_dir(BOTAO_B, GPIO_IN);  gpio_pull_up(BOTAO_B);

    // 1) Registra o callback global para GPIO nesse core
    gpio_set_irq_callback(isr_botoes);

    // 2) Habilita o vetor de interrupção de GPIO
    irq_set_enabled(IO_IRQ_BANK0, true);

    // 3) Escolhe quais eventos disparam no pino (borda de descida, ao apertar)
    gpio_set_irq_enabled(BOTAO_A, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);

    gpio_put(LED, 1);

    // --- Loop ocioso: a CPU só acorda nas IRQs ---
    while (true) {
        //tight_loop_contents();
        __wfi(); // núcleo dorme até próxima IRQ

    }
}