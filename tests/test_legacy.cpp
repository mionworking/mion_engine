#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cassert>
#include <memory>
#include <string>

// Sistemas a testar (headers only — sem SDL no core)
#include "components/transform.hpp"
#include "components/health.hpp"
#include "components/combat.hpp"
#include "components/collision.hpp"
#include "components/mana.hpp"
#include "components/spell_defs.hpp"
#include "components/spell_book.hpp"
#include "components/talent_tree.hpp"
#include "components/talent_state.hpp"
#include "core/input.hpp"
#include "world/room.hpp"
#include "world/tilemap.hpp"
#include "entities/actor.hpp"
#include "systems/room_collision.hpp"
#include "systems/melee_combat.hpp"
#include "systems/enemy_ai.hpp"
#include "systems/spell_effects.hpp"
#include "systems/room_flow_system.hpp"
#include "core/scene.hpp"
#include "core/scene_context.hpp"
#include "core/register_scenes.hpp"
#include "core/save_system.hpp"
#include "core/scene_registry.hpp"
#include "core/engine_paths.hpp"
#include "core/audio.hpp"
#include "core/ini_loader.hpp"
#include "core/scripted_input.hpp"
#include "core/config_loader.hpp"
#include "core/dialogue.hpp"
#include "systems/dialogue_system.hpp"
#include "systems/drop_system.hpp"
#include "scenes/title_scene.hpp"

#include "test_common.hpp"

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
// Mana / SpellBook / Talents
// ---------------------------------------------------------------
static void test_mana_consume_and_regen_delay() {
    mion::ManaState m;
    m.max = 100.0f;
    m.current = 100.0f;
    m.regen_rate = 20.0f;
    m.regen_delay = 0.5f;

    m.consume(40.0f);
    EXPECT_NEAR(m.current, 60.0f, 0.001f);

    m.tick(0.25f); // ainda em delay
    EXPECT_NEAR(m.current, 60.0f, 0.001f);

    m.tick(0.35f); // zera delay
    m.tick(0.10f); // agora regenera
    EXPECT_TRUE(m.current > 60.0f);
}

static void test_mana_clamp_not_negative() {
    mion::ManaState m;
    m.current = 10.0f;
    m.consume(25.0f);
    EXPECT_NEAR(m.current, 0.0f, 0.001f);
}

static void test_mana_regen_to_max() {
    mion::ManaState m;
    m.current = 20.0f;
    m.max = 50.0f;
    m.regen_rate = 100.0f;
    m.regen_delay = 0.0f;

    m.tick(1.0f);
    EXPECT_NEAR(m.current, 50.0f, 0.001f);
}

static void test_spell_cooldown_gate() {
    mion::SpellBookState sb;
    mion::ManaState  mana;
    mion::TalentState talents;
    mana.current = 100.0f;
    sb.unlock(mion::SpellId::FrostBolt);

    EXPECT_TRUE(sb.can_cast(mion::SpellId::FrostBolt, mana));
    sb.start_cooldown(mion::SpellId::FrostBolt, talents);
    EXPECT_FALSE(sb.can_cast(mion::SpellId::FrostBolt, mana));

    sb.tick(10.0f);
    EXPECT_TRUE(sb.can_cast(mion::SpellId::FrostBolt, mana));
}

static void test_spell_mana_gate() {
    mion::SpellBookState sb;
    mion::ManaState mana;
    sb.unlock(mion::SpellId::FrostBolt);
    mana.current = 0.0f;
    EXPECT_FALSE(sb.can_cast(mion::SpellId::FrostBolt, mana));
}

static void test_spellbook_syncs_nova_from_spell_power_talent() {
    mion::SpellBookState sb;
    mion::TalentState talents;
    talents.pending_points = 2;

    sb.sync_from_talents(talents);
    EXPECT_FALSE(sb.is_unlocked(mion::SpellId::Nova));

    EXPECT_TRUE(talents.try_unlock(mion::TalentId::ArcaneReservoir));
    EXPECT_TRUE(talents.try_unlock(mion::TalentId::ArcaneReservoir));
    sb.sync_from_talents(talents);

    EXPECT_TRUE(sb.is_unlocked(mion::SpellId::Nova));
}

static void test_spellbook_nova_requires_arcane_reservoir_level_2() {
    mion::SpellBookState sb;
    mion::TalentState t;
    t.pending_points = 1;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    sb.sync_from_talents(t);
    EXPECT_FALSE(sb.is_unlocked(mion::SpellId::Nova));
    t.pending_points = 1;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    sb.sync_from_talents(t);
    EXPECT_TRUE(sb.is_unlocked(mion::SpellId::Nova));
}

static void test_spell_damage_rank_frostbolt_increases_with_spell_power() {
    mion::TalentState low, high;
    low.levels[static_cast<int>(mion::TalentId::SpellPower)]  = 0;
    low.levels[static_cast<int>(mion::TalentId::FrostBolt)]   = 0;
    high.levels[static_cast<int>(mion::TalentId::SpellPower)] = 3;
    high.levels[static_cast<int>(mion::TalentId::FrostBolt)]  = 0;
    const int r0 = mion::spell_damage_rank(mion::SpellId::FrostBolt, low);
    const int r1 = mion::spell_damage_rank(mion::SpellId::FrostBolt, high);
    EXPECT_EQ(r0, 0);
    EXPECT_EQ(r1, 3);
    EXPECT_GT(r1, r0);
}

static void test_spell_chain_lightning_cooldown_scales_with_rank() {
    const mion::SpellDef& def = mion::spell_def(mion::SpellId::ChainLightning);
    mion::TalentState weak, strong;
    weak.levels[static_cast<int>(mion::TalentId::SpellPower)]        = 0;
    weak.levels[static_cast<int>(mion::TalentId::ChainLightning)]  = 0;
    strong.levels[static_cast<int>(mion::TalentId::SpellPower)]     = 2;
    strong.levels[static_cast<int>(mion::TalentId::ChainLightning)] = 1;
    const int rw = mion::spell_damage_rank(mion::SpellId::ChainLightning, weak);
    const int rs = mion::spell_damage_rank(mion::SpellId::ChainLightning, strong);
    EXPECT_LT(def.effective_cooldown(rs), def.effective_cooldown(rw));
}

static void test_spell_damage_rank_battle_cry_matches_iron_or_node() {
    mion::TalentState t;
    t.levels[static_cast<int>(mion::TalentId::BattleCry)] = 2;
    t.levels[static_cast<int>(mion::TalentId::IronBody)] = 1;
    EXPECT_EQ(mion::spell_damage_rank(mion::SpellId::BattleCry, t), 2);
    mion::TalentState t2;
    t2.levels[static_cast<int>(mion::TalentId::IronBody)] = 3;
    EXPECT_EQ(mion::spell_damage_rank(mion::SpellId::BattleCry, t2), 3);
}

static void test_talent_requires_parent() {
    mion::TalentState t;
    t.pending_points = 1;

    // filho sem pai
    EXPECT_FALSE(t.try_unlock(mion::TalentId::ManaFlow));
}

