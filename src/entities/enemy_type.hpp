#pragma once
#include "../components/collision.hpp"

namespace mion {

enum class EnemyType { Skeleton, Orc, Ghost };

struct EnemyDef {
    EnemyType    type;
    int          max_hp;
    float        move_speed;
    float        aggro_range;
    float        attack_range;
    float        stop_range;
    int          damage;
    float        knockback_friction;
    CollisionBox collision;
    HurtBox      hurt_box;
    MeleeHitBox  melee_hit_box;
    float        startup_sec;
    float        active_sec;
    float        recovery_sec;
    const char*  sprite_sheet_path;
    int          frame_w;
    int          frame_h;
    float        frame_fps;
    int          idle_frames;
    int          walk_frames;
    int          attack_frames;
    int          hurt_frames;
    int          death_frames;
};

inline const EnemyDef& get_enemy_def(EnemyType type) {
    static const EnemyDef DEFS[] = {
        { // Skeleton: ágil, fraco, dano baixo
          EnemyType::Skeleton, 60, 80.0f, 400.0f, 45.0f, 30.0f, 8, 0.72f,
          {16.0f, 16.0f}, {14.0f, 14.0f}, {20.0f, 12.0f, 22.0f},
          0.10f, 0.15f, 0.25f,
          "assets/sprites/skeleton.png", 32, 32, 8.0f, 4, 4, 4, 2, 4
        },
        { // Orc: lento, tanque, muito dano
          EnemyType::Orc, 120, 55.0f, 300.0f, 50.0f, 35.0f, 18, 0.68f,
          {20.0f, 18.0f}, {18.0f, 16.0f}, {26.0f, 14.0f, 28.0f},
          0.15f, 0.20f, 0.35f,
          "assets/sprites/orc.png", 48, 48, 6.0f, 4, 4, 4, 2, 5
        },
        { // Ghost: rápido, frágil, dano mínimo
          EnemyType::Ghost, 35, 130.0f, 500.0f, 40.0f, 25.0f, 5, 0.80f,
          {14.0f, 14.0f}, {12.0f, 12.0f}, {18.0f, 10.0f, 20.0f},
          0.08f, 0.10f, 0.20f,
          "assets/sprites/ghost.png", 32, 32, 10.0f, 4, 6, 4, 2, 4
        },
    };
    return DEFS[(int)type];
}

} // namespace mion
