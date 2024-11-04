cmake_minimum_required(VERSION 3.10)
project(rom-patcher)
add_executable(rom-patcher src/main.c)

set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu2x")
