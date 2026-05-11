cmake_minimum_required(VERSION 3.10)
project(GoldenAxe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(lib/sdl/include)
include_directories(lib/sdl_image/include)

link_directories(lib/sdl/lib)
link_directories(lib/sdl_image/lib)

add_executable(GoldenAxe
        main.cpp
        bagel.h
        GoldenAxe.h
)

target_link_libraries(GoldenAxe PRIVATE SDL3 SDL3_image)

file(GLOB SDL_DLLS "${CMAKE_SOURCE_DIR}/lib/sdl/bin/*.dll")
file(GLOB SDL_IMAGE_DLLS "${CMAKE_SOURCE_DIR}/lib/sdl_image/bin/*.dll")
file(COPY ${SDL_DLLS} ${SDL_IMAGE_DLLS} DESTINATION ${CMAKE_BINARY_DIR})