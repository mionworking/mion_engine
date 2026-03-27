#pragma once
#include <vector>
#include <array>

namespace mion {

// Layout padrão da spritesheet: tira horizontal, uma animação por linha
// Linha 0: Idle | 1: Walk | 2: Attack | 3: Hurt | 4: Death
enum class ActorAnim : int {
    Idle   = 0,
    Walk   = 1,
    Attack = 2,
    Hurt   = 3,
    Death  = 4,
    COUNT  = 5
};

// Recorte na spritesheet (sem dependência de SDL — conversão feita na cena)
struct FrameRect { int x, y, w, h; };

struct AnimFrame {
    FrameRect src;
    float     duration_seconds;
};

struct AnimClip {
    std::vector<AnimFrame> frames;
    bool loop = true;   // Death deve ser false
};

struct AnimPlayer {
    std::array<AnimClip, (int)ActorAnim::COUNT> clips = {};
    ActorAnim current_anim   = ActorAnim::Idle;
    int       frame_index    = 0;
    float     time_remaining = 0.0f;
    bool      finished       = false;  // true quando clip não-loop termina

    // Muda animação; reinicia se for diferente ou se a atual terminou
    void play(ActorAnim anim) {
        if (anim == current_anim && !finished) return;
        current_anim = anim;
        frame_index  = 0;
        finished     = false;
        const auto& cl = clips[(int)anim];
        time_remaining = !cl.frames.empty() ? cl.frames[0].duration_seconds : 0.0f;
    }

    // Avança um tick de dt segundos
    void advance(float dt) {
        if (finished) return;
        const auto& clip = clips[(int)current_anim];
        if (clip.frames.empty()) return;
        time_remaining -= dt;
        while (time_remaining <= 0.0f) {
            ++frame_index;
            if (frame_index >= (int)clip.frames.size()) {
                if (clip.loop) {
                    frame_index = 0;
                } else {
                    frame_index = (int)clip.frames.size() - 1;
                    finished    = true;
                    return;
                }
            }
            time_remaining += clip.frames[frame_index].duration_seconds;
        }
    }

    // Frame atual (nullptr se não houver clip configurado)
    const AnimFrame* current_frame() const {
        const auto& clip = clips[(int)current_anim];
        if (clip.frames.empty()) return nullptr;
        int idx = (frame_index < (int)clip.frames.size()) ? frame_index : 0;
        return &clip.frames[idx];
    }

    // Popula clipes para spritesheet com layout padrão (uma animação por linha)
    // frame_w/h: dimensão de um frame em pixels; fps: frames por segundo
    void build_default_clips(int frame_w, int frame_h,
                             int idle_n, int walk_n, int attack_n,
                             int hurt_n, int death_n, float fps)
    {
        float dur = (fps > 0.0f) ? 1.0f / fps : 0.1f;
        auto make = [&](ActorAnim anim, int row, int n, bool lp) {
            AnimClip cl; cl.loop = lp;
            for (int i = 0; i < n; ++i)
                cl.frames.push_back({ { i * frame_w, row * frame_h, frame_w, frame_h }, dur });
            clips[(int)anim] = std::move(cl);
        };
        make(ActorAnim::Idle,   0, idle_n,   true);
        make(ActorAnim::Walk,   1, walk_n,   true);
        make(ActorAnim::Attack, 2, attack_n, true);
        make(ActorAnim::Hurt,   3, hurt_n,   false);
        make(ActorAnim::Death,  4, death_n,  false);
    }
};

} // namespace mion
