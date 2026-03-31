#pragma once
#include <cmath>

#include "../core/ini_loader.hpp"

namespace mion {

// Curva de XP e bónus de level-up (data/progression.ini).
struct ProgressionConfig {
    int   xp_base            = 100;
    int   xp_level_scale     = 100;
    int   damage_bonus       = 3;
    int   hp_bonus           = 15;
    float speed_bonus        = 18.0f;
    float spell_mult_bonus   = 0.08f;
};

inline ProgressionConfig g_progression_config{};

inline void reset_progression_config_defaults() { g_progression_config = ProgressionConfig{}; }

inline void apply_progression_ini(const IniData& d) {
    g_progression_config.xp_base =
        d.get_int("xp", "xp_base", g_progression_config.xp_base);
    g_progression_config.xp_level_scale =
        d.get_int("xp", "xp_level_scale", g_progression_config.xp_level_scale);
    g_progression_config.damage_bonus =
        d.get_int("level_up", "damage_bonus", g_progression_config.damage_bonus);
    g_progression_config.hp_bonus =
        d.get_int("level_up", "hp_bonus", g_progression_config.hp_bonus);
    g_progression_config.speed_bonus =
        d.get_float("level_up", "speed_bonus", g_progression_config.speed_bonus);
    g_progression_config.spell_mult_bonus =
        d.get_float("level_up", "spell_mult_bonus", g_progression_config.spell_mult_bonus);
}

inline int progression_xp_threshold_for_level(int level) {
    return g_progression_config.xp_base
         + (level - 1) * g_progression_config.xp_level_scale;
}

struct ProgressionState {
    int   xp                 = 0;
    int   level              = 1;
    int   xp_to_next         = progression_xp_threshold_for_level(1);
    int   pending_level_ups  = 0;  // uma escolha de upgrade por nível ganho no mesmo frame

    // Upgrades escolhidos no level-up
    int   bonus_attack_damage = 0;
    int   bonus_max_hp        = 0;
    float bonus_move_speed     = 0.0f;  // somado ao move_speed base
    float spell_damage_multiplier = 1.0f;

    bool level_choice_pending() const { return pending_level_ups > 0; }

    int add_xp(int amount) {
        int gained_levels = 0;
        xp += amount;
        while (xp >= xp_to_next) {
            xp        -= xp_to_next;
            level     += 1;
            xp_to_next = progression_xp_threshold_for_level(level);
            pending_level_ups++;
            gained_levels++;
        }
        return gained_levels;
    }

    void pick_upgrade_damage() {
        bonus_attack_damage += g_progression_config.damage_bonus;
        spell_damage_multiplier += g_progression_config.spell_mult_bonus;
        if (pending_level_ups > 0) pending_level_ups--;
    }
    void pick_upgrade_hp() {
        bonus_max_hp += g_progression_config.hp_bonus;
        if (pending_level_ups > 0) pending_level_ups--;
    }
    void pick_upgrade_speed() {
        bonus_move_speed += g_progression_config.speed_bonus;
        if (pending_level_ups > 0) pending_level_ups--;
    }

    int scaled_spell_damage(int base_damage) const {
        return (int)std::lround((float)base_damage * spell_damage_multiplier);
    }
};

} // namespace mion
