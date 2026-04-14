#include "../test_common.hpp"
#include "components/spell_defs.hpp"
#include "core/ini_loader.hpp"

static void test_spell_def_effective_scaling() {
    mion::SpellDef def{};
    def.damage              = 10;
    def.damage_per_level    = 4;
    def.cooldown_seconds    = 1.0f;
    def.cooldown_per_level  = 0.15f;
    EXPECT_EQ(def.effective_damage(2), 18);
    EXPECT_NEAR(def.effective_cooldown(2), 0.70f, 0.001f);
    EXPECT_NEAR(def.effective_cooldown(100), 0.05f, 0.001f);
}
REGISTER_TEST(test_spell_def_effective_scaling);

static void test_apply_spell_ini_section_per_level_fields() {
    mion::IniData d;
    d.sections["bolt"]["damage"]            = "12";
    d.sections["bolt"]["damage_per_level"]  = "5";
    d.sections["bolt"]["cooldown_per_level"] = "0.05";

    mion::SpellDef def = mion::g_spell_defs[0];
    def.id             = mion::SpellId::FrostBolt;
    def.damage         = 16;
    def.cooldown_seconds = 0.5f;
    def.damage_per_level   = 0;
    def.cooldown_per_level = 0.0f;

    mion::apply_spell_ini_section(d, "bolt", def);
    EXPECT_EQ(def.damage, 12);
    EXPECT_EQ(def.damage_per_level, 5);
    EXPECT_NEAR(def.cooldown_per_level, 0.05f, 0.001f);
    EXPECT_EQ(def.effective_damage(2), 12 + 10);
    EXPECT_NEAR(def.effective_cooldown(4), 0.30f, 0.001f);
}
REGISTER_TEST(test_apply_spell_ini_section_per_level_fields);

static void test_spell_defs_mutable_and_restorable() {
    const int original = mion::spell_def(mion::SpellId::FrostBolt).damage;

    mion::detail::_g_spell_defs_mutable[static_cast<int>(mion::SpellId::FrostBolt)].damage = 999;
    EXPECT_EQ(mion::spell_def(mion::SpellId::FrostBolt).damage, 999);

    // Restaura para não contaminar outros testes
    mion::detail::_g_spell_defs_mutable[static_cast<int>(mion::SpellId::FrostBolt)].damage = original;
    EXPECT_EQ(mion::spell_def(mion::SpellId::FrostBolt).damage, original);
}
REGISTER_TEST(test_spell_defs_mutable_and_restorable);

static void test_spell_defs_nova_unchanged_by_bolt_mutation() {
    const float original_nova_cost = mion::spell_def(mion::SpellId::Nova).mana_cost;

    mion::detail::_g_spell_defs_mutable[static_cast<int>(mion::SpellId::FrostBolt)].mana_cost = 0.0f;
    EXPECT_NEAR(mion::spell_def(mion::SpellId::Nova).mana_cost, original_nova_cost, 0.01f);

    // Restaura
    mion::detail::_g_spell_defs_mutable[static_cast<int>(mion::SpellId::FrostBolt)].mana_cost = 18.0f;
}
REGISTER_TEST(test_spell_defs_nova_unchanged_by_bolt_mutation);
