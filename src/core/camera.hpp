#pragma once
#include <random>
#include <cmath>
#include "../world/room.hpp"

namespace mion {

struct Camera2D {
    float x = 0.0f;
    float y = 0.0f;
    float viewport_w = 1280.0f;
    float viewport_h = 720.0f;

    // Shake — offset applied only during render, does not affect the real position.
    float shake_offset_x      = 0.0f;
    float shake_offset_y      = 0.0f;
    int   shake_remaining     = 0;
    int   shake_total         = 0;
    float shake_intensity     = 0.0f;

    // Converts world → screen including the shake offset.
    float world_to_screen_x(float wx) const { return wx - x + shake_offset_x; }
    float world_to_screen_y(float wy) const { return wy - y + shake_offset_y; }

    void trigger_shake(float intensity, int frames) {
        // New shake only overrides if stronger than the current one.
        if (intensity >= shake_intensity || shake_remaining == 0) {
            shake_intensity = intensity;
            shake_total     = frames;
            shake_remaining = frames;
        }
    }

    // Called once per fixed update — generates a random offset with linear decay.
    void step_shake(std::mt19937& rng) {
        if (shake_remaining <= 0) {
            shake_offset_x = 0.0f;
            shake_offset_y = 0.0f;
            shake_intensity = 0.0f;
            return;
        }

        float decay  = (float)shake_remaining / (float)shake_total;
        float radius = shake_intensity * decay;

        // Random direction each frame — produces the characteristic erratic motion.
        std::uniform_int_distribution<int> dist(0, 627);
        float angle = (float)dist(rng) * 0.01f;  // 0..2π approx
        shake_offset_x = radius * cosf(angle);
        shake_offset_y = radius * sinf(angle);

        const float cap = 52.0f;
        float       mx  = fabsf(shake_offset_x);
        float       my  = fabsf(shake_offset_y);
        float       m   = (mx > my) ? mx : my;
        if (m > cap) {
            float s = cap / m;
            shake_offset_x *= s;
            shake_offset_y *= s;
        }

        --shake_remaining;
    }

    void follow(float target_x, float target_y, const WorldBounds& bounds) {
        x = target_x - viewport_w * 0.5f;
        y = target_y - viewport_h * 0.5f;

        if (x < bounds.min_x)              x = bounds.min_x;
        if (x + viewport_w > bounds.max_x) x = bounds.max_x - viewport_w;
        if (y < bounds.min_y)              y = bounds.min_y;
        if (y + viewport_h > bounds.max_y) y = bounds.max_y - viewport_h;
    }
};

} // namespace mion
