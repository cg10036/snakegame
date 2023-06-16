#include <random>

#include "snake.h"
#include "board.h"

using namespace std;

Snake::Snake(int level) {
    this->level = level - 1;
    this->snake = { // 초기 스네이크 설정
            {11, 12}, // tail
            {12, 12},
            {13, 12} // head
    };
}

void Snake::turn_left() { // 왼쪽으로 돌리기
    if (this->now_direction-- == 0) this->now_direction = 3;
}

void Snake::turn_right() { // 오른쪽으로 돌리기
    if (++this->now_direction == 4) this->now_direction = 0;
}

void Snake::turn_back() { // 뒤로 돌리기
    this->turn_right();
    this->turn_right();
}

void Snake::set_direction(int dir) { // 뱀 방향 설정
    this->now_direction = dir;
}

void Snake::check_body(Pos pos) { // 뱀 머리가 자기 몸통에 겹치는지 확인
    for (Pos i: this->snake) {
        if (i == pos) throw GameOver();
    }
}

int Snake::check_teleport(Pos &pos) { // 텔레포트인지 확인
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

void Snake::check_wall(Pos pos) { // 벽인지 확인
    if (BOARD[this->level][pos.y][pos.x] & (WALL | IMMUNE_WALL)) throw GameOver();
}

int Snake::check_growth(Pos pos) { // Growth 아이템 확인
    for (auto it = this->growth.begin(); it != this->growth.end(); it++) {
        if (*it == pos) {
            this->growth.erase(it);
            return GROWTH;
        }
    }
    return 0;
}

int Snake::check_poison(Pos pos) { // Poison 아이템 확인
    for (auto it = this->poison.begin(); it != this->poison.end(); it++) {
        if (*it == pos) {
            this->poison.erase(it);
            return POISON;
        }
    }
    return 0;
}

int Snake::check_super_poison(Pos pos) { // Super Poison 아이템 확인
    for (auto it = this->super_poison.begin(); it != this->super_poison.end(); it++) {
        if (*it == pos) {
            this->super_poison.erase(it);
            return SUPER_POISON;
        }
    }
    return 0;
}

int Snake::move() {
    Pos pos = this->snake.back();
    Pos dir = this->direction[this->now_direction];
    Pos final = {pos.x + dir.x, pos.y + dir.y};

    // 맵 밖으로 나가면 반대편으로
    if (final.x >= BOARD_X) final.x -= BOARD_X;
    if (final.x < 0) final.x += BOARD_X;
    if (final.y >= BOARD_Y) final.y -= BOARD_Y;
    if (final.y < 0) final.y += BOARD_Y;

    Pos tmp = this->snake.front();
    this->snake.pop_front(); // 뱀 꼬리쪽 뽑기

    int result = EMPTY;

    try {
        this->check_body(final); // 몸통에 충돌했는지 확인
        result |= this->check_teleport(final); // 텔레포트 확인, 텔레포트라면 final (pos) 수정
        this->check_wall(final); // 벽에 충돌했는지 확인
        result |= this->check_growth(final); // Growth 아이템을 먹었는지 확인
        result |= this->check_poison(final); // Poison 아이템을 먹었는지 확인
        result |= this->check_super_poison(final); // Super Poison 아이템을 먹었는지 확인

        if (result & GROWTH) this->snake.push_front(tmp); // Growth 아이템을 먹었다면 길이 ++
        if (result & (POISON | SUPER_POISON)) this->snake.pop_front(); // Poison, Super Poison 아이템을 먹었다면 길이 --
        if (result & SUPER_POISON) this->snake.pop_front(); // Super Poison 아이템을 먹었다면 길이 -- (위에서 --를 한번 했기 때문에 총 -2)
    } catch (...) { // 충돌한 경우
        this->snake.push_front(tmp); // 아까 뽑았던 뱀 꼬리 복구
        throw; // 게임오버
    }

    this->snake.push_back(final); // 뱀 머리 넣기

    return result;
}

void Snake::size_down() { // 사이즈 줄이기
    this->snake.pop_front();
}

void Snake::get_map(int map[BOARD_Y][BOARD_X]) { // 맵 그리기
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
    for (Pos i: this->super_poison) map[i.y][i.x] = SUPER_POISON;
    for (Pos i: this->teleport) map[i.y][i.x] = TELEPORT;
}

int Snake::get_random(int start, int end) { // 랜덤 숫자
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> rand(start, end);
    return rand(rng);
}

void Snake::reset_growth(int num) { // Growth 아이템 초기화
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
        for (Pos j: this->super_poison) {
            if (j == tmp) continue;
        }
        for (Pos j: this->snake) {
            if (j == tmp) continue;
        }
        this->growth.push_back(tmp);
        i++;
    }
}

void Snake::reset_poison(int num) { // Poison 아이템 초기화
    this->poison.clear();
    for (int i = 0; i < num;) {
        Pos tmp = {
                this->get_random(0, BOARD_X - 1),
                this->get_random(0, BOARD_Y - 1),
        };
        if (BOARD[this->level][tmp.y][tmp.x] & (WALL | IMMUNE_WALL)) continue;
        for (Pos j: this->super_poison) {
            if (j == tmp) continue;
        }
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

void Snake::reset_super_poison(int num) {  // Super Poison 아이템 초기화
    this->super_poison.clear();
    for (int i = 0; i < num;) {
        Pos tmp = {
                this->get_random(0, BOARD_X - 1),
                this->get_random(0, BOARD_Y - 1),
        };
        if (BOARD[this->level][tmp.y][tmp.x] & (WALL | IMMUNE_WALL)) continue;
        for (Pos j: this->poison) {
            if (j == tmp) continue;
        }
        for (Pos j: this->growth) {
            if (j == tmp) continue;
        }
        for (Pos j: this->snake) {
            if (j == tmp) continue;
        }
        this->super_poison.push_back(tmp);
        i++;
    }
}

void Snake::reset_teleport(int num) {  // 텔레포트 (포탈) 초기화
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

int Snake::get_size() { // 스네이크 사이즈
    return this->snake.size();
}