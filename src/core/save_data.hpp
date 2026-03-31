#pragma once

#include "../components/attributes.hpp"
#include "../components/mana.hpp"
#include "../components/progression.hpp"
#include "../components/stamina.hpp"
#include "../components/talent_state.hpp"
#include "quest_state.hpp"
#include "run_stats.hpp"

namespace mion {

/// Formato gravado em disco.
/// v1/v2 carregam; talentos em v3 são níveis 0–3 por slot;
/// v4 adiciona atributos base; v5 adiciona pontos de atributo pendentes e scene_flags.
inline constexpr int kSaveFormatVersion = 5;
inline constexpr int kSaveMaxRoomIndex  = 63;
inline constexpr int kSaveMaxAttrPoints = 999; // teto de sanidade para attr_points_available

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
    AttributesState  attributes{};         // v4: Vigor/Forca/Destreza/Inteligencia/Endurance
    int              attr_points_available = 0; // v5: pontos de atributo nao distribuidos
    unsigned int     scene_flags = 0;      // v5: bitmask de flags persistentes de cena
    // bit 0 = boss_dungeon1_defeated, bit 1 = dungeon2_unlocked,
    // bit 2 = blessing_altar_used, bit 3 = grimjaw_intro_played
    // bits 4-31 = reservados
};

} // namespace mion
