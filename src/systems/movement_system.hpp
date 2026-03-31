#pragma once
#include <vector>
#include "../entities/actor.hpp"

namespace mion {

// Responsabilidades:
// - Resetar flag is_moving antes de cada frame de movimento.
// - Aplicar step de knockback para todos os actores vivos.
// O movimento dirigido pelo player fica em PlayerActionSystem.
// O movimento de inimigos fica em EnemyAISystem.
struct MovementSystem {
    void fixed_update(std::vector<Actor*>& actors, float dt) {
        for (auto* a : actors) a->is_moving = false;
        for (auto* a : actors) if (a->is_alive) a->step_knockback(dt);
    }
};

} // namespace mion
