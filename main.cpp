#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include "personagem.h"
#include <unistd.h>
#include <algorithm>
#include <random>

std::vector<int> filaForno;
pthread_cond_t sinal;
pthread_mutex_t forno;
pthread_mutex_t fila;

float get_random() {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0,1);
    return dis(e);
}

void ImprimirFila() {
    printf("Fila: ");
    for(auto it = filaForno.begin(); it != filaForno.end(); it++) {
        printf("%d ", *it);
    }
    printf("\n");
}

int Prioridade(int p1, int p2) {
    int max = std::max(p1, p2);
    int min = std::min(p1, p2);
    if(max == 3 && min == 1) return min;
    else return max;
}

bool TemDeadlock() {
    bool p1 = (std::find(filaForno.begin(), filaForno.end(), 1) != filaForno.end());
    bool p2 = (std::find(filaForno.begin(), filaForno.end(), 2) != filaForno.end());
    bool p3 = (std::find(filaForno.begin(), filaForno.end(), 3) != filaForno.end());
    return p1 && p2 && p3;
}

int ProximoDaFila() {
    if(TemDeadlock()) {
        printf("Deadlock detectado\n");
        return 0;
    }
    // TODO: Casais
    if(filaForno.empty()) return 0;
    int first = filaForno[0];
    for(auto it = filaForno.begin() + 1; it != filaForno.end(); it++) {
        if(Prioridade(*it, first) == *it)
            first = *it;
    }
    return first;
}

void Enfileirar(Personagem *p) {
    pthread_mutex_lock(&fila);
    filaForno.push_back(p->id);
    printf("%s quer usar o forno\n", p->Nome().c_str());
    ImprimirFila();
    pthread_mutex_unlock(&fila);
}

void Desenfileirar(Personagem *p) {
    pthread_mutex_lock(&fila);
    filaForno.erase(std::remove(filaForno.begin(), filaForno.end(), p->id), filaForno.end());
    printf("%s vai comer\n", p->Nome().c_str());
    ImprimirFila();
    pthread_mutex_unlock(&fila);
}

void ChamarProximo() {
    printf("Chamando próximo da fila\n");
    pthread_cond_broadcast(&sinal);
}

void Esquentar(Personagem *p) {
    while(p->id != ProximoDaFila()) {
        pthread_cond_wait(&sinal, &forno);
    }
    printf("%s começa a esquentar algo\n", p->Nome().c_str());
    sleep(1);
}

void Comer() {
    float tempo = 3 + get_random() * (6 - 3);
    sleep(tempo);
}

void* ThreadPersonagem(void *arg) {
    Personagem *p = (Personagem*)arg;

    while(p->vezes > 0) {
        // Entra na fila
        Enfileirar(p);
        // Tenta esquentar sua comida
        pthread_mutex_lock(&forno);
        Esquentar(p);
        // Termina e sai da fila para comer
        Desenfileirar(p);
        pthread_mutex_unlock(&forno);
        // Chama o próximo antes de comer
        ChamarProximo();
        Comer();
        p->vezes--;
    }

    pthread_exit(EXIT_SUCCESS);
}

int main() {

    int numPersonagens = 3;
    std::vector<Personagem*> personagens;
    pthread_t threads[numPersonagens];
    pthread_mutex_init(&forno, NULL);
    pthread_mutex_init(&fila, NULL);
    pthread_cond_init(&sinal, NULL);
    // Inicializando personagens
    for(int i = 0; i < numPersonagens; i++) {
        Personagem *p = new Personagem(i + 1);
        personagens.push_back(p);
    }
    // Iniciando threads
    for(int i = 0; i < numPersonagens; i++) {
        pthread_create(&threads[i], NULL, ThreadPersonagem, personagens[i]);
    }
    // Esperando threads acabarem
    for(int i = 0; i < numPersonagens; i++) {
        pthread_join(threads[i], NULL);
    }

    // Deleta personagens no final
    for(auto it = personagens.begin(); it != personagens.end(); ++it) {
        delete *it;
    }

    pthread_mutex_destroy(&forno);
    pthread_mutex_destroy(&fila);
    pthread_cond_destroy(&sinal);
    return 0;
}