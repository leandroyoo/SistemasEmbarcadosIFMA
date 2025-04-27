/*Este experimento demonstra como utilizar mecanismos de exclusão mútua baseados em spinlocks no RP2040 para proteger seções críticas do código em ambientes concorrentes.
Objetivo
Utilizar o recurso de spinlocks do RP2040 para proteger o acesso à variável global 'contador' de forma segura entre interrupções (ISR) e o código principal (loop).
Descrição do Funcionamento
Neste experimento, o contador é incrementado tanto dentro da função principal quanto dentro da ISR acionada por um botão (GPIO 5). 
Para garantir que não ocorram condições de corrida (race conditions), usamos um spinlock exclusivo da arquitetura RP2040: SPINLOCK_ID_USER0.
*/

/*
Inclusão de bibliotecas
O bloco #include puxa três conjuntos de recursos do SDK do Raspberry Pi Pico. 
stdio.h habilita as funções de entrada/saída padrão (por exemplo, printf), permitindo que o microcontrolador escreva mensagens no console USB. 
pico/stdlib.h reúne rotinas de inicialização, temporização e acesso simplificado aos periféricos básicos do RP2040. 
Já hardware/gpio.h fornece as definições de baixo nível para configurar pinos, e hardware/sync.h expõe as primitivas de sincronização por spin lock, 
necessárias quando códigos diferentes – como o loop principal e a rotina de interrupção – acessam a mesma variável crítica. 
Esse conjunto de cabeçalhos evita que você tenha de escrever registrador por registrador, 
deixando o foco na lógica do programa.
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"  // já suficiente

#define BOTAO 5

/*
Constante do pino do botão
#define BOTAO 5 cria um rótulo simbólico para o número físico do GPIO onde o botão está conectado. 
Usar um define facilita a leitura do código e permite trocar o pino em um só lugar se o hardware mudar, sem varrer o programa inteiro. 
Como o pico W utiliza numeração absoluta dos pinos, essa linha liga todas as chamadas subsequentes (como gpio_init ou gpio_get) ao GPIO 5.
*/

volatile int contador = 0;
spin_lock_t *lock;

/*
Variáveis globais e qualificadores
volatile int contador = 0; declara um contador compartilhado entre a ISR (rotina de interrupção) e o programa principal. 
O qualificador volatile avisa ao compilador que o valor de contador pode mudar “fora de vista” (isto é, entre instruções sequenciais), 
impedindo otimizações que fariam cópias em registradores e resultariam em leituras equivocadas. spin_lock_t *lock; reserva um ponteiro para uma estrutura de spin lock – 
uma trava de espera ativa nativa do RP2040 que garante exclusão mútua sem precisar de scheduler ou desativar interrupções globalmente.
*/


void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO && (events & GPIO_IRQ_EDGE_FALL)) {
        uint32_t flags = spin_lock_blocking(lock);
        contador++;
        printf("[ISR]  Contador: %d\n", contador);
        spin_unlock(lock, flags);
    }
}

/*
Rotina de interrupção gpio_callback
Essa função é executada automaticamente quando o botão gera uma borda de descida (GPIO_IRQ_EDGE_FALL). 
A primeira verificação garante que a interrupção veio do pino correto e do evento correto (evitando falsos gatilhos se outros GPIOs estiverem compartilhando a mesma callback). 
Em seguida, uint32_t flags = spin_lock_blocking(lock); adquire a trava e devolve o estado anterior das interrupções; isso impede que a seção crítica sofra preempção ou 
possa colidir com o loop principal. Dentro da região protegida o código incrementa o contador e imprime seu valor com uma etiqueta “[ISR]”, 
ajudando a diferenciar no console quem escreveu a mensagem. 
Por fim, spin_unlock libera a trava e restaura o estado de interrupções usando flags, devolvendo o controle ao fluxo normal.
*/

/*
Inicialização de stdio e atraso inicial
No início de main, stdio_init_all(); configura o canal USB-serial para que printf envie dados ao computador hospedado. 
O sleep_ms(2000); (dois segundos) dá tempo para a porta serial aparecer no host antes que as primeiras mensagens sejam enviadas, 
evitando que o usuário perca os logs iniciais.

Reserva do spin lock
lock = spin_lock_instance(31); pega o spin lock físico de número 31 (há 32 disponíveis, 0–31). 
É comum reservar um desses IDs “de usuário” para proteger recursos próprios da aplicação. 
Caso seu SDK tenha a macro SPINLOCK_ID_USER0 (sinônimo de 31), 
você pode usá-la para tornar o código mais legível; a lógica permanece idêntica.
*/

int main() {
    stdio_init_all();
    sleep_ms(2000);

    lock = spin_lock_instance(31); // ou use SPINLOCK_ID_USER0 se seu SDK permitir

    gpio_init(BOTAO);
    gpio_set_dir(BOTAO, GPIO_IN);
    gpio_pull_up(BOTAO);
    gpio_set_irq_enabled_with_callback(BOTAO, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while (true) {
        uint32_t flags = spin_lock_blocking(lock);
        contador++;
        printf("[MAIN] Contador: %d\n", contador);
        spin_unlock(lock, flags);
        sleep_ms(1000);
    }
}

/*
Configuração do GPIO e habilitação da interrupção
O bloco seguinte prepara o pino do botão: gpio_init ativa o periférico, gpio_set_dir define direção de entrada e gpio_pull_up liga o resistor de pull-up interno – 
dessa forma, o nível lógico fica alto quando o botão está solto e cai para zero quando pressionado. gpio_set_irq_enabled_with_callback(BOTAO, GPIO_IRQ_EDGE_FALL, true, 
&gpio_callback); registra a função de callback, especifica a borda de descida como gatilho e habilita a interrupção. A partir daqui, qualquer pressionar do botão fará o 
hardware desviar para gpio_callback sem intervenção do loop principal.

Laço principal e uso da exclusão mútua
Dentro do while (true) roda a tarefa contínua da aplicação. O código repete o padrão de travar com spin_lock_blocking(lock), incrementar contador, 
imprimir “[MAIN]” e destravar. Proteger a mesma variável em ambos os contextos (ISR e thread principal) evita condições de corrida, garantindo que cada incremento 
seja atômico e que a sequência de números vista no console seja consistente. Depois da seção crítica, sleep_ms(1000); cria um intervalo de um segundo, dando ritmo 
às mensagens do loop principal e diminuindo o uso de CPU. 
Esse fluxo demonstra, de forma concisa, como sincronizar acesso compartilhado entre interrupções e código de aplicativo no RP2040 usando spin locks.
*/
