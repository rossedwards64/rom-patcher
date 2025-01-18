cmake_minimum_required(VERSION 3.10)

project(
  rom-patcher
  DESCRIPTION "A tool for patching ROMs."
  LANGUAGES C)

set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions($<$<CONFIG:DEBUG>:DEBUG>)
add_compile_options(
  -Wall
  -Wpedantic
  -Werror
  -Wextra
  -Wshadow
  -Wpointer-arith
  -Wcast-align
  -Wstrict-prototypes
  -Wwrite-strings
  -Wcast-qual
  -Wswitch-default
  -Wswitch-enum
  -Wunreachable-code
  -Wuninitialized
  -Winit-self
  $<$<CONFIG:DEBUG>:-fsanitize=address>
  $<$<CONFIG:DEBUG>:-fsanitize=undefined>
  $<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>
  $<$<CONFIG:DEBUG>:-g3>)

add_executable(rom-patcher src/main.c)
target_sources(rom-patcher PRIVATE src/patchers/bps.c src/patchers/ips.c
                                   src/patchers/error.h)
