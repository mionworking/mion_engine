#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include "../components/spell_book.hpp"
#include "../components/spell_defs.hpp"
#include "../components/talent_tree.hpp"
#include "../core/audio.hpp"
#include "../core/input.hpp"
#include "../entities/actor.hpp"
#include "../entities/projectile.hpp"
#include "spell_effects.hpp"

namespace mion {

struct SpellSystem {
    static constexpr float kRangedStaminaCost = 14.0f;
    static constexpr float kArrowSpeed        = 420.0f;

    static void tick_cooldowns(Actor& player, float dt) {
        player.spell_book.tick(dt);
        if (player.ranged_cooldown_remaining_seconds > 0.0f)
            player.ranged_cooldown_remaining_seconds -= dt;
    }

    static int apply_outgoing_spell_damage(Actor& player, int base_damage) {
        int damage = static_cast<int>(
            std::lround(static_cast<float>(base_damage) * player.derived.spell_damage_mult));
        const float mult = player.outgoing_damage_multiplier();
        if (mult != 1.0f)
            damage = static_cast<int>(std::lround(static_cast<float>(damage) * mult));
        return damage;
    }

    static int apply_outgoing_physical_damage(Actor& player, int base_damage) {
        int damage = base_damage;
        const float mult = player.outgoing_damage_multiplier();
        if (mult != 1.0f)
            damage = static_cast<int>(std::lround(static_cast<float>(damage) * mult));
        return damage;
    }

    static void spawn_directional_projectile(Actor& player, std::vector<Projectile>& out,
                                             float dir_x, float dir_y, float speed,
                                             int damage, bool poison, int poison_rank) {
        const float len = std::sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len < 0.0001f)
            return;

        const float nx = dir_x / len;
        const float ny = dir_y / len;
        Projectile  projectile;
        projectile.x                          = player.transform.x + nx * 24.0f;
        projectile.y                          = player.transform.y + ny * 24.0f;
        projectile.vx                         = nx * speed;
        projectile.vy                         = ny * speed;
        projectile.lifetime_remaining_seconds = 2.2f;
        projectile.damage                     = damage;
        projectile.owner_team                 = Team::Player;
        if (poison && poison_rank > 0) {
            projectile.is_poison = true;
            projectile.poison_rank = poison_rank;
        }
        out.push_back(projectile);
    }

    static void try_ranged_attack(Actor& player, const InputState& input, AudioSystem* audio,
                                  std::vector<Projectile>* spawn_projectiles) {
        if (!spawn_projectiles || !input.ranged_pressed
            || player.ranged_cooldown_remaining_seconds > 0.0f
            || !player.combat.is_attack_idle()
            || player.combat.is_hurt_stunned()) {
            return;
        }
        if (!player.stamina.can_afford(kRangedStaminaCost))
            return;

        player.stamina.consume(kRangedStaminaCost);
        player.ranged_cooldown_remaining_seconds = player.ranged_cooldown_seconds;

        const int base_damage = player.derived.ranged_damage_final;
        int shot_count = 1;
        if (player.spell_book.is_unlocked(SpellId::MultiShot))
            shot_count += std::min(2, player.talents.level_of(TalentId::MultiShot));

        const bool poison = player.spell_book.is_unlocked(SpellId::PoisonArrow)
                         && player.talents.level_of(TalentId::PoisonArrow) > 0;
        const int poison_rank = player.talents.level_of(TalentId::PoisonArrow);
        const int damage = apply_outgoing_physical_damage(player, base_damage);
        const float base_angle = std::atan2(player.facing_y, player.facing_x);
        const float dtheta = (shot_count <= 1) ? 0.0f : 24.0f * (3.14159265f / 180.0f);

        for (int shot = 0; shot < shot_count; ++shot) {
            float offset = 0.0f;
            if (shot_count > 1)
                offset = static_cast<float>(shot - (shot_count - 1) / 2) * dtheta;
            const float angle = base_angle + offset;
            const float dx    = std::cos(angle);
            const float dy    = std::sin(angle);
            spawn_directional_projectile(player, *spawn_projectiles, dx, dy,
                                         kArrowSpeed, damage, poison, poison_rank);
        }

        if (audio)
            audio->play_sfx_pitched(SoundId::RangedAttack);
    }

