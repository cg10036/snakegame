#include <iostream>
#include <unistd.h>
#include <cmath>

#include "game.h"
#include "input.h"
#include "board.h"

Game::Game() {
    setlocale(LC_ALL, ""); // 특수문자 출력을 위해 locale 바꾸기
    cout << "\e[8;27;67tLoading..." << endl; // 터미널 사이즈를 67X27로 변경
    cout << "꼭 용이 되어서 하늘로 승천할거야!" << endl;
    usleep(3000000); // 오류 방지를 위한 터미널 사이즈 변경 대기
    initscr(); // ncurses 라이브러리 초기화
    curs_set(0); // 커서를 안보이게 설정
    start_color(); // 색상 표시 가능하게 설정
    noecho(); // 입력한 키가 표시되지 않게 변경

    // 윈도우 생성과 설정
    this->game_window = subwin(stdscr, 27, 52, 0, 0); // 게임 윈도우 생성
    box(this->game_window, 0, 0); // 윈도우에 박스 그리기

    this->score_window = subwin(stdscr, 8, 15, 0, 52); // 점수 윈도우 생성
    box(this->score_window, 0, 0); // 윈도우에 박스 그리기

    this->mission_window = subwin(stdscr, 7, 15, 8, 52); // 미션 윈도우 생성
    box(this->mission_window, 0, 0); // 윈도우에 박스 그리기

    this->timer_window = subwin(stdscr, 4, 15, 15, 52); // 타이머 윈도우 생성
    box(this->timer_window, 0, 0); // 윈도우에 박스 그리기

    this->item_window = subwin(stdscr, 8, 15, 19, 52); // 아이템 설명 윈도우 생성
    box(this->item_window, 0, 0);

    refresh(); // 화면 새로고침
}

void Game::run(int level, Mission mission) {
    this->mission = mission; // 미션 설정
    this->level = level; // 레벨 설정
    Snake snake(level); // 스네이크 엔진 초기화
    Input input; // 인풋 엔진 초기화
    this->now = {3, 0, 0, 0, 0, 3}; // 현재 상태 초기화

    // 시간 가져오기 및 초기화 (Tick)
    struct timespec tmspec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tmspec);
    uint64_t start = tmspec.tv_sec * 1000 + tmspec.tv_nsec / 1000000;
    uint64_t item = start;
    uint64_t teleport = start;

    KEY key = TIMEOUT; // 입력 키 받을 변수 초기화
    int teleport_cnt = -1; // 텔레포트시 포탈 유지시키는 용도의 변수
    while (
            (this->now.max_length < this->mission.length || // 길이가 짧거나
             this->now.growth < this->mission.growth || // 아이템을 적게 먹었거나
             this->now.poison < this->mission.poison ||
             this->now.super_poison < this->mission.super_poison ||
             this->now.teleport < this->mission.teleport) // 텔레포트를 덜탄 경우
            ) { // 와일문 반복
        if (this->now.now_length < 3) throw GameOver(); // 길이가 3보다 작아지면 게임오버
        usleep(5); // 5us (micro second) 쉬기

        // 현재 시간 가져오기
        clock_gettime(CLOCK_MONOTONIC_RAW, &tmspec);
        uint64_t end = tmspec.tv_sec * 1000 + tmspec.tv_nsec / 1000000;

        // 키가 TIMEOUT (초기) 상태가 아니라면
        if (key != TIMEOUT) {
            // 키가 이미 입력되지 않았다면 Direction 설정
            if (key != ALREADY) snake.set_direction(key - 1);
            // key를 입력된 상태로 변경
            key = ALREADY;
        } else {
            // 키 입력
            key = input.getKey(5);
        }

        if (end - item >= ITEM_RESET_TIME) { // 아이템 리셋 시간이 지나면
            item += ITEM_RESET_TIME; // 리셋 시간을 더함 (오차를 최소화 하기위한 방법)
            snake.reset_growth(1); // Growth 아이템 초기화
            snake.reset_poison(1); // Poison 아이템 초기화
            snake.reset_super_poison(1); // Super Poison 아이템 초기화
        }

        // 텔레포트 리셋 시간이 지나거나 teleport_cnt가 0보다 크다면 (텔레포트가 진행중인 경우)
        if (end - teleport >= TELEPORT_RESET_TIME || teleport_cnt >= 0) {
            if (teleport_cnt <= 0) { // 텔레포트가 진행중이거나 거의 끝난 경우 (0: 끝나기 바로 전 상태, -1: 끝난 상태)
                if (teleport_cnt == 0) { // 텔레포트가 곧 끝나는 경우
                    teleport = end; // 리셋 시간 초기화
                    teleport_cnt--; // teleport_cnt를 -1로 설정
                } else {
                    teleport += TELEPORT_RESET_TIME; // 리셋 시간을 더함
                }
                snake.reset_teleport(); // 리셋
            }
        }

        int map[BOARD_Y][BOARD_X];
        snake.get_map(map); // 맵 상태 가져오기

        // 맵 렌더링
        render(map, ITEM_RESET_TIME - end + item, TELEPORT_RESET_TIME - end + teleport);

        if (end - start < 300) continue; // 300초 (Tick)이 지나야 움직이도록 continue
        key = TIMEOUT; // 이미 위에서 direction을 설정해 줬으므로 key 초기화
        start += 300; // 시간 더하기

        int result = snake.move(); // 스네이크 움직이기
        if (result & GROWTH) this->now.growth++; // Growth 아이템 획득시 카운팅
        if (result & POISON) this->now.poison++; // Poison 아이템 획득시 카운팅
        if (result & SUPER_POISON) this->now.super_poison++; // Super Poison 아이템 획득시 카운팅

        // 최대 크기보다 현재 크기가 클 경우
        if ((this->now.now_length = snake.get_size()) > this->mission.max_length) {
            snake.size_down(); // 사이즈 줄이기
            this->now.now_length = snake.get_size(); // 변수 다시 초기화
        }

        if (result & TELEPORT) { // 텔레포트 (포탈) 들어가면
            this->now.teleport++; // 텔레포트 (포탈) 들어가면 카운팅
            teleport_cnt = this->now.now_length; // 중간에 텔레포트 (포탈)이 없어지지 않도록 내 몸 사이즈만큼 추가
        }
        this->now.max_length = max(this->now.max_length, this->now.now_length); // 최대 길이 설정

        if (teleport_cnt > 0) teleport_cnt--; // 틱 한번당 지나갔다고 생각해서 teleport_cnt를 --
    }
}

