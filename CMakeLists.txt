cmake_minimum_required(VERSION 3.20)
project(mion_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- SDL3 via FetchContent (sem necessidade de instalar no sistema) ---
include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.10
    GIT_SHALLOW    TRUE
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

# --- stb (single-header libs) — apenas stb_image.h, sem submodules ---
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(stb)

# --- Engine sources ---
file(GLOB_RECURSE ENGINE_SOURCES
    "src/core/*.cpp"
    "src/components/*.cpp"
    "src/systems/*.cpp"
    "src/world/*.cpp"
    "src/entities/*.cpp"
    "src/vendor/*.cpp"
)

# --- Executável principal ---
add_executable(mion_engine src/main.cpp ${ENGINE_SOURCES})

target_include_directories(mion_engine PRIVATE src ${stb_SOURCE_DIR})

target_link_libraries(mion_engine PRIVATE SDL3::SDL3-static)

target_compile_options(mion_engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# --- Testes ---
enable_testing()
file(GLOB TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(mion_tests ${TEST_SOURCES} ${ENGINE_SOURCES})
    target_include_directories(mion_tests PRIVATE src ${stb_SOURCE_DIR})
    target_link_libraries(mion_tests PRIVATE SDL3::SDL3-static)
    add_test(NAME mion_tests COMMAND mion_tests)
endif()
