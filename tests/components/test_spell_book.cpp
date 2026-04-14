#include "../test_common.hpp"
#include "components/mana.hpp"
#include "components/spell_book.hpp"
#include "components/spell_defs.hpp"
#include "components/talent_state.hpp"

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
REGISTER_TEST(test_spell_cooldown_gate);

static void test_spell_mana_gate() {
    mion::SpellBookState sb;
    mion::ManaState mana;
    sb.unlock(mion::SpellId::FrostBolt);
    mana.current = 0.0f;
    EXPECT_FALSE(sb.can_cast(mion::SpellId::FrostBolt, mana));
}
REGISTER_TEST(test_spell_mana_gate);

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
REGISTER_TEST(test_spellbook_syncs_nova_from_spell_power_talent);

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
REGISTER_TEST(test_spellbook_nova_requires_arcane_reservoir_level_2);

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
REGISTER_TEST(test_spell_damage_rank_frostbolt_increases_with_spell_power);

static void test_spell_chain_lightning_cooldown_scales_with_rank() {
    const mion::SpellDef& def = mion::spell_def(mion::SpellId::ChainLightning);
    mion::TalentState weak, strong;
    weak.levels[static_cast<int>(mion::TalentId::SpellPower)]        = 0;
    weak.levels[static_cast<int>(mion::TalentId::ChainLightning)]    = 0;
    strong.levels[static_cast<int>(mion::TalentId::SpellPower)]      = 2;
    strong.levels[static_cast<int>(mion::TalentId::ChainLightning)]  = 1;
    const int rw = mion::spell_damage_rank(mion::SpellId::ChainLightning, weak);
    const int rs = mion::spell_damage_rank(mion::SpellId::ChainLightning, strong);
    EXPECT_LT(def.effective_cooldown(rs), def.effective_cooldown(rw));
}
REGISTER_TEST(test_spell_chain_lightning_cooldown_scales_with_rank);

static void test_spell_damage_rank_battle_cry_matches_iron_or_node() {
    mion::TalentState t;
    t.levels[static_cast<int>(mion::TalentId::BattleCry)] = 2;
    t.levels[static_cast<int>(mion::TalentId::IronBody)] = 1;
    EXPECT_EQ(mion::spell_damage_rank(mion::SpellId::BattleCry, t), 2);
    mion::TalentState t2;
    t2.levels[static_cast<int>(mion::TalentId::IronBody)] = 3;
    EXPECT_EQ(mion::spell_damage_rank(mion::SpellId::BattleCry, t2), 3);
}
REGISTER_TEST(test_spell_damage_rank_battle_cry_matches_iron_or_node);
