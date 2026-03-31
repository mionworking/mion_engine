#pragma once
#include <cmath>
#include <vector>

#include "ai_combat.hpp"

namespace mion::enemy_ai {

inline constexpr float kPatrolArrivePx            = 24.0f;
inline constexpr float kAlertDurationSec          = 1.0f;
inline constexpr float kPatrolAlertNeighborRadius = 200.0f;

inline void alert_patrol_neighbors(Actor* source, std::vector<Actor*>& actors) {
    for (auto* other : actors) {
        if (other == source)
            continue;
        if (other->team != Team::Enemy || !other->is_alive)
            continue;
        if (other->ai_behavior != AiBehavior::Patrol)
            continue;

        const float ox = other->transform.x - source->transform.x;
        const float oy = other->transform.y - source->transform.y;
        if (ox * ox + oy * oy <= kPatrolAlertNeighborRadius * kPatrolAlertNeighborRadius) {
            other->patrol_state = PatrolState::Chase;
            other->alert_timer  = 0.0f;
        }
    }
}

inline void steer_on_nav_path(Actor* enemy, float goal_x, float goal_y,
                              float& move_nx, float& move_ny) {
    const float wdx = goal_x - enemy->transform.x;
    const float wdy = goal_y - enemy->transform.y;
    const float wd  = sqrtf(wdx * wdx + wdy * wdy);
    if (wd > kDistanceEpsilon) {
        move_nx = wdx / wd;
        move_ny = wdy / wd;
    }
    if (!enemy->nav_path.valid || enemy->nav_path.done())
        return;

    auto  wp  = enemy->nav_path.next_wp();
    float pdx = wp.x - enemy->transform.x;
    float pdy = wp.y - enemy->transform.y;
    float pd  = sqrtf(pdx * pdx + pdy * pdy);
    if (pd < kPathAdvancePx) {
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

inline void patrol_along_waypoints(Actor* enemy, float dt, Pathfinder* pathfinder) {
    if (enemy->patrol_waypoints.empty())
        return;

    auto& waypoints = enemy->patrol_waypoints;
    const int count = static_cast<int>(waypoints.size());
    if (count <= 0)
        return;

    const auto& wp = waypoints[static_cast<size_t>(enemy->patrol_wp_index % count)];
    const float wdx = wp.x - enemy->transform.x;
    const float wdy = wp.y - enemy->transform.y;
    const float wd  = sqrtf(wdx * wdx + wdy * wdy);
    if (wd < kPatrolArrivePx) {
        enemy->patrol_wp_index = (enemy->patrol_wp_index + 1) % count;
        return;
    }

    float move_nx = 0.0f;
    float move_ny = 0.0f;
    if (pathfinder) {
        enemy->path_replan_timer -= dt;
        if (enemy->path_replan_timer <= 0.0f) {
            enemy->nav_path = pathfinder->find_path(
                enemy->transform.x, enemy->transform.y, wp.x, wp.y);
            enemy->path_replan_timer = kPathReplanSec;
        }
    }

    steer_on_nav_path(enemy, wp.x, wp.y, move_nx, move_ny);
    const float speed = enemy->effective_move_speed() * 0.75f;
    enemy->transform.translate(move_nx * speed * dt, move_ny * speed * dt);
    enemy->set_facing(move_nx, move_ny);
    enemy->is_moving = true;
}

inline void update_patrol(Actor* enemy, Actor* player, float dx, float dy, float dist,
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
            enemy->patrol_state    = PatrolState::Patrol;
            enemy->patrol_wp_index = 0;
            enemy->nav_path.reset();
            break;
        }
        chase_and_melee_attack(enemy, player, dx, dy, dist, dt, pathfinder);
        break;
    }
}

} // namespace mion::enemy_ai
