#pragma once
#include <cstdio>
#include <string>
#include <SDL3/SDL.h>
#include "audio_asset_paths.hpp"

namespace mion {

// Minimal list of critical assets to check at boot (SFX + music + referenced props).
inline void log_missing_assets_optional() {
    static const char* kExtraPaths[] = {
        "assets/audio/dungeon_ambient.wav",
        "assets/props/dungeon_pillar.png",
        "assets/props/dungeon_wall_mid.png",
    };

    auto log_if_missing = [](const char* rel) {
        std::string path;
        const char* base = SDL_GetBasePath();
        if (base) {
            path = std::string(base) + rel;
            if (FILE* f = std::fopen(path.c_str(), "rb")) {
                std::fclose(f);
                return;
            }
        }
        path = rel;
        if (FILE* f = std::fopen(path.c_str(), "rb")) {
            std::fclose(f);
            return;
        }
        SDL_Log("AssetManifest: missing or unreadable (tried base+cwd): %s", rel);
    };

    for (const char* rel : audio_assets::kSfxPaths) {
        log_if_missing(rel);
    }
    for (const char* rel : audio_assets::kMusicPaths) {
        if (!rel || !rel[0]) continue;
        log_if_missing(rel);
    }
    for (const char* rel : audio_assets::kAmbientPaths) {
        log_if_missing(rel);
    }
    for (const char* rel : kExtraPaths) {
        log_if_missing(rel);
    }
}

} // namespace mion
