#include <cstdio>
#include <cmath>
#include <vector>
#include <cassert>

// Sistemas a testar (headers only — sem SDL no core)
#include "components/transform.hpp"
#include "components/health.hpp"
#include "components/combat.hpp"
#include "components/collision.hpp"
#include "core/input.hpp"
#include "world/room.hpp"
#include "world/tilemap.hpp"
#include "entities/actor.hpp"
#include "systems/room_collision.hpp"
#include "systems/melee_combat.hpp"
#include "systems/enemy_ai.hpp"

// ---------------------------------------------------------------
// Framework mínimo de testes
// ---------------------------------------------------------------
static int s_pass = 0, s_fail = 0;

#define EXPECT_TRUE(expr) do { \
    if (expr) { ++s_pass; } \
    else { ++s_fail; \
        std::printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #expr); } \
} while(0)

#define EXPECT_EQ(a, b)   EXPECT_TRUE((a) == (b))
#define EXPECT_NEAR(a, b, eps) EXPECT_TRUE(std::fabs((float)(a)-(float)(b)) < (eps))
#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

static void run(const char* name, void(*fn)()) {
    std::printf("[ %s ]\n", name);
    fn();
}

// ---------------------------------------------------------------
// Transform
// ---------------------------------------------------------------
static void test_transform() {
    mion::Transform2D t;
    EXPECT_EQ(t.x, 0.0f);
    EXPECT_EQ(t.y, 0.0f);

    t.set_position(10.0f, 20.0f);
    EXPECT_EQ(t.x, 10.0f);
    EXPECT_EQ(t.y, 20.0f);

    t.translate(5.0f, -3.0f);
    EXPECT_EQ(t.x, 15.0f);
    EXPECT_EQ(t.y, 17.0f);
}

// ---------------------------------------------------------------
// Health
// ---------------------------------------------------------------
static void test_health() {
    mion::HealthState h { 100, 100 };
    EXPECT_TRUE(h.is_alive());

    h.apply_damage(30);
    EXPECT_EQ(h.current_hp, 70);
    EXPECT_TRUE(h.is_alive());

    h.apply_damage(200);
    EXPECT_EQ(h.current_hp, 0);
    EXPECT_FALSE(h.is_alive());

    h.restore_full();
    EXPECT_EQ(h.current_hp, 100);
    EXPECT_TRUE(h.is_alive());
}

// ---------------------------------------------------------------
// CombatState — transições de fase
// ---------------------------------------------------------------
static void test_combat_phase_transitions() {
    mion::CombatState c;
    c.reset_for_spawn();
    EXPECT_TRUE(c.is_attack_idle());

    c.begin_attack();
    EXPECT_EQ(c.attack_phase, mion::AttackPhase::Startup);
    EXPECT_FALSE(c.is_in_active_phase());

    // Avança além do startup
    c.advance_time(c.attack_startup_duration_seconds + 0.01f);
    EXPECT_EQ(c.attack_phase, mion::AttackPhase::Active);
    EXPECT_TRUE(c.is_in_active_phase());

    // Avança além do active
    c.advance_time(c.attack_active_duration_seconds + 0.01f);
    EXPECT_EQ(c.attack_phase, mion::AttackPhase::Recovery);
    EXPECT_FALSE(c.is_in_active_phase());

    // Avança além do recovery — volta a idle
    c.advance_time(c.attack_recovery_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_attack_idle());
}

static void test_combat_hit_landed_reset() {
    mion::CombatState c;
    c.reset_for_spawn();
    EXPECT_FALSE(c.attack_hit_landed);

    c.begin_attack();
    c.advance_time(c.attack_startup_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_in_active_phase());

    c.attack_hit_landed = true;
    // advance_time faz 1 transição por chamada — avançar em 2 passos: Active→Recovery, Recovery→Idle
    c.advance_time(c.attack_active_duration_seconds + 0.01f);
    c.advance_time(c.attack_recovery_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_attack_idle());
    c.begin_attack();
    EXPECT_FALSE(c.attack_hit_landed);
}

