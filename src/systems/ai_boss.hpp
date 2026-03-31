#pragma once
#include <algorithm>

#include "ai_combat.hpp"

namespace mion::enemy_ai {

inline constexpr float kBossChargeSpeed    = 480.0f;
inline constexpr float kBossChargeDuration = 0.45f;
inline constexpr float kBossChargeCooldown = 4.0f;

inline void update_boss(Actor* enemy, Actor* player, float dx, float dy, float dist,
                        float dt, Pathfinder* pathfinder) {
    const int   max_hp = std::max(1, enemy->health.max_hp);
    const float half   = static_cast<float>(max_hp) * 0.5f;
    if (enemy->boss_phase == 1 && enemy->health.current_hp < half) {
        enemy->boss_phase = 2;
        enemy->move_speed = enemy->base_move_speed_at_spawn * 1.4f;
        enemy->attack_damage += 8;
        enemy->boss_charge_cd = 0.5f;
    }

    if (enemy->boss_charging) {
        enemy->transform.translate(enemy->boss_charge_dir_x * kBossChargeSpeed * dt,
                                   enemy->boss_charge_dir_y * kBossChargeSpeed * dt);
        enemy->boss_charge_remaining -= dt;
        enemy->is_moving = true;
        if (enemy->boss_charge_remaining <= 0.0f)
            enemy->boss_charging = false;
        return;
    }

    enemy->boss_charge_cd -= dt;
    if (enemy->boss_phase >= 2 && enemy->boss_charge_cd <= 0.0f
        && enemy->combat.is_attack_idle() && dist > kDistanceEpsilon) {
        enemy->boss_charging         = true;
        enemy->boss_charge_remaining = kBossChargeDuration;
        enemy->boss_charge_dir_x     = dx / dist;
        enemy->boss_charge_dir_y     = dy / dist;
        enemy->boss_charge_cd        = kBossChargeCooldown;
        enemy->set_facing(enemy->boss_charge_dir_x, enemy->boss_charge_dir_y);
        return;
    }

    chase_and_melee_attack(enemy, player, dx, dy, dist, dt, pathfinder);
}

} // namespace mion::enemy_ai