static void test_talent_spends_points_once() {
    mion::TalentState t;
    t.pending_points = 1;

    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    EXPECT_EQ(t.pending_points, 0);

    // já desbloqueado não pode de novo
    EXPECT_FALSE(t.try_unlock(mion::TalentId::ArcaneReservoir));
}

static void test_talent_no_unlockable_options_when_no_pending_points() {
    mion::TalentState t;
    t.pending_points = 2;
    EXPECT_TRUE(t.has_unlockable_options());
    t.pending_points = 0;
    EXPECT_FALSE(t.has_unlockable_options());
}

static void test_talent_applies_spell_power_multiplier() {
    mion::Actor player;
    player.progression.spell_damage_multiplier = 1.0f;
    player.progression.spell_damage_multiplier *= 1.25f;

    const mion::SpellDef& bolt = mion::spell_def(mion::SpellId::FrostBolt);
    EXPECT_EQ(player.progression.scaled_spell_damage(bolt.damage), 20);
}

static void test_nova_hits_only_in_radius() {
    mion::Actor caster;
    caster.team = mion::Team::Player;
    caster.transform.set_position(0.0f, 0.0f);
    caster.is_alive = true;

    mion::Actor near_enemy;
    near_enemy.team = mion::Team::Enemy;
    near_enemy.transform.set_position(30.0f, 0.0f);
    near_enemy.health = { 100, 100 };
    near_enemy.is_alive = true;

    mion::Actor far_enemy;
    far_enemy.team = mion::Team::Enemy;
    far_enemy.transform.set_position(200.0f, 0.0f);
    far_enemy.health = { 100, 100 };
    far_enemy.is_alive = true;

    std::vector<mion::Actor*> actors = { &caster, &near_enemy, &far_enemy };
    mion::apply_nova(caster, actors, 80.0f, 20);

    EXPECT_EQ(near_enemy.health.current_hp, 80);
    EXPECT_EQ(far_enemy.health.current_hp, 100);
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
// EnemyAISystem
// ---------------------------------------------------------------
static void test_enemy_ai_overlap_preserves_finite_facing() {
    mion::Actor player, enemy;

    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(100.0f, 100.0f);

    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.transform.set_position(100.0f, 100.0f);  // mesma posição do player
    enemy.facing_x = 0.0f;
    enemy.facing_y = -1.0f;
    enemy.aggro_range = 400.0f;
    enemy.attack_range_ai = 45.0f;
    enemy.stop_range_ai = 30.0f;
    enemy.combat.reset_for_spawn();

    mion::EnemyAISystem sys;
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 60.0f);

    EXPECT_TRUE(std::isfinite(enemy.facing_x));
    EXPECT_TRUE(std::isfinite(enemy.facing_y));
    EXPECT_EQ(enemy.facing_x, 0.0f);
    EXPECT_EQ(enemy.facing_y, -1.0f);
    EXPECT_EQ(enemy.combat.attack_phase, mion::AttackPhase::Startup);
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

// ---------------------------------------------------------------
// SceneManager / SceneRegistry + RoomFlow (sem gameplay da dungeon)
// ---------------------------------------------------------------
struct SceneTestMockScene : mion::IScene {
    int         enter_count = 0;
    int         exit_count  = 0;
    int         update_count = 0;
    std::string pending_next;
    bool        request_next = false;
    int*        external_exit_count = nullptr;

    void enter() override { ++enter_count; }
    void exit() override {
        ++exit_count;
        if (external_exit_count) ++*external_exit_count;
    }
    void fixed_update(float, const mion::InputState&) override {
        ++update_count;
        if (request_next) pending_next = "next";
    }
    void render(SDL_Renderer*) override {}
    const char* next_scene() const override {
        return pending_next.empty() ? "" : pending_next.c_str();
    }
    void clear_next_scene_request() override {
        pending_next.clear();
    }
};

static void test_scene_manager_enter_exit_on_set() {
    mion::SceneManager mgr;
    int exits = 0;

    auto a = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* pa = a.get();
    pa->external_exit_count = &exits;
    mgr.set(std::move(a));
    EXPECT_EQ(pa->enter_count, 1);
    EXPECT_EQ(exits, 0);

    auto b = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* pb = b.get();
    pb->external_exit_count = &exits;
    mgr.set(std::move(b));
    EXPECT_EQ(exits, 1);
    EXPECT_EQ(pb->enter_count, 1);

    mgr.set(nullptr);
    EXPECT_EQ(exits, 2);
}

static void test_scene_manager_fixed_update_returns_next_id() {
    mion::SceneManager mgr;
    auto s = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* p = s.get();
    p->request_next = true;
    mgr.set(std::move(s));
    mion::InputState in;
    const char* id = mgr.fixed_update(0.016f, in);
    EXPECT_TRUE(id != nullptr);
    EXPECT_EQ(std::strcmp(id, "next"), 0);
}

static void test_scene_manager_clear_pending_transition() {
    mion::SceneManager mgr;
    auto s = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* p = s.get();
    p->request_next = true;
    mgr.set(std::move(s));

    mion::InputState in;
    EXPECT_EQ(std::strcmp(mgr.fixed_update(0.016f, in), "next"), 0);
    mgr.clear_pending_transition();
    EXPECT_TRUE(p->pending_next.empty());
    EXPECT_EQ(std::strcmp(p->next_scene(), ""), 0);
}

static void test_scene_registry_unknown_id() {
    mion::SceneRegistry reg;
    mion::SceneCreateContext ctx;
    EXPECT_TRUE(reg.create("__missing__", ctx) == nullptr);
}

static void test_register_default_scenes_creates_title_and_dungeon() {
    mion::SceneRegistry reg;
    mion::register_default_scenes(reg);

    mion::SceneCreateContext ctx;
    ctx.viewport_w = 960;
    ctx.viewport_h = 540;

    EXPECT_TRUE(reg.create("title", ctx) != nullptr);
    EXPECT_TRUE(reg.create("town", ctx) != nullptr);
    EXPECT_TRUE(reg.create("dungeon", ctx) != nullptr);
    EXPECT_TRUE(reg.create("credits", ctx) != nullptr);
}

static void test_title_scene_attack_requests_town() {
    mion::DifficultyLevel diff = mion::DifficultyLevel::Normal;
    mion::TitleScene      scene;
    scene.set_difficulty_target(&diff);
    scene.enter();

    mion::InputState in;
    EXPECT_EQ(std::strcmp(scene.next_scene(), ""), 0);

    in.attack_pressed = true;
    scene.fixed_update(0.016f, in);
    EXPECT_EQ(std::strcmp(scene.next_scene(), ""), 0);

    in.attack_pressed = false;
    mion::InputState confirm;
    confirm.confirm_pressed = true;
    scene.fixed_update(0.016f, confirm);
    EXPECT_EQ(std::strcmp(scene.next_scene(), "town"), 0);
    EXPECT_TRUE(diff == mion::DifficultyLevel::Normal);

    scene.clear_next_scene_request();
    EXPECT_EQ(std::strcmp(scene.next_scene(), ""), 0);
}

static void test_title_erase_feedback_timer() {
    mion::TitleScene scene;
    scene.enter();
    EXPECT_FALSE(scene.has_erase_feedback());

    // Pressionar N ativa o timer de feedback
    mion::InputState in;
    in.erase_save_pressed = true;
    scene.fixed_update(0.016f, in);
    EXPECT_TRUE(scene.has_erase_feedback());

    // Soltar N — timer deve continuar decaindo
    in.erase_save_pressed = false;
    // 70 ticks * 0.016s = ~1.12s > 1.0s de duração do timer
    for (int i = 0; i < 70; ++i)
        scene.fixed_update(0.016f, in);
    EXPECT_FALSE(scene.has_erase_feedback());
}

static void test_title_scene_can_open_credits() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::TitleScene scene;
    scene.enter();

    mion::InputState down_press;
    down_press.ui_down_pressed = true;
    mion::InputState down_release;
    down_release.ui_down_pressed = false;
    // Move to EXTRAS via nav_up from index 0 (0 -> 4 -> 3), independent of CONTINUE state.
    mion::InputState up_press_2;
    up_press_2.ui_up_pressed = true;
    mion::InputState up_release_2;
    up_release_2.ui_up_pressed = false;
    scene.fixed_update(0.016f, up_press_2);
    scene.fixed_update(0.016f, up_release_2);
    scene.fixed_update(0.016f, up_press_2);
    scene.fixed_update(0.016f, up_release_2);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    mion::InputState pick_release;
    pick_release.confirm_pressed = false;
    // Enter EXTRAS submenu
    scene.fixed_update(0.016f, pick_press);
    scene.fixed_update(0.016f, pick_release);
    // Pick CREDITS in EXTRAS
    scene.fixed_update(0.016f, pick_release);
    scene.fixed_update(0.016f, pick_press);
    EXPECT_EQ(std::strcmp(scene.next_scene(), "credits"), 0);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}

