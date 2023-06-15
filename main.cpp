#include <iostream>
#include "game.h"
#include "snake.h"

using namespace std;

int main() {
    Game game = Game();
    try {
        for(int i = 1;i <= 4;i++) {
            game.run(i, {4, 1, 1, 1, 10});
        }
    } catch (...) {
    }
}
