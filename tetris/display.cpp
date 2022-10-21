#include "display.h"
#include <ncursesw/ncurses.h>
#include <algorithm>
#include <functional>

using namespace display;

// ================================================== local functions
static inline WINDOW *W(void *ptr) {
    return (WINDOW *) ptr;
}

static inline bool inWin(void *win, int y, int x) {
    int maxY, maxX;
    getmaxyx(W(win), maxY, maxX);
    return y >= 0 && y < maxY && x >= 0 && x <= maxX;
}

// ================================================== variables
const int display::GETCH_ERR = ERR;

// ================================================== functions
RET_CODE display::initDisplay() {
    initscr();
    if (!has_colors()) {
        endwin();
        return DIS_NO_COLOR;
    }
    noecho();
    cbreak();
    start_color();
    init_pair(PURE_BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(PURE_RED, COLOR_RED, COLOR_RED);
    init_pair(PURE_GREEN, COLOR_GREEN, COLOR_GREEN);
    init_pair(PURE_YELLOW, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(PURE_BLUE, COLOR_BLUE, COLOR_BLUE);
    init_pair(PURE_MAGENTA, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(PURE_CYAN, COLOR_CYAN, COLOR_CYAN);
    init_pair(PURE_WHITE, COLOR_WHITE, COLOR_WHITE);
    refresh();
    return DIS_OK;
}

void display::destroyDisplay() {
    endwin();
}

void display::getMaxYX(int &y, int &x) {
    getmaxyx(stdscr, y, x);
}

void *display::getGlobalWin() {
    return stdscr;
}

int display::d_wgetchar(void *win) {
    return wgetch(W(win));
}

int display::d_wprintw(void *win, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vw_printw(W(win), fmt, args);
    va_end(args);
    return r;
}

int display::d_wmove(void *win, int y, int x) {
    return wmove(W(win), y, x);
}

void display::d_wrefresh(void *win) {
    wrefresh(W(win));
}

void display::nodelay(void *win, bool enable) {
    nodelay(W(win), enable);
}

// ================================================== class Tetrimino
const int Tetrimino::HEIGHT;
const int Tetrimino::WIDTH;

Tetrimino::Tetrimino(Tetrimino::map_t m, Color c) : shapeMap(m), color(c) {}

Tetrimino::Tetrimino(int y, int x, map_t m, Color c) : topLeftY(y), topLeftX(x), shapeMap(m), color(c) {}

bool Tetrimino::exist(int y, int x) const {
    if (y < 0 || y >= HEIGHT || x < 0 || x >= WIDTH) return false;
    return shapeMap >> (y * WIDTH + x) & 1;
}

Tetrimino::map_t Tetrimino::getMap() const {
    return shapeMap;
}

Color Tetrimino::getColor() const {
    return color;
}

void Tetrimino::getPos(int &y, int &x) const {
    y = topLeftY;
    x = topLeftX;
}

int Tetrimino::getY() const {
    return topLeftY;
}

int Tetrimino::getX() const {
    return topLeftX;
}

void Tetrimino::show(void *win, bool refreshNow) {
    isShowed = true;

    wattrset(W(win), COLOR_PAIR(color));
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (exist(i, j) && inWin(W(win), topLeftY + i, topLeftX + j)) {
                mvwaddch(W(win), topLeftY + i, topLeftX + j, ' ');
            }
        }
    }
    wattroff(W(win), COLOR_PAIR(color));
    if (refreshNow) wrefresh(W(win));
}

void Tetrimino::erase(void *win, bool refreshNow) {
    if (!isShowed) return;
    isShowed = false;

    wattrset(W(win), A_NORMAL);
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (exist(i, j) && inWin(W(win), topLeftY + i, topLeftX + j)) {
                mvwaddch(W(win), topLeftY + i, topLeftX + j, ' ');
            }
        }
    }
    if (refreshNow) wrefresh(W(win));
}

void Tetrimino::moveTo(void *win, int newY, int newX, bool refreshNow) {
    erase(win, false);
    topLeftY = newY;
    topLeftX = newX;
    show(win, false);
    if (refreshNow) wrefresh(W(win));
}

