#include "test_common.hpp"

#include <vector>

#include "entities/actor.hpp"
#include "entities/projectile.hpp"
#include "systems/player_action.hpp"

static void setup_player(mion::Actor& p) {
    p.is_alive = true;
    p.team = mion::Team::Player;
    p.combat.reset_for_spawn();
    p.stamina.current = 100.0f;
    p.mana.current = 200.0f;
    p.facing_x = 1.0f;
    p.facing_y = 0.0f;
    p.transform.set_position(0.0f, 0.0f);
}

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

static void test_player_action_ranged_multishot_spawns_extra_projectiles() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.derived.ranged_damage_final = 10;
    player.spell_book.unlock(mion::SpellId::MultiShot);
    player.talents.levels[static_cast<int>(mion::TalentId::MultiShot)] = 2;

    std::vector<mion::Projectile> pr;
    mion::InputState in;
    in.ranged_pressed = true;
    sys.fixed_update(player, in, 1.0f / 60.0f, nullptr, &pr, nullptr);

    EXPECT_EQ((int)pr.size(), 3);
}

static void test_player_action_frostbolt_uses_derived_spell_multiplier() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.derived.spell_damage_mult = 2.0f;
    player.spell_book.unlock(mion::SpellId::FrostBolt);

    std::vector<mion::Projectile> pr;
    mion::InputState in;
    in.spell_1_pressed = true;
    sys.fixed_update(player, in, 1.0f / 60.0f, nullptr, &pr, nullptr);

    EXPECT_EQ((int)pr.size(), 1);
    EXPECT_EQ(pr[0].damage, 32);
}

static void test_player_action_spell_and_ranged_respect_empowered_multiplier() {
    mion::PlayerActionSystem sys;
    mion::Actor player;
    setup_player(player);
    player.derived.ranged_damage_final = 10;
    player.derived.spell_damage_mult = 1.0f;
    player.empowered_damage_multiplier = 1.5f;
    player.empowered_remaining_seconds = 3.0f;

    std::vector<mion::Projectile> pr;
    mion::InputState in_ranged;
    in_ranged.ranged_pressed = true;
    sys.fixed_update(player, in_ranged, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 1);
    EXPECT_EQ(pr[0].damage, 15);

    player.combat.reset_for_spawn();
    player.ranged_cooldown_remaining_seconds = 0.0f;
    player.spell_book.unlock(mion::SpellId::FrostBolt);
    mion::InputState in_spell;
    in_spell.spell_1_pressed = true;
    sys.fixed_update(player, in_spell, 1.0f / 60.0f, nullptr, &pr, nullptr);
    EXPECT_EQ((int)pr.size(), 2);
    EXPECT_EQ(pr[1].damage, 24);
}

void run_player_action_v2_tests() {
    run("V2.PlayerAction.RangedUsesDerived", test_player_action_ranged_uses_derived_damage);
    run("V2.PlayerAction.MultiShotCount", test_player_action_ranged_multishot_spawns_extra_projectiles);
    run("V2.PlayerAction.FrostUsesDerivedMult", test_player_action_frostbolt_uses_derived_spell_multiplier);
    run("V2.PlayerAction.EmpoweredMultiplier", test_player_action_spell_and_ranged_respect_empowered_multiplier);
}
