#pragma once
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <SDL3/SDL.h>

#include "../components/attributes.hpp"
#include "../components/mana.hpp"
#include "../components/progression.hpp"
#include "../components/stamina.hpp"
#include "../components/talent_state.hpp"
#include "debug_log.hpp"
#include "quest_state.hpp"
#include "run_stats.hpp"

namespace mion {

/// Formato gravado em disco; v1/v2 carregam; talentos em v3 são níveis 0–3 por slot.
inline constexpr int kSaveFormatVersion  = 4;
inline constexpr int kSaveMaxRoomIndex   = 63;

struct SaveData {
    int              version = kSaveFormatVersion;
    int              room_index = 0;
    int              player_hp = 100;
    int              gold = 0;
    QuestState       quest_state{};
    ProgressionState progression{};
    TalentState      talents{};
    ManaState        mana{};
    StaminaState     stamina{};
    bool             victory_reached = false;
    int              difficulty      = 1; // 0 easy, 1 normal, 2 hard
    RunStats         last_run_stats{};
    AttributesState  attributes{};   // v4: Vigor/Forca/Destreza/Inteligencia/Endurance
};

namespace SaveSystem {

inline SaveData migrate_v1_to_v2(SaveData d) {
    d.version = kSaveFormatVersion;
    if (d.room_index > kSaveMaxRoomIndex) d.room_index = kSaveMaxRoomIndex;
    if (d.room_index < 0) d.room_index = 0;
    return d;
}

// v3 -> v4: atributos zerados (novo jogador começa sem pontos distribuídos)
inline SaveData migrate_v3_to_v4(SaveData d) {
    d.version    = kSaveFormatVersion;
    d.attributes = AttributesState{};
    return d;
}

inline void clamp_save_room_index(SaveData& d) {
    if (d.room_index > kSaveMaxRoomIndex) d.room_index = kSaveMaxRoomIndex;
    if (d.room_index < 0) d.room_index = 0;
}

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
    if (ver != 1 && ver != 2 && ver != 3 && ver != 4) return false;
    // #region agent log
    append_debug_log_line(
        "pre-fix",
        "H1_save_migration_chain_gap",
        "src/core/save_system.hpp:162",
        "Parsed save version",
        std::string("{\"version\":") + std::to_string(ver) + "}"
    );
    // #endregion

    d.version     = kSaveFormatVersion;
    d.room_index  = get_int(kv, "room_index", 0);
    d.player_hp   = get_int(kv, "hp", 100);
    d.gold        = get_int(kv, "gold", 0);
    for (int qi = 0; qi < static_cast<int>(QuestId::Count); ++qi) {
        char qk[24];
        std::snprintf(qk, sizeof(qk), "quest_%d", qi);
        d.quest_state.status[qi] =
            QuestState::int_to_status(get_int(kv, qk, 0));
    }
    d.progression.xp                 = get_int(kv, "xp", 0);
    d.progression.level              = get_int(kv, "level", 1);
    d.progression.xp_to_next         = get_int(kv, "xp_to_next", 100);
    d.progression.pending_level_ups  = get_int(kv, "pending_level_ups", 0);
    d.progression.bonus_attack_damage = get_int(kv, "bonus_attack_damage", 0);
    d.progression.bonus_max_hp        = get_int(kv, "bonus_max_hp", 0);
    d.progression.bonus_move_speed    = get_float(kv, "bonus_move_speed", 0.0f);
    d.progression.spell_damage_multiplier =
        get_float(kv, "spell_damage_multiplier", 1.0f);

    d.talents.pending_points = get_int(kv, "talent_pending_points", 0);
    int max_talent_lv_raw = 0;
    int max_talent_lv_out = 0;
    for (int i = 0; i < kTalentCount; ++i) {
        char key[32];
        std::snprintf(key, sizeof(key), "talent_%d", i);
        int lv = get_int(kv, key, 0);
        if (lv > max_talent_lv_raw) max_talent_lv_raw = lv;
        if (ver < 3)
            d.talents.levels[i] = lv != 0 ? 1 : 0;
        else
            d.talents.levels[i] = std::clamp(lv, 0, 3);
        if (d.talents.levels[i] > max_talent_lv_out) max_talent_lv_out = d.talents.levels[i];
    }
    // #region agent log
    append_debug_log_line(
        "pre-fix",
        "H2_talent_level_downgrade",
        "src/core/save_system.hpp:193",
        "Talent migration summary",
        std::string("{\"version\":") + std::to_string(ver)
        + ",\"maxRaw\":"
        + std::to_string(max_talent_lv_raw)
        + ",\"maxOut\":"
        + std::to_string(max_talent_lv_out)
        + "}"
    );
    // #endregion

    d.mana.current               = get_float(kv, "mana_current", 100.0f);
    d.mana.max                   = get_float(kv, "mana_max", 100.0f);
    d.mana.regen_rate            = get_float(kv, "mana_regen_rate", 20.0f);
    d.mana.regen_delay           = get_float(kv, "mana_regen_delay", 0.75f);
    d.mana.regen_delay_remaining = get_float(kv, "mana_regen_delay_rem", 0.0f);

    d.stamina.current               = get_float(kv, "stamina_current", 100.0f);
    d.stamina.max                   = get_float(kv, "stamina_max", 100.0f);
    d.stamina.regen_rate            = get_float(kv, "stamina_regen_rate", 30.0f);
    d.stamina.regen_delay           = get_float(kv, "stamina_regen_delay", 1.0f);
    d.stamina.regen_delay_remaining = get_float(kv, "stamina_regen_delay_rem", 0.0f);

