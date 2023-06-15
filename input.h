#ifndef SNAKEGAME_INPUT_H
#define SNAKEGAME_INPUT_H

#include <ncurses.h>

enum KEY {
    TIMEOUT, RIGHT, DOWN, LEFT, UP, ALREADY
};

class Input {
public:
    KEY getKey(int time);
};


#endif //SNAKEGAME_INPUT_H
