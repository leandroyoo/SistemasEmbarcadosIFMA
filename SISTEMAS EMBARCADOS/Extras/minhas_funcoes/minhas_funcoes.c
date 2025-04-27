#define LED_VERMELHO 13   // Pino para o LED vermelho
#define LED_VERDE 11 // Pino para o LED verde
#define LED_AZUL 12  // Pino para o LED azul

#define BUZZER_PIN 21 // Configuração do pino do buzzer
#define BUZZER_FREQUENCY 2500 // Configuração da frequência do buzzer (em Hz)

#define FREQ_VERMELHO 4000
#define FREQ_VERDE    1000
#define FREQ_AMARELO  2500

#define BOTAO_A 5    // GPIO conectado ao Botão A
#define BOTAO_B 6    // GPIO conectado ao Botão B

#define DELAY_MS 500 // Define um tempo entre cores (em milissegundos)

// Tempos (ms)
#define TEMPO_VERDE    3000  // 30 s
#define TEMPO_AMARELO  1500  // 15 s
#define TEMPO_VERMELHO 100  // 1 s


// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin, uint buzzer_frequencia) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (buzzer_frequencia * 4096)); // Divisor de clock
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
