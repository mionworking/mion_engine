#pragma once
#include <vector>

#include "../entities/projectile.hpp"
#include "ai_boss.hpp"
#include "ai_combat.hpp"
#include "ai_patrol.hpp"

namespace mion {

struct EnemyAISystem {
    /// `nav_area_ox/oy` — offset da `WorldArea` cujo `pathfinder` foi passado (coords locais → mundo).
    void fixed_update(std::vector<Actor*>& actors, float dt, Pathfinder* pathfinder = nullptr,
                      std::vector<Projectile>* projectiles = nullptr,
                      float nav_area_ox = 0.f, float nav_area_oy = 0.f) {
        Actor* player = nullptr;
        for (auto* a : actors) {
            if (a->team == Team::Player && a->is_alive) {
                player = a;
                break;
            }
        }

        for (auto* enemy : actors) {
            if (enemy->team != Team::Enemy) continue;
            if (!enemy->is_alive) continue;
            if (enemy->combat.is_hurt_stunned()) continue;
            if (enemy->status_effects.has(StatusEffectType::Stun)) continue;

            enemy_ai::apply_separation(enemy, actors, dt);

            float dx = 0.0f, dy = 0.0f, dist = 1.0e9f;
            if (player) {
                dx   = player->transform.x - enemy->transform.x;
                dy   = player->transform.y - enemy->transform.y;
                dist = sqrtf(dx * dx + dy * dy);
            }

            switch (enemy->ai_behavior) {
            case AiBehavior::Melee:
            case AiBehavior::Elite:
                if (!player || dist > enemy->aggro_range) continue;
                enemy_ai::chase_and_melee_attack(
                    enemy, player, dx, dy, dist, dt, pathfinder, nav_area_ox, nav_area_oy);
                break;
            case AiBehavior::Ranged:
                if (!player || dist > enemy->aggro_range) continue;
                enemy_ai::update_ranged(enemy, player, dx, dy, dist, dt, pathfinder, projectiles,
                                        nav_area_ox, nav_area_oy);
                break;
            case AiBehavior::Patrol:
                if (player)
                    enemy_ai::update_patrol(enemy, player, dx, dy, dist, dt, actors, pathfinder,
                                            nav_area_ox, nav_area_oy);
                break;
            case AiBehavior::BossPhased:
                if (!player) continue;
                if (dist > enemy->aggro_range) {
                    enemy_ai::patrol_along_waypoints(
                        enemy, dt, pathfinder, nav_area_ox, nav_area_oy);
                    break;
                }
                enemy_ai::update_boss(
                    enemy, player, dx, dy, dist, dt, pathfinder, nav_area_ox, nav_area_oy);
                break;
            default:
                break;
            }
        }
    }
};

} // namespace mion
