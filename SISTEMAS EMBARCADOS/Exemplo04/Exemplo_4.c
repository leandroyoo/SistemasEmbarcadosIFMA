/*
Simulação de Fila Circular (fila_circular_produtor_consumidor)
*/

/*
Descrição
Este experimento simula uma fila circular simples em linguagem C para representar o padrão produtor-consumidor sem a utilização de RTOS. A aplicação funciona de forma cooperativa, alternando entre uma função produtora e outra consumidora dentro do laço principal. A fila tem tamanho fixo e comportamento de sobreposição circular.
Objetivo
- Implementar uma fila circular com buffer fixo
- Simular a tarefa de produzir e inserir elementos na fila
- Simular a tarefa de consumir e remover elementos da fila
- Exibir toda a operação no terminal via USB
*/

/*
Observações Práticas
- O experimento demonstra de forma cooperativa o funcionamento de duas tarefas compartilhando um recurso (fila).
- A estrutura circular evita sobreposição de memória e permite reaproveitamento de espaço.
- Pode ser expandido com múltiplos produtores ou consumidores reais em sistemas multitarefa com FreeRTOS ou multicore.
*/

#include <stdio.h>
#include "pico/stdlib.h"

#define TAM_FILA 8

int fila[TAM_FILA];
int inicio = 0;
int fim = 0;
int quantidade = 0;

bool inserir(int valor) {
    if (quantidade == TAM_FILA) return false;
    fila[fim] = valor;
    fim = (fim + 1) % TAM_FILA;
    quantidade++;
    return true;
}

bool remover(int *valor) {
    if (quantidade == 0) return false;
    *valor = fila[inicio];
    inicio = (inicio + 1) % TAM_FILA;
    quantidade--;
    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    int contador = 0;
    while (true) {
        if (inserir(contador)) {
            printf("Produzido: %d\n", contador);
            contador++;
        } else {
            printf("Fila cheia! Não foi possível produzir.\n");
        }
        sleep_ms(500);

        int valor;
        if (remover(&valor)) {
            printf("Consumido: %d\n", valor);
        } else {
            printf("Fila vazia! Nada para consumir.\n");
        }
        sleep_ms(500);
    }
}

