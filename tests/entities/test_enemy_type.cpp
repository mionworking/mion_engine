#include "../test_common.hpp"
#include "entities/enemy_type.hpp"
#include "core/ini_loader.hpp"
#include <cstring>
#include <string>

static void test_apply_enemy_ini_section_overrides() {
    mion::IniData d;
    d.sections["skeleton"]["max_hp"]       = "99";
    d.sections["skeleton"]["sprite_scale"] = "2.5";
    d.sections["skeleton"]["dir_row"]      = "2";
    d.sections["skeleton"]["sprite_sheet_path"] = "assets/test_enemy.png";

    mion::EnemyDef def = mion::get_enemy_def(mion::EnemyType::Skeleton);
    const int      orig_hp = def.max_hp;
    std::string    storage;
    mion::apply_enemy_ini_section(d, "skeleton", def, &storage);

    EXPECT_EQ(def.max_hp, 99);
    EXPECT_NEAR(def.sprite_scale, 2.5f, 0.001f);
    EXPECT_EQ(def.dir_row, 2);
    EXPECT_TRUE(std::strcmp(def.sprite_sheet_path, storage.c_str()) == 0);
    EXPECT_TRUE(std::strstr(def.sprite_sheet_path, "test_enemy.png") != nullptr);

    mion::IniData d2;
    mion::EnemyDef def2 = mion::get_enemy_def(mion::EnemyType::Skeleton);
    mion::apply_enemy_ini_section(d2, "skeleton", def2, nullptr);
    EXPECT_EQ(def2.max_hp, orig_hp);
}
REGISTER_TEST(test_apply_enemy_ini_section_overrides);
