#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include "../entities/actor.hpp"
#include "../entities/projectile.hpp"
#include "pathfinder.hpp"

namespace mion {

inline constexpr float kEnemyAiDistanceEpsilon = 0.0001f;
inline constexpr float kPatrolArrivePx            = 24.0f;
inline constexpr float kAlertDurationSec          = 1.0f;
inline constexpr float kPatrolAlertNeighborRadius = 200.0f;
inline constexpr float kBossChargeSpeed           = 480.0f;
inline constexpr float kBossChargeDuration        = 0.45f;
inline constexpr float kBossChargeCooldown        = 4.0f;
inline constexpr float kEnemyProjSpeed            = 340.0f;

inline void push_enemy_projectile(std::vector<Projectile>& list, Actor& shooter,
                                    Actor& target, int damage) {
    float dx = target.transform.x - shooter.transform.x;
    float dy = target.transform.y - shooter.transform.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < kEnemyAiDistanceEpsilon)
        return;
    Projectile p;
    p.x                            = shooter.transform.x;
    p.y                            = shooter.transform.y;
    p.vx                           = (dx / dist) * kEnemyProjSpeed;
    p.vy                           = (dy / dist) * kEnemyProjSpeed;
    p.damage                       = damage;
    p.owner_team                   = Team::Enemy;
    p.half_w                       = 4.0f;
    p.half_h                       = 4.0f;
    p.lifetime_remaining_seconds   = 3.2f;
    p.active                       = true;
    list.push_back(p);
}

struct EnemyAISystem {
    void fixed_update(std::vector<Actor*>& actors, float dt, Pathfinder* pathfinder = nullptr,
                      std::vector<Projectile>* projectiles = nullptr) {
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

            apply_separation(enemy, actors, dt);

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
                chase_and_melee_attack(enemy, player, dx, dy, dist, dt, pathfinder);
                break;
            case AiBehavior::Ranged:
                if (!player || dist > enemy->aggro_range) continue;
                update_ranged(enemy, player, dx, dy, dist, dt, pathfinder, projectiles);
                break;
            case AiBehavior::Patrol:
                if (player)
                    update_patrol(enemy, player, dx, dy, dist, dt, actors, pathfinder);
                break;
            case AiBehavior::BossPhased:
                if (!player) continue;
                if (dist > enemy->aggro_range) {
                    patrol_along_waypoints(enemy, dt, pathfinder);
                    break;
                }
                update_boss(enemy, player, dx, dy, dist, dt, pathfinder, projectiles);
                break;
            default:
                break;
            }
        }
    }

