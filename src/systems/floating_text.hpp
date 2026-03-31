#pragma once
#include <algorithm>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include "../core/bitmap_font.hpp"

namespace mion {

struct FloatingText {
    float       x = 0.f;
    float       y = 0.f;
    float       vy       = -60.0f;
    float       lifetime = 1.0f;
    float       elapsed  = 0.0f;
    std::string text;
    int         scale = 2;
    SDL_Color   color{255, 255, 255, 255};

    bool is_dead() const { return elapsed >= lifetime; }
};

struct FloatingTextSystem {
    std::vector<FloatingText> texts;

    void spawn(float x, float y, const char* text,
               SDL_Color color = {255, 255, 255, 255}, int scale = 2,
               float lifetime = 0.9f, float vy = -55.0f) {
        texts.push_back({x, y, vy, lifetime, 0.0f, text, scale, color});
    }

    void spawn_damage(float x, float y, int damage) {
        char buf[16];
        SDL_snprintf(buf, sizeof(buf), "-%d", damage);
        spawn(x, y - 20.0f, buf, {255, 80, 60, 255}, 2);
    }

    void spawn_heal(float x, float y, int amount) {
        char buf[16];
        SDL_snprintf(buf, sizeof(buf), "+%d", amount);
        spawn(x, y - 20.0f, buf, {80, 220, 100, 255}, 2);
    }

    void spawn_level_up(float x, float y) {
        spawn(x, y - 32.0f, "LEVEL UP!", {255, 215, 0, 255}, 3, 1.4f, -45.0f);
    }

    void spawn_upgrade(float x, float y, const char* text) {
        spawn(x, y - 48.0f, text, {180, 255, 180, 255}, 2, 1.2f, -40.0f);
    }

    void tick(float dt) {
        for (auto& ft : texts) {
            ft.elapsed += dt;
            ft.y += ft.vy * dt;
        }
        texts.erase(std::remove_if(texts.begin(), texts.end(),
                                   [](const FloatingText& f) { return f.is_dead(); }),
                    texts.end());
    }

    void render(SDL_Renderer* r,
                const std::function<float(float)>& to_sx,
                const std::function<float(float)>& to_sy) const {
        for (const auto& ft : texts) {
            float alpha = 1.0f - (ft.elapsed / ft.lifetime);
            if (alpha < 0.0f) alpha = 0.0f;
            Uint8 a = (Uint8)(alpha * 255.0f);
            draw_text(r,
                      to_sx(ft.x) - text_width(ft.text.c_str(), ft.scale) * 0.5f,
                      to_sy(ft.y),
                      ft.text.c_str(), ft.scale,
                      ft.color.r, ft.color.g, ft.color.b, a);
        }
    }
};

} // namespace mion
