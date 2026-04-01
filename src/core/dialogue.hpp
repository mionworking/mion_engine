#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>

namespace mion {

// A single line of dialogue with speaker name and portrait tint color.
struct DialogueLine {
    std::string speaker;
    std::string text;
    SDL_Color   portrait_color{200, 180, 255, 255};
};

// Ordered sequence of dialogue lines identified by a string ID.
struct DialogueSequence {
    std::string               id;
    std::vector<DialogueLine> lines;
};

} // namespace mion
