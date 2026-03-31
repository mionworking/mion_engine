#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "../entities/actor.hpp"
#include "../entities/projectile.hpp"
#include "../components/spell_book.hpp"
#include "../components/spell_defs.hpp"
#include "../components/talent_tree.hpp"
#include "../core/input.hpp"
#include "../core/audio.hpp"
#include "spell_effects.hpp"

namespace mion {

struct PlayerActionSystem {
    static constexpr float kMelee_stamina_cost  = 8.0f;
    static constexpr float kDash_stamina_cost   = 28.0f;
    static constexpr float kRanged_stamina_cost = 14.0f;
    static constexpr float kArrow_speed       = 420.0f;

    static int apply_outgoing_spell_damage(Actor& player, int base_damage) {
        // Usa derived.spell_damage_mult (centralizado em recompute_player_derived_stats)
        int d = (int)std::lround((float)base_damage * player.derived.spell_damage_mult);
        const float om = player.outgoing_damage_multiplier();
        if (om != 1.0f)
            d = (int)std::lround((float)d * om);
        return d;
    }

    static int apply_outgoing_physical_damage(Actor& player, int base_damage) {
        int d = base_damage;
        const float om = player.outgoing_damage_multiplier();
        if (om != 1.0f)
            d = (int)std::lround((float)d * om);
        return d;
    }

    static void spawn_directional_projectile(Actor& player, std::vector<Projectile>& out,
                                             float dir_x, float dir_y, float speed,
                                             int damage, bool poison, int poison_rank) {
        float len = std::sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len < 0.0001f) return;
        float nx = dir_x / len;
        float ny = dir_y / len;
        Projectile p;
        p.x = player.transform.x + nx * 24.0f;
        p.y = player.transform.y + ny * 24.0f;
        p.vx = nx * speed;
        p.vy = ny * speed;
        p.lifetime_remaining_seconds = 2.2f;
        p.damage     = damage;
        p.owner_team = Team::Player;
        if (poison && poison_rank > 0) {
            p.is_poison  = true;
            p.poison_rank = poison_rank;
        }
        out.push_back(p);
    }

