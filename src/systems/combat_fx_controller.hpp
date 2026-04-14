#pragma once

#include <random>
#include <string>
#include <vector>

#include "../core/audio.hpp"
#include "../core/camera.hpp"
#include "../core/scene_ids.hpp"
#include "../entities/actor.hpp"
#include "floating_text.hpp"
#include "melee_combat.hpp"
#include "projectile_system.hpp"
#include "screen_fx.hpp"
#include "simple_particles.hpp"

namespace mion {

namespace CombatFxController {

inline void apply_combat_feedback(const MeleeCombatSystem& combat,
                                  Camera2D& camera,
                                  AudioSystem* audio,
                                  int& hitstop) {
    if (combat.pending_hitstop > 0) {
        hitstop = combat.pending_hitstop;
        if (combat.player_hit_landed)
            camera.trigger_shake(6.0f, 8);
    }
    if (combat.parry_success && audio)
        audio->play_sfx(SoundId::Parry, 0.85f);
}

inline void apply_projectile_hit_fx(const ProjectileSystem&    proj_sys,
                                    const std::vector<Actor*>& actors,
                                    SimpleParticleSystem&       particles,
                                    FloatingTextSystem&         floating_texts,
                                    ScreenFlashState&           flash,
                                    AudioSystem*                audio,
                                    float cam_aud_x, float cam_aud_y,
                                    std::mt19937*               rng) {
    if (proj_sys.projectile_hit_actor && rng)
        particles.spawn_burst(proj_sys.last_hit_world_x, proj_sys.last_hit_world_y,
                              12, 255, 210, 120, 80.f, 200.f, *rng);

    for (const auto& ev : proj_sys.last_hit_events) {
        for (auto* ac : actors) {
            if (ac->name != ev.target_name) continue;
            ac->hit_flash_timer = 0.15f;
            floating_texts.spawn_damage(ac->transform.x, ac->transform.y, ev.damage);
            if (ac->team == Team::Player)
                flash.trigger({180, 20, 20, 120}, 0.25f);
            if (audio)
                audio->play_sfx_at(SoundId::Hit, ac->transform.x, ac->transform.y,
                                   cam_aud_x, cam_aud_y, 620.f, 1.0f);
            break;
        }
    }
}

inline void apply_melee_hit_fx(const MeleeCombatSystem&   combat,
                                const std::vector<Actor*>& actors,
                                SimpleParticleSystem&       particles,
                                FloatingTextSystem&         floating_texts,
                                ScreenFlashState&           flash,
                                AudioSystem*                audio,
                                float cam_aud_x, float cam_aud_y,
                                std::mt19937*               rng) {
    for (const auto& ev : combat.last_events) {
        for (auto* ac : actors) {
            if (ac->name != ev.target_name) continue;
            ac->hit_flash_timer = 0.15f;
            floating_texts.spawn_damage(ac->transform.x, ac->transform.y, ev.damage);
            if (ac->team == Team::Player)
                flash.trigger({180, 20, 20, 120}, 0.25f);
            if (audio)
                audio->play_sfx_at(SoundId::Hit, ac->transform.x, ac->transform.y,
                                   cam_aud_x, cam_aud_y, 620.f, 1.0f);
            if (rng)
                particles.spawn_burst(ac->transform.x, ac->transform.y,
                                      14, 255, 200, 160, 60.f, 180.f, *rng);
            break;
        }
    }
}

} // namespace CombatFxController

// Player death state: triggers snapshot on first dead tick, then fades out.
// tick() returns SceneId::kGameOver when fade completes, SceneId::kNone otherwise.
struct PlayerDeathFlow {
    static constexpr float kFadeDuration = 1.5f;

    bool  snapshot_done  = false;
    float fade_remaining = 0.f;

    void reset() { snapshot_done = false; fade_remaining = 0.f; }

    template <typename SnapshotFn>
    SceneId tick(bool player_alive, float dt, SnapshotFn on_snapshot) {
        if (!player_alive && !snapshot_done) {
            snapshot_done  = true;
            on_snapshot();
            fade_remaining = kFadeDuration;
        }
        if (fade_remaining > 0.f) {
            fade_remaining -= dt;
            if (fade_remaining <= 0.f) {
                fade_remaining = 0.f;
                return SceneId::kGameOver;
            }
        }
        return SceneId::kNone;
    }
};

} // namespace mion
