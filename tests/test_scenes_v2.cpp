#include "test_common.hpp"

#include <cstring>

#include "core/register_scenes.hpp"
#include "core/save_system.hpp"
#include "scenes/game_over_scene.hpp"
#include "scenes/town_scene.hpp"
#include "scenes/victory_scene.hpp"

static void test_scene_registry_creates_new_endgame_scenes() {
    mion::SceneRegistry reg;
    mion::register_default_scenes(reg);
    mion::SceneCreateContext ctx;
    ctx.viewport_w = 960;
    ctx.viewport_h = 540;
    EXPECT_TRUE(reg.create("game_over", ctx) != nullptr);
    EXPECT_TRUE(reg.create("victory", ctx) != nullptr);
}

static void test_town_scene_pause_quit_to_title() {
    mion::TownScene s;
    s.enter();

    mion::InputState esc_on;
    esc_on.pause_pressed = true;
    s.fixed_update(0.016f, esc_on);

    mion::InputState esc_off;
    s.fixed_update(0.016f, esc_off);

    mion::InputState down_on;
    down_on.ui_down_pressed = true;
    for (int i = 0; i < 2; ++i) {
        s.fixed_update(0.016f, down_on);
        s.fixed_update(0.016f, esc_off);
    }

    mion::InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);

    const char* next = s.next_scene();
    EXPECT_TRUE(next != nullptr);
    if (next)
        EXPECT_EQ(std::strcmp(next, "title"), 0);
}

static void test_town_scene_stable_without_transition_on_idle_ticks() {
    mion::TownScene s;
    s.enter();
    mion::InputState idle;
    for (int i = 0; i < 240; ++i)
        s.fixed_update(1.0f / 60.0f, idle);
    const char* next = s.next_scene();
    EXPECT_TRUE(next == nullptr || std::strcmp(next, "") == 0);
}

static void test_town_scene_npc_wander_stable() {
    mion::TownScene s;
    s.enter();
    mion::InputState idle;
    for (int i = 0; i < 600; ++i)
        s.fixed_update(1.0f / 60.0f, idle);
    const char* next = s.next_scene();
    EXPECT_TRUE(next == nullptr || std::strcmp(next, "") == 0);
}

static void test_victory_scene_continue_and_quit_paths() {
    mion::VictoryScene s;
    s.enter();

    mion::InputState wait;
    s.fixed_update(1.1f, wait);

    mion::InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(std::strcmp(s.next_scene(), "town"), 0);

    s.enter();
    s.fixed_update(1.1f, wait);
    mion::InputState down_on;
    down_on.ui_down_pressed = true;
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(std::strcmp(s.next_scene(), "__quit__"), 0);
}

static void test_game_over_scene_retry_title_and_quit() {
    mion::GameOverScene s;
    s.enter();
    mion::InputState wait;
    s.fixed_update(0.9f, wait);

    mion::InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(std::strcmp(s.next_scene(), "title"), 0);

    s.enter();
    s.fixed_update(0.9f, wait);
    mion::InputState down_on;
    down_on.ui_down_pressed = true;
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(std::strcmp(s.next_scene(), "__quit__"), 0);
}

void run_scenes_v2_tests() {
    run("V2.SceneRegistry.EndgameScenes", test_scene_registry_creates_new_endgame_scenes);
    run("V2.TownScene.PauseQuitTitle", test_town_scene_pause_quit_to_title);
    run("V2.TownScene.IdleStable", test_town_scene_stable_without_transition_on_idle_ticks);
    run("V2.TownScene.NpcWanderStable", test_town_scene_npc_wander_stable);
    run("V2.VictoryScene.Flow", test_victory_scene_continue_and_quit_paths);
    run("V2.GameOverScene.Flow", test_game_over_scene_retry_title_and_quit);
}
