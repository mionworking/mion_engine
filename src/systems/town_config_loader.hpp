#pragma once

#include "../core/data_file_names.hpp"
#include "../core/data_ini_loader.hpp"
#include "../core/dialogue_loader.hpp"
#include "common_player_progression_loader.hpp"

namespace mion {

namespace TownConfigLoader {

inline void load_town_player_and_progression_config() {
    CommonPlayerProgressionLoader::load_defaults_and_ini_overrides();
}

inline void load_town_dialogues(DialogueSystem& dlg) {
    dlg.clear_registry();
    IniData d = load_data_ini(data_files::kTownDialogues);
    register_dialogue_sequences_from_ini(dlg, d);
}

} // namespace TownConfigLoader

} // namespace mion
