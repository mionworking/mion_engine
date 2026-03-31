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

// v2 -> v3: etapa explicita para manter cadeia de migracao continua.
inline SaveData migrate_v2_to_v3(SaveData data) {
    data.version = kSaveFormatVersion;
    if (data.room_index > kSaveMaxRoomIndex)
        data.room_index = kSaveMaxRoomIndex;
    if (data.room_index < 0)
        data.room_index = 0;
    return data;
}

// v3 -> v4: atributos zerados (novo jogador comeca sem pontos distribuidos)
inline SaveData migrate_v3_to_v4(SaveData data) {
    data.version    = kSaveFormatVersion;
    data.attributes = AttributesState{};
    return data;
}

// v4 -> v5: attr_points_available derivado de pending_level_ups;
// scene_flags calculado a partir do que ja esta no save.
inline SaveData migrate_v4_to_v5(SaveData data) {
    data.version               = kSaveFormatVersion;
    data.attr_points_available = data.progression.pending_level_ups;
    data.scene_flags           = 0;
    if (data.victory_reached)
        data.scene_flags |= 0x01u;
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
