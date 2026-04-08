#pragma once

namespace mion {

// Canonical scene ID constants used by SceneRegistry / SceneManager.
// Use these symbols instead of string literals when scheduling transitions
// or registering scenes, so that typos become compile errors.
namespace SceneId {
    inline constexpr const char* kTitle    = "title";
    inline constexpr const char* kWorld    = "world";
    inline constexpr const char* kGameOver = "game_over";
    inline constexpr const char* kVictory  = "victory";
    inline constexpr const char* kCredits  = "credits";
    // Special sentinel: handled by the game loop as an application-quit request.
    inline constexpr const char* kQuit     = "__quit__";
} // namespace SceneId

} // namespace mion
