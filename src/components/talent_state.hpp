#pragma once
#include "talent_tree.hpp"

namespace mion {

struct TalentState {
    int pending_points        = 0;
    int levels[kTalentCount] = {};

    int level_of(TalentId id) const { return levels[static_cast<int>(id)]; }

    bool is_active(TalentId id) const { return level_of(id) > 0; }

    bool is_unlocked(TalentId id) const { return is_active(id); }

    bool can_spend(TalentId id) const {
        const TalentNode& node = talent_def(id);
        if (pending_points < node.cost_per_level) return false;
        if (level_of(id) >= node.max_level) return false;
        if (!node.has_parent) return true;
        return level_of(node.parent) >= node.parent_min_level;
    }

    bool has_spendable_options() const {
        for (int i = 0; i < kTalentCount; ++i) {
            if (can_spend(static_cast<TalentId>(i))) return true;
        }
        return false;
    }

    bool try_spend(TalentId id) {
        if (!can_spend(id)) return false;
        const TalentNode& node = talent_def(id);
        levels[static_cast<int>(id)] += 1;
        pending_points -= node.cost_per_level;
        return true;
    }

    // Compatibility aliases — kept for legacy test code.
    bool can_unlock(TalentId id) const { return can_spend(id); }

    bool try_unlock(TalentId id) { return try_spend(id); }

    bool has_unlockable_options() const { return has_spendable_options(); }
};

} // namespace mion
