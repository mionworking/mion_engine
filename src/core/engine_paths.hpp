#pragma once
#include <cstdio>
#include <string>
#include <SDL3/SDL.h>

namespace mion {

// Resolves the path to a file under data/.
// Tries SDL_GetBasePath()/"data/" first, then "data/" relative to cwd.
// Never returns an empty string: always returns a usable candidate path.
inline std::string resolve_data_path(const std::string& relative) {
    const char* base = SDL_GetBasePath(); // SDL3: const char*, no SDL_free needed
    if (base) {
        std::string p = std::string(base) + "data/" + relative;
        if (FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }

    {
        std::string p = "data/" + relative;
        if (FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }

    {
        std::string p = "../data/" + relative;
        if (FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }

    return "data/" + relative;
}

} // namespace mion
