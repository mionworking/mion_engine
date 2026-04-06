#pragma once

#include <array>
#include <string>

#include "../components/player_config.hpp"
#include "../components/progression.hpp"
#include "../components/spell_defs.hpp"
#include "../components/talent_tree.hpp"
#include "../core/engine_paths.hpp"
#include "../core/ini_loader.hpp"
#include "../entities/enemy_type.hpp"
#include "drop_system.hpp"

namespace mion {

namespace DungeonConfigLoader {

inline void load_dungeon_static_data(
    EnemyDef (&enemy_defs)[kEnemyTypeCount],
    DropConfig& drop_config,
    IniData& rooms_ini,
    std::array<std::string, kEnemyTypeCount>& enemy_sprite_paths) {
    reset_progression_config_defaults();
    reset_player_config_defaults();
    reset_talent_tree_defaults();

    for (int i = 0; i < kEnemyTypeCount; ++i)
        enemy_defs[i] = get_enemy_def(static_cast<EnemyType>(i));
    drop_config = DropConfig{};
    enemy_sprite_paths.fill({});

    {
        IniData d = ini_load(resolve_data_path("progression.ini").c_str());
        if (!d.sections.empty())
            apply_progression_ini(d);
    }
    {
        IniData d = ini_load(resolve_data_path("player.ini").c_str());
        if (!d.sections.empty())
            apply_player_ini(d);
    }
    {
        IniData d = ini_load(resolve_data_path("talents.ini").c_str());
        if (!d.sections.empty())
            apply_talents_ini(d);
    }

    {
        IniData d = ini_load(resolve_data_path("enemies.ini").c_str());
        static const char* kEnemyIniSecs[kEnemyTypeCount] = {
            "skeleton", "orc", "ghost", "archer",
            "patrol_guard", "elite_skeleton", "boss_grimjaw",
        };
        for (int i = 0; i < kEnemyTypeCount; ++i) {
            apply_enemy_ini_section(d, kEnemyIniSecs[i], enemy_defs[i],
                                    &enemy_sprite_paths[static_cast<size_t>(i)]);
        }
    }

    {
        IniData d = ini_load(resolve_data_path("spells.ini").c_str());
        apply_spell_ini_section(d, "frost_bolt",
                                g_spell_defs[static_cast<int>(SpellId::FrostBolt)]);
        apply_spell_ini_section(d, "bolt",
                                g_spell_defs[static_cast<int>(SpellId::FrostBolt)]);
        apply_spell_ini_section(d, "nova",
                                g_spell_defs[static_cast<int>(SpellId::Nova)]);
        apply_spell_ini_section(d, "chain_lightning",
                                g_spell_defs[static_cast<int>(SpellId::ChainLightning)]);
        apply_spell_ini_section(d, "multi_shot",
                                g_spell_defs[static_cast<int>(SpellId::MultiShot)]);
        apply_spell_ini_section(d, "poison_arrow",
                                g_spell_defs[static_cast<int>(SpellId::PoisonArrow)]);
        apply_spell_ini_section(d, "strafe",
                                g_spell_defs[static_cast<int>(SpellId::Strafe)]);
        apply_spell_ini_section(d, "cleave",
                                g_spell_defs[static_cast<int>(SpellId::Cleave)]);
        apply_spell_ini_section(d, "battle_cry",
                                g_spell_defs[static_cast<int>(SpellId::BattleCry)]);
    }

    {
        IniData d = ini_load(resolve_data_path("items.ini").c_str());
        drop_config.drop_chance_pct =
            d.get_int("drops", "drop_chance_pct", drop_config.drop_chance_pct);
        drop_config.pickup_radius =
            d.get_float("drops", "pickup_radius", drop_config.pickup_radius);
        drop_config.health_bonus =
            d.get_int("drops", "health_bonus", drop_config.health_bonus);
        drop_config.damage_bonus =
            d.get_int("drops", "damage_bonus", drop_config.damage_bonus);
        drop_config.speed_bonus =
            d.get_float("drops", "speed_bonus", drop_config.speed_bonus);
        drop_config.lore_drop_chance_pct =
            d.get_int("drops", "lore_drop_chance_pct", drop_config.lore_drop_chance_pct);
    }

    rooms_ini = ini_load(resolve_data_path("rooms.ini").c_str());
}

} // namespace DungeonConfigLoader

} // namespace mion
