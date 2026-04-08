#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../core/input.hpp"
#include "../core/locale.hpp"
#include "../core/pause_menu.hpp"
#include "ui_controller_contract.hpp"

namespace mion {

struct PauseMenuResult : UiControllerResult {
    bool should_open_skill_tree = false;
    bool should_quit_to_title   = false;
};

class PauseMenuController {
public:
    struct InitOptions {
        std::vector<std::string> items = {"RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"};
        std::vector<bool>        disabled_items;
        PauseMenu::Callback      on_opened = nullptr;
    };

    void set_locale(LocaleSystem* locale) {
        _locale = locale;
        _menu.set_locale(locale);
    }

    void init() {
        InitOptions options{};
        if (_locale) {
            options.items = {
                _locale->get("pause_resume"),
                _locale->get("pause_skill_tree"),
                _locale->get("menu_settings"),
                _locale->get("pause_quit_to_menu")
            };
        }
        init(options);
    }

    void init(const InitOptions& options) {
        _items                   = options.items;
        _disabled_items          = options.disabled_items;
        _on_opened               = options.on_opened;
        _request_open_skill_tree = false;
        _request_quit_to_title   = false;

        _menu.init(_items, _disabled_items);
        _menu.set_locale(_locale);
        _menu.on_item_selected(0, [this]{
            _menu.close();
        });
        _menu.on_item_selected(1, [this]{
            _request_open_skill_tree = true;
            _menu.close();
        });
        _menu.on_item_selected(2, [this]{
            _menu.open_settings();
        });
        _menu.on_item_selected(3, [this]{
            _request_quit_to_title = true;
            _menu.close();
        });
        if (_on_opened)
            _menu.on_opened(_on_opened);
    }

    PauseMenuResult update(const OverlayInputEdges& edges) {
        _request_open_skill_tree = false;
        _request_quit_to_title   = false;

        PauseMenuResult out{};
        out.world_paused           = _menu.handle_overlay_edges(edges);
        out.should_open_skill_tree = _request_open_skill_tree;
        out.should_quit_to_title   = _request_quit_to_title;
        return out;
    }

    void flush_input(const InputState& input) {
        _menu.flush_input(input);
    }

    void render(SDL_Renderer* renderer, int viewport_w, int viewport_h) const {
        _menu.render(renderer, viewport_w, viewport_h);
    }

    bool paused() const { return _menu.is_paused(); }
    bool was_pause_pressed() const { return _menu.was_pause_pressed(); }
    bool was_ui_up_pressed() const { return _menu.was_ui_up_pressed(); }
    bool was_ui_down_pressed() const { return _menu.was_ui_down_pressed(); }
    bool was_ui_cancel_pressed() const { return _menu.was_ui_cancel_pressed(); }

private:
    LocaleSystem*             _locale = nullptr;
    PauseMenu                _menu;
    std::vector<std::string> _items = {"RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"};
    std::vector<bool>        _disabled_items;
    PauseMenu::Callback      _on_opened = nullptr;
    bool                     _request_open_skill_tree = false;
    bool                     _request_quit_to_title   = false;
};

} // namespace mion
