#pragma once
#include <memory>
#include <SDL3/SDL.h>
#include "input.hpp"

namespace mion {

// Interface that every scene must implement.
class IScene {
public:
    virtual ~IScene() = default;

    virtual void enter()                                   = 0;
    virtual void exit()                                   = 0;
    virtual void fixed_update(float dt, const InputState& input) = 0;
    virtual void render(SDL_Renderer* renderer, float blend_factor) = 0;

    // Returns the name of the next scene, or "" if no transition is requested.
    virtual const char* next_scene() const { return ""; }
    virtual void clear_next_scene_request() {}
};

// Manages the active scene and handles scene transitions.
class SceneManager {
public:
    std::unique_ptr<IScene> current;

    void set(std::unique_ptr<IScene> scene) {
        if (current) current->exit();
        current = std::move(scene);
        if (current) current->enter();
    }

    // Calls fixed_update and returns the next scene name (or "" if none).
    const char* fixed_update(float dt, const InputState& input) {
        if (!current) return "";
        current->fixed_update(dt, input);
        return current->next_scene();
    }

    void clear_pending_transition() {
        if (current) current->clear_next_scene_request();
    }

    void render(SDL_Renderer* renderer, float blend_factor) {
        if (current) current->render(renderer, blend_factor);
    }
};

} // namespace mion
