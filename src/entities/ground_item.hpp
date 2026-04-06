#pragma once
#include <string>
#include "../components/equipment.hpp"
#include "../components/potion_quickslot.hpp"

namespace mion {

enum class GroundItemType { Health, Damage, Speed, Gold, Equipment, Potion };

struct GroundItem {
    float          x = 0.0f;
    float          y = 0.0f;
    GroundItemType type = GroundItemType::Health;
    int            gold_value = 0;
    bool           active = true;
    // Lore item — triggers a dialogue on pickup (fires once per pickup).
    bool           lore_pickup = false;
    // Equipment item — only valid when type == GroundItemType::Equipment.
    std::string    item_name;
    EquipSlot      item_slot     = EquipSlot::MainHand;
    // Potion item — only valid when type == GroundItemType::Potion.
    PotionQuality  potion_quality = PotionQuality::Normal;
    int            potion_amount  = 1;
};

} // namespace mion
