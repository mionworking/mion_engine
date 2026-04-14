#include "../test_common.hpp"
#include "systems/enemy_ai.hpp"
#include "entities/actor.hpp"
#include "entities/enemy_type.hpp"
#include "entities/projectile.hpp"
#include <cmath>
#include <vector>

static void test_enemy_ai_chases_player_without_pathfinder() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(300.0f, 100.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.move_speed = 120.0f;
    enemy.enemy_ai = mion::EnemyAIData{};
    enemy.enemy_ai->aggro_range = 500.0f;
    enemy.enemy_ai->attack_range_ai = 40.0f;
    enemy.enemy_ai->stop_range_ai = 25.0f;
    enemy.transform.set_position(100.0f, 100.0f);
    enemy.combat.reset_for_spawn();
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 30.0f, nullptr);
    EXPECT_TRUE(enemy.transform.x > 100.0f);
    EXPECT_TRUE(enemy.is_moving);
}
REGISTER_TEST(test_enemy_ai_chases_player_without_pathfinder);

static void test_enemy_ai_respects_stop_range_no_move() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(10.0f, 0.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.move_speed = 100.0f;
    enemy.enemy_ai = mion::EnemyAIData{};
    enemy.enemy_ai->aggro_range = 500.0f;
    enemy.enemy_ai->attack_range_ai = 50.0f;
    enemy.enemy_ai->stop_range_ai = 30.0f;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.combat.reset_for_spawn();
    float x0 = enemy.transform.x;
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr);
    EXPECT_NEAR(enemy.transform.x, x0, 2.0f);
    EXPECT_FALSE(enemy.is_moving);
}
REGISTER_TEST(test_enemy_ai_respects_stop_range_no_move);

static void test_enemy_ai_begins_attack_when_in_range() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(40.0f, 0.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.move_speed = 0.0f;
    enemy.enemy_ai = mion::EnemyAIData{};
    enemy.enemy_ai->aggro_range = 500.0f;
    enemy.enemy_ai->attack_range_ai = 50.0f;
    enemy.enemy_ai->stop_range_ai = 10.0f;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.combat.reset_for_spawn();
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr);
    EXPECT_EQ(enemy.combat.attack_phase, mion::AttackPhase::Startup);
}
REGISTER_TEST(test_enemy_ai_begins_attack_when_in_range);

static void test_enemy_def_boss_grimjaw_boss_phased_and_gold() {
    const mion::EnemyDef& d = mion::get_enemy_def(mion::EnemyType::BossGrimjaw);
    EXPECT_TRUE(d.ai_behavior == mion::AiBehavior::BossPhased);
    EXPECT_TRUE(d.gold_drop_min >= 25);
    EXPECT_TRUE(d.gold_drop_max >= d.gold_drop_min);
}
REGISTER_TEST(test_enemy_def_boss_grimjaw_boss_phased_and_gold);

static void test_parse_ai_behavior_string_variants() {
    EXPECT_TRUE(mion::parse_ai_behavior_string("melee") == mion::AiBehavior::Melee);
    EXPECT_TRUE(mion::parse_ai_behavior_string("RANGED") == mion::AiBehavior::Ranged);
    EXPECT_TRUE(mion::parse_ai_behavior_string("boss_phased") == mion::AiBehavior::BossPhased);
}
REGISTER_TEST(test_parse_ai_behavior_string_variants);

static void test_enemy_ai_patrol_alert_neighbor_chase() {
    mion::EnemyAISystem sys;
    mion::Actor         player, guard_a, guard_b;
    player.team     = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(100.0f, 0.0f);
    guard_a.team         = mion::Team::Enemy;
    guard_a.is_alive     = true;
    guard_a.enemy_ai     = mion::EnemyAIData{};
    guard_a.enemy_ai->ai_behavior   = mion::AiBehavior::Patrol;
    guard_a.enemy_ai->aggro_range   = 280.0f;
    guard_a.move_speed              = 60.0f;
    guard_a.enemy_ai->patrol_state  = mion::PatrolState::Patrol;
    guard_a.enemy_ai->patrol_waypoints = {{0.0f, 0.0f}, {20.0f, 0.0f}};
    guard_a.transform.set_position(0.0f, 0.0f);
    guard_a.combat.reset_for_spawn();
    guard_b = guard_a;
    guard_b.transform.set_position(40.0f, 0.0f);
    std::vector<mion::Actor*> actors = {&player, &guard_a, &guard_b};
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr, nullptr);
    EXPECT_TRUE(guard_a.enemy_ai->patrol_state == mion::PatrolState::Alert
                || guard_a.enemy_ai->patrol_state == mion::PatrolState::Chase);
    EXPECT_TRUE(guard_b.enemy_ai->patrol_state == mion::PatrolState::Chase);
}
REGISTER_TEST(test_enemy_ai_patrol_alert_neighbor_chase);

