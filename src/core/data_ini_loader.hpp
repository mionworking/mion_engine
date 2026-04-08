#pragma once

#include <string>

#include "engine_paths.hpp"
#include "ini_loader.hpp"

namespace mion {

inline IniData load_data_ini(const std::string& relative_path) {
    return ini_load(resolve_data_path(relative_path).c_str());
}

} // namespace mion
