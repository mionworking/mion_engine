#pragma once
#include <vector>
#include "input.hpp"

namespace mion {

// IInputSource implementation for integration tests.
// Replays a fixed sequence of InputState values, one per frame.
// When the sequence is exhausted, the last frame is repeated indefinitely.
class ScriptedInputSource : public IInputSource {
public:
    // Adds a frame to the replay sequence.
    void push(InputState s) { _frames.push_back(s); }

    // Shortcut: adds N identical frames.
    void push_n(InputState s, int count) {
        for (int i = 0; i < count; ++i) _frames.push_back(s);
    }

    InputState read_state() const override {
        if (_frames.empty()) return InputState{};
        if (_cursor < _frames.size()) return _frames[_cursor++];
        return _frames.back(); // repeat last frame when exhausted
    }

    bool done() const { return !_frames.empty() && _cursor >= _frames.size(); }
    void reset()      { _cursor = 0; }
    int  total()      const { return (int)_frames.size(); }

private:
    std::vector<InputState> _frames;
    mutable size_t          _cursor = 0;
};

} // namespace mion
