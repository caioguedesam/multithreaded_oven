#include "personagem.h"

Personagem::Personagem(int pId) {
    id = pId;
    vezes = 2;
}

std::string Personagem::Nome() {
    switch(id) {
        case 1: return std::string("Leonard");
        case 2: return std::string("Howard");
        case 3: return std::string("Sheldon");
        case -1: return std::string("Stuart");
        case -2: return std::string("Kripke");
        default: return std::string("");
    }
}