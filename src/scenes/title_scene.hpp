#pragma once
#include <string>

#include "../core/scene.hpp"
#include "../core/audio.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/engine_paths.hpp"
#include "../core/run_stats.hpp"
#include "../core/locale.hpp"
#include "../core/title_menu.hpp"

namespace mion {

class TitleScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    void set_audio(AudioSystem* a) { _audio = a; }
    void set_difficulty_target(DifficultyLevel* p) { _difficulty_ptr = p; }

    void enter() override {
        _next.clear();
        _prev_attack         = false;
        _prev_erase          = false;
        _prev_pause          = false;
        _prev_confirm        = false;
        _prev_cancel         = false;
        _erase_msg_timer     = 0.f;
        const bool save_exists  = SaveSystem::exists_default();
        const ConfigData cfg    = load_config();
        const bool lang_is_ptbr = (load_ui_language() == "ptbr");
        bool victory_unlock     = false;
        SaveData sd{};
        if (SaveSystem::load_default(sd))
            victory_unlock = sd.victory_reached;
        _menu.initialize(save_exists, victory_unlock, cfg, lang_is_ptbr);
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->set_music_state(MusicState::None);
        }
    }

    void exit() override {}

    void fixed_update(float dt, const InputState& input) override {
        if (input.erase_save_pressed && !_prev_erase) {
            _apply_action_result(TitleMenuController::erase_save(_menu, _erase_msg_timer));
        }
        _prev_erase = input.erase_save_pressed;
        if (_erase_msg_timer > 0.f) _erase_msg_timer -= dt;
        if (_erase_msg_timer < 0.f) _erase_msg_timer = 0.f;

        if (input.pause_pressed && !_prev_pause)
            _next = "__quit__";
        _prev_pause = input.pause_pressed;

        if (_menu.ui == TitleUiState::Main) {
            if (input.ui_up_pressed && !_prev_ui_up)
                _menu.main_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _menu.main_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.attack_pressed && !_prev_attack)
                || (input.confirm_pressed && !_prev_confirm);
            if (pick)
                _apply_action_result(TitleMenuController::activate_main_selection(_menu, _next));
            _prev_attack = input.attack_pressed;
            _prev_confirm = input.confirm_pressed;
        } else if (_menu.ui == TitleUiState::DifficultySelect) {
            if (input.ui_cancel_pressed && !_prev_cancel) {
                TitleMenuController::cancel_submenu(_menu);
                _prev_cancel = input.ui_cancel_pressed;
                _prev_confirm = input.confirm_pressed;
                _prev_attack  = input.attack_pressed;
                return;
            }
            _prev_cancel = input.ui_cancel_pressed;

            if (input.ui_up_pressed && !_prev_ui_up)
                _menu.diff_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _menu.diff_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.confirm_pressed && !_prev_confirm)
                || (input.attack_pressed && !_prev_attack);
            if (pick)
                TitleMenuController::activate_difficulty_selection(_menu, _difficulty_ptr, _next);
            _prev_confirm = input.confirm_pressed;
            _prev_attack  = input.attack_pressed;
        } else if (_menu.ui == TitleUiState::Settings) {
            if (input.ui_cancel_pressed && !_prev_cancel) {
                TitleMenuController::cancel_submenu(_menu);
                _prev_cancel = input.ui_cancel_pressed;
                _prev_confirm = input.confirm_pressed;
                _prev_attack  = input.attack_pressed;
                return;
            }
            _prev_cancel = input.ui_cancel_pressed;

            if (input.ui_up_pressed && !_prev_ui_up)
                _menu.settings_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _menu.settings_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.confirm_pressed && !_prev_confirm)
                || (input.attack_pressed && !_prev_attack);
            if (pick)
                _apply_action_result(TitleMenuController::activate_settings_selection(_menu));
            _prev_confirm = input.confirm_pressed;
            _prev_attack  = input.attack_pressed;
        } else { // Extras
            if (input.ui_cancel_pressed && !_prev_cancel) {
                TitleMenuController::cancel_submenu(_menu);
                _prev_cancel = input.ui_cancel_pressed;
                _prev_confirm = input.confirm_pressed;
                _prev_attack  = input.attack_pressed;
                return;
            }
            _prev_cancel = input.ui_cancel_pressed;

            if (input.ui_up_pressed && !_prev_ui_up)
                _menu.extras_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _menu.extras_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.confirm_pressed && !_prev_confirm)
                || (input.attack_pressed && !_prev_attack);
            if (pick)
                TitleMenuController::activate_extras_selection(_menu, _next);
            _prev_confirm = input.confirm_pressed;
            _prev_attack  = input.attack_pressed;
        }
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        _menu.render(r, viewport_w, viewport_h);
    }

    bool has_erase_feedback() const { return _erase_msg_timer > 0.f; }

    const char* next_scene() const override {
        return _next.empty() ? "" : _next.c_str();
    }

    void clear_next_scene_request() override { _next.clear(); }

private:
    void _apply_action_result(const TitleMenuActionResult& result) {
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
        if (detail::active_locale)
            detail::active_locale->load(resolve_data_path("locale_" + lang + ".ini"));
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

    AudioSystem*       _audio           = nullptr;
    DifficultyLevel*   _difficulty_ptr  = nullptr;
    std::string        _next;
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
