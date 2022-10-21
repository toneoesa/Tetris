#include "tetris.h"
#include <thread>
#include <experimental/random>

using namespace tetris;

// ================================================== functions
void tetris::pressAnyKey(void *win, const char *msg) {
    if (!win) win = display::getGlobalWin();
    if (msg) {
        display::d_wprintw(win, "%s", msg);
    } else {
        display::d_wprintw(win, "Press any key ...\n");
    }
    display::d_wrefresh(win);
    display::d_wgetchar(win);
}

// ================================================== class Tetris
const std::vector<const std::vector<display::Tetrimino> *> Tetris::TetrisList = {
        &display::I, &display::L, &display::J, &display::O, &display::S, &display::T, &display::Z
};
const unsigned long long Tetris::TICK_MS;
const unsigned long long Tetris::TICK_PER_FALL;
const unsigned long long Tetris::FLASH_MS;
const int Tetris::FLASH_TIMES;
const unsigned int Tetris::SCORE_BASE;
const size_t Tetris::RAND_QUEUE_LEN;
const int Tetris::RAND_NUM_MIN;
const int Tetris::RAND_NUM_MAX;
const int Tetris::DOWN_STEP;

bool Tetris::initDisplay() {
    // init
    display::RET_CODE ret = display::initDisplay();
    if (ret) {
        display::d_wprintw(display::getGlobalWin(), "%s\n", display::RET_INFO[ret]);
        pressAnyKey();
        return false;
    }
    display::getMaxYX(GlobalMaxRow, GlobalMaxCol);

    if (!initField()) {
        display::destroyDisplay();
        return false;
    }
    return true;
}

void Tetris::enter() {
    using namespace std::chrono;
    using namespace std::this_thread;

    pressAnyKey(InfoField.getWin(), "A, S, D: Left, Down, Right\n<Space>: Pause/Continue\n   Q   : exit\nPress any key to start\n");
    display::d_wprintw(InfoField.getWin(), "[Game Start]\n");

    // start prepare
    display::nodelay(InfoField.getWin(), true);
    CurScore = 0;
    display::d_wmove(ScoreField.getWin(), 0, 0);
    display::d_wprintw(ScoreField.getWin(), "Score\n%9d\n", CurScore);
    display::d_wrefresh(ScoreField.getWin());

    GameRunning = true;
    std::thread rander(&Tetris::randThread, this);
    sleep_for(milliseconds(TICK_MS));

    // init tetris
    CurTetrisList = TetrisList[getRand() % TetrisList.size()];
    CurTetrisDir = getRand() % (int) CurTetrisList->size();
    CurTetris = (*CurTetrisList)[CurTetrisDir];
    GGameField.moveTetrisToStartPoint(CurTetris);

    NxtTetrisList = TetrisList[getRand() % TetrisList.size()];
    NxtTetrisDir = getRand() % (int) NxtTetrisList->size();
    NxtTetris = (*NxtTetrisList)[NxtTetrisDir];
    PreviewField.moveTetrisToCenter(NxtTetris);

    std::thread timer(&Tetris::timerThread, this);
    rander.join();
    timer.join();

    // exit
    display::nodelay(InfoField.getWin(), false);

    display::d_wprintw(InfoField.getWin(), "Press q to exit\n");
    int ch;
    while ((ch = display::d_wgetchar(InfoField.getWin())) != 'q') {}
}

void Tetris::destroyDisplay() {
    GGameField.endWin();
    ScoreField.endWin();
    PreviewField.endWin();
    InfoField.endWin();
    display::destroyDisplay();
}

bool Tetris::initField() {
    if (GlobalMaxRow < 20 || GlobalMaxCol < 40) {
        display::d_wprintw(display::getGlobalWin(), "Your terminal are smaller than 20x40, please exit and resize.\n");
        pressAnyKey();
        return false;
    }

    int sfHeight = 4;
    int sfWidth = 30;
    int gfHeight = GlobalMaxRow;
    int gfWidth = std::min((gfHeight * 4 / 3) / 2 * 2, GlobalMaxCol - sfWidth); // ensure weight is even
    int pfWitdh = sfWidth;
    int pfHeight = std::min(pfWitdh / 2, 6);
    int ifWidth = sfWidth;
    int ifHeight = GlobalMaxRow - sfHeight - pfHeight;

    bool ret = GGameField.startWin(0, 0, gfHeight, gfWidth);
    if (ret) {
        display::d_wprintw(display::getGlobalWin(), "Create game field failed.\n");
        return false;
    }

    ret = ScoreField.startWin(0, gfWidth, sfHeight, sfWidth);
    if (ret) {
        display::d_wprintw(display::getGlobalWin(), "Create score field failed.\n");
        return false;
    }

    ret = PreviewField.startWin(sfHeight, gfWidth, pfHeight, pfWitdh);
    if (ret) {
        display::d_wprintw(display::getGlobalWin(), "Create preview field failed.\n");
        return false;
    }

    ret = InfoField.startWin(sfHeight + pfHeight, gfWidth, ifHeight, ifWidth);
    if (ret) {
        display::d_wprintw(display::getGlobalWin(), "Create info field failed.\n");
        return false;
    }
    InfoField.enScroll(true);

    return true;
}

void Tetris::randThread() {
    using namespace std::chrono;
    using namespace std::this_thread;
    using namespace std::experimental;

    reseed(duration_cast<seconds>(system_clock::now().time_since_epoch()).count());

    while (GameRunning) {
        QueueMutex.lock();
        while (RandQueue.size() < RAND_QUEUE_LEN) {
            RandQueue.push(randint(RAND_NUM_MIN, RAND_NUM_MAX));
        }
        QueueMutex.unlock();
        sleep_for(milliseconds(TICK_MS * TICK_PER_FALL / 3));
    }
}

