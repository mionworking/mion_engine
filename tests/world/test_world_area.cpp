#include "../test_common.hpp"

#include "../../src/world/world_area.hpp"

using namespace mion;

static void world_area_global_bounds_includes_offset() {
    WorldArea a;
    a.offset_x = 100.f;
    a.offset_y = -50.f;
    a.room.bounds = {10.f, 110.f, 20.f, 120.f};
    AABB g = a.global_bounds();
    EXPECT_NEAR(g.min_x, 110.f, 0.01f);
    EXPECT_NEAR(g.max_x, 210.f, 0.01f);
    EXPECT_NEAR(g.min_y, -30.f, 0.01f);
    EXPECT_NEAR(g.max_y, 70.f, 0.01f);
}

static void world_area_enemy_vectors_are_isolated() {
    WorldArea a;
    WorldArea b;
    Actor e;
    e.name = "only_a";
    a.enemies.push_back(e);
    EXPECT_EQ(static_cast<int>(a.enemies.size()), 1);
    EXPECT_EQ(static_cast<int>(b.enemies.size()), 0);
}

REGISTER_TEST(world_area_global_bounds_includes_offset);
REGISTER_TEST(world_area_enemy_vectors_are_isolated);
