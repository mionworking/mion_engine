// sprite_bench_main.cpp
// Benchmark de renderização: varia número de atores E frames por animação.
// Cria texturas sintéticas em memória (sem precisar de PNGs reais).
//
// Uso:
//   sprite_bench [largura] [altura] [max_atores] [passo_atores]
//               [frames_por_anim] [render_frames]
//
// Padrões: 1280 720 500 50 4 300
//
// Para comparar impacto de frames por animação, rode 3x:
//   sprite_bench 1280 720 500 50  4 300   <- atual (4 frames)
//   sprite_bench 1280 720 500 50  8 300   <- fluido (8 frames)
//   sprite_bench 1280 720 500 50 16 300   <- ultra (16 frames)

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

namespace {

// ---------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------
int parse_int(const char* text, int fallback, int lo, int hi) {
    if (!text || !*text) return fallback;
    char* end = nullptr;
    long v = std::strtol(text, &end, 10);
    if (end == text) return fallback;
    return (int)std::max((long)lo, std::min((long)hi, v));
}

int arg_or_env(int argc, char* argv[], int idx,
               const char* env, int fallback, int lo, int hi) {
    if (argc > idx) return parse_int(argv[idx], fallback, lo, hi);
    return parse_int(std::getenv(env), fallback, lo, hi);
}

// ---------------------------------------------------------------
// Gera textura sintética: frame_w × frames_per_anim  ×  frame_h × 5 linhas
// Cada linha recebe uma cor RGBA distinta para simular animações reais.
// ---------------------------------------------------------------
static const SDL_Color ROW_COLORS[5] = {
    {  80, 160, 220, 200 }, // Idle   — azul
    {  80, 200, 100, 200 }, // Walk   — verde
    { 220,  80,  80, 200 }, // Attack — vermelho
    { 220, 180,  50, 200 }, // Hurt   — amarelo
    { 120,  50, 200, 200 }, // Death  — roxo
};

SDL_Texture* make_synthetic_sheet(SDL_Renderer* r,
                                   int frame_w, int frame_h,
                                   int frames_per_anim)
{
    const int tex_w = frame_w * frames_per_anim;
    const int tex_h = frame_h * 5;

    // Renderiza na textura alvo
    SDL_Texture* tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         tex_w, tex_h);
    if (!tex) return nullptr;

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(r, tex);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);

    for (int row = 0; row < 5; ++row) {
        const SDL_Color& c = ROW_COLORS[row];
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);

        for (int col = 0; col < frames_per_anim; ++col) {
            // Deixa 2px de margem para simular pixel art real
            SDL_FRect rect = {
                (float)(col * frame_w + 2),
                (float)(row * frame_h + 2),
                (float)(frame_w - 4),
                (float)(frame_h - 4)
            };
            SDL_RenderFillRect(r, &rect);
        }
    }

    SDL_SetRenderTarget(r, nullptr);
    return tex;
}

// ---------------------------------------------------------------
// Ator mínimo para o bench
// ---------------------------------------------------------------
struct BenchActor {
    float x, y;           // posição no viewport
    int   anim_row;       // linha da animação (0-4)
    int   frame_col;      // coluna atual
    float frame_timer;
};

// ---------------------------------------------------------------
// Roda uma etapa do bench: renderiza `render_frames` frames com `actor_count` atores
// Retorna: avg_ms, max_ms
// ---------------------------------------------------------------
struct StepResult { double avg_ms; double max_ms; };

