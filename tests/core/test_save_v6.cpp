#include "../test_common.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#include "../../src/core/save_data.hpp"
#include "../../src/core/save_migration.hpp"
#include "../../src/core/save_system.hpp"

using namespace mion;

static void save_v6_roundtrips_world_fields() {
    std::string path = "/tmp/mion_engine_save_v6_roundtrip.txt";
    SaveData w{};
    w.version            = kSaveFormatVersion;
    w.gold               = 77;
    w.player_world_x     = 1234.5f;
    w.player_world_y     = -99.25f;
    w.visited_area_mask  = 0x0A;
    w.victory_reached    = true;

    EXPECT_TRUE(SaveSystem::save(path, w));
    SaveData r{};
    EXPECT_TRUE(SaveSystem::load(path, r));
    EXPECT_EQ(r.version, kSaveFormatVersion);
    EXPECT_EQ(r.gold, 77);
    EXPECT_NEAR(r.player_world_x, 1234.5f, 0.01f);
    EXPECT_NEAR(r.player_world_y, -99.25f, 0.01f);
    EXPECT_EQ(static_cast<int>(r.visited_area_mask), 0x0A);
    EXPECT_TRUE(r.victory_reached);
    std::remove(path.c_str());
}

static void save_migration_v5_to_v6_zeros_world_fields_keeps_gold() {
    SaveData d{};
    d.version           = 5;
    d.gold              = 42;
    d.player_world_x    = 999.f;
    d.player_world_y    = 888.f;
    d.visited_area_mask = 7;
    d                   = SaveMigration::migrate_v5_to_v6(d);
    EXPECT_EQ(d.version, kSaveFormatVersion);
    EXPECT_EQ(d.gold, 42);
    EXPECT_NEAR(d.player_world_x, 0.f, 0.01f);
    EXPECT_NEAR(d.player_world_y, 0.f, 0.01f);
    EXPECT_EQ(static_cast<int>(d.visited_area_mask), 0);
}

static void save_load_v5_file_migrates_world_fields() {
    const char* path = "/tmp/mion_engine_save_v5_probe.txt";
    {
        std::ofstream f(path, std::ios::trunc);
        f << "version=5\n";
        f << "room_index=2\n";
        f << "hp=100\n";
        f << "gold=33\n";
        f << "xp=0\n";
        f << "level=1\n";
        f << "xp_to_next=100\n";
        f << "pending_level_ups=0\n";
        f << "bonus_attack_damage=0\n";
        f << "bonus_max_hp=0\n";
        f << "bonus_move_speed=0\n";
        f << "spell_damage_multiplier=1\n";
        f << "talent_pending_points=0\n";
        for (int i = 0; i < kTalentCount; ++i)
            f << "talent_" << i << "=0\n";
        f << "mana_current=100\nmana_max=100\n";
        f << "mana_regen_rate=20\nmana_regen_delay=0.75\nmana_regen_delay_rem=0\n";
        f << "stamina_current=100\nstamina_max=100\n";
        f << "stamina_regen_rate=30\nstamina_regen_delay=1\nstamina_regen_delay_rem=0\n";
        f << "victory_reached=0\ndifficulty=1\n";
        f << "last_rooms_cleared=0\nlast_enemies_killed=0\nlast_gold_collected=0\n";
        f << "last_damage_taken=0\nlast_spells_cast=0\nlast_potions_used=0\n";
        f << "last_time_seconds=0\nlast_max_level=1\nlast_boss_defeated=0\n";
        f << "attr_vigor=0\nattr_forca=0\nattr_destreza=0\nattr_inteligencia=0\nattr_endurance=0\n";
        f << "attr_points_available=0\nscene_flags=0\n";
        for (int qi = 0; qi < static_cast<int>(QuestId::Count); ++qi)
            f << "quest_" << qi << "=0\n";
    }
    SaveData d{};
    EXPECT_TRUE(SaveSystem::load(path, d));
    EXPECT_EQ(d.version, kSaveFormatVersion);
    EXPECT_EQ(d.gold, 33);
    EXPECT_NEAR(d.player_world_x, 0.f, 0.01f);
    EXPECT_NEAR(d.player_world_y, 0.f, 0.01f);
    EXPECT_EQ(static_cast<int>(d.visited_area_mask), 0);
    std::remove(path);
}

REGISTER_TEST(save_v6_roundtrips_world_fields);
REGISTER_TEST(save_migration_v5_to_v6_zeros_world_fields_keeps_gold);
REGISTER_TEST(save_load_v5_file_migrates_world_fields);
