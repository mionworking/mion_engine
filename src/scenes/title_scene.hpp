#pragma once
#include <string>

#include "../core/scene.hpp"
#include "../core/audio.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/config_loader.hpp"
#include "../core/engine_paths.hpp"
#include "../core/run_stats.hpp"
#include "../core/save_system.hpp"
#include "../core/ui.hpp"
#include "../core/locale.hpp"

namespace mion {

enum class TitleUiState { Main, DifficultySelect, Settings, Extras };

class TitleScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    void set_audio(AudioSystem* a) { _audio = a; }
    void set_difficulty_target(DifficultyLevel* p) { _difficulty_ptr = p; }

    void enter() override {
        _next.clear();
        _ui                  = TitleUiState::Main;
        _prev_attack         = false;
        _prev_erase          = false;
        _prev_pause          = false;
        _prev_confirm        = false;
        _prev_cancel         = false;
        _erase_msg_timer     = 0.f;
        _save_exists         = SaveSystem::exists_default();
        _cfg                 = load_config();
        _lang_is_ptbr        = (load_ui_language() == "ptbr");
        _main_list.items     = {
            L("menu_new_game"),
            L("menu_continue"),
            L("menu_settings"),
            L("menu_extras"),
            L("ui_quit"),
        };
        _main_list.selected  = 0;
        _main_list.item_h_px = 32;
        _main_list.text_scale = 2;
        _main_list.disabled   = {false, !_save_exists, false, false, false};

        _diff_list.items     = {L("diff_easy"), L("diff_normal"), L("diff_hard")};
        _diff_list.selected  = 1;
        _diff_list.item_h_px = 32;

        _refresh_settings_items();
        _settings_list.selected  = 0;
        _settings_list.item_h_px = 32;
        _settings_list.text_scale = 2;

        _extras_list.items     = {
            L("menu_credits"),
            L("menu_back"),
        };
        _extras_list.selected  = 0;
        _extras_list.item_h_px = 32;
        _extras_list.text_scale = 2;

        _victory_unlock      = false;
        SaveData sd{};
        if (SaveSystem::load_default(sd))
            _victory_unlock = sd.victory_reached;
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->set_music_state(MusicState::None);
        }
    }

    void exit() override {}

    void fixed_update(float dt, const InputState& input) override {
        if (input.erase_save_pressed && !_prev_erase) {
            SaveSystem::remove_default_saves();
            _save_exists = false;
            _erase_msg_timer = 1.0f;
            _victory_unlock  = false;
            if (_audio) _audio->play_sfx(SoundId::UiDelete, 0.55f);
        }
        _prev_erase = input.erase_save_pressed;
        if (_erase_msg_timer > 0.f) _erase_msg_timer -= dt;
        if (_erase_msg_timer < 0.f) _erase_msg_timer = 0.f;

        if (input.pause_pressed && !_prev_pause)
            _next = "__quit__";
        _prev_pause = input.pause_pressed;

        if (_ui == TitleUiState::Main) {
            if (input.ui_up_pressed && !_prev_ui_up)
                _main_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _main_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.attack_pressed && !_prev_attack)
                || (input.confirm_pressed && !_prev_confirm);
            if (pick) {
                switch (_main_list.selected) {
                case 0: // New Game
                    SaveSystem::remove_default_saves();
                    _save_exists    = false;
                    _victory_unlock = false;
                    _main_list.disabled = {false, true, false, false, false};
                    _ui = TitleUiState::DifficultySelect;
                    if (_audio) _audio->play_sfx(SoundId::UiDelete, 0.55f);
                    break;
                case 1: // Continue
                    if (_save_exists) _next = "town";
                    break;
                case 2: // Settings
                    _ui = TitleUiState::Settings;
                    break;
                case 3: // Extras
                    _ui = TitleUiState::Extras;
                    break;
                case 4: // Quit
                    _next = "__quit__";
                    break;
                default: break;
                }
            }
            _prev_attack = input.attack_pressed;
            _prev_confirm = input.confirm_pressed;
        } else if (_ui == TitleUiState::DifficultySelect) {
            if (input.ui_cancel_pressed && !_prev_cancel) {
                _ui = TitleUiState::Main;
                _prev_cancel = input.ui_cancel_pressed;
                _prev_confirm = input.confirm_pressed;
                _prev_attack  = input.attack_pressed;
                return;
            }
            _prev_cancel = input.ui_cancel_pressed;

            if (input.ui_up_pressed && !_prev_ui_up)
                _diff_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _diff_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.confirm_pressed && !_prev_confirm)
                || (input.attack_pressed && !_prev_attack);
            if (pick && _difficulty_ptr) {
                *_difficulty_ptr =
                    static_cast<DifficultyLevel>(_diff_list.selected);
                SaveData sd{};
                if (SaveSystem::load_default(sd)) {
                    sd.difficulty = _diff_list.selected;
                    SaveSystem::save_default(sd);
                }
                _next = "town";
            }
            _prev_confirm = input.confirm_pressed;
            _prev_attack  = input.attack_pressed;
        } else if (_ui == TitleUiState::Settings) {
            if (input.ui_cancel_pressed && !_prev_cancel) {
                _ui = TitleUiState::Main;
                _prev_cancel = input.ui_cancel_pressed;
                _prev_confirm = input.confirm_pressed;
                _prev_attack  = input.attack_pressed;
                return;
            }
            _prev_cancel = input.ui_cancel_pressed;

            if (input.ui_up_pressed && !_prev_ui_up)
                _settings_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _settings_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.confirm_pressed && !_prev_confirm)
                || (input.attack_pressed && !_prev_attack);
            if (pick) {
                if (_settings_list.selected == 0) {
                    _cfg.mute = !_cfg.mute;
                    _apply_audio_settings();
                    _persist_runtime_config();
                    _refresh_settings_items();
                } else if (_settings_list.selected == 1) {
                    if (!_cfg.mute) {
                        _cfg.volume_master += 0.1f;
                        if (_cfg.volume_master > 1.0f) _cfg.volume_master = 0.0f;
                        _apply_audio_settings();
                        _persist_runtime_config();
                        _refresh_settings_items();
                    }
                } else if (_settings_list.selected == 2) {
                    _lang_is_ptbr = !_lang_is_ptbr;
                    _reload_locale();
                    _persist_runtime_config();
                    _refresh_all_menu_text();
                } else if (_settings_list.selected == 4) {
                    _ui = TitleUiState::Main;
                }
            }
            _prev_confirm = input.confirm_pressed;
            _prev_attack  = input.attack_pressed;
        } else { // Extras
            if (input.ui_cancel_pressed && !_prev_cancel) {
                _ui = TitleUiState::Main;
                _prev_cancel = input.ui_cancel_pressed;
                _prev_confirm = input.confirm_pressed;
                _prev_attack  = input.attack_pressed;
                return;
            }
            _prev_cancel = input.ui_cancel_pressed;

            if (input.ui_up_pressed && !_prev_ui_up)
                _extras_list.nav_up();
            if (input.ui_down_pressed && !_prev_ui_down)
                _extras_list.nav_down();
            _prev_ui_up   = input.ui_up_pressed;
            _prev_ui_down = input.ui_down_pressed;

            const bool pick = (input.confirm_pressed && !_prev_confirm)
                || (input.attack_pressed && !_prev_attack);
            if (pick) {
                if (_extras_list.selected == 0)
                    _next = "credits";
                else
                    _ui = TitleUiState::Main;
            }
            _prev_confirm = input.confirm_pressed;
            _prev_attack  = input.attack_pressed;
        }
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        SDL_SetRenderDrawColor(r, 12, 10, 22, 255);
        SDL_RenderClear(r);

        const float cx = viewport_w * 0.5f;

        if (_ui == TitleUiState::Main) {
            const char* title = "MION ENGINE";
            draw_text(r, cx - text_width(title, 3) * 0.5f, viewport_h * 0.24f, title, 3, 220, 210, 180);
            float y = viewport_h * 0.40f;
            _main_list.render(r, cx - 90.0f, y);
        } else if (_ui == TitleUiState::DifficultySelect) {
            const char* hdr = L("menu_select_difficulty");
            draw_text(r, cx - text_width(hdr, 3) * 0.5f, viewport_h * 0.28f, hdr, 3, 220, 210, 180);
            const char* sub = L("menu_diff_hint");
            draw_text(r, cx - text_width(sub, 2) * 0.5f, viewport_h * 0.38f, sub, 2, 130, 140, 170);
            _diff_list.render(r, cx - 100.0f, viewport_h * 0.48f);
        } else if (_ui == TitleUiState::Settings) {
            const char* hdr = L("menu_settings");
            draw_text(r, cx - text_width(hdr, 3) * 0.5f, viewport_h * 0.28f, hdr, 3, 220, 210, 180);
            const char* sub = L("menu_settings_hint");
            draw_text(r, cx - text_width(sub, 2) * 0.5f, viewport_h * 0.38f, sub, 2, 130, 140, 170);
            _settings_list.render(r, cx - 130.0f, viewport_h * 0.48f);
        } else {
            const char* hdr = L("menu_extras");
            draw_text(r, cx - text_width(hdr, 3) * 0.5f, viewport_h * 0.28f, hdr, 3, 220, 210, 180);
            const char* sub = L("menu_extras_hint");
            draw_text(r, cx - text_width(sub, 2) * 0.5f, viewport_h * 0.38f, sub, 2, 130, 140, 170);
            _extras_list.render(r, cx - 100.0f, viewport_h * 0.48f);
        }
    }

    bool has_erase_feedback() const { return _erase_msg_timer > 0.f; }

    const char* next_scene() const override {
        return _next.empty() ? "" : _next.c_str();
    }

    void clear_next_scene_request() override { _next.clear(); }

