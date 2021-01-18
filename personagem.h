#pragma once
#include <string>

class Personagem {
    public:
    int id;
    int vezes;

    Personagem(int pId, int pVezes);
    std::string Nome();
};

std::string Nome(int pId);