#pragma once
#include <string>
#include "equipment.hpp"

namespace mion {

// ---------------------------------------------------------------------------
// ItemBag — grid de itens acumulados (4×6 = 24 slots).
// Cada slot guarda o nome do item E o slot de equipamento correspondente,
// para que o controller saiba onde equipar sem precisar de um registry externo.
// Poções NÃO entram aqui — vão direto para PotionQuickslot.
// ---------------------------------------------------------------------------
inline constexpr int kBagCols = 4;
inline constexpr int kBagRows = 6;
inline constexpr int kBagSize = kBagCols * kBagRows; // 24

struct BagSlot {
    std::string name;
    EquipSlot   slot = EquipSlot::MainHand;

    bool is_empty() const { return name.empty(); }
    void clear()          { name.clear(); }
};

struct ItemBag {
    BagSlot slots[kBagSize];

    bool is_empty(int idx) const { return slots[idx].is_empty(); }
    bool is_full()         const { return first_empty() < 0; }

    int first_empty() const {
        for (int i = 0; i < kBagSize; ++i)
            if (slots[i].is_empty()) return i;
        return -1;
    }

    // Retorna false se bag cheia.
    bool add(const std::string& name, EquipSlot equip_slot) {
        int idx = first_empty();
        if (idx < 0) return false;
        slots[idx].name = name;
        slots[idx].slot = equip_slot;
        return true;
    }

    void remove(int idx) {
        if (idx >= 0 && idx < kBagSize) slots[idx].clear();
    }
};

} // namespace mion
