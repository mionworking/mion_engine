#pragma once
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <SDL3/SDL.h>

#include "save_data.hpp"
#include "save_migration.hpp"
#include "debug_log.hpp"

namespace mion {

namespace SaveSystem {

inline std::string default_path() {
    char* p = SDL_GetPrefPath("mion", "mion_engine");
    if (!p) return "mion_save.txt";
    std::string r = std::string(p) + "save.txt";
    SDL_free(p);
    return r;
}

inline std::string legacy_path() {
    return "mion_save.txt";
}

inline void trim_inplace(std::string& s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i > 0) s.erase(0, i);
}

inline bool parse_kv_file(const std::string& text, std::unordered_map<std::string, std::string>& out) {
    out.clear();
    std::istringstream iss(text);
    std::string        line;
    while (std::getline(iss, line)) {
        trim_inplace(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim_inplace(key);
        trim_inplace(val);
        if (!key.empty()) out[key] = val;
    }
    return true;
}

inline int get_int(const std::unordered_map<std::string, std::string>& m, const char* k, int def) {
    auto it = m.find(k);
    if (it == m.end()) return def;
    char* end = nullptr;
    long  v   = std::strtol(it->second.c_str(), &end, 10);
    if (end == it->second.c_str()) return def;
    return (int)v;
}

inline float get_float(const std::unordered_map<std::string, std::string>& m, const char* k, float def) {
    auto it = m.find(k);
    if (it == m.end()) return def;
    char* end = nullptr;
    float v   = std::strtof(it->second.c_str(), &end);
    if (end == it->second.c_str()) return def;
    return v;
}

inline bool get_bool01(const std::unordered_map<std::string, std::string>& m, const char* k, bool def) {
    return get_int(m, k, def ? 1 : 0) != 0;
}

inline bool exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

inline bool exists_candidates(const std::string& primary_path,
                              const std::string& fallback_legacy_path) {
    if (!primary_path.empty() && exists(primary_path)) return true;
    return !fallback_legacy_path.empty()
        && fallback_legacy_path != primary_path
        && exists(fallback_legacy_path);
}

inline bool remove_save(const std::string& path) {
    return ::remove(path.c_str()) == 0;
}

inline bool remove_candidates(const std::string& primary_path,
                              const std::string& fallback_legacy_path) {
    bool removed_any = false;
    if (!primary_path.empty() && exists(primary_path))
        removed_any = remove_save(primary_path) || removed_any;
    if (!fallback_legacy_path.empty()
        && fallback_legacy_path != primary_path
        && exists(fallback_legacy_path))
        removed_any = remove_save(fallback_legacy_path) || removed_any;
    return removed_any;
}

inline bool load(const std::string& path, SaveData& d) {
    std::ifstream f(path);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    std::unordered_map<std::string, std::string> kv;
    parse_kv_file(ss.str(), kv);

    int ver = get_int(kv, "version", 0);
    if (ver == 0) ver = 1;
    // Reject saves from unknown future versions to avoid silent corruption.
    if (ver < 1 || ver > kSaveFormatVersion) return false;

    d.version    = kSaveFormatVersion;

    // --- block: base state ---
    d.room_index = get_int(kv, "room_index", 0);
    d.player_hp  = get_int(kv, "hp", 100);
    d.gold       = get_int(kv, "gold", 0);

    // --- block: quests ---
    for (int qi = 0; qi < static_cast<int>(QuestId::Count); ++qi) {
        char qk[24];
        std::snprintf(qk, sizeof(qk), "quest_%d", qi);
        d.quest_state.status[qi] =
            QuestState::int_to_status(get_int(kv, qk, 0));
    }

    // --- block: progression ---
    d.progression.xp                      = get_int(kv, "xp", 0);
    d.progression.level                   = get_int(kv, "level", 1);
    d.progression.xp_to_next              = get_int(kv, "xp_to_next", 100);
    d.progression.pending_level_ups       = get_int(kv, "pending_level_ups", 0);
    d.progression.bonus_attack_damage     = get_int(kv, "bonus_attack_damage", 0);
    d.progression.bonus_max_hp            = get_int(kv, "bonus_max_hp", 0);
    d.progression.bonus_move_speed        = get_float(kv, "bonus_move_speed", 0.0f);
    d.progression.spell_damage_multiplier = get_float(kv, "spell_damage_multiplier", 1.0f);
    // Sanity: level must be >= 1.
    if (d.progression.level < 1) d.progression.level = 1;

    // --- block: talents ---
    d.talents.pending_points = get_int(kv, "talent_pending_points", 0);
    for (int i = 0; i < kTalentCount; ++i) {
        char key[32];
        std::snprintf(key, sizeof(key), "talent_%d", i);
        int lv = get_int(kv, key, 0);
        if (ver < 3)
            d.talents.levels[i] = lv != 0 ? 1 : 0;
        else
            d.talents.levels[i] = std::clamp(lv, 0, 3);
    }
    if (d.talents.pending_points < 0) d.talents.pending_points = 0;

    // --- block: mana ---
    d.mana.current               = get_float(kv, "mana_current", 100.0f);
    d.mana.max                   = get_float(kv, "mana_max", 100.0f);
    d.mana.regen_rate            = get_float(kv, "mana_regen_rate", 20.0f);
    d.mana.regen_delay           = get_float(kv, "mana_regen_delay", 0.75f);
    d.mana.regen_delay_remaining = get_float(kv, "mana_regen_delay_rem", 0.0f);
    if (d.mana.max <= 0.0f) d.mana.max = 100.0f;
    if (d.mana.current < 0.0f) d.mana.current = 0.0f;
    if (d.mana.current > d.mana.max) d.mana.current = d.mana.max;

    // --- block: stamina ---
    d.stamina.current               = get_float(kv, "stamina_current", 100.0f);
    d.stamina.max                   = get_float(kv, "stamina_max", 100.0f);
    d.stamina.regen_rate            = get_float(kv, "stamina_regen_rate", 30.0f);
    d.stamina.regen_delay           = get_float(kv, "stamina_regen_delay", 1.0f);
    d.stamina.regen_delay_remaining = get_float(kv, "stamina_regen_delay_rem", 0.0f);
    if (d.stamina.max <= 0.0f) d.stamina.max = 100.0f;
    if (d.stamina.current < 0.0f) d.stamina.current = 0.0f;
    if (d.stamina.current > d.stamina.max) d.stamina.current = d.stamina.max;

    // --- block: global flags ---
    d.victory_reached = get_bool01(kv, "victory_reached", false);
    d.difficulty      = get_int(kv, "difficulty", 1);
    if (d.difficulty < 0) d.difficulty = 0;
    if (d.difficulty > 2) d.difficulty = 2;

    // --- block: last run stats ---
    d.last_run_stats.rooms_cleared     = get_int(kv, "last_rooms_cleared", 0);
    d.last_run_stats.enemies_killed    = get_int(kv, "last_enemies_killed", 0);
    d.last_run_stats.gold_collected    = get_int(kv, "last_gold_collected", 0);
    d.last_run_stats.damage_taken      = get_int(kv, "last_damage_taken", 0);
    d.last_run_stats.spells_cast       = get_int(kv, "last_spells_cast", 0);
    d.last_run_stats.potions_used      = get_int(kv, "last_potions_used", 0);
    d.last_run_stats.time_seconds      = get_float(kv, "last_time_seconds", 0.0f);
    d.last_run_stats.max_level_reached = get_int(kv, "last_max_level", 1);
    d.last_run_stats.boss_defeated     = get_bool01(kv, "last_boss_defeated", false);

    // --- block: base attributes (v4+) ---
    // Note: these fields only exist in v4+ saves; for older versions,
    // migrate_v3_to_v4() will zero them out after this point.
    d.attributes.vigor        = get_int(kv, "attr_vigor",        0);
    d.attributes.forca        = get_int(kv, "attr_forca",        0);
    d.attributes.destreza     = get_int(kv, "attr_destreza",     0);
    d.attributes.inteligencia = get_int(kv, "attr_inteligencia", 0);
    d.attributes.endurance    = get_int(kv, "attr_endurance",    0);
    // Sanity: attributes cannot be negative.
    if (d.attributes.vigor        < 0) d.attributes.vigor        = 0;
    if (d.attributes.forca        < 0) d.attributes.forca        = 0;
    if (d.attributes.destreza     < 0) d.attributes.destreza     = 0;
    if (d.attributes.inteligencia < 0) d.attributes.inteligencia = 0;
    if (d.attributes.endurance    < 0) d.attributes.endurance    = 0;

    // --- block: pending attributes and scene flags (v5+) ---
    d.attr_points_available = get_int(kv, "attr_points_available", 0);
    if (d.attr_points_available < 0) d.attr_points_available = 0;
    if (d.attr_points_available > kSaveMaxAttrPoints) d.attr_points_available = kSaveMaxAttrPoints;
    d.scene_flags = static_cast<unsigned int>(get_int(kv, "scene_flags", 0));

    // --- migration chain ---
    SaveMigration::clamp_room_index(d);
    if (ver <= 1) d = SaveMigration::migrate_v1_to_v2(d);
    if (ver <= 2) d = SaveMigration::migrate_v2_to_v3(d);
    if (ver <= 3) d = SaveMigration::migrate_v3_to_v4(d);
    if (ver <= 4) d = SaveMigration::migrate_v4_to_v5(d);
    return true;
}

inline bool load_candidates(const std::string& primary_path,
                            const std::string& fallback_legacy_path,
                            SaveData& d) {
    if (!primary_path.empty() && load(primary_path, d)) return true;
    return !fallback_legacy_path.empty()
        && fallback_legacy_path != primary_path
        && load(fallback_legacy_path, d);
}

inline bool save(const std::string& path, const SaveData& d) {
    std::ofstream f(path, std::ios::trunc);
    if (!f) return false;

    // == header ==
    f << "version=" << kSaveFormatVersion << "\n";

    // == base state ==
    f << "room_index=" << d.room_index << "\n";
    f << "hp=" << d.player_hp << "\n";
    f << "gold=" << d.gold << "\n";

    // == quests ==
    for (int qi = 0; qi < static_cast<int>(QuestId::Count); ++qi)
        f << "quest_" << qi << "=" << QuestState::status_to_int(d.quest_state.status[qi])
          << "\n";

    // == progression ==
    f << "xp=" << d.progression.xp << "\n";
    f << "level=" << d.progression.level << "\n";
    f << "xp_to_next=" << d.progression.xp_to_next << "\n";
    f << "pending_level_ups=" << d.progression.pending_level_ups << "\n";
    f << "bonus_attack_damage=" << d.progression.bonus_attack_damage << "\n";
    f << "bonus_max_hp=" << d.progression.bonus_max_hp << "\n";
    f << "bonus_move_speed=" << d.progression.bonus_move_speed << "\n";
    f << "spell_damage_multiplier=" << d.progression.spell_damage_multiplier << "\n";

    // == talents ==
    f << "talent_pending_points=" << d.talents.pending_points << "\n";
    for (int i = 0; i < kTalentCount; ++i)
        f << "talent_" << i << "=" << d.talents.levels[i] << "\n";

    // == mana ==
    f << "mana_current=" << d.mana.current << "\n";
    f << "mana_max=" << d.mana.max << "\n";
    f << "mana_regen_rate=" << d.mana.regen_rate << "\n";
    f << "mana_regen_delay=" << d.mana.regen_delay << "\n";
    f << "mana_regen_delay_rem=" << d.mana.regen_delay_remaining << "\n";

    // == stamina ==
    f << "stamina_current=" << d.stamina.current << "\n";
    f << "stamina_max=" << d.stamina.max << "\n";
    f << "stamina_regen_rate=" << d.stamina.regen_rate << "\n";
    f << "stamina_regen_delay=" << d.stamina.regen_delay << "\n";
    f << "stamina_regen_delay_rem=" << d.stamina.regen_delay_remaining << "\n";

    // == global flags ==
    f << "victory_reached=" << (d.victory_reached ? 1 : 0) << "\n";
    f << "difficulty=" << d.difficulty << "\n";

    // == last run stats ==
    f << "last_rooms_cleared=" << d.last_run_stats.rooms_cleared << "\n";
    f << "last_enemies_killed=" << d.last_run_stats.enemies_killed << "\n";
    f << "last_gold_collected=" << d.last_run_stats.gold_collected << "\n";
    f << "last_damage_taken=" << d.last_run_stats.damage_taken << "\n";
    f << "last_spells_cast=" << d.last_run_stats.spells_cast << "\n";
    f << "last_potions_used=" << d.last_run_stats.potions_used << "\n";
    f << "last_time_seconds=" << d.last_run_stats.time_seconds << "\n";
    f << "last_max_level=" << d.last_run_stats.max_level_reached << "\n";
    f << "last_boss_defeated=" << (d.last_run_stats.boss_defeated ? 1 : 0) << "\n";

    // == base attributes (v4+) ==
    f << "attr_vigor="        << d.attributes.vigor        << "\n";
    f << "attr_forca="        << d.attributes.forca        << "\n";
    f << "attr_destreza="     << d.attributes.destreza     << "\n";
    f << "attr_inteligencia=" << d.attributes.inteligencia << "\n";
    f << "attr_endurance="    << d.attributes.endurance    << "\n";

    // == pending attributes and scene flags (v5+) ==
    f << "attr_points_available=" << d.attr_points_available << "\n";
    f << "scene_flags=" << d.scene_flags << "\n";

    return f.good();
}

inline std::string debug_path(const char* tag) {
    std::string base = default_path();
    // default_path typically ends with "save.txt" — replace it with "debug_<tag>.txt".
    const char* suffix = "save.txt";
    const size_t suf_len = std::char_traits<char>::length(suffix);
    if (base.size() >= suf_len) {
        size_t pos = base.rfind(suffix);
        if (pos != std::string::npos)
            base.replace(pos, suf_len, std::string("debug_") + tag + ".txt");
        else
            base += std::string(".debug_") + tag;
    } else {
        base += std::string(".debug_") + tag;
    }
    return base;
}

// Saves a debug snapshot alongside the main save (does not affect normal flow).
inline bool save_debug(const SaveData& d, const char* tag) {
    std::string safe_tag = tag ? tag : "snapshot";
    for (char& c : safe_tag) {
        if (c == '/' || c == '\\' || c == ' ') c = '_';
    }
    const std::string path = debug_path(safe_tag.c_str());
    const bool ok = save(path, d);
    if (ok)
        debug_log("SaveSystem::save_debug %s ok", path.c_str());
    else
        debug_log("SaveSystem::save_debug %s FAILED", path.c_str());
    return ok;
}

inline bool exists_default() {
    return exists_candidates(default_path(), legacy_path());
}

inline bool remove_default_saves() {
    return remove_candidates(default_path(), legacy_path());
}

inline bool load_default(SaveData& d) {
    return load_candidates(default_path(), legacy_path(), d);
}

inline bool save_default(const SaveData& d) {
    return save(default_path(), d);
}

} // namespace SaveSystem

} // namespace mion
