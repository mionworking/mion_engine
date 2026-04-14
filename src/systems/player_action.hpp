#pragma once
#include <vector>
#include "../entities/actor.hpp"
#include "../entities/projectile.hpp"
#include "../core/input.hpp"
#include "../core/audio.hpp"
#include "dash_system.hpp"
#include "spell_system.hpp"
#include "spell_effects.hpp"

namespace mion {

struct PlayerActionSystem {
    static constexpr float kMelee_stamina_cost  = 8.0f;

    void fixed_update(Actor& player, const InputState& input, float dt,
                      AudioSystem* audio,
                      std::vector<Projectile>* spawn_projectiles,
                      std::vector<Actor*>* actors,
                      int* out_spell_casts = nullptr) {
        if (!player.is_alive) return;

        SpellSystem::tick_cooldowns(player, dt);
        DashSystem::tick_cooldown(player, dt);

        if (player.status_effects.has(StatusEffectType::Stun)) {
            player.is_moving = false;
            return;
        }

        if (DashSystem::update_active_dash(player, dt))
            return;

        if (DashSystem::try_start_dash(player, input, dt, audio))
            return;

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
                player.player->spell_book.is_unlocked(SpellId::Cleave)
                && player.player->spell_book.can_cast(SpellId::Cleave, player.player->mana);
            if (cleave_ready && player.player->stamina.can_afford(kMelee_stamina_cost) && actors) {
                player.player->stamina.consume(kMelee_stamina_cost);
                const SpellDef& cdef = spell_def(SpellId::Cleave);
                player.player->mana.consume(cdef.mana_cost);
                const int rank =
                    spell_damage_rank(SpellId::Cleave, player.player->talents);
                int base = cdef.effective_damage(rank);
                const float mm =
                    1.0f + 0.5f * (float)player.player->talents.level_of(TalentId::MeleeForce);
                const int dmg =
                    (int)((float)SpellSystem::apply_outgoing_spell_damage(player, base) * mm);
                apply_cleave(player, *actors, cdef.radius, dmg);
                player.player->spell_book.start_cooldown(SpellId::Cleave, player.player->talents);
                if (audio) audio->play_sfx_pitched(SoundId::SkillCleave);
            } else if (player.player->stamina.can_afford(kMelee_stamina_cost)) {
                player.player->stamina.consume(kMelee_stamina_cost);
                player.combat.begin_attack();
                if (audio) audio->play_sfx_pitched(SoundId::PlayerAttack);
            }
        }

        SpellSystem::try_ranged_attack(player, input, audio, spawn_projectiles);
        SpellSystem::try_cast_spell_inputs(
            player, input, audio, spawn_projectiles, actors, out_spell_casts);
    }
};

} // namespace mion