    void fixed_update(Actor& player, const InputState& input, float dt,
                      AudioSystem* audio,
                      std::vector<Projectile>* spawn_projectiles,
                      std::vector<Actor*>* actors,
                      int* out_spell_casts = nullptr) {
        if (!player.is_alive) return;

        player.spell_book.tick(dt);
        if (player.ranged_cooldown_remaining_seconds > 0.0f)
            player.ranged_cooldown_remaining_seconds -= dt;

        if (player.dash_cooldown_remaining_seconds > 0.0f)
            player.dash_cooldown_remaining_seconds -= dt;

        if (player.status_effects.has(StatusEffectType::Stun)) {
            player.is_moving = false;
            return;
        }

        if (player.is_dashing()) {
            player.transform.translate(
                player.dash_dir_x * player.dash_speed * dt,
                player.dash_dir_y * player.dash_speed * dt
            );
            player.is_moving = true;
            player.dash_active_remaining_seconds -= dt;
            if (player.dash_active_remaining_seconds < 0.0f)
                player.dash_active_remaining_seconds = 0.0f;
            return;
        }

        if (input.dash_pressed
            && player.dash_cooldown_remaining_seconds <= 0.0f
            && !player.combat.is_hurt_stunned()
            && player.combat.is_attack_idle()) {
            if (player.stamina.can_afford(kDash_stamina_cost)) {
                player.stamina.consume(kDash_stamina_cost);

                float nx, ny;
                input.normalized_movement(nx, ny);
                if (nx == 0.0f && ny == 0.0f) {
                    nx = player.facing_x;
                    ny = player.facing_y;
                }
                player.dash_dir_x = nx;
                player.dash_dir_y = ny;
                player.dash_active_remaining_seconds  = player.dash_duration_seconds;
                player.dash_cooldown_remaining_seconds = player.dash_cooldown_seconds;
                player.combat.impact_invulnerability_remaining =
                    std::max(player.combat.impact_invulnerability_remaining,
                             player.dash_iframes_seconds);

                player.transform.translate(
                    player.dash_dir_x * player.dash_speed * dt,
                    player.dash_dir_y * player.dash_speed * dt
                );
                player.is_moving = true;
                player.dash_active_remaining_seconds -= dt;
                if (player.dash_active_remaining_seconds < 0.0f)
                    player.dash_active_remaining_seconds = 0.0f;
                if (audio) audio->play_sfx_pitched(SoundId::Dash);
                return;
            }
        }

        float nx, ny;
        input.normalized_movement(nx, ny);
        float speed = player.effective_move_speed();
        player.transform.translate(nx * speed * dt, ny * speed * dt);
        player.set_facing(nx, ny);
        player.is_moving = (nx != 0.0f || ny != 0.0f);

        if (input.parry_pressed && player.combat.is_attack_idle() && !player.combat.is_hurt_stunned())
            player.combat.begin_parry();

        if (input.attack_pressed && player.combat.is_attack_idle()) {
            const bool cleave_ready =
                player.spell_book.is_unlocked(SpellId::Cleave)
                && player.spell_book.can_cast(SpellId::Cleave, player.mana);
            if (cleave_ready && player.stamina.can_afford(kMelee_stamina_cost) && actors) {
                player.stamina.consume(kMelee_stamina_cost);
                const SpellDef& cdef = spell_def(SpellId::Cleave);
                player.mana.consume(cdef.mana_cost);
                const int rank =
                    spell_damage_rank(SpellId::Cleave, player.talents);
                int base = cdef.effective_damage(rank);
                const float mm =
                    1.0f + 0.5f * (float)player.talents.level_of(TalentId::MeleeForce);
                const int dmg =
                    (int)((float)apply_outgoing_spell_damage(player, base) * mm);
                apply_cleave(player, *actors, cdef.radius, dmg);
                player.spell_book.start_cooldown(SpellId::Cleave, player.talents);
                if (audio) audio->play_sfx_pitched(SoundId::SkillCleave);
            } else if (player.stamina.can_afford(kMelee_stamina_cost)) {
                player.stamina.consume(kMelee_stamina_cost);
                player.combat.begin_attack();
                if (audio) audio->play_sfx_pitched(SoundId::PlayerAttack);
            }
        }

        if (spawn_projectiles && input.ranged_pressed
            && player.ranged_cooldown_remaining_seconds <= 0.0f
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            if (player.stamina.can_afford(kRanged_stamina_cost)) {
                player.stamina.consume(kRanged_stamina_cost);
                player.ranged_cooldown_remaining_seconds = player.ranged_cooldown_seconds;

                int base_dmg = player.derived.ranged_damage_final;

                int n_shots = 1;
                if (player.spell_book.is_unlocked(SpellId::MultiShot)) {
                    n_shots +=
                        std::min(2, player.talents.level_of(TalentId::MultiShot));
                }

                const bool poison = player.spell_book.is_unlocked(SpellId::PoisonArrow)
                                    && player.talents.level_of(TalentId::PoisonArrow) > 0;
                const int poison_rank = player.talents.level_of(TalentId::PoisonArrow);

                const int dmg = apply_outgoing_physical_damage(player, base_dmg);

                const float base_ang =
                    std::atan2(player.facing_y, player.facing_x);
                const float dtheta =
                    (n_shots <= 1) ? 0.0f : 24.0f * (3.14159265f / 180.0f);

                for (int s = 0; s < n_shots; ++s) {
                    float off = 0.0f;
                    if (n_shots > 1)
                        off = (float)(s - (n_shots - 1) / 2) * dtheta;
                    const float ang = base_ang + off;
                    const float dx  = std::cos(ang);
                    const float dy  = std::sin(ang);
                    spawn_directional_projectile(player, *spawn_projectiles, dx, dy,
                                                 kArrow_speed, dmg, poison, poison_rank);
                }
                if (audio) audio->play_sfx_pitched(SoundId::RangedAttack);
            }
        }

        if (spawn_projectiles && input.spell_1_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const SpellId sid = SpellId::FrostBolt;
            if (player.spell_book.can_cast(sid, player.mana)) {
                const SpellDef& def = spell_def(sid);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(sid, player.talents);
                const int rank = spell_damage_rank(sid, player.talents);
                int base = def.effective_damage(rank)
                         + 2 * player.talents.level_of(TalentId::SpellPower);
                const int dmg = apply_outgoing_spell_damage(player, base);

                Projectile p;
                p.x = player.transform.x + player.facing_x * 24.0f;
                p.y = player.transform.y + player.facing_y * 24.0f;
                p.vx = player.facing_x * def.speed;
                p.vy = player.facing_y * def.speed;
                p.lifetime_remaining_seconds = 2.0f;
                p.damage                   = dmg;
                p.owner_team               = Team::Player;
                p.is_frost = true;
                p.frost_rank =
                    std::max(1, player.talents.level_of(TalentId::FrostBolt));
                spawn_projectiles->push_back(p);
                if (out_spell_casts) ++*out_spell_casts;
                if (audio) audio->play_sfx_pitched(SoundId::SpellFrost);
            }
        }

        if (actors && input.spell_2_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const SpellId sid = SpellId::Nova;
            if (player.spell_book.can_cast(sid, player.mana)) {
                const SpellDef& def = spell_def(sid);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(sid, player.talents);
                const int rank = spell_damage_rank(sid, player.talents);
                const int base = def.effective_damage(rank);
                apply_nova(player, *actors, def.radius,
                           apply_outgoing_spell_damage(player, base));
                if (out_spell_casts) ++*out_spell_casts;
                if (audio) audio->play_sfx_pitched(SoundId::SpellNova);
            }
        }

        if (spawn_projectiles && input.spell_3_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const bool can_chain = player.spell_book.can_cast(SpellId::ChainLightning,
                                                              player.mana);
            const bool can_strafe =
                player.spell_book.can_cast(SpellId::Strafe, player.mana);

            if (can_chain && actors) {
                const SpellDef& def = spell_def(SpellId::ChainLightning);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(SpellId::ChainLightning,
                                                 player.talents);
                const int rank = spell_damage_rank(SpellId::ChainLightning,
                                                   player.talents);
                int base = def.effective_damage(rank)
                         + 2 * player.talents.level_of(TalentId::SpellPower);
                const int dmg = apply_outgoing_spell_damage(player, base);
                const int bounces = 2 + std::min(3, rank / 2);
                apply_chain_lightning(player, *actors, bounces, dmg);
                if (out_spell_casts) ++*out_spell_casts;
                if (audio) audio->play_sfx_pitched(SoundId::SpellChain);
            } else if (can_strafe) {
                const SpellDef& def = spell_def(SpellId::Strafe);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(SpellId::Strafe, player.talents);
                int rdmg = apply_outgoing_physical_damage(
                    player,
                    player.derived.ranged_damage_final
                        + player.talents.level_of(TalentId::Strafe));
                const float base_ang = std::atan2(player.facing_y, player.facing_x);
                for (int i = 0; i < 5; ++i) {
                    const float da = (-30.0f + 15.0f * (float)i)
                                   * (3.14159265f / 180.0f);
                    const float ang  = base_ang + da;
                    const float dx   = std::cos(ang);
                    const float dy   = std::sin(ang);
                    spawn_directional_projectile(player, *spawn_projectiles, dx, dy,
                                                 def.speed, rdmg, false, 0);
                }
                if (out_spell_casts) ++*out_spell_casts;
                if (audio) audio->play_sfx_pitched(SoundId::SkillMultiShot);
            }
        }

        if (input.spell_4_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const SpellId sid = SpellId::BattleCry;
            if (player.spell_book.can_cast(sid, player.mana)) {
                const SpellDef& def = spell_def(sid);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(sid, player.talents);
                apply_battle_cry(player,
                                 std::max(1, player.talents.level_of(TalentId::BattleCry)));
                if (out_spell_casts) ++*out_spell_casts;
                if (audio) audio->play_sfx_pitched(SoundId::SkillBattleCry);
            }
        }
    }
};

} // namespace mion
