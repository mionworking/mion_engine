#pragma once
#include "../components/attributes.hpp"
#include "../components/equipment.hpp"
#include "../components/item_bag.hpp"
#include "../components/mana.hpp"
#include "../components/potion_quickslot.hpp"
#include "../components/progression.hpp"
#include "../components/spell_book.hpp"
#include "../components/stamina.hpp"
#include "../components/talent_state.hpp"

namespace mion {

// Dados exclusivos do player — spell book, talentos, equipamento, stamina, mana,
// progressão, bag, poção, dash, buffs temporários.
// Instanciado como std::optional<PlayerData> dentro de Actor; inimigos deixam vazio.
struct PlayerData {
    EquipmentState  equipment;  // slots de equipamento (11)
    ItemBag         bag;        // itens acumulados
    PotionQuickslot potion;     // quickslot de poção
    SpellBookState  spell_book; // feitiços + cooldowns
    TalentState     talents;    // árvore de talentos
    ProgressionState progression; // XP/nível — só player
    AttributesState  attributes;  // cinco atributos base
    StaminaState     stamina;   // pool de stamina
    ManaState        mana;      // pool de mana
    int              gold = 0;
};

} // namespace mion
