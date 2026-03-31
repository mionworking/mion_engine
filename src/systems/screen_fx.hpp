#pragma once
#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"

namespace mion {

struct ScreenFx {
    static void render_boss_intro(SDL_Renderer* r,
                                  int viewport_w,
                                  int viewport_h,
                                  bool enabled,
                                  float intro_timer,
                                  float intro_duration) {
        if (!enabled)
            return;

        float t = intro_timer / intro_duration;
        float alpha = (t < 0.4f) ? (t / 0.4f)
                    : (t < 0.8f) ? 1.0f
                                 : (1.0f - (t - 0.8f) / 0.2f);
        if (alpha < 0.0f)
            alpha = 0.0f;
        if (alpha > 1.0f)
            alpha = 1.0f;

        const Uint8 text_alpha = static_cast<Uint8>(alpha * 255.0f);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, static_cast<Uint8>(alpha * 160.0f));
        SDL_FRect full{0, 0, static_cast<float>(viewport_w), static_cast<float>(viewport_h)};
        SDL_RenderFillRect(r, &full);

        const char* boss_name = "GRIMJAW";
        const int   boss_scale = 5;
        const float boss_x = viewport_w * 0.5f - text_width(boss_name, boss_scale) * 0.5f;
        const float boss_y = viewport_h * 0.42f;
        draw_text(r, boss_x, boss_y, boss_name, boss_scale, 220, 60, 40, text_alpha);

        const char* sub = "Guardian of the Third Depth";
        const int   sub_scale = 2;
        const float sub_x = viewport_w * 0.5f - text_width(sub, sub_scale) * 0.5f;
        draw_text(r, sub_x, boss_y + boss_scale * 12 + 8, sub, sub_scale,
                  180, 140, 120, static_cast<Uint8>(alpha * 200.0f));
    }

    static void render_death_fade(SDL_Renderer* r,
                                  int viewport_w,
                                  int viewport_h,
                                  bool player_is_alive,
                                  float fade_remaining,
                                  float fade_duration) {
        if (player_is_alive || fade_remaining <= 0.0f)
            return;

        float alpha = 1.0f - (fade_remaining / fade_duration);
        if (alpha < 0.0f)
            alpha = 0.0f;
        if (alpha > 1.0f)
            alpha = 1.0f;

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 80, 5, 5, static_cast<Uint8>(alpha * 220.0f));
        SDL_FRect full{0, 0, static_cast<float>(viewport_w), static_cast<float>(viewport_h)};
        SDL_RenderFillRect(r, &full);
    }

    static void render_screen_flash(SDL_Renderer* r,
                                    int viewport_w,
                                    int viewport_h,
                                    float flash_timer,
                                    float flash_duration,
                                    SDL_Color flash_color) {
        if (flash_timer <= 0.0f || flash_duration <= 1e-6f)
            return;

        float alpha = flash_timer / flash_duration;
        if (alpha > 1.0f)
            alpha = 1.0f;

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, flash_color.r, flash_color.g, flash_color.b,
                               static_cast<Uint8>(alpha * static_cast<float>(flash_color.a)));
        SDL_FRect full{0, 0, static_cast<float>(viewport_w), static_cast<float>(viewport_h)};
        SDL_RenderFillRect(r, &full);
    }
};

} // namespace mion
