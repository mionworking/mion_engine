#pragma once
#include <string>

namespace mion {

// ---------------------------------------------------------------------------
// Equipment slots v2: 11 slots (armadura, acessórios, armas).
// ---------------------------------------------------------------------------
enum class EquipSlot {
    // Armadura (5)
    Head = 0, Chest, Legs, Feet, Hands,
    // Acessórios (4)
    Belt, Amulet, RingLeft, RingRight,
    // Armas (2)
    MainHand, OffHand,
    Count
};
inline constexpr int kEquipSlotCount = static_cast<int>(EquipSlot::Count); // 11

inline const char* equip_slot_name(EquipSlot s) {
    switch (s) {
        case EquipSlot::Head:     return "Head";
        case EquipSlot::Chest:    return "Chest";
        case EquipSlot::Legs:     return "Legs";
        case EquipSlot::Feet:     return "Feet";
        case EquipSlot::Hands:    return "Hands";
        case EquipSlot::Belt:     return "Belt";
        case EquipSlot::Amulet:   return "Amulet";
        case EquipSlot::RingLeft: return "Ring L";
        case EquipSlot::RingRight:return "Ring R";
        case EquipSlot::MainHand: return "Main Hand";
        case EquipSlot::OffHand:  return "Off Hand";
        default:                  return "?";
    }
}

// ---------------------------------------------------------------------------
// Modifiers applied to DerivedStats by an equipped item.
// All fields are additive bonuses; zero = no effect.
// ---------------------------------------------------------------------------
struct ItemModifiers {
    int   melee_damage  = 0;
    int   ranged_damage = 0;
    float spell_mult    = 0.0f;
    int   hp_bonus      = 0;
    float stamina_bonus = 0.0f;
    float mana_bonus    = 0.0f;
};

// ---------------------------------------------------------------------------
// Item definition (data-driven: loaded from data/equipment.ini by name).
// ---------------------------------------------------------------------------
struct ItemDef {
    std::string    name;
    EquipSlot      slot      = EquipSlot::MainHand;
    ItemModifiers  modifiers{};
    bool           valid     = false; // false = empty slot
};

// ---------------------------------------------------------------------------
// Player equipment state: one slot per type.
// ---------------------------------------------------------------------------
struct EquipmentState {
    ItemDef slots[kEquipSlotCount]{};

    bool is_equipped(EquipSlot s) const {
        return slots[static_cast<int>(s)].valid;
    }

    void equip(EquipSlot s, const ItemDef& item) {
        slots[static_cast<int>(s)]       = item;
        slots[static_cast<int>(s)].valid = true;
    }

    void unequip(EquipSlot s) {
        slots[static_cast<int>(s)] = ItemDef{};
    }

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
