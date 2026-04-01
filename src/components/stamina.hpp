#pragma once

namespace mion {

// Stamina pool with regeneration delay.
// Regen is paused for regen_delay seconds after any consumption.
struct StaminaState {
    float current               = 100.0f;
    float max                   = 100.0f;
    float regen_rate            = 30.0f;  // per second
    float regen_delay           = 1.0f;   // seconds without regen after consumption
    float regen_delay_remaining = 0.0f;

    bool can_afford(float cost) const { return current >= cost; }

    void consume(float cost) {
        current -= cost;
        if (current < 0.0f) current = 0.0f;
        regen_delay_remaining = regen_delay;
    }

    void tick(float dt) {
        if (regen_delay_remaining > 0.0f) {
            regen_delay_remaining -= dt;
            return;
        }
        current += regen_rate * dt;
        if (current > max) current = max;
    }
};

} // namespace mion
