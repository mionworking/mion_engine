#pragma once
#include <vector>
#include <string>
#include <cmath>
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../core/debug_log.hpp"

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
    float combo_finisher_knockback_mult = 2.35f;
    int   hitstop_frames       = 4;
    int   pending_hitstop      = 0;
    bool  player_hit_landed    = false;
    bool  parry_success        = false;

    static constexpr float kParryConnect_range = 58.0f;

    void resolve_parry_vs_startup(std::vector<Actor*>& actors) {
        parry_success = false;
        Actor* player = nullptr;
        for (auto* a : actors) {
            if (a->team == Team::Player && a->is_alive) { player = a; break; }
        }
        if (!player || player->combat.parry_window_remaining_seconds <= 0.0f) return;

        for (auto* enemy : actors) {
            if (enemy->team != Team::Enemy || !enemy->is_alive) continue;
            if (!enemy->combat.is_in_startup_phase()) continue;

            float dx = player->transform.x - enemy->transform.x;
            float dy = player->transform.y - enemy->transform.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > kParryConnect_range) continue;

            enemy->combat.apply_parry_break_stun(0.65f);
            player->combat.parry_window_remaining_seconds = 0.0f;
            parry_success = true;
            return;
        }
    }

    void fixed_update(std::vector<Actor*>& actors, float /*dt*/) {
        last_events.clear();
        pending_hitstop   = 0;
        player_hit_landed = false;

        resolve_parry_vs_startup(actors);

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
                if (target->team == attacker->team) continue;
                if (!target->combat.can_receive_hit()) continue;

                AABB hurt_bounds = target->hurt_box.bounds_at(
                    target->transform.x, target->transform.y
                );

                if (!hit_bounds.intersects(hurt_bounds)) continue;

                // Dano base — player usa derived.melee_damage_final (centralizado)
                int dmg = (attacker->team == Team::Player)
                    ? attacker->derived.melee_damage_final
                    : attacker->attack_damage;
                // #region agent log
                append_debug_log_line(
                    "pre-fix",
                    "H4_melee_damage_asymmetry",
                    "src/systems/melee_combat.hpp:84",
                    "Melee base damage source",
                    std::string("{\"attackerTeam\":")
                    + std::to_string(static_cast<int>(attacker->team))
                    + ",\"derivedMelee\":"
                    + std::to_string(attacker->derived.melee_damage_final)
                    + ",\"attackDamage\":"
                    + std::to_string(attacker->attack_damage)
                    + ",\"chosenBase\":"
                    + std::to_string(dmg)
                    + "}"
                );
                // #endregion
                {
                    const float om = attacker->outgoing_damage_multiplier();
                    if (om != 1.0f)
                        dmg = (int)std::lround((float)dmg * om);
                }

                bool combo_finisher = false;
                if (attacker->team == Team::Player) {
                    attacker->combat.register_combo_hit();
                    if (attacker->combat.combo_chain_hits >= 3) {
                        attacker->combat.combo_chain_hits = 0;
                        attacker->combat.combo_chain_time_remaining_seconds = 0.0f;
                        combo_finisher = true;
                    }
                }

                target->combat.apply_hit_reaction();
                target->health.apply_damage(dmg);
                attacker->combat.attack_hit_landed = true;

                if (!target->health.is_alive()) target->is_alive = false;

                float impulse = knockback_impulse;
                if (combo_finisher) impulse *= combo_finisher_knockback_mult;

                float dx = target->transform.x - attacker->transform.x;
                float dy = target->transform.y - attacker->transform.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 0.0f) {
                    target->apply_knockback_impulse(
                        (dx / dist) * impulse,
                        (dy / dist) * impulse
                    );
                }

                // Status em hits de inimigos
                if (attacker->team == Team::Enemy && target->team == Team::Player) {
                    switch (attacker->enemy_type) {
                        case EnemyType::Ghost:
                        case EnemyType::Archer:
                            target->status_effects.apply_full(
                                StatusEffectType::Slow, 2.4f, 0.55f, 3, 0.48f);
                            break;
                        case EnemyType::Skeleton:
                        case EnemyType::PatrolGuard:
                            target->status_effects.apply_full(
                                StatusEffectType::Poison, 5.0f, 0.5f, 4, 0.52f);
                            break;
                        case EnemyType::EliteSkeleton:
                            target->status_effects.apply_full(
                                StatusEffectType::Poison, 6.0f, 0.5f, 6, 0.52f);
                            break;
                        case EnemyType::Orc:
                        case EnemyType::BossGrimjaw:
                            target->status_effects.apply(StatusEffectType::Stun, 0.38f);
                            break;
                        default:
                            break;
                    }
                }

                pending_hitstop = hitstop_frames;
                if (attacker->team == Team::Player)
                    player_hit_landed = true;

                last_events.push_back({
                    attacker->name,
                    target->name,
                    dmg
                });

                break;
            }
        }
    }
};

} // namespace mion
