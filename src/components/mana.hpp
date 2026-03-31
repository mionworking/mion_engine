#pragma once

namespace mion {

struct ManaState {
    float current               = 100.0f;
    float max                   = 100.0f;
    float regen_rate            = 20.0f;  // por segundo
    float regen_delay           = 0.75f;  // segundos sem regen após consumo
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
            if (regen_delay_remaining < 0.0f) regen_delay_remaining = 0.0f;
            return;
        }
        current += regen_rate * dt;
        if (current > max) current = max;
    }
};

} // namespace mion
