#pragma once
#include <optional>
#include <string>
#include <vector>
#include "../components/transform.hpp"
#include "../components/collision.hpp"
#include "../components/combat.hpp"
#include "../components/health.hpp"
#include "../components/status_effect.hpp"
#include "../components/attributes.hpp"
#include "../core/animation.hpp"
#include "enemy_type.hpp"
#include "nav_path.hpp"
#include "player_data.hpp"
#include "enemy_ai_data.hpp"

struct SDL_Texture;

namespace mion {

enum class Team { Player, Enemy };


struct Actor {
    std::string  name;
    Team         team            = Team::Enemy;
    Transform2D  transform;
    CollisionBox collision       = { 16.0f, 16.0f };
    HurtBox      hurt_box        = { 14.0f, 14.0f };
    MeleeHitBox  melee_hit_box   = { 20.0f, 12.0f, 22.0f };
    CombatState  combat;
    HealthState  health          = { 100, 100 };
    float        move_speed      = 0.0f;
    float        facing_x        = 1.0f;
    float        facing_y        = 0.0f;
    bool         is_alive        = true;
    bool         was_alive       = true;   // detect death transition
    bool         is_moving       = false;  // signals the animation system

    // Knockback
    float        knockback_vx       = 0.0f;
    float        knockback_vy       = 0.0f;
    float        knockback_friction = 0.72f;

    // Animation via spritesheet
    AnimPlayer   anim;
    SDL_Texture* sprite_sheet = nullptr;
    float        sprite_scale = 1.0f;

    // Enemy type + per-actor stats (usados por inimigos e sistema de drop/render)
    EnemyType    enemy_type      = EnemyType::Skeleton;
    int          attack_damage   = 10;
    int          ranged_damage   = 8;

    // Gameplay components
    StatusEffectState status_effects;    // Poison/Slow/Stun
    DerivedStats      derived;           // final stats — player e inimigos usam

    // Sub-structs opcionais — preenchidos na Sprint 5 (Actor split).
    // player:    só o player instancia (spell_book, talents, equipment, bag, etc.)
    // enemy_ai:  só inimigos hostis instanciam (patrulha, boss, ranged AI, etc.)
    std::optional<PlayerData>  player;
    std::optional<EnemyAIData> enemy_ai;

    // Brief white flash on hit (seconds).
    float hit_flash_timer = 0.0f;

    // Footstep anchor (accumulated distance since last sound).
    float footstep_prev_x       = 0.0f;
    float footstep_prev_y       = 0.0f;
    float footstep_accum_dist   = 0.0f;

    // Outgoing damage multiplier (melee, spells, arrows) while buff is active.
    float outgoing_damage_multiplier() const {
        return (player && player->empowered_remaining_seconds > 0.0f)
            ? player->empowered_damage_multiplier : 1.0f;
    }

    // --- Methods ---

    void apply_knockback_impulse(float ix, float iy) {
        knockback_vx = ix;
        knockback_vy = iy;
    }

    void step_knockback(float dt) {
        if (knockback_vx == 0.0f && knockback_vy == 0.0f) return;
        transform.translate(knockback_vx * dt, knockback_vy * dt);
        knockback_vx *= knockback_friction;
        knockback_vy *= knockback_friction;
        if (knockback_vx * knockback_vx + knockback_vy * knockback_vy < 1.0f) { // ||v|| < 1 px/s
            knockback_vx = 0.0f;
            knockback_vy = 0.0f;
        }
    }

    void set_facing(float dx, float dy) {
        if (dx != 0.0f || dy != 0.0f) {
            facing_x = dx;
            facing_y = dy;
        }
    }

    bool is_dashing() const { return player && player->dash_active_remaining_seconds > 0.0f; }

    float effective_move_speed() const {
        float s = move_speed;
        if (player)
            s += player->progression.bonus_move_speed;
        float slow_m = status_effects.slow_multiplier();
        return s * slow_m;
    }
};

} // namespace mion
