#pragma once

#include "../core/animation.hpp"
#include "../entities/actor.hpp"
#include "../systems/world_renderer.hpp"

namespace mion {

namespace AnimationDriver {

inline void update_basic_actor_anim(Actor& actor, float dt) {
    actor.anim.update_puny_dir_row(facing_to_puny_row(actor.facing_x, actor.facing_y));
    if (!actor.sprite_sheet)
        return;
    if (!actor.is_alive)
        actor.anim.play(ActorAnim::Death);
    else if (actor.combat.attack_phase != AttackPhase::Idle)
        actor.anim.play(ActorAnim::Attack);
    else if (actor.combat.hurt_stun_remaining_seconds > 0.0f)
        actor.anim.play(ActorAnim::Hurt);
    else if (actor.is_moving)
        actor.anim.play(ActorAnim::Walk);
    else
        actor.anim.play(ActorAnim::Idle);
    actor.anim.advance(dt);
}

inline void update_player_town_anim(Actor& player, float dt) {
    player.anim.update_puny_dir_row(facing_to_puny_row(player.facing_x, player.facing_y));
    if (!player.sprite_sheet)
        return;
    if (player.is_dashing())
        player.anim.play(ActorAnim::Dash);
    else if (player.combat.attack_phase != AttackPhase::Idle)
        player.anim.play(ActorAnim::Attack);
    else if (player.is_moving)
        player.anim.play(ActorAnim::Walk);
    else
        player.anim.play(ActorAnim::Idle);
    player.anim.advance(dt);
}

inline void update_player_dungeon_anim(Actor& player, float cast_timer, float dt) {
    player.anim.update_puny_dir_row(facing_to_puny_row(player.facing_x, player.facing_y));
    if (!player.sprite_sheet)
        return;
    if (!player.is_alive)
        player.anim.play(ActorAnim::Death);
    else if (player.is_dashing())
        player.anim.play(ActorAnim::Dash);
    else if (cast_timer > 0.0f)
        player.anim.play(ActorAnim::Cast);
    else if (player.combat.attack_phase != AttackPhase::Idle)
        player.anim.play(ActorAnim::Attack);
    else if (player.combat.hurt_stun_remaining_seconds > 0.0f)
        player.anim.play(ActorAnim::Hurt);
    else if (player.is_moving)
        player.anim.play(ActorAnim::Walk);
    else
        player.anim.play(ActorAnim::Idle);
    player.anim.advance(dt);
}

} // namespace AnimationDriver

} // namespace mion
