#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "bitmap_font.hpp"

namespace mion::ui {

inline void draw_panel_border(SDL_Renderer* r, const SDL_FRect& rect, int border_px,
                              SDL_Color border) {
    SDL_SetRenderDrawColor(r, border.r, border.g, border.b, border.a);
    const float x = rect.x, y = rect.y, w = rect.w, h = rect.h;
    const float t = (float)border_px;
    SDL_FRect top{x, y, w, t};
    SDL_FRect bot{x, y + h - t, w, t};
    SDL_FRect lef{x, y, t, h};
    SDL_FRect rig{x + w - t, y, t, h};
    SDL_RenderFillRect(r, &top);
    SDL_RenderFillRect(r, &bot);
    SDL_RenderFillRect(r, &lef);
    SDL_RenderFillRect(r, &rig);
}

struct Panel {
    SDL_FRect rect{};
    SDL_Color bg{10, 8, 20, 220};
    SDL_Color border{180, 160, 100, 255};
    int       border_px = 2;

    void render(SDL_Renderer* r) const {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderFillRect(r, &rect);
        if (border_px > 0)
            draw_panel_border(r, rect, border_px, border);
    }
};

struct Label {
    float       x = 0.0f;
    float       y = 0.0f;
    std::string text;
    int         scale = 2;
    SDL_Color   color{230, 230, 210, 255};

    void render(SDL_Renderer* r) const {
        draw_text(r, x, y, text.c_str(), scale, color.r, color.g, color.b, color.a);
    }
};

struct List {
    std::vector<std::string> items;
    int                      selected      = 0;
    int                      item_h_px     = 28;
    int                      text_scale    = 2;
    SDL_Color                color_normal{200, 200, 190, 255};
    SDL_Color                color_selected{255, 220, 60, 255};
    SDL_Color                color_disabled{100, 95, 85, 255};
    std::vector<bool>        disabled;

    bool is_disabled(int idx) const {
        return idx >= 0 && idx < static_cast<int>(disabled.size()) && disabled[static_cast<size_t>(idx)];
    }

    void nav_up() {
        const int n = static_cast<int>(items.size());
        if (n <= 0) return;
        const int start = selected;
        for (int k = 0; k < n; ++k) {
            selected = (selected - 1 + n) % n;
            if (!is_disabled(selected)) return;
        }
        selected = start;
    }

    void nav_down() {
        const int n = static_cast<int>(items.size());
        if (n <= 0) return;
        const int start = selected;
        for (int k = 0; k < n; ++k) {
            selected = (selected + 1) % n;
            if (!is_disabled(selected)) return;
        }
        selected = start;
    }

    void render(SDL_Renderer* r, float x, float y) const {
        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            const float       row_y = y + (float)(i * item_h_px);
            const bool        dis   = is_disabled(i);
            const bool        sel   = (i == selected);
            const SDL_Color&  c =
                dis ? color_disabled : (sel ? color_selected : color_normal);
            if (sel && !dis) {
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(r, 50, 45, 70, 200);
                SDL_FRect hi{x - 4.0f, row_y - 2.0f, 280.0f, (float)item_h_px - 4};
                SDL_RenderFillRect(r, &hi);
            }
            draw_text(r, x, row_y + 4.0f, items[static_cast<size_t>(i)].c_str(), text_scale, c.r,
                      c.g, c.b, c.a);
        }
    }
};

struct ProgressBar {
    SDL_FRect rect{};
    float     value      = 1.0f;
    SDL_Color color_bg{40, 30, 30, 200};
    SDL_Color color_fill{200, 60, 60, 255};
    SDL_Color color_border{120, 100, 80, 180};

    float normalized_fill() const { return std::clamp(value, 0.0f, 1.0f); }

    void render(SDL_Renderer* r) const {
        const float v = normalized_fill();
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, color_bg.r, color_bg.g, color_bg.b, color_bg.a);
        SDL_RenderFillRect(r, &rect);
        SDL_FRect fill = rect;
        fill.w *= v;
        if (fill.w > 0.0f) {
            SDL_SetRenderDrawColor(r, color_fill.r, color_fill.g, color_fill.b, color_fill.a);
            SDL_RenderFillRect(r, &fill);
        }
        SDL_SetRenderDrawColor(r, color_border.r, color_border.g, color_border.b, color_border.a);
        SDL_RenderRect(r, &rect);
    }
};

} // namespace mion::ui
