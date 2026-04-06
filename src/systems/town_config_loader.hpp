#pragma once

#include "../components/player_config.hpp"
#include "../components/progression.hpp"
#include "../components/talent_tree.hpp"
#include "../core/dialogue_loader.hpp"
#include "../core/engine_paths.hpp"
#include "../core/ini_loader.hpp"

namespace mion {

namespace TownConfigLoader {

inline void load_town_player_and_progression_config() {
    reset_player_config_defaults();
    reset_progression_config_defaults();
    reset_talent_tree_defaults();

    {
        IniData d = ini_load(resolve_data_path("player.ini").c_str());
        if (!d.sections.empty())
            apply_player_ini(d);
    }
    {
        IniData d = ini_load(resolve_data_path("progression.ini").c_str());
        if (!d.sections.empty())
            apply_progression_ini(d);
    }
    {
        IniData d = ini_load(resolve_data_path("talents.ini").c_str());
        if (!d.sections.empty())
            apply_talents_ini(d);
    }
}

inline void load_town_dialogues(DialogueSystem& dlg) {
    dlg.clear_registry();
    IniData d = ini_load(resolve_data_path("town_dialogues.ini").c_str());
    register_dialogue_sequences_from_ini(dlg, d);
}

} // namespace TownConfigLoader

} // namespace mion
