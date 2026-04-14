#include "../test_common.hpp"
#include <vector>
#include "entities/actor.hpp"
#include "systems/spell_effects.hpp"

static void test_nova_hits_only_in_radius() {
    mion::Actor caster;
    caster.team = mion::Team::Player;
    caster.transform.set_position(0.0f, 0.0f);
    caster.is_alive = true;

    mion::Actor near_enemy;
    near_enemy.team = mion::Team::Enemy;
    near_enemy.transform.set_position(30.0f, 0.0f);
    near_enemy.health = { 100, 100 };
    near_enemy.is_alive = true;

    mion::Actor far_enemy;
    far_enemy.team = mion::Team::Enemy;
    far_enemy.transform.set_position(200.0f, 0.0f);
    far_enemy.health = { 100, 100 };
    far_enemy.is_alive = true;

    std::vector<mion::Actor*> actors = { &caster, &near_enemy, &far_enemy };
    mion::apply_nova(caster, actors, 80.0f, 20);

    EXPECT_EQ(near_enemy.health.current_hp, 80);
    EXPECT_EQ(far_enemy.health.current_hp, 100);
}
REGISTER_TEST(test_nova_hits_only_in_radius);
