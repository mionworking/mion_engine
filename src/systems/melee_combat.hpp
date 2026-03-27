#pragma once
#include <vector>
#include <string>
#include <cmath>
#include "../entities/actor.hpp"

namespace mion {

struct CombatEvent {
    std::string attacker_name;
    std::string target_name;
    int         damage;
};

struct MeleeCombatSystem {
    std::vector<CombatEvent> last_events;
    int   base_damage          = 10;
    float knockback_impulse    = 320.0f;
    int   hitstop_frames       = 4;
    int   pending_hitstop      = 0;       // lido pelo main loop após fixed_update
    bool  player_hit_landed    = false;   // true só quando player acertou — controla shake

    void fixed_update(std::vector<Actor*>& actors, float /*dt*/) {
        last_events.clear();
        pending_hitstop   = 0;
        player_hit_landed = false;

        for (auto* attacker : actors) {
            if (!attacker->is_alive) continue;
            if (!attacker->combat.is_in_active_phase()) continue;
            if (attacker->combat.attack_hit_landed) continue;

            AABB hit_bounds = attacker->melee_hit_box.bounds_at(
                attacker->transform.x, attacker->transform.y,
                attacker->facing_x,    attacker->facing_y
            );

            for (auto* target : actors) {
                if (target == attacker) continue;
                if (!target->is_alive) continue;
                if (target->team == attacker->team) continue;  // friendly fire off
                if (!target->combat.can_receive_hit()) continue;

                AABB hurt_bounds = target->hurt_box.bounds_at(
                    target->transform.x, target->transform.y
                );

                if (!hit_bounds.intersects(hurt_bounds)) continue;

                // --- Acerto ---
                target->combat.apply_hit_reaction();
                target->health.apply_damage(attacker->attack_damage);
                attacker->combat.attack_hit_landed = true;

                if (!target->health.is_alive()) target->is_alive = false;

                // Knockback: empurra o alvo para longe do atacante
                float dx = target->transform.x - attacker->transform.x;
                float dy = target->transform.y - attacker->transform.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 0.0f) {
                    target->apply_knockback_impulse(
                        (dx / dist) * knockback_impulse,
                        (dy / dist) * knockback_impulse
                    );
                }

                pending_hitstop = hitstop_frames;
                if (attacker->team == Team::Player)
                    player_hit_landed = true;

                last_events.push_back({
                    attacker->name,
                    target->name,
                    attacker->attack_damage
                });

                break;  // um hit por swing, mesmo comportamento do Python
            }
        }
    }
};

} // namespace mion