static void test_combat_hit_reaction_interrupts_attack() {
    mion::CombatState c;
    c.reset_for_spawn();
    c.begin_attack();
    c.advance_time(c.attack_startup_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_in_active_phase());

    c.apply_hit_reaction();
    EXPECT_TRUE(c.is_attack_idle());
    EXPECT_TRUE(c.is_hurt_stunned());
}

static void test_combat_invulnerability() {
    mion::CombatState c;
    c.reset_for_spawn();
    EXPECT_TRUE(c.can_receive_hit());

    c.apply_hit_reaction();
    EXPECT_FALSE(c.can_receive_hit());

    // Avança além da invulnerabilidade
    c.advance_time(c.impact_invulnerability_seconds + 0.01f);
    EXPECT_TRUE(c.can_receive_hit());
}

// ---------------------------------------------------------------
// AABB — colisão
// ---------------------------------------------------------------
static void test_aabb_intersection() {
    mion::AABB a { 0, 10, 0, 10 };
    mion::AABB b { 5, 15, 5, 15 };
    mion::AABB c { 11, 20, 0, 10 };  // não toca

    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
    EXPECT_FALSE(a.intersects(c));
    EXPECT_FALSE(c.intersects(a));
}

static void test_collision_box_bounds() {
    mion::CollisionBox box { 16.0f, 16.0f };
    mion::AABB b = box.bounds_at(100.0f, 100.0f);
    EXPECT_EQ(b.min_x, 84.0f);
    EXPECT_EQ(b.max_x, 116.0f);
    EXPECT_EQ(b.min_y, 84.0f);
    EXPECT_EQ(b.max_y, 116.0f);
}

// ---------------------------------------------------------------
// Input — normalização de movimento
// ---------------------------------------------------------------
static void test_input_normalization() {
    mion::InputState s;
    s.move_x = 1.0f; s.move_y = 0.0f;
    float nx, ny;
    s.normalized_movement(nx, ny);
    EXPECT_NEAR(nx, 1.0f, 0.001f);
    EXPECT_NEAR(ny, 0.0f, 0.001f);

    // Diagonal deve ter comprimento 1
    s.move_x = 1.0f; s.move_y = 1.0f;
    s.normalized_movement(nx, ny);
    float len = std::sqrt(nx*nx + ny*ny);
    EXPECT_NEAR(len, 1.0f, 0.001f);

    // Sem movimento
    s.move_x = 0.0f; s.move_y = 0.0f;
    s.normalized_movement(nx, ny);
    EXPECT_NEAR(nx, 0.0f, 0.001f);
    EXPECT_NEAR(ny, 0.0f, 0.001f);
    EXPECT_FALSE(s.is_moving());
}

// ---------------------------------------------------------------
// Tilemap
// ---------------------------------------------------------------
static void test_tilemap_init_and_fill() {
    mion::Tilemap tm;
    tm.init(10, 8, 32);
    EXPECT_EQ(tm.cols, 10);
    EXPECT_EQ(tm.rows, 8);
    EXPECT_EQ((int)tm.get(0, 0), (int)mion::TileType::Void);

    tm.fill(0, 0, 9, 7, mion::TileType::Floor);
    EXPECT_EQ(tm.get(5, 3), mion::TileType::Floor);

    tm.set(2, 2, mion::TileType::Wall);
    EXPECT_EQ(tm.get(2, 2), mion::TileType::Wall);

    // Fora dos limites retorna Void
    EXPECT_EQ(tm.get(-1, 0), mion::TileType::Void);
    EXPECT_EQ(tm.get(10, 0), mion::TileType::Void);
}

static void test_tilemap_world_dimensions() {
    mion::Tilemap tm;
    tm.init(50, 37, 32);
    EXPECT_NEAR(tm.world_width(),  50 * 32.0f, 0.001f);
    EXPECT_NEAR(tm.world_height(), 37 * 32.0f, 0.001f);
}

// ---------------------------------------------------------------
// RoomCollisionSystem — actor vs world bounds
// ---------------------------------------------------------------
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

