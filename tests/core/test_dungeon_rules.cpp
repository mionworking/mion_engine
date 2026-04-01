#include "../test_common.hpp"
#include "world/dungeon_rules.hpp"
#include "world/room.hpp"
#include "core/ini_loader.hpp"
#include "scenes/dungeon_scene.hpp"
#include "entities/enemy_type.hpp"
#include <SDL3/SDL.h>
#include <cstring>
#include <cstdlib>

static void test_enemy_type_name_new_types() {
    EXPECT_EQ(std::strcmp(mion::dungeon_rules::enemy_type_name(mion::EnemyType::Archer), "archer"), 0);
    EXPECT_EQ(std::strcmp(mion::dungeon_rules::enemy_type_name(mion::EnemyType::BossGrimjaw),
                          "boss_grimjaw"),
              0);
}
REGISTER_TEST(test_enemy_type_name_new_types);

static void test_dungeon_rules_xp_and_spawn_budget() {
    EXPECT_EQ(mion::dungeon_rules::xp_per_enemy_kill(0), 14);
    EXPECT_EQ(mion::dungeon_rules::xp_per_enemy_kill(2), 26);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_normal(0), 3);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_normal(40), 80);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_stress(0), 1);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_stress(12), 12);
}
REGISTER_TEST(test_dungeon_rules_xp_and_spawn_budget);

static void test_dungeon_rules_enemy_type_rotation() {
    EXPECT_TRUE(mion::dungeon_rules::enemy_type_for_spawn_index(0) == mion::EnemyType::Skeleton);
    EXPECT_TRUE(mion::dungeon_rules::enemy_type_for_spawn_index(1) == mion::EnemyType::Orc);
    EXPECT_TRUE(mion::dungeon_rules::enemy_type_for_spawn_index(2) == mion::EnemyType::Ghost);
    EXPECT_EQ(std::strcmp(mion::dungeon_rules::enemy_type_name(mion::EnemyType::Orc), "orc"), 0);
}
REGISTER_TEST(test_dungeon_rules_enemy_type_rotation);

static void test_dungeon_rules_room_bounds_width_height() {
    mion::WorldBounds b0 = mion::dungeon_rules::room_bounds(0);
    EXPECT_NEAR(b0.max_x - b0.min_x, 1200.0f, 0.01f);
    EXPECT_NEAR(b0.max_y - b0.min_y, 900.0f, 0.01f);

    mion::WorldBounds b4 = mion::dungeon_rules::room_bounds(4);
    EXPECT_NEAR(b4.max_x - b4.min_x, 1400.0f, 0.01f);
    EXPECT_NEAR(b4.max_y - b4.min_y, 1050.0f, 0.01f);

    mion::WorldBounds b8 = mion::dungeon_rules::room_bounds(8);
    EXPECT_NEAR(b8.max_x - b8.min_x, 1600.0f, 0.01f);

    mion::WorldBounds b11 = mion::dungeon_rules::room_bounds(11);
    EXPECT_NEAR(b11.max_x - b11.min_x, 1800.0f, 0.01f);
    EXPECT_NEAR(b11.max_y - b11.min_y, 1350.0f, 0.01f);
}
REGISTER_TEST(test_dungeon_rules_room_bounds_width_height);

static void test_dungeon_rules_room_template_ids() {
    EXPECT_EQ(mion::dungeon_rules::room_template(0), 5);
    EXPECT_EQ(mion::dungeon_rules::room_template(3), 4);
    EXPECT_EQ(mion::dungeon_rules::room_template(6), 4);
    EXPECT_EQ(mion::dungeon_rules::room_template(1), 1);
    EXPECT_EQ(mion::dungeon_rules::room_template(2), 2);
    EXPECT_EQ(mion::dungeon_rules::room_template(4), 0);
    EXPECT_EQ(mion::dungeon_rules::room_template(5), 1);
}
REGISTER_TEST(test_dungeon_rules_room_template_ids);

