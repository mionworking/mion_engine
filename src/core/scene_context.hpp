#pragma once
#include <SDL3/SDL.h>

#include "run_stats.hpp"

namespace mion {

class AudioSystem;

// Dados mínimos para instanciar cenas via SceneRegistry (SDL pointers não-owned).
struct SceneCreateContext {
    SDL_Renderer* renderer = nullptr;
    AudioSystem*         audio    = nullptr;
    int                  viewport_w = 1280;
    int                  viewport_h = 720;

    // Eco de MION_STRESS_ENEMIES: 0 = normal; 1–3 = spawn count; >3 = stress mode
    int stress_enemy_count = 0;

    bool show_autosave_indicator = false;

    /// Instância em main.cpp; DungeonScene e outras cenas escrevem aqui.
    RunStats*          run_stats  = nullptr;
    /// Dificuldade da sessão (main); TitleScene actualiza ao confirmar; DungeonScene lê no spawn.
    DifficultyLevel*   difficulty = nullptr;
};

} // namespace mion
