#pragma once
#include <SDL3/SDL.h>
#include "input.hpp"

namespace mion {

// Interface que toda cena implementa
class IScene {
public:
    virtual ~IScene() = default;

    virtual void enter()                                   = 0;
    virtual void exit()                                    = 0;
    virtual void fixed_update(float dt, const InputState& input) = 0;
    virtual void render(SDL_Renderer* renderer)            = 0;

    // Retorna o nome da próxima cena, ou "" se não há transição
    virtual const char* next_scene() const { return ""; }
};

// Gerencia a cena ativa e troca entre cenas
class SceneManager {
public:
    IScene* current = nullptr;

    void set(IScene* scene) {
        if (current) current->exit();
        current = scene;
        if (current) current->enter();
    }

    // Chama fixed_update e checa se a cena pediu transição
    // Retorna o nome da próxima cena (ou "" se nenhuma)
    const char* fixed_update(float dt, const InputState& input) {
        if (!current) return "";
        current->fixed_update(dt, input);
        return current->next_scene();
    }

    void render(SDL_Renderer* renderer) {
        if (current) current->render(renderer);
    }
};

} // namespace mion
