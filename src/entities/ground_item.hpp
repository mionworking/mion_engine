#pragma once

namespace mion {

enum class GroundItemType { Health, Damage, Speed, Gold };

struct GroundItem {
    float          x = 0.0f;
    float          y = 0.0f;
    GroundItemType type = GroundItemType::Health;
    int            gold_value = 0;
    bool           active = true;
    /// Item de narrativa — ao apanhar dispara diálogo (uma vez por pickup).
    bool           lore_pickup = false;
};

} // namespace mion
