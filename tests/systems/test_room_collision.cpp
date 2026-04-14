#include "../test_common.hpp"
#include "entities/actor.hpp"
#include "systems/room_collision.hpp"
#include "world/room.hpp"
#include <vector>

static void test_room_collision_bounds() {
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };

    mion::Actor a;
    a.is_alive = true;
    a.collision = { 16.0f, 16.0f };
    a.transform.set_position(-10.0f, 100.0f);  // fora da borda esquerda

    mion::RoomCollisionSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.resolve_all(actors, room);

    EXPECT_TRUE(a.transform.x >= 16.0f);  // empurrado para dentro
}
REGISTER_TEST(test_room_collision_bounds);

static void test_room_collision_obstacle() {
    mion::RoomDefinition room;
    room.bounds = { 0, 400, 0, 400 };
    room.add_obstacle("wall", 100, 100, 200, 200);

    mion::Actor a;
    a.is_alive = true;
    a.collision = { 16.0f, 16.0f };
    a.transform.set_position(150.0f, 150.0f);  // dentro do obstáculo

    mion::RoomCollisionSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.resolve_all(actors, room);

    // Actor deve ter sido empurrado para fora do obstáculo
    mion::AABB ab = a.collision.bounds_at(a.transform.x, a.transform.y);
    mion::AABB obs = { 100, 200, 100, 200 };
    EXPECT_FALSE(ab.intersects(obs));
}
REGISTER_TEST(test_room_collision_obstacle);

static void test_actor_actor_no_overlap() {
    mion::Actor a, b;
    a.is_alive = true; a.collision = { 16.0f, 16.0f };
    b.is_alive = true; b.collision = { 16.0f, 16.0f };
    a.transform.set_position(0.0f, 0.0f);
    b.transform.set_position(0.0f, 0.0f);  // mesma posição — sobreposição total

    mion::RoomCollisionSystem sys;
    std::vector<mion::Actor*> actors = { &a, &b };
    sys.resolve_actors(actors);

    // Após resolução não devem mais se sobrepor
    mion::AABB ab = a.collision.bounds_at(a.transform.x, a.transform.y);
    mion::AABB bb = b.collision.bounds_at(b.transform.x, b.transform.y);
    EXPECT_FALSE(ab.intersects(bb));
}
REGISTER_TEST(test_actor_actor_no_overlap);

static void test_actor_actor_no_overlap_asymmetric() {
    mion::Actor a, b;
    a.is_alive = true; a.collision = { 16.0f, 16.0f };
    b.is_alive = true; b.collision = { 16.0f, 16.0f };
    a.transform.set_position(16.0f, 100.0f);
    b.transform.set_position(20.0f, 100.0f);

    mion::RoomCollisionSystem sys;
    std::vector<mion::Actor*> actors = { &a, &b };
    sys.resolve_actors(actors);

    mion::AABB ab = a.collision.bounds_at(a.transform.x, a.transform.y);
    mion::AABB bb = b.collision.bounds_at(b.transform.x, b.transform.y);
    EXPECT_FALSE(ab.intersects(bb));
}
REGISTER_TEST(test_actor_actor_no_overlap_asymmetric);

static void test_actor_actor_no_overlap_near_bounds() {
    mion::Actor a, b;
    a.is_alive = true; a.collision = { 16.0f, 16.0f };
    b.is_alive = true; b.collision = { 16.0f, 16.0f };
    a.transform.set_position(16.0f, 100.0f);
    b.transform.set_position(20.0f, 100.0f);

    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };

    mion::RoomCollisionSystem sys;
    std::vector<mion::Actor*> actors = { &a, &b };
    sys.resolve_all(actors, room);
    sys.resolve_actors(actors, &room);

    mion::AABB ab = a.collision.bounds_at(a.transform.x, a.transform.y);
    mion::AABB bb = b.collision.bounds_at(b.transform.x, b.transform.y);
    EXPECT_FALSE(ab.intersects(bb));
    EXPECT_TRUE(a.transform.x >= a.collision.half_w);
    EXPECT_TRUE(b.transform.x <= room.bounds.max_x - b.collision.half_w);
}
REGISTER_TEST(test_actor_actor_no_overlap_near_bounds);
