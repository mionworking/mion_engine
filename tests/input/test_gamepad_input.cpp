#include "test_common.hpp"

#include <SDL3/SDL.h>

#include "core/input.hpp"

static void test_gamepad_default_disconnected_state() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }

    mion::GamepadInputSource g;
    EXPECT_FALSE(g.is_connected());
    mion::InputState s = g.read_state();
    EXPECT_NEAR(s.move_x, 0.0f, 0.0001f);
    EXPECT_NEAR(s.move_y, 0.0f, 0.0001f);
    EXPECT_FALSE(s.attack_pressed);
    EXPECT_FALSE(s.pause_pressed);
    SDL_Quit();
}

static void test_gamepad_try_connect_and_read_safe() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }

    mion::GamepadInputSource g;
    g.try_connect();
    (void)g.is_connected();
    mion::InputState s = g.read_state();
    EXPECT_TRUE(s.move_x >= -1.0f && s.move_x <= 1.0f);
    EXPECT_TRUE(s.move_y >= -1.0f && s.move_y <= 1.0f);
    SDL_Quit();
}

void run_gamepad_input_tests() {
    run("Gamepad.DefaultState", test_gamepad_default_disconnected_state);
    run("Gamepad.TryConnectRead", test_gamepad_try_connect_and_read_safe);
}
