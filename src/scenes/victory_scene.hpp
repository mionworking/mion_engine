#pragma once
#include <string>

#include <SDL3/SDL.h>

#include "../core/scene.hpp"
#include "../core/audio.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/debug_log.hpp"
#include "../core/run_stats.hpp"
#include "../core/ui.hpp"
#include "../core/locale.hpp"
#include "../core/scene_ids.hpp"
#include "../systems/world_save_controller.hpp"

namespace mion {

class VictoryScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    RunStats run_stats_snapshot{};

    void set_audio(AudioSystem* a) { _audio = a; }
    void set_run_stats_source(RunStats* p) { _run_stats_src = p; }
    void set_locale(LocaleSystem* l) { _locale = l; }

    void enter() override {
        _next = SceneId::kNone;
        _prev_confirm = false;
        _prev_up = false;
        _prev_down = false;
        _elapsed = 0.0f;
        if (_run_stats_src)
            run_stats_snapshot = *_run_stats_src;
        _list.items    = {tr("menu_continue_town"), "NEW GAME+", tr("menu_main_menu"), tr("ui_quit")};
        _list.selected = 0;
        _list.item_h_px = 28;
        _list.text_scale = 2;
        if (_audio) {
            _audio->play_sfx(SoundId::UiConfirm, 0.55f);
            _audio->set_music_state(MusicState::Victory);
        }
    }

    void exit() override {}

    void fixed_update(float dt, const InputState& input) override {
        _elapsed += dt;
        if (input.ui_up_pressed && !_prev_up)
            _list.nav_up();
        if (input.ui_down_pressed && !_prev_down)
            _list.nav_down();
        _prev_up   = input.ui_up_pressed;
        _prev_down = input.ui_down_pressed;

        const bool ok = input.confirm_pressed && !_prev_confirm && _elapsed > 1.0f;
        if (ok) {
            switch (_list.selected) {
            case 0:
                _next = SceneId::kWorld;
                break;
            case 1: {
                if (!WorldSaveController::mark_victory_reached(run_stats_snapshot))
                    debug_log("VictoryScene: failed to persist victory snapshot");
                _next = SceneId::kTitle;
                break;
            }
            case 2:
                _next = SceneId::kTitle;
                break;
            case 3:
                _next = SceneId::kQuit;
                break;
            default:
                break;
            }
        }
        _prev_confirm = input.confirm_pressed;
        if (_audio) _audio->tick_music();
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        SDL_SetRenderDrawColor(r, 5, 22, 12, 255);
        SDL_RenderClear(r);

        float cx = viewport_w * 0.5f;
        const char* t1 = tr("ui_victory");
        draw_text(r, cx - text_width(t1, 4) * 0.5f, viewport_h * 0.08f, t1, 4, 90, 230, 110);
        const char* t2 = tr("victory_subtitle");
        draw_text(r, cx - text_width(t2, 2) * 0.5f, viewport_h * 0.20f, t2, 2, 190, 230, 190);

        float sy = viewport_h * 0.30f;
        float sx = viewport_w * 0.20f;
        char buf[96];
        SDL_snprintf(buf, sizeof(buf), "%s:   %d", tr("stat_rooms"), run_stats_snapshot.rooms_cleared);
        draw_text(r, sx, sy, buf, 2, 200, 210, 190);
        sy += 26;
        SDL_snprintf(buf, sizeof(buf), "%s:  %d", tr("stat_enemies"), run_stats_snapshot.enemies_killed);
        draw_text(r, sx, sy, buf, 2, 200, 210, 190);
        sy += 26;
        SDL_snprintf(buf, sizeof(buf), "%s:  %d", tr("stat_gold"), run_stats_snapshot.gold_collected);
        draw_text(r, sx, sy, buf, 2, 200, 210, 190);
        sy += 26;
        SDL_snprintf(buf, sizeof(buf), "%s:    %d", tr("stat_damage"), run_stats_snapshot.damage_taken);
        draw_text(r, sx, sy, buf, 2, 200, 210, 190);
        sy += 26;
        int tsec = (int)run_stats_snapshot.time_seconds;
        SDL_snprintf(buf, sizeof(buf), "%s:  %dm %02ds", tr("stat_time"), tsec / 60, tsec % 60);
        draw_text(r, sx, sy, buf, 2, 200, 210, 190);

        _list.render(r, viewport_w * 0.28f, viewport_h * 0.62f);
    }

    SceneId next_scene() const override { return _next; }

    void clear_next_scene_request() override { _next = SceneId::kNone; }

private:
    const char* tr(const std::string& key) const {
        return _locale ? _locale->get(key) : key.c_str();
    }

    RunStats*     _run_stats_src = nullptr;
    AudioSystem*  _audio         = nullptr;
    LocaleSystem* _locale        = nullptr;
    SceneId       _next = SceneId::kNone;
    bool          _prev_confirm = false;
    bool          _prev_up      = false;
    bool          _prev_down    = false;
    float         _elapsed      = 0.0f;
    ui::List      _list;
};

} // namespace mion
