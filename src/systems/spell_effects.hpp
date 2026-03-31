#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "../entities/actor.hpp"

namespace mion {

inline void apply_nova(Actor& caster, std::vector<Actor*>& actors, float radius, int damage) {
    const float r2 = radius * radius;
    for (auto* target : actors) {
        if (!target->is_alive) continue;
        if (target->team == caster.team) continue;
        if (!target->combat.can_receive_hit()) continue;

        float dx = target->transform.x - caster.transform.x;
        float dy = target->transform.y - caster.transform.y;
        if (dx * dx + dy * dy > r2) continue;

        target->combat.apply_hit_reaction();
        target->health.apply_damage(damage);
        if (!target->health.is_alive()) target->is_alive = false;

        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.0f) {
            target->apply_knockback_impulse((dx / dist) * 300.0f, (dy / dist) * 300.0f);
        }
    }
}

inline void apply_cleave(const Actor&        caster,
                         std::vector<Actor*>& actors,
                         float                radius,
                         int                  damage) {
    const float r2 = radius * radius;
    const float cx = caster.transform.x;
    const float cy = caster.transform.y;
    for (auto* target : actors) {
        if (!target->is_alive) continue;
        if (target->team == caster.team) continue;
        if (!target->combat.can_receive_hit()) continue;

        float dx = target->transform.x - cx;
        float dy = target->transform.y - cy;
        if (dx * dx + dy * dy > r2) continue;

        target->combat.apply_hit_reaction();
        target->health.apply_damage(damage);
        if (!target->health.is_alive()) target->is_alive = false;

        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.0f) {
            target->apply_knockback_impulse((dx / dist) * 260.0f, (dy / dist) * 260.0f);
        }
    }
}

inline void apply_chain_lightning(const Actor&        caster,
                                  std::vector<Actor*>& actors,
                                  int                  bounces,
                                  int                  damage_per_hit) {
    if (bounces <= 0 || damage_per_hit <= 0) return;
    const float link_range = 160.0f;
    const float link2      = link_range * link_range;

    float ox = caster.transform.x;
    float oy = caster.transform.y;

    std::vector<Actor*> hit;
    for (int hop = 0; hop < bounces; ++hop) {
        Actor* best = nullptr;
        float  best_d2 = link2;

        for (auto* t : actors) {
            if (!t->is_alive) continue;
            if (t->team == caster.team) continue;
            if (!t->combat.can_receive_hit()) continue;
            bool already = false;
            for (Actor* h : hit) {
                if (h == t) {
                    already = true;
                    break;
                }
            }
            if (already) continue;

            float dx = t->transform.x - ox;
            float dy = t->transform.y - oy;
            float d2 = dx * dx + dy * dy;
            if (d2 <= best_d2 && d2 > 0.0001f) {
                best_d2 = d2;
                best    = t;
            }
        }

        if (!best) break;

        hit.push_back(best);
        best->combat.apply_hit_reaction();
        best->health.apply_damage(damage_per_hit);
        if (!best->health.is_alive()) best->is_alive = false;

        float dx = best->transform.x - ox;
        float dy = best->transform.y - oy;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.0f) {
            best->apply_knockback_impulse((dx / dist) * 200.0f, (dy / dist) * 200.0f);
        }
        ox = best->transform.x;
        oy = best->transform.y;
    }
}

inline void apply_frost_slow(Actor& target, int frost_bolt_rank) {
    if (frost_bolt_rank <= 0) return;
    const int   r   = std::min(frost_bolt_rank, 3);
    const float mult = 0.55f - 0.05f * (float)(r - 1);
    const float dur  = 2.2f + 0.35f * (float)r;
    target.status_effects.apply_full(StatusEffectType::Slow, dur, 0.5f, 0, mult);
}

inline void apply_battle_cry(Actor& player, int battle_cry_level) {
    const int lv = std::max(1, std::min(battle_cry_level, 3));
    player.empowered_damage_multiplier = 1.18f + 0.07f * (float)(lv - 1);
    player.empowered_remaining_seconds =
        std::max(player.empowered_remaining_seconds, 4.0f + 0.6f * (float)lv);
}

} // namespace mion
