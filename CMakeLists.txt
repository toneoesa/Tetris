cmake_minimum_required(VERSION 3.18)
project(Tetris3)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    if (NOT DEFINED NCURSES_INC_DIR)
        set(NCURSES_INC_DIR "C:\\ProgramFiles\\mingw-w64\\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\\opt\\include")
    endif ()
    if (NOT DEFINED NCURSES_LIB_DIR)
        set(NCURSES_LIB_DIR "C:\\ProgramFiles\\mingw-w64\\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\\opt\\lib")
    endif ()
endif ()

add_subdirectory(tetris)
