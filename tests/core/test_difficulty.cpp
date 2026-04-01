#include "../test_common.hpp"
#include "core/run_stats.hpp"

static void test_difficulty_spawn_and_multipliers() {
    EXPECT_EQ(mion::difficulty_adjust_spawn_budget(5, mion::DifficultyLevel::Easy), 4);
    EXPECT_EQ(mion::difficulty_adjust_spawn_budget(1, mion::DifficultyLevel::Easy), 1);
    EXPECT_EQ(mion::difficulty_adjust_spawn_budget(3, mion::DifficultyLevel::Hard), 4);
    float hp, dm;
    mion::difficulty_enemy_multipliers(mion::DifficultyLevel::Easy, hp, dm);
    EXPECT_NEAR(hp, 0.75f, 0.001f);
    EXPECT_NEAR(dm, 0.75f, 0.001f);
    mion::difficulty_enemy_multipliers(mion::DifficultyLevel::Hard, hp, dm);
    EXPECT_NEAR(hp, 1.35f, 0.001f);
    EXPECT_NEAR(dm, 1.25f, 0.001f);
}
REGISTER_TEST(test_difficulty_spawn_and_multipliers);
