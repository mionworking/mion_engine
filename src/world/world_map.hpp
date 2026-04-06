#pragma once

#include <vector>
#include <cmath>

#include "world_area.hpp"
#include "../core/camera.hpp"

namespace mion {

struct WorldMap {
    std::vector<WorldArea> areas;

    std::vector<Obstacle> get_obstacles_near(AABB query_rect) const {
        std::vector<Obstacle> result;
        for (const auto& area : areas) {
            AABB ab = area.global_bounds();
            if (!query_rect.intersects(ab)) continue;

            for (const auto& obs : area.room.obstacles) {
                Obstacle global_obs = obs;
                global_obs.bounds.min_x += area.offset_x;
                global_obs.bounds.max_x += area.offset_x;
                global_obs.bounds.min_y += area.offset_y;
                global_obs.bounds.max_y += area.offset_y;
                if (query_rect.intersects(global_obs.bounds))
                    result.push_back(global_obs);
            }
        }
        return result;
    }

    WorldBounds total_bounds() const {
        if (areas.empty()) return {};
        float mn_x = 1e12f, mn_y = 1e12f, mx_x = -1e12f, mx_y = -1e12f;
        for (const auto& a : areas) {
            AABB ab = a.global_bounds();
            if (ab.min_x < mn_x) mn_x = ab.min_x;
            if (ab.min_y < mn_y) mn_y = ab.min_y;
            if (ab.max_x > mx_x) mx_x = ab.max_x;
            if (ab.max_y > mx_y) mx_y = ab.max_y;
        }
        return { mn_x, mx_x, mn_y, mx_y };
    }

    std::vector<const WorldArea*> areas_in_view(const Camera2D& cam) const {
        std::vector<const WorldArea*> result;
        AABB frustum = {
            cam.x, cam.x + cam.viewport_w,
            cam.y, cam.y + cam.viewport_h
        };
        for (const auto& a : areas)
            if (frustum.intersects(a.global_bounds()))
                result.push_back(&a);
        return result;
    }

    const WorldArea* area_at(float px, float py) const {
        for (const auto& a : areas) {
            AABB ab = a.global_bounds();
            if (px >= ab.min_x && px <= ab.max_x &&
                py >= ab.min_y && py <= ab.max_y)
                return &a;
        }
        return nullptr;
    }

    WorldArea* area_at_mut(float px, float py) {
        for (auto& a : areas) {
            AABB ab = a.global_bounds();
            if (px >= ab.min_x && px <= ab.max_x &&
                py >= ab.min_y && py <= ab.max_y)
                return &a;
        }
        return nullptr;
    }
};

} // namespace mion
