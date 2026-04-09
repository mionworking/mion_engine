#pragma once

#include "../components/player_config.hpp"
#include "../components/progression.hpp"
#include "../components/talent_tree.hpp"
#include "../core/data_file_names.hpp"
#include "../core/data_ini_loader.hpp"

namespace mion::CommonPlayerProgressionLoader {

inline void load_defaults_and_ini_overrides() {
    reset_talent_tree_defaults();

    g_player_config     = make_player_config_from_ini(load_data_ini(data_files::kPlayer));
    g_progression_config = make_progression_config_from_ini(load_data_ini(data_files::kProgression));

    {
        IniData d = load_data_ini(data_files::kTalents);
        if (!d.sections.empty())
            apply_talents_ini(d);
    }
}

} // namespace mion::CommonPlayerProgressionLoader
