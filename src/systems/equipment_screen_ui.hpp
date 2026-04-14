#pragma once
#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/locale.hpp"
#include "../core/ui.hpp"
#include "../components/equipment.hpp"
#include "../components/item_bag.hpp"
#include "../entities/actor.hpp"

namespace mion {

// ---------------------------------------------------------------------------
// Layout:
//   Left panel  — 11 slots de equip (lista navegável)
//   Right panel — bag grid 4×6
//   Bottom bar  — hint de controles
//   Context menu — popup quando confirm é pressionado
// ---------------------------------------------------------------------------

struct EquipScreenRenderState {
    // Qual painel está focado: false = slots, true = bag
    bool focus_bag      = false;
    int  slot_cursor    = 0; // 0..10
    int  bag_cursor     = 0; // 0..23
    // Context menu
    bool ctx_open       = false;
    int  ctx_selected   = 0;
    // Fade-in alpha (0.0 → 1.0)
    float open_alpha    = 1.0f;
};

inline void render_equipment_screen(SDL_Renderer* r, int vw, int vh,
                                    const Actor& player,
                                    const EquipScreenRenderState& state,
                                    const LocaleSystem* locale = nullptr)
{
    const auto tr = [locale](const std::string& key) -> const char* {
        return locale ? locale->get(key) : key.c_str();
    };
    const float alpha = state.open_alpha;

    // --- dim fullscreen ---
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, (Uint8)(200 * alpha));
    SDL_FRect full{0, 0, (float)vw, (float)vh};
    SDL_RenderFillRect(r, &full);

    // --- Layout dimensions ---
    const float pad       = 16.0f;
    const float slot_w    = (float)vw * 0.38f;
    const float bag_w     = (float)vw * 0.38f;
    const float panel_h   = (float)vh - pad * 2.0f - 32.0f; // leave room for hint
    const float slot_x    = pad;
    const float bag_x     = (float)vw - bag_w - pad;
    const float panel_y   = pad;

    // ==== LEFT PANEL: equipment slots ====
    {
        ui::Panel left;
        left.rect   = {slot_x, panel_y, slot_w, panel_h};
        left.border = !state.focus_bag ? ui::g_theme.list_hl_border : ui::g_theme.panel_border;
        left.render(r);

        const char* title = tr("equip_title");
        draw_text(r, slot_x + 10.0f, panel_y + 8.0f, title, 2,
                  ui::g_theme.text_title.r, ui::g_theme.text_title.g,
                  ui::g_theme.text_title.b, (Uint8)(ui::g_theme.text_title.a * alpha));

        const float row_h  = 26.0f;
        const float list_y = panel_y + 32.0f;

        for (int i = 0; i < kEquipSlotCount; ++i) {
            const float ry  = list_y + (float)i * row_h;
            const bool  sel = (!state.focus_bag && i == state.slot_cursor);
            const bool  has = player.player->equipment.is_equipped(static_cast<EquipSlot>(i));

            if (sel) {
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(r, ui::g_theme.list_hl_bg.r, ui::g_theme.list_hl_bg.g,
                                          ui::g_theme.list_hl_bg.b, ui::g_theme.list_hl_bg.a);
                SDL_FRect hi{slot_x + 4.0f, ry, slot_w - 8.0f, row_h - 2.0f};
                SDL_RenderFillRect(r, &hi);
            }

            const SDL_Color& label_c = sel ? ui::g_theme.text_selected : ui::g_theme.text_hint;
            draw_text(r, slot_x + 10.0f, ry + 5.0f,
                      equip_slot_name(static_cast<EquipSlot>(i)), 1,
                      label_c.r, label_c.g, label_c.b, (Uint8)(label_c.a * alpha));

            const char* item_str = has
                ? player.player->equipment.slots[i].name.c_str()
                : "--";
            const SDL_Color& item_c = has
                ? (sel ? ui::g_theme.text_selected : ui::g_theme.text_normal)
                : ui::g_theme.text_disabled;
            draw_text(r, slot_x + slot_w * 0.45f, ry + 5.0f, item_str, 1,
                      item_c.r, item_c.g, item_c.b, (Uint8)(item_c.a * alpha));
        }
    }

