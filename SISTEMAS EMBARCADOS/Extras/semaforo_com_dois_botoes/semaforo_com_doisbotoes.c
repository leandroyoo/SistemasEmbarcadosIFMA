/*
 * semaforo_multicore_fifo.c
 * — Semáforo acessível por dois botões, um núcleo por botão,
 *    com coordenação via multicore FIFO no RP2040.
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "minhas_funcoes.c"      // Funções e definições auxiliares :contentReference[oaicite:0]{index=0}&#8203;:contentReference[oaicite:1]{index=1}

/*------------------------ CONSTANTES PARA A FIFO -----------------------*/
#define TOKEN_LOCK   0xA5   // avisa “estou usando o semáforo”
#define TOKEN_FREE   0x5A   // avisa “terminei, semáforo liberado”

/*-------------------- PROTÓTIPOS --------------------*/
static void run_semaforo(void);
static void core1_entry(void);

/* ------------------------------------------------------------------------- */
/*  CICLO COMPLETO DO SEMÁFORO (LED + BUZZER)                                */
/* ------------------------------------------------------------------------- */
static void run_semaforo(void)
{
    /* 1) Espera 1 s em vermelho (tom contínuo) */
    pwm_init_buzzer(BUZZER_PIN, FREQ_VERMELHO);
    defina_cor(1,0,0);
    beep(BUZZER_PIN, TEMPO_VERMELHO);      // tom alto contínuo

    /* 2) Verde — 3 s (tom alternado) */
    pwm_init_buzzer(BUZZER_PIN, FREQ_VERDE);
    defina_cor(0,1,0);
    beep(BUZZER_PIN, TEMPO_VERDE);

    /* 3) Amarelo — 1,5 s (tom rápido) */
    pwm_init_buzzer(BUZZER_PIN, FREQ_AMARELO);
    defina_cor(1,1,0);
    beep(BUZZER_PIN, TEMPO_AMARELO);

    /* 4) Retorna ao vermelho (silencioso)            */
    defina_cor(1,0,0);
    pwm_set_gpio_level(BUZZER_PIN, 0);      // buzzer off
}

/* ------------------------------------------------------------------------- */
/*  MAIN – CORE 0  (Botão A)                                                  */
/* ------------------------------------------------------------------------- */
int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    /* Inicialização de pinos em ambos os núcleos */
    inicializar_pino(LED_VERMELHO, GPIO_OUT);
    inicializar_pino(LED_VERDE,   GPIO_OUT);
    inicializar_pino(LED_AZUL,    GPIO_OUT);
    inicializar_pino(BUZZER_PIN,  GPIO_OUT);

    inicializar_pino(BOTAO_A, GPIO_IN);  gpio_pull_up(BOTAO_A);
    inicializar_pino(BOTAO_B, GPIO_IN);  gpio_pull_up(BOTAO_B);

    defina_cor(1,0,0);   // vermelho inicial

    /* Lança o núcleo 1 */
    multicore_launch_core1(core1_entry);

    /* Limpa possíveis resíduos na FIFO */
    while (multicore_fifo_rvalid()) multicore_fifo_pop_blocking();

    bool ocupado_local  = false;  // core 0 executando
    bool ocupado_remoto = false;  // core 1 executando

    while (true)
    {
        /* ----- Lê mensagens da FIFO ----- */
        if (multicore_fifo_rvalid())
        {
            uint32_t token = multicore_fifo_pop_blocking();
            if      (token == TOKEN_LOCK)   ocupado_remoto = true;
            else if (token == TOKEN_FREE)   ocupado_remoto = false;
        }

        /* ----- Testa Botão A (nível baixo = pressionado) ----- */
        if (!ocupado_local && !ocupado_remoto && !gpio_get(BOTAO_A))
        {
            ocupado_local = true;
            multicore_fifo_push_blocking(TOKEN_LOCK);   // avisa outro núcleo
            run_semaforo();
            multicore_fifo_push_blocking(TOKEN_FREE);   // libera outro núcleo
            ocupado_local = false;
        }

        tight_loop_contents();   // baixa latência sem ocupar CPU
    }
}

/* ------------------------------------------------------------------------- */
/*  CORE 1  – Botão B                                                         */
/* ------------------------------------------------------------------------- */
static void core1_entry(void)
{
    /* Repetimos a inicialização mínima necessária no core 1 */
    inicializar_pino(BOTAO_B, GPIO_IN);  gpio_pull_up(BOTAO_B);

    bool ocupado_local  = false;
    bool ocupado_remoto = false;

    while (true)
    {
        /* Recebe mensagens do core 0 */
        if (multicore_fifo_rvalid())
        {
            uint32_t token = multicore_fifo_pop_blocking();
            if      (token == TOKEN_LOCK)   ocupado_remoto = true;
            else if (token == TOKEN_FREE)   ocupado_remoto = false;
        }

        /* Botão B — ativo em nível baixo */
        if (!ocupado_local && !ocupado_remoto && !gpio_get(BOTAO_B))
        {
            ocupado_local = true;
            multicore_fifo_push_blocking(TOKEN_LOCK);
            run_semaforo();
            multicore_fifo_push_blocking(TOKEN_FREE);
            ocupado_local = false;
        }

        tight_loop_contents();
    }
}
