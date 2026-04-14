#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <memory>
#include <random>
#include <string>
#include <utility>

#include "core/config_loader.hpp"
#include "core/ini_loader.hpp"
#include "core/keybind_config.hpp"
#include "core/engine_config.hpp"
#include "core/engine_paths.hpp"
#include "core/input.hpp"
#include "core/locale.hpp"
#include "core/scene.hpp"
#include "core/scene_context.hpp"
#include "core/scene_registry.hpp"
#include "core/register_scenes.hpp"
#include "core/scene_ids.hpp"
#include "core/audio.hpp"
#include "core/asset_manifest.hpp"
#include "core/run_stats.hpp"
#include "core/scene_fader.hpp"
#include "systems/world_save_controller.hpp"

namespace {

int read_env_int(const char* name, int fallback) {
    const char* value = std::getenv(name);
    if (!value || !*value) return fallback;

    char* end = nullptr;
    long parsed = std::strtol(value, &end, 10);
    if (end == value) return fallback;
    if (parsed < 0) parsed = 0;
    if (parsed > 100000) parsed = 100000;
    return (int)parsed;
}

} // namespace

int main(int /*argc*/, char* /*argv*/[]) {
    mion::EngineConfig config;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return 1;
    }

    // config.ini carregado após SDL_Init (SDL_GetBasePath requer SDL inicializado)
    mion::ConfigData cfg = mion::load_config();
    config.window_width  = cfg.window_width;
    config.window_height = cfg.window_height;

    SDL_Window*   window   = SDL_CreateWindow(config.window_title,
                                              config.window_width,
                                              config.window_height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer) {
        SDL_Log("Falha ao criar janela/renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    mion::log_missing_assets_optional();

    mion::AudioSystem audio;
    if (!cfg.mute) {
        audio.init();
        audio.set_master_volume(cfg.volume_master);
    }

    // RNG determinístico na stack — seed via random_device
    std::mt19937 rng{std::random_device{}()};
    audio.set_rng(&rng);

    mion::RunStats          run_stats{};
    mion::DifficultyLevel   difficulty = mion::DifficultyLevel::Normal;
    mion::WorldSaveController::load_saved_difficulty(difficulty);

    mion::IniData             ini_full = mion::ini_load(mion::config_file_path());
    mion::KeybindConfig       keybinds = mion::load_keybinds(ini_full);
    const std::string         lang = ini_full.get_string("ui", "language", "en");

    // LocaleSystem na stack; passado explicitamente via SceneCreateContext.
    mion::LocaleSystem locale_sys;
    locale_sys.load(mion::resolve_data_path("locale_" + lang + ".ini"));

    mion::SceneCreateContext ctx;
    ctx.renderer    = renderer;
    ctx.audio       = audio.is_initialized() ? &audio : nullptr;
    ctx.viewport_w  = config.window_width;
    ctx.viewport_h  = config.window_height;
    ctx.stress_enemy_count = read_env_int("MION_STRESS_ENEMIES", 0);
    ctx.show_autosave_indicator = cfg.show_autosave_indicator;
    ctx.run_stats   = &run_stats;
    ctx.difficulty  = &difficulty;
    ctx.locale      = &locale_sys;
    ctx.rng         = &rng;
    if (ctx.stress_enemy_count > 3)
        SDL_Log("Stress mode ativo: %d inimigos", ctx.stress_enemy_count);
    else if (ctx.stress_enemy_count > 0)
        SDL_Log("Spawn count via env: %d", ctx.stress_enemy_count);

    mion::SceneRegistry registry;
    mion::register_default_scenes(registry);

    mion::SceneManager scene_mgr;
    scene_mgr.set(registry.create(mion::SceneId::kTitle, ctx));
    if (!scene_mgr.current) {
        SDL_Log("Falha ao criar cena inicial \"%s\"", mion::to_string(mion::SceneId::kTitle));
        audio.shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    mion::KeyboardInputSource keyboard(keybinds);
    mion::GamepadInputSource  gamepad;
    gamepad.try_connect();
    mion::SceneFader          scene_fader;
    mion::SceneId             pending_scene_id = mion::SceneId::kNone;

    const float fixed_dt    = config.fixed_delta_seconds;
    float       accumulator = 0.0f;
    const int   max_substeps_per_frame = 5;
    const float max_accumulator = fixed_dt * static_cast<float>(max_substeps_per_frame);
    Uint64      last_tick   = SDL_GetTicks();
    bool        running     = true;

    while (running) {
        // --- Eventos ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;
            if (event.type == SDL_EVENT_GAMEPAD_ADDED)
                gamepad.try_connect();
            if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
                gamepad.try_connect();
        }

        // --- Input lido uma vez por frame, fora do loop de física ---
        mion::InputState input = gamepad.is_connected()
            ? gamepad.read_state()
            : keyboard.read_state();

        // --- Frame timing ---
        Uint64 now     = SDL_GetTicks();
        float  dt_secs = std::min((now - last_tick) / 1000.0f,
                                  config.max_frame_time_seconds);
        last_tick = now;
        accumulator += dt_secs;
        accumulator = std::min(accumulator, max_accumulator);

        // --- Ticks físicos (input estático por frame) ---
        int substeps = 0;
        while (accumulator >= fixed_dt && substeps < max_substeps_per_frame) {
            accumulator -= fixed_dt;
            ++substeps;
            scene_fader.tick(fixed_dt);

            mion::SceneId next_id = scene_mgr.fixed_update(fixed_dt, input);

            if (next_id != mion::SceneId::kNone) {
                if (next_id == mion::SceneId::kQuit) {
                    running = false;
                    scene_mgr.clear_pending_transition();
                } else if (scene_fader.is_clear() && pending_scene_id == mion::SceneId::kNone) {
                    scene_fader.start_fade_out();
                    pending_scene_id = next_id;
                    scene_mgr.clear_pending_transition();
                }
            }

            if (scene_fader.is_black_hold() && pending_scene_id != mion::SceneId::kNone) {
                auto next_scene = registry.create(pending_scene_id, ctx);
                if (next_scene)
                    scene_mgr.set(std::move(next_scene));
                else
                    SDL_Log("Cena desconhecida: \"%s\"", mion::to_string(pending_scene_id));
                pending_scene_id = mion::SceneId::kNone;
                scene_fader.start_fade_in();
            }
        }

        // --- Render com blend_factor para interpolação suave ---
        const float blend_factor = accumulator / fixed_dt;
        scene_mgr.render(renderer, blend_factor);
        scene_fader.render(renderer, config.window_width, config.window_height);
        // VSync do renderer controla o pacing — sem SDL_Delay manual
        SDL_RenderPresent(renderer);
    }

    scene_mgr.set(nullptr);
    audio.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