// ---------------------------------------------------------------
// MeleeCombatSystem — hit, dano, knockback
// ---------------------------------------------------------------
static void test_melee_hit_lands() {
    mion::Actor attacker, target;

    attacker.name = "player"; attacker.team = mion::Team::Player; attacker.is_alive = true;
    attacker.transform.set_position(0.0f, 0.0f);
    attacker.facing_x = 1.0f; attacker.facing_y = 0.0f;
    attacker.melee_hit_box = { 22.0f, 14.0f, 28.0f };
    attacker.combat.reset_for_spawn();
    attacker.health = { 100, 100 };

    target.name = "enemy"; target.team = mion::Team::Enemy; target.is_alive = true;
    target.transform.set_position(40.0f, 0.0f);  // dentro do alcance
    target.hurt_box = { 14.0f, 14.0f };
    target.combat.reset_for_spawn();
    target.health = { 60, 60 };

    // Coloca o attacker na fase Active manualmente
    attacker.combat.begin_attack();
    attacker.combat.advance_time(attacker.combat.attack_startup_duration_seconds + 0.01f);
    EXPECT_TRUE(attacker.combat.is_in_active_phase());

    mion::MeleeCombatSystem sys;
    std::vector<mion::Actor*> actors = { &attacker, &target };
    sys.fixed_update(actors, 1.0f/60.0f);

    EXPECT_TRUE(attacker.combat.attack_hit_landed);
    EXPECT_EQ(target.health.current_hp, 60 - sys.base_damage);
    EXPECT_EQ(sys.pending_hitstop, sys.hitstop_frames);
    EXPECT_TRUE(sys.player_hit_landed);
}

static void test_melee_no_hit_outside_range() {
    mion::Actor attacker, target;

    attacker.name = "player"; attacker.team = mion::Team::Player; attacker.is_alive = true;
    attacker.transform.set_position(0.0f, 0.0f);
    attacker.facing_x = 1.0f; attacker.facing_y = 0.0f;
    attacker.melee_hit_box = { 22.0f, 14.0f, 28.0f };
    attacker.combat.reset_for_spawn();
    attacker.health = { 100, 100 };

    target.name = "enemy"; target.team = mion::Team::Enemy; target.is_alive = true;
    target.transform.set_position(300.0f, 0.0f);  // longe demais
    target.hurt_box = { 14.0f, 14.0f };
    target.combat.reset_for_spawn();
    target.health = { 60, 60 };

    attacker.combat.begin_attack();
    attacker.combat.advance_time(attacker.combat.attack_startup_duration_seconds + 0.01f);

    mion::MeleeCombatSystem sys;
    std::vector<mion::Actor*> actors = { &attacker, &target };
    sys.fixed_update(actors, 1.0f/60.0f);

    EXPECT_FALSE(attacker.combat.attack_hit_landed);
    EXPECT_EQ(target.health.current_hp, 60);
}

static void test_melee_no_friendly_fire() {
    mion::Actor a1, a2;
    a1.name = "p1"; a1.team = mion::Team::Player; a1.is_alive = true;
    a1.transform.set_position(0.0f, 0.0f);
    a1.facing_x = 1.0f; a1.facing_y = 0.0f;
    a1.melee_hit_box = { 22.0f, 14.0f, 28.0f };
    a1.combat.reset_for_spawn(); a1.health = { 100, 100 };

    a2.name = "p2"; a2.team = mion::Team::Player; a2.is_alive = true;
    a2.transform.set_position(40.0f, 0.0f);
    a2.hurt_box = { 14.0f, 14.0f };
    a2.combat.reset_for_spawn(); a2.health = { 100, 100 };

    a1.combat.begin_attack();
    a1.combat.advance_time(a1.combat.attack_startup_duration_seconds + 0.01f);

    mion::MeleeCombatSystem sys;
    std::vector<mion::Actor*> actors = { &a1, &a2 };
    sys.fixed_update(actors, 1.0f/60.0f);

    EXPECT_EQ(a2.health.current_hp, 100);  // sem dano
}

