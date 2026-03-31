#pragma once
#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/ui.hpp"
#include "../components/talent_tree.hpp"

namespace mion {

inline void render_skill_tree_overlay(SDL_Renderer* r, int vw, int vh,
                                       const TalentState& talents,
                                       int selected_col, int selected_row,
                                       const std::vector<int> col_indices[3])
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 200);
    SDL_FRect full{0, 0, (float)vw, (float)vh};
    SDL_RenderFillRect(r, &full);

    char pts[48];
    SDL_snprintf(pts, sizeof(pts), "Pontos: %d", talents.pending_points);
    draw_text(r, (float)vw - text_width(pts, 2) - 16.0f, 16.0f, pts, 2, 200, 220, 255, 255);

    const char* titles[3] = {"MELEE", "RANGED", "MAGIC"};
    const float gap    = 12.0f;
    const float colw   = (vw - gap * 4.0f) / 3.0f;
    const float top    = 56.0f;
    const float height = (float)vh - top - 24.0f;

    for (int c = 0; c < 3; ++c) {
        ui::Panel col_panel;
        col_panel.rect = {gap + (colw + gap) * (float)c, top, colw, height};
        if (c == selected_col) {
            col_panel.border    = {255, 210, 80, 255};
            col_panel.border_px = 3;
        }
        col_panel.render(r);
        draw_text(r, col_panel.rect.x + 10.0f, col_panel.rect.y + 8.0f, titles[c], 2,
                  220, 200, 160, 255);

        float ty = col_panel.rect.y + 36.0f;
        const auto& ids = col_indices[c];
        for (int ri = 0; ri < static_cast<int>(ids.size()); ++ri) {
            const int         ti   = ids[static_cast<size_t>(ri)];
            const TalentId    tid  = static_cast<TalentId>(ti);
            const TalentNode& node = talent_def(tid);
            const int         lv   = talents.level_of(tid);
            const bool        sel  = (c == selected_col && ri == selected_row);
            const bool        lock = node.has_parent
                && talents.level_of(node.parent) < node.parent_min_level;
            char line[96];
            SDL_snprintf(line, sizeof(line), "%s %d/%d",
                         talent_display_name(tid).c_str(), lv, node.max_level);
            Uint8 R = 200, G = 200, B = 190;
            if (lock && lv == 0) {
                R = 90; G = 88; B = 85;
            } else if (talents.can_spend(tid)) {
                R = 180; G = 240; B = 160;
            }
            if (sel) {
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(r, 70, 60, 40, 220);
                SDL_FRect hi{col_panel.rect.x + 6.0f, ty - 2.0f,
                              col_panel.rect.w - 12.0f, 22.0f};
                SDL_RenderFillRect(r, &hi);
            }
            draw_text(r, col_panel.rect.x + 12.0f, ty, line, 1, R, G, B, 255);
            ty += 24.0f;
        }
    }

    const char* hint = "SETAS  ENTER - gastar ponto  TAB/ESC - fechar";
    draw_text(r, 16.0f, (float)vh - 28.0f, hint, 2, 200, 200, 180, 255);
}

} // namespace mion
