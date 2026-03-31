#pragma once
#include <string>

#include <SDL3/SDL.h>

#include "bitmap_font.hpp"
#include "config_loader.hpp"
#include "locale.hpp"
#include "run_stats.hpp"
#include "save_system.hpp"
#include "ui.hpp"

namespace mion {

enum class TitleUiState { Main, DifficultySelect, Settings, Extras };

struct TitleMenuActionResult {
    bool play_delete_sfx      = false;
    bool apply_audio_settings = false;
    bool persist_runtime_cfg  = false;
    bool reload_locale        = false;
    bool refresh_menu_text    = false;
};

struct TitleMenu {
    TitleUiState ui            = TitleUiState::Main;
    bool         save_exists   = false;
    bool         victory_unlock = false;
    bool         lang_is_ptbr  = false;
    ConfigData   cfg{};
    ui::List     diff_list;
    ui::List     main_list;
    ui::List     settings_list;
    ui::List     extras_list;

    void initialize(bool save_exists_value, bool victory_unlock_value,
                    const ConfigData& cfg_value, bool lang_is_ptbr_value) {
        ui             = TitleUiState::Main;
        save_exists    = save_exists_value;
        victory_unlock = victory_unlock_value;
        lang_is_ptbr   = lang_is_ptbr_value;
        cfg            = cfg_value;

        main_list.items = {
            L("menu_new_game"),
            L("menu_continue"),
            L("menu_settings"),
            L("menu_extras"),
            L("ui_quit"),
        };
        main_list.selected   = 0;
        main_list.item_h_px  = 32;
        main_list.text_scale = 2;
        main_list.disabled   = {false, !save_exists, false, false, false};

        diff_list.items      = {L("diff_easy"), L("diff_normal"), L("diff_hard")};
        diff_list.selected   = 1;
        diff_list.item_h_px  = 32;
        diff_list.text_scale = 2;

        refresh_settings_items();
        settings_list.selected   = 0;
        settings_list.item_h_px  = 32;
        settings_list.text_scale = 2;

        extras_list.items = {
            L("menu_credits"),
            L("menu_back"),
        };
        extras_list.selected   = 0;
        extras_list.item_h_px  = 32;
        extras_list.text_scale = 2;
    }

    void refresh_settings_items() {
        char vol_buf[64];
        SDL_snprintf(vol_buf, sizeof(vol_buf), "%s: %d%%",
                     L("menu_settings_volume"),
                     static_cast<int>(cfg.volume_master * 100.0f + 0.5f));
        const char* mute_txt = cfg.mute ? L("ui_on") : L("ui_off");
        const char* lang_txt = lang_is_ptbr ? "PT-BR" : "EN";
        std::string mute_line = std::string(L("menu_settings_mute")) + ": " + mute_txt;
        std::string vol_line  = std::string(vol_buf);
        std::string lang_line = std::string(L("menu_settings_language")) + ": " + lang_txt;
        settings_list.items = {
            std::move(mute_line),
            std::move(vol_line),
            std::move(lang_line),
            L("menu_settings_controls"),
            L("menu_back"),
        };
    }

    void refresh_all_menu_text() {
        main_list.items = {
            L("menu_new_game"),
            L("menu_continue"),
            L("menu_settings"),
            L("menu_extras"),
            L("ui_quit"),
        };
        main_list.disabled = {false, !save_exists, false, false, false};
        diff_list.items = {L("diff_easy"), L("diff_normal"), L("diff_hard")};
        extras_list.items = {L("menu_credits"), L("menu_back")};
        refresh_settings_items();
    }

