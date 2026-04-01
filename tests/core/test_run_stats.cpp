#include "../test_common.hpp"
#include "core/run_stats.hpp"

static void test_run_stats_reset_clears_fields() {
    mion::RunStats r;
    r.enemies_killed = 5;
    r.time_seconds   = 12.5f;
    r.boss_defeated  = true;
    r.reset();
    EXPECT_EQ(r.enemies_killed, 0);
    EXPECT_NEAR(r.time_seconds, 0.0f, 0.001f);
    EXPECT_FALSE(r.boss_defeated);
}
REGISTER_TEST(test_run_stats_reset_clears_fields);
