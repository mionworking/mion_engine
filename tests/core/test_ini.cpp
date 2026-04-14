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
            const mion::ProgressionConfig cfg = mion::make_progression_config_from_ini(data);
            EXPECT_TRUE(cfg.xp_base > 0);
        } else if (std::strcmp(rel, "player.ini") == 0) {
            const mion::PlayerConfig cfg = mion::make_player_config_from_ini(data);
            EXPECT_TRUE(cfg.base_hp > 0);
            EXPECT_TRUE(cfg.melee_damage > 0);
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
REGISTER_TEST(test_ini_load_basic);

static void test_ini_missing_key_returns_default() {
    mion::IniData d;
    EXPECT_EQ(d.get_int("section", "key", 42), 42);
    EXPECT_NEAR(d.get_float("section", "key", 3.14f), 3.14f, 0.01f);
}
REGISTER_TEST(test_ini_missing_key_returns_default);

static void test_ini_missing_file_returns_empty() {
    mion::IniData d = mion::ini_load("__does_not_exist__.ini");
    EXPECT_EQ(d.get_int("any", "key", -1), -1);
}
REGISTER_TEST(test_ini_missing_file_returns_empty);

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
REGISTER_TEST(test_ini_ignores_comments_and_blanks);

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
REGISTER_TEST(test_ini_bad_value_returns_default);

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
REGISTER_TEST(test_ini_multiple_sections_independent);
