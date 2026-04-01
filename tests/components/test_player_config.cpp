#include "../test_common.hpp"
#include "components/player_config.hpp"
#include "core/ini_loader.hpp"

static void test_player_ini_applies_fields() {
    mion::PlayerConfig saved = mion::g_player_config;
    mion::reset_player_config_defaults();
    mion::IniData d;
    d.sections["player"]["melee_damage"]   = "17";
    d.sections["player"]["ranged_damage"]  = "11";
    d.sections["player"]["base_move_speed"] = "155.5";
    mion::apply_player_ini(d);
    EXPECT_EQ(mion::g_player_config.melee_damage, 17);
    EXPECT_EQ(mion::g_player_config.ranged_damage, 11);
    EXPECT_NEAR(mion::g_player_config.base_move_speed, 155.5f, 0.001f);
    mion::g_player_config = saved;
}
REGISTER_TEST(test_player_ini_applies_fields);

static void test_player_ini_clamps_mana_and_stamina_max() {
    mion::PlayerConfig saved = mion::g_player_config;
    mion::reset_player_config_defaults();
    mion::IniData d;
    d.sections["player"]["base_mana"]      = "120";
    d.sections["player"]["base_mana_max"]  = "50";
    d.sections["player"]["base_stamina"]     = "80";
    d.sections["player"]["base_stamina_max"] = "40";
    mion::apply_player_ini(d);
    EXPECT_NEAR(mion::g_player_config.base_mana_max, 120.0f, 0.001f);
    EXPECT_NEAR(mion::g_player_config.base_stamina_max, 80.0f, 0.001f);
    mion::g_player_config = saved;
}
REGISTER_TEST(test_player_ini_clamps_mana_and_stamina_max);
