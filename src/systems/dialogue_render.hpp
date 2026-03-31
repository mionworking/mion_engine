#pragma once
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/dialogue.hpp"
#include "dialogue_system.hpp"

namespace mion {

// Renderiza a UI de diálogo usando o estado atual do DialogueSystem.
inline void render_dialogue_ui(SDL_Renderer* r,
                               int viewport_w,
                               int viewport_h,
                               const DialogueSystem& dlg)
{
    if (!dlg.is_active())
        return;

    const std::vector<DialogueLine>& lines = dlg.active_lines();
    const int                       index = dlg.current_line_index();
    if (index < 0 || index >= static_cast<int>(lines.size()))
        return;

    const DialogueLine& line = lines[static_cast<size_t>(index)];

    const int   text_scale  = 2;
    const float pad_x       = 24.0f;
    const float pad_y       = 16.0f;
    const float bar_h       = 140.0f;
    const float bar_y       = static_cast<float>(viewport_h) - bar_h;
    const int   max_text_px = viewport_w - static_cast<int>(pad_x * 2.0f) - 40;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 220);
    SDL_FRect bar{0.0f, bar_y, static_cast<float>(viewport_w), bar_h};
    SDL_RenderFillRect(r, &bar);

    const float name_y = bar_y + pad_y;
    draw_text(r,
              pad_x,
              name_y,
              line.speaker.c_str(),
              text_scale,
              line.portrait_color.r,
              line.portrait_color.g,
              line.portrait_color.b,
              255);

    auto wrap_text_to_width = [](const std::string& text,
                                 int                max_px,
                                 int                scale,
                                 std::vector<std::string>& out) {
        out.clear();
        if (text.empty()) {
            out.push_back("");
            return;
        }

        auto fits = [&](const std::string& s) {
            return text_width(s.c_str(), scale) <= static_cast<float>(max_px);
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
            while (i < text.size() && text[i] == ' ')
                ++i;
            if (i >= text.size())
                break;

            size_t j = i;
            while (j < text.size() && text[j] != ' ')
                ++j;
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
        if (!cur.empty())
            out.push_back(cur);
    };

    std::vector<std::string> wrapped;
    wrap_text_to_width(line.text, max_text_px, text_scale, wrapped);
    float ty = name_y + 22.0f * static_cast<float>(text_scale);
    for (const auto& row : wrapped) {
        draw_text(r, pad_x, ty, row.c_str(), text_scale, 230, 230, 220, 255);
        ty += 12.0f * static_cast<float>(text_scale);
    }

    const Uint32 t        = SDL_GetTicks();
    const bool   blink_on = (t / 500u) % 2u == 0u;
    if (blink_on) {
        const char* hint = "ENTER - continuar";
        float       hx   = static_cast<float>(viewport_w)
                         - pad_x
                         - text_width(hint, text_scale);
        draw_text(r,
                  hx,
                  bar_y + bar_h - pad_y - 14.0f * static_cast<float>(text_scale),
                  hint,
                  text_scale,
                  255,
                  220,
                  80,
                  255);
    }
}

} // namespace mion

