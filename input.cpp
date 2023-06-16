#include <climits>

#include "input.h"

KEY Input::getKey(int time = INT_MAX) {
    timeout(time); // 타임아웃 설정
//    int input = getch(); // 정확하지만 인풋 씹힘이 있음
//    if (input == 27 && (input = getch()) == 91) {
//        switch (input = getch()) {
//            case 65:
//                return UP;
//            case 66:
//                return DOWN;
//            case 67:
//                return RIGHT;
//            case 68:
//                return LEFT;
//        }
//    }
    switch (getch()) { // 부정확하지만 인풋 씹힘이 없음
        case 65:
            return UP;
        case 66:
            return DOWN;
        case 67:
            return RIGHT;
        case 68:
            return LEFT;
    }
    return TIMEOUT;
}