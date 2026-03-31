#pragma once
#include <vector>
#include "../entities/actor.hpp"

namespace mion {

struct StatusEffectSystem {
    void fixed_update(std::vector<Actor*>& actors, float dt) {
        for (auto* a : actors) {
            if (!a->is_alive) continue;

            for (auto& e : a->status_effects.slots) {
                if (e.type == StatusEffectType::None) continue;

                // Ticks ANTES do decremento para não perder o último tick
                // quando remaining cai a ≤ 0 exactamente neste frame.
                if (e.type == StatusEffectType::Poison && e.remaining > 0.0f) {
                    e.tick_timer += dt;
                    while (e.tick_timer >= e.poison_interval_seconds && e.remaining > 0.0f) {
                        e.tick_timer -= e.poison_interval_seconds;
                        a->health.apply_damage(e.poison_damage_per_tick);
                        if (!a->health.is_alive()) {
                            a->is_alive = false;
                            break;
                        }
                    }
                }
                e.remaining -= dt;
            }
            a->status_effects.clear_expired();
        }
    }
};

} // namespace mion
