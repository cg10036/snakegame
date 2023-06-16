#!/bin/sh

# build with cmake
#cmake .
#make

# build with only g++
g++ board.h game.cpp game.h input.cpp input.h main.cpp snake.cpp snake.h -lncursesw -o snakegame