#include "input.hpp"
#include <SDL3/SDL.h>

namespace mion {

InputState KeyboardInputSource::read_state() const {
    InputState state;
    const bool* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  state.move_x -= 1.0f;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) state.move_x += 1.0f;
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    state.move_y -= 1.0f;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  state.move_y += 1.0f;

    state.ui_up_pressed    = keys[SDL_SCANCODE_UP];
    state.ui_down_pressed  = keys[SDL_SCANCODE_DOWN];
    state.ui_left_pressed  = keys[SDL_SCANCODE_LEFT];
    state.ui_right_pressed = keys[SDL_SCANCODE_RIGHT];
    state.skill_tree_pressed = keys[_kb.skill_tree];
    const bool esc = keys[_kb.pause];
    state.pause_pressed  = esc;
    state.cancel_pressed = esc;

    state.attack_pressed =
        keys[_kb.attack] || keys[_kb.attack_alt];
    state.confirm_pressed =
        keys[_kb.confirm] || keys[SDL_SCANCODE_KP_ENTER];
    // Backspace: fechar loja / diálogos; Escape → pausa (tratado nas cenas)
    state.ui_cancel_pressed = keys[SDL_SCANCODE_BACKSPACE];
    state.erase_save_pressed = keys[_kb.erase_save];
    state.dash_pressed    = keys[_kb.dash];
    state.ranged_pressed  = keys[_kb.ranged];
    state.parry_pressed   = keys[_kb.parry];
    state.spell_1_pressed = keys[_kb.spell_1];
    state.spell_2_pressed = keys[_kb.spell_2];
    state.spell_3_pressed = keys[_kb.spell_3];
    state.spell_4_pressed = keys[_kb.spell_4];
    state.upgrade_1         = keys[_kb.upgrade_1];
    state.upgrade_2         = keys[_kb.upgrade_2];
    state.upgrade_3         = keys[_kb.upgrade_3];
    state.talent_1_pressed = keys[_kb.talent_1];
    state.talent_2_pressed = keys[_kb.talent_2];
    state.talent_3_pressed = keys[_kb.talent_3];

    return state;
}

GamepadInputSource::~GamepadInputSource() {
    if (_gamepad) {
        SDL_CloseGamepad(_gamepad);
        _gamepad = nullptr;
    }
}

bool GamepadInputSource::is_connected() const {
    return _gamepad && SDL_GamepadConnected(_gamepad);
}

void GamepadInputSource::try_connect() {
    if (is_connected()) return;
    if (_gamepad) {
        SDL_CloseGamepad(_gamepad);
        _gamepad = nullptr;
    }

    int             count = 0;
    SDL_JoystickID* ids   = SDL_GetGamepads(&count);
    if (!ids || count <= 0) {
        if (ids) SDL_free(ids);
        return;
    }
    _gamepad = SDL_OpenGamepad(ids[0]);
    SDL_free(ids);
}

InputState GamepadInputSource::read_state() const {
    InputState state;
    if (!is_connected()) return state;

    const float deadzone = 8000.0f;
    const float axis_max = 32767.0f;

    const float lx = (float)SDL_GetGamepadAxis(_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    const float ly = (float)SDL_GetGamepadAxis(_gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    if (lx <= -deadzone) state.move_x = lx / axis_max;
    if (lx >= deadzone) state.move_x = lx / axis_max;
    if (ly <= -deadzone) state.move_y = ly / axis_max;
    if (ly >= deadzone) state.move_y = ly / axis_max;

    state.attack_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    state.ranged_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_WEST);
    state.dash_pressed   = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_EAST);
    state.parry_pressed  = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    state.spell_1_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    state.spell_2_pressed =
        SDL_GetGamepadAxis(_gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 16000;
    state.spell_3_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    state.spell_4_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);

    state.confirm_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    state.pause_pressed   = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_START);
    state.cancel_pressed  = state.pause_pressed;
    state.ui_up_pressed   = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    state.ui_down_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    state.ui_left_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    state.ui_right_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    state.skill_tree_pressed = SDL_GetGamepadButton(_gamepad, SDL_GAMEPAD_BUTTON_BACK);

    return state;
}

} // namespace mion
