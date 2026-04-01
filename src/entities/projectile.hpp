#pragma once
#include "actor.hpp"

namespace mion {

struct Projectile {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float half_w = 5.0f;
    float half_h = 5.0f;
    float lifetime_remaining_seconds = 2.0f;
    int   damage                    = 8;
    Team  owner_team                = Team::Player;
    bool  active                    = true;

    // Slow on hit (FrostBolt). Field names aligned with the roadmap (`is_frost` / `frost_rank`).
    bool is_frost  = false;
    int  frost_rank = 0;

    // Poison on hit (PoisonArrow). `is_poison` + `poison_rank`.
    bool is_poison  = false;
    int  poison_rank = 0;
};

} // namespace mion
