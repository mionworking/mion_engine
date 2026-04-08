#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "bitmap_font.hpp"

namespace mion::ui {

// ---------------------------------------------------------------------------
// Theme — único ponto de controle de estilo. Editar g_theme muda o visual
// de todas as telas. Campos com defaults preservam o visual original.
// ---------------------------------------------------------------------------
struct Theme {
    // Overlay background
    SDL_Color overlay_dim        = {0,   0,   0,   200};

    // Panel
    SDL_Color panel_bg           = {10,  8,   20,  220};
    SDL_Color panel_border       = {180, 160, 100, 255};
    int       panel_border_px    = 2;

    // Texto
    SDL_Color text_normal        = {230, 230, 210, 255};
    SDL_Color text_selected      = {255, 220, 60,  255};
    SDL_Color text_disabled      = {100, 95,  85,  255};
    SDL_Color text_title         = {255, 220, 80,  255};
    SDL_Color text_hint          = {200, 200, 180, 255};
    SDL_Color text_subtitle      = {190, 190, 175, 255};
    SDL_Color text_desc          = {160, 165, 145, 200};

    // Highlight de seleção em listas/painéis
    SDL_Color list_hl_bg         = {50,  45,  70,  200};
    SDL_Color list_hl_border     = {255, 210, 80,  255};

    // Barras de progresso
    SDL_Color bar_bg             = {40,  30,  30,  200};
    SDL_Color bar_hp             = {200, 60,  60,  255};
    SDL_Color bar_mana           = {60,  100, 200, 255};
    SDL_Color bar_stamina        = {60,  180, 80,  255};
    SDL_Color bar_border         = {120, 100, 80,  180};
};

inline constexpr Theme kDefaultTheme{};
inline const Theme g_theme = kDefaultTheme;

// ---------------------------------------------------------------------------
// Utilitário: dim fullscreen (usado por todas as telas modais)
// ---------------------------------------------------------------------------
inline void draw_dim(SDL_Renderer* r, int vw, int vh,
                     SDL_Color c = g_theme.overlay_dim) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_FRect full{0, 0, (float)vw, (float)vh};
    SDL_RenderFillRect(r, &full);
}

// ---------------------------------------------------------------------------

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
    SDL_Color bg        = g_theme.panel_bg;
    SDL_Color border    = g_theme.panel_border;
    int       border_px = g_theme.panel_border_px;

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
    SDL_Color   color = g_theme.text_normal;

    void render(SDL_Renderer* r) const {
        draw_text(r, x, y, text.c_str(), scale, color.r, color.g, color.b, color.a);
    }
};

struct List {
    std::vector<std::string> items;
    int                      selected       = 0;
    int                      item_h_px      = 28;
    int                      text_scale     = 2;
    SDL_Color                color_normal   = g_theme.text_normal;
    SDL_Color                color_selected = g_theme.text_selected;
    SDL_Color                color_disabled = g_theme.text_disabled;
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
            const float      row_y = y + (float)(i * item_h_px);
            const bool       dis   = is_disabled(i);
            const bool       sel   = (i == selected);
            const SDL_Color& c     = dis ? color_disabled : (sel ? color_selected : color_normal);
            if (sel && !dis) {
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(r, g_theme.list_hl_bg.r, g_theme.list_hl_bg.g,
                                          g_theme.list_hl_bg.b, g_theme.list_hl_bg.a);
                SDL_FRect hi{x - 4.0f, row_y - 2.0f, 280.0f, (float)item_h_px - 4};
                SDL_RenderFillRect(r, &hi);
            }
            draw_text(r, x, row_y + 4.0f, items[static_cast<size_t>(i)].c_str(),
                      text_scale, c.r, c.g, c.b, c.a);
        }
    }
};

struct ProgressBar {
    SDL_FRect rect{};
    float     value        = 1.0f;
    SDL_Color color_bg     = g_theme.bar_bg;
    SDL_Color color_fill   = g_theme.bar_hp;
    SDL_Color color_border = g_theme.bar_border;

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