void Tetris::timerThread() {
    using namespace std::chrono;
    using namespace std::this_thread;

    unsigned long long tick = 0;
    while (GameRunning) {
        std::thread run(&Tetris::runningThread, this, tick);
        run.detach();

        sleep_for(milliseconds(TICK_MS));
        ++tick;
    }
}

void Tetris::runningThread(unsigned long long tick) {
    if (RunningMutex.try_lock()) {
        int ch = -1;
        int tmp;
        while ((tmp = display::d_wgetchar(InfoField.getWin())) != display::GETCH_ERR) {
            ch = tmp;
        }

        switch (ch) {
            case -1:
                break;
            case ' ':
                display::nodelay(InfoField.getWin(), false);
                pressAnyKey(InfoField.getWin(), "[Game Pause]\n");
                display::d_wprintw(InfoField.getWin(), "[Game Continue]\n");
                display::nodelay(InfoField.getWin(), true);
                break;
            case 'q':
                GameRunning = false;
                display::d_wprintw(InfoField.getWin(), "[Game Exit]\n");
                break;
            case 't':
                display::d_wprintw(InfoField.getWin(), "Test info\n");
                break;
            case 'w':
            case 'a':
            case 's':
            case 'd':
                moveTetris(CurTetris, ch);
                break;
            case 'e':
                rotateTetris(CurTetris);
                break;
            default:
                display::d_wprintw(InfoField.getWin(), "Unsupported key 0x%2X [%c]\n", ch, ch);
        }

        // time to fall
        if (!(tick % TICK_PER_FALL)) {
            auto checkRet = GGameField.hitCheck(1, 0, CurTetris);
            if (checkRet == display::GameField::CHECK_OK) {
                CurTetris.move(GGameField.getWin(), 1, 0);
            } else {
                checkRet = GGameField.hitCheck(0, 0, CurTetris, true);
                if (checkRet != display::GameField::CHECK_OK) { // Game Over
                    GameRunning = false;
                    display::d_wprintw(InfoField.getWin(), "[Game Over]\n");
                    return;
                }

                // fall to bottom, no need to erase CurTetris
                GGameField.add(CurTetris);

                // check line complete
                int lineList[display::Tetrimino::HEIGHT];
                int completeNum = GGameField.checkComplete(CurTetris, lineList);
                if (completeNum) {
                    for (int k = 0; k < FLASH_TIMES; ++k) {
                        for (int i = 0; i < completeNum; ++i) {
                            GGameField.hideLine(lineList[i]);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(FLASH_MS / 2));
                        for (int i = 0; i < completeNum; ++i) {
                            GGameField.hideLine(lineList[i], false);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(FLASH_MS / 2));
                    }
                    GGameField.fall(lineList, completeNum);
                }

                CurScore += completeNum * completeNum * SCORE_BASE;
                display::d_wmove(ScoreField.getWin(), 0, 0);
                display::d_wprintw(ScoreField.getWin(), "Score\n%9d\n", CurScore);
                display::d_wrefresh(ScoreField.getWin());

                // move next tetris to game field
                NxtTetris.erase(PreviewField.getWin());
                CurTetrisList = NxtTetrisList;
                CurTetrisDir = NxtTetrisDir;
                CurTetris = NxtTetris;
                GGameField.moveTetrisToStartPoint(CurTetris);

                NxtTetrisList = TetrisList[getRand() % TetrisList.size()];
                NxtTetrisDir = getRand() % (int) NxtTetrisList->size();
                NxtTetris = (*NxtTetrisList)[NxtTetrisDir];
                PreviewField.moveTetrisToCenter(NxtTetris);
            }
        }

        RunningMutex.unlock();
    } else {
//        display::d_wprintw(InfoField.getWin(), "Skip tick %llu\n", tick);
    }
}

int Tetris::getRand() {
    int r = 0;
    QueueMutex.lock();
    if (!RandQueue.empty()) {
        r = RandQueue.front();
        RandQueue.pop();
    } else {
        display::d_wprintw(InfoField.getWin(), "Get random number failed\n");
    }
    QueueMutex.unlock();
    return r;
}

void Tetris::moveTetris(display::Tetrimino &t, char dir) {
    int offsetY, offsetX;
    switch (dir) {
        case 'a':
            offsetY = 0;
            offsetX = -2;
            break;
        case 'd':
            offsetY = 0;
            offsetX = 2;
            break;
        case 's':
            offsetY = DOWN_STEP;
            offsetX = 0;
            break;
        default:
            return;
    }

    auto checkRet = GGameField.hitCheck(offsetY, offsetX, t);
    if (dir == 's') {
        while (offsetY && checkRet != display::GameField::CHECK_OK) {
            --offsetY;
            checkRet = GGameField.hitCheck(offsetY, offsetX, t);
        }
    }
    if (checkRet == display::GameField::CHECK_OK) {
        t.move(GGameField.getWin(), offsetY, offsetX);
    }
}

void Tetris::rotateTetris(display::Tetrimino &t) {
    int y, x;
    CurTetris.getPos(y, x);
    int newDir = (CurTetrisDir + 1) % (int) CurTetrisList->size();
    display::Tetrimino newTetris = (*CurTetrisList)[newDir];
    newTetris.moveWithOutPrint(y, x);

    auto checkRet = GGameField.hitCheck(0, 0, newTetris);
    if (checkRet == display::GameField::CHECK_OK) {
        CurTetris.erase(GGameField.getWin(), false);

        CurTetrisDir = newDir;
        CurTetris = newTetris;
        CurTetris.show(GGameField.getWin());
    }
}
