#pragma once
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <SDL3/SDL.h>

#include "../core/camera.hpp"
#include "../core/sprite.hpp"
#include "../core/animation.hpp"
#include "../entities/actor.hpp"
#include "../world/room.hpp"
#include "../components/combat.hpp"
#include "../components/collision.hpp"
#include "../components/status_effect.hpp"

namespace mion {

// ---------------------------------------------------------------
// Primitivas de render (mundo → tela, via Camera2D)
// ---------------------------------------------------------------

inline void draw_world_rect(SDL_Renderer* r, const Camera2D& cam,
                            float cx, float cy, float hw, float hh,
                            Uint8 R, Uint8 G, Uint8 B, Uint8 A = 255)
{
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_FRect rect = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(cy-hh),
                       hw*2.0f, hh*2.0f };
    SDL_RenderFillRect(r, &rect);
}

inline void draw_world_outline(SDL_Renderer* r, const Camera2D& cam,
                               float cx, float cy, float hw, float hh,
                               Uint8 R, Uint8 G, Uint8 B)
{
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_FRect rect = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(cy-hh),
                       hw*2.0f, hh*2.0f };
    SDL_RenderRect(r, &rect);
}

inline void draw_hp_bar(SDL_Renderer* r, const Camera2D& cam,
                        float cx, float top_y, float hw, int hp, int max_hp)
{
    const float bw = hw*2.0f, bh = 5.0f;
    const float by = top_y - 10.0f;
    SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
    SDL_FRect bg = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(by), bw, bh };
    SDL_RenderFillRect(r, &bg);
    float ratio = (max_hp > 0) ? (float)hp / max_hp : 0.0f;
    SDL_SetRenderDrawColor(r, (Uint8)(255*(1-ratio)), (Uint8)(255*ratio), 30, 255);
    SDL_FRect fill = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(by),
                       bw*ratio, bh };
    SDL_RenderFillRect(r, &fill);
}

// ---------------------------------------------------------------
// Obstacle sprite rendering
// ---------------------------------------------------------------

inline bool render_obstacle_sprite(SDL_Renderer* r, const Camera2D& cam,
                                   const Obstacle& obs, TextureCache* tex_cache)
{
    if (!tex_cache || obs.sprite_path.empty()) return false;

    std::error_code ec;
    if (!std::filesystem::exists(obs.sprite_path, ec) || ec) return false;

    SDL_Texture* tex = tex_cache->load(obs.sprite_path);
    if (!tex) return false;

    const float bounds_w = obs.bounds.max_x - obs.bounds.min_x;
    const float bounds_h = obs.bounds.max_y - obs.bounds.min_y;
    const float draw_w   = obs.sprite_world_w > 0.0f ? obs.sprite_world_w : bounds_w;
    const float draw_h   = obs.sprite_world_h > 0.0f ? obs.sprite_world_h : bounds_h;
    const float anchor_x = (obs.bounds.min_x + obs.bounds.max_x) * 0.5f;
    const float anchor_y = obs.bounds.max_y;

    SDL_FRect dst = {
        cam.world_to_screen_x(anchor_x - draw_w * obs.sprite_anchor_x),
        cam.world_to_screen_y(anchor_y - draw_h * obs.sprite_anchor_y),
        draw_w,
        draw_h
    };
    SDL_RenderTexture(r, tex, nullptr, &dst);
    return true;
}

// ---------------------------------------------------------------
// Actor status overlays (poison/slow/stun + hit flash)
// ---------------------------------------------------------------

inline void render_actor_status_overlay_screen(SDL_Renderer* r, float sx, float sy,
                                               float sw, float sh, const Actor& a) {
    if (a.hit_flash_timer > 0.0f) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 180);
        SDL_FRect rect = {sx, sy, sw, sh};
        SDL_RenderFillRect(r, &rect);
        return;
    }
    if (!a.is_alive) return;

    Uint8 R = 0, G = 0, B = 0, A = 0;
    if (a.status_effects.has(StatusEffectType::Poison)) {
        R = 80; G = 220; B = 80; A = 80;
    } else if (a.status_effects.has(StatusEffectType::Slow)) {
        R = 80; G = 120; B = 220; A = 80;
    } else if (a.status_effects.has(StatusEffectType::Stun)) {
        R = 240; G = 220; B = 60; A = 80;
    } else
        return;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_FRect rect = {sx, sy, sw, sh};
    SDL_RenderFillRect(r, &rect);
}

inline void render_actor_status_overlay_world(SDL_Renderer* r, const Camera2D& cam,
                                              float cx, float cy, float hw, float hh,
                                              const Actor& a) {
    float sx = cam.world_to_screen_x(cx - hw);
    float sy = cam.world_to_screen_y(cy - hh);
    float sw = cam.world_to_screen_x(cx + hw) - sx;
    float sh = cam.world_to_screen_y(cy + hh) - sy;
    render_actor_status_overlay_screen(r, sx, sy, sw, sh, a);
}