static void test_title_scene_continue_with_save_requests_town() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::SaveData sd{};
    sd.version = 3;
    sd.player_hp = 90;
    sd.room_index = 0;
    EXPECT_TRUE(mion::SaveSystem::save("mion_save.txt", sd));

    mion::TitleScene scene;
    scene.enter();

    mion::InputState down_press;
    down_press.ui_down_pressed = true;
    mion::InputState down_release;
    down_release.ui_down_pressed = false;
    scene.fixed_update(0.016f, down_press);   // NEW GAME -> CONTINUE
    scene.fixed_update(0.016f, down_release);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    scene.fixed_update(0.016f, pick_press);
    EXPECT_EQ(std::strcmp(scene.next_scene(), "town"), 0);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}

static void test_title_scene_main_menu_quit_requests_quit() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::TitleScene scene;
    scene.enter();

    mion::InputState up_press;
    up_press.ui_up_pressed = true;
    mion::InputState up_release;
    up_release.ui_up_pressed = false;
    // From index 0, nav_up always wraps to QUIT (index 4), independent of disabled CONTINUE.
    scene.fixed_update(0.016f, up_press);
    scene.fixed_update(0.016f, up_release);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    scene.fixed_update(0.016f, pick_press);
    EXPECT_EQ(std::strcmp(scene.next_scene(), "__quit__"), 0);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}

static void test_title_scene_settings_backspace_returns_to_main() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::TitleScene scene;
    scene.enter();

    // Reach SETTINGS robustly: index 0 -> up(quit=4) -> up(extras=3) -> up(settings=2)
    mion::InputState up_press_after_back;
    up_press_after_back.ui_up_pressed = true;
    mion::InputState up_release_after_back;
    up_release_after_back.ui_up_pressed = false;
    scene.fixed_update(0.016f, up_press_after_back);
    scene.fixed_update(0.016f, up_release_after_back);
    scene.fixed_update(0.016f, up_press_after_back);
    scene.fixed_update(0.016f, up_release_after_back);
    scene.fixed_update(0.016f, up_press_after_back);
    scene.fixed_update(0.016f, up_release_after_back);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    mion::InputState pick_release;
    pick_release.confirm_pressed = false;
    scene.fixed_update(0.016f, pick_press);   // Enter settings
    scene.fixed_update(0.016f, pick_release);

    mion::InputState back_press;
    back_press.ui_cancel_pressed = true;
    mion::InputState back_release;
    back_release.ui_cancel_pressed = false;
    scene.fixed_update(0.016f, back_press);
    scene.fixed_update(0.016f, back_release);

    // Back in settings should not trigger any scene transition request.
    EXPECT_EQ(std::strcmp(scene.next_scene(), ""), 0);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}

static void test_room_flow_door_with_target_scene() {
    mion::RoomFlowSystem flow;
    mion::RoomDefinition room;
    room.bounds = { 0, 800, 0, 600 };
    room.add_door(10.0f, 200.0f, 70.0f, 300.0f, false, "title");

    mion::Actor player;
    player.team      = mion::Team::Player;
    player.is_alive  = true;
    player.collision = { 16.0f, 16.0f };
    player.transform.set_position(40.0f, 250.0f); // centro dentro da porta

    std::vector<mion::Actor*> actors; // sala “limpa”
    flow.fixed_update(actors, player, room);
    EXPECT_EQ(flow.scene_exit_to, "title");
    EXPECT_FALSE(flow.transition_requested);

    flow.fixed_update(actors, player, room);
    EXPECT_EQ(flow.scene_exit_to, "title");
}

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


// ---------------------------------------------------------------
// Save / load (key=value)
// ---------------------------------------------------------------
static void test_save_roundtrip() {
    mion::SaveData d;
    d.room_index  = 2;
    d.player_hp   = 77;
    d.progression.level = 4;
    d.progression.xp    = 50;
    d.progression.xp_to_next = 200;
    d.progression.pending_level_ups = 1;
    d.progression.bonus_attack_damage = 6;
    d.progression.bonus_max_hp        = 30;
    d.progression.bonus_move_speed    = 36.0f;
    d.progression.spell_damage_multiplier = 1.25f;
    d.talents.pending_points = 2;
    d.talents.levels[0]      = 2;
    d.talents.levels[1]      = 0;
    d.mana.current    = 80.0f;
    d.mana.max        = 130.0f;
    d.mana.regen_rate = 25.0f;
    d.stamina.current = 90.0f;
    d.gold              = 142;
    d.quest_state.set(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::InProgress);

    const char* path = "mion_save_roundtrip_test.txt";
    std::remove(path);
    EXPECT_TRUE(mion::SaveSystem::save(path, d));
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.room_index, 2);
    EXPECT_EQ(out.player_hp, 77);
    EXPECT_EQ(out.progression.level, 4);
    EXPECT_EQ(out.progression.xp, 50);
    EXPECT_EQ(out.progression.pending_level_ups, 1);
    EXPECT_EQ(out.talents.levels[0], 2);
    EXPECT_EQ(out.talents.levels[1], 0);
    EXPECT_NEAR(out.mana.current, 80.0f, 0.01f);
    EXPECT_NEAR(out.mana.max, 130.0f, 0.01f);
    EXPECT_EQ(out.gold, 142);
    EXPECT_TRUE(out.quest_state.is(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::InProgress));
    std::remove(path);
}

