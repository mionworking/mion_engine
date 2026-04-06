#pragma once

#include <vector>

#include "../core/bitmap_font.hpp"
#include "../core/camera.hpp"
#include "../core/sprite.hpp"
#include "../core/sprite_layout.hpp"
#include "../entities/npc.hpp"
#include "../systems/world_renderer.hpp"

namespace mion {

struct TownRenderContext {
    const Camera2D*                camera     = nullptr;
    const RoomDefinition*          room       = nullptr;
    const std::vector<NpcEntity>*  npcs       = nullptr;
    TextureCache*                  tex_cache  = nullptr;
    float                          player_sprite_scale = 2.0f;
};

namespace TownWorldRenderer {

inline void render_buildings(SDL_Renderer* r, const Camera2D& camera,
                             const RoomDefinition& room, TextureCache* tex_cache) {
    for (const auto& obs : room.obstacles) {
        const float x0 = camera.world_to_screen_x(obs.bounds.min_x);
        const float y0 = camera.world_to_screen_y(obs.bounds.min_y);
        const float bw = obs.bounds.max_x - obs.bounds.min_x;
        const float bh = obs.bounds.max_y - obs.bounds.min_y;

        auto has = [&](const char* s) { return obs.name.find(s) != obs.name.npos; };
        const char* sprite_path = nullptr;
        if (has("tavern")) sprite_path = "assets/props/building_tavern.png";
        else if (has("forge")) sprite_path = "assets/props/building_forge.png";
        else if (has("elder")) sprite_path = "assets/props/building_elder.png";
        else if (has("fountain")) sprite_path = "assets/props/fountain.png";

        SDL_Texture* texture = (sprite_path && tex_cache) ? tex_cache->load(sprite_path) : nullptr;
        if (texture) {
            SDL_FRect dst = {x0, y0, bw, bh};
            SDL_RenderTexture(r, texture, nullptr, &dst);
            continue;
        }

        const float cx = (obs.bounds.min_x + obs.bounds.max_x) * 0.5f;
        const float cy = (obs.bounds.min_y + obs.bounds.max_y) * 0.5f;
        const float hw = bw * 0.5f;
        const float hh = bh * 0.5f;
        Uint8 rr = 80, gg = 90, bb = 100;
        if (has("tavern")) { rr = 120; gg = 90; bb = 40; }
        else if (has("forge")) { rr = 90; gg = 88; bb = 95; }
        else if (has("elder")) { rr = 70; gg = 50; bb = 90; }
        else if (has("fountain")) { rr = 50; gg = 100; bb = 160; }
        draw_world_rect(r, camera, cx, cy, hw, hh, rr, gg, bb);
    }
}

inline void render_doors(SDL_Renderer* r, const Camera2D& camera,
                         const RoomDefinition& room, TextureCache* tex_cache) {
    for (const auto& door : room.doors) {
        const float cx = (door.bounds.min_x + door.bounds.max_x) * 0.5f;
        const float cy = (door.bounds.min_y + door.bounds.max_y) * 0.5f;
        const float hw = (door.bounds.max_x - door.bounds.min_x) * 0.5f;
        const float hh = (door.bounds.max_y - door.bounds.min_y) * 0.5f;
        SDL_Texture* texture = tex_cache ? tex_cache->load("assets/props/door_open.png") : nullptr;
        if (texture) {
            SDL_FRect dst = {
                camera.world_to_screen_x(cx - hw),
                camera.world_to_screen_y(cy - hh),
                hw * 2.0f,
                hh * 2.0f
            };
            SDL_RenderTexture(r, texture, nullptr, &dst);
        } else {
            draw_world_rect(r, camera, cx, cy, hw, hh, 80, 140, 80);
        }
    }
}

inline void render_npcs(SDL_Renderer* r, const Camera2D& camera,
                        const std::vector<NpcEntity>& npcs,
                        TextureCache* tex_cache,
                        float player_sprite_scale) {
    static const struct { const char* name; const char* path; } kNpcSprites[] = {
        {"Mira", "assets/npcs/npc_mira.png"},
        {"Forge", "assets/npcs/npc_forge.png"},
        {"Villager", "assets/npcs/npc_villager.png"},
        {"Elder", "assets/npcs/npc_elder.png"},
    };

    for (const auto& npc : npcs) {
        SDL_Texture* texture = nullptr;
        if (tex_cache) {
            for (const auto& sprite : kNpcSprites) {
                if (npc.name == sprite.name) {
                    texture = tex_cache->load(sprite.path);
                    break;
                }
            }
        }

        if (texture) {
            constexpr float kFW = static_cast<float>(SpriteLayout::PunyFrameW);
            constexpr float kFH = static_cast<float>(SpriteLayout::PunyFrameH);
            SDL_FRect src = {12.0f * kFW, 0.0f, kFW, kFH};

            float scale = player_sprite_scale > 0.0f ? player_sprite_scale : 2.0f;
            float draw_w = kFW * scale;
            float draw_h = kFH * scale;
            SDL_FRect dst = {
                camera.world_to_screen_x(npc.actor ? npc.actor->transform.x : npc.x) - draw_w * 0.5f,
                camera.world_to_screen_y(npc.actor ? npc.actor->transform.y : npc.y) - draw_h * 0.5f,
                draw_w,
                draw_h
            };
            SDL_RenderTexture(r, texture, &src, &dst);
        } else {
            const float cx = npc.actor ? npc.actor->transform.x : npc.x;
            const float cy = npc.actor ? npc.actor->transform.y : npc.y;
            draw_world_rect(r, camera, cx, cy, 14.0f, 18.0f,
                            npc.portrait_color.r, npc.portrait_color.g, npc.portrait_color.b);
        }

        const float label_x = (npc.actor ? npc.actor->transform.x : npc.x) - 40.0f;
        const float label_y = (npc.actor ? npc.actor->transform.y : npc.y) - 36.0f;
        draw_text(r,
                  camera.world_to_screen_x(label_x),
                  camera.world_to_screen_y(label_y),
                  npc.name.c_str(),
                  2,
                  220, 220, 200, 255);
    }
}

inline void render_town_world(SDL_Renderer* r, const TownRenderContext& ctx) {
    if (!ctx.camera || !ctx.room || !ctx.npcs)
        return;
    render_buildings(r, *ctx.camera, *ctx.room, ctx.tex_cache);
    render_doors(r, *ctx.camera, *ctx.room, ctx.tex_cache);
    render_npcs(r, *ctx.camera, *ctx.npcs, ctx.tex_cache, ctx.player_sprite_scale);
}

} // namespace TownWorldRenderer

} // namespace mion
