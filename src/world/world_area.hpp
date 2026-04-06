#pragma once

#include <vector>
#include <string>

#include "room.hpp"
#include "tilemap.hpp"
#include "../systems/pathfinder.hpp"
#include "../entities/actor.hpp"

namespace mion {

enum class ZoneId : int {
    Town,
    DungeonRoom0,
    DungeonRoom1,
    DungeonRoom2,
    Boss,
    Transition
};

struct WorldArea {
    ZoneId         zone      = ZoneId::Town;
    float          offset_x  = 0.f;
    float          offset_y  = 0.f;
    RoomDefinition room;
    Tilemap        tilemap;
    Pathfinder     pathfinder;
    std::vector<Actor> enemies;

    AABB global_bounds() const {
        return {
            offset_x + room.bounds.min_x,
            offset_x + room.bounds.max_x,
            offset_y + room.bounds.min_y,
            offset_y + room.bounds.max_y,
        };
    }
};

} // namespace mion
