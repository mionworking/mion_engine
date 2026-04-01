#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../core/input.hpp"
#include "../core/pause_menu.hpp"

namespace mion {

struct PauseMenuResult {
    bool world_paused            = false;
    bool should_open_skill_tree  = false;
    bool should_quit_to_title    = false;
};

class PauseMenuController {
public:
    struct InitOptions {
        std::vector<std::string> items = {"RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"};
        std::vector<bool>        disabled_items;
        PauseMenu::Callback      on_opened = nullptr;
    };

    void init() {
        init(InitOptions{});
    }

    void init(const InitOptions& options) {
        _items                   = options.items;
        _disabled_items          = options.disabled_items;
        _on_opened               = options.on_opened;
        _request_open_skill_tree = false;
        _request_quit_to_title   = false;

        _menu.init(_items, _disabled_items);
        _menu.on_item_selected(0, [this]{
            _menu.paused = false;
        });
        _menu.on_item_selected(1, [this]{
            _request_open_skill_tree = true;
            _menu.paused = false;
        });
        _menu.on_item_selected(2, [this]{
            _menu.settings_open = true;
        });
        _menu.on_item_selected(3, [this]{
            _request_quit_to_title = true;
            _menu.paused = false;
        });
        if (_on_opened)
            _menu.on_opened(_on_opened);
    }

    PauseMenuResult update(const InputState& input) {
        _request_open_skill_tree = false;
        _request_quit_to_title   = false;

        PauseMenuResult out{};
        out.world_paused           = _menu.handle_input(input);
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

    bool paused() const { return _menu.paused; }
    bool settings_open() const { return _menu.settings_open; }

    PauseMenu& menu() { return _menu; }
    const PauseMenu& menu() const { return _menu; }

private:
    PauseMenu                _menu;
    std::vector<std::string> _items = {"RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"};
    std::vector<bool>        _disabled_items;
    PauseMenu::Callback      _on_opened = nullptr;
    bool                     _request_open_skill_tree = false;
    bool                     _request_quit_to_title   = false;
};

} // namespace mion
