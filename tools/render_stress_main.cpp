// render_stress_main.cpp
// Teste de performance de renderização com SDL3 real.
// Sobe gradativamente o número de inimigos e mede FPS médio por etapa.
//
// Uso:
//   render_stress [largura] [altura] [max_inimigos] [passo] [frames_por_etapa]
//
// Padrões:
//   1280 720 500 50 300
//
// Exemplo: 1920 1080 1000 100 300
//   Testa de 100 a 1000 inimigos (passo 100) a 1080p, 300 frames por etapa.

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>

#include "core/input.hpp"
#include "scenes/world_scene.hpp"

namespace {

int parse_int(const char* text, int fallback, int min_val, int max_val) {
    if (!text || !*text) return fallback;
    char* end = nullptr;
    long v = std::strtol(text, &end, 10);
    if (end == text) return fallback;
    v = std::max((long)min_val, std::min((long)max_val, v));
    return (int)v;
}

int arg_or_env(int argc, char* argv[], int idx,
               const char* env, int fallback, int mn, int mx) {
    if (argc > idx) return parse_int(argv[idx], fallback, mn, mx);
    return parse_int(std::getenv(env), fallback, mn, mx);
}

} // namespace

int main(int argc, char* argv[]) {
    const int win_w      = arg_or_env(argc, argv, 1, "RS_WIDTH",       1280, 320,  7680);
    const int win_h      = arg_or_env(argc, argv, 2, "RS_HEIGHT",       720, 240,  4320);
    const int max_n      = arg_or_env(argc, argv, 3, "RS_MAX",          500,   1, 50000);
    const int step       = arg_or_env(argc, argv, 4, "RS_STEP",          50,   1, 50000);
    const int frames     = arg_or_env(argc, argv, 5, "RS_FRAMES",       300,  10, 10000);
    const int frames_mult= arg_or_env(argc, argv, 6, "RS_FRAMES_MULT",    1,   1,    64);

    std::printf("=== mion render_stress ===\n");
    std::printf("resolucao=%dx%d  max_inimigos=%d  passo=%d  frames/etapa=%d  frames_mult=%dx\n\n",
                win_w, win_h, max_n, step, frames, frames_mult);
    std::printf("%-12s %-12s %-14s %-10s\n",
                "inimigos", "fps_medio", "frame_ms_avg", "frame_ms_max");
    std::printf("%-12s %-12s %-14s %-10s\n",
                "--------", "---------", "------------", "------------");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return 1;
    }

    SDL_Window*   window   = SDL_CreateWindow("mion render_stress",
                                              win_w, win_h,
                                              SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer) {
        SDL_Log("Falha ao criar janela/renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    mion::InputState idle_input{};

    for (int n = step; n <= max_n; n += step) {
        mion::WorldScene scene;
        scene.viewport_w = win_w;
        scene.viewport_h = win_h;
        scene.set_renderer(renderer);
        scene.enable_stress_mode(n);
        scene.enter();
        if (frames_mult > 1) scene.tile_sprite_sheets(frames_mult);

        double total_ms = 0.0;
        double max_ms   = 0.0;

        for (int f = 0; f < frames; ++f) {
            // Verifica eventos para não travar o compositor
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) goto done;
            }

            auto t0 = std::chrono::steady_clock::now();

            scene.fixed_update(1.0f / 60.0f, idle_input);
            scene.render(renderer, 0.0f);
            SDL_RenderPresent(renderer);

            auto t1  = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            total_ms += ms;
            if (ms > max_ms) max_ms = ms;
        }

        scene.exit();

        double avg_ms  = total_ms / frames;
        double avg_fps = avg_ms > 0.0 ? 1000.0 / avg_ms : 0.0;

        std::printf("%-12d %-12.1f %-14.3f %-10.3f\n",
                    n, avg_fps, avg_ms, max_ms);
        std::fflush(stdout);
    }

done:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
