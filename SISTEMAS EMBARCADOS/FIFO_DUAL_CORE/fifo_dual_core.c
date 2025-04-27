/* fifo_dual_core.c
 *
 * Demonstração simples de troca de mensagens pela FIFO do RP2040.
 *
 * • Núcleo 0 vigia o BOTAO_A (GPIO 5). Ao detectar borda de descida,
 *   envia a palavra MSG_BOTAO_A para a FIFO.  
 *   Núcleo 1 lê essa palavra e imprime: “Botão A pressionado”.
 *
 * • Núcleo 1 vigia o BOTAO_B (GPIO 6). Ao detectar borda de descida,
 *   envia MSG_BOTAO_B para a FIFO.  
 *   Núcleo 0 lê e imprime: “Botão B pressionado”.
 *
 * Estrutura de interrupções, macros de pinos e função
 * `inicializar_pino()` vêm do seu arquivo **minhas_funcoes.c**. :contentReference[oaicite:0]{index=0}&#8203;:contentReference[oaicite:1]{index=1}
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
//#include "minhas_funcoes.c"          // BOTAO_A, BOTAO_B, inicializar_pino()

/* ――― Mensagens trocadas pela FIFO ――― */
#define MSG_BOTAO_A  0x0A01          // qualquer valor de 32 bits serve
#define MSG_BOTAO_B  0x0B02

#define BOTAO_A 5    // GPIO conectado ao Botão A
#define BOTAO_B 6    // GPIO conectado ao Botão B

/* ───────────────────────── Núcleo 0 ───────────────────────── */

/* ISR executa-se no núcleo que registrou o callback */
static void gpio_callback_core0(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_A && (events & GPIO_IRQ_EDGE_FALL))
        multicore_fifo_push_blocking(MSG_BOTAO_A);   // avisa core 1
}

static void core1_entry(void);   /* protótipo da função do core 1 */

void inicializar_pino(uint pino, uint direcao) {
    //Função do tipo void para inicializa os pinos
    //O número do pino (uint pino)
    //A direção (GPIO_IN ou GPIO_OUT).
    
    //Uma única função para não repetir código.
    
        gpio_init(pino);            // Inicializa o pino
        gpio_set_dir(pino, direcao); // Define como entrada ou saída
    }

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);                          // tempo p/ USB enumerar

    /* --- Inicialização dos botões --- */
    inicializar_pino(BOTAO_A, GPIO_IN);  gpio_pull_up(BOTAO_A);
    inicializar_pino(BOTAO_B, GPIO_IN);  gpio_pull_up(BOTAO_B);

    /* Callback deste núcleo → só botão A */
    gpio_set_irq_enabled_with_callback(BOTAO_A,GPIO_IRQ_EDGE_FALL, true,  &gpio_callback_core0);

    /* Lança o segundo núcleo */
    multicore_launch_core1(core1_entry);

    /* Loop principal: espera mensagens do core 1 */
    while (true)
    {
        if (multicore_fifo_rvalid())
        {
            uint32_t msg = multicore_fifo_pop_blocking();
            if (msg == MSG_BOTAO_B)
                printf("[CORE 0] Botão B pressionado\n");
        }
        tight_loop_contents();         // economia de energia / latência
    }
}

/* ───────────────────────── Núcleo 1 ───────────────────────── */

static void gpio_callback_core1(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_B && (events & GPIO_IRQ_EDGE_FALL))
        multicore_fifo_push_blocking(MSG_BOTAO_B);   // avisa core 0
}

static void core1_entry(void)
{
    /* Cada núcleo precisa configurar o seu botão e callback */
    inicializar_pino(BOTAO_B, GPIO_IN);  gpio_pull_up(BOTAO_B);

    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback_core1);

    /* Loop principal: espera mensagens do core 0 */
    while (true)
    {
        if (multicore_fifo_rvalid())
        {
            uint32_t msg = multicore_fifo_pop_blocking();
            if (msg == MSG_BOTAO_A)
                printf("[CORE 1] Botão A pressionado\n");
        }
        tight_loop_contents();
    }
}

