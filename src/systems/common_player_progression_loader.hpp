#pragma once

#include "../components/player_config.hpp"
#include "../components/progression.hpp"
#include "../components/talent_tree.hpp"
#include "../core/data_file_names.hpp"
#include "../core/data_ini_loader.hpp"

namespace mion::CommonPlayerProgressionLoader {

inline void load_defaults_and_ini_overrides() {
    reset_player_config_defaults();
    reset_progression_config_defaults();
    reset_talent_tree_defaults();

    {
        IniData d = load_data_ini(data_files::kPlayer);
        if (!d.sections.empty())
            apply_player_ini(d);
    }
    {
        IniData d = load_data_ini(data_files::kProgression);
        if (!d.sections.empty())
            apply_progression_ini(d);
    }
    {
        IniData d = load_data_ini(data_files::kTalents);
        if (!d.sections.empty())
            apply_talents_ini(d);
    }
}

} // namespace mion::CommonPlayerProgressionLoader
