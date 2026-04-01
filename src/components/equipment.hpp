#pragma once
#include <string>

namespace mion {

// ---------------------------------------------------------------------------
// Equipment slots v1: weapon, armor, accessory.
// ---------------------------------------------------------------------------
enum class EquipSlot { Weapon = 0, Armor, Accessory, Count };
inline constexpr int kEquipSlotCount = static_cast<int>(EquipSlot::Count);

// ---------------------------------------------------------------------------
// Modifiers applied to DerivedStats by an equipped item.
// All fields are additive bonuses; zero = no effect.
// ---------------------------------------------------------------------------
struct ItemModifiers {
    int   melee_damage  = 0;
    int   ranged_damage = 0;
    float spell_mult    = 0.0f;  // added to the final spell_damage_mult
    int   hp_bonus      = 0;
    float stamina_bonus = 0.0f;
    float mana_bonus    = 0.0f;
};

// ---------------------------------------------------------------------------
// Item definition (v1: no rarity, no tier).
// ---------------------------------------------------------------------------
struct ItemDef {
    std::string    name;
    EquipSlot      slot      = EquipSlot::Weapon;
    ItemModifiers  modifiers{};
    bool           valid     = false; // false = empty slot
};

// ---------------------------------------------------------------------------
// Player equipment state: one slot per type.
// Exposes helpers to compute the total modifier sum across all slots.
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

    // Sum of modifiers from all equipped slots.
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
