#pragma once

#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "../core/audio.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/locale.hpp"
#include "../core/scene.hpp"
#include "../core/scene_ids.hpp"

namespace mion {

class CreditsScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    void set_audio(AudioSystem* a) { _audio = a; }
    void set_locale(LocaleSystem* l) { _locale = l; }

    void enter() override {
        _scroll_y    = (float)viewport_h;
        _next = SceneId::kNone;
        _prev_cancel = false;
        if (_audio) _audio->set_music_state(MusicState::None);
    }

    void exit() override {}

    void fixed_update(float dt, const InputState& input) override {
        _scroll_y -= 40.0f * dt;
        if (input.ui_cancel_pressed && !_prev_cancel)
            _next = SceneId::kTitle;
        _prev_cancel = input.ui_cancel_pressed;

        float total_h = (float)(_lines.size() * 30);
        if (_scroll_y + total_h < 0.0f)
            _next = SceneId::kTitle;
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        SDL_SetRenderDrawColor(r, 6, 6, 14, 255);
        SDL_RenderClear(r);

        float y = _scroll_y;
        for (const auto& line : _lines) {
            if (y > -30.0f && y < viewport_h + 30.0f) {
                const int   sc = line.header ? 3 : 2;
                SDL_Color   c  = line.header ? SDL_Color{255, 215, 100, 255}
                                             : SDL_Color{200, 200, 190, 255};
                const char* t  = (line.text_key && line.text_key[0] != '\0')
                    ? tr(line.text_key)
                    : line.literal;
                float x = viewport_w * 0.5f - text_width(t, sc) * 0.5f;
                draw_text(r, x, y, t, sc, c.r, c.g, c.b, c.a);
            }
            y += line.header ? 46.0f : 30.0f;
        }

        const char* hint = tr("credits_back_hint");
        draw_text(r, 12.0f, viewport_h - 24.0f, hint, 2, 130, 130, 120, 255);
    }

    SceneId next_scene() const override { return _next; }

    void clear_next_scene_request() override { _next = SceneId::kNone; }

private:
    const char* tr(const std::string& key) const {
        return _locale ? _locale->get(key) : key.c_str();
    }

    struct CreditLine {
        const char* literal = "";
        const char* text_key = "";
        bool header = false;
    };

    std::vector<CreditLine> _lines = {
        {"MION ENGINE", "", true},
        {"", "", false},
        {"", "credits_team_header", true},
        {"", "credits_team_name", false},
        {"", "", false},
        {"", "credits_tools_header", true},
        {"SDL3 | stb_image | C++17", "", false},
        {"", "", false},
        {"", "credits_thanks_header", true},
        {"", "credits_thanks_line", false},
        {"", "", false},
        {"", "credits_final_line", false},
    };

    AudioSystem* _audio = nullptr;
    LocaleSystem* _locale = nullptr;
    float        _scroll_y = 0.0f;
    SceneId      _next = SceneId::kNone;
    bool         _prev_cancel = false;
};

} // namespace mion
