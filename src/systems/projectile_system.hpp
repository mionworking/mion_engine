#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "../entities/projectile.hpp"
#include "../entities/actor.hpp"
#include "../components/status_effect.hpp"
#include "../world/room.hpp"

namespace mion {

struct ProjectileSystem {
    /// Último impacto em ator (mundo) — para partículas no frame atual.
    bool  projectile_hit_actor = false;
    float last_hit_world_x       = 0.f;
    float last_hit_world_y       = 0.f;

    struct HitEvent {
        std::string target_name;
        int         damage = 0;
    };
    std::vector<HitEvent> last_hit_events;

    /// `world_origin_*` — offset da `WorldArea` em que o player está; posições de projéteis e atores são globais.
    void fixed_update(std::vector<Projectile>& projectiles,
                      std::vector<Actor*>& actors,
                      const RoomDefinition& room,
                      float world_origin_x,
                      float world_origin_y,
                      float dt) {
        projectile_hit_actor = false;
        last_hit_events.clear();
        const float bx0 = room.bounds.min_x + world_origin_x;
        const float bx1 = room.bounds.max_x + world_origin_x;
        const float by0 = room.bounds.min_y + world_origin_y;
        const float by1 = room.bounds.max_y + world_origin_y;

        for (auto& pr : projectiles) {
            if (!pr.active) continue;

            pr.lifetime_remaining_seconds -= dt;
            if (pr.lifetime_remaining_seconds <= 0.0f) {
                pr.active = false;
                continue;
            }

            pr.x += pr.vx * dt;
            pr.y += pr.vy * dt;

            AABB pb = {
                pr.x - pr.half_w, pr.x + pr.half_w,
                pr.y - pr.half_h, pr.y + pr.half_h
            };

            if (pb.min_x < bx0 || pb.max_x > bx1 || pb.min_y < by0 || pb.max_y > by1) {
                pr.active = false;
                continue;
            }

            for (const auto& obs : room.obstacles) {
                AABB ob = obs.bounds;
                ob.min_x += world_origin_x;
                ob.max_x += world_origin_x;
                ob.min_y += world_origin_y;
                ob.max_y += world_origin_y;
                if (pb.intersects(ob)) {
                    pr.active = false;
                    break;
                }
            }
            if (!pr.active) continue;

            for (auto* target : actors) {
                if (!target->is_alive) continue;
                if (target->team == pr.owner_team) continue;
                if (!target->combat.can_receive_hit()) continue;

                AABB hurt = target->hurt_box.bounds_at(target->transform.x, target->transform.y);
                if (!pb.intersects(hurt)) continue;

                projectile_hit_actor = true;
                last_hit_world_x     = pr.x;
                last_hit_world_y     = pr.y;

                target->combat.apply_hit_reaction();
                target->health.apply_damage(pr.damage);
                if (!target->health.is_alive()) target->is_alive = false;

                last_hit_events.push_back({target->name, pr.damage});

                if (pr.is_frost && pr.frost_rank > 0) {
                    const float rank = (float)std::min(pr.frost_rank, 3);
                    const float mult = 0.55f - 0.05f * (rank - 1.0f);
                    const float dur  = 2.2f + 0.35f * rank;
                    target->status_effects.apply_full(StatusEffectType::Slow, dur, 0.5f, 0,
                                                      mult);
                }
                if (pr.is_poison && pr.poison_rank > 0) {
                    const int   r    = std::min(pr.poison_rank, 3);
                    const float dur  = 4.0f + 0.6f * (float)r;
                    const int   dpt  = 2 + r * 2;
                    target->status_effects.apply_full(
                        StatusEffectType::Poison, dur, 0.5f, dpt, 0.52f);
                }

                float dx = target->transform.x - pr.x;
                float dy = target->transform.y - pr.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 0.0f) {
                    target->apply_knockback_impulse(
                        (dx / dist) * 240.0f,
                        (dy / dist) * 240.0f
                    );
                }
                pr.active = false;
                break;
            }
        }

        // compact remove inactive at end — caller can also erase periodically
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                           [](const Projectile& p) { return !p.active; }),
            projectiles.end()
        );
    }
};

} // namespace mion