    d.victory_reached = get_bool01(kv, "victory_reached", false);
    d.difficulty      = get_int(kv, "difficulty", 1);
    if (d.difficulty < 0) d.difficulty = 0;
    if (d.difficulty > 2) d.difficulty = 2;

    d.last_run_stats.rooms_cleared     = get_int(kv, "last_rooms_cleared", 0);
    d.last_run_stats.enemies_killed    = get_int(kv, "last_enemies_killed", 0);
    d.last_run_stats.gold_collected    = get_int(kv, "last_gold_collected", 0);
    d.last_run_stats.damage_taken      = get_int(kv, "last_damage_taken", 0);
    d.last_run_stats.spells_cast       = get_int(kv, "last_spells_cast", 0);
    d.last_run_stats.potions_used      = get_int(kv, "last_potions_used", 0);
    d.last_run_stats.time_seconds      = get_float(kv, "last_time_seconds", 0.0f);
    d.last_run_stats.max_level_reached = get_int(kv, "last_max_level", 1);
    d.last_run_stats.boss_defeated     = get_bool01(kv, "last_boss_defeated", false);

    // v4: atributos base do player
    d.attributes.vigor        = get_int(kv, "attr_vigor",        0);
    d.attributes.forca        = get_int(kv, "attr_forca",        0);
    d.attributes.destreza     = get_int(kv, "attr_destreza",     0);
    d.attributes.inteligencia = get_int(kv, "attr_inteligencia", 0);
    d.attributes.endurance    = get_int(kv, "attr_endurance",    0);

    clamp_save_room_index(d);
    if (ver <= 1) d = migrate_v1_to_v2(d);
    if (ver <= 3) d = migrate_v3_to_v4(d);
    // #region agent log
    append_debug_log_line(
        "pre-fix",
        "H1_save_migration_chain_gap",
        "src/core/save_system.hpp:231",
        "Applied migrations",
        std::string("{\"version\":") + std::to_string(ver)
        + ",\"ran_v1_to_v2\":"
        + (ver <= 1 ? "true" : "false")
        + ",\"ran_v3_to_v4\":"
        + (ver <= 3 ? "true" : "false")
        + ",\"attributes_vigor\":"
        + std::to_string(d.attributes.vigor)
        + "}"
    );
    // #endregion
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
    f << "version=" << kSaveFormatVersion << "\n";
    f << "room_index=" << d.room_index << "\n";
    f << "hp=" << d.player_hp << "\n";
    f << "gold=" << d.gold << "\n";
    for (int qi = 0; qi < static_cast<int>(QuestId::Count); ++qi)
        f << "quest_" << qi << "=" << QuestState::status_to_int(d.quest_state.status[qi])
          << "\n";
    f << "xp=" << d.progression.xp << "\n";
    f << "level=" << d.progression.level << "\n";
    f << "xp_to_next=" << d.progression.xp_to_next << "\n";
    f << "pending_level_ups=" << d.progression.pending_level_ups << "\n";
    f << "bonus_attack_damage=" << d.progression.bonus_attack_damage << "\n";
    f << "bonus_max_hp=" << d.progression.bonus_max_hp << "\n";
    f << "bonus_move_speed=" << d.progression.bonus_move_speed << "\n";
    f << "spell_damage_multiplier=" << d.progression.spell_damage_multiplier << "\n";
    f << "talent_pending_points=" << d.talents.pending_points << "\n";
    for (int i = 0; i < kTalentCount; ++i)
        f << "talent_" << i << "=" << d.talents.levels[i] << "\n";
    f << "mana_current=" << d.mana.current << "\n";
    f << "mana_max=" << d.mana.max << "\n";
    f << "mana_regen_rate=" << d.mana.regen_rate << "\n";
    f << "mana_regen_delay=" << d.mana.regen_delay << "\n";
    f << "mana_regen_delay_rem=" << d.mana.regen_delay_remaining << "\n";
    f << "stamina_current=" << d.stamina.current << "\n";
    f << "stamina_max=" << d.stamina.max << "\n";
    f << "stamina_regen_rate=" << d.stamina.regen_rate << "\n";
    f << "stamina_regen_delay=" << d.stamina.regen_delay << "\n";
    f << "stamina_regen_delay_rem=" << d.stamina.regen_delay_remaining << "\n";
    f << "victory_reached=" << (d.victory_reached ? 1 : 0) << "\n";
    f << "difficulty=" << d.difficulty << "\n";
    f << "last_rooms_cleared=" << d.last_run_stats.rooms_cleared << "\n";
    f << "last_enemies_killed=" << d.last_run_stats.enemies_killed << "\n";
    f << "last_gold_collected=" << d.last_run_stats.gold_collected << "\n";
    f << "last_damage_taken=" << d.last_run_stats.damage_taken << "\n";
    f << "last_spells_cast=" << d.last_run_stats.spells_cast << "\n";
    f << "last_potions_used=" << d.last_run_stats.potions_used << "\n";
    f << "last_time_seconds=" << d.last_run_stats.time_seconds << "\n";
    f << "last_max_level=" << d.last_run_stats.max_level_reached << "\n";
    f << "last_boss_defeated=" << (d.last_run_stats.boss_defeated ? 1 : 0) << "\n";
    // v4: atributos
    f << "attr_vigor="        << d.attributes.vigor        << "\n";
    f << "attr_forca="        << d.attributes.forca        << "\n";
    f << "attr_destreza="     << d.attributes.destreza     << "\n";
    f << "attr_inteligencia=" << d.attributes.inteligencia << "\n";
    f << "attr_endurance="    << d.attributes.endurance    << "\n";
    return f.good();
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
