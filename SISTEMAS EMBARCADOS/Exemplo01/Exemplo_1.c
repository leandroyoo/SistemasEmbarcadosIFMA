#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define LED_VERMELHO 13   // Pino para o LED vermelho
#define LED_VERDE 11 // Pino para o LED verde
#define LED_AZUL 12  // Pino para o LED azul

#define BUZZER_PIN 21 // Configuração do pino do buzzer
#define BUZZER_FREQUENCY 2500 // Configuração da frequência do buzzer (em Hz)

#define BOTAO_A 5    // GPIO conectado ao Botão A
#define BOTAO_B 6    // GPIO conectado ao Botão B

#define DELAY_MS 500 // Define um tempo entre cores (em milissegundos)


// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}

void defina_cor(bool r, bool g, bool b) {
//Função definir a cor do LED RGB
//Variáveis r, g, b assume os valores 1 ou 0.
//A combinação destes valores gera uma das oito cores.

//Uma única função para não repetir código.

    gpio_put(LED_VERMELHO, r);
    gpio_put(LED_VERDE, g);
    gpio_put(LED_AZUL, b);
}

void inicializar_pino(uint pino, uint direcao) {
//Função do tipo void para inicializa os pinos
//O número do pino (uint pino)
//A direção (GPIO_IN ou GPIO_OUT).

//Uma única função para não repetir código.

    gpio_init(pino);            // Inicializa o pino
    gpio_set_dir(pino, direcao); // Define como entrada ou saída
}

volatile bool botao_A_ativo = false; //variável bool botão ativo A, com modificador de tipo volatile.
volatile bool botao_B_ativo = false; //variável bool botão ativo A, com modificador de tipo volatile.

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A && (events & GPIO_IRQ_EDGE_FALL)) {
        botao_A_ativo= true;
    }
    if (gpio == BOTAO_B && (events & GPIO_IRQ_EDGE_FALL)) {
        botao_B_ativo= true;
    }
}

int main()
{
    stdio_init_all();

    // Inicializa LEDs como saída
    inicializar_pino(LED_VERMELHO, GPIO_OUT);
    inicializar_pino(LED_VERDE, GPIO_OUT);
    inicializar_pino(LED_AZUL, GPIO_OUT);

    // Inicializar Botão A e B
    // Inicializa botão como entrada
    inicializar_pino(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A); // Ativa pull-up interno.

    inicializar_pino(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B); // Ativa pull-down interno.

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

     // Inicializar o PWM no pino do buzzer
     pwm_init_buzzer(BUZZER_PIN);

    while (true) {
        if (!gpio_get(BOTAO_A) && gpio_get(BOTAO_B)) {
            defina_cor(0, 0, 1); // Azul
            sleep_ms(DELAY_MS);
            defina_cor(0,0,0);
            sleep_ms(DELAY_MS);
            botao_A_ativo= false;
        } else if (!gpio_get(BOTAO_B) && gpio_get(BOTAO_A)) {
            defina_cor(0, 1, 0); // Verde
            sleep_ms(DELAY_MS);
            defina_cor(0,0,0);
            sleep_ms(DELAY_MS);
            botao_B_ativo= false;
        } else if (!gpio_get(BOTAO_B) && !gpio_get(BOTAO_A)) {
            
            defina_cor(1, 0, 0); // Vermelho
            beep(BUZZER_PIN, 1000); // Bipe de 500ms
            defina_cor(0,0,0);
            sleep_ms(DELAY_MS); 
            botao_A_ativo= false;
            botao_B_ativo= false;
        }
       
        
        else {
            defina_cor(1, 1, 1); // branco
            sleep_ms(DELAY_MS);
            defina_cor(0, 0, 0); // Apagado
            sleep_ms(DELAY_MS);
        }
    }
    return 0;
}