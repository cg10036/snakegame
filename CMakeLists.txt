cmake_minimum_required(VERSION 3.24)
project(snakegame)

set(CMAKE_CXX_STANDARD 17)

set(CURSES_NEED_NCURSED TRUE)
set(CURSES_NEED_WIDE TRUE)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(snakegame main.cpp game.cpp game.h board.h input.cpp input.h items.cpp items.h snake.cpp snake.h)
target_link_libraries(snakegame ${CURSES_LIBRARIES})