    void render(SDL_Renderer* r, int viewport_w, int viewport_h) const {
        SDL_SetRenderDrawColor(r, 12, 10, 22, 255);
        SDL_RenderClear(r);

        const float cx = viewport_w * 0.5f;
        if (ui == TitleUiState::Main) {
            const char* title = "MION ENGINE";
            draw_text(r, cx - text_width(title, 3) * 0.5f, viewport_h * 0.24f,
                      title, 3, 220, 210, 180);
            main_list.render(r, cx - 90.0f, viewport_h * 0.40f);
        } else if (ui == TitleUiState::DifficultySelect) {
            const char* hdr = L("menu_select_difficulty");
            draw_text(r, cx - text_width(hdr, 3) * 0.5f, viewport_h * 0.28f,
                      hdr, 3, 220, 210, 180);
            const char* sub = L("menu_diff_hint");
            draw_text(r, cx - text_width(sub, 2) * 0.5f, viewport_h * 0.38f,
                      sub, 2, 130, 140, 170);
            diff_list.render(r, cx - 100.0f, viewport_h * 0.48f);
        } else if (ui == TitleUiState::Settings) {
            const char* hdr = L("menu_settings");
            draw_text(r, cx - text_width(hdr, 3) * 0.5f, viewport_h * 0.28f,
                      hdr, 3, 220, 210, 180);
            const char* sub = L("menu_settings_hint");
            draw_text(r, cx - text_width(sub, 2) * 0.5f, viewport_h * 0.38f,
                      sub, 2, 130, 140, 170);
            settings_list.render(r, cx - 130.0f, viewport_h * 0.48f);
        } else {
            const char* hdr = L("menu_extras");
            draw_text(r, cx - text_width(hdr, 3) * 0.5f, viewport_h * 0.28f,
                      hdr, 3, 220, 210, 180);
            const char* sub = L("menu_extras_hint");
            draw_text(r, cx - text_width(sub, 2) * 0.5f, viewport_h * 0.38f,
                      sub, 2, 130, 140, 170);
            extras_list.render(r, cx - 100.0f, viewport_h * 0.48f);
        }
    }
};

struct TitleMenuController {
    static TitleMenuActionResult erase_save(TitleMenu& menu, float& erase_msg_timer) {
        SaveSystem::remove_default_saves();
        menu.save_exists     = false;
        menu.victory_unlock  = false;
        menu.main_list.disabled = {false, true, false, false, false};
        erase_msg_timer      = 1.0f;

        TitleMenuActionResult result;
        result.play_delete_sfx = true;
        return result;
    }

    static TitleMenuActionResult activate_main_selection(TitleMenu& menu, std::string& next) {
        TitleMenuActionResult result;
        switch (menu.main_list.selected) {
        case 0:
            SaveSystem::remove_default_saves();
            menu.save_exists        = false;
            menu.victory_unlock     = false;
            menu.main_list.disabled = {false, true, false, false, false};
            menu.ui                 = TitleUiState::DifficultySelect;
            result.play_delete_sfx  = true;
            break;
        case 1:
            if (menu.save_exists)
                next = "town";
            break;
        case 2:
            menu.ui = TitleUiState::Settings;
            break;
        case 3:
            menu.ui = TitleUiState::Extras;
            break;
        case 4:
            next = "__quit__";
            break;
        default:
            break;
        }
        return result;
    }

    static void cancel_submenu(TitleMenu& menu) {
        menu.ui = TitleUiState::Main;
    }

    static void activate_difficulty_selection(TitleMenu& menu,
                                              DifficultyLevel* difficulty_ptr,
                                              std::string& next) {
        if (!difficulty_ptr)
            return;

        *difficulty_ptr = static_cast<DifficultyLevel>(menu.diff_list.selected);
        SaveData save_data{};
        if (SaveSystem::load_default(save_data)) {
            save_data.difficulty = menu.diff_list.selected;
            SaveSystem::save_default(save_data);
        }
        next = "town";
    }

    static TitleMenuActionResult activate_settings_selection(TitleMenu& menu) {
        TitleMenuActionResult result;
        if (menu.settings_list.selected == 0) {
            menu.cfg.mute = !menu.cfg.mute;
            menu.refresh_settings_items();
            result.apply_audio_settings = true;
            result.persist_runtime_cfg  = true;
        } else if (menu.settings_list.selected == 1) {
            if (!menu.cfg.mute) {
                menu.cfg.volume_master += 0.1f;
                if (menu.cfg.volume_master > 1.0f)
                    menu.cfg.volume_master = 0.0f;
                menu.refresh_settings_items();
                result.apply_audio_settings = true;
                result.persist_runtime_cfg  = true;
            }
        } else if (menu.settings_list.selected == 2) {
            menu.lang_is_ptbr = !menu.lang_is_ptbr;
            result.reload_locale       = true;
            result.persist_runtime_cfg = true;
            result.refresh_menu_text   = true;
        } else if (menu.settings_list.selected == 4) {
            menu.ui = TitleUiState::Main;
        }
        return result;
    }

    static void activate_extras_selection(TitleMenu& menu, std::string& next) {
        if (menu.extras_list.selected == 0)
            next = "credits";
        else
            menu.ui = TitleUiState::Main;
    }
};

} // namespace mion
