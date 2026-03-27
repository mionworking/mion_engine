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

    state.attack_pressed = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_Z];

    return state;
}

} // namespace mion
