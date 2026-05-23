#pragma once
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "../core/ini_loader.hpp"

namespace mion {

// Karma level thresholds — defaults compilados; override por data/progression.ini § [karma_levels].
// Ordem: thresholds[0] = karma mínimo pra level 1, [1] pra level 2, ...
inline std::vector<int64_t> default_karma_thresholds() {
    return {
        0,        // L1
        100,      // L2
        250,      // L3
        500,      // L4
        1000,     // L5
        2000,     // L6
        4000,     // L7
        7000,     // L8
        11000,    // L9
        16000,    // L10
        22000,    // L11
        30000,    // L12
        40000,    // L13
        55000,    // L14
        75000,    // L15
        100000,   // L16
        130000,   // L17
        170000,   // L18
        220000,   // L19
        290000,   // L20
    };
}

// XP curve and level-up bonuses (data/progression.ini).
struct ProgressionConfig {
    int   xp_base            = 100;
    int   xp_level_scale     = 100;
    int   damage_bonus       = 3;
    int   hp_bonus           = 15;
    float speed_bonus        = 18.0f;
    float spell_mult_bonus   = 0.08f;
    std::vector<int64_t> karma_thresholds = default_karma_thresholds();
};

inline const ProgressionConfig kDefaultProgressionConfig{};

// Builds a ProgressionConfig from INI overrides applied on top of `base`.
// Pure factory — does not touch any global. Missing INI keys keep the base value.
inline ProgressionConfig make_progression_config_from_ini(
    const IniData& d,
    const ProgressionConfig& base = kDefaultProgressionConfig) {
    ProgressionConfig cfg = base;
    cfg.xp_base         = d.get_int("xp",       "xp_base",          cfg.xp_base);
    cfg.xp_level_scale  = d.get_int("xp",       "xp_level_scale",   cfg.xp_level_scale);
    cfg.damage_bonus    = d.get_int("level_up",  "damage_bonus",     cfg.damage_bonus);
    cfg.hp_bonus        = d.get_int("level_up",  "hp_bonus",         cfg.hp_bonus);
    cfg.speed_bonus     = d.get_float("level_up","speed_bonus",      cfg.speed_bonus);
    cfg.spell_mult_bonus= d.get_float("level_up","spell_mult_bonus", cfg.spell_mult_bonus);

    // [karma_levels] level_1, level_2, ... — sobrescreve elemento a elemento, preservando defaults pra keys ausentes.
    for (size_t i = 0; i < cfg.karma_thresholds.size(); ++i) {
        const std::string key = "level_" + std::to_string(i + 1);
        const int default_val = static_cast<int>(cfg.karma_thresholds[i]);
        cfg.karma_thresholds[i] = d.get_int("karma_levels", key, default_val);
    }
    return cfg;
}

// Global config — read-only após bootstrap. Escrita apenas em init (make_progression_config_from_ini).
namespace detail { inline ProgressionConfig _g_progression_config_mutable = kDefaultProgressionConfig; }
inline const ProgressionConfig& g_progression_config = detail::_g_progression_config_mutable;

inline void reset_progression_config_defaults() {
    detail::_g_progression_config_mutable = kDefaultProgressionConfig;
}

inline int progression_xp_threshold_for_level(int level) {
    return g_progression_config.xp_base
         + (level - 1) * g_progression_config.xp_level_scale;
}

// Função pura: retorna o último level cujo threshold <= karma_total.
// Mínimo 1 (mesmo com vetor vazio ou karma negativo). Clampa no máximo definido.
inline int karma_level_for(int64_t karma_total,
                           const std::vector<int64_t>& thresholds) {
    int lvl = 1;
    for (size_t i = 0; i < thresholds.size(); ++i) {
        if (karma_total >= thresholds[i])
            lvl = static_cast<int>(i) + 1;
        else
            break;
    }
    return lvl;
}

// Helper que usa a config global (chamável de qualquer sistema).
inline int karma_level(int64_t karma_total) {
    return karma_level_for(karma_total, g_progression_config.karma_thresholds);
}

// Ratio [0, 1] do karma_total dentro do level atual.
// Útil pra barra de progresso de karma no HUD.
// - Vetor vazio: retorna 0.0.
// - karma_total negativo ou abaixo do primeiro threshold: 0.0.
// - karma_total >= último threshold: 1.0 (barra cheia, level máximo).
// - Entre dois thresholds: progresso linear normalizado.
inline float karma_progress_ratio_for(int64_t karma_total,
                                       const std::vector<int64_t>& thresholds) {
    if (thresholds.empty()) return 0.0f;
    const int lvl = karma_level_for(karma_total, thresholds);
    const size_t idx = static_cast<size_t>(lvl - 1);
    if (idx + 1 >= thresholds.size()) return 1.0f;  // último level: cheio
    const int64_t lo = thresholds[idx];
    const int64_t hi = thresholds[idx + 1];
    if (hi <= lo) return 0.0f;  // sanity
    const double r = (double)(karma_total - lo) / (double)(hi - lo);
    if (r < 0.0) return 0.0f;
    if (r > 1.0) return 1.0f;
    return (float)r;
}

// Helper que usa a config global.
inline float karma_progress_ratio(int64_t karma_total) {
    return karma_progress_ratio_for(karma_total, g_progression_config.karma_thresholds);
}

struct ProgressionState {
    int   xp                 = 0;
    int   level              = 1;
    int   xp_to_next         = progression_xp_threshold_for_level(1);
    int   pending_level_ups  = 0;  // one upgrade choice per level gained in the same frame

    // Upgrades chosen at level-up
    int   bonus_attack_damage = 0;
    int   bonus_max_hp        = 0;
    float bonus_move_speed     = 0.0f;  // added to the base move_speed
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
