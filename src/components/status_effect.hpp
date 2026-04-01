#pragma once

namespace mion {

enum class StatusEffectType { None, Poison, Slow, Stun };

struct ActiveEffect {
    StatusEffectType type      = StatusEffectType::None;
    float            remaining = 0.0f;  // seconds remaining
    float            tick_timer = 0.0f;
    float            poison_interval_seconds = 0.55f;
    int              poison_damage_per_tick  = 3;
    float            slow_move_multiplier    = 0.52f;  // move_speed multiplier
};

struct StatusEffectState {
    static constexpr int kMaxSlots = 3;
    ActiveEffect slots[kMaxSlots] = {};

    bool has(StatusEffectType t) const {
        for (const auto& e : slots)
            if (e.type == t && e.remaining > 0.0f) return true;
        return false;
    }

    // Apply effect — replaces an existing slot of the same type or an empty slot.
    // If all slots are full, overwrites slot 0.
    void apply(StatusEffectType t, float duration) {
        apply_full(t, duration, 0.55f, 3, 0.52f);
    }

    void apply_full(StatusEffectType t, float duration,
                    float poison_interval, int poison_dmg, float slow_mult) {
        for (auto& e : slots) {
            if (e.type == t || e.type == StatusEffectType::None) {
                e.type                      = t;
                e.remaining                 = duration;
                e.tick_timer                = 0.0f;
                e.poison_interval_seconds   = poison_interval;
                e.poison_damage_per_tick    = poison_dmg;
                e.slow_move_multiplier      = slow_mult;
                return;
            }
        }
        slots[0] = { t, duration, 0.0f, poison_interval, poison_dmg, slow_mult };
    }

    float slow_multiplier() const {
        float m = 1.0f;
        for (const auto& e : slots) {
            if (e.type != StatusEffectType::Slow || e.remaining <= 0.0f) continue;
            if (e.slow_move_multiplier < m) m = e.slow_move_multiplier;
        }
        return m;
    }

    void clear_expired() {
        for (auto& e : slots)
            if (e.remaining <= 0.0f) e = ActiveEffect{};
    }
};

} // namespace mion
