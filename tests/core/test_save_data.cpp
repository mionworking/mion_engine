#include "../test_common.hpp"
#include "core/save_data.hpp"
#include "core/save_system.hpp"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <system_error>

static void test_save_roundtrip_meta_run_fields() {
    mion::SaveData d{};
    d.gold             = 12;
    d.victory_reached  = true;
    d.difficulty       = 2;
    d.last_run_stats.enemies_killed = 7;
    d.last_run_stats.time_seconds   = 99.5f;
    d.last_run_stats.boss_defeated  = true;
    const char* path = "mion_save_meta_run_test.txt";
    std::remove(path);
    EXPECT_TRUE(mion::SaveSystem::save(path, d));
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_TRUE(out.victory_reached);
    EXPECT_EQ(out.difficulty, 2);
    EXPECT_EQ(out.last_run_stats.enemies_killed, 7);
    EXPECT_NEAR(out.last_run_stats.time_seconds, 99.5f, 0.05f);
    EXPECT_TRUE(out.last_run_stats.boss_defeated);
    std::remove(path);
}
REGISTER_TEST(test_save_roundtrip_meta_run_fields);

static void test_save_roundtrip() {
    mion::SaveData d;
    d.room_index  = 2;
    d.player_hp   = 77;
    d.progression.level = 4;
    d.progression.xp    = 50;
    d.progression.xp_to_next = 200;
    d.progression.pending_level_ups = 1;
    d.progression.bonus_attack_damage = 6;
    d.progression.bonus_max_hp        = 30;
    d.progression.bonus_move_speed    = 36.0f;
    d.progression.spell_damage_multiplier = 1.25f;
    d.talents.pending_points = 2;
    d.talents.levels[0]      = 2;
    d.talents.levels[1]      = 0;
    d.mana.current    = 80.0f;
    d.mana.max        = 130.0f;
    d.mana.regen_rate = 25.0f;
    d.stamina.current = 90.0f;
    d.gold              = 142;
    d.quest_state.set(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::InProgress);

    const char* path = "mion_save_roundtrip_test.txt";
    std::remove(path);
    EXPECT_TRUE(mion::SaveSystem::save(path, d));
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.room_index, 2);
    EXPECT_EQ(out.player_hp, 77);
    EXPECT_EQ(out.progression.level, 4);
    EXPECT_EQ(out.progression.xp, 50);
    EXPECT_EQ(out.progression.pending_level_ups, 1);
    EXPECT_EQ(out.talents.levels[0], 2);
    EXPECT_EQ(out.talents.levels[1], 0);
    EXPECT_NEAR(out.mana.current, 80.0f, 0.01f);
    EXPECT_NEAR(out.mana.max, 130.0f, 0.01f);
    EXPECT_EQ(out.gold, 142);
    EXPECT_TRUE(out.quest_state.is(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::InProgress));
    std::remove(path);
}
REGISTER_TEST(test_save_roundtrip);

static void test_save_v3_loads_talent_int_levels_from_disk() {
    const char* path = "mion_save_v3_talent_int_test.txt";
    std::remove(path);
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("version=3\nroom_index=0\nhp=100\nxp=0\nlevel=1\nxp_to_next=100\n"
                   "pending_level_ups=0\nbonus_attack_damage=0\nbonus_max_hp=0\n"
                   "bonus_move_speed=0\nspell_damage_multiplier=1\n"
                   "talent_pending_points=0\n",
                   fp);
        for (int i = 0; i < mion::kTalentCount; ++i) {
            int v = (i == 7) ? 3 : ((i == 2) ? 1 : 0);
            std::fprintf(fp, "talent_%d=%d\n", i, v);
        }
        std::fputs("mana_current=100\nmana_max=100\nmana_regen_rate=20\n"
                   "mana_regen_delay=0.75\nmana_regen_delay_rem=0\n"
                   "stamina_current=100\nstamina_max=100\n"
                   "stamina_regen_rate=30\nstamina_regen_delay=1\n"
                   "stamina_regen_delay_rem=0\n",
                   fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.talents.levels[7], 3);
    EXPECT_EQ(out.talents.levels[2], 1);
    std::remove(path);
}
REGISTER_TEST(test_save_v3_loads_talent_int_levels_from_disk);

static void test_save_v1_file_loads_and_promotes_to_v2() {
    const char* path = "mion_save_v1_promote_test.txt";
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("version=1\nroom_index=5\nhp=10\nxp=0\nlevel=1\nxp_to_next=100\n"
                   "pending_level_ups=0\nbonus_attack_damage=0\nbonus_max_hp=0\n"
                   "bonus_move_speed=0\nspell_damage_multiplier=1\n"
                   "talent_pending_points=0\n",
                   fp);
        for (int i = 0; i < mion::kTalentCount; ++i)
            std::fprintf(fp, "talent_%d=0\n", i);
        for (int i = 0; i < mion::kSpellCount; ++i)
            std::fprintf(fp, "spell_%d=%d\n", i, i == 0 ? 1 : 0);
        std::fputs("mana_current=100\nmana_max=100\nmana_regen_rate=20\n"
                   "mana_regen_delay=0.75\nmana_regen_delay_rem=0\n"
                   "stamina_current=100\nstamina_max=100\n"
                   "stamina_regen_rate=30\nstamina_regen_delay=1\n"
                   "stamina_regen_delay_rem=0\n",
                   fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.room_index, 5);
    std::remove(path);
}
REGISTER_TEST(test_save_v1_file_loads_and_promotes_to_v2);