static void test_melee_no_hit_during_startup() {
    mion::Actor attacker, target;
    attacker.name = "player"; attacker.team = mion::Team::Player; attacker.is_alive = true;
    attacker.transform.set_position(0.0f, 0.0f);
    attacker.facing_x = 1.0f; attacker.facing_y = 0.0f;
    attacker.melee_hit_box = { 22.0f, 14.0f, 28.0f };
    attacker.combat.reset_for_spawn(); attacker.health = { 100, 100 };

    target.name = "enemy"; target.team = mion::Team::Enemy; target.is_alive = true;
    target.transform.set_position(40.0f, 0.0f);
    target.hurt_box = { 14.0f, 14.0f };
    target.combat.reset_for_spawn(); target.health = { 60, 60 };

    attacker.combat.begin_attack();
    // Não avança além do startup — fase ainda é Startup, não Active
    EXPECT_EQ(attacker.combat.attack_phase, mion::AttackPhase::Startup);

    mion::MeleeCombatSystem sys;
    std::vector<mion::Actor*> actors = { &attacker, &target };
    sys.fixed_update(actors, 1.0f/60.0f);

    EXPECT_EQ(target.health.current_hp, 60);  // sem dano durante startup
}

// ---------------------------------------------------------------
// Knockback
// ---------------------------------------------------------------
static void test_knockback_decays() {
    mion::Actor a;
    a.is_alive = true;
    a.transform.set_position(100.0f, 100.0f);
    a.apply_knockback_impulse(300.0f, 0.0f);
    EXPECT_TRUE(a.knockback_vx > 0.0f);

    // Após muitos frames deve decair para zero
    for (int i = 0; i < 60; ++i)
        a.step_knockback(1.0f / 60.0f);

    EXPECT_NEAR(a.knockback_vx, 0.0f, 1.0f);
    EXPECT_NEAR(a.knockback_vy, 0.0f, 1.0f);
}

// ---------------------------------------------------------------
// Actor-actor collision
// ---------------------------------------------------------------
static void test_actor_actor_no_overlap() {
    mion::Actor a, b;
    a.is_alive = true; a.collision = { 16.0f, 16.0f };
    b.is_alive = true; b.collision = { 16.0f, 16.0f };
    a.transform.set_position(0.0f, 0.0f);
    b.transform.set_position(0.0f, 0.0f);  // mesma posição — sobreposição total

    mion::RoomCollisionSystem sys;
    mion::RoomDefinition room; room.bounds = { -500, 500, -500, 500 };
    std::vector<mion::Actor*> actors = { &a, &b };
    sys.resolve_actors(actors);

    // Após resolução não devem mais se sobrepor
    mion::AABB ab = a.collision.bounds_at(a.transform.x, a.transform.y);
    mion::AABB bb = b.collision.bounds_at(b.transform.x, b.transform.y);
    EXPECT_FALSE(ab.intersects(bb));
}

// ---------------------------------------------------------------
// main
// ---------------------------------------------------------------
int main() {
    run("Transform",               test_transform);
    run("Health",                  test_health);
    run("CombatState.Phases",      test_combat_phase_transitions);
    run("CombatState.HitLanded",   test_combat_hit_landed_reset);
    run("CombatState.HitReaction", test_combat_hit_reaction_interrupts_attack);
    run("CombatState.Invuln",      test_combat_invulnerability);
    run("AABB.Intersection",       test_aabb_intersection);
    run("AABB.CollisionBox",       test_collision_box_bounds);
    run("Input.Normalization",     test_input_normalization);
    run("Tilemap.InitFill",        test_tilemap_init_and_fill);
    run("Tilemap.Dimensions",      test_tilemap_world_dimensions);
    run("RoomCollision.Bounds",    test_room_collision_bounds);
    run("RoomCollision.Obstacle",  test_room_collision_obstacle);
    run("Melee.HitLands",          test_melee_hit_lands);
    run("Melee.OutOfRange",        test_melee_no_hit_outside_range);
    run("Melee.FriendlyFire",      test_melee_no_friendly_fire);
    run("Melee.StartupNoHit",      test_melee_no_hit_during_startup);
    run("Knockback.Decays",        test_knockback_decays);
    run("ActorCollision.NoOverlap",test_actor_actor_no_overlap);

    std::printf("\n%d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
