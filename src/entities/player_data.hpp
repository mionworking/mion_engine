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

    // Dash / roll
    float dash_active_remaining_seconds  = 0.0f;
    float dash_cooldown_remaining_seconds = 0.0f;
    float dash_dir_x                     = 1.0f;
    float dash_dir_y                     = 0.0f;
    float dash_speed                     = 520.0f;
    float dash_duration_seconds          = 0.18f;
    float dash_iframes_seconds           = 0.20f;
    float dash_cooldown_seconds          = 0.55f;

    // Battle Cry e outros buffs temporários
    float empowered_damage_multiplier   = 1.0f;
    float empowered_remaining_seconds   = 0.0f;

    // Ranged cooldown
    float ranged_cooldown_remaining_seconds = 0.0f;
    float ranged_cooldown_seconds           = 0.35f;
};

} // namespace mion
