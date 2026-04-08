#pragma once

namespace mion {

// Canonical strings for open-world room labels and door routing tokens.
//
// - `RoomDefinition::name` uses kTown / kCorridor (and dungeon rooms keep dynamic
//   `dungeon_rNN` names from `RoomManager`).
// - `DoorZone::target_scene_id` uses kDungeon / kTown for in-world handoff; these are
//   semantic destination tags, not necessarily equal to `SceneId` registry ids.
namespace WorldLayoutId {
    inline constexpr const char* kTown     = "town";
    inline constexpr const char* kCorridor = "corridor";
    inline constexpr const char* kDungeon  = "dungeon";
} // namespace WorldLayoutId

} // namespace mion
