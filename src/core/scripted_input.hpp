#pragma once
#include <vector>
#include "input.hpp"

namespace mion {

// Implementação de IInputSource para testes de integração.
// Reproduce uma sequência fixa de InputState, frame a frame.
// Quando a sequência acaba, repete o último frame indefinidamente.
class ScriptedInputSource : public IInputSource {
public:
    // Adiciona um frame à sequência de replay
    void push(InputState s) { _frames.push_back(s); }

    // Atalho: adiciona N frames idênticos
    void push_n(InputState s, int count) {
        for (int i = 0; i < count; ++i) _frames.push_back(s);
    }

    InputState read_state() const override {
        if (_frames.empty()) return InputState{};
        if (_cursor < _frames.size()) return _frames[_cursor++];
        return _frames.back(); // repete último quando exausto
    }

    bool done() const { return !_frames.empty() && _cursor >= _frames.size(); }
    void reset()      { _cursor = 0; }
    int  total()      const { return (int)_frames.size(); }

private:
    std::vector<InputState> _frames;
    mutable size_t          _cursor = 0;
};

} // namespace mion
