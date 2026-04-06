#pragma once

#include <cstdint>
#include <string>
#include "../components/attributes.hpp"
#include "../components/equipment.hpp"
#include "../components/item_bag.hpp"
#include "../components/mana.hpp"
#include "../components/progression.hpp"
#include "../components/stamina.hpp"
#include "../components/talent_state.hpp"
#include "quest_state.hpp"
#include "run_stats.hpp"

namespace mion {

// Format written to disk.
// v1/v2 load; talent levels in v3 are 0–3 per slot;
// v4 adds base attributes; v5 adds pending attribute points and scene_flags.
inline constexpr int kSaveFormatVersion = 7;
inline constexpr int kSaveMaxRoomIndex  = 63;
inline constexpr int kSaveMaxAttrPoints = 999; // sanity cap for attr_points_available

struct SaveData {
    int              version = kSaveFormatVersion;
    int              room_index = 0;
    int              player_hp = 100;
    int              gold = 0;
    QuestState       quest_state{};
    ProgressionState progression{};
    TalentState      talents{};
    ManaState        mana{};
    StaminaState     stamina{};
    bool             victory_reached = false;
    int              difficulty      = 1; // 0=easy, 1=normal, 2=hard
    RunStats         last_run_stats{};
    AttributesState  attributes{};         // v4: all five base attributes
    int              attr_points_available = 0; // v5: unspent attribute points
    unsigned int     scene_flags = 0;      // v5: persistent scene flag bitmask
    // bit 0 = boss_dungeon1_defeated, bit 1 = dungeon2_unlocked,
    // bit 2 = blessing_altar_used, bit 3 = grimjaw_intro_played
    // bits 4-31 = reserved

    // v6: open world
    float        player_world_x    = 0.f;
    float        player_world_y    = 0.f;
    uint8_t      visited_area_mask = 0; // bit0=Room0, bit1=Room1, bit2=Room2, bit3=Boss

    // v7: equipment slots (11) + bag (24 slots).
    // Stored as item names; "" = empty slot.
    std::string  equipped_names[kEquipSlotCount];
    std::string  bag_names[kBagSize];               // item name; "" = vazio
    int          bag_equip_slots[kBagSize] = {};    // EquipSlot enum value

    // v7: potion quickslot
    int          potion_stack   = 0;
    int          potion_quality = 1; // PotionQuality enum value (default: Normal)
};

} // namespace mion
