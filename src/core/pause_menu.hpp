#pragma once
#include <functional>
#include <SDL3/SDL.h>

#include "ui.hpp"
#include "bitmap_font.hpp"
#include "input.hpp"

namespace mion {

// Unified pause-menu state machine shared by DungeonScene and TownScene.
//
// Each scene provides per-item callbacks via on_item_selected(index, callback).
// The menu owns _paused / _settings_open state, edge detection for ESC / nav,
// rendering of the overlay and the settings placeholder.

struct PauseMenu {
    bool      paused        = false;
    bool      settings_open = false;
    ui::List  list;

    using Callback = std::function<void()>;

    void init(std::vector<std::string> items,
              std::vector<bool> disabled = {})
    {
        list.items    = std::move(items);
        list.selected = 0;
        if (disabled.empty())
            list.disabled.assign(list.items.size(), false);
        else
            list.disabled = std::move(disabled);
        _item_callbacks.clear();
        _item_callbacks.resize(list.items.size());
        _on_open = nullptr;
        paused        = false;
        settings_open = false;
        prev_esc       = false;
        prev_up        = false;
        prev_down      = false;
        prev_confirm   = false;
        prev_cancel    = false;
    }

    void on_item_selected(int index, Callback cb) {
        if (index >= 0 && index < (int)_item_callbacks.size())
            _item_callbacks[index] = std::move(cb);
    }

    void on_opened(Callback cb) { _on_open = std::move(cb); }

    void flush_input(const InputState& in) {
        prev_esc     = in.pause_pressed;
        prev_up      = in.ui_up_pressed;
        prev_down    = in.ui_down_pressed;
        prev_confirm = in.confirm_pressed;
        prev_cancel  = in.ui_cancel_pressed;
    }

    // Returns true when the pause menu consumed input (scene should skip gameplay).
    bool handle_input(const InputState& in) {
        const bool esc_edge  = in.pause_pressed       && !prev_esc;
        const bool up_edge   = in.ui_up_pressed        && !prev_up;
        const bool down_edge = in.ui_down_pressed      && !prev_down;
        const bool conf_edge = in.confirm_pressed      && !prev_confirm;
        const bool back_edge = in.ui_cancel_pressed    && !prev_cancel;

        if (settings_open) {
            if (esc_edge || conf_edge || back_edge)
                settings_open = false;
            flush_input(in);
            prev_confirm = in.confirm_pressed;
            return true;
        }

        if (paused) {
            if (esc_edge)
                paused = false;
            else {
                if (up_edge)   list.nav_up();
                if (down_edge) list.nav_down();
                if (conf_edge) {
                    int sel = list.selected;
                    if (sel >= 0 && sel < (int)_item_callbacks.size() && _item_callbacks[sel])
                        _item_callbacks[sel]();
                }
            }
            return true;
        }

        if (esc_edge) {
            paused        = true;
            list.selected = 0;
            if (_on_open) _on_open();
            return true;
        }
        return false;
    }

    void open() {
        paused        = true;
        list.selected = 0;
    }

    void render(SDL_Renderer* r, int vw, int vh) const {
        if (paused)
            _render_overlay(r, vw, vh);
        if (settings_open)
            _render_settings(r, vw, vh);
    }

    // Previous-frame state exposed for scenes that compute edge detection
    // outside the menu (e.g. skill tree / attribute screen sharing the same
    // nav keys). Read-only in spirit; written only by flush_input().
    bool prev_esc     = false;
    bool prev_up      = false;
    bool prev_down    = false;
    bool prev_confirm = false;
    bool prev_cancel  = false;

private:
    std::vector<Callback> _item_callbacks;
    Callback              _on_open;

    void _render_overlay(SDL_Renderer* r, int vw, int vh) const {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
        SDL_FRect full{0, 0, (float)vw, (float)vh};
        SDL_RenderFillRect(r, &full);

        ui::Panel panel;
        panel.rect = {vw * 0.35f, vh * 0.25f, vw * 0.30f, vh * 0.50f};
        panel.render(r);
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 16.0f,
                  "PAUSED", 3, 255, 220, 60, 255);
        list.render(r, panel.rect.x + 20.0f, panel.rect.y + 56.0f);
    }

    void _render_settings(SDL_Renderer* r, int vw, int vh) const {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 200);
        SDL_FRect dim{0, 0, (float)vw, (float)vh};
        SDL_RenderFillRect(r, &dim);

        ui::Panel panel;
        panel.rect = {vw * 0.30f, vh * 0.30f, vw * 0.40f, vh * 0.40f};
        panel.render(r);
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 16.0f,
                  "SETTINGS", 3, 220, 200, 120, 255);
        const char* l1 = "Audio e teclas vivem em config.ini";
        const char* l2 = "por agora. Mais opcoes em breve.";
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 56.0f,
                  l1, 2, 190, 190, 175, 255);
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 84.0f,
                  l2, 2, 190, 190, 175, 255);
        const char* hint = "ESC / ENTER / BACKSPACE - voltar";
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + panel.rect.h - 36.0f,
                  hint, 1, 160, 200, 220, 255);
    }
};

} // namespace mion
