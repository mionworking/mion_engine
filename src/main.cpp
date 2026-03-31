#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
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
#include "core/save_system.hpp"
#include "core/audio.hpp"
#include "core/asset_manifest.hpp"
#include "core/debug_log.hpp"
#include "core/run_stats.hpp"
#include "core/scene_fader.hpp"

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
    // #region agent log
    mion::append_debug_log_line(
        "pre-fix",
        "H5_logging_pipeline_not_writing",
        "src/main.cpp:43",
        "Main entered",
        "{\"stage\":\"startup\"}"
    );
    // #endregion
    srand(static_cast<unsigned>(SDL_GetTicks()));
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

    mion::RunStats          run_stats{};
    mion::DifficultyLevel   difficulty = mion::DifficultyLevel::Normal;
    {
        mion::SaveData boot{};
        if (mion::SaveSystem::load_default(boot))
            difficulty = static_cast<mion::DifficultyLevel>(std::clamp(boot.difficulty, 0, 2));
    }

    mion::SceneCreateContext ctx;
    ctx.renderer    = renderer;
    ctx.audio       = audio.is_initialized() ? &audio : nullptr;
    ctx.viewport_w  = config.window_width;
    ctx.viewport_h  = config.window_height;
    ctx.stress_enemy_count = read_env_int("MION_STRESS_ENEMIES", 0);
    ctx.show_autosave_indicator = cfg.show_autosave_indicator;
    ctx.run_stats   = &run_stats;
    ctx.difficulty  = &difficulty;
    if (ctx.stress_enemy_count > 3)
        SDL_Log("Stress mode ativo: %d inimigos", ctx.stress_enemy_count);
    else if (ctx.stress_enemy_count > 0)
        SDL_Log("Spawn count via env: %d", ctx.stress_enemy_count);

    mion::SceneRegistry registry;
    mion::register_default_scenes(registry);

    mion::SceneManager scene_mgr;
    scene_mgr.set(registry.create("title", ctx));
    if (!scene_mgr.current) {
        SDL_Log("Falha ao criar cena inicial \"title\"");
        audio.shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    mion::IniData             ini_full = mion::ini_load(mion::config_file_path());
    mion::KeybindConfig       keybinds = mion::load_keybinds(ini_full);
    const std::string         lang = ini_full.get_string("ui", "language", "en");
    mion::g_locale.load(mion::resolve_data_path("locale_" + lang + ".ini"));
    mion::KeyboardInputSource keyboard(keybinds);
    mion::GamepadInputSource  gamepad;
    gamepad.try_connect();
    mion::SceneFader          scene_fader;
    std::string               pending_scene_id;

    const float fixed_dt    = config.fixed_delta_seconds;
    float       accumulator = 0.0f;
    Uint64      last_tick   = SDL_GetTicks();
    bool        running     = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;
            if (event.type == SDL_EVENT_GAMEPAD_ADDED)
                gamepad.try_connect();
            if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
                gamepad.try_connect();
        }

        Uint64 now     = SDL_GetTicks();
        float  dt_secs = std::min((now - last_tick) / 1000.0f,
                                  config.max_frame_time_seconds);
        last_tick = now;
        accumulator += dt_secs;

        while (accumulator >= fixed_dt) {
            accumulator -= fixed_dt;
            scene_fader.tick(fixed_dt);

            mion::InputState input = gamepad.is_connected()
                ? gamepad.read_state()
                : keyboard.read_state();
            const char* requested_next = scene_mgr.fixed_update(fixed_dt, input);
            std::string next_id = (requested_next && requested_next[0])
                ? requested_next
                : "";

            if (!next_id.empty()) {
                if (next_id == "__quit__") {
                    running = false;
                    scene_mgr.clear_pending_transition();
                } else if (scene_fader.is_clear() && pending_scene_id.empty()) {
                    scene_fader.start_fade_out();
                    pending_scene_id = next_id;
                    scene_mgr.clear_pending_transition();
                }
            }

            if (scene_fader.is_black_hold() && !pending_scene_id.empty()) {
                auto next_scene = registry.create(pending_scene_id, ctx);
                if (next_scene)
                    scene_mgr.set(std::move(next_scene));
                else
                    SDL_Log("Cena desconhecida: \"%s\"", pending_scene_id.c_str());
                pending_scene_id.clear();
                scene_fader.start_fade_in();
            }
        }

        scene_mgr.render(renderer);
        scene_fader.render(renderer, config.window_width, config.window_height);
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
