#pragma once

#include "../core/input.hpp"
#include "pause_menu_controller.hpp"

namespace mion {

class OverlayInputTracker {
public:
    OverlayInputEdges capture(const InputState& input, const PauseMenuController& pause_menu) const {
        OverlayInputEdges edges;
        edges.pause   = input.pause_pressed      && !pause_menu.was_pause_pressed();
        edges.tab       = input.skill_tree_pressed  && !_prev_tab;
        edges.inventory = input.inventory_pressed    && !_prev_inventory;
        edges.up      = input.ui_up_pressed      && !pause_menu.was_ui_up_pressed();
        edges.down    = input.ui_down_pressed    && !pause_menu.was_ui_down_pressed();
        edges.left    = input.ui_left_pressed    && !_prev_ui_left;
        edges.right   = input.ui_right_pressed   && !_prev_ui_right;
        edges.confirm = input.confirm_pressed    && !_prev_confirm;
        edges.back    = input.ui_cancel_pressed  && !pause_menu.was_ui_cancel_pressed();
        return edges;
    }

    void flush(const InputState& input, PauseMenuController& pause_menu) {
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
