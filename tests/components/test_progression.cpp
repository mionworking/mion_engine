#include "../test_common.hpp"
#include "components/progression.hpp"
#include "core/ini_loader.hpp"

static void test_progression_add_xp_multi_level() {
    mion::ProgressionState p;
    // Nível 1→2 custa 100 XP; nível 2→3 custa 200 XP (xp_to_next = level * 100).
    int g = p.add_xp(300);
    EXPECT_EQ(g, 2);
    EXPECT_EQ(p.level, 3);
    EXPECT_EQ(p.pending_level_ups, 2);
    EXPECT_TRUE(p.level_choice_pending());
}
REGISTER_TEST(test_progression_add_xp_multi_level);

static void test_progression_pick_upgrades_reduce_pending() {
    mion::ProgressionState p;
    p.pending_level_ups = 2;
    p.pick_upgrade_damage();
    EXPECT_EQ(p.pending_level_ups, 1);
    EXPECT_EQ(p.bonus_attack_damage, 3);
    EXPECT_NEAR(p.spell_damage_multiplier, 1.08f, 0.001f);
    p.pick_upgrade_hp();
    EXPECT_EQ(p.pending_level_ups, 0);
    EXPECT_EQ(p.bonus_max_hp, 15);
}
REGISTER_TEST(test_progression_pick_upgrades_reduce_pending);

static void test_progression_ini_xp_curve() {
    mion::IniData d;
    d.sections["xp"]["xp_base"]        = "40";
    d.sections["xp"]["xp_level_scale"] = "25";
    const mion::ProgressionConfig cfg = mion::make_progression_config_from_ini(d);
    mion::g_progression_config = cfg;
    mion::ProgressionState p{};
    EXPECT_EQ(p.xp_to_next, 40);
    (void)p.add_xp(40);
    EXPECT_EQ(p.level, 2);
    EXPECT_EQ(p.xp_to_next, 65);
    mion::reset_progression_config_defaults();
}
REGISTER_TEST(test_progression_ini_xp_curve);

static void test_progression_ini_custom_level_up_bonuses() {
    mion::IniData d;
    d.sections["level_up"]["damage_bonus"]     = "7";
    d.sections["level_up"]["hp_bonus"]         = "22";
    d.sections["level_up"]["speed_bonus"]      = "9.5";
    d.sections["level_up"]["spell_mult_bonus"] = "0";
    const mion::ProgressionConfig cfg = mion::make_progression_config_from_ini(d);
    EXPECT_EQ(cfg.damage_bonus, 7);
    EXPECT_EQ(cfg.hp_bonus, 22);
    EXPECT_NEAR(cfg.speed_bonus, 9.5f, 0.001f);
    EXPECT_NEAR(cfg.spell_mult_bonus, 0.0f, 0.001f);
}
REGISTER_TEST(test_progression_ini_custom_level_up_bonuses);
