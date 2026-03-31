#pragma once
#include <cstdio>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/audio.hpp"
#include "../core/ini_loader.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/dialogue.hpp"
#include "../core/input.hpp"
#include <SDL3/SDL.h>

namespace mion {

// UI de diálogo: avanço apenas com confirm_pressed (Enter), edge interno
class DialogueSystem {
public:
    void set_audio(AudioSystem* audio) { _audio = audio; }

    void register_sequence(DialogueSequence seq) {
        _sequences[std::move(seq.id)] = std::move(seq);
    }

    void clear_registry() { _sequences.clear(); }

    void start(const std::string& id, std::function<void()> on_finish = nullptr) {
        auto it = _sequences.find(id);
        if (it == _sequences.end()) return;
        _active_lines = it->second.lines;
        _line_index   = 0;
        _is_active    = !_active_lines.empty();
        _on_finish    = std::move(on_finish);
        _prev_confirm = true; // soltar Enter antes da primeira confirmação
    }

    void fixed_update(const InputState& input) {
        if (!_is_active) return;
        const bool edge = input.confirm_pressed && !_prev_confirm;
        _prev_confirm   = input.confirm_pressed;
        if (edge) {
            if (_audio && !_active_lines.empty()
                && _line_index >= 0
                && _line_index < static_cast<int>(_active_lines.size()))
                _audio->play_sfx(SoundId::UiConfirm, 0.42f);
            advance();
        }
    }

    void advance() {
        if (!_is_active) return;
        _line_index++;
        if (_line_index >= static_cast<int>(_active_lines.size())) {
            _is_active = false;
            _active_lines.clear();
            if (_on_finish) {
                auto cb = std::move(_on_finish);
                _on_finish = nullptr;
                cb();
            }
        }
    }

    bool is_active() const { return _is_active; }

    void render(SDL_Renderer* r, int viewport_w, int viewport_h) {
        if (!_is_active || _line_index < 0
            || _line_index >= static_cast<int>(_active_lines.size()))
            return;

        const DialogueLine& line = _active_lines[static_cast<size_t>(_line_index)];
        const int   text_scale  = 2;
        const float pad_x       = 24.0f;
        const float pad_y       = 16.0f;
        const float bar_h       = 140.0f;
        const float bar_y       = (float)viewport_h - bar_h;
        const int   max_text_px = viewport_w - (int)(pad_x * 2) - 40;

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 220);
        SDL_FRect bar{0.0f, bar_y, (float)viewport_w, bar_h};
        SDL_RenderFillRect(r, &bar);

        const float name_y = bar_y + pad_y;
        draw_text(r, pad_x, name_y, line.speaker.c_str(), text_scale,
                  line.portrait_color.r, line.portrait_color.g, line.portrait_color.b, 255);

        std::vector<std::string> wrapped;
        wrap_text_to_width(line.text, max_text_px, text_scale, wrapped);
        float ty = name_y + 22.0f * (float)text_scale;
        for (const auto& row : wrapped) {
            draw_text(r, pad_x, ty, row.c_str(), text_scale, 230, 230, 220, 255);
            ty += 12.0f * (float)text_scale;
        }

        const Uint32 t         = SDL_GetTicks();
        const bool   blink_on  = (t / 500u) % 2u == 0u;
        if (blink_on) {
            const char* hint = "ENTER - continuar";
            float       hx   = (float)viewport_w - pad_x - text_width(hint, text_scale);
            draw_text(r, hx, bar_y + bar_h - pad_y - 14.0f * (float)text_scale, hint,
                      text_scale, 255, 220, 80, 255);
        }
    }

private:
    static void wrap_text_to_width(const std::string& text, int max_px, int scale,
                                   std::vector<std::string>& out) {
        out.clear();
        if (text.empty()) {
            out.push_back("");
            return;
        }

        auto fits = [&](const std::string& s) {
            return text_width(s.c_str(), scale) <= (float)max_px;
        };

        auto flush_long_word = [&](std::string& word) {
            std::string chunk;
            for (char c : word) {
                std::string next = chunk + c;
                if (!chunk.empty() && !fits(next)) {
                    out.push_back(chunk);
                    chunk = std::string(1, c);
                } else {
                    chunk = std::move(next);
                }
            }
            return chunk;
        };

        std::string cur;
        size_t      i = 0;
        while (i < text.size()) {
            while (i < text.size() && text[i] == ' ') ++i;
            if (i >= text.size()) break;

            size_t j = i;
            while (j < text.size() && text[j] != ' ') ++j;
            std::string word = text.substr(i, j - i);

            if (cur.empty()) {
                if (fits(word))
                    cur = std::move(word);
                else
                    cur = flush_long_word(word);
            } else {
                std::string trial = cur + " " + word;
                if (fits(trial)) {
                    cur = std::move(trial);
                } else {
                    out.push_back(cur);
                    cur.clear();
                    if (fits(word))
                        cur = std::move(word);
                    else
                        cur = flush_long_word(word);
                }
            }
            i = j;
        }
        if (!cur.empty()) out.push_back(cur);
    }

    std::unordered_map<std::string, DialogueSequence> _sequences;
    std::vector<DialogueLine>                         _active_lines;
    int                                               _line_index   = 0;
    bool                                              _is_active    = false;
    bool                                              _prev_confirm = false;
    std::function<void()>                             _on_finish;
    AudioSystem*                                      _audio = nullptr;
};

/// Regista uma sequência por secção do INI com `line_N_speaker` / `line_N_text`.
inline void register_dialogue_sequences_from_ini(DialogueSystem& dlg, const IniData& ini) {
    for (const auto& sec_pair : ini.sections) {
        const std::string& sec_id = sec_pair.first;
        const auto&        kv     = sec_pair.second;
        DialogueSequence   seq;
        seq.id = sec_id;
        for (int i = 0;; ++i) {
            char ksp[48], ktx[48];
            std::snprintf(ksp, sizeof(ksp), "line_%d_speaker", i);
            std::snprintf(ktx, sizeof(ktx), "line_%d_text", i);
            auto its = kv.find(ksp);
            auto itt = kv.find(ktx);
            if (its == kv.end() || itt == kv.end())
                break;
            DialogueLine line;
            line.speaker = its->second;
            line.text    = itt->second;
            seq.lines.push_back(std::move(line));
        }
        if (!seq.lines.empty())
            dlg.register_sequence(std::move(seq));
    }
}

} // namespace mion
