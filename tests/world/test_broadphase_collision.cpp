#include "../test_common.hpp"

#include "../../src/entities/actor.hpp"
#include "../../src/systems/room_collision.hpp"
#include "../../src/world/world_map.hpp"

using namespace mion;

static void broadphase_matches_single_room_resolve_for_same_obstacles() {
    RoomDefinition room;
    room.bounds = {0.f, 400.f, 0.f, 400.f};
    room.add_obstacle("b", 100.f, 100.f, 180.f, 180.f);

    Actor actor;
    actor.is_alive = true;
    actor.collision = {16.f, 16.f};
    actor.transform.set_position(140.f, 140.f);

    RoomCollisionSystem col;
    std::vector<Actor*> actors = {&actor};

    Actor copy_a = actor;
    std::vector<Actor*> acopy = {&copy_a};
    col.resolve_all(acopy, room);

    Actor copy_b = actor;
    std::vector<Actor*> bcopy = {&copy_b};
    col.resolve_all(bcopy, room.obstacles, room.bounds);

    EXPECT_NEAR(copy_a.transform.x, copy_b.transform.x, 0.1f);
    EXPECT_NEAR(copy_a.transform.y, copy_b.transform.y, 0.1f);
}

static void broadphase_sees_obstacles_from_two_offset_areas() {
    WorldMap map;
    WorldArea left;
    left.room.bounds = {0.f, 200.f, 0.f, 200.f};
    left.room.add_obstacle("L", 30.f, 30.f, 70.f, 70.f);
    map.areas.push_back(std::move(left));

    WorldArea right;
    right.offset_x = 200.f;
    right.room.bounds = {0.f, 200.f, 0.f, 200.f};
    right.room.add_obstacle("R", 30.f, 30.f, 70.f, 70.f);
    map.areas.push_back(std::move(right));

    AABB q{20.f, 280.f, 20.f, 90.f};
    auto obs = map.get_obstacles_near(q);
    EXPECT_EQ(static_cast<int>(obs.size()), 2);

    Actor actor;
    actor.is_alive = true;
    actor.collision = {8.f, 8.f};
    actor.transform.set_position(250.f, 50.f);

    RoomCollisionSystem col;
    std::vector<Actor*> actors = {&actor};
    col.resolve_all(actors, obs, map.total_bounds());

    AABB box = actor.collision.bounds_at(actor.transform.x, actor.transform.y);
    for (const auto& o : obs)
        EXPECT_FALSE(box.intersects(o.bounds));
}

REGISTER_TEST(broadphase_matches_single_room_resolve_for_same_obstacles);
REGISTER_TEST(broadphase_sees_obstacles_from_two_offset_areas);
