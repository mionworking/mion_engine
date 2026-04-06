#include "test_common.hpp"

#include <cstring>

#include "scenes/credits_scene.hpp"

static void test_credits_scene_backspace_returns_title() {
    mion::CreditsScene s;
    s.enter();
    mion::InputState in;
    in.ui_cancel_pressed = true;
    s.fixed_update(0.016f, in);
    EXPECT_EQ(std::strcmp(s.next_scene(), "title"), 0);
}

static void test_credits_scene_auto_returns_after_scroll() {
    mion::CreditsScene s;
    s.viewport_h = 200;
    s.enter();
    mion::InputState in;
    for (int i = 0; i < 4000 && std::strcmp(s.next_scene(), "") == 0; ++i)
        s.fixed_update(0.016f, in);
    EXPECT_EQ(std::strcmp(s.next_scene(), "title"), 0);
}

void run_credits_scene_tests() {
    run("Credits.BackToTitle", test_credits_scene_backspace_returns_title);
    run("Credits.AutoReturn", test_credits_scene_auto_returns_after_scroll);
}