void Tetrimino::move(void *win, int offsetY, int offsetX, bool refreshNow) {
    if (offsetY || offsetX) {
        moveTo(win, topLeftY + offsetY, topLeftX + offsetX, refreshNow);
    }
}

void Tetrimino::moveWin(void *newWin, void *oldWin, int newY, int newX, bool refreshNow) {
    erase(oldWin, refreshNow);
    topLeftY = newY;
    topLeftX = newX;
    show(newWin, refreshNow);
}

void Tetrimino::moveWithOutPrint(int newY, int newX) {
    topLeftY = newY;
    topLeftX = newX;
}

// ================================================== class Field
Field::Field(int y, int x, int h, int w) : topLeftY(y), topLeftX(x), height(h), width(w),
                                           iTopLeftY(y + 1), iTopLeftX(x + 1), iHeight(h - 2), iWidth(w - 2) {}

Field::~Field() {
    endWin();
}

RET_CODE Field::startWin() {
    if (height < 3 || width < 3) {
        return DIS_ERR_CREATE_WIN;
    }
    win = newwin(height, width, topLeftY, topLeftX);
    if (!win) {
        return DIS_ERR_CREATE_WIN;
    }
    subWin = subwin(W(win), iHeight, iWidth, iTopLeftY, iTopLeftX);
    if (!subWin) {
        return DIS_ERR_CREATE_WIN;
    }

    // there is nothing to write on win directly after box created.
    // otherwise we need touchwin(win) before wrefresh(subWin)
    box(W(win), 0, 0);
    wrefresh(W(win));
    return DIS_OK;
}

void Field::endWin() {
    if (subWin) {
        delwin(W(subWin));
        subWin = nullptr;
    }
    if (win) {
        wattrset(W(win), A_NORMAL);
        wborder(W(win), ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        wrefresh(W(win));
        delwin(W(win));
        win = nullptr;
    }
}

void Field::refreshWin() {
    wrefresh(W(subWin));
}

void *Field::getWin() const {
    return subWin;
}

void Field::getHW(int &h, int &w) const {
    h = height;
    w = width;
}

void Field::getYX(int &y, int &x) const {
    y = topLeftY;
    x = topLeftX;
}

void Field::getInnerHW(int &h, int &w) const {
    h = iHeight;
    w = iWidth;
}

void Field::getInnerYX(int &y, int &x) const {
    y = iTopLeftY;
    x = iTopLeftX;
}

void Field::enScroll(bool enable) {
    scrollok(W(subWin), enable);
}

RET_CODE Field::startWin(int y, int x, int h, int w) {
    topLeftY = y;
    topLeftX = x;
    height = h;
    width = w;
    iTopLeftY = y + 1;
    iTopLeftX = x + 1;
    iHeight = h - 2;
    iWidth = w - 2;
    return startWin();
}

void Field::moveTetrisToCenter(Tetrimino &t, bool refreshNow) {
    t.moveTo(subWin, (iHeight - Tetrimino::HEIGHT) / 2, (iWidth - Tetrimino::WIDTH) / 2, refreshNow);
}

// ================================================== class GameField
const GameField::check_res_t GameField::CHECK_OK;
const GameField::check_res_t GameField::CHECK_HIT;
const GameField::check_res_t GameField::CHECK_OUT;

GameField::GameField(int y, int x, int h, int w) : Field(y, x, h, w) {
    initMap();
}

RET_CODE GameField::startWin(int y, int x, int h, int w) {
    if (w % 2) return DIS_ERR_CREATE_WIN;
    RET_CODE ret = Field::startWin(y, x, h, w);
    initMap();
    return ret;
}

Color GameField::getColor(int y, int x) const {
    if (y >= 0 && y < iHeight && x >= 0 && x < iWidth) return map[y][x];
    return INVALID_COLOR;
}

GameField::check_res_t GameField::hitCheck(int offsetY, int offsetX, const Tetrimino &t, bool includeTop) const {
    check_res_t result = CHECK_OK;
    int y, x;
    t.getPos(y, x);
    int h = Tetrimino::HEIGHT;
    int w = Tetrimino::WIDTH;
    auto tMap = t.getMap();

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            if (tMap >> (i * w + j) & 1) {
                int cy = y + offsetY + i;
                int cx = x + offsetX + j;
                if ((includeTop && cy < 0) || cy >= iHeight || cx < 0 || cx >= iWidth) {
                    result |= CHECK_OUT;
                }
                if (getColor(cy, cx) != INVALID_COLOR) {
                    result |= CHECK_HIT;
                }
            }
        }
    }
    return result;
}