static void test_save_v3_loads_talent_int_levels_from_disk() {
    const char* path = "mion_save_v3_talent_int_test.txt";
    std::remove(path);
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("version=3\nroom_index=0\nhp=100\nxp=0\nlevel=1\nxp_to_next=100\n"
                   "pending_level_ups=0\nbonus_attack_damage=0\nbonus_max_hp=0\n"
                   "bonus_move_speed=0\nspell_damage_multiplier=1\n"
                   "talent_pending_points=0\n",
                   fp);
        for (int i = 0; i < mion::kTalentCount; ++i) {
            int v = (i == 7) ? 3 : ((i == 2) ? 1 : 0);
            std::fprintf(fp, "talent_%d=%d\n", i, v);
        }
        std::fputs("mana_current=100\nmana_max=100\nmana_regen_rate=20\n"
                   "mana_regen_delay=0.75\nmana_regen_delay_rem=0\n"
                   "stamina_current=100\nstamina_max=100\n"
                   "stamina_regen_rate=30\nstamina_regen_delay=1\n"
                   "stamina_regen_delay_rem=0\n",
                   fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.talents.levels[7], 3);
    EXPECT_EQ(out.talents.levels[2], 1);
    std::remove(path);
}

static void test_save_v1_file_loads_and_promotes_to_v2() {
    const char* path = "mion_save_v1_promote_test.txt";
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("version=1\nroom_index=5\nhp=10\nxp=0\nlevel=1\nxp_to_next=100\n"
                   "pending_level_ups=0\nbonus_attack_damage=0\nbonus_max_hp=0\n"
                   "bonus_move_speed=0\nspell_damage_multiplier=1\n"
                   "talent_pending_points=0\n",
                   fp);
        for (int i = 0; i < mion::kTalentCount; ++i)
            std::fprintf(fp, "talent_%d=0\n", i);
        for (int i = 0; i < mion::kSpellCount; ++i)
            std::fprintf(fp, "spell_%d=%d\n", i, i == 0 ? 1 : 0);
        std::fputs("mana_current=100\nmana_max=100\nmana_regen_rate=20\n"
                   "mana_regen_delay=0.75\nmana_regen_delay_rem=0\n"
                   "stamina_current=100\nstamina_max=100\n"
                   "stamina_regen_rate=30\nstamina_regen_delay=1\n"
                   "stamina_regen_delay_rem=0\n",
                   fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.room_index, 5);
    std::remove(path);
}

static void test_save_clamps_room_index_past_max() {
    const char* path = "mion_save_clamp_room_test.txt";
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fprintf(fp,
                     "version=2\nroom_index=%d\nhp=10\nxp=0\nlevel=1\nxp_to_next=100\n"
                     "pending_level_ups=0\nbonus_attack_damage=0\nbonus_max_hp=0\n"
                     "bonus_move_speed=0\nspell_damage_multiplier=1\n"
                     "talent_pending_points=0\n",
                     mion::kSaveMaxRoomIndex + 50);
        for (int i = 0; i < mion::kTalentCount; ++i)
            std::fprintf(fp, "talent_%d=0\n", i);
        for (int i = 0; i < mion::kSpellCount; ++i)
            std::fprintf(fp, "spell_%d=%d\n", i, i == 0 ? 1 : 0);
        std::fputs("mana_current=100\nmana_max=100\nmana_regen_rate=20\n"
                   "mana_regen_delay=0.75\nmana_regen_delay_rem=0\n"
                   "stamina_current=100\nstamina_max=100\n"
                   "stamina_regen_rate=30\nstamina_regen_delay=1\n"
                   "stamina_regen_delay_rem=0\n",
                   fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.room_index, mion::kSaveMaxRoomIndex);
    std::remove(path);
}

static void test_save_rejects_bad_version() {
    const char* path = "mion_save_badver_test.txt";
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("version=99\nroom_index=1\nhp=10\n", fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_FALSE(mion::SaveSystem::load(path, out));
    std::remove(path);
}

static void test_save_load_candidates_falls_back_to_legacy() {
    const char* primary = "mion_save_primary_missing_test.txt";
    const char* legacy  = "mion_save_legacy_present_test.txt";
    std::remove(primary);
    std::remove(legacy);

    mion::SaveData d;
    d.room_index = 4;
    d.player_hp  = 63;
    d.progression.level = 3;
    EXPECT_TRUE(mion::SaveSystem::save(legacy, d));

    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load_candidates(primary, legacy, out));
    EXPECT_EQ(out.room_index, 4);
    EXPECT_EQ(out.player_hp, 63);
    EXPECT_EQ(out.progression.level, 3);

    std::remove(primary);
    std::remove(legacy);
}

static void test_save_load_candidates_prefers_primary() {
    const char* primary = "mion_save_primary_present_test.txt";
    const char* legacy  = "mion_save_legacy_present_test.txt";
    std::remove(primary);
    std::remove(legacy);

    mion::SaveData primary_data;
    primary_data.room_index = 7;
    EXPECT_TRUE(mion::SaveSystem::save(primary, primary_data));

    mion::SaveData legacy_data;
    legacy_data.room_index = 2;
    EXPECT_TRUE(mion::SaveSystem::save(legacy, legacy_data));

    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load_candidates(primary, legacy, out));
    EXPECT_EQ(out.room_index, 7);

    std::remove(primary);
    std::remove(legacy);
}

static void test_save_remove_candidates_clears_primary_and_legacy() {
    const char* primary = "mion_save_remove_primary_test.txt";
    const char* legacy  = "mion_save_remove_legacy_test.txt";
    std::remove(primary);
    std::remove(legacy);

    mion::SaveData d;
    EXPECT_TRUE(mion::SaveSystem::save(primary, d));
    EXPECT_TRUE(mion::SaveSystem::save(legacy, d));
    EXPECT_TRUE(mion::SaveSystem::exists_candidates(primary, legacy));

    EXPECT_TRUE(mion::SaveSystem::remove_candidates(primary, legacy));
    EXPECT_FALSE(mion::SaveSystem::exists(primary));
    EXPECT_FALSE(mion::SaveSystem::exists(legacy));
}

// ---------------------------------------------------------------
// IniLoader
// ---------------------------------------------------------------
// ---------------------------------------------------------------
// EnginePaths — resolve_data_path
// ---------------------------------------------------------------
static void test_resolve_data_path_never_empty() {
    std::string p = mion::resolve_data_path("enemies.ini");
    EXPECT_TRUE(!p.empty());
}

static void test_resolve_data_path_ends_with_relative() {
    // O resultado deve sempre terminar com o nome do arquivo solicitado
    std::string p = mion::resolve_data_path("spells.ini");
    const std::string suffix = "spells.ini";
    EXPECT_TRUE(p.size() >= suffix.size());
    EXPECT_TRUE(p.compare(p.size() - suffix.size(), suffix.size(), suffix) == 0);
}

static void test_resolve_data_path_missing_file_fallback() {
    // Arquivo inexistente: deve retornar fallback "data/xxx", nunca vazio
    std::string p = mion::resolve_data_path("__nao_existe__.ini");
    EXPECT_TRUE(!p.empty());
    EXPECT_TRUE(p.find("__nao_existe__.ini") != std::string::npos);
}

