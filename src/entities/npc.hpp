#pragma once
#include <string>
#include <SDL3/SDL.h>

namespace mion {

enum class NpcType { QuestGiver, Merchant, Generic };

struct NpcEntity {
    float       x = 0.0f;
    float       y = 0.0f;
    std::string name;
    NpcType     type            = NpcType::Generic;
    float       interact_radius = 48.0f;
    SDL_Color   portrait_color  = {180, 200, 160, 255};

    std::string dialogue_default;
    std::string dialogue_quest_active;
    std::string dialogue_quest_done;
};

} // namespace mion
