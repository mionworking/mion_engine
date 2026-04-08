#pragma once
#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/locale.hpp"
#include "../core/ui.hpp"
#include "../components/attributes.hpp"

namespace mion {

inline void render_attribute_screen(SDL_Renderer* r, int vw, int vh,
                                     const AttributesState& attrs,
                                     int pending_level_ups,
                                     int selected,
                                     const LocaleSystem* locale = nullptr)
{
    const auto tr = [locale](const std::string& key) -> const char* {
        return locale ? locale->get(key) : key.c_str();
    };
    ui::draw_dim(r, vw, vh, {0, 0, 0, 210});

    const char* title = tr("attr_title");
    draw_text(r, vw * 0.5f - text_width(title, 2) * 0.5f, 18.0f,
              title, 2,
              ui::g_theme.text_title.r, ui::g_theme.text_title.g,
              ui::g_theme.text_title.b, ui::g_theme.text_title.a);

    char pts_buf[48];
    SDL_snprintf(pts_buf, sizeof(pts_buf), tr("attr_points_remaining"), pending_level_ups);
    draw_text(r, vw * 0.5f - text_width(pts_buf, 2) * 0.5f, 42.0f,
              pts_buf, 2,
              ui::g_theme.text_hint.r, ui::g_theme.text_hint.g,
              ui::g_theme.text_hint.b, ui::g_theme.text_hint.a);

    struct AttrRow { const char* name; const char* desc; int current; };
    const AttrRow rows[5] = {
        { tr("attr_vigor_name"),        tr("attr_vigor_desc"),       attrs.vigor        },
        { tr("attr_forca_name"),        tr("attr_forca_desc"),       attrs.forca        },
        { tr("attr_destreza_name"),     tr("attr_destreza_desc"),    attrs.destreza     },
        { tr("attr_inteligencia_name"), tr("attr_inteligencia_desc"),attrs.inteligencia },
        { tr("attr_endurance_name"),    tr("attr_endurance_desc"),   attrs.endurance    },
    };

    const float panel_w = vw * 0.55f;
    const float panel_h = vh * 0.65f;
    const float panel_x = (vw - panel_w) * 0.5f;
    const float panel_y = 72.0f;

    ui::Panel panel;
    panel.rect   = {panel_x, panel_y, panel_w, panel_h};
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

        const SDL_Color& tc = sel ? ui::g_theme.text_selected : ui::g_theme.text_normal;
        draw_text(r, panel_x + 18.0f, ry + 10.0f,
                  rows[i].name, 2, tc.r, tc.g, tc.b, tc.a);

        char lvbuf[16];
        SDL_snprintf(lvbuf, sizeof(lvbuf), "%d", rows[i].current);
        draw_text(r, panel_x + panel_w - text_width(lvbuf, 3) - 20.0f, ry + 8.0f,
                  lvbuf, 3, tc.r, tc.g, tc.b, tc.a);

        draw_text(r, panel_x + 18.0f, ry + 32.0f,
                  rows[i].desc, 1,
                  ui::g_theme.text_desc.r, ui::g_theme.text_desc.g,
                  ui::g_theme.text_desc.b, ui::g_theme.text_desc.a);
    }

    const char* hint = tr("attr_hint");
    draw_text(r, 16.0f, (float)vh - 28.0f, hint, 2,
              ui::g_theme.text_hint.r, ui::g_theme.text_hint.g,
              ui::g_theme.text_hint.b, ui::g_theme.text_hint.a);
}

} // namespace mion
