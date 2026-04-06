#pragma once
#include <algorithm>
#include <vector>

#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/camera.hpp"
#include "../core/sprite.hpp"
#include "../entities/actor.hpp"
#include "../entities/ground_item.hpp"
#include "../entities/projectile.hpp"
#include "../world/room.hpp"
#include "../world/tilemap.hpp"
#include "floating_text.hpp"
#include "lighting.hpp"
#include "simple_particles.hpp"
#include "tilemap_renderer.hpp"
#include "world_renderer.hpp"

namespace mion {

struct DungeonWorldRenderInputs {
    /// Câmera em coordenadas **locais da área** (top-left = câmara mundo − offset da área).
    /// Usada para tilemap, obstáculos e portas (dados da room em espaço local).
    const Camera2D&                 camera;
    /// Câmera em **coordenadas de mundo** — atores, projéteis, drops, partículas, textos flutuantes.
    const Camera2D&                 world_camera;
    const RoomDefinition&           room;
    const Tilemap&                  tilemap;
    const TilemapRenderer&          tile_renderer;
    TextureCache*                   texture_cache;
    const std::vector<Actor*>&      actors;
    const std::vector<Projectile>&  projectiles;
    const std::vector<GroundItem>&  ground_items;
    const SimpleParticleSystem&     particles;
    const FloatingTextSystem&       floating_texts;
    LightingSystem&                 lighting;
    const Actor&                    player;
    bool                            room_cleared = false;
    // When false, skip full-screen torch mask (WorldScene applies it once after all areas).
    bool                            apply_lighting_overlay = true;
};

struct DungeonWorldRenderer {
    static void render(SDL_Renderer* r, const DungeonWorldRenderInputs& in) {
        in.tile_renderer.render(r, in.camera, in.tilemap);
        render_obstacles(r, in);
        render_doors(r, in);
        const Camera2D& wcam = in.world_camera;
        for (const auto* actor : in.actors)
            render_actor(r, wcam, *actor);
        in.particles.render(r, wcam);
        render_projectiles(r, in, wcam);
        render_ground_items(r, in, wcam);
        render_actor_hp_values(r, in, wcam);
        in.floating_texts.render(
            r,
            [&](float wx) { return wcam.world_to_screen_x(wx); },
            [&](float wy) { return wcam.world_to_screen_y(wy); });
        if (in.apply_lighting_overlay && in.player.is_alive) {
            const float lx = wcam.world_to_screen_x(in.player.transform.x);
            const float ly = wcam.world_to_screen_y(in.player.transform.y);
            in.lighting.render(r, lx, ly);
        }
    }

private:
    static void render_obstacles(SDL_Renderer* r, const DungeonWorldRenderInputs& in) {
        for (const auto& obstacle : in.room.obstacles) {
            const float cx = (obstacle.bounds.min_x + obstacle.bounds.max_x) * 0.5f;
            const float cy = (obstacle.bounds.min_y + obstacle.bounds.max_y) * 0.5f;
            const float hw = (obstacle.bounds.max_x - obstacle.bounds.min_x) * 0.5f;
            const float hh = (obstacle.bounds.max_y - obstacle.bounds.min_y) * 0.5f;
            if (!render_obstacle_sprite(r, in.camera, obstacle, in.texture_cache)) {
                draw_world_rect(r, in.camera, cx, cy, hw, hh, 60, 55, 70);
                draw_world_outline(r, in.camera, cx, cy, hw, hh, 90, 85, 100);
            }
        }
    }

    static void render_doors(SDL_Renderer* r, const DungeonWorldRenderInputs& in) {
        for (const auto& door : in.room.doors) {
            const float cx = (door.bounds.min_x + door.bounds.max_x) * 0.5f;
            const float cy = (door.bounds.min_y + door.bounds.max_y) * 0.5f;
            const float hw = (door.bounds.max_x - door.bounds.min_x) * 0.5f;
            const float hh = (door.bounds.max_y - door.bounds.min_y) * 0.5f;
            const bool  cleared = !door.requires_room_cleared || in.room_cleared;
            const char* door_path = cleared ? "assets/props/door_open.png"
                                            : "assets/props/door_closed.png";
            SDL_Texture* door_tex = in.texture_cache ? in.texture_cache->load(door_path) : nullptr;
            if (door_tex) {
                const float sx = in.camera.world_to_screen_x(cx - hw);
                const float sy = in.camera.world_to_screen_y(cy - hh);
                SDL_FRect    dst{sx, sy, hw * 2.0f, hh * 2.0f};
                SDL_RenderTexture(r, door_tex, nullptr, &dst);
            } else {
                const Uint8 dr = cleared ? 80 : 160;
                const Uint8 dg = cleared ? 140 : 80;
                const Uint8 db = cleared ? 80 : 40;
                draw_world_rect(r, in.camera, cx, cy, hw, hh, dr, dg, db);
            }
        }
    }

    static void render_projectiles(SDL_Renderer* r, const DungeonWorldRenderInputs& in,
                                   const Camera2D& wcam) {
        for (const auto& projectile : in.projectiles)
            draw_world_rect(r, wcam, projectile.x, projectile.y,
                            projectile.half_w, projectile.half_h, 255, 220, 60);
    }

    static void render_ground_items(SDL_Renderer* r, const DungeonWorldRenderInputs& in,
                                    const Camera2D& wcam) {
        for (const auto& item : in.ground_items) {
            if (!item.active)
                continue;
            Uint8 rr = 90;
            Uint8 gg = 220;
            Uint8 bb = 130;
            if (item.type == GroundItemType::Damage) {
                rr = 230;
                gg = 120;
                bb = 80;
            } else if (item.type == GroundItemType::Speed) {
                rr = 100;
                gg = 180;
                bb = 255;
            }
            draw_world_rect(r, wcam, item.x, item.y, 10.0f, 10.0f, rr, gg, bb);
        }
    }

    static void render_actor_hp_values(SDL_Renderer* r, const DungeonWorldRenderInputs& in,
                                       const Camera2D& wcam) {
        for (const auto* actor : in.actors) {
            if (!actor->is_alive)
                continue;
            char hp_text[8];
            SDL_snprintf(hp_text, sizeof(hp_text), "%d", actor->health.current_hp);
            const float sx = wcam.world_to_screen_x(actor->transform.x)
                           - text_width(hp_text, 1) * 0.5f;
            const float sy = wcam.world_to_screen_y(actor->transform.y) - 36.0f;
            draw_text(r, sx, sy, hp_text, 1, 200, 200, 200);
        }
    }
};

} // namespace mion