static void test_save_clamps_room_index_past_max() {
    const char* path = "mion_save_clamp_room_test.txt";
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fprintf(fp,
                     "version=2\nroom_index=%d\nhp=10\nxp=0\nlevel=1\nxp_to_next=100\n"
                     "pending_level_ups=0\nbonus_attack_damage=0\nbonus_max_hp=0\n"
                     "bonus_move_speed=0\nspell_damage_multiplier=1\n"
                     "talent_pending_points=0\n",
                     mion::kSaveMaxRoomIndex + 50);
        for (int i = 0; i < mion::kTalentCount; ++i)
            std::fprintf(fp, "talent_%d=0\n", i);
        for (int i = 0; i < mion::kSpellCount; ++i)
            std::fprintf(fp, "spell_%d=%d\n", i, i == 0 ? 1 : 0);
        std::fputs("mana_current=100\nmana_max=100\nmana_regen_rate=20\n"
                   "mana_regen_delay=0.75\nmana_regen_delay_rem=0\n"
                   "stamina_current=100\nstamina_max=100\n"
                   "stamina_regen_rate=30\nstamina_regen_delay=1\n"
                   "stamina_regen_delay_rem=0\n",
                   fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.room_index, mion::kSaveMaxRoomIndex);
    std::remove(path);
}
REGISTER_TEST(test_save_clamps_room_index_past_max);

static void test_save_rejects_bad_version() {
    const char* path = "mion_save_badver_test.txt";
    {
        std::FILE* fp = std::fopen(path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("version=99\nroom_index=1\nhp=10\n", fp);
        std::fclose(fp);
    }
    mion::SaveData out{};
    EXPECT_FALSE(mion::SaveSystem::load(path, out));
    std::remove(path);
}
REGISTER_TEST(test_save_rejects_bad_version);

static void test_save_load_candidates_falls_back_to_legacy() {
    const char* primary = "mion_save_primary_missing_test.txt";
    const char* legacy  = "mion_save_legacy_present_test.txt";
    std::remove(primary);
    std::remove(legacy);

    mion::SaveData d;
    d.room_index = 4;
    d.player_hp  = 63;
    d.progression.level = 3;
    EXPECT_TRUE(mion::SaveSystem::save(legacy, d));

    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load_candidates(primary, legacy, out));
    EXPECT_EQ(out.room_index, 4);
    EXPECT_EQ(out.player_hp, 63);
    EXPECT_EQ(out.progression.level, 3);

    std::remove(primary);
    std::remove(legacy);
}
REGISTER_TEST(test_save_load_candidates_falls_back_to_legacy);

static void test_save_load_candidates_prefers_primary() {
    const char* primary = "mion_save_primary_present_test.txt";
    const char* legacy  = "mion_save_legacy_present_test.txt";
    std::remove(primary);
    std::remove(legacy);

    mion::SaveData primary_data;
    primary_data.room_index = 7;
    EXPECT_TRUE(mion::SaveSystem::save(primary, primary_data));

    mion::SaveData legacy_data;
    legacy_data.room_index = 2;
    EXPECT_TRUE(mion::SaveSystem::save(legacy, legacy_data));

    // Deterministic preference: when both files exist, newer mtime wins.
    // Force primary newer to avoid filesystem timestamp granularity flakes.
    namespace fs = std::filesystem;
    std::error_code ec;
    const auto now = fs::file_time_type::clock::now();
    fs::last_write_time(legacy, now - std::chrono::seconds(2), ec);
    EXPECT_TRUE(!ec);
    ec.clear();
    fs::last_write_time(primary, now, ec);
    EXPECT_TRUE(!ec);

    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load_candidates(primary, legacy, out));
    EXPECT_EQ(out.room_index, 7);

    std::remove(primary);
    std::remove(legacy);
}
REGISTER_TEST(test_save_load_candidates_prefers_primary);

static void test_save_remove_candidates_clears_primary_and_legacy() {
    const char* primary = "mion_save_remove_primary_test.txt";
    const char* legacy  = "mion_save_remove_legacy_test.txt";
    std::remove(primary);
    std::remove(legacy);

    mion::SaveData d;
    EXPECT_TRUE(mion::SaveSystem::save(primary, d));
    EXPECT_TRUE(mion::SaveSystem::save(legacy, d));
    EXPECT_TRUE(mion::SaveSystem::exists_candidates(primary, legacy));

    EXPECT_TRUE(mion::SaveSystem::remove_candidates(primary, legacy));
    EXPECT_FALSE(mion::SaveSystem::exists(primary));
    EXPECT_FALSE(mion::SaveSystem::exists(legacy));
}
REGISTER_TEST(test_save_remove_candidates_clears_primary_and_legacy);