private:
    static void apply_separation(Actor* enemy, std::vector<Actor*>& actors, float dt) {
        float sep_x = 0.0f, sep_y = 0.0f;
        for (auto* other : actors) {
            if (other == enemy) continue;
            if (other->team != Team::Enemy || !other->is_alive) continue;
            float ox    = enemy->transform.x - other->transform.x;
            float oy    = enemy->transform.y - other->transform.y;
            float odist = sqrtf(ox * ox + oy * oy);
            const float min_sep = 36.0f;
            if (odist < min_sep && odist > 0.0f) {
                float push = (min_sep - odist) / min_sep;
                sep_x += (ox / odist) * push;
                sep_y += (oy / odist) * push;
            }
        }
        const float sep_strength = 60.0f;
        enemy->transform.translate(sep_x * sep_strength * dt, sep_y * sep_strength * dt);
    }

    static void alert_patrol_neighbors(Actor* source, std::vector<Actor*>& actors) {
        for (auto* other : actors) {
            if (other == source) continue;
            if (other->team != Team::Enemy || !other->is_alive) continue;
            if (other->ai_behavior != AiBehavior::Patrol) continue;
            float ox = other->transform.x - source->transform.x;
            float oy = other->transform.y - source->transform.y;
            if (ox * ox + oy * oy <= kPatrolAlertNeighborRadius * kPatrolAlertNeighborRadius) {
                other->patrol_state = PatrolState::Chase;
                other->alert_timer  = 0.0f;
            }
        }
    }

    static void replan_chase_path(Actor* enemy, Actor* player, Pathfinder* pathfinder,
                                  float dt) {
        if (!pathfinder) return;
        enemy->path_replan_timer -= dt;
        if (enemy->path_replan_timer <= 0.0f) {
            enemy->nav_path = pathfinder->find_path(
                enemy->transform.x, enemy->transform.y, player->transform.x,
                player->transform.y);
            enemy->path_replan_timer = 0.35f;
        }
    }

    static void move_along_path_toward(Actor* enemy, Actor* player, float& move_nx,
                                       float& move_ny, Pathfinder* pathfinder) {
        float dx = player->transform.x - enemy->transform.x;
        float dy = player->transform.y - enemy->transform.y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist > kEnemyAiDistanceEpsilon) {
            move_nx = dx / dist;
            move_ny = dy / dist;
        }
        if (!pathfinder) return;

        if (enemy->nav_path.valid && !enemy->nav_path.done()) {
            auto  wp  = enemy->nav_path.next_wp();
            float wdx = wp.x - enemy->transform.x;
            float wdy = wp.y - enemy->transform.y;
            float wd  = sqrtf(wdx * wdx + wdy * wdy);

            if (wd < 20.0f) {
                enemy->nav_path.advance();
                if (!enemy->nav_path.done()) {
                    wp  = enemy->nav_path.next_wp();
                    wdx = wp.x - enemy->transform.x;
                    wdy = wp.y - enemy->transform.y;
                    wd  = sqrtf(wdx * wdx + wdy * wdy);
                }
            }

            if (!enemy->nav_path.done() && wd > 0.0f) {
                move_nx = wdx / wd;
                move_ny = wdy / wd;
            }
        }
    }

    static void chase_and_melee_attack(Actor* enemy, Actor* player, float dx, float dy,
                                       float dist, float dt, Pathfinder* pathfinder) {
        if (enemy->boss_charging) return;

        replan_chase_path(enemy, player, pathfinder, dt);

        if (dist > enemy->stop_range_ai && enemy->combat.is_attack_idle()) {
            float move_nx = 0.0f, move_ny = 0.0f;
            move_along_path_toward(enemy, player, move_nx, move_ny, pathfinder);
            float spd = enemy->effective_move_speed();
            enemy->transform.translate(move_nx * spd * dt, move_ny * spd * dt);
            enemy->set_facing(move_nx, move_ny);
            enemy->is_moving = true;
        }

        if (dist <= enemy->attack_range_ai && enemy->combat.is_attack_idle()) {
            if (dist > kEnemyAiDistanceEpsilon)
                enemy->set_facing(dx / dist, dy / dist);
            enemy->combat.begin_attack();
        }
    }

    static void update_ranged(Actor* enemy, Actor* player, float dx, float dy, float dist,
                              float dt, Pathfinder* pathfinder,
                              std::vector<Projectile>* projectiles) {
        replan_chase_path(enemy, player, pathfinder, dt);

        float keep = std::max(80.0f, enemy->ranged_keep_dist);
        float move_nx = 0.0f, move_ny = 0.0f;

        if (dist > keep * 1.05f) {
            move_along_path_toward(enemy, player, move_nx, move_ny, pathfinder);
        } else if (dist < keep * 0.6f) {
            if (dist > kEnemyAiDistanceEpsilon) {
                move_nx = -dx / dist;
                move_ny = -dy / dist;
            }
        } else {
            if (dist > kEnemyAiDistanceEpsilon) {
                move_nx = -dy / dist;
                move_ny = dx / dist;
            }
        }

        if (enemy->combat.is_attack_idle()) {
            float spd = enemy->effective_move_speed() * 0.85f;
            enemy->transform.translate(move_nx * spd * dt, move_ny * spd * dt);
            enemy->set_facing(dx / std::max(dist, kEnemyAiDistanceEpsilon),
                              dy / std::max(dist, kEnemyAiDistanceEpsilon));
            enemy->is_moving = (move_nx != 0.0f || move_ny != 0.0f);
        }

        enemy->ranged_fire_cd -= dt;
        if (enemy->ranged_fire_cd <= 0.0f && projectiles && enemy->combat.is_attack_idle()
            && dist <= enemy->aggro_range) {
            push_enemy_projectile(*projectiles, *enemy, *player, enemy->ranged_proj_damage);
            enemy->ranged_fire_cd = std::max(0.2f, enemy->ranged_fire_rate);
        }
    }

    static void steer_on_nav_path(Actor* enemy, float goal_x, float goal_y, float& move_nx,
                                  float& move_ny) {
        float wdx = goal_x - enemy->transform.x;
        float wdy = goal_y - enemy->transform.y;
        float wd  = sqrtf(wdx * wdx + wdy * wdy);
        if (wd > kEnemyAiDistanceEpsilon) {
            move_nx = wdx / wd;
            move_ny = wdy / wd;
        }
        if (!enemy->nav_path.valid || enemy->nav_path.done()) return;
        auto  wp  = enemy->nav_path.next_wp();
        float pdx = wp.x - enemy->transform.x;
        float pdy = wp.y - enemy->transform.y;
        float pd  = sqrtf(pdx * pdx + pdy * pdy);
        if (pd < 20.0f) {
            enemy->nav_path.advance();
            if (!enemy->nav_path.done()) {
                wp  = enemy->nav_path.next_wp();
                pdx = wp.x - enemy->transform.x;
                pdy = wp.y - enemy->transform.y;
                pd  = sqrtf(pdx * pdx + pdy * pdy);
            }
        }
        if (!enemy->nav_path.done() && pd > 0.0f) {
            move_nx = pdx / pd;
            move_ny = pdy / pd;
        }
    }

    static void patrol_along_waypoints(Actor* enemy, float dt, Pathfinder* pathfinder) {
        if (enemy->patrol_waypoints.empty()) return;
        auto& wpv = enemy->patrol_waypoints;
        int   n   = static_cast<int>(wpv.size());
        if (n <= 0) return;
        const auto& wp = wpv[static_cast<size_t>(enemy->patrol_wp_index % n)];
        float wdx      = wp.x - enemy->transform.x;
        float wdy      = wp.y - enemy->transform.y;
        float wd       = sqrtf(wdx * wdx + wdy * wdy);
        if (wd < kPatrolArrivePx) {
            enemy->patrol_wp_index = (enemy->patrol_wp_index + 1) % n;
            return;
        }
        float move_nx = 0.0f, move_ny = 0.0f;
        if (pathfinder) {
            enemy->path_replan_timer -= dt;
            if (enemy->path_replan_timer <= 0.0f) {
                enemy->nav_path =
                    pathfinder->find_path(enemy->transform.x, enemy->transform.y, wp.x, wp.y);
                enemy->path_replan_timer = 0.35f;
            }
        }
        steer_on_nav_path(enemy, wp.x, wp.y, move_nx, move_ny);
        float spd = enemy->effective_move_speed() * 0.75f;
        enemy->transform.translate(move_nx * spd * dt, move_ny * spd * dt);
        enemy->set_facing(move_nx, move_ny);
        enemy->is_moving = true;
    }

    static void update_patrol(Actor* enemy, Actor* player, float dx, float dy, float dist,
                              float dt, std::vector<Actor*>& actors, Pathfinder* pathfinder) {
        const float aggro = enemy->aggro_range;

        switch (enemy->patrol_state) {
        case PatrolState::Patrol:
            if (dist < aggro) {
                enemy->patrol_state = PatrolState::Alert;
                enemy->alert_timer  = kAlertDurationSec;
                alert_patrol_neighbors(enemy, actors);
            } else {
                patrol_along_waypoints(enemy, dt, pathfinder);
            }
            break;
        case PatrolState::Alert:
            enemy->alert_timer -= dt;
            enemy->is_moving = false;
            if (enemy->alert_timer <= 0.0f)
                enemy->patrol_state = PatrolState::Chase;
            break;
        case PatrolState::Chase:
            if (dist > aggro * 1.5f) {
                enemy->patrol_state     = PatrolState::Patrol;
                enemy->patrol_wp_index  = 0;
                enemy->nav_path.reset();
                break;
            }
            chase_and_melee_attack(enemy, player, dx, dy, dist, dt, pathfinder);
            break;
        }
    }

    static void update_boss(Actor* enemy, Actor* player, float dx, float dy, float dist,
                            float dt, Pathfinder* pathfinder,
                            std::vector<Projectile>* /*projectiles*/) {
        const int   max_hp = std::max(1, enemy->health.max_hp);
        const float half   = (float)max_hp * 0.5f;
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
            && enemy->combat.is_attack_idle() && dist > kEnemyAiDistanceEpsilon) {
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
};

} // namespace mion
