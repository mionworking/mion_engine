#pragma once
#include <string>

namespace mion {

// ---------------------------------------------------------------------------
// Slots de equipamento v1: arma, armadura, acessório.
// ---------------------------------------------------------------------------
enum class EquipSlot { Weapon = 0, Armor, Accessory, Count };
inline constexpr int kEquipSlotCount = static_cast<int>(EquipSlot::Count);

// ---------------------------------------------------------------------------
// Modificadores que um item equipado aplica nos DerivedStats.
// Todos os campos são bonuses aditivos; zero = sem efeito.
// ---------------------------------------------------------------------------
struct ItemModifiers {
    int   melee_damage  = 0;
    int   ranged_damage = 0;
    float spell_mult    = 0.0f;  // somado ao spell_damage_mult final
    int   hp_bonus      = 0;
    float stamina_bonus = 0.0f;
    float mana_bonus    = 0.0f;
};

// ---------------------------------------------------------------------------
// Definição de um item (v1 simples: sem raridade, sem tier).
// ---------------------------------------------------------------------------
struct ItemDef {
    std::string    name;
    EquipSlot      slot      = EquipSlot::Weapon;
    ItemModifiers  modifiers{};
    bool           valid     = false; // false = slot vazio
};

// ---------------------------------------------------------------------------
// Estado de equipamento do player: um slot por tipo.
// Expõe helpers para calcular a soma total dos modificadores.
// ---------------------------------------------------------------------------
struct EquipmentState {
    ItemDef slots[kEquipSlotCount]{};

    bool is_equipped(EquipSlot s) const {
        return slots[static_cast<int>(s)].valid;
    }

    void equip(EquipSlot s, const ItemDef& item) {
        slots[static_cast<int>(s)] = item;
        slots[static_cast<int>(s)].valid = true;
    }

    void unequip(EquipSlot s) {
        slots[static_cast<int>(s)] = ItemDef{};
    }

    // Soma dos modificadores de todos os slots equipados.
    ItemModifiers total_modifiers() const {
        ItemModifiers total{};
        for (int i = 0; i < kEquipSlotCount; ++i) {
            if (!slots[i].valid) continue;
            const ItemModifiers& m = slots[i].modifiers;
            total.melee_damage  += m.melee_damage;
            total.ranged_damage += m.ranged_damage;
            total.spell_mult    += m.spell_mult;
            total.hp_bonus      += m.hp_bonus;
            total.stamina_bonus += m.stamina_bonus;
            total.mana_bonus    += m.mana_bonus;
        }
        return total;
    }
};

} // namespace mion
