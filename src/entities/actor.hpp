#pragma once
#include <string>
#include <vector>
#include "../components/transform.hpp"
#include "../components/collision.hpp"
#include "../components/combat.hpp"
#include "../components/health.hpp"
#include "../components/stamina.hpp"
#include "../components/mana.hpp"
#include "../components/status_effect.hpp"
#include "../components/progression.hpp"
#include "../components/spell_book.hpp"
#include "../components/talent_state.hpp"
#include "../components/attributes.hpp"
#include "../components/equipment.hpp"
#include "../core/animation.hpp"
#include "enemy_type.hpp"

struct SDL_Texture;

namespace mion {

enum class Team { Player, Enemy };

enum class PatrolState { Patrol, Alert, Chase };

// Waypoint path — defined here to avoid a circular dependency with pathfinder.hpp.
// Pathfinder fills this struct; EnemyAISystem consumes it.
struct Path {
    struct Point { float x, y; };
    std::vector<Point> waypoints;
    int  current = 0;
    bool valid   = false;

    bool  done()    const { return !valid || current >= (int)waypoints.size(); }
    Point next_wp() const { return done() ? Point{0,0} : waypoints[current]; }
    void  advance()       { if (!done()) ++current; }
    void  reset()         { waypoints.clear(); current = 0; valid = false; }
};

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

    // Enemy type + per-actor stats
    EnemyType    enemy_type      = EnemyType::Skeleton;
    int          attack_damage   = 10;
    int          ranged_damage   = 8;
    float        aggro_range     = 400.0f;
    float        attack_range_ai = 45.0f;
    float        stop_range_ai   = 30.0f;

    // Pathfinding
    Path         nav_path;
    float        path_replan_timer = 0.0f;

    // Behavior-based AI (copied from EnemyDef at spawn)
    AiBehavior   ai_behavior = AiBehavior::Melee;
    float        ranged_fire_cd        = 0.0f;
    float        ranged_fire_rate      = 1.5f;
    float        ranged_keep_dist      = 200.0f;
    int          ranged_proj_damage    = 8;
    struct PatrolWaypoint { float x, y; };
    std::vector<PatrolWaypoint> patrol_waypoints;
    int          patrol_wp_index   = 0;
    PatrolState  patrol_state      = PatrolState::Patrol;
    float        alert_timer       = 0.0f;
    bool         is_elite          = false;
    int          boss_phase        = 1;
    float        boss_charge_cd    = 0.0f;
    bool         boss_charging     = false;
    float        boss_charge_remaining = 0.0f;
    float        boss_charge_dir_x     = 0.0f;
    float        boss_charge_dir_y     = 0.0f;
    float        base_move_speed_at_spawn = 0.0f;

    // Gameplay components
    StaminaState      stamina;           // used by player; default empty for enemies
    ManaState         mana;              // player mana pool
    StatusEffectState status_effects;    // Poison/Slow/Stun
    ProgressionState  progression;       // XP/level — player only
    SpellBookState    spell_book;        // spells + cooldowns
    TalentState       talents;           // skill tree points and unlocks
    AttributesState   attributes;        // base attributes
    EquipmentState    equipment;         // equipment slots v1
    DerivedStats      derived;           // final stats computed by recompute_player_derived_stats
    int               gold              = 0;

    // Brief white flash on hit (seconds).
    float hit_flash_timer = 0.0f;

    // Footstep anchor (accumulated distance since last sound).
    float footstep_prev_x       = 0.0f;
    float footstep_prev_y       = 0.0f;
    float footstep_accum_dist   = 0.0f;

    // Dash / roll (primarily player)
    float dash_active_remaining_seconds  = 0.0f;
    float dash_cooldown_remaining_seconds = 0.0f;
    float dash_dir_x                     = 1.0f;
    float dash_dir_y                     = 0.0f;
    float dash_speed                     = 520.0f;
    float dash_duration_seconds          = 0.18f;
    float dash_iframes_seconds           = 0.20f;
    float dash_cooldown_seconds          = 0.55f;

    // Battle Cry (and other temporary damage buffs)
    float empowered_damage_multiplier   = 1.0f;
    float empowered_remaining_seconds   = 0.0f;

    // Outgoing damage multiplier (melee, spells, arrows) while buff is active.
    float outgoing_damage_multiplier() const {
        return empowered_remaining_seconds > 0.0f ? empowered_damage_multiplier : 1.0f;
    }

    // Ranged
    float ranged_cooldown_remaining_seconds = 0.0f;
    float ranged_cooldown_seconds           = 0.35f;

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

    bool is_dashing() const { return dash_active_remaining_seconds > 0.0f; }

    float effective_move_speed() const {
        float s = move_speed;
        if (team == Team::Player)
            s += progression.bonus_move_speed;
        float slow_m = status_effects.slow_multiplier();
        return s * slow_m;
    }
};

} // namespace mion
