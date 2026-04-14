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
        if (other->enemy_ai->ai_behavior != AiBehavior::Patrol)
            continue;

        const float ox = other->transform.x - source->transform.x;
        const float oy = other->transform.y - source->transform.y;
        if (ox * ox + oy * oy <= kPatrolAlertNeighborRadius * kPatrolAlertNeighborRadius) {
            other->enemy_ai->patrol_state = PatrolState::Chase;
            other->enemy_ai->alert_timer  = 0.0f;
        }
    }
}

inline void steer_on_nav_path(Actor* enemy, float goal_x, float goal_y,
                              float& move_nx, float& move_ny,
                              float nav_ox, float nav_oy) {
    const float wdx = goal_x - enemy->transform.x;
    const float wdy = goal_y - enemy->transform.y;
    const float wd  = sqrtf(wdx * wdx + wdy * wdy);
    if (wd > kDistanceEpsilon) {
        move_nx = wdx / wd;
        move_ny = wdy / wd;
    }
    if (!enemy->enemy_ai->nav_path.valid || enemy->enemy_ai->nav_path.done())
        return;

    auto  wp  = enemy->enemy_ai->nav_path.next_wp();
    float pdx = (wp.x + nav_ox) - enemy->transform.x;
    float pdy = (wp.y + nav_oy) - enemy->transform.y;
    float pd  = sqrtf(pdx * pdx + pdy * pdy);
    if (pd < kPathAdvancePx) {
        enemy->enemy_ai->nav_path.advance();
        if (!enemy->enemy_ai->nav_path.done()) {
            wp  = enemy->enemy_ai->nav_path.next_wp();
            pdx = (wp.x + nav_ox) - enemy->transform.x;
            pdy = (wp.y + nav_oy) - enemy->transform.y;
            pd  = sqrtf(pdx * pdx + pdy * pdy);
        }
    }

    if (!enemy->enemy_ai->nav_path.done() && pd > 0.0f) {
        move_nx = pdx / pd;
        move_ny = pdy / pd;
    }
}

inline void patrol_along_waypoints(Actor* enemy, float dt, Pathfinder* pathfinder,
                                   float nav_ox, float nav_oy) {
    if (enemy->enemy_ai->patrol_waypoints.empty())
        return;

    auto& waypoints = enemy->enemy_ai->patrol_waypoints;
    const int count = static_cast<int>(waypoints.size());
    if (count <= 0)
        return;

    const auto& wp = waypoints[static_cast<size_t>(enemy->enemy_ai->patrol_wp_index % count)];
    const float wdx = wp.x - enemy->transform.x;
    const float wdy = wp.y - enemy->transform.y;
    const float wd  = sqrtf(wdx * wdx + wdy * wdy);
    if (wd < kPatrolArrivePx) {
        enemy->enemy_ai->patrol_wp_index = (enemy->enemy_ai->patrol_wp_index + 1) % count;
        return;
    }

    float move_nx = 0.0f;
    float move_ny = 0.0f;
    if (pathfinder) {
        enemy->enemy_ai->path_replan_timer -= dt;
        if (enemy->enemy_ai->path_replan_timer <= 0.0f) {
            enemy->enemy_ai->nav_path = pathfinder->find_path(
                enemy->transform.x - nav_ox, enemy->transform.y - nav_oy,
                wp.x - nav_ox, wp.y - nav_oy);
            enemy->enemy_ai->path_replan_timer = kPathReplanSec;
        }
    }

    steer_on_nav_path(enemy, wp.x, wp.y, move_nx, move_ny, nav_ox, nav_oy);
    const float speed = enemy->effective_move_speed() * 0.75f;
    enemy->transform.translate(move_nx * speed * dt, move_ny * speed * dt);
    enemy->set_facing(move_nx, move_ny);
    enemy->is_moving = true;
}

inline void update_patrol(Actor* enemy, Actor* player, float dx, float dy, float dist,
                          float dt, std::vector<Actor*>& actors, Pathfinder* pathfinder,
                          float nav_ox, float nav_oy) {
    const float aggro = enemy->enemy_ai->aggro_range;

    switch (enemy->enemy_ai->patrol_state) {
    case PatrolState::Patrol:
        if (dist < aggro) {
            enemy->enemy_ai->patrol_state = PatrolState::Alert;
            enemy->enemy_ai->alert_timer  = kAlertDurationSec;
            alert_patrol_neighbors(enemy, actors);
        } else {
            patrol_along_waypoints(enemy, dt, pathfinder, nav_ox, nav_oy);
        }
        break;
    case PatrolState::Alert:
        enemy->enemy_ai->alert_timer -= dt;
        enemy->is_moving = false;
        if (enemy->enemy_ai->alert_timer <= 0.0f)
            enemy->enemy_ai->patrol_state = PatrolState::Chase;
        break;
    case PatrolState::Chase:
        if (dist > aggro * 1.5f) {
            enemy->enemy_ai->patrol_state    = PatrolState::Patrol;
            enemy->enemy_ai->patrol_wp_index = 0;
            enemy->enemy_ai->nav_path.reset();
            break;
        }
        chase_and_melee_attack(enemy, player, dx, dy, dist, dt, pathfinder, nav_ox, nav_oy);
        break;
    }
}

} // namespace mion::enemy_ai
