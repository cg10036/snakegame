#ifndef SNAKEGAME_ITEMS_H
#define SNAKEGAME_ITEMS_H

class Item {
public:
    int x, y;

    Item(int x, int y);
};

class Growth : Item {
public:
    Growth(int x, int y);
};

class Poison : Item {
public:
    Poison(int x, int y);
};


#endif //SNAKEGAME_ITEMS_H
