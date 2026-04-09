#include "../test_common.hpp"

#include "../../src/core/world_layout_ids.hpp"
#include "../../src/systems/room_manager.hpp"

using namespace mion;

static void room_manager_build_room_non_boss_has_forward_and_town_doors() {
    RoomDefinition room;
    Tilemap tilemap;
    IniData rooms_ini{};

    RoomManager::build_room(room, tilemap, 0, rooms_ini);

    EXPECT_EQ(static_cast<int>(room.doors.size()), 2);
    EXPECT_TRUE(room.doors[0].exit_to_zone.empty() && room.doors[0].exit_to_scene.empty());
    EXPECT_EQ(room.doors[1].exit_to_zone, WorldLayoutId::kTown);
}

static void room_manager_build_room_boss_has_only_town_exit_door() {
    RoomDefinition room;
    Tilemap tilemap;
    IniData rooms_ini{};

    RoomManager::build_room(room, tilemap, 3, rooms_ini);

    EXPECT_EQ(static_cast<int>(room.doors.size()), 1);
    EXPECT_EQ(room.doors[0].exit_to_zone, WorldLayoutId::kTown);
}

REGISTER_TEST(room_manager_build_room_non_boss_has_forward_and_town_doors);
REGISTER_TEST(room_manager_build_room_boss_has_only_town_exit_door);
