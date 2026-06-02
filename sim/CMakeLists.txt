cmake_minimum_required(VERSION 3.20)
project(warbot_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Fetch raylib ──────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
set(BUILD_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# ── Sources ───────────────────────────────────────────────────────────────────
set(WARBOT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/autons.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/warbot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/exampleSubsystem/example.cpp
)

set(SIM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main_sim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/world.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp
)

add_executable(warbot_sim ${SIM_SOURCES} ${WARBOT_SOURCES})

# ── Include paths — shims/ MUST come first to shadow include/api.h etc. ──────
target_include_directories(warbot_sim BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shims        # shims/main.h, shims/api.h shadow real ones
)

target_include_directories(warbot_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include   # warbotTemplate/, autons.h, auton_selector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src       # exampleSubsystem/example.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}             # sim/world.hpp, sim/config.hpp, etc.
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(warbot_sim PRIVATE
    SIM=1
    _USE_MATH_DEFINES   # required on MSVC for M_PI
    _PROS_API_H_        # blocks include/api.h; our shim/api.h (pragma once only) still runs
)

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(warbot_sim PRIVATE raylib)

if(WIN32)
    target_link_libraries(warbot_sim PRIVATE winmm)
endif()

if(MSVC)
    target_compile_options(warbot_sim PRIVATE /W3 /wd4244 /wd4305)
else()
    target_compile_options(warbot_sim PRIVATE -Wall -Wno-unused-parameter)
endif()
