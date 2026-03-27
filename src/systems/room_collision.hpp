#pragma once
#include <vector>
#include "../entities/actor.hpp"
#include "../world/room.hpp"

namespace mion {

// Resolve colisão de actors vs obstáculos estáticos e world bounds
// Usa push-out por menor eixo (mesmo comportamento do Python)
struct RoomCollisionSystem {

    void resolve(Actor& actor, const RoomDefinition& room) {
        if (!actor.is_alive) return;

        float& cx = actor.transform.x;
        float& cy = actor.transform.y;
        float hw   = actor.collision.half_w;
        float hh   = actor.collision.half_h;

        // --- World bounds ---
        room.bounds.clamp_box(cx, cy, hw, hh);

        // --- Obstáculos estáticos ---
        for (const auto& obs : room.obstacles) {
            AABB actor_box = actor.collision.bounds_at(cx, cy);
            if (!actor_box.intersects(obs.bounds)) continue;

            // Push-out: descobre qual eixo tem menor sobreposição
            float overlap_left  = actor_box.max_x - obs.bounds.min_x;
            float overlap_right = obs.bounds.max_x - actor_box.min_x;
            float overlap_up    = actor_box.max_y - obs.bounds.min_y;
            float overlap_down  = obs.bounds.max_y - actor_box.min_y;

            float min_x_overlap = (overlap_left < overlap_right) ? -overlap_left : overlap_right;
            float min_y_overlap = (overlap_up   < overlap_down)  ? -overlap_up   : overlap_down;

            if (std::abs(min_x_overlap) < std::abs(min_y_overlap))
                cx += min_x_overlap;
            else
                cy += min_y_overlap;
        }
    }

    void resolve_all(std::vector<Actor*>& actors, const RoomDefinition& room) {
        for (auto* a : actors) resolve(*a, room);
    }

    // Colisão actor vs actor — 3 iterações para convergir quando há vários actors empilhados
    void resolve_actors(std::vector<Actor*>& actors) {
        for (int iter = 0; iter < 3; ++iter)
        for (size_t i = 0; i < actors.size(); ++i) {
            Actor* a = actors[i];
            if (!a->is_alive) continue;

            for (size_t j = i + 1; j < actors.size(); ++j) {
                Actor* b = actors[j];
                if (!b->is_alive) continue;

                AABB ab = a->collision.bounds_at(a->transform.x, a->transform.y);
                AABB bb = b->collision.bounds_at(b->transform.x, b->transform.y);
                if (!ab.intersects(bb)) continue;

                float overlap_left  = ab.max_x - bb.min_x;
                float overlap_right = bb.max_x - ab.min_x;
                float overlap_up    = ab.max_y - bb.min_y;
                float overlap_down  = bb.max_y - ab.min_y;

                float min_x = (overlap_left < overlap_right) ? -overlap_left : overlap_right;
                float min_y = (overlap_up   < overlap_down)  ? -overlap_up   : overlap_down;

                // Push-out dividido igualmente entre os dois
                if (std::abs(min_x) < std::abs(min_y)) {
                    a->transform.x -= min_x * 0.5f;
                    b->transform.x += min_x * 0.5f;
                } else {
                    a->transform.y -= min_y * 0.5f;
                    b->transform.y += min_y * 0.5f;
                }
            }
        }
    }
};

} // namespace mion
