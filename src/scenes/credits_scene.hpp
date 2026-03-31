#pragma once

#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "../core/audio.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/locale.hpp"
#include "../core/scene.hpp"

namespace mion {

class CreditsScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    void set_audio(AudioSystem* a) { _audio = a; }

    void enter() override {
        _scroll_y    = (float)viewport_h;
        _next.clear();
        _prev_cancel = false;
        if (_audio) _audio->set_music_state(MusicState::None);
    }

    void exit() override {}

    void fixed_update(float dt, const InputState& input) override {
        _scroll_y -= 40.0f * dt;
        if (input.ui_cancel_pressed && !_prev_cancel)
            _next = "title";
        _prev_cancel = input.ui_cancel_pressed;

        float total_h = (float)(_lines.size() * 30);
        if (_scroll_y + total_h < 0.0f)
            _next = "title";
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
                    ? L(line.text_key)
                    : line.literal;
                float x = viewport_w * 0.5f - text_width(t, sc) * 0.5f;
                draw_text(r, x, y, t, sc, c.r, c.g, c.b, c.a);
            }
            y += line.header ? 46.0f : 30.0f;
        }

        const char* hint = L("credits_back_hint");
        draw_text(r, 12.0f, viewport_h - 24.0f, hint, 2, 130, 130, 120, 255);
    }

    const char* next_scene() const override {
        return _next.empty() ? "" : _next.c_str();
    }

    void clear_next_scene_request() override { _next.clear(); }

private:
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
    float        _scroll_y = 0.0f;
    std::string  _next;
    bool         _prev_cancel = false;
};

} // namespace mion