    // ==== RIGHT PANEL: bag grid ====
    {
        ui::Panel right;
        right.rect   = {bag_x, panel_y, bag_w, panel_h};
        right.border = state.focus_bag ? ui::g_theme.list_hl_border : ui::g_theme.panel_border;
        right.render(r);

        const char* title = tr("equip_inventory_title");
        draw_text(r, bag_x + 10.0f, panel_y + 8.0f, title, 2,
                  ui::g_theme.text_title.r, ui::g_theme.text_title.g,
                  ui::g_theme.text_title.b, (Uint8)(ui::g_theme.text_title.a * alpha));

        const float cell_w = (bag_w - 16.0f) / (float)kBagCols;
        const float cell_h = 28.0f;
        const float grid_y = panel_y + 32.0f;

        for (int i = 0; i < kBagSize; ++i) {
            const int   col   = i % kBagCols;
            const int   row   = i / kBagCols;
            const float cx    = bag_x + 8.0f + (float)col * cell_w;
            const float cy    = grid_y + (float)row * cell_h;
            const bool  sel   = (state.focus_bag && i == state.bag_cursor);
            const bool  empty = player.player->bag.is_empty(i);

            // cell background
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            if (sel) {
                SDL_SetRenderDrawColor(r, ui::g_theme.list_hl_bg.r, ui::g_theme.list_hl_bg.g,
                                          ui::g_theme.list_hl_bg.b, ui::g_theme.list_hl_bg.a);
            } else {
                SDL_SetRenderDrawColor(r, 20, 18, 35, 180);
            }
            SDL_FRect cell{cx + 1.0f, cy + 1.0f, cell_w - 2.0f, cell_h - 2.0f};
            SDL_RenderFillRect(r, &cell);

            // cell border
            SDL_Color bc = sel ? ui::g_theme.list_hl_border : SDL_Color{80, 75, 60, 200};
            SDL_SetRenderDrawColor(r, bc.r, bc.g, bc.b, bc.a);
            SDL_RenderRect(r, &cell);

            if (!empty) {
                const SDL_Color& tc = sel ? ui::g_theme.text_selected : ui::g_theme.text_normal;
                // Truncate long names to fit cell
                const std::string& name = player.player->bag.slots[i].name;
                char buf[16];
                SDL_snprintf(buf, sizeof(buf), "%.12s", name.c_str());
                draw_text(r, cx + 3.0f, cy + 8.0f, buf, 1,
                          tc.r, tc.g, tc.b, (Uint8)(tc.a * alpha));
            }
        }
    }

    // ==== CONTEXT MENU ====
    if (state.ctx_open) {
        // Options depend on focus: slots → Unequip/Cancel, bag → Equip/Cancel
        const char* opts[2] = {
            state.focus_bag ? tr("equip_ctx_equip") : tr("equip_ctx_unequip"),
            tr("equip_ctx_cancel")
        };
        const float cm_w = 160.0f;
        const float cm_h = 72.0f;
        const float cm_x = (float)vw * 0.5f - cm_w * 0.5f;
        const float cm_y = (float)vh * 0.5f - cm_h * 0.5f;

        ui::Panel ctx_panel;
        ctx_panel.rect   = {cm_x, cm_y, cm_w, cm_h};
        ctx_panel.border = ui::g_theme.list_hl_border;
        ctx_panel.render(r);

        for (int i = 0; i < 2; ++i) {
            const float oy  = cm_y + 10.0f + (float)i * 28.0f;
            const bool  sel = (i == state.ctx_selected);
            if (sel) {
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(r, ui::g_theme.list_hl_bg.r, ui::g_theme.list_hl_bg.g,
                                          ui::g_theme.list_hl_bg.b, ui::g_theme.list_hl_bg.a);
                SDL_FRect hi{cm_x + 4.0f, oy - 2.0f, cm_w - 8.0f, 24.0f};
                SDL_RenderFillRect(r, &hi);
            }
            const SDL_Color& tc = sel ? ui::g_theme.text_selected : ui::g_theme.text_normal;
            draw_text(r, cm_x + 12.0f, oy + 2.0f, opts[i], 2, tc.r, tc.g, tc.b, tc.a);
        }
    }

    // ==== HINT BAR ====
    const char* hint = state.focus_bag
        ? tr("equip_hint_focus_bag")
        : tr("equip_hint_focus_slots");
    draw_text(r, pad, (float)vh - 24.0f, hint, 1,
              ui::g_theme.text_hint.r, ui::g_theme.text_hint.g,
              ui::g_theme.text_hint.b, (Uint8)(ui::g_theme.text_hint.a * alpha));
}

} // namespace mion
