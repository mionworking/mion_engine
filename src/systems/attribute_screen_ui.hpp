#pragma once
#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/ui.hpp"
#include "../components/attributes.hpp"

namespace mion {

inline void render_attribute_screen(SDL_Renderer* r, int vw, int vh,
                                     const AttributesState& attrs,
                                     int pending_level_ups,
                                     int selected)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 210);
    SDL_FRect full{0, 0, (float)vw, (float)vh};
    SDL_RenderFillRect(r, &full);

    const char* title = "LEVEL UP - Distribute Attribute Point";
    draw_text(r, vw * 0.5f - text_width(title, 2) * 0.5f, 18.0f,
              title, 2, 255, 220, 80, 255);

    char pts_buf[48];
    SDL_snprintf(pts_buf, sizeof(pts_buf), "Points remaining: %d", pending_level_ups);
    draw_text(r, vw * 0.5f - text_width(pts_buf, 2) * 0.5f, 42.0f,
              pts_buf, 2, 200, 200, 180, 255);

    struct AttrRow { const char* name; const char* desc; int current; };
    const AttrRow rows[5] = {
        { "Vigor",        "+HP max por ponto",       attrs.vigor        },
        { "Forca",        "+Dano melee por ponto",   attrs.forca        },
        { "Destreza",     "+Dano ranged por ponto",  attrs.destreza     },
        { "Inteligencia", "+Dano magic / +Mana max", attrs.inteligencia },
        { "Endurance",    "+Stamina max por ponto",  attrs.endurance    },
    };

    const float panel_w = vw * 0.55f;
    const float panel_h = vh * 0.65f;
    const float panel_x = (vw - panel_w) * 0.5f;
    const float panel_y = 72.0f;

    ui::Panel panel;
    panel.rect   = {panel_x, panel_y, panel_w, panel_h};
    panel.border = {180, 140, 60, 255};
    panel.render(r);

    const float row_h = panel_h / 5.0f;
    for (int i = 0; i < 5; ++i) {
        const float ry  = panel_y + (float)i * row_h;
        const bool  sel = (i == selected);

        if (sel) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 80, 65, 30, 200);
            SDL_FRect hi{panel_x + 4.0f, ry + 4.0f, panel_w - 8.0f, row_h - 8.0f};
            SDL_RenderFillRect(r, &hi);
            SDL_SetRenderDrawColor(r, 255, 210, 80, 255);
            SDL_RenderRect(r, &hi);
        }

        Uint8 nr = sel ? 255 : 200, ng = sel ? 220 : 200, nb = sel ? 80 : 180;
        draw_text(r, panel_x + 18.0f, ry + 10.0f,
                  rows[i].name, 2, nr, ng, nb, 255);

        char lvbuf[16];
        SDL_snprintf(lvbuf, sizeof(lvbuf), "%d", rows[i].current);
        draw_text(r, panel_x + panel_w - text_width(lvbuf, 3) - 20.0f, ry + 8.0f,
                  lvbuf, 3, nr, ng, nb, 255);

        draw_text(r, panel_x + 18.0f, ry + 32.0f,
                  rows[i].desc, 1, 160, 165, 145, 200);
    }

    const char* hint = "UP/DOWN selecionar   ENTER confirmar   ESC fechar";
    draw_text(r, 16.0f, (float)vh - 28.0f, hint, 2, 200, 200, 180, 255);
}

} // namespace mion
