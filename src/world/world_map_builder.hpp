#pragma once

#include "world_map.hpp"
#include "../core/sprite.hpp"
#include "../core/world_layout_ids.hpp"
#include "../core/ini_loader.hpp"
#include "../systems/town_builder.hpp"
#include "../systems/room_manager.hpp"
#include "../systems/tilemap_renderer.hpp"
#include "../entities/npc.hpp"
#include "../entities/shop.hpp"

namespace mion {

namespace WorldMapBuilder {

inline WorldMap build(const IniData& rooms_ini,
                      TextureCache* tex_cache,
                      std::vector<NpcEntity>& npcs,
                      ShopInventory& shop_forge,
                      Actor& player,
                      TilemapRenderer& town_tile_renderer) {
    WorldMap map;

    // --- Town area ---
    {
        WorldArea town;
        town.zone     = ZoneId::Town;
        town.offset_x = 0.f;
        town.offset_y = 0.f;
        TownBuilder::build_town_world(
            town.room, town.tilemap, town_tile_renderer,
            npcs, shop_forge, player, tex_cache);
        // Keep `TownBuilder` door zones (town→dungeon): used by town render and matches
        // `DoorZone` / `WorldLayoutId` contract; unlike dungeon areas, we do not strip doors here.
        town.pathfinder.build_nav(town.tilemap, town.room);
        map.areas.push_back(std::move(town));
    }

    const float town_width = map.areas[0].room.bounds.max_x;

    // --- Transition corridor ---
    {
        WorldArea corridor;
        corridor.zone     = ZoneId::Transition;
        corridor.offset_x = town_width;
        corridor.offset_y = 300.f;

        corridor.room.name   = WorldLayoutId::kCorridor;
        // Match dungeon room vertical span (900) so there is no "void" strip beside the corridor.
        corridor.room.bounds = { 0.f, 192.f, 0.f, 900.f };
        corridor.room.obstacles.clear();
        corridor.room.doors.clear();

        constexpr int ts = 32;
        const int     cols = 6;
        const int     rows = static_cast<int>(corridor.room.bounds.max_y / ts) + 1;
        corridor.tilemap.init(cols, rows, ts);
        corridor.tilemap.fill(0, 0, cols - 1, rows - 1, TileType::Floor);
        corridor.tilemap.fill(0, 0, 0, rows - 1, TileType::Wall);
        corridor.tilemap.fill(cols - 1, 0, cols - 1, rows - 1, TileType::Wall);

        corridor.pathfinder.build_nav(corridor.tilemap, corridor.room);
        map.areas.push_back(std::move(corridor));
    }

    const float dungeon_start_x = town_width + 192.f;

    // --- Dungeon rooms (0, 1, 2, boss=3) ---
    for (int ri = 0; ri <= 3; ++ri) {
        WorldArea area;
        switch (ri) {
            case 0: area.zone = ZoneId::DungeonRoom0; break;
            case 1: area.zone = ZoneId::DungeonRoom1; break;
            case 2: area.zone = ZoneId::DungeonRoom2; break;
            case 3: area.zone = ZoneId::Boss;         break;
        }

        RoomManager::build_room(area.room, area.tilemap, ri, rooms_ini);
        area.room.doors.clear();

        float prev_end = dungeon_start_x;
        for (int p = 0; p < ri; ++p) {
            for (const auto& a : map.areas) {
                if ((p == 0 && a.zone == ZoneId::DungeonRoom0) ||
                    (p == 1 && a.zone == ZoneId::DungeonRoom1) ||
                    (p == 2 && a.zone == ZoneId::DungeonRoom2)) {
                    float end = a.offset_x + (a.room.bounds.max_x - a.room.bounds.min_x);
                    if (end > prev_end) prev_end = end;
                }
            }
        }

        area.offset_x = prev_end;
        area.offset_y = 300.f;

        area.pathfinder.build_nav(area.tilemap, area.room);
        map.areas.push_back(std::move(area));
    }

    return map;
}

} // namespace WorldMapBuilder

} // namespace mion
