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

# ── Fetch raygui (header-only) ────────────────────────────────────────────────
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raygui)

# ── Robot code (compiled into the HOST only, against the PROS shims) ──────────
set(WARBOT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/autons.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/warbot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/exampleSubsystem/example.cpp
)

# ══════════════════════════════════════════════════════════════════════════════
#  HOST  —  robot code + physics + control window + localhost table server
# ══════════════════════════════════════════════════════════════════════════════
add_executable(warbot_host
    ${CMAKE_CURRENT_SOURCE_DIR}/host_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/world.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer_panel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/net/table_server.cpp
    ${WARBOT_SOURCES}
)

# shims/ MUST come first to shadow include/api.h etc.
target_include_directories(warbot_host BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shims
)
target_include_directories(warbot_host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${raygui_SOURCE_DIR}/src
)
target_compile_definitions(warbot_host PRIVATE
    SIM=1
    _USE_MATH_DEFINES
    _PROS_API_H_
)
target_link_libraries(warbot_host PRIVATE raylib)
if(WIN32)
    target_link_libraries(warbot_host PRIVATE winmm ws2_32)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  VIEWER  —  read-only field view, table client (no robot code, no shims)
# ══════════════════════════════════════════════════════════════════════════════
add_executable(warbot_viewer
    ${CMAKE_CURRENT_SOURCE_DIR}/viewer_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer_field.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/net/table_client.cpp
)
target_include_directories(warbot_viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(warbot_viewer PRIVATE
    _USE_MATH_DEFINES
)
target_link_libraries(warbot_viewer PRIVATE raylib)
if(WIN32)
    target_link_libraries(warbot_viewer PRIVATE winmm ws2_32)
endif()

# ── Shared compiler warnings ──────────────────────────────────────────────────
foreach(tgt warbot_host warbot_viewer)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /W3 /wd4244 /wd4305)
    else()
        target_compile_options(${tgt} PRIVATE -Wall -Wno-unused-parameter)
    endif()
endforeach()