static void test_ini_load_basic() {
    const char* path = "mion_ini_basic_test.ini";
    std::FILE* fp = std::fopen(path, "w");
    EXPECT_TRUE(fp != nullptr);
    std::fputs("[enemies]\nmax_hp=80\nmove_speed=55.5\n\n[drops]\ndrop_chance=75\n", fp);
    std::fclose(fp);

    mion::IniData d = mion::ini_load(path);
    EXPECT_EQ(d.get_int("enemies", "max_hp", 0), 80);
    EXPECT_NEAR(d.get_float("enemies", "move_speed", 0.0f), 55.5f, 0.01f);
    EXPECT_EQ(d.get_int("drops", "drop_chance", 0), 75);
    std::remove(path);
}

static void test_ini_missing_key_returns_default() {
    mion::IniData d;
    EXPECT_EQ(d.get_int("section", "key", 42), 42);
    EXPECT_NEAR(d.get_float("section", "key", 3.14f), 3.14f, 0.01f);
}

static void test_ini_missing_file_returns_empty() {
    mion::IniData d = mion::ini_load("__does_not_exist__.ini");
    EXPECT_EQ(d.get_int("any", "key", -1), -1);
}

static void test_ini_ignores_comments_and_blanks() {
    const char* path = "mion_ini_comments_test.ini";
    std::FILE* fp = std::fopen(path, "w");
    EXPECT_TRUE(fp != nullptr);
    std::fputs("# comment\n[sec]\n\n; another\nval=10\n", fp);
    std::fclose(fp);

    mion::IniData d = mion::ini_load(path);
    EXPECT_EQ(d.get_int("sec", "val", 0), 10);
    std::remove(path);
}

static void test_ini_bad_value_returns_default() {
    const char* path = "mion_ini_badval_test.ini";
    std::FILE* fp = std::fopen(path, "w");
    EXPECT_TRUE(fp != nullptr);
    std::fputs("[sec]\nkey=notanumber\n", fp);
    std::fclose(fp);

    mion::IniData d = mion::ini_load(path);
    EXPECT_EQ(d.get_int("sec", "key", 99), 99);
    std::remove(path);
}

static void test_ini_multiple_sections_independent() {
    const char* path = "mion_ini_multisec_test.ini";
    std::FILE* fp = std::fopen(path, "w");
    EXPECT_TRUE(fp != nullptr);
    std::fputs("[a]\nval=1\n[b]\nval=2\n", fp);
    std::fclose(fp);

    mion::IniData d = mion::ini_load(path);
    EXPECT_EQ(d.get_int("a", "val", 0), 1);
    EXPECT_EQ(d.get_int("b", "val", 0), 2);
    std::remove(path);
}

// ---------------------------------------------------------------
// DialogueSystem — lógica (sem render)
// ---------------------------------------------------------------
static mion::DialogueSequence _make_seq(const char* id, int num_lines) {
    mion::DialogueSequence seq;
    seq.id = id;
    for (int i = 0; i < num_lines; ++i)
        seq.lines.push_back({"Speaker", "Line text", {255, 255, 255, 255}});
    return seq;
}

static void test_dialogue_starts_active() {
    mion::DialogueSystem sys;
    sys.register_sequence(_make_seq("t", 2));
    EXPECT_FALSE(sys.is_active());
    sys.start("t");
    EXPECT_TRUE(sys.is_active());
}

static void test_dialogue_unknown_id_stays_inactive() {
    mion::DialogueSystem sys;
    sys.start("does_not_exist");
    EXPECT_FALSE(sys.is_active());
}

static void test_dialogue_advance_clears_on_last_line() {
    mion::DialogueSystem sys;
    sys.register_sequence(_make_seq("s", 3));
    sys.start("s");

    sys.advance();
    EXPECT_TRUE(sys.is_active());  // linha 2
    sys.advance();
    EXPECT_TRUE(sys.is_active());  // linha 3
    sys.advance();
    EXPECT_FALSE(sys.is_active()); // fim
}

static void test_dialogue_on_finish_called_once() {
    mion::DialogueSystem sys;
    sys.register_sequence(_make_seq("cb", 1));
    int calls = 0;
    sys.start("cb", [&]() { ++calls; });
    EXPECT_EQ(calls, 0);
    sys.advance();
    EXPECT_EQ(calls, 1);
    EXPECT_FALSE(sys.is_active());
    // segunda chamada não deve re-disparar
    sys.advance();
    EXPECT_EQ(calls, 1);
}

static void test_dialogue_confirm_edge_detection() {
    mion::DialogueSystem sys;
    sys.register_sequence(_make_seq("ed", 2));
    sys.start("ed"); // _prev_confirm = true internamente

    mion::InputState held;
    held.confirm_pressed = true;

    // Enter segurado logo após start → não avança (sem borda)
    sys.fixed_update(held);
    EXPECT_TRUE(sys.is_active());

    // Solta
    mion::InputState released;
    released.confirm_pressed = false;
    sys.fixed_update(released);
    EXPECT_TRUE(sys.is_active());

    // Aperta de novo → borda → avança para linha 2
    sys.fixed_update(held);
    EXPECT_TRUE(sys.is_active()); // ainda tem a segunda linha

    // Solta + aperta → avança para fim
    sys.fixed_update(released);
    sys.fixed_update(held);
    EXPECT_FALSE(sys.is_active());
}

static void test_dialogue_advance_safe_when_inactive() {
    mion::DialogueSystem sys;
    sys.advance(); // não deve crashar
    EXPECT_FALSE(sys.is_active());
}

// ---------------------------------------------------------------
// DropConfig + DropSystem
// ---------------------------------------------------------------
static void test_drop_config_defaults() {
    mion::DropConfig cfg;
    EXPECT_EQ(cfg.drop_chance_pct, 50);
    EXPECT_NEAR(cfg.pickup_radius, 36.0f, 0.01f);
    EXPECT_EQ(cfg.health_bonus, 22);
    EXPECT_EQ(cfg.damage_bonus, 1);
    EXPECT_NEAR(cfg.speed_bonus, 10.0f, 0.01f);
    EXPECT_EQ(cfg.lore_drop_chance_pct, 8);
}

static void test_drop_system_full_chance_always_drops() {
    mion::DropConfig cfg;
    cfg.drop_chance_pct = 100;
    std::vector<mion::GroundItem> items;
    const mion::EnemyDef& def = mion::get_enemy_def(mion::EnemyType::Skeleton);
    mion::DropSystem::on_enemy_died(items, 0.0f, 0.0f, def, cfg);
    EXPECT_TRUE((int)items.size() >= 2);
    EXPECT_TRUE(items[0].active);
}

static void test_drop_system_zero_chance_still_drops_gold() {
    mion::DropConfig cfg;
    cfg.drop_chance_pct = 0;
    std::vector<mion::GroundItem> items;
    const mion::EnemyDef& def = mion::get_enemy_def(mion::EnemyType::Orc);
    mion::DropSystem::on_enemy_died(items, 0.0f, 0.0f, def, cfg);
    EXPECT_EQ((int)items.size(), 1);
    if (!items.empty()) {
        EXPECT_EQ(items[0].type, mion::GroundItemType::Gold);
        EXPECT_GT(items[0].gold_value, 0);
    }
}