void Game::render(int map[BOARD_Y][BOARD_X], int item_timer, int teleport_timer) {
    // 화면에 출력
    for (int i = 0; i < BOARD_Y; i++) {
        for (int j = 0; j < BOARD_X; j++) {
            char const *c = " ";
            switch (map[i][j]) {
                case HEAD:
                    c = "○";
                    break;
                case BODY:
                    c = "●";
                    break;
                case POISON:
                    c = "-";
                    break;
                case SUPER_POISON:
                    c = "=";
                    break;
                case GROWTH:
                    c = "+";
                    break;
                case WALL:
                case IMMUNE_WALL:
                    c = "■";
                    break;
                case TELEPORT:
                    c = "▢";
            }
            mvwprintw(this->game_window, 1 + i, 1 + j * 2, "%s", c);
        }
    }

    // 스코어 보드 출력
    mvwprintw(this->score_window, 0, 2, "Score Board");
    mvwprintw(this->score_window, 1, 1, "Level: %d ", this->level);
    mvwprintw(this->score_window, 2, 1, "B: (%d / %d)  ", this->now.now_length, this->now.max_length);
    mvwprintw(this->score_window, 3, 1, "+: %d ", this->now.growth);
    mvwprintw(this->score_window, 4, 1, "-: %d ", this->now.poison);
    mvwprintw(this->score_window, 5, 1, "=: %d ", this->now.super_poison);
    mvwprintw(this->score_window, 6, 1, "G: %d ", this->now.teleport);

    // 미션 출력
    mvwprintw(this->mission_window, 0, 2, "Mission");
    mvwprintw(this->mission_window, 1, 1, "B: %d (%c) ", this->mission.length,
              this->now.max_length < this->mission.length ? ' ' : 'v');
    mvwprintw(this->mission_window, 2, 1, "+: %d (%c) ", this->mission.growth,
              this->now.growth < this->mission.growth ? ' ' : 'v');
    mvwprintw(this->mission_window, 3, 1, "-: %d (%c) ", this->mission.poison,
              this->now.poison < this->mission.poison ? ' ' : 'v');
    mvwprintw(this->mission_window, 4, 1, "=: %d (%c) ", this->mission.super_poison,
              this->now.super_poison < this->mission.super_poison ? ' ' : 'v');
    mvwprintw(this->mission_window, 5, 1, "G: %d (%c) ", this->mission.teleport,
              this->now.teleport < this->mission.teleport ? ' ' : 'v');

    // 타이머 출력
    mvwprintw(this->timer_window, 0, 2, "Reset Timer");
    mvwprintw(this->timer_window, 1, 1, "Items:  %0.1lf ", (double) item_timer / 1000);
    mvwprintw(this->timer_window, 2, 1, "Portal: %0.1lf ", (double) teleport_timer / 1000);

    // 아이셈 설명 윈도우 출력
    mvwprintw(this->item_window, 0, 2, "Items");
    mvwprintw(this->item_window, 1, 1, "Wall: ■");
    mvwprintw(this->item_window, 2, 1, "Portal: ▢");
    mvwprintw(this->item_window, 4, 1, "Growth: +");
    mvwprintw(this->item_window, 5, 1, "Poison: -");
    mvwprintw(this->item_window, 6, 1, "SPoison: =");

    // 새로고침
    wrefresh(this->item_window);
    wrefresh(this->game_window);
    wrefresh(this->score_window);
    wrefresh(this->mission_window);
    wrefresh(this->timer_window);
    refresh();
}

Game::~Game() {
    // 윈도우 삭제
    delwin(this->mission_window);
    delwin(this->score_window);
    delwin(this->game_window);
    endwin();
}