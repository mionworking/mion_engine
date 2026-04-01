#include "../test_common.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "core/ini_loader.hpp"
#include "core/engine_paths.hpp"
#include "components/progression.hpp"
#include "components/player_config.hpp"
#include "components/talent_tree.hpp"
#include "components/spell_defs.hpp"
#include "entities/enemy_type.hpp"

static void test_ini_get_string() {
    mion::IniData d;
    d.sections["sec"]["k"] = "hello";
    EXPECT_TRUE(d.get_string("sec", "k", "") == "hello");
    EXPECT_TRUE(d.get_string("sec", "missing", "x") == "x");
}
REGISTER_TEST(test_ini_get_string);

static void test_ini_load_get_string_from_file() {
    const char* path = "mion_ini_string_roundtrip.ini";
    std::FILE*  fp   = std::fopen(path, "w");
    EXPECT_TRUE(fp != nullptr);
    std::fputs("[labels]\ntitle=Olá INI\nempty_default=x\n", fp);
    std::fclose(fp);

    mion::IniData d = mion::ini_load(path);
    EXPECT_TRUE(d.get_string("labels", "title", "") == "Olá INI");
    EXPECT_TRUE(d.get_string("labels", "missing", "fallback") == "fallback");
    std::remove(path);
}
REGISTER_TEST(test_ini_load_get_string_from_file);

static void test_data_repo_inis_parse_if_present() {
    const char* files[] = {
        "progression.ini",
        "player.ini",
        "talents.ini",
        "enemies.ini",
        "spells.ini",
    };
    for (const char* rel : files) {
        mion::IniData data = mion::ini_load(mion::resolve_data_path(rel));
        if (data.sections.empty())
            continue;

        if (std::strcmp(rel, "progression.ini") == 0) {
            mion::ProgressionConfig stash = mion::g_progression_config;
            mion::reset_progression_config_defaults();
            mion::apply_progression_ini(data);
            EXPECT_TRUE(mion::g_progression_config.xp_base > 0);
            mion::g_progression_config = stash;
        } else if (std::strcmp(rel, "player.ini") == 0) {
            mion::PlayerConfig stash = mion::g_player_config;
            mion::reset_player_config_defaults();
            mion::apply_player_ini(data);
            EXPECT_TRUE(mion::g_player_config.base_hp > 0);
            EXPECT_TRUE(mion::g_player_config.melee_damage > 0);
            mion::g_player_config = stash;
        } else if (std::strcmp(rel, "talents.ini") == 0) {
            mion::reset_talent_tree_defaults();
            mion::apply_talents_ini(data);
            EXPECT_TRUE(!mion::talent_display_name(mion::TalentId::ManaFlow).empty());
            mion::reset_talent_tree_defaults();
        } else if (std::strcmp(rel, "enemies.ini") == 0) {
            mion::EnemyDef def = mion::get_enemy_def(mion::EnemyType::Skeleton);
            std::string store;
            mion::apply_enemy_ini_section(data, "skeleton", def, &store);
            EXPECT_TRUE(def.max_hp > 0);
        } else if (std::strcmp(rel, "spells.ini") == 0) {
            mion::SpellDef def{};
            def.id = mion::SpellId::FrostBolt;
            mion::apply_spell_ini_section(data, "frost_bolt", def);
            EXPECT_TRUE(def.damage >= 0);
        }
    }
}
REGISTER_TEST(test_data_repo_inis_parse_if_present);

static void test_ini_sections_with_prefix_sorted() {
    mion::IniData d;
    d.sections["arena.obstacle.2"]["k"] = "1";
    d.sections["arena.obstacle.0"]["k"] = "1";
    d.sections["arena.obstacle.1"]["k"] = "1";
    d.sections["arena"]["x"]              = "1";
    d.sections["other"]["k"]              = "1";
    auto v = d.sections_with_prefix("arena.obstacle");
    EXPECT_EQ((int)v.size(), 3);
    EXPECT_TRUE(v[0] == "arena.obstacle.0");
    EXPECT_TRUE(v[1] == "arena.obstacle.1");
    EXPECT_TRUE(v[2] == "arena.obstacle.2");
}
REGISTER_TEST(test_ini_sections_with_prefix_sorted);
