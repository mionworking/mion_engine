#include "../test_common.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#include "../../src/core/save_data.hpp"
#include "../../src/core/save_migration.hpp"
#include "../../src/core/save_system.hpp"

using namespace mion;

static void save_v8_roundtrips_karma_fields() {
    const std::string path = "/tmp/mion_engine_save_v8_roundtrip.txt";
    SaveData w{};
    w.version         = kSaveFormatVersion;
    w.gold            = 50;
    w.karma_total     = 1234;
    w.karma_available = 567;

    EXPECT_TRUE(SaveSystem::save(path, w));
    SaveData r{};
    EXPECT_TRUE(SaveSystem::load(path, r));
    EXPECT_EQ(r.version, kSaveFormatVersion);
    EXPECT_EQ(r.gold, 50);
    EXPECT_EQ(r.karma_total,     (int64_t)1234);
    EXPECT_EQ(r.karma_available, (int64_t)567);
    std::remove(path.c_str());
}
REGISTER_TEST(save_v8_roundtrips_karma_fields);

static void save_migration_v7_to_v8_zeros_karma() {
    SaveData d{};
    d.version         = 7;
    d.karma_total     = 9999;
    d.karma_available = 9999;
    d                 = SaveMigration::migrate_v7_to_v8(d);
    EXPECT_EQ(d.version, kSaveFormatVersion);
    EXPECT_EQ(d.karma_total,     (int64_t)0);
    EXPECT_EQ(d.karma_available, (int64_t)0);
}
REGISTER_TEST(save_migration_v7_to_v8_zeros_karma);

static void save_load_v7_file_migrates_karma() {
    const char* path = "/tmp/mion_engine_save_v7_probe.txt";
    {
        std::ofstream f(path, std::ios::trunc);
        f << "version=7\nroom_index=0\nhp=100\ngold=10\n";
        f << "xp=0\nlevel=1\nxp_to_next=100\npending_level_ups=0\n";
        f << "bonus_attack_damage=0\nbonus_max_hp=0\nbonus_move_speed=0\nspell_damage_multiplier=1\n";
        f << "talent_pending_points=0\n";
        for (int i = 0; i < kTalentCount; ++i) f << "talent_" << i << "=0\n";
        f << "mana_current=100\nmana_max=100\nmana_regen_rate=20\nmana_regen_delay=0.75\nmana_regen_delay_rem=0\n";
        f << "stamina_current=100\nstamina_max=100\nstamina_regen_rate=30\nstamina_regen_delay=1\nstamina_regen_delay_rem=0\n";
        f << "victory_reached=0\ndifficulty=1\n";
        f << "last_rooms_cleared=0\nlast_enemies_killed=0\nlast_gold_collected=0\n";
        f << "last_damage_taken=0\nlast_spells_cast=0\nlast_potions_used=0\n";
        f << "last_time_seconds=0\nlast_max_level=1\nlast_boss_defeated=0\n";
        f << "attr_vigor=0\nattr_forca=0\nattr_destreza=0\nattr_inteligencia=0\nattr_endurance=0\n";
        f << "attr_points_available=0\nscene_flags=0\n";
        f << "player_world_x=0\nplayer_world_y=0\nvisited_area_mask=0\n";
        f << "potion_stack=0\npotion_quality=1\n";
        for (int qi = 0; qi < static_cast<int>(QuestId::Count); ++qi) f << "quest_" << qi << "=0\n";
    }
    SaveData d{};
    EXPECT_TRUE(SaveSystem::load(path, d));
    EXPECT_EQ(d.version, kSaveFormatVersion);
    EXPECT_EQ(d.karma_total,     (int64_t)0);
    EXPECT_EQ(d.karma_available, (int64_t)0);
    std::remove(path);
}
REGISTER_TEST(save_load_v7_file_migrates_karma);

static void save_v8_negative_karma_clamps_to_zero() {
    const char* path = "/tmp/mion_engine_save_v8_neg.txt";
    {
        std::ofstream f(path, std::ios::trunc);
        f << "version=8\nkarma_total=-5\nkarma_available=-10\n";
    }
    SaveData d{};
    EXPECT_TRUE(SaveSystem::load(path, d));
    EXPECT_EQ(d.karma_total,     (int64_t)0);
    EXPECT_EQ(d.karma_available, (int64_t)0);
    std::remove(path);
}
REGISTER_TEST(save_v8_negative_karma_clamps_to_zero);

static void save_v8_available_exceeds_total_clamps() {
    const char* path = "/tmp/mion_engine_save_v8_exceeds.txt";
    {
        std::ofstream f(path, std::ios::trunc);
        f << "version=8\nkarma_total=100\nkarma_available=500\n";
    }
    SaveData d{};
    EXPECT_TRUE(SaveSystem::load(path, d));
    EXPECT_EQ(d.karma_total,     (int64_t)100);
    EXPECT_EQ(d.karma_available, (int64_t)100);
    std::remove(path);
}
REGISTER_TEST(save_v8_available_exceeds_total_clamps);
