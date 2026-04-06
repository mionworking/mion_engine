#pragma once

#include "save_data.hpp"

namespace mion {
namespace SaveMigration {

inline SaveData migrate_v1_to_v2(SaveData data) {
    data.version = kSaveFormatVersion;
    if (data.room_index > kSaveMaxRoomIndex)
        data.room_index = kSaveMaxRoomIndex;
    if (data.room_index < 0)
        data.room_index = 0;
    return data;
}

// v2 → v3: explicit step to keep the migration chain unbroken.
inline SaveData migrate_v2_to_v3(SaveData data) {
    data.version = kSaveFormatVersion;
    if (data.room_index > kSaveMaxRoomIndex)
        data.room_index = kSaveMaxRoomIndex;
    if (data.room_index < 0)
        data.room_index = 0;
    return data;
}

// v3 → v4: attributes zeroed out (new player starts with no points spent).
inline SaveData migrate_v3_to_v4(SaveData data) {
    data.version    = kSaveFormatVersion;
    data.attributes = AttributesState{};
    return data;
}

// v4 → v5: attr_points_available derived from pending_level_ups;
// scene_flags computed from existing save state.
inline SaveData migrate_v4_to_v5(SaveData data) {
    data.version               = kSaveFormatVersion;
    data.attr_points_available = data.progression.pending_level_ups;
    data.scene_flags           = 0;
    if (data.victory_reached)
        data.scene_flags |= 0x01u;
    return data;
}

// v5 → v6: open world — add player world position and visited area mask.
inline SaveData migrate_v5_to_v6(SaveData data) {
    data.version            = kSaveFormatVersion;
    data.player_world_x     = 0.f;
    data.player_world_y     = 0.f;
    data.visited_area_mask  = 0;
    return data;
}

// v6 → v7: equipment expanded to 11 slots.
// Old slots (Weapon=0, Armor=1, Accessory=2) map to new enum positions:
//   Weapon    → MainHand (index 9)
//   Armor     → Chest    (index 1)
//   Accessory → Amulet   (index 6)
// All remaining slots start empty ("").
inline SaveData migrate_v6_to_v7(SaveData data) {
    data.version = kSaveFormatVersion;
    // All slots default to "" (empty) — the array is already zero-initialized.
    // No old equipment names were persisted in v6, so nothing to remap.
    return data;
}

inline void clamp_room_index(SaveData& data) {
    if (data.room_index > kSaveMaxRoomIndex)
        data.room_index = kSaveMaxRoomIndex;
    if (data.room_index < 0)
        data.room_index = 0;
}

} // namespace SaveMigration
} // namespace mion
