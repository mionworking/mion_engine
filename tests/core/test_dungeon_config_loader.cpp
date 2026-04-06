#include "../test_common.hpp"

#include "../../src/systems/dungeon_config_loader.hpp"

using namespace mion;

static void dungeon_config_loader_reads_expected_data_files() {
    EnemyDef enemy_defs[kEnemyTypeCount]{};
    DropConfig drop_config{};
    IniData rooms_ini{};
    std::array<std::string, kEnemyTypeCount> enemy_sprite_paths{};

    DungeonConfigLoader::load_dungeon_static_data(
        enemy_defs, drop_config, rooms_ini, enemy_sprite_paths);

    EXPECT_GT(g_player_config.base_hp, 0);
    EXPECT_GT(enemy_defs[static_cast<int>(EnemyType::Skeleton)].max_hp, 0);
    EXPECT_TRUE(enemy_defs[static_cast<int>(EnemyType::BossGrimjaw)].sprite_sheet_path != nullptr);
    EXPECT_EQ(drop_config.drop_chance_pct, 50);
    EXPECT_NEAR(drop_config.pickup_radius, 36.0f, 0.001f);
    EXPECT_TRUE(!rooms_ini.sections.empty());
    EXPECT_TRUE(rooms_ini.sections.find("arena") != rooms_ini.sections.end());
}

REGISTER_TEST(dungeon_config_loader_reads_expected_data_files);
