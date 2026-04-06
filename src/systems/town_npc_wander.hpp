#pragma once

#include <cmath>
#include <vector>

#include "../entities/npc.hpp"

namespace mion {

namespace TownNpcWanderSystem {

inline unsigned next_rng(unsigned& rng_state) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

inline float next_rng_f01(unsigned& rng_state) {
    return static_cast<float>(next_rng(rng_state) & 0xFFFFu) / 65535.0f;
}

inline void update_town_npcs(std::vector<NpcEntity>& npcs, float dt, unsigned& rng_state) {
    for (auto& npc : npcs) {
        Actor* actor = npc.actor;
        if (!actor)
            continue;

        float& nx = actor->transform.x;
        float& ny = actor->transform.y;

        npc.wander_timer -= dt;
        if (npc.wander_timer <= 0.0f) {
            const float angle = next_rng_f01(rng_state) * 6.2831853f;
            const float tx = npc.spawn_x + std::cos(angle) * (next_rng_f01(rng_state) * npc.wander_radius);
            const float ty = npc.spawn_y + std::sin(angle) * (next_rng_f01(rng_state) * npc.wander_radius);
            const float dx = tx - npc.x;
            const float dy = ty - npc.y;
            const float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.5f) {
                npc.wander_dir_x = dx / len;
                npc.wander_dir_y = dy / len;
            } else {
                npc.wander_dir_x = 0.0f;
                npc.wander_dir_y = 0.0f;
            }
            npc.wander_timer = 1.4f + next_rng_f01(rng_state) * 1.2f;
        }

        nx += npc.wander_dir_x * npc.wander_speed * dt;
        ny += npc.wander_dir_y * npc.wander_speed * dt;
        actor->is_moving = (npc.wander_dir_x != 0.0f || npc.wander_dir_y != 0.0f);

        const float dx = nx - npc.spawn_x;
        const float dy = ny - npc.spawn_y;
        const float dist2 = dx * dx + dy * dy;
        if (dist2 > npc.wander_radius * npc.wander_radius) {
            const float dist = std::sqrt(dist2);
            nx = npc.spawn_x + dx / dist * npc.wander_radius;
            ny = npc.spawn_y + dy / dist * npc.wander_radius;
            npc.wander_dir_x = 0.0f;
            npc.wander_dir_y = 0.0f;
        }

        npc.x = nx;
        npc.y = ny;
    }
}

} // namespace TownNpcWanderSystem

} // namespace mion
