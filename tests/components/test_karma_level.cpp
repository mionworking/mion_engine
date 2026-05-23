#include <cmath>

#include "../test_common.hpp"
#include "components/progression.hpp"

static void test_karma_level_zero() {
    auto t = mion::default_karma_thresholds();
    EXPECT_EQ(mion::karma_level_for(0, t), 1);
}
REGISTER_TEST(test_karma_level_zero);

static void test_karma_level_below_l2_threshold() {
    auto t = mion::default_karma_thresholds();
    EXPECT_EQ(mion::karma_level_for(99, t), 1);
}
REGISTER_TEST(test_karma_level_below_l2_threshold);

static void test_karma_level_exact_threshold() {
    auto t = mion::default_karma_thresholds();
    EXPECT_EQ(mion::karma_level_for(100, t), 2);
    EXPECT_EQ(mion::karma_level_for(250, t), 3);
}
REGISTER_TEST(test_karma_level_exact_threshold);

static void test_karma_level_between_thresholds() {
    auto t = mion::default_karma_thresholds();
    // Entre L4 (500) e L5 (1000) → ainda L4.
    EXPECT_EQ(mion::karma_level_for(999, t), 4);
}
REGISTER_TEST(test_karma_level_between_thresholds);

static void test_karma_level_at_max_threshold() {
    auto t = mion::default_karma_thresholds();
    EXPECT_EQ(mion::karma_level_for(290000, t), 20);
}
REGISTER_TEST(test_karma_level_at_max_threshold);

static void test_karma_level_overshoot_clamps_to_max() {
    auto t = mion::default_karma_thresholds();
    EXPECT_EQ(mion::karma_level_for(99999999LL, t), 20);
}
REGISTER_TEST(test_karma_level_overshoot_clamps_to_max);

static void test_karma_level_empty_thresholds_returns_one() {
    std::vector<int64_t> empty;
    EXPECT_EQ(mion::karma_level_for(0, empty), 1);
    EXPECT_EQ(mion::karma_level_for(9999999LL, empty), 1);
}
REGISTER_TEST(test_karma_level_empty_thresholds_returns_one);

static void test_karma_level_negative_total_returns_one() {
    auto t = mion::default_karma_thresholds();
    // Defensive: -1 não deve crashar nem retornar 0.
    EXPECT_EQ(mion::karma_level_for(-1, t), 1);
}
REGISTER_TEST(test_karma_level_negative_total_returns_one);

static void test_karma_progress_ratio_empty_thresholds() {
    EXPECT_EQ(mion::karma_progress_ratio_for(100, {}), 0.0f);
}
REGISTER_TEST(test_karma_progress_ratio_empty_thresholds);

static void test_karma_progress_ratio_negative_total() {
    const std::vector<int64_t> t = { 0, 100, 250 };
    EXPECT_EQ(mion::karma_progress_ratio_for(-50, t), 0.0f);
}
REGISTER_TEST(test_karma_progress_ratio_negative_total);

static void test_karma_progress_ratio_at_level_start() {
    const std::vector<int64_t> t = { 0, 100, 250 };
    EXPECT_EQ(mion::karma_progress_ratio_for(0, t), 0.0f);
}
REGISTER_TEST(test_karma_progress_ratio_at_level_start);

static void test_karma_progress_ratio_exact_next_threshold() {
    const std::vector<int64_t> t = { 0, 100, 250 };
    EXPECT_EQ(mion::karma_progress_ratio_for(100, t), 0.0f);
}
REGISTER_TEST(test_karma_progress_ratio_exact_next_threshold);

static void test_karma_progress_ratio_mid_between_thresholds() {
    const std::vector<int64_t> t = { 0, 100, 250 };
    EXPECT_EQ(mion::karma_progress_ratio_for(50, t), 0.5f);
}
REGISTER_TEST(test_karma_progress_ratio_mid_between_thresholds);

static void test_karma_progress_ratio_above_max_threshold() {
    const std::vector<int64_t> t = { 0, 100, 250 };
    EXPECT_EQ(mion::karma_progress_ratio_for(999999, t), 1.0f);
}
REGISTER_TEST(test_karma_progress_ratio_above_max_threshold);

static void test_karma_progress_ratio_near_next_level() {
    const std::vector<int64_t> t = { 0, 100 };
    const float r = mion::karma_progress_ratio_for(99, t);
    EXPECT_TRUE(std::fabs(r - 0.99f) < 0.001f);
}
REGISTER_TEST(test_karma_progress_ratio_near_next_level);
