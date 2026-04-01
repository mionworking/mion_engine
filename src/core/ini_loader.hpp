#pragma once
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mion {

// Result of parsing an INI file: section → (key → value string) map.
struct IniData {
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> sections;

    int          get_int   (const std::string& sec, const std::string& key, int def) const;
    float        get_float (const std::string& sec, const std::string& key, float def) const;
    std::string  get_string(const std::string& sec, const std::string& key,
                             const std::string& def) const;

    // Sections whose name starts with `prefix` + `.` (e.g. prefix `arena.obstacle` → `arena.obstacle.0`, …).
    std::vector<std::string> sections_with_prefix(const std::string& prefix) const;
};

namespace _ini_detail {
inline void trim(std::string& s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}
} // namespace _ini_detail

// Loads an INI file. Returns an empty IniData if the file does not exist.
inline IniData ini_load(const std::string& path) {
    IniData     data;
    std::ifstream f(path);
    if (!f) return data;

    std::string section;
    std::string line;
    while (std::getline(f, line)) {
        _ini_detail::trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        if (line[0] == '[') {
            size_t end = line.find(']');
            if (end != std::string::npos)
                section = line.substr(1, end - 1);
            continue;
        }
        if (section.empty()) continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        _ini_detail::trim(key);
        _ini_detail::trim(val);
        if (!key.empty()) data.sections[section][key] = std::move(val);
    }
    return data;
}

inline int IniData::get_int(const std::string& sec, const std::string& key, int def) const {
    auto si = sections.find(sec);
    if (si == sections.end()) return def;
    auto ki = si->second.find(key);
    if (ki == si->second.end()) return def;
    char* end = nullptr;
    long  v   = std::strtol(ki->second.c_str(), &end, 10);
    return (end == ki->second.c_str()) ? def : (int)v;
}

inline float IniData::get_float(const std::string& sec, const std::string& key, float def) const {
    auto si = sections.find(sec);
    if (si == sections.end()) return def;
    auto ki = si->second.find(key);
    if (ki == si->second.end()) return def;
    char* end = nullptr;
    float v   = std::strtof(ki->second.c_str(), &end);
    return (end == ki->second.c_str()) ? def : v;
}

inline std::string IniData::get_string(const std::string& sec, const std::string& key,
                                       const std::string& def) const {
    auto si = sections.find(sec);
    if (si == sections.end()) return def;
    auto ki = si->second.find(key);
    if (ki == si->second.end()) return def;
    return ki->second;
}

inline std::vector<std::string> IniData::sections_with_prefix(const std::string& prefix) const {
    std::vector<std::string> out;
    if (prefix.empty()) return out;
    const std::string pref_dot = prefix + ".";
    for (const auto& kv : sections) {
        const std::string& name = kv.first;
        if (name.size() <= pref_dot.size()) continue;
        if (name.compare(0, pref_dot.size(), pref_dot) != 0) continue;
        out.push_back(name);
    }
    std::sort(out.begin(), out.end());
    return out;
}

} // namespace mion
