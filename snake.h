//
// Created by 이수혁 on 2023/06/15.
//

#ifndef SNAKEGAME_SNAKE_H
#define SNAKEGAME_SNAKE_H

#include <deque>
#include "board.h"

using namespace std;

struct Pos {
    int x;
    int y;

    bool operator==(Pos a) {
        return this->x == a.x && this->y == a.y;
    }
};

enum TYPE {
    EMPTY = 0,
    WALL = 1 << 0,
    IMMUNE_WALL = 1 << 1,
    BODY = 1 << 2,
    HEAD = 1 << 3,
    GROWTH = 1 << 4,
    POISON = 1 << 5,
    TELEPORT = 1 << 6
};

class GameOver {
};

class Snake {
    deque<Pos> snake, growth, poison, teleport;
    Pos direction[4] = {
            {1,  0}, // right
            {0,  1}, // down
            {-1, 0}, // left
            {0,  -1} // up
    };
    int now_direction = 0, level;

    void check_body(Pos pos);

    void check_wall(Pos pos);

    int check_teleport(Pos &pos);

    int check_growth(Pos pos);

    int check_poison(Pos pos);

public:
    Snake(int level);

    void turn_left();

    void turn_right();

    int move();

    void turn_back();

    void get_map(int map[BOARD_Y][BOARD_X]);

    void set_direction(int dir);

    void reset_growth(int num);

    void reset_poison(int num);

    void reset_teleport(int num = 2); // num should be 0 or 2

    int get_size();

    int get_random(int start, int end);

    void size_down();
};

#endif //SNAKEGAME_SNAKE_H
