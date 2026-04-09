#pragma once

#include <array>
#include <string>

#include "../components/spell_defs.hpp"
#include "../core/data_file_names.hpp"
#include "../core/data_ini_loader.hpp"
#include "../entities/enemy_type.hpp"
#include "common_player_progression_loader.hpp"
#include "drop_system.hpp"

namespace mion {

namespace DungeonConfigLoader {

inline void load_dungeon_static_data(
    EnemyDef (&enemy_defs)[kEnemyTypeCount],
    DropConfig& drop_config,
    IniData& rooms_ini,
    std::array<std::string, kEnemyTypeCount>& enemy_sprite_paths) {
    CommonPlayerProgressionLoader::load_defaults_and_ini_overrides();
    g_spell_defs = make_spell_defs_from_ini(load_data_ini(data_files::kSpells));

    for (int i = 0; i < kEnemyTypeCount; ++i)
        enemy_defs[i] = get_enemy_def(static_cast<EnemyType>(i));
    drop_config = DropConfig{};
    enemy_sprite_paths.fill({});

    {
        IniData d = load_data_ini(data_files::kEnemies);
        apply_enemies_ini_overrides(d, enemy_defs, enemy_sprite_paths);
    }

    {
        IniData d = load_data_ini(data_files::kItems);
        apply_drop_ini_overrides(d, drop_config);
    }

    rooms_ini = load_data_ini(data_files::kRooms);
}

} // namespace DungeonConfigLoader

} // namespace mion
