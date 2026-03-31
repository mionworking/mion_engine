cmake_minimum_required(VERSION 3.20)
project(mion_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

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

# --- Executável de stress test lógico/headless ---
add_executable(mion_stress tools/stress_main.cpp ${ENGINE_SOURCES})

target_include_directories(mion_stress PRIVATE src ${stb_SOURCE_DIR})

target_link_libraries(mion_stress PRIVATE SDL3::SDL3-static)

target_compile_options(mion_stress PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# --- Render stress test (SDL real, janela oculta) ---
add_executable(mion_render_stress tools/render_stress_main.cpp ${ENGINE_SOURCES})

target_include_directories(mion_render_stress PRIVATE src ${stb_SOURCE_DIR})

target_link_libraries(mion_render_stress PRIVATE SDL3::SDL3-static)

target_compile_options(mion_render_stress PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# --- Sprite bench: textura sintética, sem PNGs, varia atores × frames/anim ---
add_executable(mion_sprite_bench tools/sprite_bench_main.cpp)

target_include_directories(mion_sprite_bench PRIVATE src ${stb_SOURCE_DIR})

target_link_libraries(mion_sprite_bench PRIVATE SDL3::SDL3-static)

target_compile_options(mion_sprite_bench PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# --- Testes ---
option(MION_ENABLE_COVERAGE "Compilar mion_tests com --coverage (gcov/llvm-cov)" OFF)
option(MION_ENABLE_TEST_SANITIZERS "ASan+UBSan no alvo mion_tests (recomendado com Debug)" OFF)

enable_testing()
file(GLOB TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(mion_tests_legacy ${TEST_SOURCES} ${ENGINE_SOURCES})
    target_include_directories(mion_tests_legacy PRIVATE src ${stb_SOURCE_DIR})
    target_link_libraries(mion_tests_legacy PRIVATE SDL3::SDL3-static)
    add_test(NAME mion_tests_legacy COMMAND mion_tests_legacy)

    if(MION_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mion_tests_legacy PRIVATE --coverage -O0 -g)
        target_link_options(mion_tests_legacy PRIVATE --coverage)
    endif()
    if(MION_ENABLE_TEST_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mion_tests_legacy PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -g)
        target_link_options(mion_tests_legacy PRIVATE -fsanitize=address,undefined)
    endif()
endif()

file(GLOB TEST_SOURCES_V2 "tests_v2/*.cpp")
if(TEST_SOURCES_V2)
    set(TEST_SOURCES_BASE ${TEST_SOURCES})
    list(REMOVE_ITEM TEST_SOURCES_BASE "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp")

    set(V2_COMMON_SOURCES ${TEST_SOURCES_BASE} ${TEST_SOURCES_V2})

    set(V2_ALL_SOURCES ${V2_COMMON_SOURCES})
    list(REMOVE_ITEM V2_ALL_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_core_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_scenes_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_input_main.cpp")

    set(V2_CORE_SOURCES ${V2_COMMON_SOURCES})
    list(REMOVE_ITEM V2_CORE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_scenes_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_input_main.cpp")

    set(V2_SCENES_SOURCES ${V2_COMMON_SOURCES})
    list(REMOVE_ITEM V2_SCENES_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_core_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_input_main.cpp")

    set(V2_INPUT_SOURCES ${V2_COMMON_SOURCES})
    list(REMOVE_ITEM V2_INPUT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_core_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests_v2/test_v2_scenes_main.cpp")

    add_executable(mion_tests_v2 ${V2_ALL_SOURCES} ${ENGINE_SOURCES})
    target_include_directories(mion_tests_v2 PRIVATE src tests ${stb_SOURCE_DIR})
    target_link_libraries(mion_tests_v2 PRIVATE SDL3::SDL3-static)
    add_test(NAME mion_tests_v2 COMMAND mion_tests_v2)

    add_executable(mion_tests_v2_core ${V2_CORE_SOURCES} ${ENGINE_SOURCES})
    target_include_directories(mion_tests_v2_core PRIVATE src tests ${stb_SOURCE_DIR})
    target_link_libraries(mion_tests_v2_core PRIVATE SDL3::SDL3-static)
    add_test(NAME mion_tests_v2_core COMMAND mion_tests_v2_core)

    add_executable(mion_tests_v2_scenes ${V2_SCENES_SOURCES} ${ENGINE_SOURCES})
    target_include_directories(mion_tests_v2_scenes PRIVATE src tests ${stb_SOURCE_DIR})
    target_link_libraries(mion_tests_v2_scenes PRIVATE SDL3::SDL3-static)
    add_test(NAME mion_tests_v2_scenes COMMAND mion_tests_v2_scenes)

    add_executable(mion_tests_v2_input ${V2_INPUT_SOURCES} ${ENGINE_SOURCES})
    target_include_directories(mion_tests_v2_input PRIVATE src tests ${stb_SOURCE_DIR})
    target_link_libraries(mion_tests_v2_input PRIVATE SDL3::SDL3-static)
    add_test(NAME mion_tests_v2_input COMMAND mion_tests_v2_input)

    if(MION_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        foreach(_t mion_tests_v2 mion_tests_v2_core mion_tests_v2_scenes mion_tests_v2_input)
            target_compile_options(${_t} PRIVATE --coverage -O0 -g)
            target_link_options(${_t} PRIVATE --coverage)
        endforeach()
    endif()
    if(MION_ENABLE_TEST_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        foreach(_t mion_tests_v2 mion_tests_v2_core mion_tests_v2_scenes mion_tests_v2_input)
            target_compile_options(${_t} PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -g)
            target_link_options(${_t} PRIVATE -fsanitize=address,undefined)
        endforeach()
    endif()
endif()
