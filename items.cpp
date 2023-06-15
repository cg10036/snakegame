#include "items.h"

Item::Item(int x, int y) : x(x), y(y) {}

Growth::Growth(int x, int y) : Item(x, y) {}

Poison::Poison(int x, int y) : Item(x, y) {}