#pragma once
#include <string>
#include <vector>
#include "../components/transform.hpp"
#include "../components/collision.hpp"
#include "../components/combat.hpp"
#include "../components/health.hpp"
#include "../core/animation.hpp"
#include "enemy_type.hpp"

namespace mion {

enum class Team { Player, Enemy };

// Waypoint path — definido aqui para evitar dependência circular com pathfinder.hpp
// O Pathfinder preenche esta struct; o EnemyAISystem a consome.
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
    bool         was_alive       = true;   // detectar transição de morte (Feature 4)
    bool         is_moving       = false;  // sinaliza para animação (Feature 1)

    // Knockback
    float        knockback_vx       = 0.0f;
    float        knockback_vy       = 0.0f;
    float        knockback_friction = 0.72f;

    // Feature 1 — Animação por spritesheet
    AnimPlayer   anim;
    void*        sprite_sheet = nullptr;  // SDL_Texture* — void* para manter actor.hpp sem SDL

    // Feature 2 — Tipo de inimigo + stats por-actor
    EnemyType    enemy_type      = EnemyType::Skeleton;
    int          attack_damage   = 10;
    float        aggro_range     = 400.0f;
    float        attack_range_ai = 45.0f;
    float        stop_range_ai   = 30.0f;

    // Feature 3 — Pathfinding
    Path         nav_path;
    float        path_replan_timer = 0.0f;

    // --- Métodos existentes ---

    void apply_knockback_impulse(float ix, float iy) {
        knockback_vx = ix;
        knockback_vy = iy;
    }

    void step_knockback(float dt) {
        if (knockback_vx == 0.0f && knockback_vy == 0.0f) return;
        transform.translate(knockback_vx * dt, knockback_vy * dt);
        knockback_vx *= knockback_friction;
        knockback_vy *= knockback_friction;
        if (knockback_vx * knockback_vx + knockback_vy * knockback_vy < 0.5f) {
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
};

} // namespace mion
