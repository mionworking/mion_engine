#include "../test_common.hpp"

#include "../../src/core/camera.hpp"
#include "../../src/world/world_map.hpp"

using namespace mion;

static void world_map_get_obstacles_near_applies_area_offset() {
    WorldMap map;
    WorldArea a;
    a.offset_x = 500.f;
    a.offset_y = 100.f;
    a.room.bounds = {0.f, 200.f, 0.f, 200.f};
    a.room.add_obstacle("wall", 10.f, 10.f, 50.f, 50.f);
    map.areas.push_back(std::move(a));

    AABB query{400.f, 700.f, 0.f, 300.f};
    auto obs = map.get_obstacles_near(query);
    EXPECT_EQ(static_cast<int>(obs.size()), 1);
    EXPECT_NEAR(obs[0].bounds.min_x, 510.f, 0.01f);
    EXPECT_NEAR(obs[0].bounds.max_x, 550.f, 0.01f);
    EXPECT_NEAR(obs[0].bounds.min_y, 110.f, 0.01f);
    EXPECT_NEAR(obs[0].bounds.max_y, 150.f, 0.01f);
}

static void world_map_total_bounds_unions_all_areas() {
    WorldMap map;
    WorldArea a;
    a.room.bounds = {0.f, 100.f, 0.f, 50.f};
    map.areas.push_back(std::move(a));
    WorldArea b;
    b.offset_x = 200.f;
    b.room.bounds = {0.f, 80.f, 0.f, 40.f};
    map.areas.push_back(std::move(b));

    WorldBounds tb = map.total_bounds();
    EXPECT_NEAR(tb.min_x, 0.f, 0.01f);
    EXPECT_NEAR(tb.max_x, 280.f, 0.01f);
    EXPECT_NEAR(tb.min_y, 0.f, 0.01f);
    EXPECT_NEAR(tb.max_y, 50.f, 0.01f);
}

static void world_map_area_at_center_and_edges() {
    WorldMap map;
    WorldArea a;
    a.zone = ZoneId::Town;
    a.room.bounds = {0.f, 100.f, 0.f, 100.f};
    map.areas.push_back(std::move(a));

    const WorldArea* p = map.area_at(50.f, 50.f);
    EXPECT_TRUE(p != nullptr);
    EXPECT_TRUE(p->zone == ZoneId::Town);

    EXPECT_TRUE(map.area_at(0.f, 0.f) != nullptr);
    EXPECT_TRUE(map.area_at(100.f, 100.f) != nullptr);
}

static void world_map_area_at_returns_null_outside() {
    WorldMap map;
    WorldArea a;
    a.room.bounds = {0.f, 10.f, 0.f, 10.f};
    map.areas.push_back(std::move(a));
    EXPECT_TRUE(map.area_at(20.f, 5.f) == nullptr);
    EXPECT_TRUE(map.area_at(5.f, 20.f) == nullptr);
}

// Fase 7 audit: shared inclusive edge (corridor max_x == next area min_x) must not fall in a gap.
static void world_map_area_at_shared_border_not_null() {
    WorldMap map;
    WorldArea corridor;
    corridor.zone = ZoneId::Transition;
    corridor.room.bounds = {0.f, 300.f, 0.f, 200.f};
    map.areas.push_back(std::move(corridor));
    WorldArea room0;
    room0.zone     = ZoneId::DungeonRoom0;
    room0.offset_x = 300.f;
    room0.room.bounds = {0.f, 400.f, 0.f, 200.f};
    map.areas.push_back(std::move(room0));

    const WorldArea* p = map.area_at(300.f, 100.f);
    EXPECT_TRUE(p != nullptr);
}

// Mirrors WorldScene::render culling: areas fully outside the camera frustum are omitted.
static void world_map_areas_in_view_excludes_offscreen_areas() {
    WorldMap map;
    WorldArea near_area;
    near_area.room.bounds = {0.f, 100.f, 0.f, 100.f};
    map.areas.push_back(std::move(near_area));
    WorldArea far_area;
    far_area.offset_x = 5000.f;
    far_area.room.bounds = {0.f, 100.f, 0.f, 100.f};
    map.areas.push_back(std::move(far_area));

    Camera2D cam;
    cam.x = 0.f;
    cam.y = 0.f;
    cam.viewport_w = 1280.f;
    cam.viewport_h = 720.f;
    auto vis = map.areas_in_view(cam);
    EXPECT_EQ(static_cast<int>(vis.size()), 1);
    EXPECT_TRUE(vis[0]->offset_x == 0.f);
}

static void world_map_empty_total_bounds() {
    WorldMap map;
    WorldBounds tb = map.total_bounds();
    EXPECT_NEAR(tb.min_x, 0.f, 0.01f);
    EXPECT_NEAR(tb.max_x, 800.f, 0.01f);
}

REGISTER_TEST(world_map_get_obstacles_near_applies_area_offset);
REGISTER_TEST(world_map_total_bounds_unions_all_areas);
REGISTER_TEST(world_map_area_at_center_and_edges);
REGISTER_TEST(world_map_area_at_returns_null_outside);
REGISTER_TEST(world_map_area_at_shared_border_not_null);
REGISTER_TEST(world_map_areas_in_view_excludes_offscreen_areas);
REGISTER_TEST(world_map_empty_total_bounds);
