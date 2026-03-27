#pragma once
#include <vector>
#include <string>
#include "../components/collision.hpp"

namespace mion {

struct Obstacle {
    std::string name;
    AABB        bounds;
};

struct WorldBounds {
    float min_x = 0.0f;
    float max_x = 800.0f;
    float min_y = 0.0f;
    float max_y = 600.0f;

    // Clamp o centro de uma caixa para ficar dentro dos bounds
    void clamp_box(float& cx, float& cy, float half_w, float half_h) const {
        if (cx - half_w < min_x) cx = min_x + half_w;
        if (cx + half_w > max_x) cx = max_x - half_w;
        if (cy - half_h < min_y) cy = min_y + half_h;
        if (cy + half_h > max_y) cy = max_y - half_h;
    }
};

struct RoomDefinition {
    std::string          name;
    WorldBounds          bounds;
    std::vector<Obstacle> obstacles;

    void add_obstacle(std::string n, float x1, float y1, float x2, float y2) {
        obstacles.push_back({ std::move(n), {x1, x2, y1, y2} });
    }
};

} // namespace mion
