#pragma once
#include <vector>
#include <string>
#include "../components/collision.hpp"

namespace mion {

// A collidable prop placed in a room (wall segment, barrel, pillar, etc.).
struct Obstacle {
    std::string name;
    AABB        bounds;
    std::string sprite_path;
    float       sprite_world_w = 0.0f;
    float       sprite_world_h = 0.0f;
    float       sprite_anchor_x = 0.5f;
    float       sprite_anchor_y = 1.0f;
};

// Axis-aligned playable region of a room.
struct WorldBounds {
    float min_x = 0.0f;
    float max_x = 800.0f;
    float min_y = 0.0f;
    float max_y = 600.0f;

    // Clamp a box center so it stays fully inside the bounds.
    void clamp_box(float& cx, float& cy, float half_w, float half_h) const {
        if (cx - half_w < min_x) cx = min_x + half_w;
        if (cx + half_w > max_x) cx = max_x - half_w;
        if (cy - half_h < min_y) cy = min_y + half_h;
        if (cy + half_h > max_y) cy = max_y - half_h;
    }
};

// Trigger zone that moves the player to the next room or a different scene.
struct DoorZone {
    AABB        bounds;
    bool        requires_room_cleared = true;
    std::string target_scene_id; // empty = advance to next room; otherwise switches scene
};

// Full runtime description of a single dungeon room.
struct RoomDefinition {
    std::string          name;
    WorldBounds          bounds;
    std::vector<Obstacle> obstacles;
    std::vector<DoorZone> doors;

    void add_door(float x1, float y1, float x2, float y2, bool requires_clear = true,
                  std::string target_scene = {}) {
        doors.push_back({ { x1, x2, y1, y2 }, requires_clear, std::move(target_scene) });
    }

    void add_obstacle(std::string n, float x1, float y1, float x2, float y2,
                      std::string sprite_path = "",
                      float sprite_world_w = 0.0f,
                      float sprite_world_h = 0.0f,
                      float sprite_anchor_x = 0.5f,
                      float sprite_anchor_y = 1.0f) {
        obstacles.push_back({
            std::move(n),
            {x1, x2, y1, y2},
            std::move(sprite_path),
            sprite_world_w,
            sprite_world_h,
            sprite_anchor_x,
            sprite_anchor_y,
        });
    }

    // Four 24×24 barrels in an irregular cluster around (cx, cy).
    void add_barrel_cluster(float cx, float cy) {
        const float s = 24.0f;
        add_obstacle("barrel_a", cx - 38.0f, cy - 8.0f, cx - 14.0f, cy + 16.0f,
                     "assets/props/barrel.png", s, s);
        add_obstacle("barrel_b", cx + 4.0f, cy - 22.0f, cx + 28.0f, cy + 2.0f,
                     "assets/props/barrel.png", s, s);
        add_obstacle("barrel_c", cx - 12.0f, cy + 10.0f, cx + 12.0f, cy + 34.0f,
                     "assets/props/barrel.png", s, s);
        add_obstacle("barrel_d", cx + 18.0f, cy + 6.0f, cx + 42.0f, cy + 30.0f,
                     "assets/props/barrel.png", s, s);
    }

    // Two L-shaped wall segments; orientation: 0=NW 1=NE 2=SW 3=SE (inner corner of the L).
    void add_wall_L(float corner_x, float corner_y, int orientation) {
        const float t = 40.0f;
        const float L = 150.0f;
        switch (orientation & 3) {
        case 0: // NW
            add_obstacle("wall_L_h", corner_x - L, corner_y - t * 0.5f, corner_x,
                         corner_y + t * 0.5f,
                         "assets/props/dungeon_wall_mid.png", L, t);
            add_obstacle("wall_L_v", corner_x - t * 0.5f, corner_y - L,
                         corner_x + t * 0.5f, corner_y,
                         "assets/props/dungeon_wall_mid.png", t, L);
            break;
        case 1: // NE
            add_obstacle("wall_L_h", corner_x, corner_y - t * 0.5f, corner_x + L,
                         corner_y + t * 0.5f,
                         "assets/props/dungeon_wall_mid.png", L, t);
            add_obstacle("wall_L_v", corner_x - t * 0.5f, corner_y - L,
                         corner_x + t * 0.5f, corner_y,
                         "assets/props/dungeon_wall_mid.png", t, L);
            break;
        case 2: // SW
            add_obstacle("wall_L_h", corner_x - L, corner_y - t * 0.5f, corner_x,
                         corner_y + t * 0.5f,
                         "assets/props/dungeon_wall_mid.png", L, t);
            add_obstacle("wall_L_v", corner_x - t * 0.5f, corner_y,
                         corner_x + t * 0.5f, corner_y + L,
                         "assets/props/dungeon_wall_mid.png", t, L);
            break;
        default: // SE (3)
            add_obstacle("wall_L_h", corner_x, corner_y - t * 0.5f, corner_x + L,
                         corner_y + t * 0.5f,
                         "assets/props/dungeon_wall_mid.png", L, t);
            add_obstacle("wall_L_v", corner_x - t * 0.5f, corner_y,
                         corner_x + t * 0.5f, corner_y + L,
                         "assets/props/dungeon_wall_mid.png", t, L);
            break;
        }
    }

    void add_altar(float cx, float cy) {
        add_obstacle("altar", cx - 40.0f, cy - 40.0f, cx + 40.0f, cy + 40.0f,
                     "assets/props/altar.png", 80.0f, 80.0f);
    }

    // Two 80×80 pillars centered 60 px apart; axis 0=horizontal 1=vertical.
    void add_pillar_pair(float cx, float cy, int axis) {
        const float ph = 40.0f;
        if ((axis % 2) == 0) {
            const float xl = cx - 60.0f;
            const float xr = cx + 60.0f;
            add_obstacle("pillar_l", xl - ph, cy - ph, xl + ph, cy + ph,
                         "assets/props/dungeon_pillar.png", 96.0f, 128.0f);
            add_obstacle("pillar_r", xr - ph, cy - ph, xr + ph, cy + ph,
                         "assets/props/dungeon_pillar.png", 96.0f, 128.0f);
        } else {
            const float yu = cy - 60.0f;
            const float yd = cy + 60.0f;
            add_obstacle("pillar_u", cx - ph, yu - ph, cx + ph, yu + ph,
                         "assets/props/dungeon_pillar.png", 96.0f, 128.0f);
            add_obstacle("pillar_d", cx - ph, yd - ph, cx + ph, yd + ph,
                         "assets/props/dungeon_pillar.png", 96.0f, 128.0f);
        }
    }
};

} // namespace mion
