#pragma once

#include "scene_registry.hpp"

namespace mion {

// Registers all built-in scene factories (title, town, dungeon, game_over, victory, credits).
void register_default_scenes(SceneRegistry& registry);

} // namespace mion
