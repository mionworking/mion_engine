#pragma once
#include <algorithm>

namespace mion {

struct HealthState {
    int max_hp     = 100;
    int current_hp = 100;

    bool is_alive() const { return current_hp > 0; }

    void apply_damage(int amount) {
        current_hp = std::max(0, current_hp - amount);
    }

    void restore_full() { current_hp = max_hp; }
};

} // namespace mion
