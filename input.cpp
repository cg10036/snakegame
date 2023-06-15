#include <climits>

#include "input.h"

KEY Input::getKey(int time = INT_MAX) {
    timeout(time);
    int input = getch();
    if (input == 27 && (input = getch()) == 91) {
        switch (input = getch()) {
            case 65:
                return UP;
            case 66:
                return DOWN;
            case 67:
                return RIGHT;
            case 68:
                return LEFT;
        }
    }
    return TIMEOUT;
}