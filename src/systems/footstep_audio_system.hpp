#pragma once

#include <cmath>
#include <vector>

#include "../core/audio.hpp"
#include "../core/camera.hpp"
#include "../entities/actor.hpp"

namespace mion {

namespace FootstepAudioSystem {

inline void update_footsteps(AudioSystem& audio,
                             Actor& player,
                             std::vector<Actor>& enemies,
                             const Camera2D& camera,
                             float player_step_dist,
                             float enemy_step_dist) {
    const float cam_x = camera.x + camera.viewport_w * 0.5f;
    const float cam_y = camera.y + camera.viewport_h * 0.5f;

    const float pdx = player.transform.x - player.footstep_prev_x;
    const float pdy = player.transform.y - player.footstep_prev_y;
    if (player.is_moving) {
        player.footstep_accum_dist += std::sqrt(pdx * pdx + pdy * pdy);
        while (player.footstep_accum_dist >= player_step_dist) {
            player.footstep_accum_dist -= player_step_dist;
            audio.play_sfx(SoundId::FootstepPlayer, 0.26f);
        }
    } else {
        player.footstep_accum_dist = 0.0f;
    }
    player.footstep_prev_x = player.transform.x;
    player.footstep_prev_y = player.transform.y;

    for (auto& enemy : enemies) {
        if (!enemy.is_alive)
            continue;
        const float dx = enemy.transform.x - enemy.footstep_prev_x;
        const float dy = enemy.transform.y - enemy.footstep_prev_y;
        if (enemy.is_moving) {
            enemy.footstep_accum_dist += std::sqrt(dx * dx + dy * dy);
            while (enemy.footstep_accum_dist >= enemy_step_dist) {
                enemy.footstep_accum_dist -= enemy_step_dist;
                audio.play_sfx_at(SoundId::FootstepEnemy,
                                  enemy.transform.x,
                                  enemy.transform.y,
                                  cam_x,
                                  cam_y,
                                  480.0f,
                                  0.2f);
            }
        } else {
            enemy.footstep_accum_dist = 0.0f;
        }
        enemy.footstep_prev_x = enemy.transform.x;
        enemy.footstep_prev_y = enemy.transform.y;
    }
}

} // namespace FootstepAudioSystem

} // namespace mion
