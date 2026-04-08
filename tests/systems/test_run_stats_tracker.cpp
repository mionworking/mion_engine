#include "../test_common.hpp"

#include "../../src/systems/run_stats_tracker.hpp"

using namespace mion;

static void run_stats_tracker_damage_only_sums_melee_and_projectile() {
    const std::vector<CombatEvent> melee = {
        {"orc", "player", 7},
        {"orc", "ally", 5},
    };
    const std::vector<ProjectileSystem::HitEvent> projectiles = {
        {"player", 4},
        {"ally", 9},
    };

    const int damage = RunStatsTracker::damage_taken_for_actor("player", melee, projectiles);
    EXPECT_EQ(damage, 11);
}

static void run_stats_tracker_heal_only_frame_adds_no_damage() {
    const std::vector<CombatEvent> melee = {};
    const std::vector<ProjectileSystem::HitEvent> projectiles = {};

    // Simula frame com cura sem eventos de dano: o tracker deve manter dano em zero.
    const int damage = RunStatsTracker::damage_taken_for_actor("player", melee, projectiles);
    EXPECT_EQ(damage, 0);
}

static void run_stats_tracker_damage_and_heal_same_frame_uses_events() {
    const std::vector<CombatEvent> melee = {
        {"orc", "player", 6},
    };
    const std::vector<ProjectileSystem::HitEvent> projectiles = {
        {"player", 5},
    };

    // Mesmo que uma cura aconteça no mesmo frame e HP final nao mude, o dano recebido
    // deve ser o somatorio dos eventos de dano.
    const int damage = RunStatsTracker::damage_taken_for_actor("player", melee, projectiles);
    EXPECT_EQ(damage, 11);
}

REGISTER_TEST(run_stats_tracker_damage_only_sums_melee_and_projectile);
REGISTER_TEST(run_stats_tracker_heal_only_frame_adds_no_damage);
REGISTER_TEST(run_stats_tracker_damage_and_heal_same_frame_uses_events);
