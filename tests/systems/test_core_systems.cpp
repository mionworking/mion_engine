#include "../test_common.hpp"
#include "systems/resource_system.hpp"
#include "systems/movement_system.hpp"
#include "systems/status_effect_system.hpp"
#include "entities/actor.hpp"
#include <vector>
#include <cmath>

static void test_resource_system_ticks_mana_and_stamina() {
    mion::Actor a;
    a.player = mion::PlayerData{};
    a.is_alive = true;
    a.player->mana.current = 10.0f;
    a.player->mana.max = 100.0f;
    a.player->mana.regen_rate = 20.0f;
    a.player->mana.regen_delay = 0.0f;
    a.player->stamina.current = 10.0f;
    a.player->stamina.max = 100.0f;
    a.player->stamina.regen_rate = 30.0f;
    a.player->stamina.regen_delay = 0.0f;
    mion::ResourceSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.fixed_update(actors, 1.0f);
    EXPECT_NEAR(a.player->mana.current, 30.0f, 0.01f);
    EXPECT_NEAR(a.player->stamina.current, 40.0f, 0.01f);
}
REGISTER_TEST(test_resource_system_ticks_mana_and_stamina);

static void test_resource_system_skips_dead() {
    mion::Actor a;
    a.player = mion::PlayerData{};
    a.is_alive = false;
    a.player->mana.current = 10.0f;
    mion::ResourceSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.fixed_update(actors, 1.0f);
    EXPECT_NEAR(a.player->mana.current, 10.0f, 0.001f);
}
REGISTER_TEST(test_resource_system_skips_dead);

static void test_movement_system_resets_moving_and_knockback() {
    mion::Actor a, b;
    a.is_alive = true;
    b.is_alive = true;
    a.is_moving = true;
    b.is_moving = true;
    a.transform.set_position(0.0f, 0.0f);
    a.apply_knockback_impulse(60.0f, 0.0f);
    mion::MovementSystem sys;
    std::vector<mion::Actor*> actors = { &a, &b };
    sys.fixed_update(actors, 1.0f / 60.0f);
    EXPECT_FALSE(a.is_moving);
    EXPECT_FALSE(b.is_moving);
    EXPECT_TRUE(std::fabs(a.transform.x) > 0.0f || std::fabs(a.knockback_vx) > 0.0f);
}
REGISTER_TEST(test_movement_system_resets_moving_and_knockback);

static void test_status_effect_system_poison_ticks() {
    mion::Actor a;
    a.is_alive = true;
    a.health = { 100, 100 };
    a.status_effects.apply_full(mion::StatusEffectType::Poison, 2.0f, 0.1f, 7, 0);
    mion::StatusEffectSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.fixed_update(actors, 0.25f);
    EXPECT_TRUE(a.health.current_hp < 100);
    EXPECT_TRUE(a.is_alive);
}
REGISTER_TEST(test_status_effect_system_poison_ticks);
