#include "../test_common.hpp"
#include "systems/pathfinder.hpp"
#include "world/room.hpp"
#include "world/tilemap.hpp"

static void test_nav_grid_obstacle_blocks_tile() {
    mion::Tilemap tm;
    tm.init(5, 5, 32);
    tm.fill(0, 0, 4, 4, mion::TileType::Floor);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    room.add_obstacle("b", 48, 48, 80, 80);
    mion::NavGrid grid;
    grid.build(tm, room);
    EXPECT_FALSE(grid.is_walkable(1, 1));
    EXPECT_TRUE(grid.is_walkable(0, 0));
}
REGISTER_TEST(test_nav_grid_obstacle_blocks_tile);

static void test_nav_grid_world_to_tile_clamp() {
    mion::Tilemap tm;
    tm.init(3, 3, 32);
    tm.fill(0, 0, 2, 2, mion::TileType::Floor);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    mion::NavGrid grid;
    grid.build(tm, room);
    int c = -1, r = -1;
    grid.world_to_tile(-100.0f, -100.0f, c, r);
    EXPECT_EQ(c, 0);
    EXPECT_EQ(r, 0);
    grid.world_to_tile(1.0e6f, 1.0e6f, c, r);
    EXPECT_EQ(c, 2);
    EXPECT_EQ(r, 2);
}
REGISTER_TEST(test_nav_grid_world_to_tile_clamp);

static void test_pathfinder_same_tile_valid_empty_waypoints() {
    mion::Tilemap tm;
    tm.init(4, 4, 32);
    tm.fill(0, 0, 3, 3, mion::TileType::Floor);
    mion::RoomDefinition room;
    room.bounds = { 0, 500, 0, 500 };
    mion::Pathfinder pf;
    pf.build_nav(tm, room);
    mion::Path path = pf.find_path(48.0f, 48.0f, 50.0f, 50.0f);
    EXPECT_TRUE(path.valid);
    EXPECT_TRUE(path.waypoints.empty());
}
REGISTER_TEST(test_pathfinder_same_tile_valid_empty_waypoints);

static void test_pathfinder_find_path_around_wall() {
    mion::Tilemap tm;
    tm.init(3, 3, 32);
    tm.fill(0, 0, 2, 2, mion::TileType::Floor);
    tm.set(1, 0, mion::TileType::Wall);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    mion::Pathfinder pf;
    pf.build_nav(tm, room);
    mion::Path path = pf.find_path(16.0f, 16.0f, 80.0f, 16.0f);
    EXPECT_TRUE(path.valid);
    EXPECT_TRUE(!path.waypoints.empty());
}
REGISTER_TEST(test_pathfinder_find_path_around_wall);

static void test_pathfinder_unreachable_goal() {
    mion::Tilemap tm;
    tm.init(3, 1, 32);
    tm.fill(0, 0, 2, 0, mion::TileType::Floor);
    tm.set(1, 0, mion::TileType::Wall);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 64 };
    mion::Pathfinder pf;
    pf.build_nav(tm, room);
    mion::Path path = pf.find_path(16.0f, 16.0f, 80.0f, 16.0f);
    EXPECT_FALSE(path.valid);
}
REGISTER_TEST(test_pathfinder_unreachable_goal);

static void test_path_advance_and_done() {
    mion::Path p;
    p.valid = true;
    p.waypoints.push_back({ 1.0f, 2.0f });
    p.waypoints.push_back({ 3.0f, 4.0f });
    p.current = 0;
    EXPECT_FALSE(p.done());
    EXPECT_NEAR(p.next_wp().x, 1.0f, 0.001f);
    p.advance();
    EXPECT_NEAR(p.next_wp().x, 3.0f, 0.001f);
    p.advance();
    EXPECT_TRUE(p.done());
    p.reset();
    EXPECT_FALSE(p.valid);
}
REGISTER_TEST(test_path_advance_and_done);