static void test_dungeon_rules_room_template_id_names() {
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(0), "arena") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(1), "corredor") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(4), "boss_arena") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(5), "intro") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(99), "intro") == 0);
}
REGISTER_TEST(test_dungeon_rules_room_template_id_names);

static void test_roomdefinition_compound_obstacle_helpers() {
    mion::RoomDefinition r;
    r.bounds = { 0.0f, 1000.0f, 0.0f, 800.0f };
    r.add_barrel_cluster(100.0f, 100.0f);
    EXPECT_EQ((int)r.obstacles.size(), 4);
    r.add_altar(200.0f, 200.0f);
    EXPECT_NEAR(r.obstacles.back().bounds.min_x, 160.0f, 0.01f);
    r.add_pillar_pair(400.0f, 300.0f, 0);
    EXPECT_EQ((int)r.obstacles.size(), 7);
    r.obstacles.clear();
    r.add_wall_L(150.0f, 150.0f, 0);
    EXPECT_EQ((int)r.obstacles.size(), 2);
}
REGISTER_TEST(test_roomdefinition_compound_obstacle_helpers);

static void test_room_loader_normalized_obstacles() {
    mion::IniData d;
    d.sections["arena"]["ini_obstacles"] = "1";
    d.sections["arena.obstacle.0"]["name"] = "box";
    d.sections["arena.obstacle.0"]["nx1"] = "0";
    d.sections["arena.obstacle.0"]["ny1"] = "0";
    d.sections["arena.obstacle.0"]["nx2"] = "0.1";
    d.sections["arena.obstacle.0"]["ny2"] = "0.2";
    mion::RoomDefinition room;
    mion::WorldBounds  b{0.0f, 1000.0f, 0.0f, 800.0f};
    EXPECT_TRUE(mion::load_room_obstacles_from_ini(room, d, b, "arena"));
    EXPECT_EQ((int)room.obstacles.size(), 1);
    EXPECT_NEAR(room.obstacles[0].bounds.min_x, 0.0f, 0.01f);
    EXPECT_NEAR(room.obstacles[0].bounds.max_x, 100.0f, 0.01f);
    EXPECT_NEAR(room.obstacles[0].bounds.min_y, 0.0f, 0.01f);
    EXPECT_NEAR(room.obstacles[0].bounds.max_y, 160.0f, 0.01f);
}
REGISTER_TEST(test_room_loader_normalized_obstacles);

static void test_room_loader_ini_obstacles_off_skips() {
    mion::IniData d;
    d.sections["arena"]["ini_obstacles"] = "0";
    d.sections["arena.obstacle.0"]["nx1"] = "0";
    d.sections["arena.obstacle.0"]["ny1"] = "0";
    d.sections["arena.obstacle.0"]["nx2"] = "1";
    d.sections["arena.obstacle.0"]["ny2"] = "1";
    mion::RoomDefinition room;
    room.obstacles.push_back({ "keep", {0.0f, 10.0f, 0.0f, 10.0f}, "", 0.0f, 0.0f });
    mion::WorldBounds b{0.0f, 100.0f, 0.0f, 100.0f};
    EXPECT_FALSE(mion::load_room_obstacles_from_ini(room, d, b, "arena"));
    EXPECT_EQ((int)room.obstacles.size(), 1);
}
REGISTER_TEST(test_room_loader_ini_obstacles_off_skips);

static void test_dungeon_scene_smoke_optional() {
    if (!std::getenv("MION_DUNGEON_SMOKE"))
        return;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return;
    SDL_Window* win = SDL_CreateWindow("t", 32, 32, SDL_WINDOW_HIDDEN);
    if (!win) {
        SDL_Quit();
        return;
    }
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (!ren) {
        SDL_DestroyWindow(win);
        SDL_Quit();
        return;
    }
    {
        mion::DungeonScene scene;
        scene.set_renderer(ren);
        scene.viewport_w = 320;
        scene.viewport_h = 240;
        scene.enter();
        mion::InputState in;
        scene.fixed_update(0.016f, in);
        scene.exit();
    }
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    EXPECT_TRUE(true);
}
REGISTER_TEST(test_dungeon_scene_smoke_optional);
