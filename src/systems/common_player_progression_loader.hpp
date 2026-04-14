#pragma once

#include "../components/attributes.hpp"
#include "../components/player_config.hpp"
#include "../components/progression.hpp"
#include "../components/talent_tree.hpp"
#include "../core/data_file_names.hpp"
#include "../core/data_ini_loader.hpp"

namespace mion::CommonPlayerProgressionLoader {

inline void load_defaults_and_ini_overrides() {
    reset_talent_tree_defaults();

    detail::_g_player_config_mutable      = make_player_config_from_ini(load_data_ini(data_files::kPlayer));
    detail::_g_progression_config_mutable = make_progression_config_from_ini(load_data_ini(data_files::kProgression));
    detail::_g_attribute_scales_mutable   = make_attribute_scales_from_ini(load_data_ini(data_files::kAttributes));

    {
        IniData d = load_data_ini(data_files::kTalents);
        if (!d.sections.empty())
            apply_talents_ini(d);
    }
}

} // namespace mion::CommonPlayerProgressionLoader