    static void try_cast_spell_inputs(Actor& player, const InputState& input, AudioSystem* audio,
                                      std::vector<Projectile>* spawn_projectiles,
                                      std::vector<Actor*>* actors,
                                      int* out_spell_casts = nullptr) {
        if (spawn_projectiles && input.spell_1_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const SpellId spell_id = SpellId::FrostBolt;
            if (player.spell_book.can_cast(spell_id, player.mana)) {
                const SpellDef& def = spell_def(spell_id);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(spell_id, player.talents);
                const int rank = spell_damage_rank(spell_id, player.talents);
                const int base = def.effective_damage(rank)
                               + 2 * player.talents.level_of(TalentId::SpellPower);
                const int damage = apply_outgoing_spell_damage(player, base);

                Projectile projectile;
                projectile.x                          = player.transform.x + player.facing_x * 24.0f;
                projectile.y                          = player.transform.y + player.facing_y * 24.0f;
                projectile.vx                         = player.facing_x * def.speed;
                projectile.vy                         = player.facing_y * def.speed;
                projectile.lifetime_remaining_seconds = 2.0f;
                projectile.damage                     = damage;
                projectile.owner_team                 = Team::Player;
                projectile.is_frost                   = true;
                projectile.frost_rank =
                    std::max(1, player.talents.level_of(TalentId::FrostBolt));
                spawn_projectiles->push_back(projectile);
                if (out_spell_casts)
                    ++*out_spell_casts;
                if (audio)
                    audio->play_sfx_pitched(SoundId::SpellFrost);
            }
        }

        if (actors && input.spell_2_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const SpellId spell_id = SpellId::Nova;
            if (player.spell_book.can_cast(spell_id, player.mana)) {
                const SpellDef& def = spell_def(spell_id);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(spell_id, player.talents);
                const int rank = spell_damage_rank(spell_id, player.talents);
                const int base = def.effective_damage(rank);
                apply_nova(player, *actors, def.radius,
                           apply_outgoing_spell_damage(player, base));
                if (out_spell_casts)
                    ++*out_spell_casts;
                if (audio)
                    audio->play_sfx_pitched(SoundId::SpellNova);
            }
        }

        if (spawn_projectiles && input.spell_3_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const bool can_chain = player.spell_book.can_cast(SpellId::ChainLightning, player.mana);
            const bool can_strafe = player.spell_book.can_cast(SpellId::Strafe, player.mana);

            if (can_chain && actors) {
                const SpellDef& def = spell_def(SpellId::ChainLightning);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(SpellId::ChainLightning, player.talents);
                const int rank = spell_damage_rank(SpellId::ChainLightning, player.talents);
                const int base = def.effective_damage(rank)
                               + 2 * player.talents.level_of(TalentId::SpellPower);
                const int damage = apply_outgoing_spell_damage(player, base);
                const int bounces = 2 + std::min(3, rank / 2);
                apply_chain_lightning(player, *actors, bounces, damage);
                if (out_spell_casts)
                    ++*out_spell_casts;
                if (audio)
                    audio->play_sfx_pitched(SoundId::SpellChain);
            } else if (can_strafe) {
                const SpellDef& def = spell_def(SpellId::Strafe);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(SpellId::Strafe, player.talents);
                const int damage = apply_outgoing_physical_damage(
                    player,
                    player.derived.ranged_damage_final
                        + player.talents.level_of(TalentId::Strafe));
                const float base_angle = std::atan2(player.facing_y, player.facing_x);
                for (int i = 0; i < 5; ++i) {
                    const float delta_angle = (-30.0f + 15.0f * static_cast<float>(i))
                                            * (3.14159265f / 180.0f);
                    const float angle = base_angle + delta_angle;
                    const float dx = std::cos(angle);
                    const float dy = std::sin(angle);
                    spawn_directional_projectile(player, *spawn_projectiles, dx, dy,
                                                 def.speed, damage, false, 0);
                }
                if (out_spell_casts)
                    ++*out_spell_casts;
                if (audio)
                    audio->play_sfx_pitched(SoundId::SkillMultiShot);
            }
        }

        if (input.spell_4_pressed
            && player.combat.is_attack_idle()
            && !player.combat.is_hurt_stunned()) {
            const SpellId spell_id = SpellId::BattleCry;
            if (player.spell_book.can_cast(spell_id, player.mana)) {
                const SpellDef& def = spell_def(spell_id);
                player.mana.consume(def.mana_cost);
                player.spell_book.start_cooldown(spell_id, player.talents);
                apply_battle_cry(player,
                                 std::max(1, player.talents.level_of(TalentId::BattleCry)));
                if (out_spell_casts)
                    ++*out_spell_casts;
                if (audio)
                    audio->play_sfx_pitched(SoundId::SkillBattleCry);
            }
        }
    }
};

} // namespace mion
