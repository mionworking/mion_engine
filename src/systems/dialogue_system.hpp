#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/audio.hpp"
#include "../core/dialogue.hpp"
#include "../core/input.hpp"

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
    const std::vector<DialogueLine>& active_lines() const { return _active_lines; }
    int  current_line_index() const { return _line_index; }

    std::unordered_map<std::string, DialogueSequence> _sequences;
    std::vector<DialogueLine>                         _active_lines;
    int                                               _line_index   = 0;
    bool                                              _is_active    = false;
    bool                                              _prev_confirm = false;
    std::function<void()>                             _on_finish;
    AudioSystem*                                      _audio = nullptr;
};

} // namespace mion
