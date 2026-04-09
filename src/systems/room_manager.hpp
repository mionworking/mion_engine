#pragma once
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "../core/run_stats.hpp"
#include "../core/world_layout_ids.hpp"
#include "../entities/actor.hpp"
#include "../entities/ground_item.hpp"
#include "../entities/projectile.hpp"
#include "../world/dungeon_rules.hpp"
#include "../world/room.hpp"
#include "../world/room_loader.hpp"
#include "../world/tilemap.hpp"

namespace mion {

struct RoomManager {
    static void reset_room_runtime(std::vector<Projectile>& projectiles,
                                   std::vector<GroundItem>& ground_items) {
        projectiles.clear();
        ground_items.clear();
    }

    static void begin_next_room(int& room_index, RunStats* run_stats) {
        ++room_index;
        if (run_stats)
            run_stats->rooms_cleared++;
    }

    static void build_room(RoomDefinition& room,
                           Tilemap& tilemap,
                           int room_index,
                           const IniData& rooms_ini) {
        char room_name[48];
        SDL_snprintf(room_name, sizeof(room_name), "dungeon_r%02d", room_index);
        room.name      = room_name;
        room.bounds    = dungeon_rules::room_bounds(room_index);
        room.obstacles.clear();
        room.doors.clear();

        const WorldBounds& bounds = room.bounds;
        const float        midy   = (bounds.min_y + bounds.max_y) * 0.5f;

        if (room_index < 3) {
            room.add_door(bounds.max_x - 72.0f, midy - 56.0f,
                          bounds.max_x - 20.0f, midy + 56.0f, true);
            room.add_zone_door(bounds.min_x + 20.0f, midy - 56.0f,
                               bounds.min_x + 72.0f, midy + 56.0f, false, WorldLayoutId::kTown);
        } else {
            room.add_zone_door(bounds.max_x - 72.0f, midy - 56.0f,
                               bounds.max_x - 20.0f, midy + 56.0f, true, WorldLayoutId::kTown);
        }

        const int         tmpl = dungeon_rules::room_template(room_index);
        const char*       tkey = dungeon_rules::room_template_id_name(tmpl);
        const std::string tstr(tkey);
        if (!load_room_obstacles_from_ini(room, rooms_ini, bounds, tstr))
            apply_room_layout_template(room, tmpl, bounds);

        constexpr int tile_size = 32;
        const int     cols      = static_cast<int>(bounds.max_x / tile_size) + 1;
        const int     rows      = static_cast<int>(bounds.max_y / tile_size) + 1;
        tilemap.init(cols, rows, tile_size);
        tilemap.fill(0, 0, cols - 1, rows - 1, TileType::Floor);
        tilemap.fill(0, 0, cols - 1, 0, TileType::Wall);
        tilemap.fill(0, rows - 1, cols - 1, rows - 1, TileType::Wall);
        tilemap.fill(0, 0, 0, rows - 1, TileType::Wall);
        tilemap.fill(cols - 1, 0, cols - 1, rows - 1, TileType::Wall);
    }

    static void place_player_at_room_center(Actor& player, const RoomDefinition& room) {
        player.transform.set_position(room.bounds.max_x * 0.5f, room.bounds.max_y * 0.5f);
    }

    static void place_player_intro_spawn(Actor& player, const RoomDefinition& room) {
        const WorldBounds& bounds = room.bounds;
        const float        width  = bounds.max_x - bounds.min_x;
        const float        height = bounds.max_y - bounds.min_y;
        player.transform.set_position(bounds.min_x + width * 0.25f,
                                      bounds.min_y + height * 0.5f);
    }

    static void place_player_from_west_gate(Actor& player, const RoomDefinition& room) {
        const WorldBounds& bounds = room.bounds;
        const float        width  = bounds.max_x - bounds.min_x;
        player.transform.set_position(bounds.min_x + width * 0.1f,
                                      (bounds.min_y + bounds.max_y) * 0.5f);
    }

    static void place_player_for_loaded_room(Actor& player,
                                             const RoomDefinition& room,
                                             bool stress_mode,
                                             int room_index) {
        if (stress_mode)
            place_player_at_room_center(player, room);
        else if (room_index == 0)
            place_player_intro_spawn(player, room);
        else
            place_player_from_west_gate(player, room);
    }

    static void place_player_for_room_advance(Actor& player,
                                              const RoomDefinition& room,
                                              bool stress_mode) {
        if (stress_mode)
            place_player_at_room_center(player, room);
        else
            place_player_from_west_gate(player, room);
        player.knockback_vx = 0.0f;
        player.knockback_vy = 0.0f;
    }

private:
    static void layout_intro(RoomDefinition& room, const WorldBounds& bounds) {
        const float width = bounds.max_x - bounds.min_x;
        const float cx    = (bounds.min_x + bounds.max_x) * 0.5f + width * 0.04f;
        const float cy    = (bounds.min_y + bounds.max_y) * 0.5f;
        room.add_altar(cx, cy);
    }

