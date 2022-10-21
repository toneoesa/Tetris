#ifndef TETRIS_DISPLAY_H
#define TETRIS_DISPLAY_H

#include <vector>
#include <deque>

namespace display {
    // ================================================== variables
    enum Color {
        INVALID_COLOR = -1,
        PURE_BLACK = 0,
        PURE_RED,
        PURE_GREEN,
        PURE_YELLOW,
        PURE_BLUE,
        PURE_MAGENTA,
        PURE_CYAN,
        PURE_WHITE,
        PURE_COLOR_NUM
    };

    enum RET_CODE {
        DIS_OK = 0,
        DIS_NO_COLOR,
        DIS_ERR_CREATE_WIN,
        DIS_ERR
    };

    const char * const RET_INFO[] = {
            "OK",
            "Your terminal does not support color",
            "Can not create window",
            "Undefined Error"
    };

    extern const int GETCH_ERR;

    // ================================================== functions
    RET_CODE initDisplay();
    void destroyDisplay();
    void getMaxYX(int &y, int &x);
    void *getGlobalWin();
    int d_wgetchar(void *win);
    int d_wprintw(void *win, const char *fmt, ...);
    int d_wmove(void *win, int y, int x);
    void d_wrefresh(void *win);
    void nodelay(void *win, bool enable);

    // ================================================== class Tetrimino
    class Tetrimino {
    public:
        typedef unsigned int map_t; // store 4x8 tetrimino in 32 bit
        const static int HEIGHT = 4;
        const static int WIDTH = 8; // fix terminal character width

        Tetrimino() = default;
        Tetrimino(map_t m, Color c);
        Tetrimino(int y, int x, map_t m, Color c);

        bool exist(int y, int x) const;
        map_t getMap() const;
        Color getColor() const;
        void getPos(int &y, int &x) const;
        int getY() const;
        int getX() const;

        void show(void *win, bool refreshNow = true);
        void erase(void *win, bool refreshNow = true);
        void moveTo(void *win, int newY, int newX, bool refreshNow = true);
        void move(void *win, int offsetY, int offsetX, bool refreshNow = true);
        void moveWin(void *newWin, void *oldWin, int newY, int newX, bool refreshNow = true);
        void moveWithOutPrint(int newY, int newX);


    protected:
        bool isShowed = false;
        Color color = PURE_BLACK;
        int topLeftY = 0;
        int topLeftX = 0;
        map_t shapeMap = 0;
    };

    const std::vector<Tetrimino> I = {
            Tetrimino{0b00000000111111110000000000000000, PURE_CYAN},
            Tetrimino{0b00110000001100000011000000110000, PURE_CYAN}
    };
    const std::vector<Tetrimino> L = {
            Tetrimino{0b00000000000011110000110000001100, PURE_BLUE},
            Tetrimino{0b00000000000000000011111100000011, PURE_BLUE},
            Tetrimino{0b00000000000000110000001100001111, PURE_BLUE},
            Tetrimino{0b00000000000000000011000000111111, PURE_BLUE}
    };
    const std::vector<Tetrimino> J = {
            Tetrimino{0b00000000000011110000001100000011, PURE_WHITE},
            Tetrimino{0b00000000000000000000001100111111, PURE_WHITE},
            Tetrimino{0b00000000000011000000110000001111, PURE_WHITE},
            Tetrimino{0b00000000000000000011111100110000, PURE_WHITE}
    };
    const std::vector<Tetrimino> O = {
            Tetrimino{0b00000000000000000000111100001111, PURE_YELLOW}
    };
    const std::vector<Tetrimino> S = {
            Tetrimino{0b00000000000011110011110000000000, PURE_GREEN},
            Tetrimino{0b00000000000011000000111100000011, PURE_GREEN}
    };
    const std::vector<Tetrimino> T = {
            Tetrimino{0b00000000000000000011111100001100, PURE_MAGENTA},
            Tetrimino{0b00000000000000110000111100000011, PURE_MAGENTA},
            Tetrimino{0b00000000000011000011111100000000, PURE_MAGENTA},
            Tetrimino{0b00000000000011000000111100001100, PURE_MAGENTA}
    };
    const std::vector<Tetrimino> Z = {
            Tetrimino{0b00000000001111000000111100000000, PURE_RED},
            Tetrimino{0b00000000000000110000111100001100, PURE_RED}
    };

    // ================================================== class Field
    class Field {
    public:
        Field() = default;
        Field(int y, int x, int h, int w);
        ~Field();

        RET_CODE startWin(int y, int x, int h, int w);
        RET_CODE startWin();
        void endWin();
        void refreshWin();
        void *getWin() const;
        void getHW(int &h, int &w) const;
        void getYX(int &y, int &x) const;
        void getInnerHW(int &h, int &w) const;
        void getInnerYX(int &y, int &x) const;
        void enScroll(bool enable);
        void moveTetrisToCenter(Tetrimino &t, bool refreshNow = true);

    protected:
        void *win = nullptr;
        void *subWin = nullptr;
        int topLeftY = 0;
        int topLeftX = 0;
        int height = 0;
        int width = 0;
        // sub window
        int iTopLeftY = 0;
        int iTopLeftX = 0;
        int iHeight = 0;
        int iWidth = 0;
    };

    // ================================================== class GameField
    class GameField : public Field {
    public:
        typedef unsigned char check_res_t;
        const static check_res_t CHECK_OK = 0;
        const static check_res_t CHECK_HIT = 1;
        const static check_res_t CHECK_OUT = 2;

        GameField() = default;
        GameField(int y, int x, int h, int w);

        RET_CODE startWin(int y, int x, int h, int w);

        Color getColor(int y, int x) const;
        check_res_t hitCheck(int offsetY, int offsetX, const Tetrimino &t, bool includeTop = false) const;

        void moveTetrisToStartPoint(Tetrimino &t, bool refreshNow = true);

        void hideLine(int line, bool hide = true, bool refreshNow = true);
        void fall(int *lines, int lineNum, bool refreshNow = true);
        void add(const Tetrimino &t, bool needPrint = false, bool refreshNow = false);
        int checkComplete(const Tetrimino &t, int *lineList);

    protected:
        void initMap();
        void print();
        void erase();

    protected:
        std::deque<std::vector<Color>> map;
    };
}

#endif //TETRIS_DISPLAY_H
