#pragma once

#include <array>
#include <string>

namespace mion {

// Which class archetype a talent belongs to.
enum class Discipline { Melee, Ranged, Magic };

// Whether the talent grants a passive bonus or unlocks an active ability.
enum class SkillType { Passive, Active };

// Unique identifier for each node in the talent tree.
enum class TalentId {
    MeleeForce = 0,
    IronBody,
    CrushingBlow,
    Cleave,
    Whirlwind,
    BattleCry,
    SharpArrow,
    MultiShot,
    PoisonArrow,
    PiercingShot,
    Strafe,
    ArcaneReservoir,
    ManaFlow,
    SpellPower,
    FrostBolt,
    Nova,
    ChainLightning,
    Count
};

inline constexpr int kTalentCount = static_cast<int>(TalentId::Count);

// Static definition of a talent node: prerequisites, cost, and level cap.
struct TalentNode {
    TalentId    id               = TalentId::MeleeForce;
    Discipline  discipline       = Discipline::Melee;
    SkillType   skill_type       = SkillType::Passive;
    bool        has_parent       = false;
    TalentId    parent           = TalentId::MeleeForce; // required prerequisite (if has_parent)
    int         parent_min_level = 1;                    // minimum level needed in parent
    int         cost_per_level   = 1;
    int         max_level        = 3;
};

inline constexpr std::array<TalentNode, kTalentCount> kDefaultTalentNodes = {{
    { TalentId::MeleeForce, Discipline::Melee, SkillType::Passive, false, TalentId::MeleeForce, 1, 1,
      3 },
    { TalentId::IronBody, Discipline::Melee, SkillType::Passive, true, TalentId::MeleeForce, 1, 1,
      3 },
    { TalentId::CrushingBlow, Discipline::Melee, SkillType::Passive, true, TalentId::MeleeForce, 1,
      1, 3 },
    { TalentId::Cleave, Discipline::Melee, SkillType::Active, true, TalentId::MeleeForce, 1, 1, 3 },
    { TalentId::Whirlwind, Discipline::Melee, SkillType::Active, true, TalentId::Cleave, 1, 1, 3 },
    { TalentId::BattleCry, Discipline::Melee, SkillType::Active, true, TalentId::IronBody, 2, 1, 3 },
    { TalentId::SharpArrow, Discipline::Ranged, SkillType::Passive, false, TalentId::SharpArrow, 1,
      1, 3 },
    { TalentId::MultiShot, Discipline::Ranged, SkillType::Active, true, TalentId::SharpArrow, 1, 1,
      3 },
    { TalentId::PoisonArrow, Discipline::Ranged, SkillType::Active, true, TalentId::SharpArrow, 1,
      1, 3 },
    { TalentId::PiercingShot, Discipline::Ranged, SkillType::Passive, true, TalentId::MultiShot, 1,
      1, 3 },
    { TalentId::Strafe, Discipline::Ranged, SkillType::Active, true, TalentId::MultiShot, 2, 1, 3 },
    { TalentId::ArcaneReservoir, Discipline::Magic, SkillType::Passive, false,
      TalentId::ArcaneReservoir, 1, 1, 3 },
    { TalentId::ManaFlow, Discipline::Magic, SkillType::Passive, true, TalentId::ArcaneReservoir,
      1, 1, 3 },
    { TalentId::SpellPower, Discipline::Magic, SkillType::Passive, true, TalentId::ArcaneReservoir,
      1, 1, 3 },
    { TalentId::FrostBolt, Discipline::Magic, SkillType::Active, true, TalentId::SpellPower, 1, 1,
      3 },
    { TalentId::Nova, Discipline::Magic, SkillType::Active, true, TalentId::ArcaneReservoir, 2, 1,
      3 },
    { TalentId::ChainLightning, Discipline::Magic, SkillType::Active, true, TalentId::SpellPower, 2,
      1, 3 },
}};

// Display names shown in the talent tree UI.
inline const std::array<std::string, kTalentCount> kDefaultTalentDisplayNames = {
    "Melee Force",     "Iron Body",     "Crushing Blow",    "Cleave",
    "Whirlwind",       "Battle Cry",    "Sharp Arrow",      "Multi Shot",
    "Poison Arrow",    "Piercing Shot", "Strafe",           "Arcane Reservoir",
    "Mana Flow",       "Spell Power",   "Frost Bolt",       "Nova",
    "Chain Lightning",
};

// Global talent tables — read-only após bootstrap. Escrita apenas em init (apply_talents_ini).
namespace detail {
    inline std::array<TalentNode, kTalentCount> _g_talent_nodes_mutable = kDefaultTalentNodes;
    inline std::array<std::string, kTalentCount> _g_talent_display_names_mutable = kDefaultTalentDisplayNames;
}
inline const std::array<TalentNode, kTalentCount>&  g_talent_nodes         = detail::_g_talent_nodes_mutable;
inline const std::array<std::string, kTalentCount>& g_talent_display_names = detail::_g_talent_display_names_mutable;

// Restores both global tables to their compiled-in defaults.
inline void reset_talent_tree_defaults() {
    detail::_g_talent_nodes_mutable         = kDefaultTalentNodes;
    detail::_g_talent_display_names_mutable = kDefaultTalentDisplayNames;
}

// Accessor helpers — prefer these over direct array indexing.
inline const TalentNode& talent_def(TalentId id) {
    return g_talent_nodes[static_cast<int>(id)];
}

inline const std::string& talent_display_name(TalentId id) {
    return g_talent_display_names[static_cast<int>(id)];
}

} // namespace mion
