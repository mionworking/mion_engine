#include "../test_common.hpp"
#include <vector>
#include "entities/actor.hpp"
#include "entities/projectile.hpp"
#include "systems/player_action.hpp"
#include "components/attributes.hpp"
#include "components/progression.hpp"

// Setup
static void setup_player(mion::Actor& p) {
    if (!p.player) p.player = mion::PlayerData{};
    p.is_alive = true;
    p.team = mion::Team::Player;
    p.combat.reset_for_spawn();
    p.player->stamina.current = 100.0f;
    p.player->mana.current = 200.0f;
    p.facing_x = 1.0f;
    p.facing_y = 0.0f;
    p.transform.set_position(0.0f, 0.0f);
}

// From test_player_action_v2.cpp
static void test_player_action_ranged_uses_derived_damage() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.ranged_damage = 1;
    player.derived.ranged_damage_final = 17;
    std::vector<mion::Projectile> pr;
    mion::InputState in;
    in.ranged_pressed = true;
    sys.fixed_update(player, in, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 1);
    EXPECT_EQ(pr[0].damage, 17);
}
REGISTER_TEST(test_player_action_ranged_uses_derived_damage);

static void test_player_action_ranged_multishot_spawns_extra_projectiles() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.derived.ranged_damage_final = 10;
    player.player->spell_book.unlock(mion::SpellId::MultiShot);
    player.player->talents.levels[static_cast<int>(mion::TalentId::MultiShot)] = 2;
    std::vector<mion::Projectile> pr;
    mion::InputState in;
    in.ranged_pressed = true;
    sys.fixed_update(player, in, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 3);
}
REGISTER_TEST(test_player_action_ranged_multishot_spawns_extra_projectiles);

static void test_player_action_frostbolt_uses_derived_spell_multiplier() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.derived.spell_damage_mult = 2.0f;
    player.player->spell_book.unlock(mion::SpellId::FrostBolt);
    std::vector<mion::Projectile> pr;
    mion::InputState in;
    in.spell_1_pressed = true;
    sys.fixed_update(player, in, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 1);
    EXPECT_EQ(pr[0].damage, 32);
}
REGISTER_TEST(test_player_action_frostbolt_uses_derived_spell_multiplier);

static void test_player_action_spell_and_ranged_respect_empowered_multiplier() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.derived.ranged_damage_final = 10;
    player.derived.spell_damage_mult = 1.0f;
    player.player->empowered_damage_multiplier = 1.5f;
    player.player->empowered_remaining_seconds = 3.0f;
    std::vector<mion::Projectile> pr;
    mion::InputState in_ranged;
    in_ranged.ranged_pressed = true;
    sys.fixed_update(player, in_ranged, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 1);
    EXPECT_EQ(pr[0].damage, 15);
    player.combat.reset_for_spawn();
    player.player->ranged_cooldown_remaining_seconds = 0.0f;
    player.player->spell_book.unlock(mion::SpellId::FrostBolt);
    mion::InputState in_spell;
    in_spell.spell_1_pressed = true;
    sys.fixed_update(player, in_spell, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 2);
    EXPECT_EQ(pr[1].damage, 24);
}
REGISTER_TEST(test_player_action_spell_and_ranged_respect_empowered_multiplier);

// From test_plan.cpp
static void test_player_action_moves_and_sets_facing() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    player.move_speed = 200.0f;
    mion::InputState in;
    in.move_x = 1.0f;
    in.move_y = 0.0f;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_TRUE(player.is_moving);
    EXPECT_NEAR(player.facing_x, 1.0f, 0.01f);
    EXPECT_TRUE(player.transform.x > 0.0f);
}
REGISTER_TEST(test_player_action_moves_and_sets_facing);

static void test_player_action_melee_requires_stamina() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    player.player->stamina.current = 0.0f;
    mion::InputState in;
    in.attack_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_TRUE(player.combat.is_attack_idle());
}
REGISTER_TEST(test_player_action_melee_requires_stamina);

static void test_player_action_melee_begins_attack_with_stamina() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    mion::InputState in;
    in.attack_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_EQ(player.combat.attack_phase, mion::AttackPhase::Startup);
}
REGISTER_TEST(test_player_action_melee_begins_attack_with_stamina);

static void test_player_action_ranged_spawns_and_damage_formula() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    player.player->progression.bonus_attack_damage = 4;
    mion::recompute_player_derived_stats(
        player.derived, player.player->attributes, player.player->progression,
        player.player->talents, player.player->equipment, /*base_melee=*/10, /*base_ranged=*/8);
    std::vector<mion::Projectile> prs;
    mion::InputState in;
    in.ranged_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, &prs, nullptr);
    EXPECT_EQ((int)prs.size(), 1);
    EXPECT_EQ(prs[0].damage, 10);
}
REGISTER_TEST(test_player_action_ranged_spawns_and_damage_formula);

static void test_player_action_bolt_consumes_mana() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    player.player->spell_book.unlock(mion::SpellId::FrostBolt);
    std::vector<mion::Projectile> prs;
    mion::InputState in;
    in.spell_1_pressed = true;
    float mana_before = player.player->mana.current;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, &prs, nullptr);
    EXPECT_TRUE(player.player->mana.current < mana_before);
    EXPECT_EQ((int)prs.size(), 1);
}
REGISTER_TEST(test_player_action_bolt_consumes_mana);

static void test_player_action_nova_hits_enemy_list() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    player.player->spell_book.unlock(mion::SpellId::Nova);
    mion::Actor enemy;
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.transform.set_position(40.0f, 0.0f);
    enemy.health = { 100, 100 };
    enemy.combat.reset_for_spawn();
    std::vector<mion::Actor*> actors = { &player, &enemy };
    mion::InputState in;
    in.spell_2_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, &actors);
    EXPECT_TRUE(enemy.health.current_hp < 100);
}
REGISTER_TEST(test_player_action_nova_hits_enemy_list);

static void test_player_action_dash_consumes_stamina() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    mion::InputState in;
    in.dash_pressed = true;
    in.move_x = 1.0f;
    in.move_y = 0.0f;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_NEAR(player.player->stamina.current, 72.0f, 1.0f);
    EXPECT_TRUE(player.is_dashing());
}
REGISTER_TEST(test_player_action_dash_consumes_stamina);

static void test_player_action_stun_blocks_movement() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    player.move_speed = 200.0f;
    player.status_effects.apply(mion::StatusEffectType::Stun, 1.0f);
    mion::InputState in;
    in.move_x = 1.0f;
    float x0 = player.transform.x;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_NEAR(player.transform.x, x0, 0.001f);
    EXPECT_FALSE(player.is_moving);
}
REGISTER_TEST(test_player_action_stun_blocks_movement);

static void test_player_action_parry_sets_window() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    setup_player(player);
    mion::InputState in;
    in.parry_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_TRUE(player.combat.parry_window_remaining_seconds > 0.0f);
}
REGISTER_TEST(test_player_action_parry_sets_window);