static void test_drop_system_pickup_heals_with_config_bonus() {
    mion::DropConfig cfg;
    cfg.drop_chance_pct = 100;
    cfg.pickup_radius   = 50.0f;
    cfg.health_bonus    = 40;

    std::vector<mion::GroundItem> items;
    const mion::EnemyDef& sk = mion::get_enemy_def(mion::EnemyType::Skeleton);
    mion::DropSystem::on_enemy_died(items, 0.0f, 0.0f, sk, cfg);
    items.erase(std::remove_if(items.begin(), items.end(),
                               [](const mion::GroundItem& g) {
                                   return g.type == mion::GroundItemType::Gold;
                               }),
                items.end());
    for (auto& it : items) it.type = mion::GroundItemType::Health;

    mion::Actor player;
    player.health = { 100, 60 };  // max_hp=100, current_hp=60
    player.transform.set_position(0.0f, 0.0f);

    EXPECT_FALSE(mion::DropSystem::pickup_near_player(player, items, cfg));
    EXPECT_EQ(player.health.current_hp, 100); // 60 + 40 = 100 (clamped ao max)
    EXPECT_TRUE(items.empty());               // item consumido e removido
}

static void test_drop_lore_pickup_returns_true() {
    mion::DropConfig cfg;
    cfg.pickup_radius = 50.0f;
    cfg.health_bonus  = 10;

    std::vector<mion::GroundItem> items;
    mion::GroundItem              g;
    g.x           = 0.0f;
    g.y           = 0.0f;
    g.type        = mion::GroundItemType::Health;
    g.lore_pickup = true;
    g.active      = true;
    items.push_back(g);

    mion::Actor player;
    player.health = { 100, 50 };
    player.transform.set_position(0.0f, 0.0f);

    EXPECT_TRUE(mion::DropSystem::pickup_near_player(player, items, cfg));
    EXPECT_EQ(player.health.current_hp, 60);
    EXPECT_TRUE(items.empty());
}

// ---------------------------------------------------------------
// g_spell_defs mutável (Fase 3)
// ---------------------------------------------------------------
static void test_spell_defs_mutable_and_restorable() {
    const int original = mion::spell_def(mion::SpellId::FrostBolt).damage;

    mion::g_spell_defs[static_cast<int>(mion::SpellId::FrostBolt)].damage = 999;
    EXPECT_EQ(mion::spell_def(mion::SpellId::FrostBolt).damage, 999);

    // Restaura para não contaminar outros testes
    mion::g_spell_defs[static_cast<int>(mion::SpellId::FrostBolt)].damage = original;
    EXPECT_EQ(mion::spell_def(mion::SpellId::FrostBolt).damage, original);
}

static void test_spell_defs_nova_unchanged_by_bolt_mutation() {
    const float original_nova_cost = mion::spell_def(mion::SpellId::Nova).mana_cost;

    mion::g_spell_defs[static_cast<int>(mion::SpellId::FrostBolt)].mana_cost = 0.0f;
    EXPECT_NEAR(mion::spell_def(mion::SpellId::Nova).mana_cost, original_nova_cost, 0.01f);

    // Restaura
    mion::g_spell_defs[static_cast<int>(mion::SpellId::FrostBolt)].mana_cost = 18.0f;
}

// ---------------------------------------------------------------
// ScriptedInputSource
// ---------------------------------------------------------------
static void test_scripted_input_replay_in_order() {
    mion::ScriptedInputSource src;
    mion::InputState a, b, c;
    a.move_x = 1.0f;
    b.move_y = -1.0f;
    c.attack_pressed = true;
    src.push(a); src.push(b); src.push(c);

    EXPECT_NEAR(src.read_state().move_x,  1.0f, 0.001f);
    EXPECT_NEAR(src.read_state().move_y, -1.0f, 0.001f);
    EXPECT_TRUE(src.read_state().attack_pressed);
}

static void test_scripted_input_exhausted_repeats_last() {
    mion::ScriptedInputSource src;
    mion::InputState last; last.dash_pressed = true;
    src.push(mion::InputState{}); // frame 1
    src.push(last);               // frame 2 (last)

    src.read_state(); // consume frame 1
    src.read_state(); // consume frame 2
    EXPECT_TRUE(src.done());

    // Frames extras devem retornar o último
    EXPECT_TRUE(src.read_state().dash_pressed);
    EXPECT_TRUE(src.read_state().dash_pressed);
}

static void test_scripted_input_empty_returns_default() {
    mion::ScriptedInputSource src;
    mion::InputState s = src.read_state();
    EXPECT_FALSE(s.attack_pressed);
    EXPECT_NEAR(s.move_x, 0.0f, 0.001f);
}

static void test_scripted_input_reset() {
    mion::ScriptedInputSource src;
    mion::InputState a; a.move_x = 1.0f;
    src.push(a);
    src.read_state(); // consume
    EXPECT_TRUE(src.done());
    src.reset();
    EXPECT_FALSE(src.done());
    EXPECT_NEAR(src.read_state().move_x, 1.0f, 0.001f);
}

// ---------------------------------------------------------------
// ConfigLoader
// ---------------------------------------------------------------
static void test_config_defaults_when_missing() {
    mion::ConfigData cfg = mion::load_config("__no_config__.ini");
    EXPECT_EQ(cfg.window_width,  1280);
    EXPECT_EQ(cfg.window_height, 720);
    EXPECT_NEAR(cfg.volume_master, 1.0f, 0.001f);
    EXPECT_FALSE(cfg.mute);
    EXPECT_FALSE(cfg.show_autosave_indicator);
}

static void test_config_load_window_size() {
    const char* path = "mion_config_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[window]\nwidth=960\nheight=540\n", fp);
        std::fclose(fp);
    }
    mion::ConfigData cfg = mion::load_config(path);
    EXPECT_EQ(cfg.window_width,  960);
    EXPECT_EQ(cfg.window_height, 540);
    std::remove(path);
}

static void test_config_load_autosave_indicator() {
    const char* path = "mion_config_ui_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[ui]\nautosave_indicator=1\n", fp);
        std::fclose(fp);
    }
    mion::ConfigData cfg = mion::load_config(path);
    EXPECT_TRUE(cfg.show_autosave_indicator);
    std::remove(path);
}

static void test_config_load_mute() {
    const char* path = "mion_config_mute_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[audio]\nmute=1\nvolume_master=0.5\n", fp);
        std::fclose(fp);
    }
    mion::ConfigData cfg = mion::load_config(path);
    EXPECT_TRUE(cfg.mute);
    EXPECT_NEAR(cfg.volume_master, 0.5f, 0.001f);
    std::remove(path);
}

