cmake_minimum_required(VERSION 3.10)

project(GoldenAxe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-static")

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_subdirectory(box2d/box2d-3.1.1)

add_executable(GoldenAxe
        main.cpp
        bagel.h
        GoldenAxe.h
        GoldenAxe.cpp
)

target_include_directories(GoldenAxe PRIVATE
        box2d/box2d-3.1.1/include
)

target_link_libraries(GoldenAxe
        SDL3::SDL3-static
        SDL3_image::SDL3_image-static
        box2d
)
