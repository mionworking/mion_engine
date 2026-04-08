#pragma once

#include <algorithm>

#include "../core/data_section_names.hpp"
#include "../entities/enemy_type.hpp"
#include "room.hpp"

namespace mion {
namespace dungeon_rules {

// XP awarded per enemy kill (mirrors DungeonScene scaling).
inline int xp_per_enemy_kill(int room_index) { return 14 + room_index * 6; }

// Enemy spawn budget in normal mode (stress=false).
inline int enemy_spawn_budget_normal(int room_index) {
    return (int)std::min(3 + room_index * 2, 80);
}

inline int enemy_spawn_budget_stress(int requested_count) {
    return std::max(1, requested_count);
}

inline EnemyType enemy_type_for_spawn_index(int idx) {
    switch (idx % 6) {
        case 0: return EnemyType::Skeleton;
        case 1: return EnemyType::Orc;
        case 2: return EnemyType::Ghost;
        case 3: return EnemyType::Archer;
        case 4: return EnemyType::PatrolGuard;
        default: return EnemyType::EliteSkeleton;
    }
}

inline const char* enemy_type_name(EnemyType type) {
    const int idx = static_cast<int>(type);
    if (idx >= 0 && idx < static_cast<int>(data_sections::kEnemyIniSections.size()))
        return data_sections::kEnemyIniSections[idx];
    return "enemy";
}

// Playable area of the room (origin at 0,0 — aligned to the tilemap).
inline WorldBounds room_bounds(int room_index) {
    float max_x = 1600.0f;
    float max_y = 1200.0f;
    if (room_index <= 2) {
        max_x = 1200.0f;
        max_y = 900.0f;
    } else if (room_index <= 5) {
        max_x = 1400.0f;
        max_y = 1050.0f;
    } else if (room_index <= 9) {
        max_x = 1600.0f;
        max_y = 1200.0f;
    } else {
        max_x = 1800.0f;
        max_y = 1350.0f;
    }
    return { 0.0f, max_x, 0.0f, max_y };
}

// Template index: 0–3 = normal combat; 4 = boss/rest room; 5 = intro.
inline int room_template(int room_index) {
    if (room_index == 0) return 5;
    if (room_index % 3 == 0) return 4;
    return room_index % 4;
}

// Section key used in `data/rooms.ini` (e.g. `[arena]`, `[arena.obstacle.0]`).
inline const char* room_template_id_name(int tmpl) {
    switch (tmpl) {
    case 0: return "arena";
    case 1: return "corredor";
    case 2: return "cruzamento";
    case 3: return "labirinto";
    case 4: return "boss_arena";
    default: return "intro";
    }
}

} // namespace dungeon_rules
} // namespace mion
