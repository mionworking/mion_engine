#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include "../entities/actor.hpp"
#include "../entities/projectile.hpp"
#include "pathfinder.hpp"

namespace mion::enemy_ai {

inline constexpr float kDistanceEpsilon = 0.0001f;
inline constexpr float kProjectileSpeed = 340.0f;
inline constexpr float kPathAdvancePx   = 20.0f;
inline constexpr float kPathReplanSec   = 0.35f;

inline void push_enemy_projectile(std::vector<Projectile>& list, Actor& shooter,
                                  Actor& target, int damage) {
    const float dx   = target.transform.x - shooter.transform.x;
    const float dy   = target.transform.y - shooter.transform.y;
    const float dist = sqrtf(dx * dx + dy * dy);
    if (dist < kDistanceEpsilon)
        return;

    Projectile projectile;
    projectile.x                          = shooter.transform.x;
    projectile.y                          = shooter.transform.y;
    projectile.vx                         = (dx / dist) * kProjectileSpeed;
    projectile.vy                         = (dy / dist) * kProjectileSpeed;
    projectile.damage                     = damage;
    projectile.owner_team                 = Team::Enemy;
    projectile.half_w                     = 4.0f;
    projectile.half_h                     = 4.0f;
    projectile.lifetime_remaining_seconds = 3.2f;
    projectile.active                     = true;
    list.push_back(projectile);
}

inline void apply_separation(Actor* enemy, std::vector<Actor*>& actors, float dt) {
    float sep_x = 0.0f;
    float sep_y = 0.0f;
    constexpr float kMinSep      = 36.0f;
    constexpr float kSepStrength = 60.0f;

    for (auto* other : actors) {
        if (other == enemy)
            continue;
        if (other->team != Team::Enemy || !other->is_alive)
            continue;

        const float ox    = enemy->transform.x - other->transform.x;
        const float oy    = enemy->transform.y - other->transform.y;
        const float odist = sqrtf(ox * ox + oy * oy);
        if (odist < kMinSep && odist > 0.0f) {
            const float push = (kMinSep - odist) / kMinSep;
            sep_x += (ox / odist) * push;
            sep_y += (oy / odist) * push;
        }
    }

    enemy->transform.translate(sep_x * kSepStrength * dt, sep_y * kSepStrength * dt);
}

inline void replan_chase_path(Actor* enemy, Actor* player, Pathfinder* pathfinder, float dt) {
    if (!pathfinder)
        return;

    enemy->path_replan_timer -= dt;
    if (enemy->path_replan_timer <= 0.0f) {
        enemy->nav_path = pathfinder->find_path(
            enemy->transform.x, enemy->transform.y,
            player->transform.x, player->transform.y);
        enemy->path_replan_timer = kPathReplanSec;
    }
}

inline void move_along_path_toward(Actor* enemy, Actor* player, float& move_nx,
                                   float& move_ny, Pathfinder* pathfinder) {
    const float dx   = player->transform.x - enemy->transform.x;
    const float dy   = player->transform.y - enemy->transform.y;
    const float dist = sqrtf(dx * dx + dy * dy);
    if (dist > kDistanceEpsilon) {
        move_nx = dx / dist;
        move_ny = dy / dist;
    }
    if (!pathfinder)
        return;

    if (enemy->nav_path.valid && !enemy->nav_path.done()) {
        auto  wp  = enemy->nav_path.next_wp();
        float wdx = wp.x - enemy->transform.x;
        float wdy = wp.y - enemy->transform.y;
        float wd  = sqrtf(wdx * wdx + wdy * wdy);

        if (wd < kPathAdvancePx) {
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

inline void chase_and_melee_attack(Actor* enemy, Actor* player, float dx, float dy,
                                   float dist, float dt, Pathfinder* pathfinder) {
    if (enemy->boss_charging)
        return;

    replan_chase_path(enemy, player, pathfinder, dt);

    if (dist > enemy->stop_range_ai && enemy->combat.is_attack_idle()) {
        float move_nx = 0.0f;
        float move_ny = 0.0f;
        move_along_path_toward(enemy, player, move_nx, move_ny, pathfinder);
        const float speed = enemy->effective_move_speed();
        enemy->transform.translate(move_nx * speed * dt, move_ny * speed * dt);
        enemy->set_facing(move_nx, move_ny);
        enemy->is_moving = true;
    }

    if (dist <= enemy->attack_range_ai && enemy->combat.is_attack_idle()) {
        if (dist > kDistanceEpsilon)
            enemy->set_facing(dx / dist, dy / dist);
        enemy->combat.begin_attack();
    }
}

inline void update_ranged(Actor* enemy, Actor* player, float dx, float dy, float dist,
                          float dt, Pathfinder* pathfinder,
                          std::vector<Projectile>* projectiles) {
    replan_chase_path(enemy, player, pathfinder, dt);

    const float keep = std::max(80.0f, enemy->ranged_keep_dist);
    float move_nx = 0.0f;
    float move_ny = 0.0f;

    if (dist > keep * 1.05f) {
        move_along_path_toward(enemy, player, move_nx, move_ny, pathfinder);
    } else if (dist < keep * 0.6f) {
        if (dist > kDistanceEpsilon) {
            move_nx = -dx / dist;
            move_ny = -dy / dist;
        }
    } else if (dist > kDistanceEpsilon) {
        move_nx = -dy / dist;
        move_ny = dx / dist;
    }

    if (enemy->combat.is_attack_idle()) {
        const float speed = enemy->effective_move_speed() * 0.85f;
        enemy->transform.translate(move_nx * speed * dt, move_ny * speed * dt);
        enemy->set_facing(dx / std::max(dist, kDistanceEpsilon),
                          dy / std::max(dist, kDistanceEpsilon));
        enemy->is_moving = (move_nx != 0.0f || move_ny != 0.0f);
    }

    enemy->ranged_fire_cd -= dt;
    if (enemy->ranged_fire_cd <= 0.0f && projectiles && enemy->combat.is_attack_idle()
        && dist <= enemy->aggro_range) {
        push_enemy_projectile(*projectiles, *enemy, *player, enemy->ranged_proj_damage);
        enemy->ranged_fire_cd = std::max(0.2f, enemy->ranged_fire_rate);
    }
}

} // namespace mion::enemy_ai
