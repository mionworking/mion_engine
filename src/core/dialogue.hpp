#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>

namespace mion {

struct DialogueLine {
    std::string speaker;
    std::string text;
    SDL_Color   portrait_color{200, 180, 255, 255};
};

struct DialogueSequence {
    std::string               id;
    std::vector<DialogueLine> lines;
};

} // namespace mion
