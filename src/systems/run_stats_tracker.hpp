#pragma once

#include <string>
#include <vector>

#include "../core/run_stats.hpp"
#include "melee_combat.hpp"
#include "projectile_system.hpp"

namespace mion {

namespace RunStatsTracker {

struct FrameStatsDelta {
    int   damage_taken   = 0;
    int   gold_collected = 0;
    int   spells_cast    = 0;
    float time_seconds   = 0.0f;
};

inline int damage_taken_for_actor(const std::string& actor_name,
                                  const std::vector<CombatEvent>& melee_events,
                                  const std::vector<ProjectileSystem::HitEvent>& projectile_events) {
    int total = 0;
    for (const auto& ev : melee_events) {
        if (ev.target_name == actor_name && ev.damage > 0)
            total += ev.damage;
    }
    for (const auto& ev : projectile_events) {
        if (ev.target_name == actor_name && ev.damage > 0)
            total += ev.damage;
    }
    return total;
}

inline void apply_frame_delta(RunStats* stats, const FrameStatsDelta& delta) {
    if (!stats) return;
    stats->damage_taken += delta.damage_taken;
    stats->gold_collected += delta.gold_collected;
    stats->spells_cast += delta.spells_cast;
    stats->time_seconds += delta.time_seconds;
}

} // namespace RunStatsTracker

} // namespace mion
