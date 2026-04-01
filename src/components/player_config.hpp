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

inline PlayerConfig g_player_config{};

inline void reset_player_config_defaults() { g_player_config = PlayerConfig{}; }

inline void apply_player_ini(const IniData& d) {
    const std::string sec = "player";
    g_player_config.base_hp =
        d.get_int(sec, "base_hp", g_player_config.base_hp);
    g_player_config.base_move_speed =
        d.get_float(sec, "base_move_speed", g_player_config.base_move_speed);
    g_player_config.base_mana =
        d.get_float(sec, "base_mana", g_player_config.base_mana);
    g_player_config.base_mana_max =
        d.get_float(sec, "base_mana_max", g_player_config.base_mana_max);
    if (g_player_config.base_mana_max < g_player_config.base_mana)
        g_player_config.base_mana_max = g_player_config.base_mana;
    g_player_config.base_mana_regen =
        d.get_float(sec, "base_mana_regen", g_player_config.base_mana_regen);
    g_player_config.mana_regen_delay =
        d.get_float(sec, "mana_regen_delay", g_player_config.mana_regen_delay);
    g_player_config.base_stamina =
        d.get_float(sec, "base_stamina", g_player_config.base_stamina);
    g_player_config.base_stamina_max =
        d.get_float(sec, "base_stamina_max", g_player_config.base_stamina_max);
    if (g_player_config.base_stamina_max < g_player_config.base_stamina)
        g_player_config.base_stamina_max = g_player_config.base_stamina;
    g_player_config.stamina_regen =
        d.get_float(sec, "stamina_regen", g_player_config.stamina_regen);
    g_player_config.stamina_delay =
        d.get_float(sec, "stamina_delay", g_player_config.stamina_delay);
    g_player_config.melee_damage =
        d.get_int(sec, "melee_damage", g_player_config.melee_damage);
    g_player_config.ranged_damage =
        d.get_int(sec, "ranged_damage", g_player_config.ranged_damage);
    g_player_config.ranged_cooldown =
        d.get_float(sec, "ranged_cooldown", g_player_config.ranged_cooldown);
    g_player_config.dash_speed =
        d.get_float(sec, "dash_speed", g_player_config.dash_speed);
    g_player_config.dash_duration =
        d.get_float(sec, "dash_duration", g_player_config.dash_duration);
    g_player_config.dash_cooldown =
        d.get_float(sec, "dash_cooldown", g_player_config.dash_cooldown);
    g_player_config.dash_iframes =
        d.get_float(sec, "dash_iframes", g_player_config.dash_iframes);
}

} // namespace mion