static void test_config_resolve_prefers_existing_base_file() {
    const char* preferred_name = "mion_config_base_path_test.ini";
    const char* fallback_path  = "mion_config_base_fallback_test.ini";
    std::remove(preferred_name);
    std::remove(fallback_path);

    {
        std::FILE* fp = std::fopen(preferred_name, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("[window]\nwidth=800\n", fp);
        std::fclose(fp);
    }

    std::string resolved = mion::resolve_config_path(".", fallback_path, preferred_name);
    EXPECT_TRUE(resolved.find(preferred_name) != std::string::npos);
    EXPECT_TRUE(resolved != fallback_path);

    std::remove(preferred_name);
    std::remove(fallback_path);
}

static void test_config_resolve_falls_back_to_cwd_file() {
    const char* preferred_name = "mion_config_missing_base_test.ini";
    const char* fallback_path  = "mion_config_cwd_fallback_test.ini";
    std::remove(preferred_name);
    std::remove(fallback_path);

    {
        std::FILE* fp = std::fopen(fallback_path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("[audio]\nmute=1\n", fp);
        std::fclose(fp);
    }

    std::string resolved = mion::resolve_config_path(".", fallback_path, preferred_name);
    EXPECT_EQ(resolved, std::string(fallback_path));

    std::remove(preferred_name);
    std::remove(fallback_path);
}

static void test_config_resolve_returns_non_empty_when_missing() {
    const char* preferred_name = "mion_config_missing_both_test.ini";
    const char* fallback_path  = "mion_config_missing_both_fallback.ini";
    std::remove(preferred_name);
    std::remove(fallback_path);

    std::string resolved = mion::resolve_config_path(".", fallback_path, preferred_name);
    EXPECT_TRUE(!resolved.empty());
    EXPECT_TRUE(resolved.find(preferred_name) != std::string::npos);
}

static void test_config_load_ui_language_default_and_value() {
    EXPECT_EQ(mion::load_ui_language("__no_config_lang__.ini"), std::string("en"));

    const char* path = "mion_config_lang_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[ui]\nlanguage=ptbr\n", fp);
        std::fclose(fp);
    }
    EXPECT_EQ(mion::load_ui_language(path), std::string("ptbr"));
    std::remove(path);
}

static void test_config_save_runtime_config_persists_and_keeps_keybinds() {
    const char* path = "mion_runtime_config_save_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[keybinds]\nattack=J\nconfirm=Return\n", fp);
        std::fclose(fp);
    }

    mion::ConfigData cfg{};
    cfg.window_width             = 1024;
    cfg.window_height            = 576;
    cfg.volume_master            = 0.7f;
    cfg.mute                     = true;
    cfg.show_autosave_indicator  = true;
    EXPECT_TRUE(mion::save_runtime_config(cfg, "ptbr", path));

    mion::IniData d = mion::ini_load(path);
    EXPECT_EQ(d.get_int("window", "width", 0), 1024);
    EXPECT_EQ(d.get_int("window", "height", 0), 576);
    EXPECT_NEAR(d.get_float("audio", "volume_master", 0.0f), 0.7f, 0.001f);
    EXPECT_EQ(d.get_int("audio", "mute", 0), 1);
    EXPECT_EQ(d.get_int("ui", "autosave_indicator", 0), 1);
    EXPECT_EQ(d.get_string("ui", "language", "en"), std::string("ptbr"));
    EXPECT_EQ(d.get_string("keybinds", "attack", ""), std::string("J"));
    EXPECT_EQ(d.get_string("keybinds", "confirm", ""), std::string("Return"));

    std::remove(path);
}

// ---------------------------------------------------------------
// AudioSystem
// ---------------------------------------------------------------
static void test_audio_master_volume_defaults_to_full() {
    mion::AudioSystem audio;
    EXPECT_NEAR(audio.master_volume(), 1.0f, 0.001f);
    EXPECT_NEAR(audio.effective_gain(0.6f), 0.6f, 0.001f);
}

static void test_audio_master_volume_clamps_and_scales_gain() {
    mion::AudioSystem audio;
    audio.set_master_volume(0.25f);
    EXPECT_NEAR(audio.master_volume(), 0.25f, 0.001f);
    EXPECT_NEAR(audio.effective_gain(0.8f), 0.20f, 0.001f);
    EXPECT_NEAR(audio.effective_gain(0.8f, 0.5f), 0.10f, 0.001f);

    audio.set_master_volume(2.0f);
    EXPECT_NEAR(audio.master_volume(), 1.0f, 0.001f);
    EXPECT_NEAR(audio.effective_gain(2.0f), 1.0f, 0.001f);

    audio.set_master_volume(-1.0f);
    EXPECT_NEAR(audio.master_volume(), 0.0f, 0.001f);
    EXPECT_NEAR(audio.effective_gain(0.8f), 0.0f, 0.001f);
}

// ---------------------------------------------------------------
// Integração: ScriptedInputSource + TitleScene
// ---------------------------------------------------------------
static void test_integration_scripted_title_to_town() {
    mion::DifficultyLevel diff = mion::DifficultyLevel::Normal;
    mion::TitleScene      scene;
    scene.set_difficulty_target(&diff);
    scene.enter();

    mion::ScriptedInputSource src;
    mion::InputState        to_diff;
    to_diff.attack_pressed = true;
    src.push(to_diff);
    scene.fixed_update(0.016f, src.read_state());
    EXPECT_EQ(std::strcmp(scene.next_scene(), ""), 0);

    mion::InputState ok;
    ok.confirm_pressed = true;
    src.push(ok);
    scene.fixed_update(0.016f, src.read_state());
    EXPECT_EQ(std::strcmp(scene.next_scene(), "town"), 0);
    EXPECT_TRUE(diff == mion::DifficultyLevel::Normal);
}

// ---------------------------------------------------------------
// Integração: ScriptedInputSource + DialogueSystem
// ---------------------------------------------------------------
static void test_integration_scripted_dialogue_flow() {
    mion::DialogueSystem sys;
    mion::DialogueSequence seq;
    seq.id = "scripted_test";
    seq.lines = {
        {"A", "line 1", {255,255,255,255}},
        {"A", "line 2", {255,255,255,255}},
        {"A", "line 3", {255,255,255,255}},
    };
    sys.register_sequence(seq);

    int done = 0;
    sys.start("scripted_test", [&]() { ++done; });
    // start() seta _prev_confirm=true, então precisa release→press para cada avanço

    mion::ScriptedInputSource src;
    mion::InputState on,  off;
    on.confirm_pressed  = true;
    off.confirm_pressed = false;
    // 3 pares off/on para avançar as 3 linhas
    src.push(off); src.push(on);
    src.push(off); src.push(on);
    src.push(off); src.push(on);

    for (int i = 0; i < 6; ++i)
        sys.fixed_update(src.read_state());

    EXPECT_FALSE(sys.is_active());
    EXPECT_EQ(done, 1);
}

