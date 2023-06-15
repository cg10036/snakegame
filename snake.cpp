//
// Created by 이수혁 on 2023/06/15.
//
#include <random>

#include "snake.h"
#include "board.h"


using namespace std;

Snake::Snake(int level) {
    this->level = level - 1;
    this->snake = {
            {11, 12}, // tail
            {12, 12},
            {13, 12} // head
    };
}

void Snake::turn_left() {
    if (this->now_direction-- == 0) this->now_direction = 3;
}

void Snake::turn_right() {
    if (++this->now_direction == 4) this->now_direction = 0;
}

void Snake::turn_back() {
    this->turn_right();
    this->turn_right();
}

void Snake::set_direction(int dir) {
    this->now_direction = dir;
}

void Snake::check_body(Pos pos) {
    for (Pos i: this->snake) {
        if (i == pos) throw GameOver();
    }
}

int Snake::check_teleport(Pos &pos) {
    for (auto it = this->teleport.begin(); it != this->teleport.end(); it++) {
        if (*it == pos) {
            if (it + 1 == this->teleport.end()) {
                pos = *(it - 1);
            } else {
                pos = *(it + 1);
            }

            while (true) {
                Pos next = {
                        pos.x + this->direction[this->now_direction].x,
                        pos.y + this->direction[this->now_direction].y
                };
                if (next.x < 0 || next.x >= BOARD_X || next.y < 0 || next.y >= BOARD_Y ||
                    BOARD[this->level][next.y][next.x] & (WALL | IMMUNE_WALL)) {
                    this->turn_right();
                    continue;
                }
                pos = next;
                break;
            }

            return TELEPORT;
        }
    }
    return 0;
}

void Snake::check_wall(Pos pos) {
    if (BOARD[this->level][pos.y][pos.x] & (WALL | IMMUNE_WALL)) throw GameOver();
}

int Snake::check_growth(Pos pos) {
    for (auto it = this->growth.begin(); it != this->growth.end(); it++) {
        if (*it == pos) {
            this->growth.erase(it);
            return GROWTH;
        }
    }
    return 0;
}

int Snake::check_poison(Pos pos) {
    for (auto it = this->poison.begin(); it != this->poison.end(); it++) {
        if (*it == pos) {
            this->poison.erase(it);
            return POISON;
        }
    }
    return 0;
}

int Snake::move() {
    Pos pos = this->snake.back();
    Pos dir = this->direction[this->now_direction];
    Pos final = {pos.x + dir.x, pos.y + dir.y};

    if (final.x >= BOARD_X) final.x -= BOARD_X;
    if (final.x < 0) final.x += BOARD_X;
    if (final.y >= BOARD_Y) final.y -= BOARD_Y;
    if (final.y < 0) final.y += BOARD_Y;

    Pos tmp = this->snake.front();
    this->snake.pop_front();

    int result = EMPTY;

    try {
        this->check_body(final);
        result |= this->check_teleport(final);
        this->check_wall(final);
        result |= this->check_growth(final);
        result |= this->check_poison(final);

        if (result & GROWTH) this->snake.push_front(tmp);
        if (result & POISON) this->snake.pop_front();
    } catch (...) {
        this->snake.push_front(tmp);
        throw;
    }

    this->snake.push_back(final);

    return result;
}

void Snake::size_down() {
    this->snake.pop_front();
}

void Snake::get_map(int map[BOARD_Y][BOARD_X]) {
    for (int i = 0; i < BOARD_Y; i++) {
        for (int j = 0; j < BOARD_X; j++) {
            map[i][j] = BOARD[this->level][i][j] ? WALL : EMPTY;
        }
    }
    for (int i = 0; i < this->snake.size(); i++) {
        Pos tmp = this->snake[i];
        map[tmp.y][tmp.x] = i == this->snake.size() - 1 ? HEAD : BODY;
    }
    for (Pos i: this->growth) map[i.y][i.x] = GROWTH;
    for (Pos i: this->poison) map[i.y][i.x] = POISON;
    for (Pos i: this->teleport) map[i.y][i.x] = TELEPORT;
}

int Snake::get_random(int start, int end) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> rand(start, end);
    return rand(rng);
}

void Snake::reset_growth(int num) {
    this->growth.clear();
    for (int i = 0; i < num;) {
        Pos tmp = {
                this->get_random(0, BOARD_X - 1),
                this->get_random(0, BOARD_Y - 1),
        };
        if (BOARD[this->level][tmp.y][tmp.x] & (WALL | IMMUNE_WALL)) continue;
        for (Pos j: this->poison) {
            if (j == tmp) continue;
        }
        for (Pos j: this->snake) {
            if (j == tmp) continue;
        }
        this->growth.push_back(tmp);
        i++;
    }
}

void Snake::reset_poison(int num) {
    this->poison.clear();
    for (int i = 0; i < num;) {
        Pos tmp = {
                this->get_random(0, BOARD_X - 1),
                this->get_random(0, BOARD_Y - 1),
        };
        if (BOARD[this->level][tmp.y][tmp.x] & (WALL | IMMUNE_WALL)) continue;
        for (Pos j: this->growth) {
            if (j == tmp) continue;
        }
        for (Pos j: this->snake) {
            if (j == tmp) continue;
        }
        this->poison.push_back(tmp);
        i++;
    }
}

void Snake::reset_teleport(int num) {
    this->teleport.clear();
    for (int i = 0; i < num;) {
        Pos tmp = {
                this->get_random(0, BOARD_X - 1),
                this->get_random(0, BOARD_Y - 1),
        };
        if (i && this->teleport.front() == tmp) continue;
        if (!(BOARD[this->level][tmp.y][tmp.x] & WALL)) continue;
        this->teleport.push_back(tmp);
        i++;
    }
}

int Snake::get_size() {
    return this->snake.size();
}