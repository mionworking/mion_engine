#pragma once
#include <cmath>

#include "keybind_config.hpp"

namespace mion {

struct InputState {
    float move_x         = 0.0f;  // -1.0 to +1.0
    float move_y         = 0.0f;  // -1.0 to +1.0
    bool  attack_pressed  = false;
    bool  confirm_pressed = false; // Enter — dialogue / UI (not attack)
    bool  ui_cancel_pressed = false; // Backspace — close shop / UI
    bool  pause_pressed       = false; // Escape — pause (scene interprets edge)
    bool  cancel_pressed      = false; // same key as pause; closes overlays
    bool  ui_up_pressed       = false; // arrow ↑ (menu)
    bool  ui_down_pressed     = false; // arrow ↓
    bool  ui_left_pressed     = false; // arrow ←
    bool  ui_right_pressed    = false; // arrow →
    bool  skill_tree_pressed  = false; // Tab — talent tree
    bool  inventory_pressed   = false; // I — equipment screen
    bool  potion_pressed      = false; // H — use potion
    bool  erase_save_pressed = false; // N — erase save on title screen
    bool  dash_pressed   = false;  // LShift
    bool  ranged_pressed = false;  // X
    bool  parry_pressed  = false;  // C
    bool  spell_1_pressed = false; // Q
    bool  spell_2_pressed = false; // E
    bool  spell_3_pressed = false; // R
    bool  spell_4_pressed = false; // F
    bool  upgrade_1      = false;  // level-up keys (1/2/3)
    bool  upgrade_2      = false;
    bool  upgrade_3      = false;
    bool  talent_1_pressed = false; // 4
    bool  talent_2_pressed = false; // 5
    bool  talent_3_pressed = false; // 6

    bool is_moving() const {
        return move_x != 0.0f || move_y != 0.0f;
    }

    // Normalized vector — ensures equal speed on diagonals.
    void normalized_movement(float& out_x, float& out_y) const {
        float len = std::sqrt(move_x * move_x + move_y * move_y);
        if (len > 0.0f) {
            out_x = move_x / len;
            out_y = move_y / len;
        } else {
            out_x = 0.0f;
            out_y = 0.0f;
        }
    }
};

// Per-frame rising-edge flags for modal UI (pause menu, skill tree, shop, etc.).
// Produced by `OverlayInputTracker::capture` using `InputState` + prior levels from
// `PauseMenu::flush_input` and the tracker's own previous keys.
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

// Interface — PlayerController reads from here, not directly from SDL.
// Allows swapping in ScriptedInputSource for tests without changing anything.
class IInputSource {
public:
    virtual ~IInputSource() = default;
    virtual InputState read_state() const = 0;
};

// Real keyboard input via SDL3.
class KeyboardInputSource : public IInputSource {
public:
    explicit KeyboardInputSource(const KeybindConfig& kb = {}) : _kb(kb) {}
    InputState read_state() const override;

private:
    KeybindConfig _kb;
};

// Gamepad input via SDL3 (SDL_Gamepad API).
class GamepadInputSource : public IInputSource {
public:
    GamepadInputSource() = default;
    ~GamepadInputSource();

    bool is_connected() const;
    void try_connect();
    InputState read_state() const override;

private:
    SDL_Gamepad* _gamepad = nullptr;
};

} // namespace mion
