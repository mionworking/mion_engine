#pragma once
#include <algorithm>

#include "../core/audio.hpp"
#include "../core/input.hpp"
#include "../entities/actor.hpp"

namespace mion {

struct DashSystem {
    static constexpr float kDashStaminaCost = 28.0f;

    static void tick_cooldown(Actor& player, float dt) {
        if (player.player->dash_cooldown_remaining_seconds > 0.0f)
            player.player->dash_cooldown_remaining_seconds -= dt;
    }

    static bool update_active_dash(Actor& player, float dt) {
        if (!player.is_dashing())
            return false;

        player.transform.translate(
            player.player->dash_dir_x * player.player->dash_speed * dt,
            player.player->dash_dir_y * player.player->dash_speed * dt);
        player.is_moving = true;
        player.player->dash_active_remaining_seconds -= dt;
        if (player.player->dash_active_remaining_seconds < 0.0f)
            player.player->dash_active_remaining_seconds = 0.0f;
        return true;
    }

    static bool try_start_dash(Actor& player, const InputState& input, float dt,
                               AudioSystem* audio) {
        if (!input.dash_pressed
            || player.player->dash_cooldown_remaining_seconds > 0.0f
            || player.combat.is_hurt_stunned()
            || !player.combat.is_attack_idle()) {
            return false;
        }
        if (!player.player->stamina.can_afford(kDashStaminaCost))
            return false;

        player.player->stamina.consume(kDashStaminaCost);

        float nx = 0.0f;
        float ny = 0.0f;
        input.normalized_movement(nx, ny);
        if (nx == 0.0f && ny == 0.0f) {
            nx = player.facing_x;
            ny = player.facing_y;
        }
        player.player->dash_dir_x = nx;
        player.player->dash_dir_y = ny;
        player.player->dash_active_remaining_seconds   = player.player->dash_duration_seconds;
        player.player->dash_cooldown_remaining_seconds = player.player->dash_cooldown_seconds;
        player.combat.impact_invulnerability_remaining =
            std::max(player.combat.impact_invulnerability_remaining,
                     player.player->dash_iframes_seconds);

        player.transform.translate(
            player.player->dash_dir_x * player.player->dash_speed * dt,
            player.player->dash_dir_y * player.player->dash_speed * dt);
        player.is_moving = true;
        player.player->dash_active_remaining_seconds -= dt;
        if (player.player->dash_active_remaining_seconds < 0.0f)
            player.player->dash_active_remaining_seconds = 0.0f;
        if (audio)
            audio->play_sfx_pitched(SoundId::Dash);
        return true;
    }
};

} // namespace mion
