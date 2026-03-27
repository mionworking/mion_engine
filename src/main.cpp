#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>

#include "core/engine_config.hpp"
#include "core/input.hpp"
#include "core/scene.hpp"
#include "core/audio.hpp"
#include "scenes/dungeon_scene.hpp"

int main(int /*argc*/, char* /*argv*/[]) {
    mion::EngineConfig config;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return 1;
    }

    SDL_Window*   window   = SDL_CreateWindow(config.window_title,
                                              config.window_width,
                                              config.window_height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer) {
        SDL_Log("Falha ao criar janela/renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // --- Áudio ---
    mion::AudioSystem audio;
    audio.init();  // falha silenciosamente se não houver dispositivo

    // --- Cenas ---
    mion::DungeonScene dungeon;
    dungeon.viewport_w = config.window_width;
    dungeon.viewport_h = config.window_height;
    dungeon.set_renderer(renderer);
    dungeon.set_audio(audio.is_initialized() ? &audio : nullptr);

    mion::SceneManager scene_mgr;
    scene_mgr.set(&dungeon);

    // --- Input ---
    mion::KeyboardInputSource keyboard;

    // --- Loop ---
    const float fixed_dt    = config.fixed_delta_seconds;
    float       accumulator = 0.0f;
    Uint64      last_tick   = SDL_GetTicks();
    bool        running     = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_KEY_DOWN
                && event.key.scancode == SDL_SCANCODE_ESCAPE) running = false;
        }

        Uint64 now     = SDL_GetTicks();
        float  dt_secs = std::min((now - last_tick) / 1000.0f,
                                  config.max_frame_time_seconds);
        last_tick = now;
        accumulator += dt_secs;

        while (accumulator >= fixed_dt) {
            accumulator -= fixed_dt;
            mion::InputState input = keyboard.read_state();
            scene_mgr.fixed_update(fixed_dt, input);
        }

        scene_mgr.render(renderer);
        SDL_RenderPresent(renderer);

        float frame_ms  = (float)(SDL_GetTicks() - now);
        float target_ms = 1000.0f / config.target_fps;
        if (frame_ms < target_ms)
            SDL_Delay((Uint32)(target_ms - frame_ms));
    }

    scene_mgr.set(nullptr);
    audio.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
