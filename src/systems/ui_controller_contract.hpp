#pragma once

namespace mion {

// Canonical minimal contract for modal UI controllers:
// - world_paused: caller should suspend gameplay simulation for this tick.
// - should_save: caller (owner scene) should persist state.
struct UiControllerResult {
    bool world_paused = false;
    bool should_save  = false;
};

} // namespace mion
