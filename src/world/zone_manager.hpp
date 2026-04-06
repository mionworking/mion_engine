#pragma once

#include "world_map.hpp"

namespace mion {

struct ZoneManager {
    ZoneId current      = ZoneId::Town;
    ZoneId previous     = ZoneId::Town;
    bool   just_changed = false;

    void update(float player_x, float player_y, const WorldMap& map) {
        const WorldArea* area = map.area_at(player_x, player_y);
        ZoneId detected = area ? area->zone : current;

        just_changed = (detected != current);
        if (just_changed) {
            previous = current;
            current  = detected;
        }
    }

    bool in_dungeon() const {
        return current == ZoneId::DungeonRoom0
            || current == ZoneId::DungeonRoom1
            || current == ZoneId::DungeonRoom2
            || current == ZoneId::Boss;
    }

    bool in_town() const { return current == ZoneId::Town; }

    int room_index_equiv() const {
        switch (current) {
            case ZoneId::DungeonRoom0: return 0;
            case ZoneId::DungeonRoom1: return 1;
            case ZoneId::DungeonRoom2: return 2;
            case ZoneId::Boss:         return 3;
            default: return 0;
        }
    }
};

} // namespace mion
