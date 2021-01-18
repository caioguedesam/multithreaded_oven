#include "personagem.h"

Personagem::Personagem(int pId, int pVezes) {
    id = pId;
    vezes = pVezes;
}

std::string Personagem::Nome() {
    switch(id) {
        case 1: return std::string("Leonard");
        case 2: return std::string("Howard");
        case 3: return std::string("Sheldon");
        case 4: return std::string("Penny");
        case 5: return std::string("Bernadette");
        case 6: return std::string("Amy");
        case -1: return std::string("Stuart");
        case -2: return std::string("Kripke");
        default: return std::string("");
    }
}

std::string Nome(int pId) {
    switch(pId) {
        case 1: return std::string("Leonard");
        case 2: return std::string("Howard");
        case 3: return std::string("Sheldon");
        case 4: return std::string("Penny");
        case 5: return std::string("Bernadette");
        case 6: return std::string("Amy");
        case -1: return std::string("Stuart");
        case -2: return std::string("Kripke");
        default: return std::string("");
    }
}