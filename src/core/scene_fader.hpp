#pragma once
#include <algorithm>
#include <SDL3/SDL.h>

namespace mion {

// Fullscreen black fade between scenes. Cycle: Clear → FadingOut → Black → FadingIn → Clear.
struct SceneFader {
    enum class Phase { Clear, FadingOut, Black, FadingIn };

    Phase  phase    = Phase::Clear;
    float  duration = 0.35f;
    float  elapsed  = 0.0f;
    Uint8  alpha    = 0;

    bool is_clear() const { return phase == Phase::Clear; }
    bool is_black_hold() const { return phase == Phase::Black; }

    void start_fade_out(float dur = 0.35f) {
        phase    = Phase::FadingOut;
        duration = dur;
        elapsed  = 0.0f;
        alpha    = 0;
    }

    void start_fade_in(float dur = 0.35f) {
        phase    = Phase::FadingIn;
        duration = dur;
        elapsed  = 0.0f;
        alpha    = 255;
    }

    void tick(float dt) {
        switch (phase) {
        case Phase::Clear:
        case Phase::Black:
            return;
        case Phase::FadingOut:
        case Phase::FadingIn:
            break;
        }

        elapsed += dt;
        float t = (duration > 0.0f) ? std::min(1.0f, elapsed / duration) : 1.0f;

        if (phase == Phase::FadingOut) {
            alpha = (Uint8)(t * 255.0f);
            if (t >= 1.0f) {
                phase   = Phase::Black;
                alpha   = 255;
                elapsed = 0.0f;
            }
        } else if (phase == Phase::FadingIn) {
            alpha = (Uint8)((1.0f - t) * 255.0f);
            if (t >= 1.0f) {
                phase   = Phase::Clear;
                alpha   = 0;
                elapsed = 0.0f;
            }
        }
    }

    void render(SDL_Renderer* r, int viewport_w, int viewport_h) const {
        if (alpha == 0) return;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, alpha);
        SDL_FRect full = {0.0f, 0.0f, (float)viewport_w, (float)viewport_h};
        SDL_RenderFillRect(r, &full);
    }
};

} // namespace mion
