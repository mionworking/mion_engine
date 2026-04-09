#pragma once

#include <string>

#include "../core/ini_loader.hpp"
#include "talent_data.hpp"

namespace mion {

struct TalentData {
    std::array<TalentNode, kTalentCount>   nodes        = kDefaultTalentNodes;
    std::array<std::string, kTalentCount>  display_names = kDefaultTalentDisplayNames;
};

// Builds TalentData from INI overrides applied on top of `base`.
// Pure factory — does not touch any global. Missing INI keys keep base values.
inline TalentData make_talent_data_from_ini(
    const IniData& d,
    const TalentData& base = TalentData{}) {
    struct Row {
        const char* section;
        TalentId    id;
    };
    static const Row rows[] = {
        { "melee_force",      TalentId::MeleeForce },
        { "iron_body",        TalentId::IronBody },
        { "crushing_blow",    TalentId::CrushingBlow },
        { "cleave",           TalentId::Cleave },
        { "whirlwind",        TalentId::Whirlwind },
        { "battle_cry",       TalentId::BattleCry },
        { "sharp_arrow",      TalentId::SharpArrow },
        { "multi_shot",       TalentId::MultiShot },
        { "poison_arrow",     TalentId::PoisonArrow },
        { "piercing_shot",    TalentId::PiercingShot },
        { "strafe",           TalentId::Strafe },
        { "arcane_reservoir", TalentId::ArcaneReservoir },
        { "mana_flow",        TalentId::ManaFlow },
        { "spell_power",      TalentId::SpellPower },
        { "frost_bolt",       TalentId::FrostBolt },
        { "nova",             TalentId::Nova },
        { "chain_lightning",  TalentId::ChainLightning },
    };
    TalentData out = base;
    for (const Row& row : rows) {
        const int   i   = static_cast<int>(row.id);
        TalentNode& n   = out.nodes[i];
        const std::string sec(row.section);
        n.cost_per_level   = d.get_int(sec, "cost_per_level",   n.cost_per_level);
        n.max_level        = d.get_int(sec, "max_level",        n.max_level);
        n.parent_min_level = d.get_int(sec, "parent_min_level", n.parent_min_level);
        std::string dn     = d.get_string(sec, "display_name", "");
        if (!dn.empty())
            out.display_names[i] = std::move(dn);
    }
    return out;
}

// Convenience wrapper that writes the factory result into the global tables.
// Used by CommonPlayerProgressionLoader during bootstrap.
inline void apply_talents_ini(const IniData& d) {
    TalentData td = make_talent_data_from_ini(d);
    g_talent_nodes        = std::move(td.nodes);
    g_talent_display_names = std::move(td.display_names);
}

} // namespace mion
