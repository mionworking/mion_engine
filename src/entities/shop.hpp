#pragma once
#include <string>
#include <vector>

namespace mion {

// Types of purchasable items available in the town shop.
enum class ShopItemType { HpPotion, StaminaPotion, AttackUpgrade, ManaUpgrade };

struct ShopItem {
    std::string  display_name;
    ShopItemType type;
    int          gold_cost;
    int          value;
};

// The shop's current item listing with selection state.
struct ShopInventory {
    std::vector<ShopItem> items;
    int                   selected_index = 0;
};

} // namespace mion
