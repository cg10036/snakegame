#ifndef SNAKEGAME_GAME_H
#define SNAKEGAME_GAME_H

#include <ncurses.h>
#include "input.h"
#include "snake.h"

using namespace std;

struct Mission {
    int length;
    int growth;
    int poison;
    int teleport;
    int max_length;
};

struct Status {
    int max_length;
    int growth;
    int poison;
    int teleport;
    int now_length;
};

class Game {
    Mission mission;
    Status now;
    int level;
    WINDOW *game_window, *score_window, *mission_window;
public:
    Game();
    ~Game();
    void run(int level, Mission mission);
    void render(int map[BOARD_Y][BOARD_X]);
};

#endif //SNAKEGAME_GAME_H