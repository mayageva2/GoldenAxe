cmake_minimum_required(VERSION 3.10)

project(GoldenAxe)

set(CMAKE_CXX_STANDARD 17)

# ----------------------------
# Include directories
# ----------------------------

include_directories(
        "box2d/box2d-3.1.1/include"

        "lib/SDL3-3.4.8/x86_64-w64-mingw32/include"
        "lib/SDL3_image-3.4.4/x86_64-w64-mingw32/include"
)

# ----------------------------
# Executable
# ----------------------------

add_executable(GoldenAxe
        main.cpp
        GoldenAxe.cpp
        GoldenAxe.h
        bagel.h
)

# ----------------------------
# Box2D
# ----------------------------

add_subdirectory("box2d/box2d-3.1.1")

# ----------------------------
# Link libraries
# ----------------------------

target_link_libraries(GoldenAxe
        box2d

        "${CMAKE_SOURCE_DIR}/lib/SDL3-3.4.8/x86_64-w64-mingw32/lib/libSDL3.dll.a"

        "${CMAKE_SOURCE_DIR}/lib/SDL3_image-3.4.4/x86_64-w64-mingw32/lib/libSDL3_image.dll.a"
)

# ----------------------------
# Copy SDL runtime DLLs
# ----------------------------

file(COPY
        "${CMAKE_SOURCE_DIR}/lib/SDL3-3.4.8/x86_64-w64-mingw32/bin/SDL3.dll"
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

file(COPY
        "${CMAKE_SOURCE_DIR}/lib/SDL3_image-3.4.4/x86_64-w64-mingw32/bin/SDL3_image.dll"
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

# ----------------------------
# Copy external assets
# ----------------------------

file(COPY external/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/external)