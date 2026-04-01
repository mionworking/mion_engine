#pragma once

#include <string>

#include "../core/ini_loader.hpp"
#include "talent_data.hpp"

namespace mion {

// Patches g_talent_nodes from data/talents.ini — one INI section per talent.
// Only fields present in the file are overwritten; missing keys keep their defaults.
inline void apply_talents_ini(const IniData& d) {
    struct Row {
        const char* section;
        TalentId    id;
    };
    static const Row rows[] = {
        { "melee_force", TalentId::MeleeForce },
        { "iron_body", TalentId::IronBody },
        { "crushing_blow", TalentId::CrushingBlow },
        { "cleave", TalentId::Cleave },
        { "whirlwind", TalentId::Whirlwind },
        { "battle_cry", TalentId::BattleCry },
        { "sharp_arrow", TalentId::SharpArrow },
        { "multi_shot", TalentId::MultiShot },
        { "poison_arrow", TalentId::PoisonArrow },
        { "piercing_shot", TalentId::PiercingShot },
        { "strafe", TalentId::Strafe },
        { "arcane_reservoir", TalentId::ArcaneReservoir },
        { "mana_flow", TalentId::ManaFlow },
        { "spell_power", TalentId::SpellPower },
        { "frost_bolt", TalentId::FrostBolt },
        { "nova", TalentId::Nova },
        { "chain_lightning", TalentId::ChainLightning },
    };
    for (const Row& row : rows) {
        const int         i = static_cast<int>(row.id);
        TalentNode&       n = g_talent_nodes[i];
        const std::string sec(row.section);
        n.cost_per_level   = d.get_int(sec, "cost_per_level", n.cost_per_level);
        n.max_level        = d.get_int(sec, "max_level", n.max_level);
        n.parent_min_level = d.get_int(sec, "parent_min_level", n.parent_min_level);
        std::string dn     = d.get_string(sec, "display_name", "");
        if (!dn.empty())
            g_talent_display_names[i] = std::move(dn);
    }
}

} // namespace mion
