#pragma once

#include <string>
#include <unordered_map>

#include "ini_loader.hpp"

namespace mion {

struct LocaleSystem {
    std::unordered_map<std::string, std::string> strings;

    void clear() { strings.clear(); }

    void load(const std::string& path) {
        clear();
        IniData d = ini_load(path);
        auto    si = d.sections.find("strings");
        if (si == d.sections.end()) return;
        for (const auto& [k, v] : si->second)
            strings[k] = v;
    }

    const char* get(const std::string& key) const {
        auto it = strings.find(key);
        return (it != strings.end()) ? it->second.c_str() : key.c_str();
    }
};

// Instance owned by main()'s stack; pointed to after locale_bind().
namespace detail { inline LocaleSystem* active_locale = nullptr; }

inline void locale_bind(LocaleSystem* ls) { detail::active_locale = ls; }

inline const char* L(const std::string& key) {
    return detail::active_locale ? detail::active_locale->get(key) : key.c_str();
}

} // namespace mion
