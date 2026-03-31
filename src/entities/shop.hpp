#pragma once
#include <string>
#include <vector>

namespace mion {

enum class ShopItemType { HpPotion, StaminaPotion, AttackUpgrade, ManaUpgrade };

struct ShopItem {
    std::string  display_name;
    ShopItemType type;
    int          gold_cost;
    int          value;
};

struct ShopInventory {
    std::vector<ShopItem> items;
    int                   selected_index = 0;
};

} // namespace mion
