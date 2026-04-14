#include "test_common.hpp"

#include "scenes/credits_scene.hpp"

static void test_credits_scene_backspace_returns_title() {
    mion::CreditsScene s;
    s.enter();
    mion::InputState in;
    in.ui_cancel_pressed = true;
    s.fixed_update(0.016f, in);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kTitle);
}

static void test_credits_scene_auto_returns_after_scroll() {
    mion::CreditsScene s;
    s.viewport_h = 200;
    s.enter();
    mion::InputState in;
    for (int i = 0; i < 4000 && s.next_scene() == mion::SceneId::kNone; ++i)
        s.fixed_update(0.016f, in);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kTitle);
}

void run_credits_scene_tests() {
    run("Credits.BackToTitle", test_credits_scene_backspace_returns_title);
    run("Credits.AutoReturn", test_credits_scene_auto_returns_after_scroll);
}
