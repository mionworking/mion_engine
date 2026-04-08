#pragma once

namespace mion {

// Canonical `Actor::name` for the player — must match values recorded in
// `CombatEvent::target_name` / `ProjectileSystem::HitEvent::target_name` when the player is hit.
namespace PlayerActorId {
    inline constexpr const char* kName = "player";
}

} // namespace mion
