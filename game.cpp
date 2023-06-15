#include <iostream>
#include <unistd.h>
#include <cmath>

#include "game.h"
#include "input.h"
#include "board.h"

Game::Game() {
    setlocale(LC_ALL, "");
    cout << "\e[8;27;67tLoading..." << endl;
    usleep(1000000);
    initscr();
    curs_set(0);
    start_color();
    noecho();

    this->game_window = subwin(stdscr, 27, 52, 0, 0);
    box(game_window, 0, 0);

    this->score_window = subwin(stdscr, 7, 15, 0, 52);
    box(score_window, 0, 0);

    this->mission_window = subwin(stdscr, 6, 15, 7, 52);
    box(mission_window, 0, 0);

    refresh();
}

void Game::run(int level, Mission mission) {
    this->mission = mission;
    this->level = level;
    Snake snake(level);
    Input input;
    this->now = {3, 0, 0, 0, 3};
    struct timespec tmspec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tmspec);
    KEY key = TIMEOUT;
    uint64_t start = tmspec.tv_sec * 1000 + tmspec.tv_nsec / 1000000;
    uint64_t item = start;
    uint64_t teleport = start;
    int teleport_cnt = -1;
    while (
            (this->now.now_length < this->mission.length ||
             this->now.growth < this->mission.growth ||
             this->now.poison < this->mission.poison ||
             this->now.teleport < this->mission.teleport)
            ) {
        if (this->now.now_length < 3) throw GameOver();
        usleep(5);
        clock_gettime(CLOCK_MONOTONIC_RAW, &tmspec);
        uint64_t end = tmspec.tv_sec * 1000 + tmspec.tv_nsec / 1000000;

        if (key != TIMEOUT) {
            if (key != ALREADY) snake.set_direction(key - 1);
            key = ALREADY;
        } else {
            key = input.getKey(5);
        }

        if (end - start < 300) continue;
        key = TIMEOUT;
        start += 300;

        int result = snake.move();
        if (result & GROWTH) this->now.growth++;
        if (result & POISON) this->now.poison++;
        if ((this->now.now_length = snake.get_size()) > this->mission.max_length) {
            snake.size_down();
            this->now.now_length = snake.get_size();
        }
        if (result & TELEPORT) {
            this->now.teleport++;
            teleport_cnt = this->now.now_length;
        }
        this->now.max_length = max(this->now.max_length, this->now.now_length);


        if (end - item >= 8000) {
            item += 8000;
            snake.reset_growth(2);
            snake.reset_poison(1);
        }
        if (end - teleport >= 10000 || teleport_cnt >= 0) {
            if (teleport_cnt <= 0) {
                if (teleport_cnt == 0) {
                    teleport = end;
                    teleport_cnt--;
                } else {
                    teleport += 10000;
                }
                snake.reset_teleport();
            } else {
                teleport_cnt--;
            }
        }

        int map[BOARD_Y][BOARD_X];
        snake.get_map(map);
        render(map);
    }

}

void Game::render(int map[BOARD_Y][BOARD_X]) {
    for (int i = 0; i < BOARD_Y; i++) {
        for (int j = 0; j < BOARD_X; j++) {
            char *c = " ";
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
            mvwprintw(this->game_window, 1 + i, 1 + j * 2, c);
        }
    }

    mvwprintw(this->score_window, 0, 2, "Score Board");
    mvwprintw(this->score_window, 1, 1, "Level: %d ", this->level);
    mvwprintw(this->score_window, 2, 1, "B: (%d / %d)  ", this->now.now_length, this->now.max_length);
    mvwprintw(this->score_window, 3, 1, "+: %d ", this->now.growth);
    mvwprintw(this->score_window, 4, 1, "-: %d ", this->now.poison);
    mvwprintw(this->score_window, 5, 1, "G: %d ", this->now.teleport);

    mvwprintw(this->mission_window, 0, 2, "Mission");
    mvwprintw(this->mission_window, 1, 1, "B: %d (%c) ", this->mission.length,
              this->now.now_length < this->mission.length ? ' ' : 'v');
    mvwprintw(this->mission_window, 2, 1, "+: %d (%c) ", this->mission.growth,
              this->now.growth < this->mission.growth ? ' ' : 'v');
    mvwprintw(this->mission_window, 3, 1, "-: %d (%c) ", this->mission.poison,
              this->now.poison < this->mission.poison ? ' ' : 'v');
    mvwprintw(this->mission_window, 4, 1, "G: %d (%c) ", this->mission.teleport,
              this->now.teleport < this->mission.teleport ? ' ' : 'v');

    wrefresh(this->game_window);
    wrefresh(this->score_window);
    wrefresh(this->mission_window);
    refresh();
}

Game::~Game() {
    delwin(this->mission_window);
    delwin(this->score_window);
    delwin(this->game_window);
    endwin();
}