// ---------------------------------------------------------------
// Suíte legada (chamada a partir de test_main.cpp)
// ---------------------------------------------------------------
void run_legacy_tests() {
    run("Transform",               test_transform);
    run("Health",                  test_health);
    run("CombatState.Phases",      test_combat_phase_transitions);
    run("CombatState.HitLanded",   test_combat_hit_landed_reset);
    run("CombatState.HitReaction", test_combat_hit_reaction_interrupts_attack);
    run("CombatState.Invuln",      test_combat_invulnerability);
    run("AABB.Intersection",       test_aabb_intersection);
    run("AABB.CollisionBox",       test_collision_box_bounds);
    run("Input.Normalization",     test_input_normalization);
    run("Mana.ConsumeRegen",       test_mana_consume_and_regen_delay);
    run("Mana.ClampNotNegative",   test_mana_clamp_not_negative);
    run("Mana.RegenToMax",         test_mana_regen_to_max);
    run("Spell.CooldownGate",      test_spell_cooldown_gate);
    run("Spell.ManaGate",          test_spell_mana_gate);
    run("SpellBook.SyncsNova",     test_spellbook_syncs_nova_from_spell_power_talent);
    run("SpellBook.NovaArcaneL2",  test_spellbook_nova_requires_arcane_reservoir_level_2);
    run("Spell.RankFrostSpellPow", test_spell_damage_rank_frostbolt_increases_with_spell_power);
    run("Spell.ChainCDScales",     test_spell_chain_lightning_cooldown_scales_with_rank);
    run("Spell.RankBattleCry",     test_spell_damage_rank_battle_cry_matches_iron_or_node);
    run("Talent.RequiresParent",   test_talent_requires_parent);
    run("Talent.SpendsOnce",       test_talent_spends_points_once);
    run("Talent.NoOptionsNoPending", test_talent_no_unlockable_options_when_no_pending_points);
    run("Talent.AppliesSpellMult", test_talent_applies_spell_power_multiplier);
    run("Spell.NovaRadius",        test_nova_hits_only_in_radius);
    run("Tilemap.InitFill",        test_tilemap_init_and_fill);
    run("Tilemap.Dimensions",      test_tilemap_world_dimensions);
    run("RoomCollision.Bounds",    test_room_collision_bounds);
    run("RoomCollision.Obstacle",  test_room_collision_obstacle);
    run("Melee.HitLands",          test_melee_hit_lands);
    run("Melee.OutOfRange",        test_melee_no_hit_outside_range);
    run("Melee.FriendlyFire",      test_melee_no_friendly_fire);
    run("Melee.StartupNoHit",      test_melee_no_hit_during_startup);
    run("EnemyAI.OverlapFacing",   test_enemy_ai_overlap_preserves_finite_facing);
    run("Knockback.Decays",        test_knockback_decays);
    run("ActorCollision.NoOverlap",test_actor_actor_no_overlap);
    run("ActorCollision.Asymmetric", test_actor_actor_no_overlap_asymmetric);
    run("ActorCollision.NearBounds", test_actor_actor_no_overlap_near_bounds);
    run("SceneManager.EnterExit",     test_scene_manager_enter_exit_on_set);
    run("SceneManager.NextId",       test_scene_manager_fixed_update_returns_next_id);
    run("SceneManager.ClearNext",    test_scene_manager_clear_pending_transition);
    run("SceneRegistry.Unknown",     test_scene_registry_unknown_id);
    run("SceneRegistry.Defaults",    test_register_default_scenes_creates_title_and_dungeon);
    run("TitleScene.ToTown",         test_title_scene_attack_requests_town);
    run("TitleScene.EraseFeedback",  test_title_erase_feedback_timer);
    run("TitleScene.ToCredits",      test_title_scene_can_open_credits);
    run("TitleScene.Continue",       test_title_scene_continue_with_save_requests_town);
    run("TitleScene.MainQuit",       test_title_scene_main_menu_quit_requests_quit);
    run("TitleScene.SettingsBack",   test_title_scene_settings_backspace_returns_to_main);
    run("RoomFlow.TargetScene",      test_room_flow_door_with_target_scene);
    run("RoomFlow.AdvanceRoom",      test_room_flow_inner_door_advance_room);
    run("Save.Roundtrip",            test_save_roundtrip);
    run("Save.V3TalentInt",          test_save_v3_loads_talent_int_levels_from_disk);
    run("Save.V1PromotesV2",         test_save_v1_file_loads_and_promotes_to_v2);
    run("Save.ClampRoom",            test_save_clamps_room_index_past_max);
    run("Save.BadVersion",           test_save_rejects_bad_version);
    run("Save.LegacyFallback",       test_save_load_candidates_falls_back_to_legacy);
    run("Save.PrimaryPreferred",     test_save_load_candidates_prefers_primary);
    run("Save.RemoveCandidates",     test_save_remove_candidates_clears_primary_and_legacy);
    run("Paths.NeverEmpty",          test_resolve_data_path_never_empty);
    run("Paths.EndsSuffix",          test_resolve_data_path_ends_with_relative);
    run("Paths.MissingFallback",     test_resolve_data_path_missing_file_fallback);
    run("Ini.Basic",                 test_ini_load_basic);
    run("Ini.MissingKeyDefault",     test_ini_missing_key_returns_default);
    run("Ini.MissingFile",           test_ini_missing_file_returns_empty);
    run("Ini.CommentsAndBlanks",     test_ini_ignores_comments_and_blanks);
    run("Ini.BadValueDefault",       test_ini_bad_value_returns_default);
    run("Ini.MultipleSections",      test_ini_multiple_sections_independent);
    run("Dialogue.StartsActive",     test_dialogue_starts_active);
    run("Dialogue.UnknownId",        test_dialogue_unknown_id_stays_inactive);
    run("Dialogue.AdvanceToEnd",     test_dialogue_advance_clears_on_last_line);
    run("Dialogue.OnFinish",         test_dialogue_on_finish_called_once);
    run("Dialogue.EdgeDetection",    test_dialogue_confirm_edge_detection);
    run("Dialogue.SafeWhenInactive", test_dialogue_advance_safe_when_inactive);
    run("Drop.ConfigDefaults",       test_drop_config_defaults);
    run("Drop.FullChance",           test_drop_system_full_chance_always_drops);
    run("Drop.ZeroChanceGold",       test_drop_system_zero_chance_still_drops_gold);
    run("Drop.PickupBonus",          test_drop_system_pickup_heals_with_config_bonus);
    run("Drop.LorePickup",           test_drop_lore_pickup_returns_true);
    run("SpellDefs.Mutable",         test_spell_defs_mutable_and_restorable);
    run("SpellDefs.IsolatedMut",     test_spell_defs_nova_unchanged_by_bolt_mutation);
    run("ScriptedInput.ReplayInOrder", test_scripted_input_replay_in_order);
    run("ScriptedInput.ExhaustedLast", test_scripted_input_exhausted_repeats_last);
    run("ScriptedInput.EmptyDefault",  test_scripted_input_empty_returns_default);
    run("ScriptedInput.Reset",         test_scripted_input_reset);
    run("Config.Defaults",             test_config_defaults_when_missing);
    run("Config.WindowSize",           test_config_load_window_size);
    run("Config.AutosaveIndicator",    test_config_load_autosave_indicator);
    run("Config.Mute",                 test_config_load_mute);
    run("Config.ResolveBase",          test_config_resolve_prefers_existing_base_file);
    run("Config.ResolveFallback",      test_config_resolve_falls_back_to_cwd_file);
    run("Config.ResolveMissing",       test_config_resolve_returns_non_empty_when_missing);
    run("Config.Language",             test_config_load_ui_language_default_and_value);
    run("Config.SaveRuntime",          test_config_save_runtime_config_persists_and_keeps_keybinds);
    run("Audio.MasterVolumeDefault",   test_audio_master_volume_defaults_to_full);
    run("Audio.MasterVolumeScale",     test_audio_master_volume_clamps_and_scales_gain);
    run("Integration.ScriptedTitle",   test_integration_scripted_title_to_town);
    run("Integration.ScriptedDialogue", test_integration_scripted_dialogue_flow);
}
