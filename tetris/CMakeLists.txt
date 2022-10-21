add_executable(tetris main.cpp tetris.cpp display.cpp)

if (WIN32)
    target_include_directories(tetris PRIVATE ${NCURSES_INC_DIR})
    target_link_directories(tetris PRIVATE ${NCURSES_LIB_DIR})
endif()

target_link_libraries(tetris ncurses pthread)