// ---------------------------------------------------------------
// Full actor rendering — sprite if available, colored rect fallback
// ---------------------------------------------------------------

inline void render_actor(SDL_Renderer* r, const Camera2D& cam, const Actor& a)
{
    // Keep rendering dead actors until death animation finishes
    bool death_done = !a.is_alive && a.anim.finished;
    if (death_done) return;

    float cx = a.transform.x, cy = a.transform.y;
    float hw = a.collision.half_w,  hh = a.collision.half_h;

    // --- Sprite path ---
    SDL_Texture* tex = a.sprite_sheet;
    if (tex) {
        const AnimFrame* af = a.anim.current_frame();
        if (af) {
            const float sprite_scale = a.sprite_scale;
            const float sprite_hw    = (float)af->src.w * sprite_scale * 0.5f;
            const float sprite_hh    = (float)af->src.h * sprite_scale * 0.5f;
            SpriteFrame frame;
            frame.texture = tex;
            frame.src     = { af->src.x, af->src.y, af->src.w, af->src.h };
            const float scr_cx = cam.world_to_screen_x(cx);
            const float scr_cy = cam.world_to_screen_y(cy);
            draw_sprite(r, frame,
                        scr_cx,
                        scr_cy,
                        sprite_scale, sprite_scale, a.facing_x < 0.0f);

            render_actor_status_overlay_screen(
                r, scr_cx - sprite_hw, scr_cy - sprite_hh, sprite_hw * 2.0f, sprite_hh * 2.0f, a);

            if (a.is_alive) {
                draw_hp_bar(r, cam, cx, cy - sprite_hh, std::max(hw, sprite_hw * 0.5f),
                             a.health.current_hp, a.health.max_hp);
                if (a.combat.is_in_active_phase()) {
                    AABB hb = a.melee_hit_box.bounds_at(cx, cy, a.facing_x, a.facing_y);
                    float hcx=(hb.min_x+hb.max_x)*0.5f, hcy=(hb.min_y+hb.max_y)*0.5f;
                    float hhw=(hb.max_x-hb.min_x)*0.5f, hhh=(hb.max_y-hb.min_y)*0.5f;
                    draw_world_outline(r, cam, hcx, hcy, hhw, hhh, 255, 60, 60);
                }
            }
            return;
        }
    }

    // --- Fallback rectangle ---
    if (!a.is_alive) return;

    if (a.team == Team::Player) {
        switch (a.combat.attack_phase) {
            case AttackPhase::Idle:     draw_world_rect(r,cam,cx,cy,hw,hh,180,210,255); break;
            case AttackPhase::Startup:  draw_world_rect(r,cam,cx,cy,hw,hh,255,220, 50); break;
            case AttackPhase::Active:   draw_world_rect(r,cam,cx,cy,hw,hh,255, 60, 60); break;
            case AttackPhase::Recovery: draw_world_rect(r,cam,cx,cy,hw,hh, 80, 80,220); break;
        }
    } else {
        if (a.combat.hurt_stun_remaining_seconds > 0.0f)
            draw_world_rect(r,cam,cx,cy,hw,hh,255,255,255);
        else
            draw_world_rect(r,cam,cx,cy,hw,hh,180,40,40);
    }

    draw_world_outline(r,cam,cx,cy,hw,hh,220,220,220);

    float fx = cx + a.facing_x*(hw+6.0f);
    float fy = cy + a.facing_y*(hh+6.0f);
    SDL_SetRenderDrawColor(r,255,255,100,255);
    SDL_RenderLine(r, cam.world_to_screen_x(cx), cam.world_to_screen_y(cy),
                      cam.world_to_screen_x(fx), cam.world_to_screen_y(fy));

    draw_hp_bar(r,cam,cx,cy - hh,hw, a.health.current_hp, a.health.max_hp);

    if (a.combat.is_in_active_phase()) {
        AABB hb = a.melee_hit_box.bounds_at(cx,cy,a.facing_x,a.facing_y);
        float hcx=(hb.min_x+hb.max_x)*0.5f, hcy=(hb.min_y+hb.max_y)*0.5f;
        float hhw=(hb.max_x-hb.min_x)*0.5f, hhh=(hb.max_y-hb.min_y)*0.5f;
        draw_world_outline(r,cam,hcx,hcy,hhw,hhh,255,60,60);
    }

    render_actor_status_overlay_world(r, cam, cx, cy, hw, hh, a);
}

// ---------------------------------------------------------------
// Maps facing direction to Puny Characters spritesheet row.
// Row 0 = South | Row 4 = North | Row 6 = East (West = Row 6 + flip).
// Shared between DungeonScene and TownScene.
// ---------------------------------------------------------------

inline int facing_to_puny_row(float fx, float fy) {
    const float ax = fx < 0.0f ? -fx : fx;
    const float ay = fy < 0.0f ? -fy : fy;
    if (ay > ax)
        return fy > 0.0f ? 0 : 4;  // South or North
    return 6;                        // horizontal; West via flip on facing_x
}

} // namespace mion
