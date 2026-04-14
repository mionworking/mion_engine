#pragma once

namespace mion {

// Strongly-typed dungeon dialogue identifiers used by gameplay logic.
// Strings are confined to the registry/serialization boundary in
// dungeon_dialogue.hpp via to_string(...). Lookups in code paths
// (boss state, enemy death, world scene) should use the enum directly.
enum class DungeonDialogueId {
    Prologue,
    Room2,
    Deeper,
    RareRelic,
    MinibossDeath,
    BossPhase2,
};

inline constexpr const char* to_string(DungeonDialogueId id) {
    switch (id) {
        case DungeonDialogueId::Prologue:      return "dungeon_prologue";
        case DungeonDialogueId::Room2:         return "dungeon_room2";
        case DungeonDialogueId::Deeper:        return "dungeon_deeper";
        case DungeonDialogueId::RareRelic:     return "dungeon_rare_relic";
        case DungeonDialogueId::MinibossDeath: return "miniboss_grimjaw_death";
        case DungeonDialogueId::BossPhase2:    return "boss_grimjaw_phase2";
    }
    return "";
}

} // namespace mion
