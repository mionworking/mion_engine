#include "../test_common.hpp"
#include "core/save_data.hpp"
#include "core/save_system.hpp"
#include <cstdio>

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
