#include <iostream>
#include "game.h"

using namespace std;

int main() {
    Game game = Game(); // Game 클래스 생성
    for (int i = 1; i <= 4; i++) {
        try {
            // 레벨별 게임 실행
            game.run(i, {4, 1, 1, 1, 1, 10});
        } catch (...) {
            // 죽었을 경우
            i--; // 해당 레벨을 다시 실행
        }
    }
    game.Game::~Game(); // 소멸자 명시적 실행
    cout << "Game Clear!" << endl;
    cout << "나는 이무기가 아니었다!" << endl << flush;
}
