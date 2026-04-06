#pragma once

#include "../core/audio.hpp"
#include "../core/scene_context.hpp"

namespace mion {

namespace DungeonAudioSystem {

inline void init_dungeon_audio(AudioSystem& audio) {
    audio.stop_all_ambient();
    audio.play_ambient(AmbientId::DungeonDrips, 0.22f);
    audio.play_ambient(AmbientId::DungeonTorch, 0.14f);
    audio.set_music_state(MusicState::DungeonCalm);
}

inline void update_dungeon_music(AudioSystem& audio, const Actor& player,
                                 const std::vector<Actor>& enemies,
                                 int room_index) {
    MusicState target = MusicState::DungeonCalm;
    bool boss_alive = false;
    if (room_index == 3) {
        for (const auto& enemy : enemies) {
            if (enemy.is_alive && enemy.name == "Grimjaw") {
                boss_alive = true;
                break;
            }
        }
    }

    if (boss_alive) {
        target = MusicState::DungeonBoss;
    } else {
        bool any_aggro = false;
        for (const auto& enemy : enemies) {
            if (!enemy.is_alive)
                continue;
            const float dx = player.transform.x - enemy.transform.x;
            const float dy = player.transform.y - enemy.transform.y;
            if (dx * dx + dy * dy < enemy.aggro_range * enemy.aggro_range) {
                any_aggro = true;
                break;
            }
        }
        target = any_aggro ? MusicState::DungeonCombat : MusicState::DungeonCalm;
    }

    if (target != audio.current_music_state())
        audio.set_music_state(target);
}

} // namespace DungeonAudioSystem

} // namespace mion
