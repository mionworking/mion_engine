#include "../test_common.hpp"
#include "entities/actor.hpp"

static void test_actor_effective_move_speed_player_bonus_and_slow() {
    mion::Actor a;
    a.team = mion::Team::Player;
    a.move_speed = 100.0f;
    a.progression.bonus_move_speed = 50.0f;
    EXPECT_NEAR(a.effective_move_speed(), 150.0f, 0.001f);
    a.status_effects.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.5f);
    EXPECT_NEAR(a.effective_move_speed(), 75.0f, 0.001f);
}
REGISTER_TEST(test_actor_effective_move_speed_player_bonus_and_slow);

static void test_actor_is_dashing() {
    mion::Actor a;
    a.dash_active_remaining_seconds = 0.0f;
    EXPECT_FALSE(a.is_dashing());
    a.dash_active_remaining_seconds = 0.01f;
    EXPECT_TRUE(a.is_dashing());
}
REGISTER_TEST(test_actor_is_dashing);
