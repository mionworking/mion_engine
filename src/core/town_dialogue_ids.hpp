#pragma once

namespace mion {

// Canonical dialogue sequence ids for town. Domain logic uses this enum;
// to_string() devolve a chave de conteúdo (deve casar com seções de
// town_dialogues.ini) e existe apenas como ponte para o registry de
// DialogueSystem (que ainda é endereçado por string).
enum class TownDialogueId {
    MiraDefault,
    MiraQuestOffer,
    MiraQuestActive,
    MiraQuestDone,
    ForgeGreeting,
};

inline constexpr const char* to_string(TownDialogueId id) {
    switch (id) {
        case TownDialogueId::MiraDefault:     return "mira_default";
        case TownDialogueId::MiraQuestOffer:  return "mira_quest_offer";
        case TownDialogueId::MiraQuestActive: return "mira_quest_active";
        case TownDialogueId::MiraQuestDone:   return "mira_quest_done";
        case TownDialogueId::ForgeGreeting:   return "forge_greeting";
    }
    return "";
}

} // namespace mion