private:
    void _reload_locale() {
        const std::string lang = _lang_is_ptbr ? "ptbr" : "en";
        if (detail::active_locale)
            detail::active_locale->load(resolve_data_path("locale_" + lang + ".ini"));
    }

    void _apply_audio_settings() {
        if (!_audio) return;
        const float v = _cfg.mute ? 0.0f : _cfg.volume_master;
        _audio->set_master_volume(v);
    }

    void _persist_runtime_config() {
        const std::string lang = _lang_is_ptbr ? "ptbr" : "en";
        save_runtime_config(_cfg, lang);
    }

    void _refresh_settings_items() {
        char vol_buf[64];
        SDL_snprintf(vol_buf, sizeof(vol_buf), "%s: %d%%",
                     L("menu_settings_volume"),
                     (int)(_cfg.volume_master * 100.0f + 0.5f));
        const char* mute_txt = _cfg.mute ? L("ui_on") : L("ui_off");
        const char* lang_txt = _lang_is_ptbr ? "PT-BR" : "EN";
        std::string mute_line = std::string(L("menu_settings_mute")) + ": " + mute_txt;
        std::string vol_line  = std::string(vol_buf);
        std::string lang_line = std::string(L("menu_settings_language")) + ": " + lang_txt;
        _settings_list.items = {
            std::move(mute_line),
            std::move(vol_line),
            std::move(lang_line),
            L("menu_settings_controls"),
            L("menu_back"),
        };
    }

    void _refresh_all_menu_text() {
        _main_list.items = {
            L("menu_new_game"),
            L("menu_continue"),
            L("menu_settings"),
            L("menu_extras"),
            L("ui_quit"),
        };
        _main_list.disabled = {false, !_save_exists, false, false, false};
        _diff_list.items = {L("diff_easy"), L("diff_normal"), L("diff_hard")};
        _extras_list.items = {L("menu_credits"), L("menu_back")};
        _refresh_settings_items();
    }

    AudioSystem*       _audio           = nullptr;
    DifficultyLevel*   _difficulty_ptr  = nullptr;
    std::string        _next;
    TitleUiState       _ui              = TitleUiState::Main;
    bool               _prev_attack     = false;
    bool               _prev_erase      = false;
    bool               _prev_pause     = false;
    bool               _prev_confirm    = false;
    bool               _prev_cancel     = false;
    bool               _prev_ui_up      = false;
    bool               _prev_ui_down    = false;
    bool               _save_exists     = false;
    bool               _victory_unlock  = false;
    bool               _lang_is_ptbr    = false;
    float              _erase_msg_timer = 0.f;
    ConfigData         _cfg{};
    ui::List           _diff_list;
    ui::List           _main_list;
    ui::List           _settings_list;
    ui::List           _extras_list;
};

} // namespace mion
