#ifndef TETRIS_TETRIS_H
#define TETRIS_TETRIS_H

#include "display.h"
#include <atomic>

#include <mutex>
#include <vector>
#include <queue>

namespace tetris {
    void pressAnyKey(void *win = nullptr, const char *msg = nullptr);

// ================================================== class Tetris
    class Tetris {
    public:
        Tetris() = default;
        bool initDisplay();
        void enter();
        void destroyDisplay();

    private:
        bool initField();
        void randThread();
        void timerThread();
        void runningThread(unsigned long long tick);
        int getRand();
        void moveTetris(display::Tetrimino &t, char dir);
        void rotateTetris(display::Tetrimino &t);

    private:
        const static unsigned long long TICK_MS = 20;
        const static unsigned long long TICK_PER_FALL = 8;
        const static unsigned long long FLASH_MS = 200;
        const static int FLASH_TIMES = 2;
        const static unsigned int SCORE_BASE = 100;
        const static size_t RAND_QUEUE_LEN = 10;
        const static int RAND_NUM_MIN = 0;
        const static int RAND_NUM_MAX = 27;
        const static int DOWN_STEP = 5;

        const static std::vector<const std::vector<display::Tetrimino> *> TetrisList;

        int GlobalMaxRow = 0;
        int GlobalMaxCol = 0;
        display::GameField GGameField;
        display::Field PreviewField;
        display::Field ScoreField;
        display::Field InfoField;

        std::atomic<bool> GameRunning = false;
        std::mutex RunningMutex;
        std::mutex QueueMutex;
        std::queue<int> RandQueue;

        unsigned int CurScore = 0;
        const std::vector<display::Tetrimino> *CurTetrisList = nullptr;
        int CurTetrisDir = -1;
        display::Tetrimino CurTetris;
        const std::vector<display::Tetrimino> *NxtTetrisList = nullptr;
        int NxtTetrisDir = -1;
        display::Tetrimino NxtTetris;
    };
}

#endif //TETRIS_TETRIS_H
