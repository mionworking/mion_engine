#pragma once
#include <memory>
#include <SDL3/SDL.h>
#include "input.hpp"
#include "scene_ids.hpp"

namespace mion {

// Interface que toda cena deve implementar.
class IScene {
public:
    virtual ~IScene() = default;

    virtual void enter()                                         = 0;
    virtual void exit()                                          = 0;
    virtual void fixed_update(float dt, const InputState& input) = 0;
    virtual void render(SDL_Renderer* renderer, float blend_factor) = 0;

    // Retorna kNone se nenhuma transição está pendente.
    virtual SceneId next_scene() const { return SceneId::kNone; }
    virtual void clear_next_scene_request() {}
};

// Gerencia a cena ativa e trata transições.
class SceneManager {
public:
    std::unique_ptr<IScene> current;

    void set(std::unique_ptr<IScene> scene) {
        if (current) current->exit();
        current = std::move(scene);
        if (current) current->enter();
    }

    // Executa fixed_update e retorna o próximo SceneId (kNone = sem transição).
    SceneId fixed_update(float dt, const InputState& input) {
        if (!current) return SceneId::kNone;
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
