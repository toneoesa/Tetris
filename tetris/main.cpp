#include "tetris.h"

int main() {
    tetris::Tetris game;
    game.initDisplay();
    game.enter();
    game.destroyDisplay();
    return 0;
}
