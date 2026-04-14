#include "../test_common.hpp"
#include "entities/actor.hpp"

static void test_actor_effective_move_speed_player_bonus_and_slow() {
    mion::Actor a;
    a.player = mion::PlayerData{};
    a.team = mion::Team::Player;
    a.move_speed = 100.0f;
    a.player->progression.bonus_move_speed = 50.0f;
    EXPECT_NEAR(a.effective_move_speed(), 150.0f, 0.001f);
    a.status_effects.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.5f);
    EXPECT_NEAR(a.effective_move_speed(), 75.0f, 0.001f);
}
REGISTER_TEST(test_actor_effective_move_speed_player_bonus_and_slow);

static void test_actor_is_dashing() {
    mion::Actor a;
    a.player = mion::PlayerData{};
    a.player->dash_active_remaining_seconds = 0.0f;
    EXPECT_FALSE(a.is_dashing());
    a.player->dash_active_remaining_seconds = 0.01f;
    EXPECT_TRUE(a.is_dashing());
}
REGISTER_TEST(test_actor_is_dashing);

static void test_knockback_decays() {
    mion::Actor a;
    a.is_alive = true;
    a.transform.set_position(100.0f, 100.0f);
    a.apply_knockback_impulse(300.0f, 0.0f);
    EXPECT_TRUE(a.knockback_vx > 0.0f);

    // Após muitos frames deve decair para zero
    for (int i = 0; i < 60; ++i)
        a.step_knockback(1.0f / 60.0f);

    EXPECT_NEAR(a.knockback_vx, 0.0f, 1.0f);
    EXPECT_NEAR(a.knockback_vy, 0.0f, 1.0f);
}
REGISTER_TEST(test_knockback_decays);