static void test_enemy_ai_boss_phase2_on_low_hp() {
    mion::EnemyAISystem sys;
    mion::Actor         player, boss;
    player.team     = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(200.0f, 100.0f);
    boss.team       = mion::Team::Enemy;
    boss.is_alive   = true;
    boss.enemy_ai   = mion::EnemyAIData{};
    boss.enemy_ai->ai_behavior             = mion::AiBehavior::BossPhased;
    boss.enemy_ai->aggro_range             = 800.0f;
    boss.enemy_ai->attack_range_ai         = 40.0f;
    boss.enemy_ai->stop_range_ai           = 20.0f;
    boss.move_speed                        = 50.0f;
    boss.enemy_ai->base_move_speed_at_spawn = 50.0f;
    boss.health.max_hp     = 200;
    boss.health.current_hp = 80;
    boss.enemy_ai->boss_phase = 1;
    boss.combat.reset_for_spawn();
    boss.transform.set_position(100.0f, 100.0f);
    std::vector<mion::Actor*> actors = {&player, &boss};
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr, nullptr);
    EXPECT_EQ(boss.enemy_ai->boss_phase, 2);
    EXPECT_GT(boss.move_speed, 60.0f);
}
REGISTER_TEST(test_enemy_ai_boss_phase2_on_low_hp);

static void test_enemy_ai_ranged_fires_projectile_when_cd_ready() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(220.0f, 100.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.enemy_ai = mion::EnemyAIData{};
    enemy.enemy_ai->ai_behavior        = mion::AiBehavior::Ranged;
    enemy.enemy_ai->aggro_range        = 500.0f;
    enemy.enemy_ai->ranged_fire_rate   = 0.2f;
    enemy.enemy_ai->ranged_fire_cd     = 0.0f;
    enemy.enemy_ai->ranged_proj_damage = 7;
    enemy.move_speed                   = 0.0f;
    enemy.enemy_ai->stop_range_ai      = 10.0f;
    enemy.enemy_ai->attack_range_ai    = 40.0f;
    enemy.transform.set_position(100.0f, 100.0f);
    enemy.combat.reset_for_spawn();
    std::vector<mion::Projectile> pr;
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 30.0f, nullptr, &pr);
    EXPECT_FALSE(pr.empty());
    EXPECT_TRUE(pr[0].owner_team == mion::Team::Enemy);
}
REGISTER_TEST(test_enemy_ai_ranged_fires_projectile_when_cd_ready);

static void test_enemy_ai_overlap_preserves_finite_facing() {
    mion::Actor player, enemy;

    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(100.0f, 100.0f);

    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.transform.set_position(100.0f, 100.0f);  // mesma posição do player
    enemy.facing_x = 0.0f;
    enemy.facing_y = -1.0f;
    enemy.enemy_ai = mion::EnemyAIData{};
    enemy.enemy_ai->aggro_range     = 400.0f;
    enemy.enemy_ai->attack_range_ai = 45.0f;
    enemy.enemy_ai->stop_range_ai   = 30.0f;
    enemy.combat.reset_for_spawn();

    mion::EnemyAISystem sys;
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 60.0f);

    EXPECT_TRUE(std::isfinite(enemy.facing_x));
    EXPECT_TRUE(std::isfinite(enemy.facing_y));
    EXPECT_EQ(enemy.facing_x, 0.0f);
    EXPECT_EQ(enemy.facing_y, -1.0f);
    EXPECT_EQ(enemy.combat.attack_phase, mion::AttackPhase::Startup);
}
REGISTER_TEST(test_enemy_ai_overlap_preserves_finite_facing);
