#pragma once
#include <algorithm>
#include <array>
#include <string>

#include "../core/ini_loader.hpp"

namespace mion {

enum class SpellId {
    FrostBolt = 0,
    Nova,
    ChainLightning,
    MultiShot,
    PoisonArrow,
    Strafe,
    Cleave,
    BattleCry,
    Count
};

struct SpellDef {
    SpellId id               = SpellId::FrostBolt;
    float   mana_cost        = 0.0f;
    float   cooldown_seconds = 0.0f;
    int     damage           = 0;
    float   speed            = 0.0f;
    float   radius           = 0.0f;
    int     damage_per_level = 0;
    float   cooldown_per_level = 0.0f;

    int effective_damage(int talent_rank) const {
        int r = std::max(0, talent_rank);
        return damage + damage_per_level * r;
    }

    float effective_cooldown(int talent_rank) const {
        int   r = std::max(0, talent_rank);
        float c = cooldown_seconds - cooldown_per_level * (float)r;
        return c < 0.05f ? 0.05f : c;
    }
};

inline constexpr int kSpellCount = static_cast<int>(SpellId::Count);

inline std::array<SpellDef, kSpellCount> g_spell_defs = {{
    { SpellId::FrostBolt, 18.0f, 0.45f, 16, 460.0f, 0.0f, 0, 0.0f },
    { SpellId::Nova, 30.0f, 1.20f, 20, 0.0f, 96.0f, 0, 0.0f },
    { SpellId::ChainLightning, 22.0f, 1.0f, 14, 0.0f, 0.0f, 4, 0.05f },
    { SpellId::MultiShot, 0.0f, 0.5f, 0, 420.0f, 0.0f, 0, 0.0f },
    { SpellId::PoisonArrow, 0.0f, 0.5f, 0, 420.0f, 0.0f, 0, 0.0f },
    { SpellId::Strafe, 12.0f, 0.9f, 0, 400.0f, 0.0f, 0, 0.04f },
    { SpellId::Cleave, 0.0f, 0.85f, 14, 0.0f, 88.0f, 3, 0.06f },
    { SpellId::BattleCry, 0.0f, 12.0f, 0, 0.0f, 0.0f, 0, 0.0f },
}};

inline const SpellDef& spell_def(SpellId id) {
    return g_spell_defs[static_cast<int>(id)];
}

inline void apply_spell_ini_section(const IniData& d, const std::string& sec, SpellDef& def) {
    def.mana_cost = d.get_float(sec, "mana_cost", def.mana_cost);
    def.cooldown_seconds =
        d.get_float(sec, "cooldown_seconds", def.cooldown_seconds);
    def.damage = d.get_int(sec, "damage", def.damage);
    def.speed  = d.get_float(sec, "speed", def.speed);
    def.radius = d.get_float(sec, "radius", def.radius);
    def.damage_per_level =
        d.get_int(sec, "damage_per_level", def.damage_per_level);
    def.cooldown_per_level =
        d.get_float(sec, "cooldown_per_level", def.cooldown_per_level);
}

} // namespace mion
