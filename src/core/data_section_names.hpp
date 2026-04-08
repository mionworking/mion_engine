#pragma once

#include <array>

namespace mion::data_sections {

inline constexpr const char* kDrops = "drops";

inline constexpr const char* kSpellFrostBolt = "frost_bolt";
inline constexpr const char* kSpellBolt = "bolt";
inline constexpr const char* kSpellNova = "nova";
inline constexpr const char* kSpellChainLightning = "chain_lightning";
inline constexpr const char* kSpellMultiShot = "multi_shot";
inline constexpr const char* kSpellPoisonArrow = "poison_arrow";
inline constexpr const char* kSpellStrafe = "strafe";
inline constexpr const char* kSpellCleave = "cleave";
inline constexpr const char* kSpellBattleCry = "battle_cry";

inline constexpr std::array<const char*, 7> kEnemyIniSections = {
    "skeleton",
    "orc",
    "ghost",
    "archer",
    "patrol_guard",
    "elite_skeleton",
    "boss_grimjaw",
};

} // namespace mion::data_sections
