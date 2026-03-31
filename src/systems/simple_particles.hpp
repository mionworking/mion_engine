#pragma once
#include <SDL3/SDL.h>
#include <cstdlib>
#include <cmath>

#include "../core/camera.hpp"

namespace mion {

struct SimpleParticle {
    float x = 0, y = 0, vx = 0, vy = 0;
    float life = 0, max_life = 0.01f;
    Uint8 r = 255, g = 255, b = 255;
};

// Pool fixo, sem alocações — impactos melee/projétil e mortes.
class SimpleParticleSystem {
public:
    static constexpr int kMaxParticles = 400;

    void clear() { _count = 0; }

    int live_particle_count() const { return _count; }

    void spawn_burst(float wx, float wy, int n, Uint8 cr, Uint8 cg, Uint8 cb,
                     float speed_min = 48.f, float speed_max = 160.f) {
        for (int i = 0; i < n && _count < kMaxParticles; ++i) {
            float a = (float)(std::rand() % 628) * 0.01f;
            int   span = (int)(speed_max - speed_min) + 1;
            if (span < 1) span = 1;
            float sp = speed_min + (float)(std::rand() % span);
            float life = 0.22f + (float)(std::rand() % 80) * 0.002f;
            SimpleParticle& p = _particles[_count++];
            p.x = wx;
            p.y = wy;
            p.vx = cosf(a) * sp;
            p.vy = sinf(a) * sp;
            p.life = p.max_life = life;
            p.r    = cr;
            p.g    = cg;
            p.b    = cb;
        }
    }

    void update(float dt) {
        int w = 0;
        for (int i = 0; i < _count; ++i) {
            SimpleParticle& p = _particles[i];
            p.life -= dt;
            if (p.life <= 0.f) continue;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.vx *= 0.91f;
            p.vy *= 0.91f;
            _particles[w++] = p;
        }
        _count = w;
    }

    void render(SDL_Renderer* r, const Camera2D& cam) const {
        for (int i = 0; i < _count; ++i) {
            const SimpleParticle& p = _particles[i];
            float t = (p.max_life > 0.f) ? (p.life / p.max_life) : 0.f;
            if (t < 0.f) t = 0.f;
            Uint8 a = (Uint8)(220.f * t);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, p.r, p.g, p.b, a);
            float px = cam.world_to_screen_x(p.x);
            float py = cam.world_to_screen_y(p.y);
            SDL_FRect dot = { px - 2.0f, py - 2.0f, 4.0f, 4.0f };
            SDL_RenderFillRect(r, &dot);
        }
    }

private:
    SimpleParticle _particles[kMaxParticles]{};
    int            _count = 0;
};

} // namespace mion
