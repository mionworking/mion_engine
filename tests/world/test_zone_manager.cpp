#include "../test_common.hpp"

#include "../../src/world/world_map.hpp"
#include "../../src/world/zone_manager.hpp"

using namespace mion;

static WorldMap make_two_zone_map() {
    WorldMap map;
    WorldArea town;
    town.zone = ZoneId::Town;
    town.room.bounds = {0.f, 200.f, 0.f, 200.f};
    map.areas.push_back(std::move(town));
    WorldArea d0;
    d0.zone     = ZoneId::DungeonRoom0;
    d0.offset_x = 200.f;
    d0.room.bounds = {0.f, 200.f, 0.f, 200.f};
    map.areas.push_back(std::move(d0));
    return map;
}

static void zone_manager_detects_zone_and_just_changed() {
    WorldMap map = make_two_zone_map();
    ZoneManager zm;
    zm.current = ZoneId::Town;

    zm.update(50.f, 50.f, map);
    EXPECT_TRUE(zm.current == ZoneId::Town);
    EXPECT_FALSE(zm.just_changed);

    zm.update(250.f, 50.f, map);
    EXPECT_TRUE(zm.current == ZoneId::DungeonRoom0);
    EXPECT_TRUE(zm.just_changed);
    EXPECT_TRUE(zm.previous == ZoneId::Town);

    zm.update(260.f, 50.f, map);
    EXPECT_TRUE(zm.current == ZoneId::DungeonRoom0);
    EXPECT_FALSE(zm.just_changed);
}

static void zone_manager_in_dungeon_in_town() {
    WorldMap map = make_two_zone_map();
    ZoneManager zm;
    zm.update(50.f, 50.f, map);
    EXPECT_TRUE(zm.in_town());
    EXPECT_FALSE(zm.in_dungeon());

    zm.update(250.f, 50.f, map);
    EXPECT_FALSE(zm.in_town());
    EXPECT_TRUE(zm.in_dungeon());
}

static void zone_manager_room_index_equiv() {
    ZoneManager zm;
    zm.current = ZoneId::DungeonRoom0;
    EXPECT_EQ(zm.room_index_equiv(), 0);
    zm.current = ZoneId::DungeonRoom1;
    EXPECT_EQ(zm.room_index_equiv(), 1);
    zm.current = ZoneId::DungeonRoom2;
    EXPECT_EQ(zm.room_index_equiv(), 2);
    zm.current = ZoneId::Boss;
    EXPECT_EQ(zm.room_index_equiv(), 3);
    zm.current = ZoneId::Transition;
    EXPECT_EQ(zm.room_index_equiv(), 0);
    zm.current = ZoneId::Town;
    EXPECT_EQ(zm.room_index_equiv(), 0);
}

REGISTER_TEST(zone_manager_detects_zone_and_just_changed);
REGISTER_TEST(zone_manager_in_dungeon_in_town);
REGISTER_TEST(zone_manager_room_index_equiv);
