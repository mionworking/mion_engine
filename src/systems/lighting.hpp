#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include "../core/camera.hpp"

namespace mion {

// Iluminação fake estilo Diablo 1:
// Preenche a máscara com escuridão e "fura" a região de luz usando
// SDL_RenderGeometry com alpha por vértice (centro=0, borda=darkness).
// BLENDMODE_NONE no render target grava os pixels diretamente —
// alpha=0 vira transparente quando a máscara é composta sobre a cena.
struct LightingSystem {
    float torch_radius   = 220.0f;  // raio da luz em pixels de tela
    float torch_softness = 120.0f;  // zona de transição (fade) em pixels
    Uint8 darkness       = 210;     // opacidade máxima do escuro (0=transparente, 255=preto)

    bool init(SDL_Renderer* r, int viewport_w, int viewport_h) {
        _w = viewport_w;
        _h = viewport_h;
        _mask = SDL_CreateTexture(r,
                    SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_TARGET,
                    viewport_w, viewport_h);
        if (!_mask) {
            SDL_Log("LightingSystem: falha ao criar render target: %s", SDL_GetError());
            return false;
        }
        SDL_SetTextureBlendMode(_mask, SDL_BLENDMODE_BLEND);
        return true;
    }

    void destroy() {
        if (_mask) { SDL_DestroyTexture(_mask); _mask = nullptr; }
    }

    // light_sx, light_sy — posição do player em coords de tela
    void render(SDL_Renderer* r, float light_sx, float light_sy) {
        if (!_mask) return;

        // --- Desenha no render target ---
        SDL_SetRenderTarget(r, _mask);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

        // Preenche toda a máscara com escuridão
        SDL_SetRenderDrawColor(r, 0, 0, 0, darkness);
        SDL_RenderClear(r);

        // "Fura" a luz usando geometria com alpha por vértice.
        // BLENDMODE_NONE grava o alpha diretamente no render target:
        //   alpha=0   → pixel transparente → cena aparece ali
        //   alpha=255 → pixel opaco        → cena bloqueada
        const int   seg     = 64;
        const float outer_r = torch_radius + torch_softness;
        const float da      = (float)darkness / 255.0f;

        // 1. Disco central totalmente transparente (área de luz plena)
        {
            SDL_Vertex center = { {light_sx, light_sy}, {0.f,0.f,0.f,0.f}, {0,0} };
            for (int i = 0; i < seg; ++i) {
                float a0 = (float)i       / seg * 6.2832f;
                float a1 = (float)(i + 1) / seg * 6.2832f;
                SDL_Vertex tri[3] = {
                    center,
                    { {light_sx + cosf(a0)*torch_radius, light_sy + sinf(a0)*torch_radius}, {0,0,0,0}, {0,0} },
                    { {light_sx + cosf(a1)*torch_radius, light_sy + sinf(a1)*torch_radius}, {0,0,0,0}, {0,0} },
                };
                SDL_RenderGeometry(r, nullptr, tri, 3, nullptr, 0);
            }
        }

        // 2. Anel de transição: interno alpha=0, externo alpha=darkness
        {
            for (int i = 0; i < seg; ++i) {
                float a0 = (float)i       / seg * 6.2832f;
                float a1 = (float)(i + 1) / seg * 6.2832f;
                SDL_Vertex quad[4] = {
                    { {light_sx + cosf(a0)*torch_radius, light_sy + sinf(a0)*torch_radius}, {0,0,0,0 }, {0,0} },
                    { {light_sx + cosf(a0)*outer_r,      light_sy + sinf(a0)*outer_r     }, {0,0,0,da}, {0,0} },
                    { {light_sx + cosf(a1)*torch_radius, light_sy + sinf(a1)*torch_radius}, {0,0,0,0 }, {0,0} },
                    { {light_sx + cosf(a1)*outer_r,      light_sy + sinf(a1)*outer_r     }, {0,0,0,da}, {0,0} },
                };
                int idx[6] = {0,1,2, 1,3,2};
                SDL_RenderGeometry(r, nullptr, quad, 4, idx, 6);
            }
        }

        // --- Composta a máscara sobre a cena ---
        SDL_SetRenderTarget(r, nullptr);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_RenderTexture(r, _mask, nullptr, nullptr);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

private:
    SDL_Texture* _mask = nullptr;
    int          _w    = 0;
    int          _h    = 0;
};

} // namespace mion
