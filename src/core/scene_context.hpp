#pragma once
#include <random>
#include <SDL3/SDL.h>

#include "run_stats.hpp"

namespace mion {

class AudioSystem;
struct LocaleSystem;

// Minimal data needed to instantiate scenes via SceneRegistry (SDL pointers are non-owning).
struct SceneCreateContext {
    SDL_Renderer* renderer = nullptr;
    AudioSystem*         audio    = nullptr;
    int                  viewport_w = 1280;
    int                  viewport_h = 720;

    // Mirror of MION_STRESS_ENEMIES: 0 = normal; 1–3 = spawn count; >3 = stress mode.
    int stress_enemy_count = 0;

    bool show_autosave_indicator = false;

    // Instance in main.cpp; written by DungeonScene and other scenes.
    RunStats*          run_stats  = nullptr;
    // Session difficulty (main); TitleScene updates on confirm; DungeonScene reads at spawn.
    DifficultyLevel*   difficulty = nullptr;

    // Localization system — owned by main()'s stack, non-owning here.
    LocaleSystem*      locale     = nullptr;
    // Deterministic RNG — owned by main()'s stack, non-owning here.
    std::mt19937*      rng        = nullptr;
};

} // namespace mion