void GameField::moveTetrisToStartPoint(Tetrimino &t, bool refreshNow) {
    t.moveTo(subWin, iTopLeftY - Tetrimino::HEIGHT, iTopLeftX + (iWidth - Tetrimino::WIDTH) / 2, refreshNow);
}


void GameField::hideLine(int line, bool hide, bool refreshNow) {
    if (line < 0 || line >= iHeight) return;

    if (hide) wattrset(W(subWin), A_NORMAL);
    for (int j = 0; j < iWidth; ++j) {
        if (map[line][j] != INVALID_COLOR) {
            if (!hide) wattrset(W(subWin), COLOR_PAIR(map[line][j]));
            mvwaddch(W(subWin), line, j, ' ');
        }
    }
    if (refreshNow) wrefresh(W(subWin));
}

void GameField::fall(int *lines, int lineNum, bool refreshNow) {
    erase();

    std::sort(lines, lines + lineNum, std::greater<>());
    for (int i = 0; i < lineNum; ++i) {
        map.erase(map.begin() + lines[i]);
    }
    for (int i = 0; i < lineNum; ++i) {
        map.emplace_front(iWidth, INVALID_COLOR);
    }

    print();
    if (refreshNow) wrefresh(W(subWin));
}

void GameField::add(const Tetrimino &t, bool needPrint, bool refreshNow) {
    Tetrimino::map_t tMap = t.getMap();
    Color color = t.getColor();
    int y, x;
    t.getPos(y, x);
    int h = Tetrimino::HEIGHT;
    int w = Tetrimino::WIDTH;

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            if (tMap >> (i * w + j) & 1) {
                map[y + i][x + j] = color;
            }
        }
    }
    if (needPrint) {
        print();
        if (refreshNow) wrefresh(W(subWin));
    }
}

int GameField::checkComplete(const Tetrimino &t, int *lineList) {
    int res = 0;
    for (int i = Tetrimino::HEIGHT - 1; i >= 0; --i) {
        bool flag = false; // there is a part in this line in tetrimino
        for (int j = 0; j < Tetrimino::WIDTH; ++j) {
            if (t.exist(i, j)) {
                flag = true;
                break;
            }
        }
        if (!flag) continue;

        bool complete = true; // this line is completed
        for (int j = 0; j < iWidth; ++j) {
            if (getColor(t.getY() + i, j) == INVALID_COLOR) {
                complete = false;
                break;
            }
        }
        if (complete) lineList[res++] = t.getY() + i;
    }
    return res;
}

void GameField::initMap() {
    map.resize(iHeight);
    for (auto &m: map) {
        m.resize(iWidth, INVALID_COLOR);
    }
}

void GameField::print() {
    for (int i = 0; i < iHeight; ++i) {
        for (int j = 0; j < iWidth; ++j) {
            if (map[i][j] != INVALID_COLOR) {
                wattrset(W(subWin), COLOR_PAIR(map[i][j]));
                mvwaddch(W(subWin), i, j, ' ');
            }
        }
    }
}

void GameField::erase() {
    wattrset(W(subWin), A_NORMAL);
    for (int i = 0; i < iHeight; ++i) {
        for (int j = 0; j < iWidth; ++j) {
            if (map[i][j] != INVALID_COLOR) {
                mvwaddch(W(subWin), i, j, ' ');
            }
        }
    }
}
