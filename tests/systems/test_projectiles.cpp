#include "../test_common.hpp"
#include "systems/projectile_system.hpp"
#include "entities/projectile.hpp"
#include "entities/actor.hpp"
#include "world/room.hpp"
#include <vector>

static void test_projectile_expires_on_lifetime() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = 100.0f;
    p.y = 100.0f;
    p.vx = p.vy = 0.0f;
    p.lifetime_remaining_seconds = 0.02f;
    prs.push_back(p);
    mion::RoomDefinition room;
    room.bounds = { 0, 500, 0, 500 };
    std::vector<mion::Actor*> actors;
    psys.fixed_update(prs, actors, room, 0.05f);
    EXPECT_EQ((int)prs.size(), 0);
}
REGISTER_TEST(test_projectile_expires_on_lifetime);

static void test_projectile_despawns_outside_room() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = p.y = 100.0f;
    p.vx = 5000.0f;
    p.vy = 0.0f;
    p.half_w = p.half_h = 5.0f;
    p.lifetime_remaining_seconds = 10.0f;
    prs.push_back(p);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    std::vector<mion::Actor*> actors;
    psys.fixed_update(prs, actors, room, 0.05f);
    EXPECT_EQ((int)prs.size(), 0);
}
REGISTER_TEST(test_projectile_despawns_outside_room);

static void test_projectile_hits_enemy_and_applies_damage() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = 0.0f;
    p.y = 0.0f;
    p.vx = p.vy = 0.0f;
    p.half_w = 20.0f;
    p.half_h = 20.0f;
    p.damage = 25;
    p.owner_team = mion::Team::Player;
    p.lifetime_remaining_seconds = 1.0f;
    prs.push_back(p);

    mion::Actor enemy;
    enemy.name = "target_a";
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.hurt_box = { 14.0f, 14.0f };
    enemy.health = { 100, 100 };
    enemy.combat.reset_for_spawn();

    mion::RoomDefinition room;
    room.bounds = { -500, 500, -500, 500 };
    std::vector<mion::Actor*> actors = { &enemy };
    psys.fixed_update(prs, actors, room, 0.016f);
    EXPECT_EQ(enemy.health.current_hp, 75);
    EXPECT_TRUE(psys.projectile_hit_actor);
    EXPECT_EQ((int)prs.size(), 0);
    EXPECT_EQ(psys.last_hit_events.size(), (size_t)1);
    EXPECT_TRUE(psys.last_hit_events[0].target_name == "target_a");
    EXPECT_EQ(psys.last_hit_events[0].damage, 25);
}
REGISTER_TEST(test_projectile_hits_enemy_and_applies_damage);

static void test_projectile_skips_same_team() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = p.y = 0.0f;
    p.vx = p.vy = 0.0f;
    p.half_w = 30.0f;
    p.half_h = 30.0f;
    p.damage = 50;
    p.owner_team = mion::Team::Player;
    p.lifetime_remaining_seconds = 1.0f;
    prs.push_back(p);

    mion::Actor ally;
    ally.team = mion::Team::Player;
    ally.is_alive = true;
    ally.transform.set_position(0.0f, 0.0f);
    ally.hurt_box = { 14.0f, 14.0f };
    ally.health = { 100, 100 };
    ally.combat.reset_for_spawn();

    mion::RoomDefinition room;
    room.bounds = { -500, 500, -500, 500 };
    std::vector<mion::Actor*> actors = { &ally };
    psys.fixed_update(prs, actors, room, 0.016f);
    EXPECT_EQ(ally.health.current_hp, 100);
}
REGISTER_TEST(test_projectile_skips_same_team);

static void test_projectile_blocked_by_obstacle() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = 50.0f;
    p.y = 100.0f;
    p.vx = 200.0f;
    p.vy = 0.0f;
    p.half_w = p.half_h = 4.0f;
    p.lifetime_remaining_seconds = 2.0f;
    prs.push_back(p);

    mion::RoomDefinition room;
    room.bounds = { 0, 400, 0, 400 };
    room.add_obstacle("x", 80, 80, 120, 120);

    std::vector<mion::Actor*> actors;
    psys.fixed_update(prs, actors, room, 0.2f);
    EXPECT_EQ((int)prs.size(), 0);
}
REGISTER_TEST(test_projectile_blocked_by_obstacle);
