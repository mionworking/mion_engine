#include "../test_common.hpp"
#include "components/talent_tree.hpp"
#include "components/talent_state.hpp"
#include "core/ini_loader.hpp"
#include "entities/actor.hpp"

static void test_talents_ini_display_name() {
    mion::reset_talent_tree_defaults();
    mion::IniData d;
    d.sections["mana_flow"]["display_name"] = "Fluxo de Mana";
    mion::apply_talents_ini(d);
    EXPECT_TRUE(mion::talent_display_name(mion::TalentId::ManaFlow) == "Fluxo de Mana");
    mion::reset_talent_tree_defaults();
}
REGISTER_TEST(test_talents_ini_display_name);

static void test_talent_ini_cost_per_level_affects_unlock() {
    mion::reset_talent_tree_defaults();
    mion::IniData d;
    d.sections["mana_flow"]["cost_per_level"] = "3";
    mion::apply_talents_ini(d);

    mion::TalentState t{};
    EXPECT_FALSE(t.can_unlock(mion::TalentId::ManaFlow));
    t.pending_points = 3;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    EXPECT_EQ(t.pending_points, 2);
    EXPECT_FALSE(t.can_unlock(mion::TalentId::ManaFlow));
    t.pending_points = 3;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ManaFlow));

    mion::reset_talent_tree_defaults();
}
REGISTER_TEST(test_talent_ini_cost_per_level_affects_unlock);

static void test_talent_requires_parent() {
    mion::TalentState t;
    t.pending_points = 1;

    // filho sem pai
    EXPECT_FALSE(t.try_unlock(mion::TalentId::ManaFlow));
}
REGISTER_TEST(test_talent_requires_parent);

static void test_talent_spends_points_once() {
    mion::TalentState t;
    t.pending_points = 1;

    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    EXPECT_EQ(t.pending_points, 0);

    // já desbloqueado não pode de novo
    EXPECT_FALSE(t.try_unlock(mion::TalentId::ArcaneReservoir));
}
REGISTER_TEST(test_talent_spends_points_once);

static void test_talent_no_unlockable_options_when_no_pending_points() {
    mion::TalentState t;
    t.pending_points = 2;
    EXPECT_TRUE(t.has_unlockable_options());
    t.pending_points = 0;
    EXPECT_FALSE(t.has_unlockable_options());
}
REGISTER_TEST(test_talent_no_unlockable_options_when_no_pending_points);

static void test_talent_applies_spell_power_multiplier() {
    mion::Actor player;
    player.progression.spell_damage_multiplier = 1.0f;
    player.progression.spell_damage_multiplier *= 1.25f;

    const mion::SpellDef& bolt = mion::spell_def(mion::SpellId::FrostBolt);
    EXPECT_EQ(player.progression.scaled_spell_damage(bolt.damage), 20);
}
REGISTER_TEST(test_talent_applies_spell_power_multiplier);
