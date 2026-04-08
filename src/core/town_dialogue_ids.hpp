#pragma once

namespace mion {

// Canonical dialogue sequence ids for town (content keys; must match town_dialogues.ini sections).
namespace TownDialogueId {
    inline constexpr const char* kMiraDefault     = "mira_default";
    inline constexpr const char* kMiraQuestOffer  = "mira_quest_offer";
    inline constexpr const char* kMiraQuestActive = "mira_quest_active";
    inline constexpr const char* kMiraQuestDone   = "mira_quest_done";
    inline constexpr const char* kForgeGreeting   = "forge_greeting";
} // namespace TownDialogueId

} // namespace mion
