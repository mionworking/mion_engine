#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <SDL3/SDL.h>
#include "ini_loader.hpp"

namespace mion {

struct ConfigData {
    int   window_width  = 1280;
    int   window_height = 720;
    float volume_master = 1.0f;
    bool  mute          = false;
    /// Indicador discreto no canto quando o jogo grava (mudança de sala / saída).
    bool  show_autosave_indicator = false;
};

namespace _config_detail {

inline bool file_exists(const std::string& path) {
    if (path.empty()) return false;
    if (FILE* f = std::fopen(path.c_str(), "r")) {
        std::fclose(f);
        return true;
    }
    return false;
}

inline std::string join_base_and_name(const char* base, const char* file_name) {
    if (!base || !*base || !file_name || !*file_name) return "";
    std::string out = base;
    const char tail = out.back();
    if (tail != '/' && tail != '\\') out.push_back('/');
    out += file_name;
    return out;
}

} // namespace _config_detail

// Resolve config.ini tentando:
// 1) ao lado do executável, se o arquivo existir
// 2) caminho de fallback (cwd por padrão), se existir
// 3) um caminho tentável não-vazio
inline std::string resolve_config_path(const char* base_path,
                                       const std::string& fallback_path = "config.ini",
                                       const char* file_name = "config.ini") {
    const std::string preferred = _config_detail::join_base_and_name(base_path, file_name);
    if (!preferred.empty() && _config_detail::file_exists(preferred)) return preferred;
    if (_config_detail::file_exists(fallback_path)) return fallback_path;
    return !preferred.empty() ? preferred : fallback_path;
}

// Localiza config.ini: tenta ao lado do executável, depois cwd.
inline std::string config_file_path() {
    return resolve_config_path(SDL_GetBasePath());
}

// Carrega config.ini. Se ausente, retorna defaults sem erro.
// path=nullptr → usa config_file_path() (SDL necessário); path explícito para testes.
inline ConfigData load_config(const char* path = nullptr) {
    ConfigData cfg{};
    std::string resolved = path ? path : config_file_path();
    IniData d = ini_load(resolved.c_str());
    cfg.window_width  = d.get_int  ("window", "width",         cfg.window_width);
    cfg.window_height = d.get_int  ("window", "height",        cfg.window_height);
    cfg.volume_master = d.get_float("audio",  "volume_master", cfg.volume_master);
    cfg.mute          = d.get_int  ("audio",  "mute",          (int)cfg.mute) != 0;
    cfg.show_autosave_indicator =
        d.get_int("ui", "autosave_indicator", (int)cfg.show_autosave_indicator) != 0;
    return cfg;
}

inline std::string load_ui_language(const char* path = nullptr) {
    std::string resolved = path ? path : config_file_path();
    IniData     d        = ini_load(resolved.c_str());
    return d.get_string("ui", "language", "en");
}

// Persiste secções de runtime no config.ini e preserva [keybinds], se existir.
inline bool save_runtime_config(const ConfigData& cfg,
                                const std::string& language,
                                const char* path = nullptr) {
    const std::string resolved = path ? path : config_file_path();
    IniData           prev     = ini_load(resolved.c_str());
    std::ofstream     out(resolved);
    if (!out) return false;

    out << "[window]\n";
    out << "width=" << cfg.window_width << "\n";
    out << "height=" << cfg.window_height << "\n\n";

    out << "[audio]\n";
    out << "volume_master=" << cfg.volume_master << "\n";
    out << "mute=" << (cfg.mute ? 1 : 0) << "\n\n";

    out << "[ui]\n";
    out << "autosave_indicator=" << (cfg.show_autosave_indicator ? 1 : 0) << "\n";
    out << "language=" << language << "\n\n";

    auto ks = prev.sections.find("keybinds");
    if (ks != prev.sections.end() && !ks->second.empty()) {
        out << "[keybinds]\n";
        static const char* k_order[] = {
            "attack", "attack_alt", "ranged", "dash", "parry",
            "spell_1", "spell_2", "spell_3", "spell_4",
            "confirm", "pause", "skill_tree", "erase_save",
            "upgrade_1", "upgrade_2", "upgrade_3",
            "talent_1", "talent_2", "talent_3",
        };
        for (const char* key : k_order) {
            auto it = ks->second.find(key);
            if (it != ks->second.end())
                out << key << "=" << it->second << "\n";
        }
    }

    return true;
}

} // namespace mion
