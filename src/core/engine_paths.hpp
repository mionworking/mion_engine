#pragma once
#include <cstdio>
#include <string>
#include <SDL3/SDL.h>

namespace mion {

// Resolve o caminho de um arquivo em data/.
// Tenta primeiro SDL_GetBasePath()/"data/", depois "data/" relativo ao cwd.
// Nunca retorna string vazia: sempre retorna um caminho tentável.
inline std::string resolve_data_path(const std::string& relative) {
    const char* base = SDL_GetBasePath(); // SDL3: const char*, não precisa de SDL_free
    if (base) {
        std::string p = std::string(base) + "data/" + relative;
        if (FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }
    return "data/" + relative;
}

} // namespace mion
