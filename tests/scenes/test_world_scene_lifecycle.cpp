#include "../test_common.hpp"

#include "../../src/core/save_data.hpp"
#include "../../src/core/save_system.hpp"
#include "../../src/scenes/world_scene.hpp"

using namespace mion;

static void world_scene_enter_and_idle_next_scene_empty() {
    SaveSystem::remove_default_saves();

    WorldScene s;
    s.enter();

    EXPECT_EQ(s.next_scene(), SceneId::kNone);

    InputState idle{};
    s.fixed_update(1.f / 60.f, idle);
    EXPECT_EQ(s.next_scene(), SceneId::kNone);

    s.exit();
    SaveSystem::remove_default_saves();
}

// Uses legacy mion_save.txt so CI/sandbox without writable SDL pref dir still loads (load_candidates fallback).
static void world_scene_enter_restores_saved_world_position() {
    SaveSystem::remove_default_saves();

    SaveData sd{};
    sd.version         = kSaveFormatVersion;
    sd.player_hp       = 100;
    sd.gold            = 0;
    sd.player_world_x  = 1000.f;
    sd.player_world_y  = 500.f;
    EXPECT_TRUE(SaveSystem::save(SaveSystem::legacy_path(), sd));

    WorldScene s;
    s.enter();
    EXPECT_NEAR(s.player_actor().transform.x, 1000.f, 1.f);
    EXPECT_NEAR(s.player_actor().transform.y, 500.f, 1.f);

    s.exit();
    SaveSystem::remove_default_saves();
}

static void world_scene_pause_quit_goes_to_title() {
    SaveSystem::remove_default_saves();

    WorldScene s;
    s.enter();

    InputState esc_on;
    esc_on.pause_pressed = true;
    s.fixed_update(0.016f, esc_on);

    InputState esc_off;
    s.fixed_update(0.016f, esc_off);

    InputState down_on;
    down_on.ui_down_pressed = true;
    for (int i = 0; i < 3; ++i) {
        s.fixed_update(0.016f, down_on);
        s.fixed_update(0.016f, esc_off);
    }

    InputState ok_on;
    ok_on.confirm_pressed = true;
    s.fixed_update(0.016f, ok_on);

    EXPECT_EQ(s.next_scene(), SceneId::kTitle);

    s.exit();
    SaveSystem::remove_default_saves();
}

REGISTER_TEST(world_scene_enter_and_idle_next_scene_empty);
REGISTER_TEST(world_scene_enter_restores_saved_world_position);
REGISTER_TEST(world_scene_pause_quit_goes_to_title);
