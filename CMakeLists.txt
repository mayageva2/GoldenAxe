cmake_minimum_required(VERSION 3.10)
project(GoldenAxe)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        "box2d/box2d-3.1.1/include"
        "lib/sdl/include"
        "lib/sdl_image/include"
)

add_executable(GoldenAxe
        main.cpp
        GoldenAxe.cpp
        GoldenAxe.h
        bagel.h
)

add_subdirectory("box2d/box2d-3.1.1")

target_link_libraries(GoldenAxe
        box2d
        "${CMAKE_SOURCE_DIR}/lib/sdl/lib/libSDL3.dll.a"
        "${CMAKE_SOURCE_DIR}/lib/sdl_image/lib/libSDL3_image.dll.a"
)

file(COPY external/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/external)