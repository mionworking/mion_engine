#pragma once
#include "../core/ini_loader.hpp"

namespace mion {

// Values applied to the player in DungeonScene::_configure_player_state (data/player.ini).
struct PlayerConfig {
    int   base_hp             = 100;
    float base_move_speed     = 200.0f;
    float base_mana           = 100.0f;
    float base_mana_max       = 100.0f;
    float base_mana_regen     = 20.0f;
    float mana_regen_delay    = 0.75f;
    float base_stamina        = 100.0f;
    float base_stamina_max    = 100.0f;
    float stamina_regen       = 30.0f;
    float stamina_delay       = 1.0f;
    int   melee_damage        = 10;
    int   ranged_damage       = 8;
    float ranged_cooldown     = 0.35f;
    float dash_speed          = 520.0f;
    float dash_duration       = 0.18f;
    float dash_cooldown       = 0.55f;
    float dash_iframes        = 0.20f;
};

inline constexpr PlayerConfig kDefaultPlayerConfig{};

// Builds a PlayerConfig from INI overrides applied on top of `base`.
// Pure factory — does not touch any global. Missing INI keys keep the base value.
inline PlayerConfig make_player_config_from_ini(const IniData& d,
                                                 const PlayerConfig& base = kDefaultPlayerConfig) {
    PlayerConfig cfg = base;
    const std::string sec = "player";
    cfg.base_hp          = d.get_int(sec,   "base_hp",          cfg.base_hp);
    cfg.base_move_speed  = d.get_float(sec, "base_move_speed",  cfg.base_move_speed);
    cfg.base_mana        = d.get_float(sec, "base_mana",        cfg.base_mana);
    cfg.base_mana_max    = d.get_float(sec, "base_mana_max",    cfg.base_mana_max);
    if (cfg.base_mana_max < cfg.base_mana) cfg.base_mana_max = cfg.base_mana;
    cfg.base_mana_regen  = d.get_float(sec, "base_mana_regen",  cfg.base_mana_regen);
    cfg.mana_regen_delay = d.get_float(sec, "mana_regen_delay", cfg.mana_regen_delay);
    cfg.base_stamina     = d.get_float(sec, "base_stamina",     cfg.base_stamina);
    cfg.base_stamina_max = d.get_float(sec, "base_stamina_max", cfg.base_stamina_max);
    if (cfg.base_stamina_max < cfg.base_stamina) cfg.base_stamina_max = cfg.base_stamina;
    cfg.stamina_regen    = d.get_float(sec, "stamina_regen",    cfg.stamina_regen);
    cfg.stamina_delay    = d.get_float(sec, "stamina_delay",    cfg.stamina_delay);
    cfg.melee_damage     = d.get_int(sec,   "melee_damage",     cfg.melee_damage);
    cfg.ranged_damage    = d.get_int(sec,   "ranged_damage",    cfg.ranged_damage);
    cfg.ranged_cooldown  = d.get_float(sec, "ranged_cooldown",  cfg.ranged_cooldown);
    cfg.dash_speed       = d.get_float(sec, "dash_speed",       cfg.dash_speed);
    cfg.dash_duration    = d.get_float(sec, "dash_duration",    cfg.dash_duration);
    cfg.dash_cooldown    = d.get_float(sec, "dash_cooldown",    cfg.dash_cooldown);
    cfg.dash_iframes     = d.get_float(sec, "dash_iframes",     cfg.dash_iframes);
    return cfg;
}

// Global config — assigned once during bootstrap via make_player_config_from_ini.
// Treat as read-only after CommonPlayerProgressionLoader::load_defaults_and_ini_overrides().
inline PlayerConfig g_player_config = kDefaultPlayerConfig;

inline void reset_player_config_defaults() { g_player_config = kDefaultPlayerConfig; }

} // namespace mion
