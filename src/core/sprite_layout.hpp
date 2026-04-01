#pragma once

namespace mion {

// Standard logical layout for Puny-style character sprites.
// Keeping these values centralized makes it easy to migrate from 32×32
// to 48×48 or 64×64 in the future without hunting down magic numbers.
struct SpriteLayout {
    static constexpr int PunyFrameW  = 32;
    static constexpr int PunyFrameH  = 32;
    static constexpr int PunyCols    = 24;
    static constexpr int PunyRows    = 8;
};

} // namespace mion

