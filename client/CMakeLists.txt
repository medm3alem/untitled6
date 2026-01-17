cmake_minimum_required(VERSION 3.16)
project(TetrisClient)

set(CMAKE_CXX_STANDARD 17)

find_package(raylib REQUIRED)

add_executable(tetris
        main.cpp
        network.cpp
)

target_include_directories(tetris PRIVATE .)

target_link_libraries(tetris
        raylib
        pthread
)
