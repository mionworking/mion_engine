#pragma once
#include <algorithm>

namespace mion {

enum class DifficultyLevel { Easy = 0, Normal = 1, Hard = 2 };

struct RunStats {
    int   rooms_cleared     = 0;
    int   enemies_killed    = 0;
    int   gold_collected    = 0;
    int   damage_taken      = 0;
    int   spells_cast       = 0;
    int   potions_used      = 0;
    float time_seconds      = 0.0f;
    int   max_level_reached = 1;
    bool  boss_defeated     = false;

    void reset() { *this = RunStats{}; }
};

// Enemy HP / damage multipliers by difficulty level (mirrors DungeonScene logic).
inline void difficulty_enemy_multipliers(DifficultyLevel d, float& hp_mult, float& dmg_mult) {
    hp_mult  = 1.0f;
    dmg_mult = 1.0f;
    if (d == DifficultyLevel::Easy) {
        hp_mult  = 0.75f;
        dmg_mult = 0.75f;
    } else if (d == DifficultyLevel::Hard) {
        hp_mult  = 1.35f;
        dmg_mult = 1.25f;
    }
}

// Adjusts spawn budget by difficulty (Easy −1 min. 1; Hard +1).
inline int difficulty_adjust_spawn_budget(int base, DifficultyLevel d) {
    if (d == DifficultyLevel::Easy)
        return std::max(1, base - 1);
    if (d == DifficultyLevel::Hard)
        return base + 1;
    return base;
}

} // namespace mion
