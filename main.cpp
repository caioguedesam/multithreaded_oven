#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include "personagem.h"
#include <unistd.h>
#include <algorithm>
#include <random>

std::vector<int> filaForno;
std::vector<int> filaCasal;
pthread_cond_t sinal;
pthread_mutex_t forno;
pthread_mutex_t fila;
bool raj;
int rajSelect;
int rajLimit;

float GetRandom() {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0,1);
    return dis(e);
}

void ImprimirFila() {
    printf("Fila Casal: ");
    for(auto it = filaCasal.begin(); it != filaCasal.end(); it++) {
        printf("%d ", *it);
    }
    printf("\n");
    printf("Fila Solteiro: ");
    for(auto it = filaForno.begin(); it != filaForno.end(); it++) {
        printf("%d ", *it);
    }
    printf("\n");
}

bool NaFila(int p) {
    return (std::find(filaForno.begin(), filaForno.end(), p) != filaForno.end());
}

bool NaFilaCasal(int p) {
    return (std::find(filaCasal.begin(), filaCasal.end(), p) != filaCasal.end());
}

void RemoverDaFila(int p) {
    filaForno.erase(std::remove(filaForno.begin(), filaForno.end(), p), filaForno.end());
    filaCasal.erase(std::remove(filaCasal.begin(), filaCasal.end(), p), filaCasal.end());
}

int Casal(int p) {
    if(p < 0) return 0;
    else if(p <= 3) return p + 3;
    else return p - 3;
}

int Prioridade(int p1, int p2) {
    int modP1 = (p1 % 4) + 1;
    int modP2 = (p2 % 4) + 1;
    // Membros do mesmo casal (ordem da chamada importa)
    if(modP1 == modP2) return p1;
    // Caso comum
    else {
        int max = std::max(modP1, modP2);
        int min = std::min(modP1, modP2);
        if(max == 3 && min == 1) return min;
        else return max;
    }
}

bool TemDeadlock() {
    bool naFila = (NaFila(1) || NaFila(4)) && (NaFila(2) || NaFila(5)) && (NaFila(3) || NaFila(6));
    bool naFilaCasal = (NaFilaCasal(1) || NaFilaCasal(4)) && (NaFilaCasal(2) || NaFilaCasal(5)) && (NaFilaCasal(3) || NaFilaCasal(6));
    return naFila || naFilaCasal;
}

int ProximoDaFila() {
    if(TemDeadlock()) {
        return 0;
    }
    // Primeiro vê na fila de casal
    if(!filaCasal.empty()) {
        int first = filaCasal[0];
        for(auto it = filaCasal.begin() + 1; it != filaCasal.end(); it++) {
            if(Prioridade(first, *it) == *it)
                first = *it;
        }
        return first;
    }
    // Ninguém na fila de casal
    else {
        if(filaForno.empty()) return 0;
        int first = filaForno[0];
        for(auto it = filaForno.begin() + 1; it != filaForno.end(); it++) {
            if(Prioridade(*it, first) == *it)
                first = *it;
        }
        return first;
    }
}

void Enfileirar(Personagem *p) {
    pthread_mutex_lock(&fila);
    int pCasal = Casal(p->id);
    // Se encontra o casal, move ambos para a fila de casais
    if(pCasal != 0 && (NaFila(pCasal) || NaFilaCasal(pCasal))) {
        RemoverDaFila(pCasal);
        filaCasal.push_back(pCasal);
        filaCasal.push_back(p->id);
    }
    // Se não, coloca na fila normal de baixa prioridade
    else {
        filaForno.push_back(p->id);
    }
    printf("%s quer usar o forno\n", p->Nome().c_str());
    ImprimirFila();
    pthread_mutex_unlock(&fila);
}

void Desenfileirar(Personagem *p) {
    pthread_mutex_lock(&fila);
    RemoverDaFila(p->id);
    printf("%s vai comer\n", p->Nome().c_str());
    ImprimirFila();
    pthread_mutex_unlock(&fila);
}

void ChamarProximo() {
    printf("Chamando próximo da fila\n");
    pthread_cond_broadcast(&sinal);
}

void ResetRaj() {
    raj = false;
    rajSelect = 0;
}

void Esquentar(Personagem *p) {
    while(p->id != ProximoDaFila()) {
        // Verificar seleção do raj caso haja deadlock
        if(raj && rajSelect == p->id) {
            ResetRaj();
            break;
        }
        pthread_cond_wait(&sinal, &forno);
    }
    printf("%s começa a esquentar algo\n", p->Nome().c_str());
    sleep(1);
}

void Comer() {
    float tempo = 3 + GetRandom() * (6 - 3);
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

    rajLimit++;
    pthread_exit(EXIT_SUCCESS);
}

int GetRandomAtDeadlock() {
    std::vector<int> temp = (filaCasal.empty()) ? filaForno : filaCasal;
    // Remove Stuart e Kripke das seleções de deadlock
    temp.erase(std::remove(temp.begin(), temp.end(), -1), temp.end());
    temp.erase(std::remove(temp.begin(), temp.end(), -2), temp.end());

    static std::default_random_engine e;
    static std::uniform_int_distribution<int> dis(0, temp.size());
    int sample = temp[dis(e)];
    printf("Raj selected %d\n", sample);
    return sample;
}

void *ThreadRaj(void *arg) {
    int *count = (int *)arg;
    // Raj não para até que todos os outros indiquem a ele que pararam
    while(rajLimit < *count) {
        sleep(5);
        // Resolver deadlock
        if(!raj && TemDeadlock()) {
            // Ativa a seleção do raj para ser desligada após a seleção
            raj = true;
            // Escolhe um aleatório
            rajSelect = GetRandomAtDeadlock();
            // Manda signal broadcast
            pthread_cond_broadcast(&sinal);
        }
    }

    pthread_exit(EXIT_SUCCESS);
}

int main() {

    int numPersonagens = 6;
    std::vector<Personagem*> personagens;
    pthread_t threads[numPersonagens];
    pthread_t rajThread;
    pthread_mutex_init(&forno, NULL);
    pthread_mutex_init(&fila, NULL);
    pthread_cond_init(&sinal, NULL);
    raj = false;
    rajLimit = 0;

    // Inicializando personagens
    for(int i = 0; i < numPersonagens; i++) {
        Personagem *p = new Personagem(i + 1);
        personagens.push_back(p);
    }
    // Iniciando threads
    for(int i = 0; i < numPersonagens; i++) {
        pthread_create(&threads[i], NULL, ThreadPersonagem, personagens[i]);
    }
    pthread_create(&rajThread, NULL, ThreadRaj, &numPersonagens);
    // Esperando threads acabarem
    for(int i = 0; i < numPersonagens; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(rajThread, NULL);

    // Deleta personagens no final
    for(auto it = personagens.begin(); it != personagens.end(); ++it) {
        delete *it;
    }

    pthread_mutex_destroy(&forno);
    pthread_mutex_destroy(&fila);
    pthread_cond_destroy(&sinal);
    return 0;
}