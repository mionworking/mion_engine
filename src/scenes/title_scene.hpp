#pragma once
#include <string>

#include "../core/scene.hpp"
#include "../core/audio.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/debug_log.hpp"
#include "../core/engine_paths.hpp"
#include "../core/run_stats.hpp"
#include "../core/locale.hpp"
#include "../core/scene_ids.hpp"
#include "../core/title_menu.hpp"
#include "../systems/world_save_controller.hpp"

namespace mion {

class TitleScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    void set_audio(AudioSystem* a) { _audio = a; }
    void set_difficulty_target(DifficultyLevel* p) { _difficulty_ptr = p; }
    void set_locale(LocaleSystem* l) { _locale = l; _menu.locale = l; }

    void enter() override {
        _next = SceneId::kNone;
        _prev_attack         = false;
        _prev_erase          = false;
        _prev_pause          = false;
        _prev_confirm        = false;
        _prev_cancel         = false;
        _erase_msg_timer     = 0.f;
        const bool save_exists  = WorldSaveController::has_default_save();
        const ConfigData cfg    = load_config();
        const bool lang_is_ptbr = (load_ui_language() == "ptbr");
        bool victory_unlock     = false;
        WorldSaveController::load_victory_unlock_flag(victory_unlock);
        _menu.initialize(save_exists, victory_unlock, cfg, lang_is_ptbr);
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->set_music_state(MusicState::None);
        }
    }

    void exit() override {}

    void fixed_update(float dt, const InputState& input) override {
        // Compute all edges from history, then sync history in one block.
        // No early return can leave _prev_* stale.
        const bool edge_erase   = input.erase_save_pressed && !_prev_erase;
        const bool edge_pause   = input.pause_pressed      && !_prev_pause;
        const bool edge_up      = input.ui_up_pressed      && !_prev_ui_up;
        const bool edge_down    = input.ui_down_pressed    && !_prev_ui_down;
        const bool edge_confirm = (input.confirm_pressed   && !_prev_confirm)
                               || (input.attack_pressed    && !_prev_attack);
        const bool edge_cancel  = input.ui_cancel_pressed  && !_prev_cancel;

        _prev_erase   = input.erase_save_pressed;
        _prev_pause   = input.pause_pressed;
        _prev_ui_up   = input.ui_up_pressed;
        _prev_ui_down = input.ui_down_pressed;
        _prev_confirm = input.confirm_pressed;
        _prev_attack  = input.attack_pressed;
        _prev_cancel  = input.ui_cancel_pressed;

        if (edge_erase)
            _apply_action_result(TitleMenuController::erase_save(_menu, _erase_msg_timer));
        if (_erase_msg_timer > 0.f) _erase_msg_timer -= dt;
        if (_erase_msg_timer < 0.f) _erase_msg_timer = 0.f;

        if (edge_pause)
            _next = SceneId::kQuit;

        if (_menu.ui == TitleUiState::Main) {
            if (edge_up)   _menu.main_list.nav_up();
            if (edge_down) _menu.main_list.nav_down();
            if (edge_confirm)
                _apply_action_result(TitleMenuController::activate_main_selection(_menu, _next));
        } else if (_menu.ui == TitleUiState::DifficultySelect) {
            if (edge_cancel) { TitleMenuController::cancel_submenu(_menu); return; }
            if (edge_up)   _menu.diff_list.nav_up();
            if (edge_down) _menu.diff_list.nav_down();
            if (edge_confirm)
                _apply_action_result(TitleMenuController::activate_difficulty_selection(
                    _menu, _difficulty_ptr, _next));
        } else if (_menu.ui == TitleUiState::Settings) {
            if (edge_cancel) { TitleMenuController::cancel_submenu(_menu); return; }
            if (edge_up)   _menu.settings_list.nav_up();
            if (edge_down) _menu.settings_list.nav_down();
            if (edge_confirm)
                _apply_action_result(TitleMenuController::activate_settings_selection(_menu));
        } else { // Extras
            if (edge_cancel) { TitleMenuController::cancel_submenu(_menu); return; }
            if (edge_up)   _menu.extras_list.nav_up();
            if (edge_down) _menu.extras_list.nav_down();
            if (edge_confirm)
                TitleMenuController::activate_extras_selection(_menu, _next);
        }
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        _menu.render(r, viewport_w, viewport_h);
    }

    bool has_erase_feedback() const { return _erase_msg_timer > 0.f; }

    SceneId next_scene() const override { return _next; }

    void clear_next_scene_request() override { _next = SceneId::kNone; }

private:
    void _apply_action_result(const TitleMenuActionResult& result) {
        if (result.clear_default_saves)
            _clear_default_saves();
        if (result.persist_difficulty_to_save)
            _persist_difficulty_to_save(result.persisted_difficulty);
        if (result.apply_audio_settings)
            _apply_audio_settings();
        if (result.reload_locale)
            _reload_locale();
        if (result.persist_runtime_cfg)
            _persist_runtime_config();
        if (result.refresh_menu_text)
            _menu.refresh_all_menu_text();
        if (result.play_delete_sfx && _audio)
            _audio->play_sfx(SoundId::UiDelete, 0.55f);
    }

    void _reload_locale() {
        const std::string lang = _menu.lang_is_ptbr ? "ptbr" : "en";
        if (_locale)
            _locale->load(resolve_data_path("locale_" + lang + ".ini"));
    }

    void _apply_audio_settings() {
        if (!_audio) return;
        const float v = _menu.cfg.mute ? 0.0f : _menu.cfg.volume_master;
        _audio->set_master_volume(v);
    }

    void _persist_runtime_config() {
        const std::string lang = _menu.lang_is_ptbr ? "ptbr" : "en";
        save_runtime_config(_menu.cfg, lang);
    }

    void _clear_default_saves() {
        if (!WorldSaveController::clear_default_saves())
            debug_log("TitleScene: failed to clear default saves");
    }

    void _persist_difficulty_to_save(int difficulty_index) {
        if (!WorldSaveController::persist_difficulty_if_save_exists(difficulty_index))
            debug_log("TitleScene: failed to persist difficulty (or no save present)");
    }

    AudioSystem*       _audio           = nullptr;
    DifficultyLevel*   _difficulty_ptr  = nullptr;
    LocaleSystem*      _locale          = nullptr;
    SceneId            _next = SceneId::kNone;
    TitleMenu          _menu;
    bool               _prev_attack     = false;
    bool               _prev_erase      = false;
    bool               _prev_pause     = false;
    bool               _prev_confirm    = false;
    bool               _prev_cancel     = false;
    bool               _prev_ui_up      = false;
    bool               _prev_ui_down    = false;
    float              _erase_msg_timer = 0.f;
};

} // namespace mion
