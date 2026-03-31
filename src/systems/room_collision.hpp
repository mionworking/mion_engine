#pragma once
#include <cmath>
#include <vector>
#include "../entities/actor.hpp"
#include "../world/room.hpp"

namespace mion {

// Resolve colisão de actors vs obstáculos estáticos e world bounds
// Usa push-out por menor eixo (mesmo comportamento do Python)
struct RoomCollisionSystem {
private:
    static float _penetration_1d(float a_center, float a_half,
                                 float b_center, float b_half)
    {
        return (a_half + b_half) - std::abs(b_center - a_center);
    }

    void _apply_axis_push(Actor& a, Actor& b, bool use_x,
                          float direction_sign, float amount) const
    {
        if (use_x) {
            a.transform.x -= direction_sign * amount;
            b.transform.x += direction_sign * amount;
        } else {
            a.transform.y -= direction_sign * amount;
            b.transform.y += direction_sign * amount;
        }
    }

    void _push_single_actor(Actor& actor, bool use_x,
                            float direction_sign, float amount) const
    {
        if (use_x)
            actor.transform.x += direction_sign * amount;
        else
            actor.transform.y += direction_sign * amount;
    }

    void _resolve_pair(Actor& a, Actor& b, const RoomDefinition* room) {
        AABB ab = a.collision.bounds_at(a.transform.x, a.transform.y);
        AABB bb = b.collision.bounds_at(b.transform.x, b.transform.y);
        if (!ab.intersects(bb)) return;

        float pen_x = _penetration_1d(a.transform.x, a.collision.half_w,
                                      b.transform.x, b.collision.half_w);
        float pen_y = _penetration_1d(a.transform.y, a.collision.half_h,
                                      b.transform.y, b.collision.half_h);
        bool  use_x = pen_x <= pen_y;
        float pen   = use_x ? pen_x : pen_y;
        if (pen <= 0.0f) return;

        float delta = use_x ? (b.transform.x - a.transform.x)
                            : (b.transform.y - a.transform.y);
        float sign  = (delta >= 0.0f) ? 1.0f : -1.0f;

        _apply_axis_push(a, b, use_x, sign, pen * 0.5f);

        if (!room) return;

        resolve(a, *room);
        resolve(b, *room);

        ab = a.collision.bounds_at(a.transform.x, a.transform.y);
        bb = b.collision.bounds_at(b.transform.x, b.transform.y);
        if (!ab.intersects(bb)) return;

        float remaining = use_x
            ? _penetration_1d(a.transform.x, a.collision.half_w,
                              b.transform.x, b.collision.half_w)
            : _penetration_1d(a.transform.y, a.collision.half_h,
                              b.transform.y, b.collision.half_h);
        if (remaining <= 0.0f) return;

        _push_single_actor(a, use_x, -sign, remaining);
        resolve(a, *room);

        ab = a.collision.bounds_at(a.transform.x, a.transform.y);
        bb = b.collision.bounds_at(b.transform.x, b.transform.y);
        if (!ab.intersects(bb)) return;

        remaining = use_x
            ? _penetration_1d(a.transform.x, a.collision.half_w,
                              b.transform.x, b.collision.half_w)
            : _penetration_1d(a.transform.y, a.collision.half_h,
                              b.transform.y, b.collision.half_h);
        if (remaining <= 0.0f) return;

        _push_single_actor(b, use_x, sign, remaining);
        resolve(b, *room);
    }

public:

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
    void resolve_actors(std::vector<Actor*>& actors,
                        const RoomDefinition* room = nullptr)
    {
        for (int iter = 0; iter < 3; ++iter)
        for (size_t i = 0; i < actors.size(); ++i) {
            Actor* a = actors[i];
            if (!a->is_alive) continue;

            for (size_t j = i + 1; j < actors.size(); ++j) {
                Actor* b = actors[j];
                if (!b->is_alive) continue;
                _resolve_pair(*a, *b, room);
            }
        }
    }
};

} // namespace mion