StepResult run_step(SDL_Renderer* r, SDL_Texture* sheet,
                    std::vector<BenchActor>& actors, int actor_count,
                    int frames_per_anim, int frame_w, int frame_h,
                    int win_w, int win_h, int render_frames)
{
    // Posiciona atores em grade simples
    actors.resize((size_t)actor_count);
    for (int i = 0; i < actor_count; ++i) {
        actors[i].x           = (float)((i * 97) % win_w);
        actors[i].y           = (float)((i * 61) % win_h);
        actors[i].anim_row    = i % 5;
        actors[i].frame_col   = i % frames_per_anim;
        actors[i].frame_timer = (float)(i % 8) * 0.016f;
    }

    const float dt = 1.0f / 60.0f;

    double total_ms = 0.0;
    double max_ms   = 0.0;

    for (int f = 0; f < render_frames; ++f) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_EVENT_QUIT) goto bail;

        auto t0 = std::chrono::steady_clock::now();

        SDL_SetRenderDrawColor(r, 20, 20, 30, 255);
        SDL_RenderClear(r);

        // Avança animação e desenha cada ator
        for (auto& a : actors) {
            a.frame_timer -= dt;
            if (a.frame_timer <= 0.0f) {
                a.frame_col   = (a.frame_col + 1) % frames_per_anim;
                a.frame_timer = 0.1f;
            }

            SDL_FRect src = {
                (float)(a.frame_col * frame_w),
                (float)(a.anim_row  * frame_h),
                (float)frame_w,
                (float)frame_h
            };
            SDL_FRect dst = {
                a.x - frame_w,
                a.y - frame_h,
                (float)(frame_w * 2),
                (float)(frame_h * 2)
            };
            SDL_RenderTexture(r, sheet, &src, &dst);
        }

        SDL_RenderPresent(r);

        auto t1  = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        total_ms += ms;
        if (ms > max_ms) max_ms = ms;
    }

bail:
    return { total_ms / render_frames, max_ms };
}

} // namespace

int main(int argc, char* argv[]) {
    const int win_w          = arg_or_env(argc, argv, 1, "SB_WIDTH",          1280,  320, 7680);
    const int win_h          = arg_or_env(argc, argv, 2, "SB_HEIGHT",          720,  240, 4320);
    const int max_actors     = arg_or_env(argc, argv, 3, "SB_MAX_ACTORS",      500,    1, 50000);
    const int step_actors    = arg_or_env(argc, argv, 4, "SB_STEP_ACTORS",      50,    1, 50000);
    const int frames_per_anim= arg_or_env(argc, argv, 5, "SB_FRAMES_PER_ANIM",   4,    1,   256);
    const int render_frames  = arg_or_env(argc, argv, 6, "SB_RENDER_FRAMES",   300,   10, 10000);

    const int frame_w = 32;
    const int frame_h = 32;
    const int tex_w   = frame_w * frames_per_anim;
    const int tex_h   = frame_h * 5;

    std::printf("=== mion sprite_bench ===\n");
    std::printf("resolucao=%dx%d  frames_por_anim=%d  textura=%dx%d  "
                "max_atores=%d  passo=%d  render_frames=%d\n\n",
                win_w, win_h, frames_per_anim, tex_w, tex_h,
                max_actors, step_actors, render_frames);
    std::printf("%-10s %-12s %-14s %-10s\n",
                "atores", "fps_medio", "frame_ms_avg", "frame_ms_max");
    std::printf("%-10s %-12s %-14s %-10s\n",
                "------", "---------", "------------", "------------");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return 1;
    }

    SDL_Window*   window = SDL_CreateWindow("mion sprite_bench",
                                            win_w, win_h,
                                            SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer) {
        SDL_Log("Falha ao criar janela/renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Texture* sheet = make_synthetic_sheet(renderer, frame_w, frame_h, frames_per_anim);
    if (!sheet) {
        SDL_Log("Falha ao criar textura sintetica: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<BenchActor> actors;
    actors.reserve((size_t)max_actors);

    for (int n = step_actors; n <= max_actors; n += step_actors) {
        auto res = run_step(renderer, sheet, actors, n,
                            frames_per_anim, frame_w, frame_h,
                            win_w, win_h, render_frames);

        double fps = res.avg_ms > 0.0 ? 1000.0 / res.avg_ms : 0.0;
        std::printf("%-10d %-12.1f %-14.3f %-10.3f\n",
                    n, fps, res.avg_ms, res.max_ms);
        std::fflush(stdout);
    }

    SDL_DestroyTexture(sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
