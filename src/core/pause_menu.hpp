#pragma once
#include <functional>
#include <SDL3/SDL.h>

#include "ui.hpp"
#include "bitmap_font.hpp"
#include "input.hpp"
#include "locale.hpp"

namespace mion {

// Unified pause-menu state machine shared by DungeonScene and TownScene.
//
// Each scene provides per-item callbacks via on_item_selected(index, callback).
// The menu owns _paused / _settings_open state, edge detection for ESC / nav,
// rendering of the overlay and the settings placeholder.

struct PauseMenu {
    using Callback = std::function<void()>;

    void init(std::vector<std::string> items,
              std::vector<bool> disabled = {})
    {
        _list.items    = std::move(items);
        _list.selected = 0;
        if (disabled.empty())
            _list.disabled.assign(_list.items.size(), false);
        else
            _list.disabled = std::move(disabled);
        _item_callbacks.clear();
        _item_callbacks.resize(_list.items.size());
        _on_open = nullptr;
        _paused        = false;
        _settings_open = false;
        _prev_esc       = false;
        _prev_up        = false;
        _prev_down      = false;
        _prev_confirm   = false;
        _prev_cancel    = false;
    }

    void on_item_selected(int index, Callback cb) {
        if (index >= 0 && index < (int)_item_callbacks.size())
            _item_callbacks[index] = std::move(cb);
    }

    void on_opened(Callback cb) { _on_open = std::move(cb); }
    void set_locale(LocaleSystem* locale) { _locale = locale; }

    void flush_input(const InputState& in) {
        _prev_esc     = in.pause_pressed;
        _prev_up      = in.ui_up_pressed;
        _prev_down    = in.ui_down_pressed;
        _prev_confirm = in.confirm_pressed;
        _prev_cancel  = in.ui_cancel_pressed;
    }

    // Rising edges from `OverlayInputTracker::capture` (same contract as other modal UIs).
    // Caller must still `flush_input` once per tick after handling overlays.
    bool handle_overlay_edges(const OverlayInputEdges& e) {
        if (_settings_open) {
            if (e.pause || e.confirm || e.back)
                _settings_open = false;
            return true;
        }

        if (_paused) {
            if (e.pause)
                _paused = false;
            else {
                if (e.up)
                    _list.nav_up();
                if (e.down)
                    _list.nav_down();
                if (e.confirm) {
                    int sel = _list.selected;
                    if (sel >= 0 && sel < (int)_item_callbacks.size() && _item_callbacks[sel])
                        _item_callbacks[sel]();
                }
            }
            return true;
        }

        if (e.pause) {
            _paused        = true;
            _list.selected = 0;
            if (_on_open) _on_open();
            return true;
        }
        return false;
    }

    void open() {
        _paused        = true;
        _list.selected = 0;
    }

    void close() { _paused = false; }
    void open_settings() { _settings_open = true; }
    bool is_paused() const { return _paused; }
    bool is_settings_open() const { return _settings_open; }

    void render(SDL_Renderer* r, int vw, int vh) const {
        if (_paused)
            _render_overlay(r, vw, vh);
        if (_settings_open)
            _render_settings(r, vw, vh);
    }

    // Read-only access for external edge trackers.
    bool was_pause_pressed() const { return _prev_esc; }
    bool was_ui_up_pressed() const { return _prev_up; }
    bool was_ui_down_pressed() const { return _prev_down; }
    bool was_ui_cancel_pressed() const { return _prev_cancel; }

private:
    ui::List               _list;
    std::vector<Callback> _item_callbacks;
    Callback              _on_open;
    bool                  _prev_esc     = false;
    bool                  _prev_up      = false;
    bool                  _prev_down    = false;
    bool                  _prev_confirm = false;
    bool                  _prev_cancel  = false;
    bool                  _paused        = false;
    bool                  _settings_open = false;
    LocaleSystem*         _locale        = nullptr;

    void _render_overlay(SDL_Renderer* r, int vw, int vh) const {
        ui::draw_dim(r, vw, vh, {0, 0, 0, 160});

        ui::Panel panel;
        panel.rect = {vw * 0.35f, vh * 0.25f, vw * 0.30f, vh * 0.50f};
        panel.render(r);
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 16.0f,
                  tr("pause_title"), 3,
                  ui::g_theme.text_title.r, ui::g_theme.text_title.g,
                  ui::g_theme.text_title.b, ui::g_theme.text_title.a);
        _list.render(r, panel.rect.x + 20.0f, panel.rect.y + 56.0f);
    }

    void _render_settings(SDL_Renderer* r, int vw, int vh) const {
        ui::draw_dim(r, vw, vh);

        ui::Panel panel;
        panel.rect = {vw * 0.30f, vh * 0.30f, vw * 0.40f, vh * 0.40f};
        panel.render(r);
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 16.0f,
                  tr("menu_settings"), 3,
                  ui::g_theme.text_title.r, ui::g_theme.text_title.g,
                  ui::g_theme.text_title.b, ui::g_theme.text_title.a);
        const char* l1 = tr("pause_settings_line_1");
        const char* l2 = tr("pause_settings_line_2");
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 56.0f,
                  l1, 2,
                  ui::g_theme.text_subtitle.r, ui::g_theme.text_subtitle.g,
                  ui::g_theme.text_subtitle.b, ui::g_theme.text_subtitle.a);
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + 84.0f,
                  l2, 2,
                  ui::g_theme.text_subtitle.r, ui::g_theme.text_subtitle.g,
                  ui::g_theme.text_subtitle.b, ui::g_theme.text_subtitle.a);
        const char* hint = tr("pause_settings_back_hint");
        draw_text(r, panel.rect.x + 20.0f, panel.rect.y + panel.rect.h - 36.0f,
                  hint, 1,
                  ui::g_theme.text_hint.r, ui::g_theme.text_hint.g,
                  ui::g_theme.text_hint.b, ui::g_theme.text_hint.a);
    }

    const char* tr(const std::string& key) const {
        return _locale ? _locale->get(key) : key.c_str();
    }
};

} // namespace mion
