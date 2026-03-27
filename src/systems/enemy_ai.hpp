#pragma once
#include <vector>
#include <cmath>
#include "../entities/actor.hpp"
#include "pathfinder.hpp"

namespace mion {

// IA de inimigos:
//   aggro range (por-actor) → separação entre inimigos → perseguir via A* → atacar
struct EnemyAISystem {
    void fixed_update(std::vector<Actor*>& actors, float dt,
                      Pathfinder* pathfinder = nullptr)
    {
        // Encontra o player vivo
        Actor* player = nullptr;
        for (auto* a : actors) {
            if (a->team == Team::Player && a->is_alive) { player = a; break; }
        }
        if (!player) return;

        for (auto* enemy : actors) {
            if (enemy->team != Team::Enemy) continue;
            if (!enemy->is_alive) continue;
            if (enemy->combat.is_hurt_stunned()) continue;

            float dx   = player->transform.x - enemy->transform.x;
            float dy   = player->transform.y - enemy->transform.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > enemy->aggro_range) continue;

            // --- Separação entre inimigos ---
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
            enemy->transform.translate(sep_x * sep_strength * dt,
                                       sep_y * sep_strength * dt);

            // --- Movimento --- (apenas fora do range de parada e sem atacar)
            if (dist > enemy->stop_range_ai && enemy->combat.is_attack_idle()) {
                float move_nx = dx / dist;
                float move_ny = dy / dist;

                if (pathfinder) {
                    // Replan periódico (escalonado pelo índice do enemy no vetor)
                    enemy->path_replan_timer -= dt;
                    if (enemy->path_replan_timer <= 0.0f) {
                        enemy->nav_path = pathfinder->find_path(
                            enemy->transform.x, enemy->transform.y,
                            player->transform.x, player->transform.y);
                        enemy->path_replan_timer = 0.35f;
                    }

                    // Segue o próximo waypoint
                    if (enemy->nav_path.valid && !enemy->nav_path.done()) {
                        auto wp  = enemy->nav_path.next_wp();
                        float wdx = wp.x - enemy->transform.x;
                        float wdy = wp.y - enemy->transform.y;
                        float wd  = sqrtf(wdx * wdx + wdy * wdy);

                        // Avança waypoint quando suficientemente próximo (~0.6 tile)
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

                enemy->transform.translate(move_nx * enemy->move_speed * dt,
                                           move_ny * enemy->move_speed * dt);
                enemy->set_facing(move_nx, move_ny);
                enemy->is_moving = true;
            }

            // --- Ataque ---
            if (dist <= enemy->attack_range_ai && enemy->combat.is_attack_idle()) {
                enemy->set_facing(dx / dist, dy / dist);
                enemy->combat.begin_attack();
            }
        }
    }
};

} // namespace mion
