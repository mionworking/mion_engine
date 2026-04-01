#pragma once
#include <algorithm>

#include "mana.hpp"
#include "spell_defs.hpp"
#include "talent_state.hpp"

namespace mion {

// Resolves the effective talent rank used for a spell's damage and cooldown scaling.
// Takes the highest relevant talent level so synergy nodes are always considered.
inline int spell_damage_rank(SpellId sid, const TalentState& t) {
    switch (sid) {
    case SpellId::FrostBolt:
        return std::max(t.level_of(TalentId::FrostBolt), t.level_of(TalentId::SpellPower));
    case SpellId::Nova:
        return std::max(t.level_of(TalentId::Nova), t.level_of(TalentId::ArcaneReservoir));
    case SpellId::ChainLightning:
        return std::max(t.level_of(TalentId::ChainLightning), t.level_of(TalentId::SpellPower));
    case SpellId::MultiShot:
        return t.level_of(TalentId::SharpArrow) + t.level_of(TalentId::MultiShot);
    case SpellId::PoisonArrow:
        return t.level_of(TalentId::SharpArrow) + t.level_of(TalentId::PoisonArrow);
    case SpellId::Strafe:
        return t.level_of(TalentId::Strafe) + t.level_of(TalentId::SharpArrow);
    case SpellId::Cleave:
        return t.level_of(TalentId::Cleave) + t.level_of(TalentId::MeleeForce);
    case SpellId::BattleCry:
        return std::max(t.level_of(TalentId::BattleCry), t.level_of(TalentId::IronBody));
    default:
        return 0;
    }
}

// Tracks per-spell cooldowns and unlock state.
// Call sync_from_talents() whenever the talent tree changes.
struct SpellBookState {
    float cooldown_remaining[kSpellCount] = {};
    bool  unlocked[kSpellCount]           = {};

    void tick(float dt) {
        for (int i = 0; i < kSpellCount; ++i) {
            if (cooldown_remaining[i] > 0.0f) {
                cooldown_remaining[i] -= dt;
                if (cooldown_remaining[i] < 0.0f) cooldown_remaining[i] = 0.0f;
            }
        }
    }

    bool is_unlocked(SpellId id) const {
        return unlocked[static_cast<int>(id)];
    }

    bool can_cast(SpellId id, const ManaState& mana) const {
        const int         idx = static_cast<int>(id);
        const SpellDef&   def = spell_def(id);
        return unlocked[idx] && cooldown_remaining[idx] <= 0.0f && mana.can_afford(def.mana_cost);
    }

    void start_cooldown(SpellId id, const TalentState& talents) {
        const int idx = static_cast<int>(id);
        const int r   = spell_damage_rank(id, talents);
        cooldown_remaining[idx] = spell_def(id).effective_cooldown(r);
    }

    void unlock(SpellId id) {
        unlocked[static_cast<int>(id)] = true;
    }

    // Derives the unlocked set directly from the talent tree state.
    void sync_from_talents(const TalentState& t) {
        auto ge = [&](TalentId id, int min_lv) { return t.level_of(id) >= min_lv; };
        unlocked[static_cast<int>(SpellId::FrostBolt)]       = ge(TalentId::SpellPower, 1);
        unlocked[static_cast<int>(SpellId::Nova)]              = ge(TalentId::ArcaneReservoir, 2);
        unlocked[static_cast<int>(SpellId::ChainLightning)]  = ge(TalentId::SpellPower, 2);
        unlocked[static_cast<int>(SpellId::MultiShot)]         = ge(TalentId::SharpArrow, 1);
        unlocked[static_cast<int>(SpellId::PoisonArrow)]       = ge(TalentId::SharpArrow, 1);
        unlocked[static_cast<int>(SpellId::Strafe)]            = ge(TalentId::MultiShot, 2);
        unlocked[static_cast<int>(SpellId::Cleave)]            = ge(TalentId::MeleeForce, 1);
        unlocked[static_cast<int>(SpellId::BattleCry)]         = ge(TalentId::IronBody, 2);
    }
};

} // namespace mion
