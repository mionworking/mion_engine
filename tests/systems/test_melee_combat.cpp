#include "../test_common.hpp"
#include "core/player_actor_id.hpp"
#include "entities/actor.hpp"
#include "systems/melee_combat.hpp"
#include <vector>

static void test_melee_hit_lands() {
    mion::Actor attacker, target;

    attacker.name = mion::PlayerActorId::kName;
    attacker.team = mion::Team::Player;
    attacker.is_alive = true;
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
REGISTER_TEST(test_melee_hit_lands);

static void test_melee_no_hit_outside_range() {
    mion::Actor attacker, target;

    attacker.name = mion::PlayerActorId::kName;
    attacker.team = mion::Team::Player;
    attacker.is_alive = true;
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
REGISTER_TEST(test_melee_no_hit_outside_range);

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
REGISTER_TEST(test_melee_no_friendly_fire);

static void test_melee_no_hit_during_startup() {
    mion::Actor attacker, target;
    attacker.name = mion::PlayerActorId::kName;
    attacker.team = mion::Team::Player;
    attacker.is_alive = true;
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
REGISTER_TEST(test_melee_no_hit_during_startup);
