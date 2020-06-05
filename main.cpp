#include "gamelogic.h"
#include "macro.h"

#include <cstdlib>
#include <iostream>

void instruction(int &difficulty, int &color_accent, int &players, int &timing) {
    players = 1;
    timing = 0;
    std::cout << "Welcome to the maze game!\n";
    std::cout << "Please enter difficulty(0~9)...\n";
    std::cin >> difficulty;
    while (difficulty < 0 || difficulty > 9) {
        std::cout << "Please enter difficulty(0~9)...\n";
        std::cin >> difficulty;
    }
    std::cout << "Please enter color_accent(0~9)...\n";
    std::cin >> color_accent;
    while (color_accent < 0 || color_accent > 9) {
        std::cout << "Please enter color_accent(0~9)...\n";
        std::cin >> color_accent;
    }
    std::cout << "Would you enable timing? (1=Y, 0=N)...\n";
    std::cin >> timing;
    while (timing != 0 && timing != 1) {
        std::cout << "Would you enable timing? (1=Y, 0=N)...\n";
        std::cin >> timing;
    }
    std::cout << "Loading...\n\n";
}

int main() {
    int difficulty = 0, color_accent = 1, players = 1, timing = 1;
    instruction(difficulty, color_accent, players, timing);
    main_loop(difficulty, color_accent, players, timing);
    return 0;
}
