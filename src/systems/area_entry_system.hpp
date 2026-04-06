#pragma once

#include <set>

#include "../world/world_area.hpp"
#include "../world/world_context.hpp"
#include "../world/zone_manager.hpp"

namespace mion {

struct AreaEntrySystem {
    std::set<ZoneId> visited_areas;

    using SpawnCallback = void(*)(WorldArea& area, WorldContext& ctx);

    void update(const ZoneManager& zone, WorldContext ctx,
                SpawnCallback spawn_cb) {
        if (!zone.just_changed) return;
        if (visited_areas.count(zone.current)) return;
        if (zone.current == ZoneId::Town || zone.current == ZoneId::Transition) {
            visited_areas.insert(zone.current);
            return;
        }

        if (ctx.world_map && ctx.player && spawn_cb) {
            WorldArea* area = ctx.world_map->area_at_mut(
                ctx.player->transform.x, ctx.player->transform.y);
            if (area)
                spawn_cb(*area, ctx);
        }

        visited_areas.insert(zone.current);
    }

    uint8_t to_mask() const {
        uint8_t m = 0;
        if (visited_areas.count(ZoneId::DungeonRoom0)) m |= 0x01;
        if (visited_areas.count(ZoneId::DungeonRoom1)) m |= 0x02;
        if (visited_areas.count(ZoneId::DungeonRoom2)) m |= 0x04;
        if (visited_areas.count(ZoneId::Boss))         m |= 0x08;
        return m;
    }

    void from_mask(uint8_t mask) {
        visited_areas.clear();
        if (mask & 0x01) visited_areas.insert(ZoneId::DungeonRoom0);
        if (mask & 0x02) visited_areas.insert(ZoneId::DungeonRoom1);
        if (mask & 0x04) visited_areas.insert(ZoneId::DungeonRoom2);
        if (mask & 0x08) visited_areas.insert(ZoneId::Boss);
    }
};

} // namespace mion
