#include "../test_common.hpp"
#include "components/player_config.hpp"
#include "core/ini_loader.hpp"

static void test_player_ini_applies_fields() {
    mion::IniData d;
    d.sections["player"]["melee_damage"]    = "17";
    d.sections["player"]["ranged_damage"]   = "11";
    d.sections["player"]["base_move_speed"] = "155.5";
    const mion::PlayerConfig cfg = mion::make_player_config_from_ini(d);
    EXPECT_EQ(cfg.melee_damage, 17);
    EXPECT_EQ(cfg.ranged_damage, 11);
    EXPECT_NEAR(cfg.base_move_speed, 155.5f, 0.001f);
}
REGISTER_TEST(test_player_ini_applies_fields);

static void test_player_ini_clamps_mana_and_stamina_max() {
    mion::IniData d;
    d.sections["player"]["base_mana"]        = "120";
    d.sections["player"]["base_mana_max"]    = "50";
    d.sections["player"]["base_stamina"]     = "80";
    d.sections["player"]["base_stamina_max"] = "40";
    const mion::PlayerConfig cfg = mion::make_player_config_from_ini(d);
    EXPECT_NEAR(cfg.base_mana_max,    120.0f, 0.001f);
    EXPECT_NEAR(cfg.base_stamina_max,  80.0f, 0.001f);
}
REGISTER_TEST(test_player_ini_clamps_mana_and_stamina_max);
