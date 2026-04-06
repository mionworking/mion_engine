#pragma once

#include "../core/input.hpp"
#include "../core/pause_menu.hpp"

namespace mion {

struct OverlayInputEdges {
    bool pause     = false;
    bool tab       = false;
    bool inventory = false;
    bool up        = false;
    bool down      = false;
    bool left      = false;
    bool right     = false;
    bool confirm   = false;
    bool back      = false;
};

class OverlayInputTracker {
public:
    OverlayInputEdges capture(const InputState& input, const PauseMenu& pause_menu) const {
        OverlayInputEdges edges;
        edges.pause   = input.pause_pressed      && !pause_menu.prev_esc;
        edges.tab       = input.skill_tree_pressed  && !_prev_tab;
        edges.inventory = input.inventory_pressed    && !_prev_inventory;
        edges.up      = input.ui_up_pressed      && !pause_menu.prev_up;
        edges.down    = input.ui_down_pressed    && !pause_menu.prev_down;
        edges.left    = input.ui_left_pressed    && !_prev_ui_left;
        edges.right   = input.ui_right_pressed   && !_prev_ui_right;
        edges.confirm = input.confirm_pressed    && !_prev_confirm;
        edges.back    = input.ui_cancel_pressed  && !pause_menu.prev_cancel;
        return edges;
    }

    void flush(const InputState& input, PauseMenu& pause_menu) {
        pause_menu.flush_input(input);
        _prev_ui_left    = input.ui_left_pressed;
        _prev_ui_right   = input.ui_right_pressed;
        _prev_tab        = input.skill_tree_pressed;
        _prev_confirm    = input.confirm_pressed;
        _prev_inventory  = input.inventory_pressed;
    }

private:
    bool _prev_ui_left   = false;
    bool _prev_ui_right  = false;
    bool _prev_tab       = false;
    bool _prev_confirm   = false;
    bool _prev_inventory = false;
};

} // namespace mion
