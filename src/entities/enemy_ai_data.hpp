#pragma once
#include <vector>
#include "nav_path.hpp"
#include "enemy_type.hpp"

namespace mion {

// Dados exclusivos de inimigos hostis — IA, patrulha, boss, ranged, pathfinding.
// Instanciado como std::optional<EnemyAIData> dentro de Actor; player deixa vazio.
struct EnemyAIData {
    struct PatrolWaypoint { float x, y; };

    // Ranges de IA
    float aggro_range     = 400.0f;
    float attack_range_ai = 45.0f;
    float stop_range_ai   = 30.0f;

    // Comportamento
    AiBehavior   ai_behavior = AiBehavior::Melee;

    // Pathfinding
    Path         nav_path;
    float        path_replan_timer = 0.0f;

    // Patrulha
    std::vector<PatrolWaypoint> patrol_waypoints;
    int          patrol_wp_index   = 0;
    PatrolState  patrol_state      = PatrolState::Patrol;
    float        alert_timer       = 0.0f;

    // Misc
    bool         is_elite                 = false;
    float        base_move_speed_at_spawn = 0.0f;

    // Boss
    int          boss_phase              = 1;
    float        boss_charge_cd          = 0.0f;
    bool         boss_charging           = false;
    float        boss_charge_remaining   = 0.0f;
    float        boss_charge_dir_x       = 0.0f;
    float        boss_charge_dir_y       = 0.0f;

    // Ranged IA
    float        ranged_fire_cd          = 0.0f;
    float        ranged_fire_rate        = 1.5f;
    float        ranged_keep_dist        = 200.0f;
    int          ranged_proj_damage      = 8;
};

} // namespace mion
