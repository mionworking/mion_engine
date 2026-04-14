#include "test_common.hpp"

#include "core/register_scenes.hpp"
#include "core/save_system.hpp"
#include "scenes/game_over_scene.hpp"
#include "scenes/world_scene.hpp"
#include "scenes/victory_scene.hpp"

static void test_scene_registry_creates_new_endgame_scenes() {
    mion::SceneRegistry reg;
    mion::register_default_scenes(reg);
    mion::SceneCreateContext ctx;
    ctx.viewport_w = 960;
    ctx.viewport_h = 540;
    EXPECT_TRUE(reg.create(mion::SceneId::kGameOver, ctx) != nullptr);
    EXPECT_TRUE(reg.create(mion::SceneId::kVictory, ctx) != nullptr);
}

static void test_world_scene_pause_quit_to_title() {
    mion::SaveSystem::remove_default_saves();
    mion::WorldScene s;
    s.enter();

    mion::InputState esc_on;
    esc_on.pause_pressed = true;
    s.fixed_update(0.016f, esc_on);

    mion::InputState esc_off;
    s.fixed_update(0.016f, esc_off);

    mion::InputState down_on;
    down_on.ui_down_pressed = true;
    for (int i = 0; i < 3; ++i) {
        s.fixed_update(0.016f, down_on);
        s.fixed_update(0.016f, esc_off);
    }

    mion::InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);

    mion::SceneId next = s.next_scene();
    EXPECT_EQ(next, mion::SceneId::kTitle);
    mion::SaveSystem::remove_default_saves();
}

static void test_world_scene_stable_without_transition_on_idle_ticks() {
    mion::SaveSystem::remove_default_saves();
    mion::WorldScene s;
    s.enter();
    mion::InputState idle;
    for (int i = 0; i < 240; ++i)
        s.fixed_update(1.0f / 60.0f, idle);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kNone);
    mion::SaveSystem::remove_default_saves();
}

static void test_world_scene_npc_wander_stable() {
    mion::SaveSystem::remove_default_saves();
    mion::WorldScene s;
    s.enter();
    mion::InputState idle;
    for (int i = 0; i < 600; ++i)
        s.fixed_update(1.0f / 60.0f, idle);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kNone);
    mion::SaveSystem::remove_default_saves();
}

static void test_victory_scene_continue_and_quit_paths() {
    mion::VictoryScene s;
    s.enter();

    mion::InputState wait;
    s.fixed_update(1.1f, wait);

    mion::InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kWorld);

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
    EXPECT_EQ(s.next_scene(), mion::SceneId::kQuit);
}

static void test_game_over_scene_retry_title_and_quit() {
    mion::GameOverScene s;
    s.enter();
    mion::InputState wait;
    s.fixed_update(0.9f, wait);

    mion::InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kTitle);

    s.enter();
    s.fixed_update(0.9f, wait);
    mion::InputState down_on;
    down_on.ui_down_pressed = true;
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, down_on);
    s.fixed_update(0.016f, wait);
    s.fixed_update(0.016f, ok_on);
    EXPECT_EQ(s.next_scene(), mion::SceneId::kQuit);
}

void run_scenes_v2_tests() {
    run("V2.SceneRegistry.EndgameScenes", test_scene_registry_creates_new_endgame_scenes);
    run("V2.WorldScene.PauseQuitTitle", test_world_scene_pause_quit_to_title);
    run("V2.WorldScene.IdleStable", test_world_scene_stable_without_transition_on_idle_ticks);
    run("V2.WorldScene.NpcWanderStable", test_world_scene_npc_wander_stable);
    run("V2.VictoryScene.Flow", test_victory_scene_continue_and_quit_paths);
    run("V2.GameOverScene.Flow", test_game_over_scene_retry_title_and_quit);
}