    static void layout_boss_arena(RoomDefinition& room, const WorldBounds& bounds) {
        const float width  = bounds.max_x - bounds.min_x;
        const float height = bounds.max_y - bounds.min_y;
        const float pillar_half = 40.0f;
        const float inset_x     = width * 0.18f;
        const float inset_y     = height * 0.18f;
        auto pillar = [&](float px, float py) {
            room.add_obstacle("pillar", px - pillar_half, py - pillar_half,
                              px + pillar_half, py + pillar_half,
                              "assets/props/dungeon_pillar.png", 96.0f, 128.0f);
        };
        pillar(bounds.min_x + inset_x, bounds.min_y + inset_y);
        pillar(bounds.max_x - inset_x, bounds.min_y + inset_y);
        pillar(bounds.min_x + inset_x, bounds.max_y - inset_y);
        pillar(bounds.max_x - inset_x, bounds.max_y - inset_y);
    }

    static void layout_arena(RoomDefinition& room, const WorldBounds& bounds) {
        const float width  = bounds.max_x - bounds.min_x;
        const float height = bounds.max_y - bounds.min_y;
        const float cx     = (bounds.min_x + bounds.max_x) * 0.5f;
        const float cy     = (bounds.min_y + bounds.max_y) * 0.5f;
        const float ox     = width * 0.2f;
        const float oy     = height * 0.2f;
        const float pillar_half = 40.0f;
        auto pillar = [&](float px, float py) {
            room.add_obstacle("pillar", px - pillar_half, py - pillar_half,
                              px + pillar_half, py + pillar_half,
                              "assets/props/dungeon_pillar.png", 96.0f, 128.0f);
        };
        pillar(cx - ox, cy - oy);
        pillar(cx + ox, cy - oy);
        pillar(cx - ox, cy + oy);
        pillar(cx + ox, cy + oy);
        room.add_barrel_cluster(bounds.min_x + width * 0.14f, bounds.min_y + height * 0.14f);
        room.add_barrel_cluster(bounds.max_x - width * 0.14f, bounds.max_y - height * 0.14f);
    }

    static void layout_corredor(RoomDefinition& room, const WorldBounds& bounds) {
        const float width  = bounds.max_x - bounds.min_x;
        const float height = bounds.max_y - bounds.min_y;
        const float cx     = (bounds.min_x + bounds.max_x) * 0.5f;
        const float cy     = (bounds.min_y + bounds.max_y) * 0.5f;
        room.add_wall_L(bounds.min_x + width * 0.22f, bounds.min_y + height * 0.22f, 0);
        room.add_wall_L(bounds.max_x - width * 0.22f, bounds.max_y - height * 0.22f, 3);
        room.add_pillar_pair(cx, cy - height * 0.12f, 0);
        room.add_pillar_pair(cx, cy + height * 0.12f, 1);
    }

    static void layout_cruzamento(RoomDefinition& room, const WorldBounds& bounds) {
        const float width   = bounds.max_x - bounds.min_x;
        const float height  = bounds.max_y - bounds.min_y;
        const float cx      = (bounds.min_x + bounds.max_x) * 0.5f;
        const float cy      = (bounds.min_y + bounds.max_y) * 0.5f;
        const float inset_x = width * 0.15f;
        const float inset_y = height * 0.15f;
        room.add_obstacle("cross_v", cx - 20.0f, bounds.min_y + inset_y,
                          cx + 20.0f, bounds.max_y - inset_y,
                          "assets/props/dungeon_wall_mid.png", 40.0f, height * 0.7f,
                          0.5f, 0.5f);
        room.add_obstacle("cross_h", bounds.min_x + inset_x, cy - 20.0f,
                          bounds.max_x - inset_x, cy + 20.0f,
                          "assets/props/dungeon_wall_mid.png", width * 0.7f, 40.0f,
                          0.5f, 0.5f);
    }

    static void layout_labirinto(RoomDefinition& room, const WorldBounds& bounds) {
        const float width  = bounds.max_x - bounds.min_x;
        const float height = bounds.max_y - bounds.min_y;
        const float cx     = (bounds.min_x + bounds.max_x) * 0.5f;
        const float cy     = (bounds.min_y + bounds.max_y) * 0.5f;
        room.add_wall_L(bounds.min_x + width * 0.28f, bounds.max_y - height * 0.28f, 2);
        room.add_wall_L(bounds.max_x - width * 0.28f, bounds.min_y + height * 0.28f, 1);
        room.add_altar(cx, cy);
    }

    static void apply_room_layout_template(RoomDefinition& room,
                                           int tmpl,
                                           const WorldBounds& bounds) {
        switch (tmpl) {
        case 0:
            layout_arena(room, bounds);
            break;
        case 1:
            layout_corredor(room, bounds);
            break;
        case 2:
            layout_cruzamento(room, bounds);
            break;
        case 3:
            layout_labirinto(room, bounds);
            break;
        case 4:
            layout_boss_arena(room, bounds);
            break;
        default:
            layout_intro(room, bounds);
            break;
        }
    }
};

} // namespace mion
