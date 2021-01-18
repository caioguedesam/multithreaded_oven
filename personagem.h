#pragma once
#include <string>

class Personagem {
    public:
    int id;
    int vezes;

    Personagem(int pId);
    std::string Nome();
};

std::string Nome(int pId);