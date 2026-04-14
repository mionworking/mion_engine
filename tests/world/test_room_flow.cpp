#include "../test_common.hpp"
#include <vector>
#include "core/scene_ids.hpp"
#include "core/world_layout_ids.hpp"
#include "entities/actor.hpp"
#include "systems/room_flow_system.hpp"
#include "world/room.hpp"

static void test_room_flow_door_with_target_scene() {
    mion::RoomFlowSystem flow;
    mion::RoomDefinition room;
    room.bounds = { 0, 800, 0, 600 };
    room.add_scene_door(10.0f, 200.0f, 70.0f, 300.0f, false, mion::to_string(mion::SceneId::kTitle));

    mion::Actor player;
    player.team      = mion::Team::Player;
    player.is_alive  = true;
    player.collision = { 16.0f, 16.0f };
    player.transform.set_position(40.0f, 250.0f); // centro dentro da porta

    std::vector<mion::Actor*> actors; // sala "limpa"
    flow.fixed_update(actors, player, room);
    EXPECT_EQ(flow.scene_exit_to, std::string(mion::to_string(mion::SceneId::kTitle)));
    EXPECT_FALSE(flow.transition_requested);

    flow.fixed_update(actors, player, room);
    EXPECT_EQ(flow.scene_exit_to, std::string(mion::to_string(mion::SceneId::kTitle)));
}
REGISTER_TEST(test_room_flow_door_with_target_scene);

static void test_room_flow_door_passthrough_world_layout_target() {
    mion::RoomFlowSystem flow;
    mion::RoomDefinition room;
    room.bounds = { 0, 800, 0, 600 };
    room.add_zone_door(10.0f, 200.0f, 70.0f, 300.0f, false, mion::WorldLayoutId::kTown);

    mion::Actor player;
    player.team      = mion::Team::Player;
    player.is_alive  = true;
    player.collision = { 16.0f, 16.0f };
    player.transform.set_position(40.0f, 250.0f);

    std::vector<mion::Actor*> actors;
    flow.fixed_update(actors, player, room);

    EXPECT_EQ(flow.scene_exit_to, mion::WorldLayoutId::kTown);
    EXPECT_FALSE(flow.transition_requested);
}
REGISTER_TEST(test_room_flow_door_passthrough_world_layout_target);

static void test_room_flow_inner_door_advance_room() {
    mion::RoomFlowSystem flow;
    mion::RoomDefinition room;
    room.bounds = { 0, 800, 0, 600 };
    room.add_door(700.0f, 200.0f, 780.0f, 400.0f, false); // sem target = próxima sala

    mion::Actor player;
    player.team      = mion::Team::Player;
    player.is_alive  = true;
    player.collision = { 16.0f, 16.0f };
    player.transform.set_position(740.0f, 300.0f);

    std::vector<mion::Actor*> actors;
    flow.fixed_update(actors, player, room);
    EXPECT_TRUE(flow.scene_exit_to.empty());
    EXPECT_TRUE(flow.transition_requested);
}
REGISTER_TEST(test_room_flow_inner_door_advance_room);
