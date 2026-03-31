#pragma once
#include <vector>
#include "../entities/actor.hpp"

namespace mion {

// Tick de recursos por frame: stamina e mana regen.
// O gating (can_afford) é checado em PlayerActionSystem; este sistema só faz regen.
struct ResourceSystem {
    void fixed_update(std::vector<Actor*>& actors, float dt) {
        for (auto* a : actors) {
            if (!a->is_alive) continue;
            a->stamina.tick(dt);
            a->mana.tick(dt);
            if (a->empowered_remaining_seconds > 0.0f) {
                a->empowered_remaining_seconds -= dt;
                if (a->empowered_remaining_seconds <= 0.0f) {
                    a->empowered_remaining_seconds = 0.0f;
                    a->empowered_damage_multiplier = 1.0f;
                }
            }
        }
    }
};

} // namespace mion
